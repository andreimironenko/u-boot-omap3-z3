require u-boot-omap3-z3.inc

COMPATIBLE_MACHINE = "dm814x-z3"

PV = "${GITVER}"

UBOOT_MACHINE = "ti8148_evm_min_uart"

do_compile_append () {
 oe_runmake u-boot.ti
}
 
do_deploy () {
 install -d ${DEPLOY_DIR_IMAGE}/nandupdate
 install  ${S}/u-boot.min.uart ${DEPLOY_DIR_IMAGE}/nandupdate
}
 
addtask deploy before do_package_stage after do_compile 