setenv nfsserver '192.168.0.205'
setenv serverip '192.168.0.205'
setenv gatewayip '10.42.43.1'
setenv netmask '255.255.0.0'
setenv ipaddr '10.42.43.85'
setenv dnsip '8.8.8.8'
setenv dnsip2 '8.8.4.4'
setenv hostname 'Z3-DM814x'

setenv tftp_root 'z3/z3-dm814x/images'
setenv nfs_root  '/opt/exports/z3/amironenko/rootfs'

setenv bootfile     'uImage'

setenv kernelsz 300000
setenv kernelerasesz   360000

setenv fserasesz      cee0000

setenv ramdisksz      1000000
setenv ramdiskerasesz 1800000

setenv loadaddr 0x81800000
setenv loadaddr_ramdisk 0x82000000

setenv x **
setenv x ** User commands to set IP address static or DHCP
setenv x **
setenv set-ipstat 'setenv ipboot ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off'
setenv set-ipdhcp 'setenv ipboot ip=dhcp'
setenv set-ipnone 'setenv ipboot ip=none'
setenv set-ip 'run set-ipstat'

setenv bootargs_misc_ezsdk500 'mem=64M vram=8M ti816xfb.vram=8M earlyprintk libata.force=1.5G musb_hdrc.use_dma=0'

setenv bootargs_misc 'mem=176M vram=50M earlyprintk notifyk.vpssm3_sva=0xBF900000'

setenv console  ttyO0,115200,quiet


setenv nandrootfstype   ubifs
setenv nandroot_ubifs   'ubi.mtd=6 root=ubi0:rootfs'

setenv bootargs_nfs        'setenv bootargs console=${console} ${bootargs_misc} noinitrd root=/dev/nfs nfsroot=${nfsserver}:${nfs_root},nolock,udp rw rootdelay=4 ${ipboot}'
setenv bootargs_nand_ubifs 'setenv nandrootfstype ubifs;setenv bootargs console=${console} ${bootargs_misc} noinitrd ${nandroot_ubifs} rootfstype=${nandrootfstype} ${ipboot}'

setenv nand_boot_ubifs   'echo Booting from NAND and ubifs...; run setpower ip=dhcp bootargs_nand_ubifs; nand read.i ${loadaddr} 280000 ${kernelsz}; bootm ${loadaddr}'

setenv bootargs_nfs_misc 'mem=176M earlyprintk'
setenv bootargs_nfs_only setenv bootargs 'console=${console} noinitrd root=/dev/nfs nfsroot=${nfsserver}:${nfs_root},nolock rw ${bootargs_nfs_misc}: ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}'
setenv boot_nfs_only 'echo Boot from NAND and NFS...; run setpower bootargs_nfs_only; nfs ${loadaddr} ${serverip}:${nfs_root}/boot/${bootfile}; bootm ${loadaddr}'


setenv uboot_min_img 'u-boot.min.nand'
setenv uboot_img     'u-boot.bin'
setenv kernel_img    'uImage'
setenv ubi_img       'ubi.img' 

setenv update_dir 'boot/nandupdate'

setenv nand_loadaddr '0x81000000'
setenv uboot_min_nand_addr '0x0'
setenv uboot_min_size '0x80000'

setenv uboot_nand_addr '0x80000'
setenv uboot_size '0x140000'

setenv uboot_env_nand_addr '0x260000'
setenv uboot_env_size '0x20000'

setenv kernel_nand_addr '0x280000'
setenv kernel_size '0x440000'

setenv ubi_nand_addr '0x6c0000'
setenv ubi_size      '0xc820000' 

setenv nandupdate_load_uboot_min 'echo Loading uboot_min into SDRAM ...; mw.b ${nand_loadaddr} 0xFF ${uboot_min_size}; nfs ${nand_loadaddr} ${nfsserver}:${nfs_root}/${update_dir}/${uboot_min_img}'
setenv nandupdate_write_uboot_min 'echo Writing uboot_min into NAND ...; nand erase ${uboot_min_nand_addr} ${uboot_min_size}; nandecc hw 2; nand write.i ${nand_loadaddr} ${uboot_min_nand_addr} ${uboot_min_size}; nandecc hw 0'
setenv nandupdate_uboot_min 'echo Updating u-boot-min image ...; run nandupdate_load_uboot_min; run nandupdate_write_uboot_min'


