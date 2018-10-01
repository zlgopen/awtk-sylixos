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
;** 文   件   名: mipsContextAsm.h
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2015 年 09 月 01 日
;**
;** 描        述: MIPS 体系架构上下文处理.
;*********************************************************************************************************/

#ifndef __ARCH_MIPSCONTEXTASM_H
#define __ARCH_MIPSCONTEXTASM_H

#include "arch/mips/arch_regs.h"

;/*********************************************************************************************************
;  保存 $0 - $31 寄存器(参数 T9: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

MACRO_DEF(SAVE_GREGS)
    .set    push
    .set    noat

    REG_S       $0  , XGREG(0)(T9)
    REG_S       $1  , XGREG(1)(T9)
    REG_S       $2  , XGREG(2)(T9)
    REG_S       $3  , XGREG(3)(T9)
    REG_S       $4  , XGREG(4)(T9)
    REG_S       $5  , XGREG(5)(T9)
    REG_S       $6  , XGREG(6)(T9)
    REG_S       $7  , XGREG(7)(T9)
    REG_S       $8  , XGREG(8)(T9)
    REG_S       $9  , XGREG(9)(T9)
    REG_S       $10 , XGREG(10)(T9)
    REG_S       $11 , XGREG(11)(T9)
    REG_S       $12 , XGREG(12)(T9)
    REG_S       $13 , XGREG(13)(T9)
    REG_S       $14 , XGREG(14)(T9)
    REG_S       $15 , XGREG(15)(T9)
    REG_S       $16 , XGREG(16)(T9)
    REG_S       $17 , XGREG(17)(T9)
    REG_S       $18 , XGREG(18)(T9)
    REG_S       $19 , XGREG(19)(T9)
    REG_S       $20 , XGREG(20)(T9)
    REG_S       $21 , XGREG(21)(T9)
    REG_S       $22 , XGREG(22)(T9)
    REG_S       $23 , XGREG(23)(T9)
    REG_S       $24 , XGREG(24)(T9)
    REG_S       $25 , XGREG(25)(T9)
    ;/*
    ; * $26 $27 是 K0 K1
    ; */
    REG_S       $28 , XGREG(28)(T9)
    REG_S       $29 , XGREG(29)(T9)
    REG_S       $30 , XGREG(30)(T9)
    REG_S       $31 , XGREG(31)(T9)

    .set    pop
    MACRO_END()

;/*********************************************************************************************************
;  恢复 $0 - $31 寄存器(参数 T9: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

MACRO_DEF(RESTORE_GREGS)
    .set    push
    .set    noat

    ;/*
    ; * $0 固定为 0
    ; */
    REG_L       $1  , XGREG(1)(T9)
    REG_L       $2  , XGREG(2)(T9)
    REG_L       $3  , XGREG(3)(T9)
    REG_L       $4  , XGREG(4)(T9)
    REG_L       $5  , XGREG(5)(T9)
    REG_L       $6  , XGREG(6)(T9)
    REG_L       $7  , XGREG(7)(T9)
    REG_L       $8  , XGREG(8)(T9)
    REG_L       $9  , XGREG(9)(T9)
    REG_L       $10 , XGREG(10)(T9)
    REG_L       $11 , XGREG(11)(T9)
    REG_L       $12 , XGREG(12)(T9)
    REG_L       $13 , XGREG(13)(T9)
    REG_L       $14 , XGREG(14)(T9)
    REG_L       $15 , XGREG(15)(T9)
    REG_L       $16 , XGREG(16)(T9)
    REG_L       $17 , XGREG(17)(T9)
    REG_L       $18 , XGREG(18)(T9)
    REG_L       $19 , XGREG(19)(T9)
    REG_L       $20 , XGREG(20)(T9)
    REG_L       $21 , XGREG(21)(T9)
    REG_L       $22 , XGREG(22)(T9)
    REG_L       $23 , XGREG(23)(T9)
    ;/*
    ; * $24 $25 是 T8 T9(后面会恢复), O32 与 N64 ABI, T8 T9 都是 $24 $25
    ; */
    ;/*
    ; * $26 $27 是 K0 K1
    ; */
    REG_L       $28 , XGREG(28)(T9)
    REG_L       $29 , XGREG(29)(T9)
    REG_L       $30 , XGREG(30)(T9)
    REG_L       $31 , XGREG(31)(T9)

    .set    pop
    MACRO_END()

;/*********************************************************************************************************
;  保存寄存器(参数 T9: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

MACRO_DEF(SAVE_REGS)
    .set    push
    .set    noat

    SAVE_GREGS                                                          ;/*  保存 $0 - $31 寄存器        */

    REG_S       RA , XEPC(T9)                                           ;/*  RA 代替 EPC 保存            */

    MFC0_EHB(T1, CP0_STATUS)                                            ;/*  保存 STATUS 寄存器          */
    REG_S       T1 , XSR(T9)

    MFC0_LONG_EHB(T1, CP0_BADVADDR)                                     ;/*  保存 BADVADDR 寄存器        */
    REG_S       T1 , XBADVADDR(T9)

    MFC0_EHB(T1, CP0_CAUSE)                                             ;/*  保存 CAUSE 寄存器           */
    REG_S       T1 , XCAUSE(T9)

    MFLO        T1                                                      ;/*  保存 LO 寄存器              */
    REG_S       T1 , XLO(T9)

    MFHI        T1                                                      ;/*  保存 HI 寄存器              */
    REG_S       T1 , XHI(T9)

    .set    pop
    MACRO_END()

;/*********************************************************************************************************
;  恢复寄存器(参数 T9: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

MACRO_DEF(RESTORE_REGS)
    .set    push
    .set    noat

    RESTORE_GREGS                                                       ;/*  恢复 $0 - $31 寄存器        */

    REG_L       T8 , XLO(T9)                                            ;/*  恢复 LO 寄存器              */
    MTLO        T8

    REG_L       T8 , XHI(T9)                                            ;/*  恢复 HI 寄存器              */
    MTHI        T8

    REG_L       T8 , XCAUSE(T9)                                         ;/*  恢复 CAUSE 寄存器           */
    MTC0_EHB(T8, CP0_CAUSE)

    REG_L       T8 , XBADVADDR(T9)                                      ;/*  恢复 BADVADDR 寄存器        */
    MTC0_LONG_EHB(T8, CP0_BADVADDR)

    ;/*
    ; * O32 与 N64 ABI, T8 T9 都是 $24 $25
    ; */

    REG_L       K0 , XSR(T9)                                            ;/*  恢复 SR  寄存器             */
    REG_L       K1 , XEPC(T9)                                           ;/*  恢复 EPC 寄存器             */

    REG_L       T8 , XGREG(24)(T9)                                      ;/*  恢复 T8 寄存器              */
    REG_L       T9 , XGREG(25)(T9)                                      ;/*  恢复 T9 寄存器              */

    ORI         K0 , K0 , ST0_EXL                                       ;/*  通过设置 EXL 位             */

    MTC0_LONG(K1, CP0_EPC)                                              ;/*  恢复 EPC 寄存器             */
    MTC0        K0 , CP0_STATUS                                         ;/*  进入内核模式，并关中断      */
    EHB
    ERET                                                                ;/*  清除 EXL 位并返回           */
    NOP

    .set    pop
    MACRO_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
