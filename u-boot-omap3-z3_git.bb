require ${OEBASE}/hanover-system/recipes/u-boot/u-boot-omap3-z3.inc

inherit srctree gitver

PV = ${GITVER}

COMPATIBLE_MACHINE = "dm814x-z3"

NANDUPDATE_DIR = "${DEPLOY_DIR_IMAGE}/nandupdate-${GITVER}"

DEPENDS = "u-boot-min-uart-omap3-z3 u-boot-min-nand-omap3-z3"
RDEPENDS = "u-boot-min-uart-omap3-z3 u-boot-min-nand-omap3-z3"
PREFERRED_VERSION_u-boot-min-uart-omap3-z3 = "git"
PREFERRED_VERSION_u-boot-min-nand-omap3-z3 = "git"

do_install_append() {
 install -d ${D}${bindir}
 #install -m 0755 ${S}/${MACHINE}/nandupdate.sh ${D}${bindir}
}



do_deploy_append () {
 install -d ${NANDUPDATE_DIR}
 
 install -m 0775 ${S}/tools/mkimage            ${NANDUPDATE_DIR}
 install -m 0775 ${S}/tools/mkenvimage         ${NANDUPDATE_DIR}
 install -m 0775 ${S}/${MACHINE}/nfsexport.sh  ${DEPLOY_DIR_IMAGE}
 install -m 0775 ${S}/${MACHINE}/boot.cmd      ${NANDUPDATE_DIR}
 install -m 0775 ${S}/${MACHINE}/ubinize.cfg   ${NANDUPDATE_DIR}
 
 install -m 0755 ${S}/${MACHINE}/nandupdate.sh ${NANDUPDATE_DIR}
 install -m 0755 ${S}/u-boot.bin               ${NANDUPDATE_DIR}
 
 ${S}/tools/mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n 'Execute uImage' -d ${S}/${MACHINE}/boot.cmd ${NANDUPDATE_DIR}/boot.scr
 
}