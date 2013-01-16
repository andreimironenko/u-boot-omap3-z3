require ${OEBASE}/hanover-system/recipes/u-boot/u-boot-omap3-z3.inc

inherit srctree gitver

PV = ${GITVER}

COMPATIBLE_MACHINE = "dm814x-z3"

DEPENDS = "u-boot-min-uart-omap3-z3 u-boot-min-nand-omap3-z3"
RDEPENDS = "u-boot-min-uart-omap3-z3 u-boot-min-nand-omap3-z3"
PREFERRED_VERSION_u-boot-min-uart-omap3-z3 = "git"
PREFERRED_VERSION_u-boot-min-nand-omap3-z3 = "git"

do_install_append() {
 install -d ${D}${bindir}
 #install -m 0755 ${S}/${MACHINE}/nandupdate.sh ${D}${bindir}
}

do_deploy_append () {
 install -d ${DEPLOY_DIR_IMAGE}/nandupdate
 
 install -m 0775 ${S}/tools/mkimage            ${DEPLOY_DIR_IMAGE}
 install -m 0775 ${S}/tools/mkenvimage         ${DEPLOY_DIR_IMAGE}
 install -m 0775 ${S}/${MACHINE}/nfsexport.sh  ${DEPLOY_DIR_IMAGE}
 install -m 0775 ${S}/${MACHINE}/boot.cmd      ${DEPLOY_DIR_IMAGE}
 install -m 0775 ${S}/${MACHINE}/ubinize.cfg   ${DEPLOY_DIR_IMAGE}
 
 install -m 0755 ${S}/${MACHINE}/nandupdate.sh ${DEPLOY_DIR_IMAGE}/nandupdate
 install -m 0755 ${S}/u-boot.bin    ${DEPLOY_DIR_IMAGE}/nandupdate/u-boot-${GITVER}.bin
 
}