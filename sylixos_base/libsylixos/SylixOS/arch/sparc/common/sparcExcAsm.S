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
;** 文   件   名: sparExcAsm.S
;**
;** 创   建   人: Xu.Guizhou (徐贵洲)
;**
;** 文件创建日期: 2017 年 05 月 15 日
;**
;** 描        述: SPARC 体系构架异常处理.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>
#include "sparcContextAsm.h"

    FILE_BEGIN()
    
    IMPORT_LABEL(API_ThreadTcbInter)
    IMPORT_LABEL(API_InterStackBaseGet)
    IMPORT_LABEL(API_InterEnter)
    IMPORT_LABEL(API_InterExit)
    
    IMPORT_LABEL(archMpCur)
    IMPORT_LABEL(archCtxLoad)
    IMPORT_LABEL(_G_ulCpu)
    IMPORT_LABEL(_G_ulIntNesting)
    IMPORT_LABEL(_G_ulIntSafeStack)

    IMPORT_LABEL(archDataStoreErrHandle)
    IMPORT_LABEL(archInstAccessMmuMissHandle)
    IMPORT_LABEL(archInstAccessErrHandle)
    IMPORT_LABEL(archRRegAccessErrHandle)
    IMPORT_LABEL(archInstAccessExcHandle)
    IMPORT_LABEL(archPrivInstHandle)
    IMPORT_LABEL(archIllegalInstHandle)
    IMPORT_LABEL(archFpDisableHandle)
    IMPORT_LABEL(archCpDisableHandle)
    IMPORT_LABEL(archUnimplFlushHandle)
    IMPORT_LABEL(archWatchPointDectectHandle)
    IMPORT_LABEL(archMemAddrNoAlignHandle)
    IMPORT_LABEL(archFpExcHandle)
    IMPORT_LABEL(archCpExcHandle)
    IMPORT_LABEL(archDataAccessErrHandle)
    IMPORT_LABEL(archDataAccessMmuMissHandle)
    IMPORT_LABEL(archDataAccessExcHandle)
    IMPORT_LABEL(archTagOverFlowHandle)
    IMPORT_LABEL(archDivZeroHandle)
    IMPORT_LABEL(archTrapInstHandle)
    IMPORT_LABEL(archSysCallHandle)
    IMPORT_LABEL(bspIntHandle)

    EXPORT_LABEL(archDataStoreErrEntry)
    EXPORT_LABEL(archInstAccessMmuMissEntry)
    EXPORT_LABEL(archInstAccessErrEntry)
    EXPORT_LABEL(archRRegAccessErrEntry)
    EXPORT_LABEL(archInstAccessExcEntry)
    EXPORT_LABEL(archPrivInstEntry)
    EXPORT_LABEL(archIllegalInstEntry)
    EXPORT_LABEL(archFpDisableEntry)
    EXPORT_LABEL(archCpDisableEntry)
    EXPORT_LABEL(archUnimplFlushEntry)
    EXPORT_LABEL(archWatchPointDectectEntry)
    EXPORT_LABEL(archMemAddrNoAlignEntry)
    EXPORT_LABEL(archFpExcEntry)
    EXPORT_LABEL(archCpExcEntry)
    EXPORT_LABEL(archDataAccessErrEntry)
    EXPORT_LABEL(archDataAccessMmuMissEntry)
    EXPORT_LABEL(archDataAccessExcEntry)
    EXPORT_LABEL(archTagOverFlowEntry)
    EXPORT_LABEL(archDivZeroEntry)
    EXPORT_LABEL(archTrapInstEntry)
    EXPORT_LABEL(archSysCallEntry)
    EXPORT_LABEL(archIntEntry)

;/*********************************************************************************************************
;  参数寄存器
;*********************************************************************************************************/

#define PSR_ARG_REG         l0
#define PC_ARG_REG          l1
#define NPC_ARG_REG         l2
#define TRAP_TYPE_ARG_REG   l3

;/*********************************************************************************************************
;  异常保存寄存器
;*********************************************************************************************************/

