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
;** 文   件   名: x64ContextAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2017 年 06 月 06 日
;**
;** 描        述: x86-64 体系构架上下文切换.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>
#include "x64ContextAsm.h"

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
;  参数为当前 CPU 控制块(%RDI 寄存器)
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxStart)
    MOV     0(%RDI) , %RAX                                              ;/*  RAX = 当前TCB的 REG_CTX 地址*/

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archTaskCtxStart)

;/*********************************************************************************************************
;  线程切换 
;  参数为当前 CPU 控制块(%RDI 寄存器)
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxSwitch)
    MOV     0(%RDI) , %RAX                                              ;/*  RAX = 当前TCB的 REG_CTX 地址*/

    SAVE_REGS                                                           ;/*  保存所有寄存器              */

#if LW_CFG_SMP_EN > 0
    MOV     %RDI , %R12                                                 ;/*  参数 = 当前 CPU 控制块指针  */
    CALL    _SchedSafeStack                                             ;/*  _SchedSafeStack();          */
    MOV     %R12 , %RDI
    MOV     %RAX , %RSP
#endif

    MOV     %RDI , %R12                                                 ;/*  参数 = 当前 CPU 控制块指针  */
    CALL    _SchedSwp                                                   ;/*  _SchedSwp();                */
    MOV     %R12 , %RDI

    MOV     0(%RDI) , %RAX                                              ;/*  RAX = 当前TCB的 REG_CTX 地址*/

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archTaskCtxSwitch)

;/*********************************************************************************************************
;  协程切换 
;  参数为当前 CPU 控制块(%RDI 寄存器)
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    MOV     16(%RDI) , %RAX                                             ;/*  RAX = 当前CCB的 REG_CTX 地址*/

    SAVE_REGS                                                           ;/*  保存所有寄存器              */

    MOV     %RDI , %R12                                                 ;/*  参数 = 当前 CPU 控制块指针  */
    CALL    _SchedCrSwp                                                 ;/*  _SchedCrSwp();              */
    MOV     %R12 , %RDI

    MOV     16(%RDI) , %RAX                                             ;/*  RAX = 当前CCB的 REG_CTX 地址*/

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archCrtCtxSwitch)
#endif

;/*********************************************************************************************************
;  中断返回时, 线程装载 
;  参数为当前 CPU 控制块(%RDI 寄存器)
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    MOV     0(%RDI) , %RAX                                              ;/*  RAX = 当前TCB的 REG_CTX 地址*/

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archIntCtxLoad)

;/*********************************************************************************************************
;  信号上下文返回
;  参数为 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    MOV     %RDI , %RAX                                                 ;/*  RAX = ARCH_REG_CTX 指针     */

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archSigCtxLoad)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
