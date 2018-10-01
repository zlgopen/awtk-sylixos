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
;** 文   件   名: riscvExcAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2018 年 03 月 20 日
;**
;** 描        述: RISC-V 体系构架异常处理.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>
#include "riscvContextAsm.h"

    FILE_BEGIN()

    IMPORT_LABEL(archTrapHandle)
    EXPORT_LABEL(archTrapEntry)

;/*********************************************************************************************************
;  异常进入点
;*********************************************************************************************************/

EXCE_DEF(archTrapEntry)
    ;/*
    ; * 使用异常临时栈, 并在异常临时栈开辟临时上下文保存区, 将 volatile 寄存器保存到临时上下文保存区
    ; */
    EXC_SAVE_VOLATILE

    ;/*
    ; * 重新加载 GP
    ; */
    LA      GP , _gp

    ;/*
    ; * API_InterEnter()
    ; */
    CALL    API_InterEnter                                              ;/*  获取中断层数放到返回值 RV0  */

    LI      T0 , 1
    BGTU    RV0 , T0 , 1f

    ;/*
    ; * 获得当前任务 TCB 的 ARCH_REG_CTX 地址
    ; */
    CALL    API_ThreadTcbInter                                          ;/*  get current tcb             */

    ;/*
    ; * 拷贝 volatile 寄存器到当前任务 TCB 的 ARCH_REG_CTX 里
    ; */
    EXC_COPY_VOLATILE

    ;/*
    ; * 保存 non volatile 寄存器到当前任务 TCB 的 ARCH_REG_CTX 里
    ; */
    EXC_SAVE_NON_VOLATILE

    MOVE    S0 , RV0

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并设置 SP
    ; */
    CALL    API_InterStackBaseGet

    MOVE    SP , RV0

    MOVE    A0 , S0
2:
    ;/*
    ; * handle(ARCH_REG_CTX 指针)
    ; */
    CALL    archTrapHandle                                              ;/*  调用处理函数                */

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数
    ; */
    CALL    API_InterExit

    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    MOVE    T0 , SP
    RESTORE_BIG_REG_CTX                                                 ;/*  恢复所有寄存器              */

1:
    ;/*
    ; * 不是第一次进入中断
    ; */
    REG_L   RV0 , SP_OFFSET(SP)                                         ;/*  异常前 SP(XSCRATCH)         */
    ADDI    RV0 , RV0 , -ARCH_REG_CTX_SIZE                              ;/*  在异常堆栈开辟上下文保存区  */

    ;/*
    ; * 拷贝 volatile 寄存器到异常堆栈里的上下文保存区
    ; */
    EXC_COPY_VOLATILE

    ;/*
    ; * 保存 non volatile 寄存器到异常堆栈里
    ; */
    EXC_SAVE_NON_VOLATILE

    MOVE    SP , RV0                                                    ;/*  使用异常堆栈                */
    MOVE    A0 , RV0

    J       2b
    FUNC_END(archTrapEntry)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
