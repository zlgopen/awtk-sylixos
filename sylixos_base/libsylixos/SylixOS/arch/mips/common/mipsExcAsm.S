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
;** 文   件   名: mipsExcAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2015 年 09 月 01 日
;**
;** 描        述: MIPS 体系架构异常处理.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include "mipsContextAsm.h"

    FILE_BEGIN()

    IMPORT_LABEL(API_ThreadTcbInter)
    IMPORT_LABEL(API_InterStackBaseGet)
    IMPORT_LABEL(API_InterEnter)
    IMPORT_LABEL(API_InterExit)

    IMPORT_LABEL(bspIntHandle)
    IMPORT_LABEL(archCacheErrorHandle)
    IMPORT_LABEL(archExceptionHandle)

    EXPORT_LABEL(archInterruptEntry)
    EXPORT_LABEL(archCacheErrorEntry)
    EXPORT_LABEL(archExceptionEntry)

;/*********************************************************************************************************
;  用于临时保存异常上下文
;  如果有 RDHWR 指令, 可以通过 RDHWR 指令获得 CPU NUM,
;  所以每个处理器逻辑核的临时异常上下文是相互独立的, 故不用加锁访问, 以提高实时性
;*********************************************************************************************************/

#define ARCH_TEMP_CTX_SIZE      4096
#define XLOCK                   4000

#if LW_CFG_MIPS_HAS_RDHWR_INSTR > 0
MACRO_DEF(QUAD_VALUE from, to)
    .quad       ((\from) * ARCH_TEMP_CTX_SIZE + archExcTempCtxMp)
    .if         \to-\from
    QUAD_VALUE  "(\from+1)",\to
    .endif
    MACRO_END()

    SECTION(.data)
    .balign     ARCH_STK_ALIGN_SIZE
LINE_LABEL(archExcTempCtx)
    QUAD_VALUE  0 , (LW_CFG_MAX_PROCESSORS - 1)

    SECTION(.bss)
    .balign     ARCH_STK_ALIGN_SIZE
LINE_LABEL(archExcTempCtxMp)
    .space      (ARCH_TEMP_CTX_SIZE * LW_CFG_MAX_PROCESSORS)
#else
    SECTION(.bss)
    .balign     ARCH_STK_ALIGN_SIZE
LINE_LABEL(archExcTempCtx)
    .space      ARCH_TEMP_CTX_SIZE
#endif

;/*********************************************************************************************************
;  异常入口宏
;*********************************************************************************************************/

MACRO_DEF(EXCEPTION_ENTRY handle)
    .set    push
    .set    noat

    ;/*
    ; * 因为这些都是内核代码，不经 TLB 转换，所以执行时不会发生 TLB 重填异常
    ; */

    ;/*
    ; * archExcTempCtx 在内核内存，不经 TLB 转换，所以访问时不会发生 TLB 重填异常
    ; */
1:
    PTR_LA      K0 , archExcTempCtx
#if LW_CFG_MIPS_HAS_RDHWR_INSTR > 0
    ;/*
    ; * 使用该核的 archExcTempCtx
    ; */
    .set        push
    .set        mips64r2
    RDHWR       K1 , HWR_CPUNUM
    .set        pop
    LONG_SLL    K1 , K1 , LONGLOG
    REG_ADDU    K1 , K1 , K0
    REG_L       K0 , 0(K1)
#else
    ;/*
    ; * 对 archExcTempCtx 进行加锁
    ; */
    LL          K1 , XLOCK(K0)
    BNEZ        K1 , 1b
    NOP

    LI          K1 , 1
    SC          K1 , XLOCK(K0)
    BEQZ        K1 , 1b
    SYNC
