require ${OEBASE}/hanover-system/recipes/u-boot/u-boot-omap3-z3.inc

inherit srctree gitver

COMPATIBLE_MACHINE = "dm814x-z3"

PV = "${GITVER}"

UBOOT_MACHINE = "ti8148_evm_min_uart"

UBOOT_DEPLOY_DIR = "${DEPLOY_DIR_IMAGE}/nandupdate"

do_configure () {
   oe_runmake distclean
   oe_runmake ti8148_evm_min_uart 
}

do_compile_append () {
 oe_runmake u-boot.ti
}
 
do_deploy () {
 install -d ${UBOOT_DEPLOY_DIR}
 install  ${S}/u-boot.min.uart ${UBOOT_DEPLOY_DIR}
}
 
#addtask deploy before do_package_stage after do_compile 