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
** 文   件   名: c6xContext.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 03 月 17 日
**
** 描        述: c6x 体系构架上下文处理.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  控制寄存器
*********************************************************************************************************/
extern volatile cregister UINT32  CSR;
extern volatile cregister UINT32  AMR;
extern volatile cregister UINT32  GFPGFR;
extern volatile cregister UINT32  IRP;
#if defined(_TMS320C6740)
extern volatile cregister UINT32  FMCR;
extern volatile cregister UINT32  FAUCR;
extern volatile cregister UINT32  FADCR;
#endif                                                                  /*  defined(_TMS320C6740)       */
extern volatile cregister UINT32  SSR;
extern volatile cregister UINT32  RILC;
extern volatile cregister UINT32  ITSR;
extern volatile cregister UINT32  GPLYB;
extern volatile cregister UINT32  GPLYA;
extern volatile cregister UINT32  ILC;
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

    /*
     * B15 is designated as the Stack Pointer (SP).
     * The stack pointer must always remain aligned on a 2-word(8 byte) boundary.
     * The SP points at the first aligned address below (less than) the currently allocated stack
     */
    pstkTop = (PLW_STACK)ROUND_DOWN(pstkTop, ARCH_STK_ALIGN_SIZE);      /*  堆栈指针向下 8 字节对齐     */
    pstkTop--;                                                          /*  堆栈指针指向空栈            */

    pfpctx  = (ARCH_FP_CTX *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX));
    pfpctx->FP_uiRetAddr = 0;
    pfpctx->FP_uiArg     = 0;

    pregctx->REG_uiContextType = 0;                                     /*  小上下文类型                */

    pregctx->REG_uiCsr    = CSR | (1 << 0);                             /*  使能中断                    */
    pregctx->REG_uiAmr    = AMR;
    pregctx->REG_uiGfpgfr = GFPGFR;
    pregctx->REG_uiIrp    = (ARCH_REG_T)pfuncTask;
#if defined(_TMS320C6740)
    pregctx->REG_uiFmcr   = FMCR;
    pregctx->REG_uiFaucr  = FAUCR;
    pregctx->REG_uiFadcr  = FADCR;
