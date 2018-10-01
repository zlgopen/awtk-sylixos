;/*********************************************************************************************************
;**
;**                                    中国软件开源组织
;**
;**                                   嵌入式实时操作系统
;**
;**                                       SylixOS(TM)
;**
;**                               Copyright  All Rights Reserved
;**
;**--------------文件信息--------------------------------------------------------------------------------
;**
;** 文   件   名: riscvContextAsm.h
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2018 年 03 月 20 日
;**
;** 描        述: RISC-V 体系构架上下文切换.
;*********************************************************************************************************/

#ifndef __ARCH_RISCV_CTX_ASM_H
#define __ARCH_RISCV_CTX_ASM_H

#include "arch/riscv/arch_regs.h"

;/*********************************************************************************************************
;  保存寄存器(参数 T0: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

MACRO_DEF(SAVE_SMALL_REG_CTX)
    LI      T1  , 1
    REG_S   T1  , CTXTYPE_OFFSET(T0)                                     /*  小上下文类型                */

    REG_S   S0  , S0_OFFSET(T0)
    REG_S   S1  , S1_OFFSET(T0)
    REG_S   S2  , S2_OFFSET(T0)
    REG_S   S3  , S3_OFFSET(T0)
    REG_S   S4  , S4_OFFSET(T0)
    REG_S   S5  , S5_OFFSET(T0)
    REG_S   S6  , S6_OFFSET(T0)
    REG_S   S7  , S7_OFFSET(T0)
    REG_S   S8  , S8_OFFSET(T0)
    REG_S   S9  , S9_OFFSET(T0)
    REG_S   S10 , S10_OFFSET(T0)
    REG_S   S11 , S11_OFFSET(T0)
    REG_S   SP  , SP_OFFSET(T0)
    REG_S   GP  , GP_OFFSET(T0)
    REG_S   TP  , TP_OFFSET(T0)

    REG_S   RA  , RA_OFFSET(T0)
    REG_S   RA  , EPC_OFFSET(T0)

    CSRR    T1  , XSTATUS
    LI      T2  , ~XSTATUS_XPIE
    AND     T1  , T1 , T2
    LI      T2  , XSTATUS_XPP
    OR      T1  , T1 , T2
    REG_S   T1  , STATUS_OFFSET(T0)
    MACRO_END()

;/*********************************************************************************************************
;  恢复大上下文寄存器(参数 T0: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

MACRO_DEF(RESTORE_BIG_REG_CTX)
    REG_L   RA  , RA_OFFSET(T0)
    REG_L   SP  , SP_OFFSET(T0)
    REG_L   GP  , GP_OFFSET(T0)
    REG_L   TP  , TP_OFFSET(T0)
    REG_L   S0  , S0_OFFSET(T0)
    REG_L   S1  , S1_OFFSET(T0)
    REG_L   A0  , A0_OFFSET(T0)
    REG_L   A1  , A1_OFFSET(T0)
    REG_L   A2  , A2_OFFSET(T0)
    REG_L   A3  , A3_OFFSET(T0)
    REG_L   A4  , A4_OFFSET(T0)
    REG_L   A5  , A5_OFFSET(T0)
    REG_L   A6  , A6_OFFSET(T0)
    REG_L   A7  , A7_OFFSET(T0)
    REG_L   S2  , S2_OFFSET(T0)
    REG_L   S3  , S3_OFFSET(T0)
    REG_L   S4  , S4_OFFSET(T0)
    REG_L   S5  , S5_OFFSET(T0)
    REG_L   S6  , S6_OFFSET(T0)
    REG_L   S7  , S7_OFFSET(T0)
    REG_L   S8  , S8_OFFSET(T0)
    REG_L   S9  , S9_OFFSET(T0)
    REG_L   S10 , S10_OFFSET(T0)
    REG_L   S11 , S11_OFFSET(T0)
    REG_L   T2  , T2_OFFSET(T0)
    REG_L   T3  , T3_OFFSET(T0)
    REG_L   T4  , T4_OFFSET(T0)
    REG_L   T5  , T5_OFFSET(T0)
    REG_L   T6  , T6_OFFSET(T0)

    REG_L   T1  , EPC_OFFSET(T0)
    CSRW    XEPC, T1

    REG_L   T1  , STATUS_OFFSET(T0)
    CSRW    XSTATUS , T1

    REG_L   T1  , T1_OFFSET(T0)
    REG_L   T0  , T0_OFFSET(T0)                                         /*  最后恢复寄存器 T1 T0         */

    XRET
    MACRO_END()

;/*********************************************************************************************************
;  恢复小上下文寄存器(参数 T0: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

MACRO_DEF(RESTORE_SMALL_REG_CTX)
    REG_L   A0  , A0_OFFSET(T0)                                         /*  恢复参数寄存器 A0            */

    REG_L   S0  , S0_OFFSET(T0)
    REG_L   S1  , S1_OFFSET(T0)
    REG_L   S2  , S2_OFFSET(T0)
    REG_L   S3  , S3_OFFSET(T0)
    REG_L   S4  , S4_OFFSET(T0)
    REG_L   S5  , S5_OFFSET(T0)
    REG_L   S6  , S6_OFFSET(T0)
    REG_L   S7  , S7_OFFSET(T0)
    REG_L   S8  , S8_OFFSET(T0)
    REG_L   S9  , S9_OFFSET(T0)
    REG_L   S10 , S10_OFFSET(T0)
    REG_L   S11 , S11_OFFSET(T0)
    REG_L   SP  , SP_OFFSET(T0)
    REG_L   GP  , GP_OFFSET(T0)
    REG_L   TP  , TP_OFFSET(T0)

    REG_L   RA  , EPC_OFFSET(T0)
    CSRW    XEPC, RA

    REG_L   T1  , STATUS_OFFSET(T0)
    CSRW    XSTATUS , T1

    XRET
    MACRO_END()