MACRO_DEF(EXC_SAVE_REGS  arg)
    ST      %g1  , [%l4 + REG_GLOBAL(1)]                                ;/*  保存全局寄存器(除了 %g0)    */
    STD     %g2  , [%l4 + REG_GLOBAL(2)]
    SPARC_B2BST_NOP
    STD     %g4  , [%l4 + REG_GLOBAL(4)]
    SPARC_B2BST_NOP
    STD     %g6  , [%l4 + REG_GLOBAL(6)]
    SPARC_B2BST_NOP

    STD     %i0  , [%l4 + REG_OUTPUT(0)]                                ;/*  保存输出寄存器              */
    SPARC_B2BST_NOP
    STD     %i2  , [%l4 + REG_OUTPUT(2)]
    SPARC_B2BST_NOP
    STD     %i4  , [%l4 + REG_OUTPUT(4)]
    SPARC_B2BST_NOP
    STD     %i6  , [%l4 + REG_OUTPUT(6)]

    ST      %l1  , [%l4 + REG_PC]                                       ;/*  保存 PC                     */
    ST      %l2  , [%l4 + REG_NPC]                                      ;/*  保存 Next PC                */

    RD      %y   , %l5
    ST      %l5  , [%l4 + REG_Y]                                        ;/*  保存 Y 寄存器               */

    AND     %l0  , SPARC_PSR_CWP_MASK , %l5                             ;/*  %l5 = CWP                   */
    ANDN    %l0  , SPARC_PSR_CWP_MASK , %l0                             ;/*  %l0 = l0 & ~CWP_MASK        */
    ADD     %l5  , 1 , %l5                                              ;/*  %l5 = (CWP + 1) % NWIN      */
    AND     %l5  , LW_CFG_SPARC_REG_WIN_NR - 1 , %l5
    OR      %l0  , %l5 , %l0
    ST      %l0  , [%l4 + REG_PSR]                                      ;/*  保存 PSR 寄存器             */

    MOV     %l4   , %g1
    MOV     %\arg , %g7

    RESTORE                                                             ;/*  回到异常前的寄存器窗口      */

    STD     %l0  , [%g1 + REG_LOCAL(0)]                                 ;/*  保存本地寄存器              */
    SPARC_B2BST_NOP
    STD     %l2  , [%g1 + REG_LOCAL(2)]
    SPARC_B2BST_NOP
    STD     %l4  , [%g1 + REG_LOCAL(4)]
    SPARC_B2BST_NOP
    STD     %l6  , [%g1 + REG_LOCAL(6)]
    SPARC_B2BST_NOP

    STD     %i0  , [%g1 + REG_INPUT(0)]                                 ;/*  保存输入寄存器              */
    SPARC_B2BST_NOP
    STD     %i2  , [%g1 + REG_INPUT(2)]
    SPARC_B2BST_NOP
    STD     %i4  , [%g1 + REG_INPUT(4)]
    SPARC_B2BST_NOP
    STD     %i6  , [%g1 + REG_INPUT(6)]

    ;/*
    ; * %g1 %g6 %g7 不会被破坏
    ; */
    FLUSH_WINDOWS                                                       ;/*  flush 窗口                  */
    MACRO_END()

;/*********************************************************************************************************
;  SPARC 异常入口宏
;   l0 = PSR
;   l1 = PC
;   l2 = nPC
;   l3 = trap type
;*********************************************************************************************************/

MACRO_DEF(SPARC_TRAP_ENTRY handle arg)
    ;/*
    ; * 中断嵌套计数加一
    ; */
    SET     _G_ulIntNesting , %l4                                       ;/*  %l4 = _G_ulIntNesting       */

#if LW_CFG_SMP_EN > 0
    READ_CPUID(%l5)                                                     ;/*  %l5 = CPU_ID                */
    SLL     %l5   , 2   , %l5                                           ;/*  %l5 = CPU_ID * 4            */
    ADD     %l5   , %l4 , %l4                                           ;/*  %l4 = &nesting[CPU_ID]      */
#endif

    LD      [%l4] , %l4                                                 ;/*  %l4 = 中断嵌套计数地址      */
    LD      [%l4] , %l6                                                 ;/*  %l6 = 中断嵌套计数          */

    ADD     %l6 , 1 , %l6                                               ;/*  %l6 = 中断嵌套计数加一!!!   */
    ST      %l6 , [%l4]                                                 ;/*  存储回去                    */

    CMP     %l6 , 1
    BNE     2f
    NOP

    ;/*
    ; * 第一次进入中断
    ; */
    ;/*
    ; * 获取当前 TCB 的 REG_CTX 地址
    ; */
    SET     _G_ulCpu , %l4                                              ;/*  %l4 = _G_ulCpu              */

#if LW_CFG_SMP_EN > 0
    ADD     %l5   , %l4 , %l4                                           ;/*  %l4 = &_G_ulCpu[CPU_ID]     */
