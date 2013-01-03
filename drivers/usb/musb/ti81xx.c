/*
 * Texas Instruments TI81XX "glue layer"
 *
 * Copyright (c) 2008, MontaVista Software, Inc. <source@mvista.com>
 *
 * Based on the DaVinci "glue layer" code.
 * Copyright (C) 2005-2006 by Texas Instruments
 *
 * This file is part of the Inventra Controller Driver for Linux.
 *
 * The Inventra Controller Driver for Linux is free software; you
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 2 as published by the Free Software
 * Foundation.
 *
 * The Inventra Controller Driver for Linux is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Inventra Controller Driver for Linux ; if not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "ti81xx.h"

static int platform_needs_initialization = 1;

#define TI81XX_USB_TIMEOUT  0x3FFFFFF

#define L3_TI81XX_BASE		0x44000000
#define L4_FAST_TI81XX_BASE	0x4a000000
#define L4_SLOW_TI81XX_BASE	0x48000000

#define TI81XX_SCM_BASE		0x48140000
#define TI81XX_CTRL_BASE	TI81XX_SCM_BASE
#define TI81XX_PRCM_BASE	0x48180000
#define TI814X_PLL_BASE		0x481C5000

#define TI81XX_ARM_INTC_BASE	0x48200000

#define TI81XX_GPMC_BASE	0x50000000
#define TI816X_PCIE_REG_BASE    0x51000000
#define TI816X_PCIE_MEM_BASE    0x20000000
#define TI816X_PCIE_IO_BASE     0x40000000	/* Using 3MB reserved space */

#define TI81XX_USBSS_BASE	0x47400000
#define TI81XX_USBSS_LEN	0xFFF
#define TI81XX_USB0_BASE	0x47401000
#define TI81XX_USB1_BASE	0x47401800
#define TI81XX_USB_CPPIDMA_BASE	0x47402000
#define TI81XX_USB_CPPIDMA_LEN	0x5FFF

#define TI816X_GPIO0_BASE	0x48032000
#define TI816X_GPIO1_BASE	0x4804C000

/* TI81XX specific definitions */
#define TI81XX_USBCTRL0				0x0620
#define TI81XX_USBSTAT0				0x0624
#define TI81XX_USBCTRL1				0x0628
#define TI81XX_USBSTAT1				0x062c

/* TI816X PHY controls bits */
#define	TI816X_USBPHY0_NORMAL_MODE		(1 << 0)
#define	TI816X_USBPHY1_NORMAL_MODE		(1 << 1)
#define	TI816X_USBPHY_REFCLK_OSC		(1 << 8)

/*
 * TI816X CM module offsets
 */

#define TI816X_CM_ACTIVE_MOD			0x0400	/* 256B */
#define TI816X_CM_DEFAULT_MOD			0x0500	/* 256B */
#define TI816X_CM_IVAHD0_MOD			0x0600	/* 256B */
#define TI816X_CM_IVAHD1_MOD			0x0700	/* 256B */
#define TI816X_CM_IVAHD2_MOD			0x0800	/* 256B */
#define TI816X_CM_SGX_MOD			0x0900	/* 256B */

/* Modulemode bit */
#define TI81XX_MODULEMODE_SWCTRL			1

#define TI81XX_PRM_DEFAULT_MOD			0x0b00	/* 256B */

/* TI81XX specific register offsets from each PRM module base */
#define TI81XX_PM_PWSTCTRL				0x0000
#define TI81XX_PM_PWSTST				0x0004
#define TI81XX_RM_RSTCTRL				0x0010
#define TI81XX_RM_RSTST					0x0014

/* Used by RM_DEFAULT_RSTCTRL */
#define TI81XX_USB1_LRST_SHIFT				5
#define TI81XX_USB1_LRST_MASK				(1 << 5)

/* Used by RM_DEFAULT_RSTCTRL */
#define TI81XX_USB2_LRST_SHIFT				6
#define TI81XX_USB2_LRST_MASK				(1 << 6)

