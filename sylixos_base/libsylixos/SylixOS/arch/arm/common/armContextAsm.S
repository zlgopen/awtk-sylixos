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
;** 文   件   名: armContextAsm.S
;**
;** 创   建   人: Han.Hui (韩辉)
;**
;** 文件创建日期: 2013 年 12 月 07 日
;**
;** 描        述: ARM 体系构架上下文切换.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>

#include "arch/arm/arch_regs.h"

#if !defined(__SYLIXOS_ARM_ARCH_M__)

    FILE_BEGIN()

;/*********************************************************************************************************
;  ARM 体系构架相关异常处理函数
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
    LDR     R0 , [R0]                                                   ;/*  获取当前 TCB 的 REG_CTX 地址*/
    
LINE_LABEL(archTaskCtxLoad)
    LDMIA   R0!, {R2-R4}                                                ;/*  读取 CPSR LR SP             */

    MSR     CPSR_c , #(DIS_INT | SYS32_MODE)                            ;/*  进入 SYS 模式, 关中断       */
    MOV     SP , R4                                                     ;/*  恢复 SP_sys                 */
    MOV     LR , R3                                                     ;/*  恢复 LR_sys                 */

    MSR     CPSR_c, #(DIS_INT | SVC32_MODE)                             ;/*  进入 SVC 模式, 关中断       */
    MSR     SPSR_cxsf , R2                                              ;/*  CPSR_sys -> SPSR_svc        */
    LDMIA   R0 , {R0-R12, PC}^                                          ;/*  恢复包括 PC 的所有寄存器,   */
                                                                        ;/*  同时更新 CPSR               */
    FUNC_END()

;/*********************************************************************************************************
;  线程切换 
;  参数为当前 CPU 控制块, 即 R0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxSwitch)
    LDR     R1 , [R0]                                                   ;/*  获取当前 TCB                */
    ADD     R1 , R1 , #(ARCH_REG_CTX_SIZE)                              ;/*  当前 TCB 的 REG_CTX 顶端地址*/

    STMFD   R1!, {LR}                                                   ;/*  保存返回地址                */
    STMFD   R1 , {R0-R12}                                               ;/*  保存寄存器                  */
    SUB     R1 , R1 , #(13 * 4)                                         ;/*  调整 R1                     */
    STMFD   R1!, {SP}                                                   ;/*  保存 SP                     */
    STMFD   R1!, {LR}                                                   ;/*  保存 LR                     */
    MRS     R2 , CPSR                                                   ;/*  保存 CPSR                   */
    STMFD   R1!, {R2}

    MOV     R9 , R0                                                     ;/*  备份 R0                     */
#if LW_CFG_SMP_EN > 0
    LDR     R1 , =_SchedSafeStack                                       ;/*  _SchedSafeStack();          */
    MOV     LR , PC
    BX      R1
    MOV     SP , R0                                                     ;/*  设置 SP                     */
    MOV     R0 , R9                                                     ;/*  恢复 R0                     */
#endif

    LDR     R1 , =_SchedSwp                                             ;/*  _SchedSwp();                */
    MOV     LR , PC
    BX      R1
    MOV     R0 , R9                                                     ;/*  恢复 R0                     */

    B       archTaskCtxStart
    FUNC_END()

;/*********************************************************************************************************
;  协程切换 
;  参数为当前 CPU 控制块, 即 R0 为当前 CPU 控制块指针
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    ADD     R1 , R0 , #8
    LDR     R2 , [R1]                                                   ;/*  获取当前 CCB                */
    ADD     R1 , R2 , #(ARCH_REG_CTX_SIZE)                              ;/*  当前 CCB 的 REG_CTX 顶端地址*/

    STMFD   R1!, {LR}                                                   ;/*  保存返回地址                */
    STMFD   R1 , {R0-R12}                                               ;/*  保存寄存器                  */
    SUB     R1 , R1 , #(13 * 4)                                         ;/*  调整 R1                     */
    STMFD   R1!, {SP}                                                   ;/*  保存 SP                     */
    STMFD   R1!, {LR}                                                   ;/*  保存 LR                     */
    MRS     R2 , CPSR                                                   ;/*  保存 CPSR                   */
    STMFD   R1!, {R2}

    MOV     R9 , R0                                                     ;/*  备份 R0                     */
    LDR     R1 , =_SchedCrSwp                                           ;/*  _SchedCrSwp();              */
    MOV     LR , PC
    BX      R1
    MOV     R0 , R9                                                     ;/*  恢复 R0                     */

    ADD     R1 , R0 , #8
    LDR     R0 , [R1]                                                   ;/*  获取当前 CCB 的 REG_CTX 地址*/
    B       archTaskCtxLoad
    FUNC_END()
#endif

;/*********************************************************************************************************
;  中断返回时, 线程装载 
;  参数为当前 CPU 控制块, 即 R0 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    B       archTaskCtxStart
    FUNC_END()

;/*********************************************************************************************************
;  信号上下文返回
;  参数为 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    B       archTaskCtxLoad
    FUNC_END()

;/*********************************************************************************************************
;  拷贝任务上下文
;  参数 R0 为目的 ARCH_REG_CTX 指针, R1 为源 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxCopy)
    STMFD   SP! , {R4-R10}

    LDMIA   R1! , {R2-R10}
    STMIA   R0! , {R2-R10}
    LDMIA   R1! , {R2-R9}
    STMIA   R0! , {R2-R9}

    LDMFD   SP! , {R4-R10}
    BX      LR
    FUNC_END()

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