#endif                                                                  /*  defined(_TMS320C6740)       */
    pregctx->REG_uiSsr    = SSR;
    pregctx->REG_uiRilc   = RILC;
    pregctx->REG_uiItsr   = ITSR;
    pregctx->REG_uiGplyb  = GPLYB;
    pregctx->REG_uiGplya  = GPLYA;
    pregctx->REG_uiIlc    = ILC;

    pregctx->REG_uiA4  = (ARCH_REG_T)pvArg;                             /*  A4 寄存器传递参数           */
    pregctx->REG_uiA10 = 0xaaaaaa00 + 10;
    pregctx->REG_uiA11 = 0xaaaaaa00 + 11;
    pregctx->REG_uiA12 = 0xaaaaaa00 + 12;
    pregctx->REG_uiA13 = 0xaaaaaa00 + 13;
    pregctx->REG_uiA14 = 0xaaaaaa00 + 14;
    pregctx->REG_uiFp  = 0;                                             /*  FP                          */

    pregctx->REG_uiB10 = 0xbbbbbb00 + 10;
    pregctx->REG_uiB11 = 0xbbbbbb00 + 11;
    pregctx->REG_uiB12 = 0xbbbbbb00 + 12;
    pregctx->REG_uiB13 = 0xbbbbbb00 + 13;
    pregctx->REG_uiDp  = archDataPointerGet();                          /*  B14 是数据指针 DP           */
    pregctx->REG_uiSp  = (ARCH_REG_T)pfpctx - 4;                        /*  B15 是堆栈指针 SP           */

    return  ((PLW_STACK)pregctx);
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
    (VOID)pstkDest;
    (VOID)pregctxDest;
    (VOID)pregctxSrc;

    /*
     * c6x 不使用栈帧来做 backtrace 回溯
     */
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
    *pregctxDest = *pregctxSrc;
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

        fdprintf(iFd, "A0    = 0x%08x  ", pregctx->REG_uiA0);
        fdprintf(iFd, "A1    = 0x%08x\n", pregctx->REG_uiA1);
        fdprintf(iFd, "A2    = 0x%08x  ", pregctx->REG_uiA2);
        fdprintf(iFd, "A3    = 0x%08x\n", pregctx->REG_uiA3);
        fdprintf(iFd, "A4    = 0x%08x  ", pregctx->REG_uiA4);
        fdprintf(iFd, "A5    = 0x%08x\n", pregctx->REG_uiA5);
        fdprintf(iFd, "A6    = 0x%08x  ", pregctx->REG_uiA6);
        fdprintf(iFd, "A7    = 0x%08x\n", pregctx->REG_uiA7);
        fdprintf(iFd, "A8    = 0x%08x  ", pregctx->REG_uiA8);
        fdprintf(iFd, "A9    = 0x%08x\n", pregctx->REG_uiA9);
        fdprintf(iFd, "A10   = 0x%08x  ", pregctx->REG_uiA10);
        fdprintf(iFd, "A11   = 0x%08x\n", pregctx->REG_uiA11);
        fdprintf(iFd, "A12   = 0x%08x  ", pregctx->REG_uiA12);
        fdprintf(iFd, "A13   = 0x%08x\n", pregctx->REG_uiA13);
        fdprintf(iFd, "A14   = 0x%08x  ", pregctx->REG_uiA14);
        fdprintf(iFd, "A15   = 0x%08x\n", pregctx->REG_uiA15);
        fdprintf(iFd, "A16   = 0x%08x  ", pregctx->REG_uiA16);
        fdprintf(iFd, "A17   = 0x%08x\n", pregctx->REG_uiA17);
        fdprintf(iFd, "A18   = 0x%08x  ", pregctx->REG_uiA18);
        fdprintf(iFd, "A19   = 0x%08x\n", pregctx->REG_uiA19);
        fdprintf(iFd, "A20   = 0x%08x  ", pregctx->REG_uiA20);
        fdprintf(iFd, "A21   = 0x%08x\n", pregctx->REG_uiA21);
        fdprintf(iFd, "A22   = 0x%08x  ", pregctx->REG_uiA22);
        fdprintf(iFd, "A23   = 0x%08x\n", pregctx->REG_uiA23);
        fdprintf(iFd, "A24   = 0x%08x  ", pregctx->REG_uiA24);
        fdprintf(iFd, "A25   = 0x%08x\n", pregctx->REG_uiA25);
        fdprintf(iFd, "A26   = 0x%08x  ", pregctx->REG_uiA26);
        fdprintf(iFd, "A27   = 0x%08x\n", pregctx->REG_uiA27);
        fdprintf(iFd, "A28   = 0x%08x  ", pregctx->REG_uiA28);
        fdprintf(iFd, "A29   = 0x%08x\n", pregctx->REG_uiA29);
        fdprintf(iFd, "A30   = 0x%08x  ", pregctx->REG_uiA30);
        fdprintf(iFd, "A31   = 0x%08x\n", pregctx->REG_uiA31);

        fdprintf(iFd, "B0    = 0x%08x  ", pregctx->REG_uiB0);
        fdprintf(iFd, "B1    = 0x%08x\n", pregctx->REG_uiB1);
        fdprintf(iFd, "B2    = 0x%08x  ", pregctx->REG_uiB2);
        fdprintf(iFd, "B3    = 0x%08x\n", pregctx->REG_uiB3);
        fdprintf(iFd, "B4    = 0x%08x  ", pregctx->REG_uiB4);
        fdprintf(iFd, "B5    = 0x%08x\n", pregctx->REG_uiB5);
        fdprintf(iFd, "B6    = 0x%08x  ", pregctx->REG_uiB6);
        fdprintf(iFd, "B7    = 0x%08x\n", pregctx->REG_uiB7);
        fdprintf(iFd, "B8    = 0x%08x  ", pregctx->REG_uiB8);
        fdprintf(iFd, "B9    = 0x%08x\n", pregctx->REG_uiB9);
        fdprintf(iFd, "B10   = 0x%08x  ", pregctx->REG_uiB10);
        fdprintf(iFd, "B11   = 0x%08x\n", pregctx->REG_uiB11);
        fdprintf(iFd, "B12   = 0x%08x  ", pregctx->REG_uiB12);
        fdprintf(iFd, "B13   = 0x%08x\n", pregctx->REG_uiB13);
        fdprintf(iFd, "B14   = 0x%08x  ", pregctx->REG_uiB14);
        fdprintf(iFd, "B15   = 0x%08x\n", pregctx->REG_uiB15);
        fdprintf(iFd, "B16   = 0x%08x  ", pregctx->REG_uiB16);
        fdprintf(iFd, "B17   = 0x%08x\n", pregctx->REG_uiB17);
        fdprintf(iFd, "B18   = 0x%08x  ", pregctx->REG_uiB18);
        fdprintf(iFd, "B19   = 0x%08x\n", pregctx->REG_uiB19);
        fdprintf(iFd, "B20   = 0x%08x  ", pregctx->REG_uiB20);
        fdprintf(iFd, "B21   = 0x%08x\n", pregctx->REG_uiB21);
        fdprintf(iFd, "B22   = 0x%08x  ", pregctx->REG_uiB22);
        fdprintf(iFd, "B23   = 0x%08x\n", pregctx->REG_uiB23);
        fdprintf(iFd, "B24   = 0x%08x  ", pregctx->REG_uiB24);
        fdprintf(iFd, "B25   = 0x%08x\n", pregctx->REG_uiB25);
        fdprintf(iFd, "B26   = 0x%08x  ", pregctx->REG_uiB26);
        fdprintf(iFd, "B27   = 0x%08x\n", pregctx->REG_uiB27);
        fdprintf(iFd, "B28   = 0x%08x  ", pregctx->REG_uiB28);
        fdprintf(iFd, "B29   = 0x%08x\n", pregctx->REG_uiB29);
        fdprintf(iFd, "B30   = 0x%08x  ", pregctx->REG_uiB30);
        fdprintf(iFd, "B31   = 0x%08x\n", pregctx->REG_uiB31);

        fdprintf(iFd, "CSR   = 0x%08x  ", pregctx->REG_uiCsr);
        fdprintf(iFd, "AMR   = 0x%08x\n", pregctx->REG_uiAmr);
        fdprintf(iFd, "IRP   = 0x%08x  ", pregctx->REG_uiIrp);
        fdprintf(iFd, "FMCR  = 0x%08x\n", pregctx->REG_uiFmcr);
        fdprintf(iFd, "FAUCR = 0x%08x  ", pregctx->REG_uiFaucr);
        fdprintf(iFd, "FADCR = 0x%08x\n", pregctx->REG_uiFadcr);
        fdprintf(iFd, "SSR   = 0x%08x  ", pregctx->REG_uiSsr);
        fdprintf(iFd, "ILC   = 0x%08x\n", pregctx->REG_uiIlc);
        fdprintf(iFd, "RILC  = 0x%08x  ", pregctx->REG_uiRilc);
        fdprintf(iFd, "ITSR  = 0x%08x\n", pregctx->REG_uiItsr);
        fdprintf(iFd, "GPLYA = 0x%08x  ", pregctx->REG_uiGplya);
        fdprintf(iFd, "GPLYB = 0x%08x\n", pregctx->REG_uiGplya);
        fdprintf(iFd, "GFPGFR= 0x%08x\n", pregctx->REG_uiGfpgfr);

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

        stOft = bnprintf(pvBuffer, stSize, stOft, "A0    = 0x%08x  ", pregctx->REG_uiA0);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A1    = 0x%08x\n", pregctx->REG_uiA1);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A2    = 0x%08x  ", pregctx->REG_uiA2);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A3    = 0x%08x\n", pregctx->REG_uiA3);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A4    = 0x%08x  ", pregctx->REG_uiA4);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A5    = 0x%08x\n", pregctx->REG_uiA5);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A6    = 0x%08x  ", pregctx->REG_uiA6);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A7    = 0x%08x\n", pregctx->REG_uiA7);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A8    = 0x%08x  ", pregctx->REG_uiA8);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A9    = 0x%08x\n", pregctx->REG_uiA9);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A10   = 0x%08x  ", pregctx->REG_uiA10);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A11   = 0x%08x\n", pregctx->REG_uiA11);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A12   = 0x%08x  ", pregctx->REG_uiA12);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A13   = 0x%08x\n", pregctx->REG_uiA13);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A14   = 0x%08x  ", pregctx->REG_uiA14);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A15   = 0x%08x\n", pregctx->REG_uiA15);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A16   = 0x%08x  ", pregctx->REG_uiA16);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A17   = 0x%08x\n", pregctx->REG_uiA17);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A18   = 0x%08x  ", pregctx->REG_uiA18);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A19   = 0x%08x\n", pregctx->REG_uiA19);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A20   = 0x%08x  ", pregctx->REG_uiA20);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A21   = 0x%08x\n", pregctx->REG_uiA21);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A22   = 0x%08x  ", pregctx->REG_uiA22);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A23   = 0x%08x\n", pregctx->REG_uiA23);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A24   = 0x%08x  ", pregctx->REG_uiA24);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A25   = 0x%08x\n", pregctx->REG_uiA25);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A26   = 0x%08x  ", pregctx->REG_uiA26);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A27   = 0x%08x\n", pregctx->REG_uiA27);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A28   = 0x%08x  ", pregctx->REG_uiA28);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A29   = 0x%08x\n", pregctx->REG_uiA29);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A30   = 0x%08x  ", pregctx->REG_uiA30);
        stOft = bnprintf(pvBuffer, stSize, stOft, "A31   = 0x%08x\n", pregctx->REG_uiA31);

        stOft = bnprintf(pvBuffer, stSize, stOft, "B0    = 0x%08x  ", pregctx->REG_uiB0);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B1    = 0x%08x\n", pregctx->REG_uiB1);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B2    = 0x%08x  ", pregctx->REG_uiB2);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B3    = 0x%08x\n", pregctx->REG_uiB3);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B4    = 0x%08x  ", pregctx->REG_uiB4);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B5    = 0x%08x\n", pregctx->REG_uiB5);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B6    = 0x%08x  ", pregctx->REG_uiB6);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B7    = 0x%08x\n", pregctx->REG_uiB7);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B8    = 0x%08x  ", pregctx->REG_uiB8);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B9    = 0x%08x\n", pregctx->REG_uiB9);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B10   = 0x%08x  ", pregctx->REG_uiB10);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B11   = 0x%08x\n", pregctx->REG_uiB11);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B12   = 0x%08x  ", pregctx->REG_uiB12);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B13   = 0x%08x\n", pregctx->REG_uiB13);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B14   = 0x%08x  ", pregctx->REG_uiB14);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B15   = 0x%08x\n", pregctx->REG_uiB15);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B16   = 0x%08x  ", pregctx->REG_uiB16);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B17   = 0x%08x\n", pregctx->REG_uiB17);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B18   = 0x%08x  ", pregctx->REG_uiB18);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B19   = 0x%08x\n", pregctx->REG_uiB19);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B20   = 0x%08x  ", pregctx->REG_uiB20);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B21   = 0x%08x\n", pregctx->REG_uiB21);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B22   = 0x%08x  ", pregctx->REG_uiB22);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B23   = 0x%08x\n", pregctx->REG_uiB23);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B24   = 0x%08x  ", pregctx->REG_uiB24);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B25   = 0x%08x\n", pregctx->REG_uiB25);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B26   = 0x%08x  ", pregctx->REG_uiB26);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B27   = 0x%08x\n", pregctx->REG_uiB27);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B28   = 0x%08x  ", pregctx->REG_uiB28);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B29   = 0x%08x\n", pregctx->REG_uiB29);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B30   = 0x%08x  ", pregctx->REG_uiB30);
        stOft = bnprintf(pvBuffer, stSize, stOft, "B31   = 0x%08x\n", pregctx->REG_uiB31);

        stOft = bnprintf(pvBuffer, stSize, stOft, "CSR   = 0x%08x  ", pregctx->REG_uiCsr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "AMR   = 0x%08x\n", pregctx->REG_uiAmr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IRP   = 0x%08x  ", pregctx->REG_uiIrp);
        stOft = bnprintf(pvBuffer, stSize, stOft, "FMCR  = 0x%08x\n", pregctx->REG_uiFmcr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "FAUCR = 0x%08x  ", pregctx->REG_uiFaucr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "FADCR = 0x%08x\n", pregctx->REG_uiFadcr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "SSR   = 0x%08x  ", pregctx->REG_uiSsr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "ILC   = 0x%08x\n", pregctx->REG_uiIlc);
        stOft = bnprintf(pvBuffer, stSize, stOft, "RILC  = 0x%08x  ", pregctx->REG_uiRilc);
        stOft = bnprintf(pvBuffer, stSize, stOft, "ITSR  = 0x%08x\n", pregctx->REG_uiItsr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "GPLYA = 0x%08x  ", pregctx->REG_uiGplya);
        stOft = bnprintf(pvBuffer, stSize, stOft, "GPLYB = 0x%08x\n", pregctx->REG_uiGplya);
        stOft = bnprintf(pvBuffer, stSize, stOft, "GFPGFR= 0x%08x\n", pregctx->REG_uiGfpgfr);

    } else {
        _PrintFormat("\r\n");

        _PrintFormat("A0    = 0x%08x  ",   pregctx->REG_uiA0);
        _PrintFormat("A1    = 0x%08x\r\n", pregctx->REG_uiA1);
        _PrintFormat("A2    = 0x%08x  ",   pregctx->REG_uiA2);
        _PrintFormat("A3    = 0x%08x\r\n", pregctx->REG_uiA3);
        _PrintFormat("A4    = 0x%08x  ",   pregctx->REG_uiA4);
        _PrintFormat("A5    = 0x%08x\r\n", pregctx->REG_uiA5);
        _PrintFormat("A6    = 0x%08x  ",   pregctx->REG_uiA6);
        _PrintFormat("A7    = 0x%08x\r\n", pregctx->REG_uiA7);
        _PrintFormat("A8    = 0x%08x  ",   pregctx->REG_uiA8);
        _PrintFormat("A9    = 0x%08x\r\n", pregctx->REG_uiA9);
        _PrintFormat("A10   = 0x%08x  ",   pregctx->REG_uiA10);
        _PrintFormat("A11   = 0x%08x\r\n", pregctx->REG_uiA11);
        _PrintFormat("A12   = 0x%08x  ",   pregctx->REG_uiA12);
        _PrintFormat("A13   = 0x%08x\r\n", pregctx->REG_uiA13);
        _PrintFormat("A14   = 0x%08x  ",   pregctx->REG_uiA14);
        _PrintFormat("A15   = 0x%08x\r\n", pregctx->REG_uiA15);
        _PrintFormat("A16   = 0x%08x  ",   pregctx->REG_uiA16);
        _PrintFormat("A17   = 0x%08x\r\n", pregctx->REG_uiA17);
        _PrintFormat("A18   = 0x%08x  ",   pregctx->REG_uiA18);
        _PrintFormat("A19   = 0x%08x\r\n", pregctx->REG_uiA19);
        _PrintFormat("A20   = 0x%08x  ",   pregctx->REG_uiA20);
        _PrintFormat("A21   = 0x%08x\r\n", pregctx->REG_uiA21);
        _PrintFormat("A22   = 0x%08x  ",   pregctx->REG_uiA22);
        _PrintFormat("A23   = 0x%08x\r\n", pregctx->REG_uiA23);
        _PrintFormat("A24   = 0x%08x  ",   pregctx->REG_uiA24);
        _PrintFormat("A25   = 0x%08x\r\n", pregctx->REG_uiA25);
        _PrintFormat("A26   = 0x%08x  ",   pregctx->REG_uiA26);
        _PrintFormat("A27   = 0x%08x\r\n", pregctx->REG_uiA27);
        _PrintFormat("A28   = 0x%08x  ",   pregctx->REG_uiA28);
        _PrintFormat("A29   = 0x%08x\r\n", pregctx->REG_uiA29);
        _PrintFormat("A30   = 0x%08x  ",   pregctx->REG_uiA30);
        _PrintFormat("A31   = 0x%08x\r\n", pregctx->REG_uiA31);

        _PrintFormat("B0    = 0x%08x  ",   pregctx->REG_uiB0);
        _PrintFormat("B1    = 0x%08x\r\n", pregctx->REG_uiB1);
        _PrintFormat("B2    = 0x%08x  ",   pregctx->REG_uiB2);
        _PrintFormat("B3    = 0x%08x\r\n", pregctx->REG_uiB3);
        _PrintFormat("B4    = 0x%08x  ",   pregctx->REG_uiB4);
        _PrintFormat("B5    = 0x%08x\r\n", pregctx->REG_uiB5);
        _PrintFormat("B6    = 0x%08x  ",   pregctx->REG_uiB6);
        _PrintFormat("B7    = 0x%08x\r\n", pregctx->REG_uiB7);
        _PrintFormat("B8    = 0x%08x  ",   pregctx->REG_uiB8);
        _PrintFormat("B9    = 0x%08x\r\n", pregctx->REG_uiB9);
        _PrintFormat("B10   = 0x%08x  ",   pregctx->REG_uiB10);
        _PrintFormat("B11   = 0x%08x\r\n", pregctx->REG_uiB11);
        _PrintFormat("B12   = 0x%08x  ",   pregctx->REG_uiB12);
        _PrintFormat("B13   = 0x%08x\r\n", pregctx->REG_uiB13);
        _PrintFormat("B14   = 0x%08x  ",   pregctx->REG_uiB14);
        _PrintFormat("B15   = 0x%08x\r\n", pregctx->REG_uiB15);
        _PrintFormat("B16   = 0x%08x  ",   pregctx->REG_uiB16);
        _PrintFormat("B17   = 0x%08x\r\n", pregctx->REG_uiB17);
        _PrintFormat("B18   = 0x%08x  ",   pregctx->REG_uiB18);
        _PrintFormat("B19   = 0x%08x\r\n", pregctx->REG_uiB19);
        _PrintFormat("B20   = 0x%08x  ",   pregctx->REG_uiB20);
        _PrintFormat("B21   = 0x%08x\r\n", pregctx->REG_uiB21);
        _PrintFormat("B22   = 0x%08x  ",   pregctx->REG_uiB22);
        _PrintFormat("B23   = 0x%08x\r\n", pregctx->REG_uiB23);
        _PrintFormat("B24   = 0x%08x  ",   pregctx->REG_uiB24);
        _PrintFormat("B25   = 0x%08x\r\n", pregctx->REG_uiB25);
        _PrintFormat("B26   = 0x%08x  ",   pregctx->REG_uiB26);
        _PrintFormat("B27   = 0x%08x\r\n", pregctx->REG_uiB27);
        _PrintFormat("B28   = 0x%08x  ",   pregctx->REG_uiB28);
        _PrintFormat("B29   = 0x%08x\r\n", pregctx->REG_uiB29);
        _PrintFormat("B30   = 0x%08x  ",   pregctx->REG_uiB30);
        _PrintFormat("B31   = 0x%08x\r\n", pregctx->REG_uiB31);

        _PrintFormat("CSR   = 0x%08x  ",   pregctx->REG_uiCsr);
        _PrintFormat("AMR   = 0x%08x\r\n", pregctx->REG_uiAmr);
        _PrintFormat("IRP   = 0x%08x  ",   pregctx->REG_uiIrp);
        _PrintFormat("FMCR  = 0x%08x\r\n", pregctx->REG_uiFmcr);
        _PrintFormat("FAUCR = 0x%08x  ",   pregctx->REG_uiFaucr);
        _PrintFormat("FADCR = 0x%08x\r\n", pregctx->REG_uiFadcr);
        _PrintFormat("SSR   = 0x%08x  ",   pregctx->REG_uiSsr);
        _PrintFormat("ILC   = 0x%08x\r\n", pregctx->REG_uiIlc);
        _PrintFormat("RILC  = 0x%08x  ",   pregctx->REG_uiRilc);
        _PrintFormat("ITSR  = 0x%08x\r\n", pregctx->REG_uiItsr);
        _PrintFormat("GPLYA = 0x%08x  ",   pregctx->REG_uiGplya);
        _PrintFormat("GPLYB = 0x%08x\r\n", pregctx->REG_uiGplya);
        _PrintFormat("GFPGFR= 0x%08x\r\n", pregctx->REG_uiGfpgfr);
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
