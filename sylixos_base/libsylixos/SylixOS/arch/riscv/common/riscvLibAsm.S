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
;** 文   件   名: riscvLibAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2018 年 03 月 20 日
;**
;** 描        述: RISC-V 体系构架内部库.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include "arch/riscv/arch_regs.h"

    FILE_BEGIN()

    IMPORT_LABEL(__setjmpSetup)
    IMPORT_LABEL(__sigsetjmpSetup)
    IMPORT_LABEL(__longjmpSetup)
    IMPORT_LABEL(__siglongjmpSetup)
    IMPORT_LABEL(bspReboot)
    
    EXPORT_LABEL(archWaitForInterrupt)
    EXPORT_LABEL(archGetGp)

    EXPORT_LABEL(setjmp)
    EXPORT_LABEL(sigsetjmp)
    EXPORT_LABEL(longjmp)
    EXPORT_LABEL(siglongjmp)

    EXPORT_LABEL(archFindLsb)
    EXPORT_LABEL(archFindMsb)

    EXPORT_LABEL(archIntDisable)
    EXPORT_LABEL(archIntEnable)
    EXPORT_LABEL(archIntEnableForce)
    EXPORT_LABEL(archReboot)
    EXPORT_LABEL(archBogoMipsLoop)

    WEAK(archIntDisable)
    WEAK(archIntEnable)
    WEAK(archIntEnableForce)

;/*********************************************************************************************************
;  RISC-V 等待中断
;*********************************************************************************************************/

FUNC_DEF(archWaitForInterrupt)
    WFI
    RET
    FUNC_END(archWaitForInterrupt)

;/*********************************************************************************************************
;  获得 GP
;*********************************************************************************************************/

FUNC_DEF(archGetGp)
    .option push
    .option norelax
    MOVE    RV0 , GP
    RET
    .option pop
    FUNC_END(archGetGp)

;/*********************************************************************************************************
;  RISC-V 开关闭总中断
;*********************************************************************************************************/

FUNC_DEF(archIntDisable)
    CSRRCI  RV0 , XSTATUS , XSTATUS_XIE
    RET
    FUNC_END(archIntDisable)

FUNC_DEF(archIntEnable)
    ANDI    A0 , A0 , XSTATUS_XIE
    BEQZ    A0 , 1f
    CSRSI   XSTATUS , XSTATUS_XIE
1:
    RET
    FUNC_END(archIntEnable)

FUNC_DEF(archIntEnableForce)
    CSRSI   XSTATUS , XSTATUS_XIE
    RET
    FUNC_END(archIntEnableForce)
    
;/*********************************************************************************************************
;  注意: setjmp 与 longjmp 上下文结构与线程上下文结构不同
;*********************************************************************************************************/

;/*********************************************************************************************************
;  调用设置函数宏
;*********************************************************************************************************/

MACRO_DEF(CALL_SETUP, setup)
    ADDI    SP  , SP , -(4 * ARCH_REG_CTX_SIZE)
    REG_S   A0  , ( 0 * ARCH_REG_SIZE)(SP)
    REG_S   A1  , ( 1 * ARCH_REG_SIZE)(SP)
    REG_S   RA  , ( 2 * ARCH_REG_SIZE)(SP)

    CALL    \setup

    REG_L   A0  , ( 0 * ARCH_REG_SIZE)(SP)
    REG_L   A1  , ( 1 * ARCH_REG_SIZE)(SP)
    REG_L   RA  , ( 2 * ARCH_REG_SIZE)(SP)

    ADDI    SP  , SP , (4 * ARCH_REG_CTX_SIZE)
    MACRO_END()

;/*********************************************************************************************************
;  sigsetjmp (参数为 jmp_buf, mask_saved)
;*********************************************************************************************************/

FUNC_DEF(sigsetjmp)
    CALL_SETUP  __sigsetjmpSetup

    REG_S   RA  , ( 0 * ARCH_REG_SIZE)(A0)
    REG_S   S0  , ( 1 * ARCH_REG_SIZE)(A0)
    REG_S   S1  , ( 2 * ARCH_REG_SIZE)(A0)
    REG_S   S2  , ( 3 * ARCH_REG_SIZE)(A0)
    REG_S   S3  , ( 4 * ARCH_REG_SIZE)(A0)
    REG_S   S4  , ( 5 * ARCH_REG_SIZE)(A0)
    REG_S   S5  , ( 6 * ARCH_REG_SIZE)(A0)
    REG_S   S6  , ( 7 * ARCH_REG_SIZE)(A0)
    REG_S   S7  , ( 8 * ARCH_REG_SIZE)(A0)
    REG_S   S8  , ( 9 * ARCH_REG_SIZE)(A0)
    REG_S   S9  , (10 * ARCH_REG_SIZE)(A0)
    REG_S   S10 , (11 * ARCH_REG_SIZE)(A0)
    REG_S   S11 , (12 * ARCH_REG_SIZE)(A0)
    REG_S   SP  , (13 * ARCH_REG_SIZE)(A0)
    LI      RV0 , 0
    RET
    FUNC_END(sigsetjmp)

