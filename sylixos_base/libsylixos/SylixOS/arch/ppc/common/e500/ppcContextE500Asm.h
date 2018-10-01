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
** 文   件   名: ppcContextE500Asm.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 05 月 12 日
**
** 描        述: PowerPC E500 体系构架上下文切换.
*********************************************************************************************************/

#ifndef __ARCH_PPCCONTEXTE500ASM_H
#define __ARCH_PPCCONTEXTE500ASM_H

#include "arch/ppc/arch_regs.h"

/*********************************************************************************************************
  E500 异常
  使用异常临时栈, 并在异常临时栈开辟临时上下文保存区, 将 volatile 寄存器保存到临时上下文保存区
  E500 总是使能 MMU, 所以没有 ENABLE_MMU 操作
*********************************************************************************************************/

MACRO_DEF(E500_EXC_SAVE_VOLATILE)
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
  E500 临界输入异常
  使用异常临时栈, 并在异常临时栈开辟临时上下文保存区, 将 volatile 寄存器保存到临时上下文保存区
  E500 总是使能 MMU, 所以没有 ENABLE_MMU 操作
*********************************************************************************************************/

MACRO_DEF(E500_CI_EXC_SAVE_VOLATILE)
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

    MFSPR   R0  , CSRR0
    ISYNC
    STW     R0  , XSRR0(SP)                                             /*  保存 CSRR0                  */
    SYNC

    MFSPR   R0  , CSRR1
    ISYNC
    STW     R0  , XSRR1(SP)                                             /*  保存 CSRR1                  */
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
  E500 机器检查异常
  使用异常临时栈, 并在异常临时栈开辟临时上下文保存区, 将 volatile 寄存器保存到临时上下文保存区
  E500 总是使能 MMU, 所以没有 ENABLE_MMU 操作
*********************************************************************************************************/

MACRO_DEF(E500_MC_EXC_SAVE_VOLATILE)
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

    MFSPR   R0  , MCSRR0
    ISYNC
    STW     R0  , XSRR0(SP)                                             /*  保存 MCSRR0                 */
    SYNC

    MFSPR   R0  , MCSRR1
    ISYNC
    STW     R0  , XSRR1(SP)                                             /*  保存 MCSRR1                 */
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

#endif                                                                  /* __ARCH_PPCCONTEXTE500ASM_H   */
/*********************************************************************************************************
  END
*********************************************************************************************************/
