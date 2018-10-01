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
** 文   件   名: ppcContextAsm.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 12 月 17 日
**
** 描        述: PowerPC 体系构架上下文切换.
*********************************************************************************************************/

#ifndef __ARCH_PPCCONTEXTASM_H
#define __ARCH_PPCCONTEXTASM_H

#include <config/kernel/vmm_cfg.h>
#include "arch/ppc/arch_regs.h"

/*********************************************************************************************************
  保存寄存器(参数 R4: ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(SAVE_REGS)
    STW     R0  ,  XR(0)(R4)                                            /*  保存 R0 - R31               */
    STW     R1  ,  XR(1)(R4)
    STW     R2  ,  XR(2)(R4)
    STW     R3  ,  XR(3)(R4)
    STW     R4  ,  XR(4)(R4)
    STW     R5  ,  XR(5)(R4)
    STW     R6  ,  XR(6)(R4)
    STW     R7  ,  XR(7)(R4)
    STW     R8  ,  XR(8)(R4)
    STW     R9  ,  XR(9)(R4)
    STW     R10 , XR(10)(R4)
    STW     R11 , XR(11)(R4)
    STW     R12 , XR(12)(R4)
    STW     R13 , XR(13)(R4)
    STW     R14 , XR(14)(R4)
    STW     R15 , XR(15)(R4)
    STW     R16 , XR(16)(R4)
    STW     R17 , XR(17)(R4)
    STW     R18 , XR(18)(R4)
    STW     R19 , XR(19)(R4)
    STW     R20 , XR(20)(R4)
    STW     R21 , XR(21)(R4)
    STW     R22 , XR(22)(R4)
    STW     R23 , XR(23)(R4)
    STW     R24 , XR(24)(R4)
    STW     R25 , XR(25)(R4)
    STW     R26 , XR(26)(R4)
    STW     R27 , XR(27)(R4)
    STW     R28 , XR(28)(R4)
    STW     R29 , XR(29)(R4)
    STW     R30 , XR(30)(R4)
    STW     R31 , XR(31)(R4)

    MFLR    R0                                                          /*  LR 代替 SRR0 被保存         */
    ISYNC
    STW     R0  , XSRR0(R4)
    SYNC

    MFMSR   R0                                                          /*  MSR 代替 SRR1 被保存        */
    ISYNC
    STW     R0  , XSRR1(R4)
    SYNC

    MFLR    R0
    ISYNC
    STW     R0  , XLR(R4)                                               /*  保存 LR                     */
    SYNC

    MFCTR   R0
    ISYNC
    STW     R0  , XCTR(R4)                                              /*  保存 CTR                    */
    SYNC

    MFXER   R0
    ISYNC
    STW     R0  , XXER(R4)                                              /*  保存 XER                    */
    SYNC

    MFCR    R0
    ISYNC
    STW     R0  , XCR(R4)                                               /*  保存 CR                     */
    SYNC
    MACRO_END()

