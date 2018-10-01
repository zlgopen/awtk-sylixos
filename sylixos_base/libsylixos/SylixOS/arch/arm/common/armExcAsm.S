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
;** 文   件   名: armExcAsm.S
;**
;** 创   建   人: Han.Hui (韩辉)
;**
;** 文件创建日期: 2013 年 12 月 07 日
;**
;** 描        述: ARM 体系构架异常处理.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>

#include "arch/arm/arch_regs.h"

#if !defined(__SYLIXOS_ARM_ARCH_M__)

    FILE_BEGIN()

;/*********************************************************************************************************
;  ARM 体系构架相关异常处理函数
;*********************************************************************************************************/

    IMPORT_LABEL(API_ThreadTcbInter)
    IMPORT_LABEL(API_InterStackBaseGet)
    IMPORT_LABEL(API_InterEnter)
    IMPORT_LABEL(API_InterExit)
    
    IMPORT_LABEL(archAbtHandle)
    IMPORT_LABEL(archUndHandle)
    IMPORT_LABEL(archSwiHandle)
    IMPORT_LABEL(bspIntHandle)
    
    EXPORT_LABEL(archIntEntry)
    EXPORT_LABEL(archAbtEntry)
    EXPORT_LABEL(archPreEntry)
    EXPORT_LABEL(archUndEntry)
    EXPORT_LABEL(archSwiEntry)
    
;/*********************************************************************************************************
;  中断入口
;*********************************************************************************************************/

FUNC_DEF(archIntEntry)
    ;/*
    ; * 保存 REG 到 IRQ 模式栈空间(这里做了个必须成立的假设, 之前必须工作在 SYS 或 USR 模式)
    ; */
    SUB     LR , LR, #4                                                 ;/*  调整用于中断返回的 PC 值    */
    STMFD   SP!, {LR}                                                   ;/*  保存返回地址                */
    STMFD   SP!, {R0-R12}                                               ;/*  保存寄存器                  */
    MOV     R1 , SP
    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */
    STMFD   R1!, {SP}                                                   ;/*  保存 SP_sys                 */
    STMFD   R1 , {LR}                                                   ;/*  保存 LR_sys                 */
    MSR     CPSR_c, #(DIS_INT | IRQ32_MODE)                             ;/*  回到 IRQ 模式               */
    SUB     SP , SP , #(2 * 4)                                          ;/*  调整 SP_irq                 */
    MRS     R2 , SPSR
    STMFD   SP!, {R2}                                                   ;/*  保存 CPSR_sys               */
    
    ;/*
    ; * API_InterEnter(SP_irq), 如果是第一次中断, 会将 IRQ 模式栈空间的 ARCH_REG_CTX
    ; * 拷贝到当前任务 TCB 的 ARCH_REG_CTX 里
    ; */
    MOV     R0 , SP
    LDR     R1 , =API_InterEnter
    MOV     LR , PC
    BX      R1

    ;/*
    ; * 如果不是第一次进入中断, 那么上一次中断(工作在 SYS 模式)已经设置 SP_sys, 只需要回到 SYS 模式
    ; */
    CMP     R0 , #1
    BNE     1f

    ;/*
    ; * 第一次进入中断: 因为已经将 IRQ 模式栈空间的 ARCH_REG_CTX 拷贝到当前任务 TCB 的 ARCH_REG_CTX 里
    ; * 调整 SP_irq
    ; */
    ADD     SP , SP , #(ARCH_REG_CTX_SIZE)

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并回到 SYS 模式, 并设置 SP_sys
    ; */
    LDR     R0 , =API_InterStackBaseGet
    MOV     LR , PC
    BX      R0

    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */
    MOV     SP , R0                                                     ;/*  设置 SP_sys                 */

1:
    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式(不是多余的)   */

    ;/*
    ; * bspIntHandle()
    ; */
    LDR     R1 , =bspIntHandle
    MOV     LR , PC
    BX      R1
    
    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数, SP_irq 在上面已经调整好
    ; */
    LDR     R1 , =API_InterExit
    MOV     LR , PC
    BX      R1
    
    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    MSR     CPSR_c, #(DIS_INT | IRQ32_MODE)                             ;/*  回到 IRQ 模式               */

    MOV     R0 , SP
    LDMIA   R0!, {R2-R4}                                                ;/*  读取 CPSR LR SP             */
    ADD     SP , SP , #(ARCH_REG_CTX_SIZE)                              ;/*  调整 SP_irq                 */

    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */

    MOV     SP , R4                                                     ;/*  恢复 SP_sys                 */
    MOV     LR , R3                                                     ;/*  恢复 LR_sys                 */

    MSR     CPSR_c, #(DIS_INT | IRQ32_MODE)                             ;/*  回到 IRQ 模式               */
    MSR     SPSR_cxsf , R2
    LDMIA   R0 , {R0-R12, PC}^                                          ;/*  恢复包括 PC 的所有寄存器,   */
                                                                        ;/*  同时更新 CPSR               */
    FUNC_END()
    
