/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: sgidefs.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 12 月 12 日
**
** 描        述: Silicon Graphics, Inc 定义头文件.
*********************************************************************************************************/
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 1999, 2001 Ralf Baechle
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Copyright (C) 2001 MIPS Technologies, Inc.
 */
#ifndef __ASM_SGIDEFS_H
#define __ASM_SGIDEFS_H

/*
 * Definitions for the ISA levels
 *
 * With the introduction of MIPS32 / MIPS64 instruction sets definitions
 * MIPS ISAs are no longer subsets of each other.  Therefore comparisons
 * on these symbols except with == may result in unexpected results and
 * are forbidden!
 */
#define _MIPS_ISA_MIPS1     1
#define _MIPS_ISA_MIPS2     2
#define _MIPS_ISA_MIPS3     3
#define _MIPS_ISA_MIPS4     4
#define _MIPS_ISA_MIPS5     5
#define _MIPS_ISA_MIPS32    6
#define _MIPS_ISA_MIPS64    7

/*
 * Subprogram calling convention
 */
#define _MIPS_SIM_ABI32     1
#define _MIPS_SIM_NABI32    2
#define _MIPS_SIM_ABI64     3

#endif                                                                  /*  __ASM_SGIDEFS_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