/*********************************************************************************************************
  恢复寄存器(参数 R4: ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(RESTORE_REGS)
    LWZ     R1  ,  XR(1)(R4)                                            /*  恢复 R1 - R31               */
    LWZ     R2  ,  XR(2)(R4)
    LWZ     R3  ,  XR(3)(R4)
    LWZ     R5  ,  XR(5)(R4)
    LWZ     R6  ,  XR(6)(R4)
    LWZ     R7  ,  XR(7)(R4)
    LWZ     R8  ,  XR(8)(R4)
    LWZ     R9  ,  XR(9)(R4)
    LWZ     R10 , XR(10)(R4)
    LWZ     R11 , XR(11)(R4)
    LWZ     R12 , XR(12)(R4)
    LWZ     R13 , XR(13)(R4)
    LWZ     R14 , XR(14)(R4)
    LWZ     R15 , XR(15)(R4)
    LWZ     R16 , XR(16)(R4)
    LWZ     R17 , XR(17)(R4)
    LWZ     R18 , XR(18)(R4)
    LWZ     R19 , XR(19)(R4)
    LWZ     R20 , XR(20)(R4)
    LWZ     R21 , XR(21)(R4)
    LWZ     R22 , XR(22)(R4)
    LWZ     R23 , XR(23)(R4)
    LWZ     R24 , XR(24)(R4)
    LWZ     R25 , XR(25)(R4)
    LWZ     R26 , XR(26)(R4)
    LWZ     R27 , XR(27)(R4)
    LWZ     R28 , XR(28)(R4)
    LWZ     R29 , XR(29)(R4)
    LWZ     R30 , XR(30)(R4)
    LWZ     R31 , XR(31)(R4)

    LWZ     R0  , XCR(R4)                                               /*  恢复 CR                     */
    SYNC
    MTCR    R0
    ISYNC

    LWZ     R0  , XXER(R4)                                              /*  恢复 XER                    */
    SYNC
    MTXER   R0
    ISYNC

    LWZ     R0  , XCTR(R4)                                              /*  恢复 CTR                    */
    SYNC
    MTCTR   R0
    ISYNC

    LWZ     R0  , XLR(R4)                                               /*  恢复 LR                     */
    SYNC
    MTLR    R0
    ISYNC

    LWZ     R0  , XSRR1(R4)                                             /*  恢复 SRR1                   */
    SYNC
    MTSPR   SRR1, R0
    ISYNC

    LWZ     R0  , XSRR0(R4)                                             /*  恢复 SRR0                   */
    SYNC
    MTSPR   SRR0, R0
    ISYNC

    LWZ     R0  , XR(0)(R4)                                             /*  恢复 R0                     */
    LWZ     R4  , XR(4)(R4)                                             /*  恢复 R4                     */
    SYNC

    RFI                                                                 /*  从 SRR0 返回，同时 MSR=SRR1 */
    MACRO_END()

/*********************************************************************************************************
  使能 MMU
*********************************************************************************************************/

#if LW_CFG_VMM_EN > 0
MACRO_DEF(ENABLE_MMU)
    MTSPR   SPRG0 , R3                                                  /*  SPRG0 暂存 R3               */
    ISYNC

    MFMSR   R3
    ISYNC
    ORI     R3 , R3 , ARCH_PPC_MSR_DR | ARCH_PPC_MSR_IR                 /*  使能 DR 与 IR 位            */
    ISYNC
    MTMSR   R3
    ISYNC

    MFSPR   R3 , SPRG0                                                  /*  恢复 R3                     */
    ISYNC
    MACRO_END()
#else
MACRO_DEF(ENABLE_MMU)
    MACRO_END()
#endif

/*********************************************************************************************************
  使用异常临时栈, 并在异常临时栈开辟临时上下文保存区, 将 volatile 寄存器保存到临时上下文保存区
*********************************************************************************************************/

MACRO_DEF(EXC_SAVE_VOLATILE)
    ENABLE_MMU

    MTSPR   SPRG0 , SP                                                  /*  SPRG0 暂存异常前 SP(R1)     */
    ISYNC

    MFSPR   SP  , SPRG1                                                 /*  读出异常临时堆栈地址        */
    ISYNC

    SUBI    SP  , SP , ARCH_REG_CTX_SIZE                                /*  在临时堆栈开辟上下文保存区  */

    STW     R0  , XR(0)(SP)

    MFSPR   R0  , SPRG0                                                 /*  保存异常前 SP(R1)           */
    ISYNC
    STW     R0  , XR(1)(SP)

    STW     R2  , XR(2)(SP)
    STW     R3  , XR(3)(SP)
    STW     R4  , XR(4)(SP)
    STW     R5  , XR(5)(SP)
    STW     R6  , XR(6)(SP)
    STW     R7  , XR(7)(SP)
    STW     R8  , XR(8)(SP)
    STW     R9  , XR(9)(SP)
    STW     R10 , XR(10)(SP)
    STW     R11 , XR(11)(SP)
    STW     R12 , XR(12)(SP)
    STW     R13 , XR(13)(SP)

    MFSPR   R0  , SRR0
    ISYNC
    STW     R0  , XSRR0(SP)                                             /*  保存 SRR0                   */
    SYNC

    MFSPR   R0  , SRR1
    ISYNC
    STW     R0  , XSRR1(SP)                                             /*  保存 SRR1                   */
    SYNC

    MFLR    R0
    ISYNC
    STW     R0  , XLR(SP)                                               /*  保存 LR                     */
    SYNC

    MFCTR   R0
    ISYNC
    STW     R0  , XCTR(SP)                                              /*  保存 CTR                    */
    SYNC

    MFXER   R0
    ISYNC
    STW     R0  , XXER(SP)                                              /*  保存 XER                    */
    SYNC

    MFCR    R0
    ISYNC
    STW     R0  , XCR(SP)                                               /*  保存 CR                     */
    SYNC
    MACRO_END()

