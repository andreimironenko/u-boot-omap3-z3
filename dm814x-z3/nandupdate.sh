KERNEL="uImage"
UBIFS="ubi.img"

KERNEL_DEV="/dev/mtd5"
UBIFS_DEV="/dev/mtd6"

echo "Sanity checks first ..."

if [ ! -f "$KERNEL" ] ; then
   printf "%s\n" "Mandatory image $KERNEL is not found"
   printf "%s\n" "Try to re-export NFS file system again"
   exit 1
fi

if [ ! -f "$UBIFS" ] ; then
   printf "%s\n" "Mandatory image $UBIFS is not found"
   printf "%s\n" "Try to re-export NFS file system again"
   exit 1
fi

echo "Sanity check has: done"

echo "Flashing kernel image ..."
flash_erase $KERNEL_DEV 0 0
nandwrite -p $KERNEL_DEV $KERNEL
echo "Flashing kernel image: done"

echo "Flashing rootfs image ..."
umount $UBIFS_DEV
flash_erase $UBIFS_DEV 0 0
ubiformat $UBIFS_DEV -f $UBIFS -s 512 -O 2048
echo "Flashing rootfs image: done"

echo "NAND flash update has successfully completed"

~                                     