;/*********************************************************************************************************
;  abort 入口 
;  data_abort     $retoff == 8
;  prefetch_abort $retoff == 4
;*********************************************************************************************************/

MACRO_DEF(ABT_ENTRY retoff)
    ;/*
    ; * 保存 REG 到 ABT 模式栈空间(这里做了个必须成立的假设, 之前必须工作在 SYS 或 USR 模式)
    ; */
    SUB     LR , LR, #\retoff                                           ;/*  调整用于中断返回的 PC 值    */
    STMFD   SP!, {LR}                                                   ;/*  保存返回地址                */
    STMFD   SP!, {R0-R12}                                               ;/*  保存寄存器                  */
    MOV     R1 , SP
    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */
    STMFD   R1!, {SP}                                                   ;/*  保存 SP_sys                 */
    STMFD   R1 , {LR}                                                   ;/*  保存 LR_sys                 */
    MSR     CPSR_c, #(DIS_INT | ABT32_MODE)                             ;/*  回到 ABT 模式               */
    SUB     SP , SP , #(2 * 4)                                          ;/*  调整 SP_abt                 */
    MRS     R2 , SPSR
    STMFD   SP!, {R2}                                                   ;/*  保存 CPSR_sys               */
    MOV     R8 , LR                                                     ;/*  中断返回的 PC 值 -> R8      */

    ;/*
    ; * API_InterEnter(SP_abt) 进入中断, 如果是第一次中断, 会将 ABT 模式栈空间的 ARCH_REG_CTX
    ; * 拷贝到当前任务 TCB 的 ARCH_REG_CTX 里
    ; */
    MOV     R0 , SP
    LDR     R1 , =API_InterEnter
    MOV     LR , PC
    BX      R1

    ;/*
    ; * 如果不是第一次进入中断, 那么上一次中断(工作在 SYS 模式)已经设置 SP_sys, 只需要回到 SYS 模式
    ; */
    CMP     R0 , #1
    BNE     1f

    ;/*
    ; * 第一次进入中断: 因为已经将 ABT 模式栈空间的 ARCH_REG_CTX 拷贝到当前任务 TCB 的 ARCH_REG_CTX 里
    ; * 调整 SP_abt
    ; */
    ADD     SP , SP , #(ARCH_REG_CTX_SIZE)

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并回到 SYS 模式, 并设置 SP_sys
    ; */
    LDR     R0 , =API_InterStackBaseGet
    MOV     LR , PC
    BX      R0

    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */
    MOV     SP , R0                                                     ;/*  设置 SP_sys                 */

1:
    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式(不是多余的)   */

    ;/*
    ; * archAbtHandle(中断返回的 PC 值, $retoff)
    ; */
    MOV     R0 , R8                                                     ;/*  R8(中断返回的 PC 值)        */
    MOV     R1 , #\retoff                                               ;/*  $retoff                     */
    LDR     R2 , =archAbtHandle
    MOV     LR , PC
    BX      R2

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数, SP_abt 在上面已经调整好
    ; */
    LDR     R1 , =API_InterExit
    MOV     LR , PC
    BX      R1

    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    MSR     CPSR_c, #(DIS_INT | ABT32_MODE)                             ;/*  回到 ABT 模式               */

    MOV     R0 , SP
    LDMIA   R0!, {R2-R4}                                                ;/*  读取 CPSR LR SP             */
    ADD     SP , SP , #(ARCH_REG_CTX_SIZE)                              ;/*  调整 SP_abt                 */

    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */

    MOV     SP , R4                                                     ;/*  恢复 SP_sys                 */
    MOV     LR , R3                                                     ;/*  恢复 LR_sys                 */

    MSR     CPSR_c, #(DIS_INT | ABT32_MODE)                             ;/*  回到 ABT 模式               */
    MSR     SPSR_cxsf , R2
    LDMIA   R0 , {R0-R12, PC}^                                          ;/*  恢复包括 PC 的所有寄存器,   */
                                                                        ;/*  同时更新 CPSR               */
    MACRO_END()
    
FUNC_DEF(archAbtEntry)
    ABT_ENTRY 8
    FUNC_END()

FUNC_DEF(archPreEntry)
    ABT_ENTRY 4
    FUNC_END()

;/*********************************************************************************************************
;  undefine 入口
;*********************************************************************************************************/

