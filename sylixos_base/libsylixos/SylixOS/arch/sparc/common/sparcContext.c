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
** 文   件   名: sparcContext.c
**
** 创   建   人: Xu.Guizhou (徐贵洲)
**
** 文件创建日期: 2017 年 05 月 15 日
**
** 描        述: SPARC 体系构架上下文处理.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "sparcLib.h"
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
    ARCH_FP_CTX  *pfpctx;
    ARCH_REG_T    uiPsr;

    pstkTop = (PLW_STACK)ROUND_DOWN(pstkTop, ARCH_STK_ALIGN_SIZE);      /*  保证出栈后 SP 8 字节对齐    */

    pfpctx  = (ARCH_FP_CTX *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX));

    pfpctx->FP_uiFp      = 0x00000000;
    pfpctx->FP_uiRetAddr = 0x00000000;

    pregctx->REG_uiGlobal[0] = 0x00000000;                              /*  Init global regs            */
    pregctx->REG_uiGlobal[1] = 0x01010101;
    pregctx->REG_uiGlobal[2] = 0x02020202;
    pregctx->REG_uiGlobal[3] = 0x03030303;
    pregctx->REG_uiGlobal[4] = 0x04040404;
    pregctx->REG_uiGlobal[5] = 0x05050505;
    pregctx->REG_uiGlobal[6] = 0x06060606;
    pregctx->REG_uiGlobal[7] = 0x07070707;

    pregctx->REG_uiOutput[0] = (ARCH_REG_T)pvArg;                       /*  Init output regs            */
    pregctx->REG_uiOutput[1] = 0x11111111;
    pregctx->REG_uiOutput[2] = 0x12121212;
    pregctx->REG_uiOutput[3] = 0x13131313;
    pregctx->REG_uiOutput[4] = 0x14141414;
    pregctx->REG_uiOutput[5] = 0x15151515;
    pregctx->REG_uiSp        = (ARCH_REG_T)pfpctx;
    pregctx->REG_uiOutput[7] = 0x00000000;

    pregctx->REG_uiLocal[0]  = 0x20202020;                              /*  Init local regs             */
    pregctx->REG_uiLocal[1]  = 0x21212121;
    pregctx->REG_uiLocal[2]  = 0x22222222;
    pregctx->REG_uiLocal[3]  = 0x23131313;
    pregctx->REG_uiLocal[4]  = 0x24242424;
    pregctx->REG_uiLocal[5]  = 0x25252525;
    pregctx->REG_uiLocal[6]  = 0x26262626;
    pregctx->REG_uiLocal[7]  = 0x27272727;

    pregctx->REG_uiInput[0]  = 0x30303030;                              /*  Init input regs             */
    pregctx->REG_uiInput[1]  = 0x31313131;
    pregctx->REG_uiInput[2]  = 0x32323232;
    pregctx->REG_uiInput[3]  = 0x33333333;
    pregctx->REG_uiInput[4]  = 0x34343434;
    pregctx->REG_uiInput[5]  = 0x35353535;
    pregctx->REG_uiFp        = pfpctx->FP_uiFp;
    pregctx->REG_uiInput[7]  = 0x00000000;

    uiPsr  = archPsrGet();
    uiPsr &= ~PSR_PIL;                                                  /*  使能中断                    */
    uiPsr &= ~PSR_CWP;                                                  /*  使用窗口 0                  */
    uiPsr &= ~PSR_EF;                                                   /*  禁能浮点                    */
    uiPsr &= ~PSR_EC;                                                   /*  禁能协处理器                */
    uiPsr |= PSR_S | PSR_PS;                                            /*  特权模式                    */
    uiPsr |= PSR_ET;                                                    /*  使能异常                    */
    pregctx->REG_uiPsr = uiPsr;
    pregctx->REG_uiPc  = (ARCH_REG_T)pfuncTask;                         /*  PC                          */
    pregctx->REG_uiNPc = pregctx->REG_uiPc + 4;                         /*  Next PC                     */
    pregctx->REG_uiY   = 0x00000000;                                    /*  Y 寄存器                    */

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
    pregctxDest->REG_uiFp  = (ARCH_REG_T)&pregctxSrc->REG_uiLocal[0];
    pregctxDest->REG_uiRet = pregctxSrc->REG_uiPc;
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
    pregctxDest->REG_uiGlobal[0] = pregctxSrc->REG_uiGlobal[0];
    pregctxDest->REG_uiGlobal[1] = pregctxSrc->REG_uiGlobal[1];
    pregctxDest->REG_uiGlobal[2] = pregctxSrc->REG_uiGlobal[2];
    pregctxDest->REG_uiGlobal[3] = pregctxSrc->REG_uiGlobal[3];
    pregctxDest->REG_uiGlobal[4] = pregctxSrc->REG_uiGlobal[4];
    pregctxDest->REG_uiGlobal[5] = pregctxSrc->REG_uiGlobal[5];
    pregctxDest->REG_uiGlobal[6] = pregctxSrc->REG_uiGlobal[6];
    pregctxDest->REG_uiGlobal[7] = pregctxSrc->REG_uiGlobal[7];

    pregctxDest->REG_uiOutput[0] = pregctxSrc->REG_uiOutput[0];
    pregctxDest->REG_uiOutput[1] = pregctxSrc->REG_uiOutput[1];
    pregctxDest->REG_uiOutput[2] = pregctxSrc->REG_uiOutput[2];
    pregctxDest->REG_uiOutput[3] = pregctxSrc->REG_uiOutput[3];
    pregctxDest->REG_uiOutput[4] = pregctxSrc->REG_uiOutput[4];
    pregctxDest->REG_uiOutput[5] = pregctxSrc->REG_uiOutput[5];
    /*
     * %o6(SP)不设置
     */
    pregctxDest->REG_uiOutput[7] = pregctxSrc->REG_uiOutput[7];

    pregctxDest->REG_uiLocal[0]  = pregctxSrc->REG_uiLocal[0];
    pregctxDest->REG_uiLocal[1]  = pregctxSrc->REG_uiLocal[1];
    pregctxDest->REG_uiLocal[2]  = pregctxSrc->REG_uiLocal[2];
    pregctxDest->REG_uiLocal[3]  = pregctxSrc->REG_uiLocal[3];
    pregctxDest->REG_uiLocal[4]  = pregctxSrc->REG_uiLocal[4];
    pregctxDest->REG_uiLocal[5]  = pregctxSrc->REG_uiLocal[5];
    pregctxDest->REG_uiLocal[6]  = pregctxSrc->REG_uiLocal[6];
    pregctxDest->REG_uiLocal[7]  = pregctxSrc->REG_uiLocal[7];

    pregctxDest->REG_uiInput[0]  = pregctxSrc->REG_uiInput[0];
    pregctxDest->REG_uiInput[1]  = pregctxSrc->REG_uiInput[1];
    pregctxDest->REG_uiInput[2]  = pregctxSrc->REG_uiInput[2];
    pregctxDest->REG_uiInput[3]  = pregctxSrc->REG_uiInput[3];
    pregctxDest->REG_uiInput[4]  = pregctxSrc->REG_uiInput[4];
    pregctxDest->REG_uiInput[5]  = pregctxSrc->REG_uiInput[5];
    /*
     * %i6(FP)不设置
     */
    pregctxDest->REG_uiInput[7]  = pregctxSrc->REG_uiInput[7];

    pregctxDest->REG_uiPsr       = pregctxSrc->REG_uiPsr;
    pregctxDest->REG_uiPc        = pregctxSrc->REG_uiPc;
    pregctxDest->REG_uiNPc       = pregctxSrc->REG_uiNPc;
    pregctxDest->REG_uiY         = pregctxSrc->REG_uiY;
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

        fdprintf(iFd, "g0  = 0x%08x  ", pregctx->REG_uiGlobal[0]);
        fdprintf(iFd, "g1  = 0x%08x\n", pregctx->REG_uiGlobal[1]);
        fdprintf(iFd, "g2  = 0x%08x  ", pregctx->REG_uiGlobal[2]);
        fdprintf(iFd, "g3  = 0x%08x\n", pregctx->REG_uiGlobal[3]);
        fdprintf(iFd, "g4  = 0x%08x  ", pregctx->REG_uiGlobal[4]);
        fdprintf(iFd, "g5  = 0x%08x\n", pregctx->REG_uiGlobal[5]);
        fdprintf(iFd, "g6  = 0x%08x  ", pregctx->REG_uiGlobal[6]);
        fdprintf(iFd, "g7  = 0x%08x\n", pregctx->REG_uiGlobal[7]);

        fdprintf(iFd, "o0  = 0x%08x  ", pregctx->REG_uiOutput[0]);
        fdprintf(iFd, "o1  = 0x%08x\n", pregctx->REG_uiOutput[1]);
        fdprintf(iFd, "o2  = 0x%08x  ", pregctx->REG_uiOutput[2]);
        fdprintf(iFd, "o3  = 0x%08x\n", pregctx->REG_uiOutput[3]);
        fdprintf(iFd, "o4  = 0x%08x  ", pregctx->REG_uiOutput[4]);
        fdprintf(iFd, "o5  = 0x%08x\n", pregctx->REG_uiOutput[5]);
        fdprintf(iFd, "o6  = 0x%08x  ", pregctx->REG_uiOutput[6]);
        fdprintf(iFd, "o7  = 0x%08x\n", pregctx->REG_uiOutput[7]);

        fdprintf(iFd, "l0  = 0x%08x  ", pregctx->REG_uiLocal[0]);
        fdprintf(iFd, "l1  = 0x%08x\n", pregctx->REG_uiLocal[1]);
        fdprintf(iFd, "l2  = 0x%08x  ", pregctx->REG_uiLocal[2]);
        fdprintf(iFd, "l3  = 0x%08x\n", pregctx->REG_uiLocal[3]);
        fdprintf(iFd, "l4  = 0x%08x  ", pregctx->REG_uiLocal[4]);
        fdprintf(iFd, "l5  = 0x%08x\n", pregctx->REG_uiLocal[5]);
        fdprintf(iFd, "l6  = 0x%08x  ", pregctx->REG_uiLocal[6]);
        fdprintf(iFd, "l7  = 0x%08x\n", pregctx->REG_uiLocal[7]);

        fdprintf(iFd, "i0  = 0x%08x  ", pregctx->REG_uiInput[0]);
        fdprintf(iFd, "i1  = 0x%08x\n", pregctx->REG_uiInput[1]);
        fdprintf(iFd, "i2  = 0x%08x  ", pregctx->REG_uiInput[2]);
        fdprintf(iFd, "i3  = 0x%08x\n", pregctx->REG_uiInput[3]);
        fdprintf(iFd, "i4  = 0x%08x  ", pregctx->REG_uiInput[4]);
        fdprintf(iFd, "i5  = 0x%08x\n", pregctx->REG_uiInput[5]);
        fdprintf(iFd, "i6  = 0x%08x  ", pregctx->REG_uiInput[6]);
        fdprintf(iFd, "i7  = 0x%08x\n", pregctx->REG_uiInput[7]);

        fdprintf(iFd, "PSR = 0x%08x  ", pregctx->REG_uiPsr);
        fdprintf(iFd, "Y   = 0x%08x\n", pregctx->REG_uiY);
        fdprintf(iFd, "PC  = 0x%08x  ", pregctx->REG_uiPc);
        fdprintf(iFd, "NPC = 0x%08x\n", pregctx->REG_uiNPc);

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

        stOft = bnprintf(pvBuffer, stSize, stOft, "g0  = 0x%08x  ", pregctx->REG_uiGlobal[0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "g1  = 0x%08x\n", pregctx->REG_uiGlobal[1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "g2  = 0x%08x  ", pregctx->REG_uiGlobal[2]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "g3  = 0x%08x\n", pregctx->REG_uiGlobal[3]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "g4  = 0x%08x  ", pregctx->REG_uiGlobal[4]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "g5  = 0x%08x\n", pregctx->REG_uiGlobal[5]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "g6  = 0x%08x  ", pregctx->REG_uiGlobal[6]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "g7  = 0x%08x\n", pregctx->REG_uiGlobal[7]);

        stOft = bnprintf(pvBuffer, stSize, stOft, "o0  = 0x%08x  ", pregctx->REG_uiOutput[0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "o1  = 0x%08x\n", pregctx->REG_uiOutput[1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "o2  = 0x%08x  ", pregctx->REG_uiOutput[2]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "o3  = 0x%08x\n", pregctx->REG_uiOutput[3]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "o4  = 0x%08x  ", pregctx->REG_uiOutput[4]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "o5  = 0x%08x\n", pregctx->REG_uiOutput[5]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "o6  = 0x%08x  ", pregctx->REG_uiOutput[6]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "o7  = 0x%08x\n", pregctx->REG_uiOutput[7]);

        stOft = bnprintf(pvBuffer, stSize, stOft, "l0  = 0x%08x  ", pregctx->REG_uiLocal[0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "l1  = 0x%08x\n", pregctx->REG_uiLocal[1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "l2  = 0x%08x  ", pregctx->REG_uiLocal[2]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "l3  = 0x%08x\n", pregctx->REG_uiLocal[3]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "l4  = 0x%08x  ", pregctx->REG_uiLocal[4]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "l5  = 0x%08x\n", pregctx->REG_uiLocal[5]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "l6  = 0x%08x  ", pregctx->REG_uiLocal[6]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "l7  = 0x%08x\n", pregctx->REG_uiLocal[7]);

        stOft = bnprintf(pvBuffer, stSize, stOft, "i0  = 0x%08x  ", pregctx->REG_uiInput[0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "i1  = 0x%08x\n", pregctx->REG_uiInput[1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "i2  = 0x%08x  ", pregctx->REG_uiInput[2]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "i3  = 0x%08x\n", pregctx->REG_uiInput[3]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "i4  = 0x%08x  ", pregctx->REG_uiInput[4]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "i5  = 0x%08x\n", pregctx->REG_uiInput[5]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "i6  = 0x%08x  ", pregctx->REG_uiInput[6]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "i7  = 0x%08x\n", pregctx->REG_uiInput[7]);

        stOft = bnprintf(pvBuffer, stSize, stOft, "PSR = 0x%08x  ", pregctx->REG_uiPsr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "Y   = 0x%08x\n", pregctx->REG_uiY);
        stOft = bnprintf(pvBuffer, stSize, stOft, "PC  = 0x%08x  ", pregctx->REG_uiPc);
        stOft = bnprintf(pvBuffer, stSize, stOft, "NPC = 0x%08x\n", pregctx->REG_uiNPc);

    } else {
        _PrintFormat("\r\n");

        _PrintFormat("g0  = 0x%08x  ",   pregctx->REG_uiGlobal[0]);
        _PrintFormat("g1  = 0x%08x\r\n", pregctx->REG_uiGlobal[1]);
        _PrintFormat("g2  = 0x%08x  ",   pregctx->REG_uiGlobal[2]);
        _PrintFormat("g3  = 0x%08x\r\n", pregctx->REG_uiGlobal[3]);
        _PrintFormat("g4  = 0x%08x  ",   pregctx->REG_uiGlobal[4]);
        _PrintFormat("g5  = 0x%08x\r\n", pregctx->REG_uiGlobal[5]);
        _PrintFormat("g6  = 0x%08x  ",   pregctx->REG_uiGlobal[6]);
        _PrintFormat("g7  = 0x%08x\r\n", pregctx->REG_uiGlobal[7]);

        _PrintFormat("o0  = 0x%08x  ",   pregctx->REG_uiOutput[0]);
        _PrintFormat("o1  = 0x%08x\r\n", pregctx->REG_uiOutput[1]);
        _PrintFormat("o2  = 0x%08x  ",   pregctx->REG_uiOutput[2]);
        _PrintFormat("o3  = 0x%08x\r\n", pregctx->REG_uiOutput[3]);
        _PrintFormat("o4  = 0x%08x  ",   pregctx->REG_uiOutput[4]);
        _PrintFormat("o5  = 0x%08x\r\n", pregctx->REG_uiOutput[5]);
        _PrintFormat("o6  = 0x%08x  ",   pregctx->REG_uiOutput[6]);
        _PrintFormat("o7  = 0x%08x\r\n", pregctx->REG_uiOutput[7]);

        _PrintFormat("l0  = 0x%08x  ",   pregctx->REG_uiLocal[0]);
        _PrintFormat("l1  = 0x%08x\r\n", pregctx->REG_uiLocal[1]);
        _PrintFormat("l2  = 0x%08x  ",   pregctx->REG_uiLocal[2]);
        _PrintFormat("l3  = 0x%08x\r\n", pregctx->REG_uiLocal[3]);
        _PrintFormat("l4  = 0x%08x  ",   pregctx->REG_uiLocal[4]);
        _PrintFormat("l5  = 0x%08x\r\n", pregctx->REG_uiLocal[5]);
        _PrintFormat("l6  = 0x%08x  ",   pregctx->REG_uiLocal[6]);
        _PrintFormat("l7  = 0x%08x\r\n", pregctx->REG_uiLocal[7]);

        _PrintFormat("i0  = 0x%08x  ",   pregctx->REG_uiInput[0]);
        _PrintFormat("i1  = 0x%08x\r\n", pregctx->REG_uiInput[1]);
        _PrintFormat("i2  = 0x%08x  ",   pregctx->REG_uiInput[2]);
        _PrintFormat("i3  = 0x%08x\r\n", pregctx->REG_uiInput[3]);
        _PrintFormat("i4  = 0x%08x  ",   pregctx->REG_uiInput[4]);
        _PrintFormat("i5  = 0x%08x\r\n", pregctx->REG_uiInput[5]);
        _PrintFormat("i6  = 0x%08x  ",   pregctx->REG_uiInput[6]);
        _PrintFormat("i7  = 0x%08x\r\n", pregctx->REG_uiInput[7]);

        _PrintFormat("PSR = 0x%08x  ",   pregctx->REG_uiPsr);
        _PrintFormat("Y   = 0x%08x\r\n", pregctx->REG_uiY);
        _PrintFormat("PC  = 0x%08x  ",   pregctx->REG_uiPc);
        _PrintFormat("NPC = 0x%08x\r\n", pregctx->REG_uiNPc);
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
