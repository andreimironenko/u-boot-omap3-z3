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
#include <command.h>
#include <asm/byteorder.h>

extern int z3updater (int dryrun);
/******************************************************************************
 * Z3 updater command intepreter
 */
int do_z3updater(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int dryrun = 1;
	if (argc != 2) {
		cmd_usage(cmdtp);
		return 1;
	}

    if (argv[1][0] == '0') dryrun = 0;

    return z3updater(dryrun);
}

U_BOOT_CMD(
	z3updater,	2,	0,	do_z3updater,
	"Z3 SW updater",
	"<dry-run> - check to update from USB and/or continue\n"
    "            the updating initiated from Linux user space.\n"
    "dry-run: 1 - test without modifying flash\n"
    "         0 - actual update." 
);
#endif
