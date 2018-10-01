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
;** 文   件   名: armExcV7MAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2017 年 11 月 14 日
;**
;** 描        述: ARMv7M 体系构架异常处理.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>

#include "arch/arm/arch_regs.h"

#if defined(__SYLIXOS_ARM_ARCH_M__)

    FILE_BEGIN()

;/*********************************************************************************************************
;  ARMv7M 体系构架相关异常处理函数
;*********************************************************************************************************/

    IMPORT_LABEL(API_ThreadTcbInter)
    IMPORT_LABEL(API_InterStackBaseGet)
    IMPORT_LABEL(API_InterEnter)
    IMPORT_LABEL(API_InterExit)
    IMPORT_LABEL(API_InterExitNoSched)
    IMPORT_LABEL(_CpuGetNesting)

#if LW_CFG_SMP_EN > 0
    IMPORT_LABEL(_SchedSafeStack)
#endif

    IMPORT_LABEL(archResetHandle)
    IMPORT_LABEL(armv7mNMIIntHandle)
    IMPORT_LABEL(armv7mHardFaultHandle)
    IMPORT_LABEL(armv7mMemFaultHandle)
    IMPORT_LABEL(armv7mBusFaultHandle)
    IMPORT_LABEL(armv7mUsageFaultHandle)
    IMPORT_LABEL(armv7mDebugMonitorEntry)
    IMPORT_LABEL(armv7mPendSVEntry)
    IMPORT_LABEL(armv7mSysTickIntEntry)
    IMPORT_LABEL(armv7mSvcHandle)
    IMPORT_LABEL(armv7mReservedIntHandle)
    IMPORT_LABEL(armv7mIntHandle)

    EXPORT_LABEL(archNMIIntEntry)
    EXPORT_LABEL(archHardFaultEntry)
    EXPORT_LABEL(archMemFaultEntry)
    EXPORT_LABEL(archBusFaultEntry)
    EXPORT_LABEL(archUsageFaultEntry)
    EXPORT_LABEL(archDebugMonitorEntry)
    EXPORT_LABEL(archPendSVEntry)
    EXPORT_LABEL(archSysTickIntEntry)
    EXPORT_LABEL(archSvcEntry)
    EXPORT_LABEL(archReservedIntEntry)
    EXPORT_LABEL(vector)

;/*********************************************************************************************************
;  这里的异常向量表 vector 声明为弱符号, BSP 可以重新实现异常向量表
;*********************************************************************************************************/

    WEAK(vector)

;/*********************************************************************************************************
;  异常/中断入口宏
;*********************************************************************************************************/

;/*********************************************************************************************************
;                                              IRQ_ENTRY
;                                                  +
;                                                  |
;                                                  |
;                                                  |
;                                                  v
;                                           API_InterEnter
;                                                  +
;                                                  |  中断嵌套
;                                                  |
;                           关中断前               +--------------------------+
;                           或                     |                          |
;                           任务切换时发生强占     |                          |
;                         +------------------------+                          v
;                         |                        |                    SAVE_CTX_REG
;                         |                        v                          +
;                         |               API_InterStackBaseGet               |
;                         |                        +                          |
;                         v                        |                          v
;                   SAVE_CTX_REG                   |                     ISR_HANDLE
;                         +                        v                          +
;                         |                API_ThreadTcbInter                 |
;                         |                        +                          |
;                         v                        |                          |
;                    ISR_HANDLE                    |                          v
;                         +                        v                    API_InterExit
;                         |                  SAVE_CTX_REG                     +
;                         |                        +                          |
;                         |                        |                          |
;                         v                        |                          |
;               API_InterExitNoSched               v                          v
;                         +                   ISR_HANDLE                LOAD_CTX_REG
;                         |                        +
;                         |                        |
;                         v                        |
;                    SEND PendSV                   v
;                         +                  API_InterExit
;                         |
;                         |
;                         |
;                         v
;                   LOAD_CTX_REG
;*********************************************************************************************************/