/* Used by RM_DEFAULT_RSTCTRL */
#define TI81XX_PCI_LRST_SHIFT				7
#define TI81XX_PCI_LRST_MASK				(1 << 7)

/* IDLEST bit */
#define TI81XX_IDLEST_SHIFT				16
#define TI81XX_IDLEST_MASK				(3 << 16)
#define TI81XX_IDLEST_VAL				3

#define MAX_MODULE_ENABLE_WAIT		100000

struct musb_config musb_cfg = {
	(struct	musb_regs *)(TI81XX_USB0_BASE + USB_MENTOR_CORE_OFFSET),
	TI81XX_USB_TIMEOUT,
	0,
	0
};

static inline u32 usbctl_read(u32 offset)
{
	return readl(TI81XX_USB0_BASE + offset);
}

static inline void usbctl_write(u32 offset, u32 data)
{
	writel(data, TI81XX_USB0_BASE + offset);
}

static inline u32 usbss_read(u32 offset)
{
	return readl(TI81XX_USBSS_BASE + offset);
}

static inline void usbss_write(u32 offset, u32 data)
{
	writel(data, TI81XX_USBSS_BASE + offset);
}

static void usbotg_ss_init(void)
{
    /* clear any USBSS interrupts */
    usbss_write(USBSS_IRQ_EOI, 0);
    usbss_write(USBSS_IRQ_STATUS, usbss_read(USBSS_IRQ_STATUS));
}

static int ti816x_usb_clk_enable(void)
{
    // refer to ti816x manual

    // *0x48180B10 &= 0xFFFFFF9F; // RSTCTRL (Release USB Module from Reset)
    writel(readl(0x48180B10) & 0xFFFFFF9F, 0x48180B10);

    //while ((0x48180B14 & 0x00000060)>>5)!=0x3); // Wait until Modules come out Reset
    while(((readl(0x48180B14) & 0x00000060)>>5) != 0x3)
    {
        printf(".");
    }
    printf("\n");

    //*0x48180B14 |= 0x00000060; // RSTST (Clear Reset Presence)
    writel(readl(0x48180B14) | 0x60, 0x48180B14);

    //*0x48180514 = 0x2; // L3_SLOW_CLKSTCTRL (Enable Interconnect Clock)
    writel(0x2, 0x48180514);

    //*0x48180558 = 0x2; // USB_CLKCTRL (Enable USB OCP Clock)
    writel(0x02, 0x48180558);

    //while(((*0x48180558) & 0x70000)>>16)==0x7); // Wait until USB Module is Ready
    while(((readl(0x48180558) & 0x70000)>>16)!=0x0); 

    // Configure Access Capability to be in Supervisory Mode
    //*0x48140620 =0x0000_0003 //USB_CTRL Use PLL Ref Clock, Wake USB PHY1/00
    writel(0x03, 0x48140620);

#if 0 //porting from kernel source
    printf ("enabling usb clock.\n");
    {
        u32 clk;

        clk = readl(TI81XX_PRCM_BASE + TI816X_CM_DEFAULT_MOD + 0x0058);
        clk |= (1 << TI81XX_MODULEMODE_SWCTRL);
        writel(clk, TI81XX_PRCM_BASE + TI816X_CM_DEFAULT_MOD + 0x0058);
        printf("usb clock control: %x | %x\n", clk,
               readl(TI81XX_PRCM_BASE + TI816X_CM_DEFAULT_MOD + 0x0058));

        
        clk = readl(TI81XX_PRCM_BASE + TI81XX_PRM_DEFAULT_MOD + TI81XX_RM_RSTCTRL);
        printf("usb clock reset: %x\n", clk);
        clk &= ~(TI81XX_USB1_LRST_MASK | TI81XX_USB2_LRST_MASK);
        clk |= (TI81XX_USB1_LRST_MASK | TI81XX_USB2_LRST_MASK);
        writel(clk, TI81XX_PRCM_BASE + TI81XX_PRM_DEFAULT_MOD + TI81XX_RM_RSTCTRL);
 
        {
            int i = 0;
            int ena = 0;

            /* Wait for lock */
            while (1)
            {
                if ((readl(TI81XX_PRCM_BASE + TI81XX_PRM_DEFAULT_MOD + TI81XX_RM_RSTCTRL)
                     & TI81XX_IDLEST_MASK) == 0) break;
                else if (i++ > MAX_MODULE_ENABLE_WAIT) break;
                printf(".");
            }
            printf("\n");
            if (i < MAX_MODULE_ENABLE_WAIT)
                printf("usb clock is now ready.\n");
            else
                printf("unable to start usb clock.\n");
        }
    }
    #if 0
	omap2_dflt_clk_enable(clk);

	/* De-assert local reset after module enable */
	omap2_prm_clear_mod_reg_bits(TI81XX_USB1_LRST_MASK
			| TI81XX_USB2_LRST_MASK,
			TI81XX_PRM_DEFAULT_MOD,
			TI81XX_RM_RSTCTRL);

	omap2_cm_wait_idlest(clk->enable_reg, TI81XX_IDLEST_MASK,
			TI81XX_IDLEST_VAL, clk->name);
    #endif
#endif

	return 0;
}