FUNC_DEF(archUndEntry)
    ;/*
    ; * 保存 REG 到 UND 模式栈空间(这里做了个必须成立的假设, 之前必须工作在 SYS 或 USR 模式)
    ; */
    SUB     SP , SP , #4                                                ;/*  预留保存返回地址的空间      */
    STMFD   SP!, {R0-R12}                                               ;/*  保存寄存器                  */

    MRS     R2 , SPSR                                                   ;/*  将 SPSR 拷贝到 R2           */
    TST     R2 , #0x20                                                  ;/*  是否为 Thumb 模式           */
    SUBNE   LR , LR, #2                                                 ;/*  计算 Thumb 模式返回位置     */
    SUBEQ   LR , LR, #4                                                 ;/*  计算 ARM 模式返回位置       */
    STR     LR , [SP , #(13 * 4)]                                       ;/*  保存返回地址                */

    MOV     R1 , SP
    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */
    STMFD   R1!, {SP}                                                   ;/*  保存 SP_sys                 */
    STMFD   R1 , {LR}                                                   ;/*  保存 LR_sys                 */
    MSR     CPSR_c, #(DIS_INT | UND32_MODE)                             ;/*  回到 UND 模式               */
    SUB     SP , SP , #(2 * 4)                                          ;/*  调整 SP_und                 */
    STMFD   SP!, {R2}                                                   ;/*  保存 CPSR_sys               */
    MOV     R8 , LR                                                     ;/*  中断返回的 PC 值 -> R8      */
    MOV     R9 , R2                                                     ;/*  中断时的 CPSR    -> R9      */

    ;/*
    ; * API_InterEnter(SP_und) 进入中断, 如果是第一次中断, 会将 UND 模式栈空间的 ARCH_REG_CTX
    ; * 拷贝到当前任务 TCB 的 ARCH_REG_CTX 里
    ; */
    MOV     R0 , SP
    LDR     R1 , =API_InterEnter
    MOV     LR , PC
    BX      R1

    ;/*
    ; * 如果不是第一次进入中断, 那么上一次中断(工作在 SYS 模式)已经设置 SP_sys, 只需要回到 SYS 模式
    ; */
    CMP     R0 , #1
    BNE     1f

    ;/*
    ; * 第一次进入中断: 因为已经将 UND 模式栈空间的 ARCH_REG_CTX 拷贝到当前任务 TCB 的 ARCH_REG_CTX 里
    ; * 调整 SP_und
    ; */
    ADD     SP , SP , #(ARCH_REG_CTX_SIZE)

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并回到 SYS 模式, 并设置 SP_sys
    ; */
    LDR     R0 , =API_InterStackBaseGet
    MOV     LR , PC
    BX      R0

    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */
    MOV     SP , R0                                                     ;/*  设置 SP_sys                 */

1:
    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式(不是多余的)   */

    ;/*
    ; * archUndHandle(中断返回的 PC 值, 中断时的 CPSR)
    ; */
    MOV     R0 , R8                                                     ;/*  R8(中断返回的 PC 值)        */
    MOV     R1 , R9                                                     ;/*  R9(中断时的 CPSR)           */
    LDR     R2 , =archUndHandle
    MOV     LR , PC
    BX      R2

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数, SP_und 在上面已经调整好
    ; */
    LDR     R1 , =API_InterExit
    MOV     LR , PC
    BX      R1

    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    MSR     CPSR_c, #(DIS_INT | UND32_MODE)                             ;/*  回到 UND 模式               */

    MOV     R0 , SP
    LDMIA   R0!, {R2-R4}                                                ;/*  读取 CPSR LR SP             */
    ADD     SP , SP , #(ARCH_REG_CTX_SIZE)                              ;/*  调整 SP_und                 */

    MSR     CPSR_c, #(DIS_INT | SYS32_MODE)                             ;/*  回到 SYS 模式               */

    MOV     SP , R4                                                     ;/*  恢复 SP_sys                 */
    MOV     LR , R3                                                     ;/*  恢复 LR_sys                 */

    MSR     CPSR_c, #(DIS_INT | UND32_MODE)                             ;/*  回到 UND 模式               */
    MSR     SPSR_cxsf , R2
    LDMIA   R0 , {R0-R12, PC}^                                          ;/*  恢复包括 PC 的所有寄存器,   */
                                                                        ;/*  同时更新 CPSR               */
    FUNC_END()
    
;/*********************************************************************************************************
;  swi 入口
;*********************************************************************************************************/

MACRO_DEF(SWI_ENTRY)
    STMFD   SP!, {R0-R12, LR}

    LDR     R0 , [LR, #-4]                                              ;/*  读取 SWI 指令               */
    BIC     R0 , R0, #0xff000000                                        ;/*  获得 SWI 指令号             */
    MOV     R1 , SP                                                     ;/*  获得参数表位置              */

    LDR     R5 , =archSwiHandle
    MOV     LR , PC
    BX      R5

    LDMFD   SP!, {R0-R12, PC}^                                          ;/*  SWI 返回                    */
    MACRO_END()

FUNC_DEF(archSwiEntry)
    SWI_ENTRY
    FUNC_END()

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/

