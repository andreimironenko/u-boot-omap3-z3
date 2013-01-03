/******************************************************************************
** Copyright (c) 2011 Z3 Technology, LLC.  All rights reserved.  This file,
** including all design, code and information therein ("file"), is exclusively
** owned and controlled by Z3 Technology, LLC ("Z3") and may only be used by
** Z3's licensee in conjunction with Z3 devices or technologies.  Use with
** non-Z3 devices or technologies is expressly prohibited.  This file is
** included among the "Licensed Materials" in the licensee's license agreement
** with Z3 and is subject to all terms therein.  All derivative works are also
** the exclusive property of Z3 and subject to all the terms herein and all
** the terms of the license agreement with Z3.  Z3 is providing this file
** "as is" solely for Z3's licensee's use in developing programs and solutions
** for Z3 devices.  Z3 makes no representation or warranty that the file is
** non-infringing of the rights of third parties and expressly disclaims any
** warranty with respect to the adequacy of the implementation, including but
** not limited to any warranties of merchantability or warranty of fitness for
** a particular purpose.
**
** This copyright notice, restricted rights legend and other proprietary
** rights markings must be reproduced without modification in any copies of
** any part of this work and in any derivative work.  Removal or modification
** of any part of this notice is prohibited.
*******************************************************************************/

#include <common.h>
#ifndef CONFIG_TI814X_MIN_CONFIG
#include <asm/cache.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/arch/nand.h>
#include <linux/mtd/nand.h>
#include <nand.h>
#include <ata.h>
#include <part.h>
#include <fat.h>
#include <usb.h>
#include "z3_img_public.h"

#define Z3_UPDATE_IMAGE_PATH "/update/z3.img"
static int updater_dryrun;

#define DBG(fmt, arg...) do {if (updater_dryrun) printf(fmt, ##arg);} while(0)
static void print_img_header (master_hdr * hdr)
{
    int ii = 0;
    bin_hdr * bh;

    if (!updater_dryrun) return;

    DBG("image header:\n"
        "\tnum_of_bins          : %d\n"
        "\tignore_version_check : %d\n"
        "\tbindata_start_block  : %08x | %d\n"
        "\tdescription          : %s\n",
        hdr->num_of_bins, hdr->ignore_version_check,
        hdr->bindata_start_block, hdr->bindata_start_block,
        hdr->desc);

    for (ii = 0; ii < hdr->num_of_bins; ii++)
    {
        bh = &hdr->bh[ii];
        DBG("binary header <%d>:\n"
            "\tbytes            : %d\n"
            "\tin_pkg_offset    : %08x | %d\n"
            "\tload_address     : %08x | %d\n"
            "\tpartition_bytes  : %08x | %d\n"
            "\tchecksum         : %08x\n"
            "\tversion          : %d\n"
            "\tskip_bad_block   : %d\n", ii,
            bh->bytes, bh->in_pkg_offset, bh->in_pkg_offset,
            bh->load_address, bh->load_address,
            bh->partition_bytes, bh->partition_bytes,
            bh->checksum, bh->version, bh->skip_bad_block);
    }
}

#define ERR(fmt, arg...) printf(fmt, ##arg)
#define INFO(fmt, arg...) printf(fmt, ##arg)

typedef struct {
    int source_addr;
    int target_addr;
    int size;
    int ecc_switch;
    int erase_source;
} relocate_t;

#ifdef CONFIG_TI814X
//refer to platform specific Z3 upater definitions.
#define Z3_IMG_LOAD_ADDR      0x81000000
#define Z3_NAND_BLOCK_SIZE    0x20000
static relocate_t img_relocation_map[] = {
    {0x1C0000, 0, 0x40000, 1, 0},  //mini u-boot
    {0x1C0000, 0x40000, 0x40000, 1, 1},  //mini u-boot copy 2
    {0x240000, -1, 0x20000, 0, 1}, //u-boot env
    {-1, -1, -1, -1, -1}
};
#endif

#ifndef Z3_IMG_LOAD_ADDR
#error "you must define updater platform specific parameters"
#endif

#define _IMG_UPDATED_MAGIC 0x5aa5aa55
#define _IMG_UPDATED_MAGIC_LOCATION (Z3_IMG_LOAD_ADDR + 0x800488)

extern void ti81xx_nand_switch_ecc(nand_ecc_modes_t hardware, int32_t mode);

static int just_updated (void)
{
    int * magic = (int *)_IMG_UPDATED_MAGIC_LOCATION;
    return (*magic == _IMG_UPDATED_MAGIC)? 1 : 0;
}

