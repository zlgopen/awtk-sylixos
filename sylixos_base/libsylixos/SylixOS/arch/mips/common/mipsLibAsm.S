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
;** 文   件   名: mipsLibAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2015 年 09 月 01 日
;**
;** 描        述: MIPS 体系架构内部库.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/cpu/cpu_cfg.h>
#include "mipsContextAsm.h"

    FILE_BEGIN()

    IMPORT_LABEL(__setjmpSetup)
    IMPORT_LABEL(__sigsetjmpSetup)
    IMPORT_LABEL(__longjmpSetup)
    IMPORT_LABEL(__siglongjmpSetup)
    IMPORT_LABEL(bspReboot)

    EXPORT_LABEL(archGetGP)
    EXPORT_LABEL(setjmp)
    EXPORT_LABEL(sigsetjmp)
    EXPORT_LABEL(longjmp)
    EXPORT_LABEL(siglongjmp)

#if LW_CFG_MIPS_HAS_CLZ_INSTR > 0
    EXPORT_LABEL(archFindLsb)
    EXPORT_LABEL(archFindMsb)
#endif

    EXPORT_LABEL(archIntDisable)
    EXPORT_LABEL(archIntEnable)
    EXPORT_LABEL(archIntEnableForce)
    EXPORT_LABEL(archPageCopy)
    EXPORT_LABEL(archReboot)
    EXPORT_LABEL(archBogoMipsLoop)

    WEAK(archIntDisable)
    WEAK(archIntEnable)
    WEAK(archIntEnableForce)
    WEAK(archGetGP)

;/*********************************************************************************************************
;  获得 _gp
;*********************************************************************************************************/

FUNC_DEF(archGetGP)
    PTR_LA      V0 , _gp
    JR          RA
    NOP
    FUNC_END(archGetGP)

;/*********************************************************************************************************
;  MIPS 关闭总中断
;*********************************************************************************************************/

FUNC_DEF(archIntDisable)
    MFC0_EHB(V0, CP0_STATUS)
    AND         T0 , V0 , ~ST0_IE
    MTC0_EHB(T0, CP0_STATUS)
    JR          RA
    NOP
    FUNC_END(archIntDisable)

;/*********************************************************************************************************
;  MIPS 打开总中断
;*********************************************************************************************************/

FUNC_DEF(archIntEnable)
    ANDI        A0 , A0 , ST0_IE
    BEQ         A0 , ZERO , 1f
    NOP
    MFC0_EHB(T0, CP0_STATUS)
    ORI         T0 , T0 , ST0_IE
    MTC0_EHB(T0, CP0_STATUS)
1:
    JR          RA
    NOP
    FUNC_END(archIntEnable)

;/*********************************************************************************************************
;  MIPS 强制打开总中断
;*********************************************************************************************************/

FUNC_DEF(archIntEnableForce)
    MFC0_EHB(T0, CP0_STATUS)
    ORI         T0 , T0 , ST0_IE
    MTC0_EHB(T0, CP0_STATUS)
    JR          RA
    NOP
    FUNC_END(archIntEnableForce)

;/*********************************************************************************************************
;  MIPS 前导零运算
;*********************************************************************************************************/

#if LW_CFG_MIPS_HAS_CLZ_INSTR > 0
FUNC_DEF(archFindLsb)
    NEGU        V0 , A0
    AND         A0 , V0 , A0
    CLZ         A0 , A0
    LI          V0 , 32
    JR          RA
    REG_SUBU    V0 , V0 , A0
    FUNC_END(archFindLsb)

FUNC_DEF(archFindMsb)
    CLZ         A0 , A0
    LI          V0 , 32
    JR          RA
    REG_SUBU    V0 , V0 , A0
    FUNC_END(archFindMsb)
#endif

;/*********************************************************************************************************
;  注意: setjmp 与 longjmp 上下文结构与线程上下文结构相同
;        但 setjmp 与 longjmp 只保存 callee save 寄存器
;*********************************************************************************************************/

;/*********************************************************************************************************
;  调用设置函数宏
;*********************************************************************************************************/

MACRO_DEF(CALL_SETUP, setup)
    .set    push
    .set    noat

    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    REG_S       RA , ARCH_STK_OFF_VAR(0)(SP)
    REG_S       FP , ARCH_STK_OFF_VAR(1)(SP)
    MOV         FP , SP
    REG_S       A0 , ARCH_STK_OFF_VAR(2)(SP)
    REG_S       A1 , ARCH_STK_OFF_VAR(3)(SP)

    JAL         \setup
    NOP

    MOV         SP , FP
    REG_L       RA , ARCH_STK_OFF_VAR(0)(SP)
    REG_L       FP , ARCH_STK_OFF_VAR(1)(SP)
    REG_L       A0 , ARCH_STK_OFF_VAR(2)(SP)
    REG_L       A1 , ARCH_STK_OFF_VAR(3)(SP)
    REG_ADDU    SP , ARCH_STK_VAR_SIZE

    .set    pop
    MACRO_END()

