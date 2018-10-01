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
;** 文   件   名: c6xExcAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2017 年 03 月 17 日
;**
;** 描        述: c6x 体系构架异常/中断处理.
;*********************************************************************************************************/

    .include "c6xContextAsmInc.asm"

    .ref    bspIntHandle
    .ref    archExcHandle
    .ref    API_InterEnter
    .ref    API_InterExit
    .ref    API_InterStackBaseGet
    .ref    API_ThreadTcbInter

    .ref    _G_ulIntSafeStack
    .ref    _G_ulIntNesting
    .ref    _G_ulCpu

    .global archIntEntry1
    .global archIntEntry2
    .global archIntEntry3
    .global archIntEntry4
    .global archIntEntry5
    .global archIntEntry6
    .global archIntEntry7
    .global archIntEntry8
    .global archIntEntry9
    .global archIntEntry10
    .global archIntEntry11
    .global archIntEntry12
    .global archIntEntry13
    .global archIntEntry14
    .global archIntEntry15

    .sect .text

;/*********************************************************************************************************
;  中断进入点
;*********************************************************************************************************/

ARCH_INT_ENTRY  .macro  vector
    .newblock

    ;/*
    ; * 中断嵌套计数加一
    ; */
    MVKL    _G_ulIntNesting , B1
    MVKH    _G_ulIntNesting , B1
    LDW     *+B1(0) , B1
    NOP     4

    LDW     *+B1(0) , B0                                        ;/*  B0 = 中断嵌套计数                   */
    NOP     4

    ADDK    +1 , B0
    STW     B0 , *+B1(0)

    CMPGT   B0  , 1 , B1
    [B1]    BNOP $4 , 5

    ;/*
    ; * 第一次进入中断
    ; */
    ;/*
    ; * 获取当前 TCB 的 REG_CTX 地址
    ; */
    MVKL    _G_ulCpu , B1
    MVKH    _G_ulCpu , B1
    LDW     *+B1(0) , B0
    NOP     4

    LDW     *+B0(0) , B1                                        ;/*  B1 = 当前 TCB 的 REG_CTX 地址       */
    NOP     4

    ;/*
    ; * 保存寄存器到当前 TCB 的 REG_CTX
    ; */
    IRQ_SAVE_BIG_REG_CTX    IRP

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并设置 SP
    ; */
    MVKL    _G_ulIntSafeStack , B1
    MVKH    _G_ulIntSafeStack , B1
    LDW     *+B1(0) , B15
    NOP     4

$1:
    ;/*
    ; * handle(vector)
    ; */
    MVK     vector , A4
    B       bspIntHandle
    ADDKPC  $2  , B3 , 4
$2:

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数
    ; */
    B       API_InterExit
    ADDKPC  $3  , B3 , 4
$3:

    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    IRQ_NEST_RESTORE_BIG_REG_CTX

$4:
    ;/*
    ; * 不是第一次进入中断
    ; */
    IRQ_NEST_SAVE_BIG_REG_CTX   IRP

    BNOP    $1 , 5
    .endm

;/*********************************************************************************************************
;  异常进入点
;*********************************************************************************************************/

ARCH_EXC_ENTRY  .macro  vector
    .newblock

    ;/*
    ; * 中断嵌套计数加一
    ; */
    MVKL    _G_ulIntNesting , B1
    MVKH    _G_ulIntNesting , B1
    LDW     *+B1(0) , B1
    NOP     4

    LDW     *+B1(0) , B0                                        ;/*  B0 = 中断嵌套计数                   */
    NOP     4

    ADDK    +1 , B0
    STW     B0 , *+B1(0)

    CMPGT   B0  , 1 , B1
    [B1]    BNOP $4 , 5

    ;/*
    ; * 第一次进入中断
    ; */
    ;/*
    ; * 获取当前 TCB 的 REG_CTX 地址
    ; */
    MVKL    _G_ulCpu , B1
    MVKH    _G_ulCpu , B1
    LDW     *+B1(0) , B0
    NOP     4

    LDW     *+B0(0) , B1                                        ;/*  B1 = 当前 TCB 的 REG_CTX 地址       */
    NOP     4

    ;/*
    ; * 保存寄存器到当前 TCB 的 REG_CTX
    ; */
    IRQ_SAVE_BIG_REG_CTX    NRP

    ;/*
    ; * 第一次进入中断: 获得当前 CPU 中断堆栈栈顶, 并设置 SP
    ; */
    MVKL    _G_ulIntSafeStack , B1
    MVKH    _G_ulIntSafeStack , B1
    LDW     *+B1(0) , B15
    NOP     4

    MV      A1 , A4                                             ;/*  当前 TCB 的 REG_CTX 地址作为参数    */

$1:
    ;/*
    ; * archExcHandle(寄存器上下文)
    ; */
    B       archExcHandle
    ADDKPC  $2  , B3 , 4
$2:

    ;/*
    ; * API_InterExit()
    ; * 如果没有发生中断嵌套, 则 API_InterExit 会调用 archIntCtxLoad 函数
    ; */
    B       API_InterExit
    ADDKPC  $3  , B3 , 4
$3:

    ;/*
    ; * 来到这里, 说明发生了中断嵌套
    ; */
    IRQ_NEST_RESTORE_BIG_REG_CTX

$4:
    ;/*
    ; * 不是第一次进入中断
    ; */
    IRQ_NEST_SAVE_BIG_REG_CTX   NRP

    B       $1
    ADD     +8  , B15 , A4                                      ;/*  栈中 ARCH_REG_CTX 地址作为参数      */
    NOP     4
    .endm

;/*********************************************************************************************************
;  中断进入点
;*********************************************************************************************************/

archIntEntry1: .asmfunc
    ARCH_EXC_ENTRY
    .endasmfunc

archIntEntry2: .asmfunc
    ARCH_INT_ENTRY 2
    .endasmfunc

archIntEntry3: .asmfunc
    ARCH_INT_ENTRY 3
    .endasmfunc

archIntEntry4: .asmfunc
    ARCH_INT_ENTRY 4
    .endasmfunc

archIntEntry5: .asmfunc
    ARCH_INT_ENTRY 5
    .endasmfunc

archIntEntry6: .asmfunc
    ARCH_INT_ENTRY 6
    .endasmfunc

archIntEntry7: .asmfunc
    ARCH_INT_ENTRY 7
    .endasmfunc

archIntEntry8: .asmfunc
    ARCH_INT_ENTRY 8
    .endasmfunc

archIntEntry9: .asmfunc
    ARCH_INT_ENTRY 9
    .endasmfunc

archIntEntry10: .asmfunc
    ARCH_INT_ENTRY 10
    .endasmfunc

archIntEntry11: .asmfunc
    ARCH_INT_ENTRY 11
    .endasmfunc

archIntEntry12: .asmfunc
    ARCH_INT_ENTRY 12
    .endasmfunc

archIntEntry13: .asmfunc
    ARCH_INT_ENTRY 13
    .endasmfunc

archIntEntry14: .asmfunc
    ARCH_INT_ENTRY 14
    .endasmfunc

archIntEntry15: .asmfunc
    ARCH_INT_ENTRY 15
    .endasmfunc

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