;/*********************************************************************************************************
;  setjmp (参数为 jmp_buf)
;*********************************************************************************************************/

FUNC_DEF(setjmp)
    CALL_SETUP  __setjmpSetup

    REG_S   RA  , ( 0 * ARCH_REG_SIZE)(A0)
    REG_S   S0  , ( 1 * ARCH_REG_SIZE)(A0)
    REG_S   S1  , ( 2 * ARCH_REG_SIZE)(A0)
    REG_S   S2  , ( 3 * ARCH_REG_SIZE)(A0)
    REG_S   S3  , ( 4 * ARCH_REG_SIZE)(A0)
    REG_S   S4  , ( 5 * ARCH_REG_SIZE)(A0)
    REG_S   S5  , ( 6 * ARCH_REG_SIZE)(A0)
    REG_S   S6  , ( 7 * ARCH_REG_SIZE)(A0)
    REG_S   S7  , ( 8 * ARCH_REG_SIZE)(A0)
    REG_S   S8  , ( 9 * ARCH_REG_SIZE)(A0)
    REG_S   S9  , (10 * ARCH_REG_SIZE)(A0)
    REG_S   S10 , (11 * ARCH_REG_SIZE)(A0)
    REG_S   S11 , (12 * ARCH_REG_SIZE)(A0)
    REG_S   SP  , (13 * ARCH_REG_SIZE)(A0)
    LI      RV0 , 0
    RET
    FUNC_END(setjmp)
    
;/*********************************************************************************************************
;  siglongjmp (参数为 jmp_buf, retval)
;*********************************************************************************************************/

FUNC_DEF(siglongjmp)
    CALL_SETUP  __siglongjmpSetup

    REG_L   RA  , ( 0 * ARCH_REG_SIZE)(A0)
    REG_L   S0  , ( 1 * ARCH_REG_SIZE)(A0)
    REG_L   S1  , ( 2 * ARCH_REG_SIZE)(A0)
    REG_L   S2  , ( 3 * ARCH_REG_SIZE)(A0)
    REG_L   S3  , ( 4 * ARCH_REG_SIZE)(A0)
    REG_L   S4  , ( 5 * ARCH_REG_SIZE)(A0)
    REG_L   S5  , ( 6 * ARCH_REG_SIZE)(A0)
    REG_L   S6  , ( 7 * ARCH_REG_SIZE)(A0)
    REG_L   S7  , ( 8 * ARCH_REG_SIZE)(A0)
    REG_L   S8  , ( 9 * ARCH_REG_SIZE)(A0)
    REG_L   S9  , (10 * ARCH_REG_SIZE)(A0)
    REG_L   S10 , (11 * ARCH_REG_SIZE)(A0)
    REG_L   S11 , (12 * ARCH_REG_SIZE)(A0)
    REG_L   SP  , (13 * ARCH_REG_SIZE)(A0)
    SEQZ    A0  , A1
    ADD     RV0 , A0 , A1                                              ;/*  RV0 = (A1 == 0) ? 1 : A1     */
    RET
    FUNC_END(siglongjmp)

;/*********************************************************************************************************
;  longjmp (参数为 jmp_buf, retval)
;*********************************************************************************************************/

FUNC_DEF(longjmp)
    CALL_SETUP  __longjmpSetup

    REG_L   RA  , ( 0 * ARCH_REG_SIZE)(A0)
    REG_L   S0  , ( 1 * ARCH_REG_SIZE)(A0)
    REG_L   S1  , ( 2 * ARCH_REG_SIZE)(A0)
    REG_L   S2  , ( 3 * ARCH_REG_SIZE)(A0)
    REG_L   S3  , ( 4 * ARCH_REG_SIZE)(A0)
    REG_L   S4  , ( 5 * ARCH_REG_SIZE)(A0)
    REG_L   S5  , ( 6 * ARCH_REG_SIZE)(A0)
    REG_L   S6  , ( 7 * ARCH_REG_SIZE)(A0)
    REG_L   S7  , ( 8 * ARCH_REG_SIZE)(A0)
    REG_L   S8  , ( 9 * ARCH_REG_SIZE)(A0)
    REG_L   S9  , (10 * ARCH_REG_SIZE)(A0)
    REG_L   S10 , (11 * ARCH_REG_SIZE)(A0)
    REG_L   S11 , (12 * ARCH_REG_SIZE)(A0)
    REG_L   SP  , (13 * ARCH_REG_SIZE)(A0)
    SEQZ    A0  , A1
    ADD     RV0 , A0 , A1                                               ;/*  RV0 = (A1 == 0) ? 1 : A1    */
    RET
    FUNC_END(longjmp)
    
;/*********************************************************************************************************
;  系统重启
;*********************************************************************************************************/

FUNC_DEF(archReboot)
    J       bspReboot
    FUNC_END(archReboot)
    
;/*********************************************************************************************************
;  Bogo 循环(两条指令)
;*********************************************************************************************************/

FUNC_DEF(archBogoMipsLoop)
    LI      T0 , 1
LINE_LABEL(again)
    SUB     A0 , A0 , T0
    BNEZ    A0 , again
    RET
    FUNC_END(archBogoMipsLoop)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