#endif

    LD      [%l4] , %l4                                                 ;/*  %l4 = 当前 CPU 结构指针     */
    LD      [%l4] , %l4                                                 ;/*  %l4 = 当前 TCB 结构指针     */

    ;/*
    ; * 保存寄存器到当前 TCB 的 REG_CTX
    ; */
    EXC_SAVE_REGS   \arg

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并设置 SP
    ; */
    SET     _G_ulIntSafeStack , %l0                                     ;/*  %l0 = _G_ulIntSafeStack     */

#if LW_CFG_SMP_EN > 0
    READ_CPUID(%l1)                                                     ;/*  %l1 = CPU_ID                */
    SLL     %l1   , 2   , %l1                                           ;/*  %l1 = CPU_ID * 4            */
    ADD     %l1   , %l0 , %l0                                           ;/*  %l0 = &safestack[CPU_ID]    */
#endif

    LD      [%l0] , %l1                                                 ;/*  %l1 = 中断安全栈            */
    SUB     %l1   , ARCH_STK_FRAME_SIZE , %sp                           ;/*  开辟栈帧空间                */

1:
    ;/*
    ; * handle(trap type 或 PC, 寄存器上下文)
    ; */
    MOV     %g7   , %o0
    MOV     %g1   , %o1
    CALL    \handle
    NOP

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数
    ; */
    CALL    API_InterExit
    NOP

    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    ADD     %sp  , ARCH_STK_FRAME_SIZE , %sp                            ;/*  回收栈帧空间                */

    ;/*
    ; * 从中断栈中恢复寄存器
    ; */
    MOV     %sp  , %g1
    RESTORE_REGS

2:
    ;/*
    ; * 不是第一次进入中断
    ; */
    SUB     %i6  , ARCH_REG_CTX_SIZE , %l4                              ;/*  在中断栈中开辟寄存器保存空间*/

    ;/*
    ; * 保存寄存器到中断栈中
    ; */
    EXC_SAVE_REGS   \arg

    B       1b
    SUB     %g1  , ARCH_STK_FRAME_SIZE , %sp                            ;/*  开辟栈帧空间                */
    MACRO_END()

;/*********************************************************************************************************
;  data_store_error 入口宏
;*********************************************************************************************************/

FUNC_DEF(archDataStoreErrEntry)
    SPARC_TRAP_ENTRY        archDataStoreErrHandle  PC_ARG_REG
    FUNC_END(archDataStoreErrEntry)

;/*********************************************************************************************************
;  instruction_access_MMU_miss 入口宏
;*********************************************************************************************************/

FUNC_DEF(archInstAccessMmuMissEntry)
    SPARC_TRAP_ENTRY        archInstAccessMmuMissHandle  PC_ARG_REG
    FUNC_END(archInstAccessMmuMissEntry)

;/*********************************************************************************************************
;  instruction_access_error 入口宏
;*********************************************************************************************************/

FUNC_DEF(archInstAccessErrEntry)
    SPARC_TRAP_ENTRY        archInstAccessErrHandle  PC_ARG_REG
    FUNC_END(archInstAccessErrEntry)

;/*********************************************************************************************************
;  r_register_access_error 入口宏
;*********************************************************************************************************/

FUNC_DEF(archRRegAccessErrEntry)
    SPARC_TRAP_ENTRY        archRRegAccessErrHandle  PC_ARG_REG
    FUNC_END(archRRegAccessErrEntry)

;/*********************************************************************************************************
;  instruction_access_exception 入口宏
;*********************************************************************************************************/

FUNC_DEF(archInstAccessExcEntry)
    SPARC_TRAP_ENTRY        archInstAccessExcHandle  PC_ARG_REG
    FUNC_END(archInstAccessExcEntry)

;/*********************************************************************************************************
;  privileged_instruction 入口宏
;*********************************************************************************************************/

FUNC_DEF(archPrivInstEntry)
    SPARC_TRAP_ENTRY        archPrivInstHandle  PC_ARG_REG
    FUNC_END(archPrivInstEntry)

;/*********************************************************************************************************
;  illegal_instruction 入口宏
;*********************************************************************************************************/

FUNC_DEF(archIllegalInstEntry)
    SPARC_TRAP_ENTRY        archIllegalInstHandle  PC_ARG_REG
    FUNC_END(archIllegalInstEntry)

;/*********************************************************************************************************
;  fp_disabled 入口宏
;*********************************************************************************************************/