static int is_image_present (void)
{
    int usb_stor_curr_dev = -1;
    block_dev_desc_t *dev_desc=NULL;
    long size;

    usb_stop();
    if (usb_init() >= 0)
    {
        usb_stor_curr_dev = usb_stor_scan(1);
    }
    if (usb_stor_curr_dev < 0)
    {
        DBG("USB device not present.\n");
        return 0;
    }

    dev_desc = get_dev("usb", usb_stor_curr_dev);
    if (dev_desc == NULL)
    {
        DBG("invalid device id: %d.\n", usb_stor_curr_dev);
        return 0;
    }

    if (fat_register_device(dev_desc, 1) !=0)
    {
		DBG("Unable to use device for fatload.\n");
		return 0;
	}

    icache_enable();

    printf("checking for Z3 software image ...\n");
    {
        master_hdr hdr;

        if (sizeof(master_hdr) !=
            file_fat_read(Z3_UPDATE_IMAGE_PATH, &hdr, sizeof(master_hdr)))
        {
            DBG("unable to find Z3 software image.\n");
            return 0;
        }
        if (hdr.magic1 != Z3_MAGIC_ID1 || hdr.magic2 != Z3_MAGIC_ID2
            || hdr.num_of_bins < 1)
        {
            ERR("unable to find valid software image.\n");
            DBG("invalid header magic ID: %x|%x %x|%x, bins: %d\n",
                hdr.magic1, Z3_MAGIC_ID1, hdr.magic2, Z3_MAGIC_ID2,
                hdr.num_of_bins);
            return 0;
        }
    }

    printf("reading software image from: %s ...\n", Z3_UPDATE_IMAGE_PATH);
    size = file_fat_read(Z3_UPDATE_IMAGE_PATH, (unsigned char *)Z3_IMG_LOAD_ADDR, 0);
    if (size <= 0)
    {
        DBG("unable to read file.\n");
        return 0;
    }

    return 1;
}

static unsigned int my_checksum (unsigned char * pc, int bytes)
{
	unsigned int checksum;

	checksum = 0;
	while (bytes--)
	{
		checksum = (checksum << 1) | (checksum >> 31);
		checksum ^= *pc++;
	}
	return checksum;
}

static int verify_img_checksum (master_hdr * hdr)
{
    void * buf;
    int ii;
    int ret = 0;
    unsigned int computed;
    bin_hdr * bh;
    unsigned int bin_offset = hdr->bindata_start_block * SDCARD_BLOCK_SIZE;

    INFO("verifying image checksum ...\n");
    for (ii = 0; ii < hdr->num_of_bins; ii++)
    {
        bh = &hdr->bh[ii];

        buf = (void *)Z3_IMG_LOAD_ADDR + bh->in_pkg_offset + bin_offset;
        if ((computed=my_checksum(buf, bh->bytes)) != bh->checksum)
        {
            ERR("binary %d checksum mismatch, broken package?\n"
                "\tin_pkg_offset     : %08x\n"
                "\tbytes             : %d\n"
                "\tcomputed checksum : %08x\n"
                "\tpackage checksum  : %08x\n",
                ii, bh->in_pkg_offset, bh->bytes, computed, bh->checksum);
            ret = -4; goto bail;
        }
    }

 bail:
    return ret;
}

static int verify_image (void)
{
    int ret = 0;
    master_hdr *hdr;

    hdr = (master_hdr *)Z3_IMG_LOAD_ADDR;
    /*
    if (hdr->magic1 != Z3_MAGIC_ID1 || hdr->magic2 != Z3_MAGIC_ID2
        || hdr->num_of_bins < 1)
	{
        ERR("invalid header magic ID: %x|%x %x|%x, bins: %d\n",
            hdr->magic1, Z3_MAGIC_ID1, hdr->magic2, Z3_MAGIC_ID2,
            hdr->num_of_bins);
        ret = -3; goto bail;
	} else
    */
    {
        print_img_header(hdr);
        if (verify_img_checksum(hdr))
        {
            ret = -4; goto bail;
        }
        INFO("firmware image detected: %s\n", hdr->desc);
    }

 bail:
    return ret;
}

static int nand_write_img (void * addr, int partition_bytes,
                           void * buf, int bytes, int dryrun)
{
	nand_info_t *nand;
    nand_erase_options_t opts;
    size_t size;

    size = ((bytes + Z3_NAND_BLOCK_SIZE - 1) / Z3_NAND_BLOCK_SIZE)
        * Z3_NAND_BLOCK_SIZE;

    if ((int)addr % Z3_NAND_BLOCK_SIZE)
    {
        ERR("ERROR: start address %p is not aligned to block boundary: %x\n",
            addr, Z3_NAND_BLOCK_SIZE);
        return -1;
    }

    nand = &nand_info[nand_curr_device];
    memset(&opts, 0, sizeof(opts));
    opts.offset = (int)addr;
    opts.length = ((partition_bytes + Z3_NAND_BLOCK_SIZE - 1)
                   / Z3_NAND_BLOCK_SIZE) * Z3_NAND_BLOCK_SIZE;
    opts.jffs2 = 0;
    opts.quiet = 1;

    DBG("erase and writing: %p %x\n", addr, size);
    if (!dryrun)
    {
        if (nand_erase_opts(nand, &opts))
        {
            ERR("ERROR: unable to erase: %p %x\n", addr, size);
            return -2;
        }

        if (nand_write_skip_bad(nand, (int)addr, &size, buf))
        {
            ERR("ERROR: unable to write flash: %p %x\n", addr, size);
            return -3;
        }
    }

    return 0;
}