static int ti814x_usb_clk_enable(void)
{
    // refer to ti816x manual

    // *0x48180B10 &= 0xFFFFFF9F; // RSTCTRL (Release USB Module from Reset)
    writel(readl(0x48180B10) & 0xFFFFFF9F, 0x48180B10);

    //while ((0x48180B14 & 0x00000060)>>5)!=0x3); // Wait until Modules come out Reset
    while(((readl(0x48180B14) & 0x00000060)>>5) != 0x3)
    {
        printf(".");
    }
    printf("\n");

    //*0x48180B14 |= 0x00000060; // RSTST (Clear Reset Presence)
    writel(readl(0x48180B14) | 0x60, 0x48180B14);

    //*0x48180514 = 0x2; // L3_SLOW_CLKSTCTRL (Enable Interconnect Clock)
    writel(0x2, 0x48180514);

    //*0x48180558 = 0x2; // USB_CLKCTRL (Enable USB OCP Clock)
    writel(0x02, 0x48180558);

    //while(((*0x48180558) & 0x70000)>>16)==0x7); // Wait until USB Module is Ready
    while(((readl(0x48180558) & 0x70000)>>16)!=0x0); 

    // Configure Access Capability to be in Supervisory Mode
    //*0x48140620 =0x0000_0003 //USB_CTRL Use PLL Ref Clock, Wake USB PHY1/00
//    writel(0x03, 0x48140620);
    writel((readl(0x48140620)&~(0x00818303)), 0x48140620);
    writel((readl(0x48140620)|(0x001E64C0)), 0x48140620);

#if 0 //porting from kernel source
    printf ("enabling usb clock.\n");
    {
        u32 clk;

        clk = readl(TI81XX_PRCM_BASE + TI816X_CM_DEFAULT_MOD + 0x0058);
        clk |= (1 << TI81XX_MODULEMODE_SWCTRL);
        writel(clk, TI81XX_PRCM_BASE + TI816X_CM_DEFAULT_MOD + 0x0058);
        printf("usb clock control: %x | %x\n", clk,
               readl(TI81XX_PRCM_BASE + TI816X_CM_DEFAULT_MOD + 0x0058));

        
        clk = readl(TI81XX_PRCM_BASE + TI81XX_PRM_DEFAULT_MOD + TI81XX_RM_RSTCTRL);
        printf("usb clock reset: %x\n", clk);
        clk &= ~(TI81XX_USB1_LRST_MASK | TI81XX_USB2_LRST_MASK);
        clk |= (TI81XX_USB1_LRST_MASK | TI81XX_USB2_LRST_MASK);
        writel(clk, TI81XX_PRCM_BASE + TI81XX_PRM_DEFAULT_MOD + TI81XX_RM_RSTCTRL);
 
        {
            int i = 0;
            int ena = 0;

            /* Wait for lock */
            while (1)
            {
                if ((readl(TI81XX_PRCM_BASE + TI81XX_PRM_DEFAULT_MOD + TI81XX_RM_RSTCTRL)
                     & TI81XX_IDLEST_MASK) == 0) break;
                else if (i++ > MAX_MODULE_ENABLE_WAIT) break;
                printf(".");
            }
            printf("\n");
            if (i < MAX_MODULE_ENABLE_WAIT)
                printf("usb clock is now ready.\n");
            else
                printf("unable to start usb clock.\n");
        }
    }
    #if 0
	omap2_dflt_clk_enable(clk);

	/* De-assert local reset after module enable */
	omap2_prm_clear_mod_reg_bits(TI81XX_USB1_LRST_MASK
			| TI81XX_USB2_LRST_MASK,
			TI81XX_PRM_DEFAULT_MOD,
			TI81XX_RM_RSTCTRL);

	omap2_cm_wait_idlest(clk->enable_reg, TI81XX_IDLEST_MASK,
			TI81XX_IDLEST_VAL, clk->name);
    #endif
#endif

	return 0;
}