MACRO_DEF(IRQ_ENTRY  irq  handle)
    ;/*
    ; * 异常会自动将 xPSR, PC, LR, R12 以及 R3-R0 依次压入适当的堆栈中：
    ; * 如果异常前的代码正在使用 PSP, 则压入 PSP, 也就是使用进程堆栈；
    ; * 否则就压入 MSP, 使用主堆栈.
    ; * 一旦进入了服务例程, 就将一直使用主堆栈
    ; */

    ;/*
    ; * R1 记录 BASEPRI, 并关中断(利用优先级屏蔽中断, 才能使用 SVC)
    ; */
    CPSID   I
    MRS     R1 , BASEPRI                                        ;/*  Set prio int mask to mask all       */
    MOV     R2 , #LW_CFG_CORTEX_M_INT_MASK                      ;/*  mask prio >= 2                      */
    MSR     BASEPRI , R2
    CPSIE   I

    ;/*
    ; * R0 等于异常自动压栈前使用的 SP, 通过 EXC_RETURN(保存在 LR) 判断异常前使用何种 SP
    ; */
    TST     LR , #(CORTEX_M_EXC_RETURN_MODE_MASK)               ;/*  nonzero if context on process stack */
    ITE     EQ                                                  ;/*  next two instructions conditional   */
    MRSEQ   R0 , MSP
    MRSNE   R0 , PSP
    ADD     R0 , R0 , #(ARCH_HW_SAVE_CTX_SIZE)

    ;/*
    ; * 现在用 MSP, 如果发生中断嵌套, 需要保存寄存器上下文到 MSP 指向的栈空间里, 预留 SW SAVE 寄存器上下文空间
    ; */
    MRS     R2 , MSP
    SUB     R2 , R2 , #(ARCH_SW_SAVE_CTX_SIZE)
    MSR     MSP, R2

    PUSH    {LR}

    ;/*
    ; * API_InterEnter(异常自动压栈前使用的 SP, BASEPRI, EXC_RETURN) 进入中断,
    ; * 如果是第一次中断,   会将 异常前 SP, BASEPRI, EXC_RETURN 保存到当前任务 TCB 的 ARCH_REG_CTX 里
    ; * 如果不是第一次中断, 会将 异常前 SP, BASEPRI, EXC_RETURN 保存到上面预留的栈空间里
    ; */
    MOV     R2 , LR
    LDR     R3 , =API_InterEnter
    BLX     R3

    ;/*
    ; * 如果不是第一次进入中断, 那么上一次中断已经设置 MSP
    ; */
    CMP     R0 , #1
    BNE     2f

    ;/*
    ; * 这里可以使用 R12，因为硬件自动保存了
    ; * 判断是否在关中断前或者任务切换时发生抢占
    ; */
    POP     {R12}
    TST     R12 , #(CORTEX_M_EXC_RETURN_MODE_MASK)
    BEQ     1f

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并设置 MSP
    ; */
    LDR     R3 , =API_InterStackBaseGet
    BLX     R3
    MSR     MSP , R0

    ;/*
    ; * 获得当前任务 TCB 的 ARCH_REG_CTX 地址
    ; */
    LDR     R3 , =API_ThreadTcbInter
    BLX     R3

    ;/*
    ; * 保存 R4-R11 到当前任务 TCB 的 ARCH_REG_CTX 里
    ; */
    ADD     R1 , R0 , #(2 * 4)                                  ;/*  跳过 SP, BASEPRI 的保存             */
    STM     R1 , {R4-R11}                                       ;/*  保存 R4-R11, 跳过 EXC_RETURN 的保存 */

    LDR     R1 , [R0]                                           ;/*  R1 = 异常自动压栈前使用的 SP        */
    SUB     R1 , R1 , #(ARCH_HW_SAVE_CTX_SIZE)                  ;/*  R1 = 异常自动压栈后使用的 SP        */

    ADD     R2 , R0 , #(ARCH_SW_SAVE_CTX_SIZE)

    LDM     R1 , {R4-R11}
    STM     R2 , {R4-R11}

    MOV     R1 , R0
