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
;** 文   件   名: mipsContextAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2015 年 09 月 01 日
;**
;** 描        述: MIPS 体系架构上下文切换.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>
#include "mipsContextAsm.h"

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
    EXPORT_LABEL(archTaskCtxCopy)
#if LW_CFG_COROUTINE_EN > 0
    EXPORT_LABEL(archCrtCtxSwitch)
#endif
    EXPORT_LABEL(archIntCtxLoad)
    EXPORT_LABEL(archSigCtxLoad)

;/*********************************************************************************************************
;  当前 CPU 开始运行多任务执行
;  参数为当前 CPU 控制块, 即 A0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxStart)
    REG_L       T9 , 0(A0)                                              ;/*  获取当前 TCB 的 REG_CTX 地址*/

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archTaskCtxStart)

;/*********************************************************************************************************
;  线程切换
;  参数为当前 CPU 控制块, 即 A0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxSwitch)
    REG_L       T9 , 0(A0)                                              ;/*  获取当前 TCB 的 REG_CTX 地址*/

    SAVE_REGS                                                           ;/*  保存所有寄存器              */

#if LW_CFG_SMP_EN > 0
    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    REG_S       A0 , ARCH_STK_OFF_VAR(0)(SP)                            ;/*  当前 CPU 信息入栈           */
    JAL         _SchedSafeStack                                         ;/*  _SchedSafeStack();          */
    NOP
    REG_L       A0 , ARCH_STK_OFF_VAR(0)(SP)                            ;/*  当前 CPU 信息出栈           */
    MOV         SP , V0
#endif

    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    REG_S       A0 , ARCH_STK_OFF_VAR(0)(SP)                            ;/*  当前 CPU 信息入栈           */
    JAL         _SchedSwp                                               ;/*  _SchedSwp();                */
    NOP
    REG_L       A0 , ARCH_STK_OFF_VAR(0)(SP)                            ;/*  当前 CPU 信息出栈           */
    REG_ADDU    SP , ARCH_STK_VAR_SIZE

    REG_L       T9 , 0(A0)                                              ;/*  获取当前 TCB 的 REG_CTX 地址*/

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archTaskCtxSwitch)

;/*********************************************************************************************************
;  协程切换
;  参数为当前 CPU 控制块, 即 A0 为当前 CPU 控制块指针
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    REG_L       T9 , (2 * PTRSIZE)(A0)                                  ;/*  当前 CCB 的 REG_CTX 地址    */

    SAVE_REGS                                                           ;/*  保存所有寄存器              */

    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    REG_S       A0 , ARCH_STK_OFF_VAR(0)(SP)                            ;/*  当前 CPU 信息入栈           */
    JAL         _SchedCrSwp                                             ;/*  _SchedCrSwp();              */
    NOP
    REG_L       A0 , ARCH_STK_OFF_VAR(0)(SP)                            ;/*  当前 CPU 信息出栈           */
    REG_ADDU    SP , ARCH_STK_VAR_SIZE

    REG_L       T9 , (2 * PTRSIZE)(A0)                                  ;/*  当前 CCB 的 REG_CTX 地址    */

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archCrtCtxSwitch)
#endif

;/*********************************************************************************************************
;  中断返回时, 线程装载
;  参数为当前 CPU 控制块, 即 A0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    REG_L       T9 , 0(A0)                                              ;/*  获取当前 TCB 的 REG_CTX 地址*/

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archIntCtxLoad)

;/*********************************************************************************************************
;  信号上下文返回
;  参数为 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    MOV         T9 , A0                                                 ;/*  T9 = ARCH_REG_CTX 指针      */

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END(archSigCtxLoad)

;/*********************************************************************************************************
;  拷贝任务上下文
;  参数 A0 为目的 ARCH_REG_CTX 指针, A1 为源 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxCopy)
    REG_L       T0 , (0 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (0 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (1 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (1 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (2 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (2 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (3 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (3 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (4 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (4 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (5 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (5 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (6 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (6 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (7 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (7 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (8 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (8 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (9 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (9 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (10 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (10 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (11 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (11 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (12 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (12 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (13 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (13 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (14 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (14 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (15 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (15 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (16 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (16 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (17 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (17 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (18 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (18 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (19 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (19 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (20 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (20 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (21 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (21 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (22 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (22 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (23 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (23 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (24 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (24 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (25 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (25 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (26 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (26 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (27 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (27 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (28 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (28 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (29 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (29 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (30 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (30 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (31 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (31 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (32 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (32 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (33 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (33 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (34 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (34 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (35 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (35 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (36 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (36 * ARCH_REG_SIZE)(A0)

    REG_L       T0 , (37 * ARCH_REG_SIZE)(A1)
    REG_S       T0 , (37 * ARCH_REG_SIZE)(A0)

    JR          RA
    NOP
    FUNC_END(archTaskCtxCopy)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