/*********************************************************************************************************
  保存 non volatile 寄存器(参数 R3: ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(EXC_SAVE_NON_VOLATILE)
    STW     R14 , XR(14)(R3)
    STW     R15 , XR(15)(R3)
    STW     R16 , XR(16)(R3)
    STW     R17 , XR(17)(R3)
    STW     R18 , XR(18)(R3)
    STW     R19 , XR(19)(R3)
    STW     R20 , XR(20)(R3)
    STW     R21 , XR(21)(R3)
    STW     R22 , XR(22)(R3)
    STW     R23 , XR(23)(R3)
    STW     R24 , XR(24)(R3)
    STW     R25 , XR(25)(R3)
    STW     R26 , XR(26)(R3)
    STW     R27 , XR(27)(R3)
    STW     R28 , XR(28)(R3)
    STW     R29 , XR(29)(R3)
    STW     R30 , XR(30)(R3)
    STW     R31 , XR(31)(R3)
    MACRO_END()

/*********************************************************************************************************
  拷贝 volatile 寄存器(参数 R3: 目的 ARCH_REG_CTX 地址, 参数 SP: 源 ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(EXC_COPY_VOLATILE)
    LWZ     R0  , XR(0)(SP)
    STW     R0  , XR(0)(R3)

    LWZ     R0  , XR(1)(SP)
    STW     R0  , XR(1)(R3)

    LWZ     R0  , XR(2)(SP)
    STW     R0  , XR(2)(R3)

    LWZ     R0  , XR(3)(SP)
    STW     R0  , XR(3)(R3)

    LWZ     R0  , XR(4)(SP)
    STW     R0  , XR(4)(R3)

    LWZ     R0  , XR(5)(SP)
    STW     R0  , XR(5)(R3)

    LWZ     R0  , XR(6)(SP)
    STW     R0  , XR(6)(R3)

    LWZ     R0  , XR(7)(SP)
    STW     R0  , XR(7)(R3)

    LWZ     R0  , XR(8)(SP)
    STW     R0  , XR(8)(R3)

    LWZ     R0  , XR(9)(SP)
    STW     R0  , XR(9)(R3)

    LWZ     R0  , XR(10)(SP)
    STW     R0  , XR(10)(R3)

    LWZ     R0  , XR(11)(SP)
    STW     R0  , XR(11)(R3)

    LWZ     R0  , XR(12)(SP)
    STW     R0  , XR(12)(R3)

    LWZ     R0  , XR(13)(SP)
    STW     R0  , XR(13)(R3)

    LWZ     R0  , XSRR0(SP)                                             /*  保存 SRR0                   */
    STW     R0  , XSRR0(R3)

    LWZ     R0  , XSRR1(SP)                                             /*  保存 SRR1                   */
    STW     R0  , XSRR1(R3)

    LWZ     R0  , XLR(SP)                                               /*  保存 LR                     */
    STW     R0  , XLR(R3)

    LWZ     R0  , XCTR(SP)                                              /*  保存 CTR                    */
    STW     R0  , XCTR(R3)

    LWZ     R0  , XXER(SP)                                              /*  保存 XER                    */
    STW     R0  , XXER(R3)

    LWZ     R0  , XCR(SP)                                               /*  保存 CR                     */
    STW     R0  , XCR(R3)
    MACRO_END()

#endif                                                                  /*  __ARCH_PPCCONTEXTASM_H      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