3:
    ;/*
    ; * handle(vector, 寄存器上下文)
    ; */
    MOV     R0 , \irq
    LDR     R3 , =\handle
    BLX     R3

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数,
    ; * 注意 MSP 并没有调整, 但不会有问题(因为下次进入中断时 MSP 会首先使用异常完全压栈后的 SP)
    ; */
    LDR     R3 , =API_InterExit
    BLX     R3

    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    ;/*
    ; * 从 MSP 指向的堆栈恢复原 SP, BASEPRI, R4-R11, EXC_RETURN(保存在 LR)寄存器
    ; */
    MRS     R0 , MSP
    LDM     R0 , {R2-R11, LR}
    ADD     R0 , R0 , #(ARCH_SW_SAVE_CTX_SIZE)
    MSR     MSP , R0

    MSR     BASEPRI , R3

    ;/*
    ; * 从 MSP 指向的堆栈恢复 xPSR, PC, LR, R12 以及 R3-R0 寄存器
    ; */
    BX      LR

2:
    POP     {R12}
    ;/*
    ; * 中断嵌套
    ; */
    MRS     R1 , MSP
    ADD     R0 , R1 , #(2 * 4)                                  ;/*  跳过 SP, BASEPRI 的保存             */
    STM     R0 , {R4-R11}                                       ;/*  保存 R4-R11, 跳过 EXC_RETURN 的保存 */

    B       3b

1:
    ;/*
    ; * 来到这里，说明在关中断前或者任务切换时发生抢占
    ; */
    MRS     R1 , MSP
    ADD     R0 , R1 , #(2 * 4)                                  ;/*  跳过 SP, BASEPRI 的保存             */
    STM     R0 , {R4-R11}                                       ;/*  保存 R4-R11, 跳过 EXC_RETURN 的保存 */

    ;/*
    ; * handle(vector, 寄存器上下文)
    ; */
    MOV     R0 , \irq
    LDR     R3 , =\handle
    BLX     R3

    LDR     R3 , =API_InterExitNoSched
    BLX     R3

    ;/*
    ; * 从 MSP 指向的堆栈恢复原 SP, BASEPRI, R4-R11, EXC_RETURN(保存在 LR)寄存器
    ; */
    MRS     R0 , MSP
    LDM     R0 , {R2-R11, LR}
    ADD     R0 , R0 , #(ARCH_SW_SAVE_CTX_SIZE)
    MSR     MSP , R0

    ;/*
    ; * Send PendSV
    ; */
    LDR     R0 , =(CORTEX_M_ICSR)
    LDR     R1 , =(CORTEX_M_PENDSV_SET_MASK)
    STR     R1 , [R0]

    MSR     BASEPRI , R3
    ;/*
    ; * 从 MSP 指向的堆栈恢复 xPSR, PC, LR, R12 以及 R3-R0 寄存器
    ; */
    BX      LR
    MACRO_END()

;/*********************************************************************************************************
;  NMI 中断入口函数
;*********************************************************************************************************/

FUNC_DEF(archNMIIntEntry)
    ;/*
    ; * R0 用于记录异常号
    ; */
    IRQ_ENTRY   2   armv7mNMIIntHandle
    FUNC_END()

;/*********************************************************************************************************
;  Hard Fault 入口函数
;*********************************************************************************************************/

FUNC_DEF(archHardFaultEntry)
    IRQ_ENTRY   3   armv7mHardFaultHandle
    FUNC_END()

;/*********************************************************************************************************
;  Mem Fault 入口函数
;*********************************************************************************************************/

