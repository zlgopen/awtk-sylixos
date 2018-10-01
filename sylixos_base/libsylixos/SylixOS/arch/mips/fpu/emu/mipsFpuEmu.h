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
** 文   件   名: mipsFpuEmu.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 12 月 1 日
**
** 描        述: MIPS FPU 模拟头文件.
*********************************************************************************************************/

#ifndef __ARCH_MIPSFPUEMU_H
#define __ARCH_MIPSFPUEMU_H

#include "arch/mips/inc/config.h"
#include "arch/mips/inc/inst.h"

/*********************************************************************************************************
  FPU 模拟统计信息
*********************************************************************************************************/

#ifdef CONFIG_DEBUG_FPU_EMU

struct mips_fpu_emulator_stats {
    unsigned long emulated;
    unsigned long loads;
    unsigned long stores;
    unsigned long branches;
    unsigned long cp1ops;
    unsigned long cp1xops;
    unsigned long errors;
    unsigned long ieee754_inexact;
    unsigned long ieee754_underflow;
    unsigned long ieee754_overflow;
    unsigned long ieee754_zerodiv;
    unsigned long ieee754_invalidop;
    unsigned long ds_emul;

    unsigned long abs_s;
    unsigned long abs_d;
    unsigned long add_s;
    unsigned long add_d;
    unsigned long bc1eqz;
    unsigned long bc1nez;
    unsigned long ceil_w_s;
    unsigned long ceil_w_d;
    unsigned long ceil_l_s;
    unsigned long ceil_l_d;
    unsigned long class_s;
    unsigned long class_d;
    unsigned long cmp_af_s;
    unsigned long cmp_af_d;
    unsigned long cmp_eq_s;
    unsigned long cmp_eq_d;
    unsigned long cmp_le_s;
    unsigned long cmp_le_d;
    unsigned long cmp_lt_s;
    unsigned long cmp_lt_d;
    unsigned long cmp_ne_s;
    unsigned long cmp_ne_d;
    unsigned long cmp_or_s;
    unsigned long cmp_or_d;
    unsigned long cmp_ueq_s;
    unsigned long cmp_ueq_d;
    unsigned long cmp_ule_s;
    unsigned long cmp_ule_d;
    unsigned long cmp_ult_s;
    unsigned long cmp_ult_d;
    unsigned long cmp_un_s;
    unsigned long cmp_un_d;
    unsigned long cmp_une_s;
    unsigned long cmp_une_d;
    unsigned long cmp_saf_s;
    unsigned long cmp_saf_d;
    unsigned long cmp_seq_s;
    unsigned long cmp_seq_d;
    unsigned long cmp_sle_s;
    unsigned long cmp_sle_d;
    unsigned long cmp_slt_s;
    unsigned long cmp_slt_d;
    unsigned long cmp_sne_s;
    unsigned long cmp_sne_d;
    unsigned long cmp_sor_s;
    unsigned long cmp_sor_d;
    unsigned long cmp_sueq_s;
    unsigned long cmp_sueq_d;
    unsigned long cmp_sule_s;
    unsigned long cmp_sule_d;
    unsigned long cmp_sult_s;
    unsigned long cmp_sult_d;
    unsigned long cmp_sun_s;
    unsigned long cmp_sun_d;
    unsigned long cmp_sune_s;
    unsigned long cmp_sune_d;
    unsigned long cvt_d_l;
    unsigned long cvt_d_s;
    unsigned long cvt_d_w;
    unsigned long cvt_l_s;
    unsigned long cvt_l_d;
    unsigned long cvt_s_d;
    unsigned long cvt_s_l;
    unsigned long cvt_s_w;
    unsigned long cvt_w_s;
    unsigned long cvt_w_d;
    unsigned long div_s;
    unsigned long div_d;
    unsigned long floor_w_s;
    unsigned long floor_w_d;
    unsigned long floor_l_s;
    unsigned long floor_l_d;
    unsigned long maddf_s;
    unsigned long maddf_d;
    unsigned long max_s;
    unsigned long max_d;
    unsigned long maxa_s;
    unsigned long maxa_d;
    unsigned long min_s;
    unsigned long min_d;
    unsigned long mina_s;
    unsigned long mina_d;
    unsigned long mov_s;
    unsigned long mov_d;
    unsigned long msubf_s;
    unsigned long msubf_d;
    unsigned long mul_s;
    unsigned long mul_d;
    unsigned long neg_s;
    unsigned long neg_d;
    unsigned long recip_s;
    unsigned long recip_d;
    unsigned long rint_s;
    unsigned long rint_d;
    unsigned long round_w_s;
    unsigned long round_w_d;
    unsigned long round_l_s;
    unsigned long round_l_d;
    unsigned long rsqrt_s;
    unsigned long rsqrt_d;
    unsigned long sel_s;
    unsigned long sel_d;
    unsigned long seleqz_s;
    unsigned long seleqz_d;
    unsigned long selnez_s;
    unsigned long selnez_d;
    unsigned long sqrt_s;
    unsigned long sqrt_d;
    unsigned long sub_s;
    unsigned long sub_d;
    unsigned long trunc_w_s;
    unsigned long trunc_w_d;
    unsigned long trunc_l_s;
    unsigned long trunc_l_d;
};

extern struct mips_fpu_emulator_stats   _G_mipsFpuEmuStats[LW_CFG_MAX_PROCESSORS];

#define MIPS_FPU_EMU_INC_STATS(M)                   \
do {                                                \
    _G_mipsFpuEmuStats[LW_CPU_GET_CUR_ID()].M++;    \
} while (0)

#else
#define MIPS_FPU_EMU_INC_STATS(M)           do { } while (0)
#endif                                                                  /*  CONFIG_DEBUG_FPU_EMU        */

extern ARCH_FPU_CTX   _G_mipsFpuCtx[LW_CFG_MAX_PROCESSORS];

extern int     fpu_emulator_cop1Handler(ARCH_REG_CTX *xcp, ARCH_FPU_CTX *ctx,
                                        int has_fpu, void **fault_addr);
extern int     mips_dsemul(ARCH_REG_CTX *regs, mips_instruction ir,
                           unsigned long branch_pc, unsigned long cont_pc);
extern int     do_dsemulret(PLW_CLASS_TCB ptcbCur, ARCH_REG_CTX *xcp);

#endif                                                                  /*  __ARCH_MIPSFPUEMU_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
