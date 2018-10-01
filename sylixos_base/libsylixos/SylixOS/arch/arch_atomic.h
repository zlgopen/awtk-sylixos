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
** 文   件   名: arch_atomic.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2018 年 07 月 27 日
**
** 描        述: SylixOS 体系构架 ATOMIC 接口.
*********************************************************************************************************/

#ifndef __ARCH_ATOMIC_H
#define __ARCH_ATOMIC_H

#include "config/cpu/cpu_cfg.h"

#if defined(LW_CFG_CPU_ARCH_ARM)
#include "./arm/arm_atomic.h"

#elif defined(LW_CFG_CPU_ARCH_X86)
#include "./x86/x86_atomic.h"

#elif defined(LW_CFG_CPU_ARCH_MIPS)
#include "./mips/mips_atomic.h"

#elif defined(LW_CFG_CPU_ARCH_PPC)
#include "./ppc/ppc_atomic.h"

#elif defined(LW_CFG_CPU_ARCH_C6X)
#include "./c6x/c6x_atomic.h"

#elif defined(LW_CFG_CPU_ARCH_SPARC)
#include "./sparc/sparc_atomic.h"

#elif defined(LW_CFG_CPU_ARCH_RISCV)
#include "./riscv/riscv_atomic.h"
#endif                                                                  /*  LW_CFG_CPU_ARCH_ARM         */

#endif                                                                  /*  __ARCH_ATOMIC_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