FUNC_DEF(archMemFaultEntry)
    IRQ_ENTRY   4   armv7mMemFaultHandle
    FUNC_END()

;/*********************************************************************************************************
;  Bus Fault 入口函数
;*********************************************************************************************************/

FUNC_DEF(archBusFaultEntry)
    IRQ_ENTRY   5   armv7mBusFaultHandle
    FUNC_END()

;/*********************************************************************************************************
;  Usage Fault 入口函数
;*********************************************************************************************************/

FUNC_DEF(archUsageFaultEntry)
    IRQ_ENTRY   6   armv7mUsageFaultHandle
    FUNC_END()

;/*********************************************************************************************************
;  Debug Monitor 入口函数
;*********************************************************************************************************/

FUNC_DEF(archDebugMonitorEntry)
    IRQ_ENTRY   12  armv7mDebugMonitorHandle
    FUNC_END()

;/*********************************************************************************************************
;  SysTick 中断入口函数
;*********************************************************************************************************/

FUNC_DEF(archSysTickIntEntry)
    IRQ_ENTRY   15  armv7mSysTickIntHandle
    FUNC_END()

;/*********************************************************************************************************
;  Reserved 中断入口函数
;*********************************************************************************************************/

FUNC_DEF(archReservedIntEntry)
    IRQ_ENTRY   7   armv7mReservedIntHandle
    FUNC_END()

;/*********************************************************************************************************
;  16 ~ 255 号中断入口函数
;*********************************************************************************************************/

    .irp        Id,                                   \
                                   16,  17,  18,  19, \
     20,  21,  22,  23,  24,  25,  26,  27,  28,  29, \
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39, \
     40,  41,  42,  43,  44,  45,  46,  47,  48,  49, \
     50,  51,  52,  53,  54,  55,  56,  57,  58,  59, \
     60,  61,  62,  63,  64,  65,  66,  67,  68,  69, \
     70,  71,  72,  73,  74,  75,  76,  77,  78,  79, \
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89, \
     90,  91,  92,  93,  94,  95,  96,  97,  98,  99, \
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, \
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119, \
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, \
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139, \
    140, 141, 142, 143, 144, 145, 146, 147, 148, 149, \
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, \
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, \
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179, \
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, \
    190, 191, 192, 193, 194, 195, 196, 197, 198, 199, \
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209, \
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, \
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, \
    230, 231, 232, 233, 234, 235, 236, 237, 238, 239, \
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, \
    250, 251, 252, 253, 254, 255
    EXPORT_LABEL(archIntEntry\Id)
FUNC_DEF(archIntEntry\Id)
    IRQ_ENTRY   \Id   armv7mIntHandle
    FUNC_END()
    .endr

;/*********************************************************************************************************
;  SVC 入口函数
;*********************************************************************************************************/

