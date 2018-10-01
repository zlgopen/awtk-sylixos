/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: ppcContext.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 12 月 15 日
**
** 描        述: PowerPC 体系构架上下文处理.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "ppcSpr.h"
#include "arch/ppc/param/ppcParam.h"
/*********************************************************************************************************
** 函数名称: archTaskCtxCreate
** 功能描述: 创建任务上下文
** 输　入  : pregctx        寄存器上下文
**           pfuncTask      任务入口
**           pvArg          入口参数
**           pstkTop        初始化堆栈起点
**           ulOpt          任务创建选项
** 输　出  : 初始化堆栈结束点
** 全局变量: 
** 调用模块: 
** 注  意  : 堆栈从高地址向低地址增长.
*********************************************************************************************************/
PLW_STACK  archTaskCtxCreate (ARCH_REG_CTX          *pregctx,
                              PTHREAD_START_ROUTINE  pfuncTask,
                              PVOID                  pvArg,
                              PLW_STACK              pstkTop, 
                              ULONG                  ulOpt)
{
    ARCH_FP_CTX        *pfpctx;
    ARCH_REG_T          uiMsr;
    PPC_PARAM          *pParam;

    uiMsr  = ppcGetMSR();                                               /*  获得当前 MSR 的值           */
    pParam = archKernelParamGet();

    if (pParam->PP_bGenericPPC32) {                                     /*  通用的 32 位 PowerPC 处理器 */
#define ARCH_PPC_MSR_VEC            0x02000000                          /*  Bit 6 of MSR                */

        uiMsr |=  ARCH_PPC_MSR_EE;                                      /*  使能中断                    */
        uiMsr &= ~ARCH_PPC_MSR_PR;                                      /*  特权模式                    */
        uiMsr &= ~ARCH_PPC_MSR_FP;                                      /*  禁能 FPU                    */
        uiMsr &= ~ARCH_PPC_MSR_VEC;                                     /*  禁能 ALTIVEC                */
#if LW_CFG_VMM_EN > 0
        uiMsr |= ARCH_PPC_MSR_IR | ARCH_PPC_MSR_DR;                     /*  使能 MMU                    */
#else
        uiMsr &= ~(ARCH_PPC_MSR_IR | ARCH_PPC_MSR_DR);                  /*  禁能 MMU                    */
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */

#undef  ARCH_PPC_MSR_VEC
    } else {                                                            /*  PowerPC E500 处理器         */
#define ARCH_PPC_MSR_SPE            0x02000000
#define ARCH_PPC_MSR_IS             0x00000020                          /*  insn address space selector */
#define ARCH_PPC_MSR_DS             0x00000010                          /*  data address space selector */

        uiMsr |=  ARCH_PPC_MSR_EE;                                      /*  使能中断                    */
        uiMsr &= ~ARCH_PPC_MSR_SPE;                                     /*  禁能 SPE(E6500 时为 ALTIVEC)*/
        uiMsr &= ~ARCH_PPC_MSR_FP;                                      /*  禁能 FPU(E500mc E5500 E6500)*/
        uiMsr &= ~(ARCH_PPC_MSR_IS | ARCH_PPC_MSR_DS);                  /*  使用地址空间 0              */

#undef  ARCH_PPC_MSR_SPE
#undef  ARCH_PPC_MSR_IS
#undef  ARCH_PPC_MSR_DS
    }

    pstkTop = (PLW_STACK)ROUND_DOWN(pstkTop, ARCH_STK_ALIGN_SIZE);      /*  堆栈指针向下 8 字节对齐     */

    pfpctx  = (ARCH_FP_CTX  *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX));

    pfpctx->FP_uiFp     = (ARCH_REG_T)LW_NULL;
    pfpctx->FP_uiLr     = (ARCH_REG_T)LW_NULL;

    pregctx->REG_uiR0   = 0;
    pregctx->REG_uiSp   = (ARCH_REG_T)pfpctx;
    pregctx->REG_uiR2   = 2;
    pregctx->REG_uiR3   = (ARCH_REG_T)pvArg;                            /*  R3 用于参数传递             */
    pregctx->REG_uiR4   = 4;
    pregctx->REG_uiR5   = 5;
    pregctx->REG_uiR6   = 6;
    pregctx->REG_uiR7   = 7;
    pregctx->REG_uiR8   = 8;
    pregctx->REG_uiR9   = 9;
    pregctx->REG_uiR10  = 10;
    pregctx->REG_uiR11  = 11;
    pregctx->REG_uiR12  = 12;
    pregctx->REG_uiR13  = 13;
    pregctx->REG_uiR14  = 14;
    pregctx->REG_uiR15  = 15;
    pregctx->REG_uiR16  = 16;
    pregctx->REG_uiR17  = 17;
    pregctx->REG_uiR18  = 18;
    pregctx->REG_uiR19  = 19;
    pregctx->REG_uiR20  = 20;
    pregctx->REG_uiR21  = 21;
    pregctx->REG_uiR22  = 22;
    pregctx->REG_uiR23  = 23;
    pregctx->REG_uiR24  = 24;
    pregctx->REG_uiR25  = 25;
    pregctx->REG_uiR26  = 26;
    pregctx->REG_uiR27  = 27;
    pregctx->REG_uiR28  = 28;
    pregctx->REG_uiR29  = 29;
    pregctx->REG_uiR30  = 30;
    pregctx->REG_uiFp   = pfpctx->FP_uiFp;

    pregctx->REG_uiLr   = (ARCH_REG_T)pfuncTask;
    pregctx->REG_uiPc   = (ARCH_REG_T)pfuncTask;                        /*  RFI 返回的地址              */
    pregctx->REG_uiMsr  = uiMsr;                                        /*  RFI 后的 MSR                */
    pregctx->REG_uiCtr  = 0;
    pregctx->REG_uiXer  = 0;
    pregctx->REG_uiCr   = 0;

    return  ((PLW_STACK)pfpctx);
}
/*********************************************************************************************************
** 函数名称: archTaskCtxSetFp
** 功能描述: 设置任务上下文栈帧 (用于 backtrace 回溯, 详情请见 backtrace 相关文件)
** 输　入  : pstkDest      目的 stack frame
**           pregctxDest   目的寄存器上下文
**           pregctxSrc    源寄存器上下文
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  archTaskCtxSetFp (PLW_STACK               pstkDest,
                        ARCH_REG_CTX           *pregctxDest,
                        const ARCH_REG_CTX     *pregctxSrc)
{
    ARCH_FP_CTX  *pfpctx = (ARCH_FP_CTX *)pstkDest;

    /*
     *  在 ARCH_FP_CTX 区域内, 模拟了一次
     *  push {fp, lr}
     *  add  fp, sp, #4
     */
    pfpctx->FP_uiFp = pregctxSrc->REG_uiFp;
    pfpctx->FP_uiLr = pregctxSrc->REG_uiLr;

    pregctxDest->REG_uiFp = (ARCH_REG_T)&pfpctx->FP_uiLr;
}
/*********************************************************************************************************
** 函数名称: archTaskRegsGet
** 功能描述: 获取寄存器上下文
** 输　入  : pregctx        寄存器上下文
**           pregSp         SP 指针
** 输　出  : 寄存器上下文
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
ARCH_REG_CTX  *archTaskRegsGet (ARCH_REG_CTX  *pregctx, ARCH_REG_T *pregSp)
{
    *pregSp = pregctx->REG_uiSp;

    return  (pregctx);
}
/*********************************************************************************************************
** 函数名称: archTaskRegsSet
** 功能描述: 设置寄存器上下文
** 输　入  : pregctxDest    目的寄存器上下文
**           pregctxSrc     源寄存器上下文
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  archTaskRegsSet (ARCH_REG_CTX  *pregctxDest, const ARCH_REG_CTX  *pregctxSrc)
{
    pregctxDest->REG_uiR0   = pregctxSrc->REG_uiR0;
    /*
     * R1(SP)不设置
     */
    pregctxDest->REG_uiR2   = pregctxSrc->REG_uiR2;
    pregctxDest->REG_uiR3   = pregctxSrc->REG_uiR3;
    pregctxDest->REG_uiR4   = pregctxSrc->REG_uiR4;
    pregctxDest->REG_uiR5   = pregctxSrc->REG_uiR5;
    pregctxDest->REG_uiR6   = pregctxSrc->REG_uiR6;
    pregctxDest->REG_uiR7   = pregctxSrc->REG_uiR7;
    pregctxDest->REG_uiR8   = pregctxSrc->REG_uiR8;
    pregctxDest->REG_uiR9   = pregctxSrc->REG_uiR9;
    pregctxDest->REG_uiR10  = pregctxSrc->REG_uiR10;
    pregctxDest->REG_uiR11  = pregctxSrc->REG_uiR11;
    pregctxDest->REG_uiR12  = pregctxSrc->REG_uiR12;
    pregctxDest->REG_uiR13  = pregctxSrc->REG_uiR13;
    pregctxDest->REG_uiR14  = pregctxSrc->REG_uiR14;
    pregctxDest->REG_uiR15  = pregctxSrc->REG_uiR15;
    pregctxDest->REG_uiR16  = pregctxSrc->REG_uiR16;
    pregctxDest->REG_uiR17  = pregctxSrc->REG_uiR17;
    pregctxDest->REG_uiR18  = pregctxSrc->REG_uiR18;
    pregctxDest->REG_uiR19  = pregctxSrc->REG_uiR19;
    pregctxDest->REG_uiR20  = pregctxSrc->REG_uiR20;
    pregctxDest->REG_uiR21  = pregctxSrc->REG_uiR21;
    pregctxDest->REG_uiR22  = pregctxSrc->REG_uiR22;
    pregctxDest->REG_uiR23  = pregctxSrc->REG_uiR23;
    pregctxDest->REG_uiR24  = pregctxSrc->REG_uiR24;
    pregctxDest->REG_uiR25  = pregctxSrc->REG_uiR25;
    pregctxDest->REG_uiR26  = pregctxSrc->REG_uiR26;
    pregctxDest->REG_uiR27  = pregctxSrc->REG_uiR27;
    pregctxDest->REG_uiR28  = pregctxSrc->REG_uiR28;
    pregctxDest->REG_uiR29  = pregctxSrc->REG_uiR29;
    pregctxDest->REG_uiR30  = pregctxSrc->REG_uiR30;
    /*
     * R31(FP)不设置
     */
    pregctxDest->REG_uiSrr0 = pregctxSrc->REG_uiSrr0;
    pregctxDest->REG_uiSrr1 = pregctxSrc->REG_uiSrr1;
    pregctxDest->REG_uiCtr  = pregctxSrc->REG_uiCtr;
    pregctxDest->REG_uiXer  = pregctxSrc->REG_uiXer;
    pregctxDest->REG_uiCr   = pregctxSrc->REG_uiCr;
    pregctxDest->REG_uiLr   = pregctxSrc->REG_uiLr;
}
/*********************************************************************************************************
** 函数名称: archTaskCtxShow
** 功能描述: 打印任务上下文
** 输　入  : iFd        文件描述符
             pregctx    寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
#if LW_CFG_DEVICE_EN > 0

VOID  archTaskCtxShow (INT  iFd, const ARCH_REG_CTX  *pregctx)
{
    if (iFd >= 0) {
        fdprintf(iFd, "\n");

        fdprintf(iFd, "R0  = 0x%08x  ", pregctx->REG_uiR0);
        fdprintf(iFd, "R1  = 0x%08x\n", pregctx->REG_uiR1);

        fdprintf(iFd, "R2  = 0x%08x  ", pregctx->REG_uiR2);
        fdprintf(iFd, "R3  = 0x%08x\n", pregctx->REG_uiR3);

        fdprintf(iFd, "R4  = 0x%08x  ", pregctx->REG_uiR4);
        fdprintf(iFd, "R5  = 0x%08x\n", pregctx->REG_uiR5);

        fdprintf(iFd, "R6  = 0x%08x  ", pregctx->REG_uiR6);
        fdprintf(iFd, "R7  = 0x%08x\n", pregctx->REG_uiR7);

        fdprintf(iFd, "R8  = 0x%08x  ", pregctx->REG_uiR8);
        fdprintf(iFd, "R9  = 0x%08x\n", pregctx->REG_uiR9);

        fdprintf(iFd, "R10 = 0x%08x  ", pregctx->REG_uiR10);
        fdprintf(iFd, "R11 = 0x%08x\n", pregctx->REG_uiR11);

        fdprintf(iFd, "R12 = 0x%08x  ", pregctx->REG_uiR12);
        fdprintf(iFd, "R13 = 0x%08x\n", pregctx->REG_uiR13);

        fdprintf(iFd, "R14 = 0x%08x  ", pregctx->REG_uiR14);
        fdprintf(iFd, "R15 = 0x%08x\n", pregctx->REG_uiR15);

        fdprintf(iFd, "R16 = 0x%08x  ", pregctx->REG_uiR16);
        fdprintf(iFd, "R17 = 0x%08x\n", pregctx->REG_uiR17);

        fdprintf(iFd, "R18 = 0x%08x  ", pregctx->REG_uiR18);
        fdprintf(iFd, "R19 = 0x%08x\n", pregctx->REG_uiR19);

        fdprintf(iFd, "R20 = 0x%08x  ", pregctx->REG_uiR20);
        fdprintf(iFd, "R21 = 0x%08x\n", pregctx->REG_uiR21);

        fdprintf(iFd, "R22 = 0x%08x  ", pregctx->REG_uiR22);
        fdprintf(iFd, "R23 = 0x%08x\n", pregctx->REG_uiR23);

        fdprintf(iFd, "R24 = 0x%08x  ", pregctx->REG_uiR24);
        fdprintf(iFd, "R25 = 0x%08x\n", pregctx->REG_uiR25);

        fdprintf(iFd, "R26 = 0x%08x  ", pregctx->REG_uiR26);
        fdprintf(iFd, "R27 = 0x%08x\n", pregctx->REG_uiR27);

        fdprintf(iFd, "R28 = 0x%08x  ", pregctx->REG_uiR28);
        fdprintf(iFd, "R29 = 0x%08x\n", pregctx->REG_uiR29);

        fdprintf(iFd, "R30 = 0x%08x  ", pregctx->REG_uiR30);
        fdprintf(iFd, "R31 = 0x%08x\n", pregctx->REG_uiR31);

        fdprintf(iFd, "FP  = 0x%08x  ", pregctx->REG_uiFp);
        fdprintf(iFd, "SP  = 0x%08x\n", pregctx->REG_uiSp);

        fdprintf(iFd, "LR  = 0x%08x  ", pregctx->REG_uiLr);
        fdprintf(iFd, "PC  = 0x%08x\n", pregctx->REG_uiPc);

        fdprintf(iFd, "MSR = 0x%08x  ", pregctx->REG_uiMsr);
        fdprintf(iFd, "CR  = 0x%08x\n", pregctx->REG_uiCr);

        fdprintf(iFd, "CTR = 0x%08x  ", pregctx->REG_uiCtr);
        fdprintf(iFd, "XER = 0x%08x\n", pregctx->REG_uiXer);

    } else {
        archTaskCtxPrint(LW_NULL, 0, pregctx);
    }
}

#endif                                                                  /*  LW_CFG_DEVICE_EN > 0        */
/*********************************************************************************************************
** 函数名称: archTaskCtxPrint
** 功能描述: 直接打印任务上下文
** 输　入  : pvBuffer   内存缓冲区 (NULL, 表示直接打印)
**           stSize     缓冲大小
**           pregctx    寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archTaskCtxPrint (PVOID  pvBuffer, size_t  stSize, const ARCH_REG_CTX  *pregctx)
{
    if (pvBuffer && stSize) {
        size_t  stOft = 0;

        stOft = bnprintf(pvBuffer, stSize, stOft, "R0  = 0x%08x  ", pregctx->REG_uiR0);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R1  = 0x%08x\n", pregctx->REG_uiR1);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R2  = 0x%08x  ", pregctx->REG_uiR2);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R3  = 0x%08x\n", pregctx->REG_uiR3);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R4  = 0x%08x  ", pregctx->REG_uiR4);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R5  = 0x%08x\n", pregctx->REG_uiR5);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R6  = 0x%08x  ", pregctx->REG_uiR6);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R7  = 0x%08x\n", pregctx->REG_uiR7);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R8  = 0x%08x  ", pregctx->REG_uiR8);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R9  = 0x%08x\n", pregctx->REG_uiR9);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R10 = 0x%08x  ", pregctx->REG_uiR10);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R11 = 0x%08x\n", pregctx->REG_uiR11);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R12 = 0x%08x  ", pregctx->REG_uiR12);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R13 = 0x%08x\n", pregctx->REG_uiR13);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R14 = 0x%08x  ", pregctx->REG_uiR14);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R15 = 0x%08x\n", pregctx->REG_uiR15);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R16 = 0x%08x  ", pregctx->REG_uiR16);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R17 = 0x%08x\n", pregctx->REG_uiR17);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R18 = 0x%08x  ", pregctx->REG_uiR18);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R19 = 0x%08x\n", pregctx->REG_uiR19);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R20 = 0x%08x  ", pregctx->REG_uiR20);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R21 = 0x%08x\n", pregctx->REG_uiR21);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R22 = 0x%08x  ", pregctx->REG_uiR22);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R23 = 0x%08x\n", pregctx->REG_uiR23);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R24 = 0x%08x  ", pregctx->REG_uiR24);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R25 = 0x%08x\n", pregctx->REG_uiR25);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R26 = 0x%08x  ", pregctx->REG_uiR26);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R27 = 0x%08x\n", pregctx->REG_uiR27);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R28 = 0x%08x  ", pregctx->REG_uiR28);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R29 = 0x%08x\n", pregctx->REG_uiR29);

        stOft = bnprintf(pvBuffer, stSize, stOft, "R30 = 0x%08x  ", pregctx->REG_uiR30);
        stOft = bnprintf(pvBuffer, stSize, stOft, "R31 = 0x%08x\n", pregctx->REG_uiR31);

        stOft = bnprintf(pvBuffer, stSize, stOft, "FP  = 0x%08x  ", pregctx->REG_uiFp);
        stOft = bnprintf(pvBuffer, stSize, stOft, "SP  = 0x%08x\n", pregctx->REG_uiSp);

        stOft = bnprintf(pvBuffer, stSize, stOft, "LR  = 0x%08x  ", pregctx->REG_uiLr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "PC  = 0x%08x\n", pregctx->REG_uiPc);

        stOft = bnprintf(pvBuffer, stSize, stOft, "MSR = 0x%08x  ", pregctx->REG_uiMsr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "CR  = 0x%08x\n", pregctx->REG_uiCr);

        stOft = bnprintf(pvBuffer, stSize, stOft, "CTR = 0x%08x  ", pregctx->REG_uiCtr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "XER = 0x%08x\n", pregctx->REG_uiXer);

    } else {
        _PrintFormat("\r\n");

        _PrintFormat("R0  = 0x%08x  ",   pregctx->REG_uiR0);
        _PrintFormat("R1  = 0x%08x\r\n", pregctx->REG_uiR1);

        _PrintFormat("R2  = 0x%08x  ",   pregctx->REG_uiR2);
        _PrintFormat("R3  = 0x%08x\r\n", pregctx->REG_uiR3);

        _PrintFormat("R4  = 0x%08x  ",   pregctx->REG_uiR4);
        _PrintFormat("R5  = 0x%08x\r\n", pregctx->REG_uiR5);

        _PrintFormat("R6  = 0x%08x  ",   pregctx->REG_uiR6);
        _PrintFormat("R7  = 0x%08x\r\n", pregctx->REG_uiR7);

        _PrintFormat("R8  = 0x%08x  ",   pregctx->REG_uiR8);
        _PrintFormat("R9  = 0x%08x\r\n", pregctx->REG_uiR9);

        _PrintFormat("R10 = 0x%08x  ",   pregctx->REG_uiR10);
        _PrintFormat("R11 = 0x%08x\r\n", pregctx->REG_uiR11);

        _PrintFormat("R12 = 0x%08x  ",   pregctx->REG_uiR12);
        _PrintFormat("R13 = 0x%08x\r\n", pregctx->REG_uiR13);

        _PrintFormat("R14 = 0x%08x  ",   pregctx->REG_uiR14);
        _PrintFormat("R15 = 0x%08x\r\n", pregctx->REG_uiR15);

        _PrintFormat("R16 = 0x%08x  ",   pregctx->REG_uiR16);
        _PrintFormat("R17 = 0x%08x\r\n", pregctx->REG_uiR17);

        _PrintFormat("R18 = 0x%08x  ",   pregctx->REG_uiR18);
        _PrintFormat("R19 = 0x%08x\r\n", pregctx->REG_uiR19);

        _PrintFormat("R20 = 0x%08x  ",   pregctx->REG_uiR20);
        _PrintFormat("R21 = 0x%08x\r\n", pregctx->REG_uiR21);

        _PrintFormat("R22 = 0x%08x  ",   pregctx->REG_uiR22);
        _PrintFormat("R23 = 0x%08x\r\n", pregctx->REG_uiR23);

        _PrintFormat("R24 = 0x%08x  ",   pregctx->REG_uiR24);
        _PrintFormat("R25 = 0x%08x\r\n", pregctx->REG_uiR25);

        _PrintFormat("R26 = 0x%08x  ",   pregctx->REG_uiR26);
        _PrintFormat("R27 = 0x%08x\r\n", pregctx->REG_uiR27);

        _PrintFormat("R28 = 0x%08x  ",   pregctx->REG_uiR28);
        _PrintFormat("R29 = 0x%08x\r\n", pregctx->REG_uiR29);

        _PrintFormat("R30 = 0x%08x  ",   pregctx->REG_uiR30);
        _PrintFormat("R31 = 0x%08x\r\n", pregctx->REG_uiR31);

        _PrintFormat("FP  = 0x%08x  ",   pregctx->REG_uiFp);
        _PrintFormat("SP  = 0x%08x\r\n", pregctx->REG_uiSp);

        _PrintFormat("LR  = 0x%08x  ",   pregctx->REG_uiLr);
        _PrintFormat("PC  = 0x%08x\r\n", pregctx->REG_uiPc);

        _PrintFormat("MSR = 0x%08x  ",   pregctx->REG_uiMsr);
        _PrintFormat("CR  = 0x%08x\r\n", pregctx->REG_uiCr);

        _PrintFormat("CTR = 0x%08x  ",   pregctx->REG_uiCtr);
        _PrintFormat("XER = 0x%08x\r\n", pregctx->REG_uiXer);
    }
}
/*********************************************************************************************************
** 函数名称: archIntCtxSaveReg
** 功能描述: 中断保存寄存器
** 输　入  : pcpu      CPU 结构
**           reg0      寄存器 0
**           reg1      寄存器 1
**           reg2      寄存器 2
**           reg3      寄存器 3
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archIntCtxSaveReg (PLW_CLASS_CPU  pcpu,
                         ARCH_REG_T     reg0,
                         ARCH_REG_T     reg1,
                         ARCH_REG_T     reg2,
                         ARCH_REG_T     reg3)
{
}
/*********************************************************************************************************
** 函数名称: archCtxStackEnd
** 功能描述: 根据寄存器上下文获得栈结束地址
** 输　入  : pregctx    寄存器上下文
** 输　出  : 栈结束地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
PLW_STACK  archCtxStackEnd (const ARCH_REG_CTX  *pregctx)
{
    return  ((PLW_STACK)pregctx->REG_uiSp);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