FUNC_DEF(archFpDisableEntry)
    SPARC_TRAP_ENTRY        archFpDisableHandle  PC_ARG_REG
    FUNC_END(archFpDisableEntry)

;/*********************************************************************************************************
;  cp_disabled 入口宏
;*********************************************************************************************************/

FUNC_DEF(archCpDisableEntry)
    SPARC_TRAP_ENTRY        archCpDisableHandle  PC_ARG_REG
    FUNC_END(archCpDisableEntry)

;/*********************************************************************************************************
;  unimplemented_FLUSH 入口宏
;*********************************************************************************************************/

FUNC_DEF(archUnimplFlushEntry)
    SPARC_TRAP_ENTRY        archUnimplFlushHandle  PC_ARG_REG
    FUNC_END(archUnimplFlushEntry)

;/*********************************************************************************************************
;  watchpoint_detected 入口宏
;*********************************************************************************************************/

FUNC_DEF(archWatchPointDectectEntry)
    SPARC_TRAP_ENTRY        archWatchPointDectectHandle  PC_ARG_REG
    FUNC_END(archWatchPointDectectEntry)

;/*********************************************************************************************************
;  mem_address_not_aligned 入口宏
;*********************************************************************************************************/

FUNC_DEF(archMemAddrNoAlignEntry)
    SPARC_TRAP_ENTRY        archMemAddrNoAlignHandle  PC_ARG_REG
    FUNC_END(archMemAddrNoAlignEntry)

;/*********************************************************************************************************
;  fp_exception 入口宏
;*********************************************************************************************************/

FUNC_DEF(archFpExcEntry)
    SPARC_TRAP_ENTRY        archFpExcHandle  PC_ARG_REG
    FUNC_END(archFpExcEntry)

;/*********************************************************************************************************
;  cp_exception 入口宏
;*********************************************************************************************************/

FUNC_DEF(archCpExcEntry)
    SPARC_TRAP_ENTRY        archCpExcHandle  PC_ARG_REG
    FUNC_END(archCpExcEntry)

;/*********************************************************************************************************
;  data_access_error 入口宏
;*********************************************************************************************************/

FUNC_DEF(archDataAccessErrEntry)
    SPARC_TRAP_ENTRY        archDataAccessErrHandle  PC_ARG_REG
    FUNC_END(archDataAccessErrEntry)

;/*********************************************************************************************************
;  data_access_MMU_miss 入口宏
;*********************************************************************************************************/

FUNC_DEF(archDataAccessMmuMissEntry)
    SPARC_TRAP_ENTRY        archDataAccessMmuMissHandle  PC_ARG_REG
    FUNC_END(archDataAccessMmuMissEntry)

;/*********************************************************************************************************
;  data_access_exception 入口宏
;*********************************************************************************************************/

FUNC_DEF(archDataAccessExcEntry)
    SPARC_TRAP_ENTRY        archDataAccessExcHandle  PC_ARG_REG
    FUNC_END(archDataAccessExcEntry)

;/*********************************************************************************************************
;  tag_overflow 入口宏
;*********************************************************************************************************/

FUNC_DEF(archTagOverFlowEntry)
    SPARC_TRAP_ENTRY        archTagOverFlowHandle  PC_ARG_REG
    FUNC_END(archTagOverFlowEntry)

;/*********************************************************************************************************
;  division_by_zero 入口宏
;*********************************************************************************************************/

FUNC_DEF(archDivZeroEntry)
    SPARC_TRAP_ENTRY        archDivZeroHandle  PC_ARG_REG
    FUNC_END(archDivZeroEntry)

;/*********************************************************************************************************
;  trap_instruction 入口宏
;*********************************************************************************************************/

FUNC_DEF(archTrapInstEntry)
    SPARC_TRAP_ENTRY        archTrapInstHandle  PC_ARG_REG
    FUNC_END(archIntEntry)

;/*********************************************************************************************************
;  system call 入口宏
;*********************************************************************************************************/

FUNC_DEF(archSysCallEntry)
    SPARC_TRAP_ENTRY        archSysCallHandle  TRAP_TYPE_ARG_REG
    FUNC_END(archSysCallEntry)

;/*********************************************************************************************************
;  interrupt_level_n 入口宏
;*********************************************************************************************************/

FUNC_DEF(archIntEntry)
    SPARC_TRAP_ENTRY        bspIntHandle  TRAP_TYPE_ARG_REG
    FUNC_END(archIntEntry)

    FILE_END()
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
