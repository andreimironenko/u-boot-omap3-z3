#! /bin/sh
# Script to create all binaries for Z3 NAND/UBI update

execute ()
{
    $* >/dev/null
    if [ $? -ne 0 ]; then
        echo
        echo "ERROR: executing $*"
        echo
        exit 1
    fi
}

declare ROOTFSDIR=""
declare BOOTCMD=""
declare ROOTFS=""
declare KERNEL=""
declare KERNEL_MODULES=""

declare NANDUPDATEDIR="nandupdate"
declare BOOTSCR="boot.scr"
declare NANDUPDATE_SCRIPT="nandupdate.sh"
declare UBOOTBIN="u-boot.bin"
declare UBOOT_MIN_UART="u-boot.min.uart"
declare UBOOT_MIN_NAND="u-boot.min.nand"  
declare UBI="ubi.img"
declare UBIFS="ubifs.img"
declare UBINIZE="ubinize.cfg"
declare AMUXI_EXPORTS_PATH="/opt/exports/z3"

declare YOURUSER=`whoami`
echo "Your user is $YOURUSER"


# Process command line...
while [ $# -gt 0 ]; do
  case $1 in
    --help | -h)
      usage $SCRIPT
      ;;
    --rootfsdir)            shift; ROOTFSDIR=$1; shift; ;;
    --bootcmd)               shift; BOOTCMD=$1; shift; ;;
    --rootfs)                shift; ROOTFS=$1; shift; ;;
    --kernel)                shift; KERNEL=$1; shift; ;;
    --kernel-modules)        shift; KERNEL_MODULES=$1; shift ;;
     _*)                     printf "%s\n" "Switch not supported" >&2; exit 192; ;;
      *)                     printf "%s\n" "Extra argument or missing switch" >&2; exit 192; ;;  
esac
done

if [ ! -f "$NANDUPDATEDIR/$NANDUPDATE_SCRIPT" ] ; then
   printf "%s\n" "Mandatory file $NANDUPDATE_SCRIPT is not found"
   printf "%s\n" "Try to rebuild system image"
fi

if [ ! -f "$NANDUPDATEDIR/$UBOOTBIN" ] ; then
   printf "%s\n" "Mandatory file $UBOOTBIN is not found"
   printf "%s\n" "Try to rebuild system image"
fi

if [ ! -f "$NANDUPDATEDIR/$UBOOT_MIN_UART" ] ; then
   printf "%s\n" "Mandatory file $UBOOT_MIN_UART is not found"
   printf "%s\n" "Try to rebuild system image"
fi

if [ ! -f "$NANDUPDATEDIR/$UBOOT_MIN_NAND" ] ; then
   printf "%s\n" "Mandatory file $UBOOT_MIN_NAND is not found"
   printf "%s\n" "Try to rebuild system image"
fi

if [ -z "$ROOTFS" ] ; then
   printf "%s\n" "Mandatory root file system image parameter is missed" >&2
   exit 192
fi

if [ -z "$KERNEL" ] ; then
   printf "%s\n" "Mandatory kernel image parameter is missed" >&2
   exit 192
fi

if [ -z "$KERNEL_MODULES" ] ; then
   printf "%s\n" "Mandatory kernel modules image parameter is missed" >&2
   exit 192
fi


echo "Creating export rootfs directory"
if [ -d $ROOTFSDIR ] ; then
  fakeroot rm -rf $ROOTFSDIR/*

else
  mkdir $ROOTFSDIR 
fi

ssh  $YOURUSER@amuxi mkdir -p $AMUXI_EXPORTS_PATH/$YOURUSER/$ROOTFSDIR
ssh  $YOURUSER@amuxi chgrp -R domain^users $AMUXI_EXPORTS_PATH/$YOURUSER/$ROOTFSDIR

echo "Exporting rootfs to AMUXI"
echo "All files from remote folder $AMUXI_EXPORTS_PATH/$YOURUSER/$ROOTFSDIR is going to be deleted, proceed (Y/N)?"

while  true; do
  read $REPLY
  if [ "$REPLY" = "y" -o "$REPLY" = "Y" ] ; then
    fakeroot rm -rf ./$ROOTFSDIR/*
    ssh $YOURUSER@amuxi rm -rf $AMUXI_EXPORTS_PATH/$YOURUSER/$ROOTFSDIR 
    break
  elif [ "$REPLY" = "n" -o "$REPLY" = "N" ] ; then
    echo "Backup the remote folder and run script again"
    exit 192
  else
    echo "Use Y/N:"
    continue
  fi
done


echo "Creating boot.scr and boot.img files"
cp $BOOTCMD  $BOOTCMD.$YOURUSER
echo "setenv nfs_root  '$AMUXI_EXPORTS_PATH/$YOURUSER/$ROOTFSDIR'" >> ./$BOOTCMD.$YOURUSER
echo "setenv uboot_env_img '$BOOTSCR'" >> ./$BOOTCMD.$YOURUSER
./mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n 'Execute uImage' -d ./$BOOTCMD.$YOURUSER ./$NANDUPDATEDIR/$BOOTSCR
rm $BOOTCMD.$YOURUSER

echo "Creating ubi.img - UBI NAND File System image"
echo "Untar rootfs tarball ..."
fakeroot tar xvfz $ROOTFS -C $ROOTFSDIR > /dev/null


echo "Untar kernel module tarball ..." 
fakeroot tar xvfz $KERNEL_MODULES -C $ROOTFSDIR > /dev/null

echo "Creating ubifs.img ..."

if [ -f $UBI ] ; then
 rm $UBI 
fi

if [ -f UBIFS ] ; then
 rm $UBIFS
fi 

fakeroot mkfs.ubifs -r $ROOTFSDIR -o $UBIFS -m 2048 -e 124KiB -c 1601
fakeroot ubinize -o $UBI -m 2048 -p 128KiB -s 512 -O 2048 $UBINIZE
mv $UBI $NANDUPDATEDIR/
rm $UBIFS

echo "Copying kernel image ... "
fakeroot cp $KERNEL $ROOTFSDIR/boot/uImage > /dev/null

echo "Copying NAND images to the NFS rootfs"
fakeroot cp -r $NANDUPDATEDIR $ROOTFSDIR/boot > /dev/null

echo "Creating exported rootfs tarball ..."
tar cvfz $ROOTFSDIR.tar.gz $ROOTFSDIR > /dev/null

echo "Copying tarball to AMUXI"
scp $ROOTFSDIR.tar.gz $YOURUSER@amuxi:$AMUXI_EXPORTS_PATH/$YOURUSER > /dev/null

echo "Inflating remote file system on AMUXI"
ssh $YOURUSER@amuxi tar xvfz $AMUXI_EXPORTS_PATH/$YOURUSER/$ROOTFSDIR.tar.gz -C $AMUXI_EXPORTS_PATH/$YOURUSER > /dev/null


echo ""
echo "-----------------------------------------------"
echo "Export has been successfully completed."
echo "Update u-boot and u-boot environment at first."
echo "Then reboot your board from NFS and "
echo "run /boot/nandupdate/nandupdate.sh script"
echo "to complete update procedure."
 