static void set_magic (void)
{
    int * magic = (int *)_IMG_UPDATED_MAGIC_LOCATION;
    *magic = _IMG_UPDATED_MAGIC;
}

static int write_image (int dryrun)
{
    void * buf;
    int ii;
    int ret = 0;
    bin_hdr * bh;
    master_hdr * hdr =(master_hdr *)Z3_IMG_LOAD_ADDR;
    unsigned int bin_offset = hdr->bindata_start_block * SDCARD_BLOCK_SIZE;

    INFO("writing firmware, please wait ...\n");
    for (ii = 0; ii < hdr->num_of_bins; ii++)
    {
        bh = &hdr->bh[ii];
        buf = (void *)Z3_IMG_LOAD_ADDR + bh->in_pkg_offset + bin_offset;
        if (nand_write_img((void*)bh->load_address, bh->partition_bytes,
                           buf, bh->bytes, dryrun))
        {
            ERR("flash write error\n"
                "\taddress           : %08x\n"
                "\tpartition         : %08x\n"
                "\tbytes             : %d\n",
                bh->load_address, bh->partition_bytes, bh->bytes);
            ret = -4; goto bail;
        }
    }
    set_magic();

 bail:
    return ret;
}

static int phase2_update (int dryrun)
{
	nand_info_t *nand;
    nand_erase_options_t opts;
    size_t size;
    relocate_t * reloc = &img_relocation_map[0];
    void * buf = (void *)Z3_IMG_LOAD_ADDR;
    int reboot = 0;

    nand = &nand_info[nand_curr_device];
    while (reloc->source_addr != -1)
    {
        //read small chunk of data to check
        DBG("relocate [%p] checking: %x %x\n", reloc,
            reloc->target_addr, reloc->source_addr);
        size = 512;
        if (nand_read_skip_bad(nand, reloc->source_addr, &size, buf))
        {
            ERR("ERROR: unable to read: %x %x\n", reloc->source_addr, size);
            return -1;
        } else
        {
            int ii = 32;
            char *pc = (char *)buf;
            while (ii--) if (*pc++ != 0xff) goto update_continue;
            reloc++;
            continue;
        }

    update_continue:
        reboot = 1;
        size = reloc->size;
        if (nand_read_skip_bad(nand, reloc->source_addr, &size, buf))
        {
            ERR("ERROR: unable to read: %x %x\n", reloc->source_addr, size);
            return -1;
        } 

        DBG("relocate erase and writing: %x %x from: %x\n",
            reloc->target_addr, size, reloc->source_addr);
        if (!dryrun)
        {
            if (reloc->target_addr == -1)
            {
                char cmd[256];
                sprintf(cmd, "source 0x%x", Z3_IMG_LOAD_ADDR);
                run_command(cmd, 0);
            } else
            {
                memset(&opts, 0, sizeof(opts));
                opts.offset = reloc->target_addr;
                opts.length = size;
                opts.jffs2 = 0;
                opts.quiet = 1;
                if (nand_erase_opts(nand, &opts))
                {
                    ERR("ERROR: unable to erase target: %x %x\n",
                        reloc->target_addr, size);
                    return -2;
                }

                if (reloc->ecc_switch)
                {
                    ti81xx_nand_switch_ecc(NAND_ECC_HW, 2);
                }

                if (nand_write_skip_bad(nand, reloc->target_addr, &size, buf))
                {
                    ERR("ERROR: unable to write: %x %x\n",
                        reloc->target_addr, size);
                    return -3;
                }

                if (reloc->ecc_switch)
                {
                    ti81xx_nand_switch_ecc(NAND_ECC_SOFT, 0);
                }
            }

            if (reloc->erase_source)
            {
                memset(&opts, 0, sizeof(opts));
                opts.offset = reloc->source_addr;
                opts.length = size;
                opts.jffs2 = 0;
                opts.quiet = 1;
                if (nand_erase_opts(nand, &opts))
                {
                    ERR("ERROR: unable to erase source: %x %x\n",
                        reloc->source_addr, size);
                    return -4;
                }
            }
        }
        reloc++;
    }

    if (reboot)
    {
        INFO("firmware has been successfully updated.\n");
        set_magic();
        run_command("reset", 0);
    }

    return 0;
}

int z3updater (int dryrun)
{
    updater_dryrun = dryrun;
    DBG("updater entry point.\n");

    //a power recycle is required before next update request.
    if (just_updated())
    {
        DBG("just updated, skip.\n");
        goto check_phase2_update;
    }

    if (!is_image_present())
    {
        DBG("no update image detected.\n");
        goto check_phase2_update;
    }
    if (verify_image())
    {
        DBG("image verification failed.\n");
        goto check_phase2_update;
    }

    if (write_image(dryrun))
    {
        DBG("unable to write image.\n");
    }

check_phase2_update:
    if (phase2_update(dryrun)) return -3;

    return 0;
}
#endif