FUNC_DEF(archSvcEntry)
    ;/*
    ; * R0 用于记录异常号
    ; */

    ;/*
    ; * 异常会自动将 xPSR, PC, LR, R12 以及 R3-R0 依次压入适当的堆栈中：
    ; * 如果异常前的代码正在使用 PSP, 则压入 PSP, 也就是使用进程堆栈；
    ; * 否则就压入 MSP, 使用主堆栈.
    ; * 一旦进入了服务例程, 就将一直使用主堆栈
    ; */

    ;/*
    ; * R3 记录 BASEPRI, 并关中断(利用优先级屏蔽中断, 才能使用 SVC)
    ; */
    MRS     R3 , BASEPRI                                        ;/*  Set prio int mask to mask all       */
    MOV     R2 , #LW_CFG_CORTEX_M_INT_MASK                      ;/*  mask prio >= 2                      */
    MSR     BASEPRI , R2

    ;/*
    ; * R0 等于异常自动压栈后使用的 SP
    ; * R2 等于异常自动压栈前使用的 SP, 通过 EXC_RETURN(保存在 LR) 判断异常前使用何种 SP
    ; */
    TST     LR , #(CORTEX_M_EXC_RETURN_MODE_MASK)               ;/*  nonzero if context on process stack */
    ITE     EQ                                                  ;/*  next two instructions conditional   */
    MRSEQ   R0 , MSP
    MRSNE   R0 , PSP
    ADD     R2 , R0 , #(ARCH_HW_SAVE_CTX_SIZE)

    ;/*
    ; * 现在用 MSP, 需要保存寄存器上下文到 MSP 指向的栈空间里, 预留 SW SAVE 寄存器上下文空间
    ; */
    MRS     R1 , MSP
    SUB     R1 , R1 , #(ARCH_SW_SAVE_CTX_SIZE)
    MSR     MSP , R1

    ;/*
    ; * 保存原 SP, BASEPRI, R4-R11, EXC_RETURN(保存在 LR)寄存器到预留 SW SAVE 寄存器上下文空间
    ; */
    STM     R1 , {R2-R11, LR}

    ;/*
    ; * armv7mSvcHandle(HW SAVE 寄存器上下文空间, SW SAVE 寄存器上下文空间)
    ; */
    LDR     R3 , =armv7mSvcHandle
    BLX     R3

    MRS     R1 , MSP
    ADD     R1 , R1 , #(ARCH_SW_SAVE_CTX_SIZE)
    MSR     MSP , R1

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
;  PendSV 入口函数
;*********************************************************************************************************/

FUNC_DEF(archPendSVEntry)
    ;/*
    ; * 异常会自动将 xPSR, PC, LR, R12 以及 R3-R0 依次压入适当的堆栈中：
    ; * 如果异常前的代码正在使用 PSP, 则压入 PSP, 也就是使用进程堆栈；
    ; * 否则就压入 MSP, 使用主堆栈.
    ; * 一旦进入了服务例程, 就将一直使用主堆栈
    ; */

    ;/*
    ; * R1 记录 BASEPRI, 并关中断(利用优先级屏蔽中断, 才能使用 SVC)
    ; */
    CPSID   I
    MRS     R1 , BASEPRI                                        ;/*  Set prio int mask to mask all       */
    MOV     R2 , #LW_CFG_CORTEX_M_INT_MASK                      ;/*  mask prio >= 2                      */
    MSR     BASEPRI , R2
    CPSIE   I

    ;/*
    ; * R0 等于异常自动压栈前使用的 SP, 通过 EXC_RETURN(保存在 LR) 判断异常前使用何种 SP
    ; */
    TST     LR , #(CORTEX_M_EXC_RETURN_MODE_MASK)               ;/*  nonzero if context on process stack */
    ITE     EQ                                                  ;/*  next two instructions conditional   */
    MRSEQ   R0 , MSP
    MRSNE   R0 , PSP
    ADD     R0 , R0 , #(ARCH_HW_SAVE_CTX_SIZE)

    ;/*
    ; * API_InterEnter(异常自动压栈前使用的 SP, BASEPRI, EXC_RETURN) 进入中断,
    ; * 如果是第一次中断,   会将 异常前 SP, BASEPRI, EXC_RETURN 保存到当前任务 TCB 的 ARCH_REG_CTX 里
    ; * 如果不是第一次中断, 会将 异常前 SP, BASEPRI, EXC_RETURN 保存到上面预留的栈空间里
    ; */
    MOV     R2 , LR
    LDR     R3 , =API_InterEnter
    BLX     R3

    ;/*
    ; * 获得当前任务 TCB 的 ARCH_REG_CTX 地址
    ; */
    LDR     R3 , =API_ThreadTcbInter
    BLX     R3

    ;/*
    ; * 保存 R4-R11 到当前任务 TCB 的 ARCH_REG_CTX 里
    ; */
    ADD     R1 , R0 , #(2 * 4)                                  ;/*  跳过 SP, BASEPRI 的保存             */
    STM     R1 , {R4-R11}                                       ;/*  保存 R4-R11, 跳过 EXC_RETURN 的保存 */

    LDR     R1 , [R0]                                           ;/*  R1 = 异常自动压栈前使用的 SP        */
    SUB     R1 , R1 , #(ARCH_HW_SAVE_CTX_SIZE)                  ;/*  R1 = 异常自动压栈后使用的 SP        */

    ADD     R2 , R0 , #(ARCH_SW_SAVE_CTX_SIZE)

    LDM     R1 , {R4-R11}
    STM     R2 , {R4-R11}

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并设置 MSP
    ; */
    LDR     R3 , =API_InterStackBaseGet
    BLX     R3
    MSR     MSP , R0

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数,
    ; * 注意 MSP 并没有调整, 但不会有问题(因为下次进入中断时 MSP 会首先使用异常完全压栈后的 SP)
    ; */
    LDR     R3 , =API_InterExit
    BLX     R3

    ;/*
    ; * 无法执行到这里，任务切换
    ; */
    BX      LR
    FUNC_END()

