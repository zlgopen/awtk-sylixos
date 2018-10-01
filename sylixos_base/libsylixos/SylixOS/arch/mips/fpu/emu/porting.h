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
** 文   件   名: porting.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 11 月 24 日
**
** 描        述: MIPS FPU 模拟移植文件.
*********************************************************************************************************/

#ifndef __ARCH_MIPSFPUEMUPORTING_H
#define __ARCH_MIPSFPUEMUPORTING_H

#include "arch/mips/inc/porting.h"
#include "mipsFpuEmu.h"

/*********************************************************************************************************
  定义
*********************************************************************************************************/

#define IS_ENABLED(x)   (x)

#ifndef CONFIG_64BIT
#define CONFIG_64BIT    0
#else
#define CONFIG_32BIT    0
#endif

#define cp0_epc         REG_ulCP0EPC
#define regs            REG_ulReg

#define fpr             FPUCTX_reg
#define fcr31           FPUCTX_uiFcsr

/*********************************************************************************************************
  获得当前 CPU 当前线程的 FPU 寄存器
*********************************************************************************************************/

#define GET_CUR_THREAD_FPU_FPR          \
        _G_mipsFpuCtx[LW_CPU_GET_CUR_ID()].FPUCTX_reg

#define GET_CUR_THREAD_FPU_FCR31        \
        _G_mipsFpuCtx[LW_CPU_GET_CUR_ID()].FPUCTX_uiFcsr

/*********************************************************************************************************
  FPU REG 访问
*********************************************************************************************************/

#if BYTE_ORDER == BIG_ENDIAN
# define FPR_IDX(width, idx)    ((idx) ^ ((64 / (width)) - 1))
#else
# define FPR_IDX(width, idx)    (idx)
#endif

#define BUILD_FPR_ACCESS(width) \
static inline UINT##width get_fpr##width(union fpureg *fpr, unsigned idx)  \
{                                   \
    return fpr->val##width[FPR_IDX(width, idx)];            \
}                                   \
                                    \
static inline void set_fpr##width(union fpureg *fpr, unsigned idx,  \
                  UINT##width val)             \
{                                   \
    fpr->val##width[FPR_IDX(width, idx)] = val;         \
}

BUILD_FPR_ACCESS(32)
BUILD_FPR_ACCESS(64)

#endif                                                                  /*  __ARCH_MIPSFPUEMUPORTING_H  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