/*********************************************************************************************************
  使用异常临时栈, 并在异常临时栈开辟临时上下文保存区, 将 volatile 寄存器保存到临时上下文保存区
*********************************************************************************************************/

MACRO_DEF(EXC_SAVE_VOLATILE)
    CSRRW   SP  , XSCRATCH , SP

    REG_S   RA  , RA_OFFSET(SP)
    REG_S   GP  , GP_OFFSET(SP)
    REG_S   TP  , TP_OFFSET(SP)
    REG_S   T0  , T0_OFFSET(SP)
    REG_S   T1  , T1_OFFSET(SP)
    REG_S   T2  , T2_OFFSET(SP)
    REG_S   A0  , A0_OFFSET(SP)
    REG_S   A1  , A1_OFFSET(SP)
    REG_S   A2  , A2_OFFSET(SP)
    REG_S   A3  , A3_OFFSET(SP)
    REG_S   A4  , A4_OFFSET(SP)
    REG_S   A5  , A5_OFFSET(SP)
    REG_S   A6  , A6_OFFSET(SP)
    REG_S   A7  , A7_OFFSET(SP)
    REG_S   T3  , T3_OFFSET(SP)
    REG_S   T4  , T4_OFFSET(SP)
    REG_S   T5  , T5_OFFSET(SP)
    REG_S   T6  , T6_OFFSET(SP)

    CSRR    T0  , XSCRATCH
    REG_S   T0  , SP_OFFSET(SP)
    CSRW    XSCRATCH , SP
    MACRO_END()

/*********************************************************************************************************
  拷贝 volatile 寄存器(参数 RV0: 目的 ARCH_REG_CTX 地址, 参数 SP: 源 ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(EXC_COPY_VOLATILE)
    REG_L   T0  , RA_OFFSET(SP)
    REG_S   T0  , RA_OFFSET(RV0)

    REG_L   T0  , GP_OFFSET(SP)
    REG_S   T0  , GP_OFFSET(RV0)

    REG_L   T0  , TP_OFFSET(SP)
    REG_S   T0  , TP_OFFSET(RV0)

    REG_L   T0  , T0_OFFSET(SP)
    REG_S   T0  , T0_OFFSET(RV0)

    REG_L   T0  , T1_OFFSET(SP)
    REG_S   T0  , T1_OFFSET(RV0)

    REG_L   T0  , T2_OFFSET(SP)
    REG_S   T0  , T2_OFFSET(RV0)

    REG_L   T0  , A0_OFFSET(SP)
    REG_S   T0  , A0_OFFSET(RV0)

    REG_L   T0  , A1_OFFSET(SP)
    REG_S   T0  , A1_OFFSET(RV0)

    REG_L   T0  , A2_OFFSET(SP)
    REG_S   T0  , A2_OFFSET(RV0)

    REG_L   T0  , A3_OFFSET(SP)
    REG_S   T0  , A3_OFFSET(RV0)

    REG_L   T0  , A4_OFFSET(SP)
    REG_S   T0  , A4_OFFSET(RV0)

    REG_L   T0  , A5_OFFSET(SP)
    REG_S   T0  , A5_OFFSET(RV0)

    REG_L   T0  , A6_OFFSET(SP)
    REG_S   T0  , A6_OFFSET(RV0)

    REG_L   T0  , A7_OFFSET(SP)
    REG_S   T0  , A7_OFFSET(RV0)

    REG_L   T0  , T3_OFFSET(SP)
    REG_S   T0  , T3_OFFSET(RV0)

    REG_L   T0  , T4_OFFSET(SP)
    REG_S   T0  , T4_OFFSET(RV0)

    REG_L   T0  , T5_OFFSET(SP)
    REG_S   T0  , T5_OFFSET(RV0)

    REG_L   T0  , T6_OFFSET(SP)
    REG_S   T0  , T6_OFFSET(RV0)

    REG_L   T0  , SP_OFFSET(SP)
    REG_S   T0  , SP_OFFSET(RV0)
    MACRO_END()

/*********************************************************************************************************
  保存 non volatile 寄存器(参数 RV0: ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(EXC_SAVE_NON_VOLATILE)
    REG_S   ZERO, CTXTYPE_OFFSET(RV0)
    REG_S   S0  , S0_OFFSET(RV0)
    REG_S   S1  , S1_OFFSET(RV0)
    REG_S   S2  , S2_OFFSET(RV0)
    REG_S   S3  , S3_OFFSET(RV0)
    REG_S   S4  , S4_OFFSET(RV0)
    REG_S   S5  , S5_OFFSET(RV0)
    REG_S   S6  , S6_OFFSET(RV0)
    REG_S   S7  , S7_OFFSET(RV0)
    REG_S   S8  , S8_OFFSET(RV0)
    REG_S   S9  , S9_OFFSET(RV0)
    REG_S   S10 , S10_OFFSET(RV0)
    REG_S   S11 , S11_OFFSET(RV0)

    CSRR    T0  , XEPC
    REG_S   T0  , EPC_OFFSET(RV0)

    CSRR    T0  , XSTATUS
    REG_S   T0  , STATUS_OFFSET(RV0)

    CSRR    T0  , XTVAL
    REG_S   T0  , TVAL_OFFSET(RV0)

    CSRR    T0  , XCAUSE
    REG_S   T0  , CAUSE_OFFSET(RV0)
    MACRO_END()

#endif                                                                  /*  __ARCH_RISCV_CTX_ASM_H       */
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
