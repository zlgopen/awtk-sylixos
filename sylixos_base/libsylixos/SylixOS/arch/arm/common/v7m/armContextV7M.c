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
** 文   件   名: armContextV7M.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 11 月 14 日
**
** 描        述: ARMv7M 体系构架上下文处理.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  ARMv7M 体系构架
*********************************************************************************************************/
#if defined(__SYLIXOS_ARM_ARCH_M__)
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

    pstkTop = (PLW_STACK)ROUND_DOWN(pstkTop, ARCH_STK_ALIGN_SIZE);      /*  堆栈指针向下 8 字节对齐     */

    pfpctx  = (ARCH_FP_CTX *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX));

    pfpctx->FP_uiFp = (ARCH_REG_T)LW_NULL;
    pfpctx->FP_uiLr = (ARCH_REG_T)LW_NULL;

    pregctx->REG_uiXpsr = 1 << 24;                                      /*  设置 Thumb 状态位           */
    pregctx->REG_uiR0   = (ARCH_REG_T)pvArg;
    pregctx->REG_uiR1   = 0x01010101;
    pregctx->REG_uiR2   = 0x02020202;
    pregctx->REG_uiR3   = 0x03030303;
    pregctx->REG_uiR4   = 0x04040404;
    pregctx->REG_uiR5   = 0x05050505;
    pregctx->REG_uiR6   = 0x06060606;
    pregctx->REG_uiR7   = 0x07070707;
    pregctx->REG_uiR8   = 0x08080808;
    pregctx->REG_uiR9   = 0x09090909;
    pregctx->REG_uiR10  = 0x10101010;
    pregctx->REG_uiFp   = pfpctx->FP_uiFp;
    pregctx->REG_uiIp   = 0x12121212;
    pregctx->REG_uiLr   = (ARCH_REG_T)pfuncTask;
    pregctx->REG_uiPc   = (ARCH_REG_T)pfuncTask;
    pregctx->REG_uiSp   = (ARCH_REG_T)pfpctx;

    pregctx->REG_uiExcRet  = 0xfffffffd;                                /*  回线程模式，并使用线程堆栈  */
                                                                        /*  (SP=PSP)                    */
    pregctx->REG_uiBASEPRI = 0;

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
    *pregctxDest = *pregctxSrc;
}
/*********************************************************************************************************
** 函数名称: archTaskCtxShow
** 功能描述: 打印任务上下文
** 输　入  : iFd        文件描述符
**           pregctx    寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
#if LW_CFG_DEVICE_EN > 0

VOID  archTaskCtxShow (INT  iFd, const ARCH_REG_CTX  *pregctx)
{
    if (iFd >= 0) {
        fdprintf(iFd, "XPSR    = 0x%08x\n", pregctx->REG_uiXpsr);
        fdprintf(iFd, "BASEPRI = 0x%08x\n", pregctx->REG_uiBASEPRI);
        fdprintf(iFd, "EXCRET  = 0x%08x\n", pregctx->REG_uiExcRet);

        fdprintf(iFd, "r0  = 0x%08x  ", pregctx->REG_uiR0);
        fdprintf(iFd, "r1  = 0x%08x\n", pregctx->REG_uiR1);
        fdprintf(iFd, "r2  = 0x%08x  ", pregctx->REG_uiR2);
        fdprintf(iFd, "r3  = 0x%08x\n", pregctx->REG_uiR3);
        fdprintf(iFd, "r4  = 0x%08x  ", pregctx->REG_uiR4);
        fdprintf(iFd, "r5  = 0x%08x\n", pregctx->REG_uiR5);
        fdprintf(iFd, "r6  = 0x%08x  ", pregctx->REG_uiR6);
        fdprintf(iFd, "r7  = 0x%08x\n", pregctx->REG_uiR7);
        fdprintf(iFd, "r8  = 0x%08x  ", pregctx->REG_uiR8);
        fdprintf(iFd, "r9  = 0x%08x\n", pregctx->REG_uiR9);
        fdprintf(iFd, "r10 = 0x%08x  ", pregctx->REG_uiR10);
        fdprintf(iFd, "fp  = 0x%08x\n", pregctx->REG_uiFp);
        fdprintf(iFd, "ip  = 0x%08x  ", pregctx->REG_uiIp);
        fdprintf(iFd, "sp  = 0x%08x\n", pregctx->REG_uiSp);
        fdprintf(iFd, "lr  = 0x%08x  ", pregctx->REG_uiLr);
        fdprintf(iFd, "pc  = 0x%08x\n", pregctx->REG_uiPc);

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

        stOft = bnprintf(pvBuffer, stSize, stOft, "XPSR    = 0x%08x\n", pregctx->REG_uiXpsr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "BASEPRI = 0x%08x\n", pregctx->REG_uiBASEPRI);
        stOft = bnprintf(pvBuffer, stSize, stOft, "EXCRET  = 0x%08x\n", pregctx->REG_uiExcRet);

        stOft = bnprintf(pvBuffer, stSize, stOft, "r0  = 0x%08x  ", pregctx->REG_uiR0);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r1  = 0x%08x\n", pregctx->REG_uiR1);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r2  = 0x%08x  ", pregctx->REG_uiR2);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r3  = 0x%08x\n", pregctx->REG_uiR3);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r4  = 0x%08x  ", pregctx->REG_uiR4);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r5  = 0x%08x\n", pregctx->REG_uiR5);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r6  = 0x%08x  ", pregctx->REG_uiR6);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r7  = 0x%08x\n", pregctx->REG_uiR7);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r8  = 0x%08x  ", pregctx->REG_uiR8);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r9  = 0x%08x\n", pregctx->REG_uiR9);
        stOft = bnprintf(pvBuffer, stSize, stOft, "r10 = 0x%08x  ", pregctx->REG_uiR10);
        stOft = bnprintf(pvBuffer, stSize, stOft, "fp  = 0x%08x\n", pregctx->REG_uiFp);
        stOft = bnprintf(pvBuffer, stSize, stOft, "ip  = 0x%08x  ", pregctx->REG_uiIp);
        stOft = bnprintf(pvBuffer, stSize, stOft, "sp  = 0x%08x\n", pregctx->REG_uiSp);
        stOft = bnprintf(pvBuffer, stSize, stOft, "lr  = 0x%08x  ", pregctx->REG_uiLr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "pc  = 0x%08x\n", pregctx->REG_uiPc);

    } else {
        _PrintFormat("XPSR    = 0x%08x\r\n", pregctx->REG_uiXpsr);
        _PrintFormat("BASEPRI = 0x%08x\r\n", pregctx->REG_uiBASEPRI);
        _PrintFormat("EXCRET  = 0x%08x\r\n", pregctx->REG_uiExcRet);

        _PrintFormat("r0  = 0x%08x  ",   pregctx->REG_uiR0);
        _PrintFormat("r1  = 0x%08x\r\n", pregctx->REG_uiR1);
        _PrintFormat("r2  = 0x%08x  ",   pregctx->REG_uiR2);
        _PrintFormat("r3  = 0x%08x\r\n", pregctx->REG_uiR3);
        _PrintFormat("r4  = 0x%08x  ",   pregctx->REG_uiR4);
        _PrintFormat("r5  = 0x%08x\r\n", pregctx->REG_uiR5);
        _PrintFormat("r6  = 0x%08x  ",   pregctx->REG_uiR6);
        _PrintFormat("r7  = 0x%08x\r\n", pregctx->REG_uiR7);
        _PrintFormat("r8  = 0x%08x  ",   pregctx->REG_uiR8);
        _PrintFormat("r9  = 0x%08x\r\n", pregctx->REG_uiR9);
        _PrintFormat("r10 = 0x%08x  ",   pregctx->REG_uiR10);
        _PrintFormat("fp  = 0x%08x\r\n", pregctx->REG_uiFp);
        _PrintFormat("ip  = 0x%08x  ",   pregctx->REG_uiIp);
        _PrintFormat("sp  = 0x%08x\r\n", pregctx->REG_uiSp);
        _PrintFormat("lr  = 0x%08x  ",   pregctx->REG_uiLr);
        _PrintFormat("pc  = 0x%08x\r\n", pregctx->REG_uiPc);
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
    ARCH_REG_CTX  *pregctx;

    if (pcpu->CPU_ulInterNesting == 1) {
        if (reg2 & (CORTEX_M_EXC_RETURN_MODE_MASK)) {
            pregctx = &pcpu->CPU_ptcbTCBCur->TCB_archRegCtx;

        } else {
            pregctx = (ARCH_REG_CTX *)(reg0 - sizeof(ARCH_REG_CTX));
        }
    } else {
        pregctx = (ARCH_REG_CTX *)(reg0 - sizeof(ARCH_REG_CTX));
    }

    pregctx->REG_uiSp      = reg0;
    pregctx->REG_uiBASEPRI = reg1;
    pregctx->REG_uiExcRet  = reg2;
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

#endif                                                                  /*  __SYLIXOS_ARM_ARCH_M__      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
