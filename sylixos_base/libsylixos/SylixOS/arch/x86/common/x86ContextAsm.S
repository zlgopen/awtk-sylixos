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
;** 文   件   名: x86ContextAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2016 年 07 月 04 日
;**
;** 描        述: x86 体系构架上下文切换.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>
#include "x86ContextAsm.h"

    FILE_BEGIN()

    IMPORT_LABEL(_SchedSwp)
#if LW_CFG_SMP_EN > 0
    IMPORT_LABEL(_SchedSafeStack)
#endif
#if LW_CFG_COROUTINE_EN > 0
    IMPORT_LABEL(_SchedCrSwp)
#endif

    EXPORT_LABEL(archTaskCtxStart)
    EXPORT_LABEL(archTaskCtxSwitch)
#if LW_CFG_COROUTINE_EN > 0
    EXPORT_LABEL(archCrtCtxSwitch)
#endif
    EXPORT_LABEL(archIntCtxLoad)
    EXPORT_LABEL(archSigCtxLoad)

;/*********************************************************************************************************
;  archResumePc
;*********************************************************************************************************/

FUNC_DEF(archResumePc)
    RET
    FUNC_END(archResumePc)

;/*********************************************************************************************************
;  当前 CPU 开始运行多任务执行
;  参数为当前 CPU 控制块, 即 X86_SP_ARG1 为当前 CPU 控制块指针
;  根据 ABI，%EDI 由被调用函数保存，所以使用 %EDI 保存当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxStart)
    MOVL    X86_SP_ARG1(%ESP) , %EDX                                    ;/*  EDX = 当前 CPU 控制块指针   */

    MOVL    0(%EDX) , %EAX                                              ;/*  EAX = 当前TCB的 REG_CTX 地址*/

    ;/*
    ; * 一定是恢复 RING0 上下文
    ; */
    KERN_RESTORE_REGS                                                   ;/*  恢复所有寄存器              */
    FUNC_END(archTaskCtxStart)

;/*********************************************************************************************************
;  线程切换 
;  参数为当前 CPU 控制块, 即 X86_SP_ARG1 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxSwitch)
    MOVL    X86_SP_ARG1(%ESP) , %EDX                                    ;/*  EDX = 当前 CPU 控制块指针   */

    MOVL    0(%EDX) , %EAX                                              ;/*  EAX = 当前TCB的 REG_CTX 地址*/

    ;/*
    ; * 一定是 RING0
    ; */
    KERN_SAVE_REGS                                                      ;/*  保存所有寄存器              */

    MOVL    %EDX , %EDI                                                 ;/*  EDI = 当前 CPU 控制块指针   */
#if LW_CFG_SMP_EN > 0
    PUSHL   %EDI                                                        ;/*  参数 = 当前 CPU 控制块指针  */
    CALL    _SchedSafeStack                                             ;/*  _SchedSafeStack();          */
    MOVL    %EAX , %ESP
#endif

    PUSHL   %EDI                                                        ;/*  参数 = 当前 CPU 控制块指针  */
    CALL    _SchedSwp                                                   ;/*  _SchedSwp();                */
    ADDL    $ARCH_REG_SIZE , %ESP

    MOVL    0(%EDI) , %EAX                                              ;/*  EAX = 当前TCB的 REG_CTX 地址*/

    ;/*
    ; * 以后可能会返回到 RING3
    ; */
    KERN_RESTORE_REGS                                                   ;/*  恢复所有寄存器              */
    FUNC_END(archTaskCtxSwitch)

;/*********************************************************************************************************
;  协程切换 
;  参数为当前 CPU 控制块, 即 X86_SP_ARG1 为当前 CPU 控制块指针
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    MOVL    X86_SP_ARG1(%ESP) , %EDX                                    ;/*  EDX = 当前 CPU 控制块指针   */

    MOVL    8(%EDX) , %EAX                                              ;/*  EAX = 当前CCB的 REG_CTX 地址*/

    KERN_SAVE_REGS                                                      ;/*  保存所有寄存器              */

    MOVL    %EDX , %EDI                                                 ;/*  EDI = 当前 CPU 控制块指针   */

    PUSHL   %EDI                                                        ;/*  参数 = 当前 CPU 控制块指针  */
    CALL    _SchedCrSwp                                                 ;/*  _SchedCrSwp();              */
    ADDL    $ARCH_REG_SIZE , %ESP

    MOVL    8(%EDI) , %EAX                                              ;/*  EAX = 当前CCB的 REG_CTX 地址*/

    ;/*
    ; * 一定是恢复同级上下文
    ; */
    KERN_RESTORE_REGS                                                   ;/*  恢复所有寄存器              */
    FUNC_END(archCrtCtxSwitch)
#endif

;/*********************************************************************************************************
;  中断返回时, 线程装载 
;  参数为当前 CPU 控制块, 即 X86_SP_ARG1 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    MOVL    X86_SP_ARG1(%ESP) , %EDX                                    ;/*  EDX = 当前 CPU 控制块指针   */

    MOVL    0(%EDX) , %EAX                                              ;/*  EAX = 当前TCB的 REG_CTX 地址*/

    ;/*
    ; * 以后可能会返回到 RING3
    ; */
    KERN_RESTORE_REGS                                                   ;/*  恢复所有寄存器              */
    FUNC_END(archIntCtxLoad)

;/*********************************************************************************************************
;  信号上下文返回
;  参数为 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    MOVL    X86_SP_ARG1(%ESP) , %EAX                                    ;/*  EAX = ARCH_REG_CTX 指针     */

    ;/*
    ; * 一定是恢复同级上下文
    ; */
    KERN_RESTORE_REGS                                                   ;/*  恢复所有寄存器              */
    FUNC_END(archSigCtxLoad)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
