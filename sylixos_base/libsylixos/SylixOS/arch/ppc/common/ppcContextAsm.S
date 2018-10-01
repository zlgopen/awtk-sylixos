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
;** 文   件   名: ppcContextAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2015 年 12 月 15 日
;**
;** 描        述: PowerPC 体系构架上下文切换.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>
#include "ppcContextAsm.h"

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
;  当前 CPU 开始运行多任务执行
;  参数为当前 CPU 控制块, 即 R3 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxStart)
    LIS     R4 , HI(~7)
    ORI     R4 , R4 , LO(~7)
    AND     SP , SP , R4                                                ;/*  SP 向下 8 字节对齐          */
    MTSPR   SPRG1 , SP                                                  ;/*  设置异常临时栈(使用启动栈)  */

    LWZ     R4 , 0(R3)                                                  ;/*  获取当前 TCB 的 REG_CTX 地址*/
    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END()

;/*********************************************************************************************************
;  线程切换 
;  参数为当前 CPU 控制块, 即 R3 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archTaskCtxSwitch)
    LWZ     R4 , 0(R3)                                                  ;/*  获取当前 TCB 的 REG_CTX 地址*/
    SAVE_REGS                                                           ;/*  保存所有寄存器              */

    OR      R14, R3 , R3                                                ;/*  备份当前 CPU 控制块         */

#if LW_CFG_SMP_EN > 0
    SUBI    SP , SP , 16                                                ;/*  开辟 C 函数栈帧             */
    BL      _SchedSafeStack                                             ;/*  _SchedSafeStack();          */
    OR      SP , R3 , R3                                                ;/*  设置 SP                     */
    OR      R3 , R14, R14                                               ;/*  恢复当前 CPU 控制块         */
#endif

    SUBI    SP , SP , 16                                                ;/*  开辟 C 函数栈帧             */
    BL      _SchedSwp                                                   ;/*  _SchedSwp();                */
    OR      R3 , R14, R14                                               ;/*  恢复当前 CPU 控制块         */

    LWZ     R4 , 0(R3)                                                  ;/*  获取当前 TCB 的 REG_CTX 地址*/
    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END()

;/*********************************************************************************************************
;  协程切换 
;  参数为当前 CPU 控制块, 即 R3 为当前 CPU 控制块指针
;*********************************************************************************************************/

#if LW_CFG_COROUTINE_EN > 0
FUNC_DEF(archCrtCtxSwitch)
    LWZ     R4 , 8(R3)                                                  ;/*  获取当前 CCB 的 REG_CTX 地址*/
    SAVE_REGS                                                           ;/*  保存所有寄存器              */

    OR      R14, R3 , R3                                                ;/*  备份当前 CPU 控制块         */
    SUBI    SP , SP , 16                                                ;/*  开辟 C 函数栈帧             */
    BL      _SchedCrSwp                                                 ;/*  _SchedCrSwp();              */
    OR      R3 , R14, R14                                               ;/*  恢复当前 CPU 控制块         */

    LWZ     R4 , 8(R3)                                                  ;/*  获取当前 CCB 的 REG_CTX 地址*/
    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END()
#endif

;/*********************************************************************************************************
;  中断返回时, 线程装载 
;  参数为当前 CPU 控制块, 即 R3 为当前 CPU 控制块指针
;*********************************************************************************************************/

FUNC_DEF(archIntCtxLoad)
    LWZ     R4 , 0(R3)                                                  ;/*  获取当前 TCB 的 REG_CTX 地址*/
    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END()

;/*********************************************************************************************************
;  信号上下文返回
;  参数为 ARCH_REG_CTX 指针
;*********************************************************************************************************/

FUNC_DEF(archSigCtxLoad)
    OR      R4 , R3 , R3                                                ;/*  返回的 ARCH_REG_CTX 指针    */
    RESTORE_REGS                                                        ;/*  恢复所有寄存器              */
    FUNC_END()

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