#endif

    ;/*
    ; * 保存寄存器到 archExcTempCtx
    ; */
    REG_S       $0  , XGREG(0)(K0)
    REG_S       $1  , XGREG(1)(K0)
    REG_S       $2  , XGREG(2)(K0)
    REG_S       $3  , XGREG(3)(K0)
    REG_S       $4  , XGREG(4)(K0)
    REG_S       $5  , XGREG(5)(K0)
    REG_S       $6  , XGREG(6)(K0)
    REG_S       $7  , XGREG(7)(K0)
    REG_S       $8  , XGREG(8)(K0)
    REG_S       $9  , XGREG(9)(K0)
    REG_S       $10 , XGREG(10)(K0)
    REG_S       $11 , XGREG(11)(K0)
    REG_S       $12 , XGREG(12)(K0)
    REG_S       $13 , XGREG(13)(K0)
    REG_S       $14 , XGREG(14)(K0)
    REG_S       $15 , XGREG(15)(K0)
    REG_S       $16 , XGREG(16)(K0)
    REG_S       $17 , XGREG(17)(K0)
    REG_S       $18 , XGREG(18)(K0)
    REG_S       $19 , XGREG(19)(K0)
    REG_S       $20 , XGREG(20)(K0)
    REG_S       $21 , XGREG(21)(K0)
    REG_S       $22 , XGREG(22)(K0)
    REG_S       $23 , XGREG(23)(K0)
    REG_S       $24 , XGREG(24)(K0)
    REG_S       $25 , XGREG(25)(K0)
    ;/*
    ; * $26 $27 是 K0 K1
    ; */
    REG_S       $28 , XGREG(28)(K0)
    REG_S       $29 , XGREG(29)(K0)
    REG_S       $30 , XGREG(30)(K0)
    REG_S       $31 , XGREG(31)(K0)

    MFC0_LONG_EHB(T1, CP0_EPC)
    REG_S       T1 , XEPC(K0)                                           ;/*  保存 EPC 寄存器             */

    MFC0_EHB(T2, CP0_STATUS)                                            ;/*  保存 STATUS 寄存器          */
    LI          T0 , ~(ST0_EXL | ST0_ERL)
    AND         T2 , T2, T0                                             ;/*  T2 等于异常前的 SR 寄存器   */
    REG_S       T2 , XSR(K0)

    MFC0_LONG_EHB(T1, CP0_BADVADDR)                                     ;/*  保存 BADVADDR 寄存器        */
    REG_S       T1 , XBADVADDR(K0)

    MFC0_EHB(T1, CP0_CAUSE)                                             ;/*  保存 CAUSE 寄存器           */
    REG_S       T1 , XCAUSE(K0)

    MFLO        T1                                                      ;/*  保存 LO 寄存器              */
    REG_S       T1 , XLO(K0)

    MFHI        T1                                                      ;/*  保存 HI 寄存器              */
    REG_S       T1 , XHI(K0)

    ;/*
    ; * 回到内核模式并关中断
    ; */
    LI          T0 , ~(ST0_IE | ST0_KSU)
    AND         T2 , T2, T0
    MTC0_EHB(T2, CP0_STATUS)

    ;/*
    ; * 已经清除 EXL 位，TLB 重填会在它自己的异常向量入口
    ; */

    ;/*
    ; * 重新设置 GP 寄存器
    ; */

    PTR_LA      GP , _gp

    ;/*
    ; * 以下的子函数调用不会破坏 S0 寄存器
    ; */
    MOV         S0 , K0
    REG_ADDU    SP , K0 , (ARCH_TEMP_CTX_SIZE - ARCH_STK_ALIGN_SIZE)

    ;/*
    ; * API_InterEnter(A0)
    ; */
    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    MOV         A0 , S0
    JAL         API_InterEnter
    NOP
    REG_ADDU    SP , ARCH_STK_VAR_SIZE

    LI          V1 , 1
    BNE         V0 , V1 , 3f
    NOP

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并设置 SP
    ; */
    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    JAL         API_InterStackBaseGet
    NOP
    MOV         SP , V0

    ;/*
    ; * 获得当前任务 TCB 的 ARCH_REG_CTX 地址
    ; */
    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    JAL         API_ThreadTcbInter
    NOP
    REG_ADDU    SP , ARCH_STK_VAR_SIZE
    MOV         A0 , V0

2:
    ;/*
    ; * 解锁 archExcTempCtx
    ; */
#if LW_CFG_MIPS_HAS_RDHWR_INSTR == 0
    SYNC
    REG_S       ZERO , XLOCK(S0)
#endif

    ;/*
    ; * handle(寄存器上下文)
    ; */
    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    JAL         \handle
    NOP

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数
    ; */
    JAL         API_InterExit
    NOP

    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    REG_ADDU    SP , ARCH_STK_VAR_SIZE

    MOV         T9 , SP

    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */

3:
    ;/*
    ; * 不是第一次进入中断
    ; */
    REG_L       SP , XGREG(29)(S0)
    REG_SUBU    SP , ARCH_REG_CTX_SIZE
    B           2b
    MOV         A0 , SP
    .set    pop
    MACRO_END()

;/*********************************************************************************************************
;  中断入口
;*********************************************************************************************************/

FUNC_DEF(archInterruptEntry)
    EXCEPTION_ENTRY     bspIntHandle
    FUNC_END(archInterruptEntry)

;/*********************************************************************************************************
;  CACHE 错误异常入口
;*********************************************************************************************************/

FUNC_DEF(archCacheErrorEntry)
    EXCEPTION_ENTRY     archCacheErrorHandle
    FUNC_END(archCacheErrorEntry)

;/*********************************************************************************************************
;  通用异常入口
;*********************************************************************************************************/

FUNC_DEF(archExceptionEntry)
    EXCEPTION_ENTRY     archExceptionHandle
    FUNC_END(archExceptionEntry)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