setenv nandupdate_load_uboot  'echo Loading uboot_img into SDRAM ...; mw.b ${nand_loadaddr} 0xFF ${uboot_size}; nfs ${nand_loadaddr} ${nfsserver}:${nfs_root}/${update_dir}/${uboot_img}'
setenv nandupdate_write_uboot 'echo Writing uboot_img into NAND ...; nandecc hw 0; nand erase ${uboot_nand_addr} ${uboot_size}; nandecc hw 0; nand write.i ${nand_loadaddr} ${uboot_nand_addr} ${uboot_size}'
setenv nandupdate_uboot 'echo Updating u-boot image ...; run nandupdate_load_uboot; run nandupdate_write_uboot'

setenv nandupdate_load_uboot_env 'echo Loading ${uboot_env_img} into SDRAM ...; mw.b ${nand_loadaddr} 0x00 ${uboot_env_size}; nfs ${nand_loadaddr} ${nfsserver}:${nfs_root}/${update_dir}/${uboot_env_scr}'
setenv nandupdate_write_uboot_env 'echo Writing ${uboot_env_img} into NAND ...; source ${nand_loadaddr}; print; nandecc hw 0; saveenv;'
setenv nandupdate_uboot_env 'echo Updating u-boot environment ...; run nandupdate_load_uboot_env; run nandupdate_write_uboot_env' 


setenv nandupdate_load_kernel    'echo Loading ${kernel_img} into SDRAM ...; mw.b ${nand_loadaddr} 0xFF ${kernel_size}; nfs ${nand_loadaddr} ${nfsserver}:${nfs_root}/${update_dir}/${kernel_img}'
setenv nandupdate_write_kernel    'echo Writing ${kernel_img} into NAND ...; nandecc hw 0; nand erase ${kernel_nand_addr} ${kernel_size}; nand write.i ${nand_loadaddr} ${kernel_nand_addr} ${kernel_size}'
setenv nandupdate_kernel 'echo Updating Linux kernel image ...; run nandupdate_load_kernel; run nandupdate_write_kernel'


setenv nandupdate_load_ubi       'echo Loading ${ubi_img} into SDRAM ...; mw.b ${nand_loadaddr} 0xFF ${ubi_size}; nfs ${nand_loadaddr} ${nfsserver}:${nfs_root}/${update_dir}/${ubi_img}'
setenv nandupdate_write_ubi       'echo Writing ${ubi_img} into NAND ...; nandecc hw 0; nand erase ${ubi_nand_addr}  ${ubi_size}; nand write.i ${nand_loadaddr} ${ubi_nand_addr} ${ubi_size}'
setenv nandupdate_ubi  'echo Updating UBI root file system image ...; run nandupdate_load_ubi; run nandupdate_write_ubi'


setenv nandupdate_all 'echo All, u-boot, kernel and UBI file system images update from NFS ...; run nandupdate_uboot_min; run nandupdate_uboot; run nandupdate_uboot_env'


setenv x **
setenv x ** Adjust core voltage
setenv x **
setenv powerlevel ${defpowerlevel}

setenv setpower 'if test x"${powerlevel}" != x ; then setenv tmpsetting ${powerlevel} ; fi; if test x"${powerlevel}" = x ; then if test x"${defpowerlevel}" = x ; then echo Default powerlevel; setenv tmpsetting 0x49 ; else setenv tmpsetting ${defpowerlevel} ; fi ; fi ; echo Set powerlevel to ${tmpsetting} ; mw.w 1000000 0;mw.w 1000000 0;i2c mw 1d 2 0;i2c mw 1d 1 ${tmpsetting} ;sleep 1;i2c mw 1d 3 0;setenv tmpsetting'

setenv app02centaurus 'i2c dev 0;mw.w 1000000 a17b;mw.w 1000000 aaA5;i2c mw 4c 1b 1;mw.w 1000000 a873;mw.w 1000000 a47b'
setenv app02bruno 'i2c dev 0;mw.w 1000000 a27b;mw.w 1000000 a273;mw.w 1000000 a27b;mw.w 1000000 a17b'
setenv tvptristate    'i2c dev 0;i2c mw 5c 17 01;i2c mw 5d 17 01;i2c dev 1;i2c mw 5c 03 00;i2c mw 5d 03 00;i2c dev 0'

setenv x **
setenv x ** These command are run at boot time
setenv x **
setenv bootcmd 'run nand_boot_ubifs'




