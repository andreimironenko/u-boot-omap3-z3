UBOOT_DIR = "${OEBASE}/hanover-system/recipes/u-boot"
require ${UBOOT_DIR}/u-boot-omap3-z3.inc

inherit srctree gitver

PV = ${GITVER}

COMPATIBLE_MACHINE = "dm814x-z3"

UBOOT_DEPLOY_DIR = "${DEPLOY_DIR_IMAGE}/nandupdate"

DEPENDS = "u-boot-min-uart-omap3-z3 u-boot-min-nand-omap3-z3"
RDEPENDS = "u-boot-min-uart-omap3-z3 u-boot-min-nand-omap3-z3"
PREFERRED_VERSION_u-boot-min-uart-omap3-z3 = "git"
PREFERRED_VERSION_u-boot-min-nand-omap3-z3 = "git"

do_install_append() {
 install -d ${D}${bindir}
}



do_deploy_append () {
 install -d ${UBOOT_DEPLOY_DIR}
 
 install -m 0775 ${S}/tools/mkimage                    ${UBOOT_DEPLOY_DIR}
 install -m 0775 ${S}/tools/mkenvimage                 ${UBOOT_DEPLOY_DIR}
 install -m 0775 ${UBOOT_DIR}/${MACHINE}/boot.cmd      ${UBOOT_DEPLOY_DIR}
 
 install -m 0755 ${S}/u-boot.bin                       ${UBOOT_DEPLOY_DIR} 
 
 ${S}/tools/mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n 'Execute uImage' -d ${UBOOT_DIR}/${MACHINE}/boot.cmd ${UBOOT_DEPLOY_DIR}/boot.scr
 
}