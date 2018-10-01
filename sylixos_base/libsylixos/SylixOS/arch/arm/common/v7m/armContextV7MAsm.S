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
;** 文   件   名: armContextV7MAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2017 年 11 月 14 日
;**
;** 描        述: ARMv7M 体系构架上下文切换.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>

#include "arch/arm/arch_regs.h"

#if defined(__SYLIXOS_ARM_ARCH_M__)

#include "armSvcV7M.h"

    FILE_BEGIN()

;/*********************************************************************************************************
;  ARMv7M 体系构架相关上下文切换函数
;*********************************************************************************************************/

    IMPORT_LABEL(_SchedSwp)
#if LW_CFG_SMP_EN > 0
    IMPORT_LABEL(_SchedSafeStack)
#endif
#if LW_CFG_COROUTINE_EN > 0
    IMPORT_LABEL(_SchedCrSwp)
#endif

    EXPORT_LABEL(archTaskCtxStart)
    EXPORT_LABEL(archTaskCtxSwitch)
    EXPORT_LABEL(archTaskCtxCopy)
#if LW_CFG_COROUTINE_EN > 0
    EXPORT_LABEL(archCrtCtxSwitch)
#endif
    EXPORT_LABEL(archIntCtxLoad)
    EXPORT_LABEL(archSigCtxLoad)

;/*********************************************************************************************************
;  当前 CPU 开始运行多任务执行
;  参数为当前 CPU 控制块, 即 R0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxStart)
    MOV     R1 , #LW_CFG_CORTEX_M_INT_MASK
    MSR     BASEPRI , R1                                        ;/*  屏蔽除 SVC 外的中断                 */
    MOV     R1 , #SVC_archTaskCtxStart                          ;/*  使用 R1 传递 SVC 命令号             */
    CPSIE   I                                                   ;/*  使能中断                            */
    SVC     0
    BX      LR
    FUNC_END()

;/*********************************************************************************************************
;  线程切换
;  参数为当前 CPU 控制块, 即 R0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxSwitch)
    MOV     R1 , #SVC_archTaskCtxSwitch                         ;/*  使用 R1 传递 SVC 命令号             */
    SVC     0
    BX      LR
    FUNC_END()

;/*********************************************************************************************************
;  协程切换
;  参数为当前 CPU 控制块, 即 R0 为当前 CPU 控制块指针
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    MOV     R1 , #SVC_archCrtCtxSwitch                          ;/*  使用 R1 传递 SVC 命令号             */
    SVC     0
    BX      LR
    FUNC_END()
#endif

;/*********************************************************************************************************
;  中断返回时, 线程装载
;  参数为当前 CPU 控制块, 即 R0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    LDR     R0 , [R0]                                           ;/*  获取当前 TCB 的 REG_CTX 地址        */
    ADD     R12 , R0 , #4                                       ;/*  最后将跳过 SP 的恢复                */

    LDR     R2 , [R0]                                           ;/*  读出 SP 到 R2                       */
    SUB     R2 , R2 , #(ARCH_HW_SAVE_CTX_SIZE)                  ;/*  从 SP 开辟自动出栈寄存器空间        */
    MSR     PSP , R2                                            ;/*  设置 PSP                            */

    ADD     R0 , R0 , #(ARCH_SW_SAVE_CTX_SIZE)                  ;/*  R0 指向 REG_CTX 中自动出栈寄存器    */
    LDM     R0 , {R3-R10}                                       ;/*  将 REG_CTX 中自动出栈寄存器复制到   */
    STM     R2 , {R3-R10}                                       ;/*  SP 中自动出栈寄存器空间             */

    LDM     R12 , {R3-R11, LR}                                  ;/*  恢复 BASEPRI, R4-R11, EXC_RETURN    */
    MSR     BASEPRI , R3                                        ;/*  恢复 BASEPRI                        */

    ;/*
    ; * 从 SP 指向的堆栈恢复 xPSR, PC, LR, R12 以及 R3-R0 寄存器
    ; */
    BX      LR
    FUNC_END()

;/*********************************************************************************************************
;  信号上下文返回
;  参数为 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    MOV     R1 , #SVC_archSigCtxLoad                            ;/*  使用 R1 传递 SVC 命令号             */
    SVC     0
    BX      LR
    FUNC_END()

;/*********************************************************************************************************
;  拷贝任务上下文
;  参数 R0 为目的 ARCH_REG_CTX 指针, R1 为源 ARCH_SW_SAVE_REG_CTX 指针, R2 为源 ARCH_HW_SAVE_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxCopy)
    STMFD   SP! , {R4-R10}

    LDMIA   R1! , {R3-R10}
    STMIA   R0! , {R3-R10}
    LDMIA   R1! , {R3-R5}
    STMIA   R0! , {R3-R5}

    LDMIA   R2! , {R3-R10}
    STMIA   R0! , {R3-R10}

    LDMFD   SP! , {R4-R10}
    BX      LR
    FUNC_END()

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