void ti81xx_usb_clk_enable(void)
{
#if defined( CONFIG_USB_TI816X ) 
    ti816x_usb_clk_enable();
#elif defined( CONFIG_USB_TI814X ) 
    ti814x_usb_clk_enable();
#else
#error "Unknown processor"
#endif
}

int musb_platform_init(void)
{
	int ret = -1;
    u32 rev;
    u32 regval;
    u32 usbphycfg;

	if (platform_needs_initialization) {

        ti81xx_usb_clk_enable();

        /* usb subsystem init */
        usbotg_ss_init();

        rev = usbctl_read(USB_REVISION_REG);
        if (!rev)
        {
            ret = 0;
            printf("ERROR: OTG_REVISION not found!\n");
            goto bail;
        } else {
            //printf("OTG_REVISION 0x%x\n", rev);
        }

        /* Reset the controller */
        usbctl_write(USB_CTRL_REG, USB_SOFT_RESET_MASK);

        /* wait till reset bit clears */
        while ((usbctl_read(USB_CTRL_REG) & 0x1))
        {
            printf("wait: %x\n", usbctl_read(USB_CTRL_REG));
        }

#if defined( CONFIG_USB_TI816X ) 
        //turn on phy power
        usbphycfg = readl(TI81XX_CTRL_BASE + TI81XX_USBCTRL0);
        usbphycfg |= (TI816X_USBPHY0_NORMAL_MODE
                      | TI816X_USBPHY1_NORMAL_MODE);
        usbphycfg &= ~(TI816X_USBPHY_REFCLK_OSC);
        writel(usbphycfg, TI81XX_CTRL_BASE + TI81XX_USBCTRL0);
        {
            int ii = 80;
            while (ii--) printf(".");
            printf("\n");
        }
#elif defined( CONFIG_USB_TI814X ) 
        /* linux driver does nothing here */
#else
#error "Unknown processor"
#endif
        //set host mode
        regval = usbctl_read(USB_MODE_REG);
		regval &= ~USBMODE_USBID_HIGH;
        regval |= USBMODE_USBID_MUXSEL;

		usbctl_write(USB_MODE_REG, regval);
		usbctl_write (USB_PHY_UTMI_REG, 0x02);
		//printf("host: value of mode reg=%x regval(%x)\n",
        //       usbctl_read(USB_MODE_REG), regval);


        usbctl_write(USB_IRQ_EOI, 0);
        usbss_write(USBSS_IRQ_EOI, 0);

		platform_needs_initialization = 0;
	}
	ret = platform_needs_initialization;

bail:
	return ret;
}

void musb_platform_deinit(void)
{
	/* noop */
}
