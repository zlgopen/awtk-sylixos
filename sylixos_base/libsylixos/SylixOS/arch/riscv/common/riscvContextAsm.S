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
;** 文   件   名: riscvContextAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2018 年 03 月 20 日
;**
;** 描        述: RISC-V 体系构架上下文切换.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>
#include "riscvContextAsm.h"

    FILE_BEGIN()

#if LW_CFG_SMP_EN > 0
    IMPORT_LABEL(_SchedSafeStack)
#endif
    IMPORT_LABEL(_SchedSwp)
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
    EXPORT_LABEL(archTaskCtxCopy)

;/*********************************************************************************************************
;  当前 CPU 开始运行多任务执行
;  参数为当前 CPU 控制块, 即 A0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxStart)
    REG_L   T0 , 0(A0)                                                  ;/*  获取当前 TCB 的 REG_CTX 地址*/

    RESTORE_SMALL_REG_CTX                                               ;/*  恢复小寄存器上下文          */
    FUNC_END(archTaskCtxStart)

;/*********************************************************************************************************
;  线程切换
;  参数为当前 CPU 控制块, 即 A0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxSwitch)
    REG_L   T0 , 0(A0)                                                  ;/*  获取当前 TCB 的 REG_CTX 地址*/

    SAVE_SMALL_REG_CTX                                                  ;/*  保存小寄存器上下文          */

    MOVE    S0 , A0                                                     ;/*  S0 暂存 A0                  */

#if LW_CFG_SMP_EN > 0
    CALL    _SchedSafeStack                                             ;/*  SP = _SchedSafeStack();     */
    MOVE    SP , RV0
    MOVE    A0 , S0                                                     ;/*  恢复 A0                     */
#endif

    CALL    _SchedSwp                                                   ;/*  _SchedSwp();                */

    REG_L   T0 , 0(S0)                                                  ;/*  获取当前 TCB 的 REG_CTX 地址*/
    REG_L   T1 , CTXTYPE_OFFSET(T0)                                     ;/*  获得上下文类型              */
    BNEZ    T1 , _RestoreSmallCtx

    RESTORE_BIG_REG_CTX                                                 ;/*  恢复大寄存器上下文          */

LINE_LABEL(_RestoreSmallCtx)
    RESTORE_SMALL_REG_CTX                                               ;/*  恢复小寄存器上下文          */
    FUNC_END(archTaskCtxSwitch)

;/*********************************************************************************************************
;  协程切换
;  参数为当前 CPU 控制块, 即 A0 为当前 CPU 控制块指针
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    REG_L   T0 , 0(A0)                                                  ;/*  获取当前 CCB 的 REG_CTX 地址*/

    SAVE_SMALL_REG_CTX                                                  ;/*  保存小寄存器上下文          */

    MOVE    S0 , A0                                                     ;/*  S0 暂存 A0                  */

    CALL    _SchedCrSwp                                                 ;/*  _SchedCrSwp();              */

    REG_L   T0 , 0(S0)                                                  ;/*  获取当前 CCB 的 REG_CTX 地址*/
    RESTORE_SMALL_REG_CTX                                               ;/*  恢复小寄存器上下文          */
    FUNC_END(archCrtCtxSwitch)
#endif

;/*********************************************************************************************************
;  中断返回时, 线程装载
;  参数为当前 CPU 控制块, 即 A0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    REG_L   T0 , 0(A0)                                                  ;/*  获取当前 CCB 的 REG_CTX 地址*/
    REG_L   T1 , CTXTYPE_OFFSET(T0)                                     ;/*  获得上下文类型              */
    BNEZ    T1 , _RestoreSmallCtx

    RESTORE_BIG_REG_CTX                                                 ;/*  恢复大寄存器上下文          */
    FUNC_END(archIntCtxLoad)

;/*********************************************************************************************************
;  信号上下文返回
;  参数为 ARCH_REG_CTX 指针, 即 A0 为 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    MOVE    T0 , A0
    REG_L   T1 , CTXTYPE_OFFSET(T0)                                     ;/*  获得上下文类型              */
    BNEZ    T1 , _RestoreSmallCtx

    RESTORE_BIG_REG_CTX                                                 ;/*  恢复大寄存器上下文          */
    FUNC_END(archSigCtxLoad)

;/*********************************************************************************************************
;  拷贝任务上下文
;  参数 A0 为目的 ARCH_REG_CTX 指针, A1 为源 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxCopy)
    REG_L   T0 , ( 0 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 0 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 1 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 1 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 2 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 2 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 3 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 3 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 4 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 4 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 5 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 5 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 6 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 6 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 7 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 7 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 8 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 8 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , ( 9 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , ( 9 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (10 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (10 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (11 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (11 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (12 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (12 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (13 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (13 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (14 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (14 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (15 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (15 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (16 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (16 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (17 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (17 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (18 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (18 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (19 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (19 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (20 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (20 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (21 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (21 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (22 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (22 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (23 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (23 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (24 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (24 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (25 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (25 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (26 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (26 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (27 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (27 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (28 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (28 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (29 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (29 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (30 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (30 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (31 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (31 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (32 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (32 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (33 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (33 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (34 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (34 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (35 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (35 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (36 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (36 * ARCH_REG_SIZE)(A0)

    REG_L   T0 , (37 * ARCH_REG_SIZE)(A1)
    REG_S   T0 , (37 * ARCH_REG_SIZE)(A0)

    RET
    FUNC_END(archTaskCtxCopy)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