;/*********************************************************************************************************
;  异常向量表
;*********************************************************************************************************/

    SECTION(.vector)

FUNC_DEF(vector)
    .long   __stack_end                                         ;/*  0  - Reset stack pointer            */
    .long   archResetHandle                                     ;/*  1  - Reset                          */
    .long   archNMIIntEntry                                     ;/*  2  - NMI                            */
    .long   archHardFaultEntry                                  ;/*  3  - HardFault                      */
    .long   archMemFaultEntry                                   ;/*  4  - MemManage                      */
    .long   archBusFaultEntry                                   ;/*  5  - BusFault                       */
    .long   archUsageFaultEntry                                 ;/*  6  - UsageFault                     */
    .long   archReservedIntEntry                                ;/*  7  - Reserved                       */
    .long   archReservedIntEntry                                ;/*  8  - Reserved                       */
    .long   archReservedIntEntry                                ;/*  9  - Reserved                       */
    .long   archReservedIntEntry                                ;/*  10 - Reserved                       */
    .long   archSvcEntry                                        ;/*  11 - SVCall                         */
    .long   archDebugMonitorEntry                               ;/*  12 - Debug Monitor                  */
    .long   archReservedIntEntry                                ;/*  13 - Reserved                       */
    .long   archPendSVEntry                                     ;/*  14 - PendSV                         */
    .long   archSysTickIntEntry                                 ;/*  15 - SysTick                        */
    .irp    Id,                                       \
                                   16,  17,  18,  19, \
     20,  21,  22,  23,  24,  25,  26,  27,  28,  29, \
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39, \
     40,  41,  42,  43,  44,  45,  46,  47,  48,  49, \
     50,  51,  52,  53,  54,  55,  56,  57,  58,  59, \
     60,  61,  62,  63,  64,  65,  66,  67,  68,  69, \
     70,  71,  72,  73,  74,  75,  76,  77,  78,  79, \
     80,  81,  82,  83,  84,  85,  86,  87,  88,  89, \
     90,  91,  92,  93,  94,  95,  96,  97,  98,  99, \
    100, 101, 102, 103, 104, 105, 106, 107, 108, 109, \
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119, \
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, \
    130, 131, 132, 133, 134, 135, 136, 137, 138, 139, \
    140, 141, 142, 143, 144, 145, 146, 147, 148, 149, \
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, \
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, \
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179, \
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, \
    190, 191, 192, 193, 194, 195, 196, 197, 198, 199, \
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209, \
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, \
    220, 221, 222, 223, 224, 225, 226, 227, 228, 229, \
    230, 231, 232, 233, 234, 235, 236, 237, 238, 239, \
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, \
    250, 251, 252, 253, 254, 255
    .long   archIntEntry\Id
    .endr
    FUNC_END()

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