;/*********************************************************************************************************
;  保存寄存器宏
;*********************************************************************************************************/

MACRO_DEF(SAVE_NON_VOLATILE_REG)
    .set    push
    .set    noat

    REG_S       $16 , XGREG(16)(A0)
    REG_S       $17 , XGREG(17)(A0)
    REG_S       $18 , XGREG(18)(A0)
    REG_S       $19 , XGREG(19)(A0)
    REG_S       $20 , XGREG(20)(A0)
    REG_S       $21 , XGREG(21)(A0)
    REG_S       $22 , XGREG(22)(A0)
    REG_S       $23 , XGREG(23)(A0)
    ;/*
    ; * $24 $25 是 T8 T9
    ; * $26 $27 是 K0 K1
    ; */
    REG_S       $28 , XGREG(28)(A0)
    REG_S       $29 , XGREG(29)(A0)
    REG_S       $30 , XGREG(30)(A0)
    REG_S       $31 , XGREG(31)(A0)

    MFC0_EHB(T0, CP0_STATUS)                                            ;/*  保存 STATUS 寄存器          */
    REG_S       T0 , XSR(A0)

    .set    pop
    MACRO_END()

;/*********************************************************************************************************
;  恢复寄存器宏
;*********************************************************************************************************/

MACRO_DEF(RESTORE_NON_VOLATILE_REG)
    .set    push
    .set    noat

    REG_L       $16 , XGREG(16)(A0)
    REG_L       $17 , XGREG(17)(A0)
    REG_L       $18 , XGREG(18)(A0)
    REG_L       $19 , XGREG(19)(A0)
    REG_L       $20 , XGREG(20)(A0)
    REG_L       $21 , XGREG(21)(A0)
    REG_L       $22 , XGREG(22)(A0)
    REG_L       $23 , XGREG(23)(A0)
    ;/*
    ; * $24 $25 是 T8 T9
    ; * $26 $27 是 K0 K1
    ; */
    REG_L       $28 , XGREG(28)(A0)
    REG_L       $29 , XGREG(29)(A0)
    REG_L       $30 , XGREG(30)(A0)
    REG_L       $31 , XGREG(31)(A0)

    REG_L       T0 , XSR(A0)                                            ;/*  恢复 STATUS 寄存器          */
    MTC0_EHB(T0, CP0_STATUS)

    .set    pop
    MACRO_END()

;/*********************************************************************************************************
;  sigsetjmp (参数为 jmp_buf, mask_saved)
;*********************************************************************************************************/

FUNC_DEF(sigsetjmp)
    CALL_SETUP  __sigsetjmpSetup

    SAVE_NON_VOLATILE_REG

    JR          RA
    MOV         V0 , ZERO
    FUNC_END(sigsetjmp)

;/*********************************************************************************************************
;  setjmp (参数为 jmp_buf)
;*********************************************************************************************************/

FUNC_DEF(setjmp)
    CALL_SETUP  __setjmpSetup

    SAVE_NON_VOLATILE_REG

    JR          RA
    MOV         V0 , ZERO
    FUNC_END(setjmp)

;/*********************************************************************************************************
;  siglongjmp (参数为 jmp_buf, retval)
;*********************************************************************************************************/

FUNC_DEF(siglongjmp)
    CALL_SETUP  __siglongjmpSetup

    RESTORE_NON_VOLATILE_REG

    MOV         V0 , A1
    BNE         V0 , ZERO , siglongjmpnotzero
    NOP
    LI          V0 , 1
LINE_LABEL(siglongjmpnotzero)
    JR          RA
    NOP
    FUNC_END(siglongjmp)

;/*********************************************************************************************************
;  longjmp (参数为 jmp_buf, retval)
;*********************************************************************************************************/

FUNC_DEF(longjmp)
    CALL_SETUP  __longjmpSetup

    RESTORE_NON_VOLATILE_REG

    MOV         V0 , A1
    BNE         V0 , ZERO , longjmpnotzero
    NOP
    LI          V0 , 1
LINE_LABEL(longjmpnotzero)
    JR          RA
    NOP
    FUNC_END(longjmp)

;/*********************************************************************************************************
;  系统重启
;*********************************************************************************************************/

FUNC_DEF(archReboot)
    J           bspReboot                                               ;/*  由BSP进行复位操作, 不再返回 */
    NOP
    FUNC_END(archReboot)

;/*********************************************************************************************************
;  Bogo 循环
;*********************************************************************************************************/

FUNC_DEF(archBogoMipsLoop)
1:
    BNE         A0 , ZERO , 1b
    REG_SUBU    A0 , 1
    JR          RA
    NOP
    FUNC_END(archBogoMipsLoop)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
