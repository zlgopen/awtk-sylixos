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
** 文   件   名: x86Context.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 07 月 04 日
**
** 描        述: x86 体系构架上下文处理.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "x86Segment.h"
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

    pstkTop = (PLW_STACK)ROUND_DOWN(pstkTop, ARCH_STK_ALIGN_SIZE);      /*  保证出栈后 SP 8 字节对齐    */

    pfpctx  = (ARCH_FP_CTX *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX));

    pfpctx->FP_uiArg     = (ARCH_REG_T)pvArg;
    pfpctx->FP_uiRetAddr = (ARCH_REG_T)LW_NULL;

    pregctx->REG_uiEAX = 0xeaeaeaea;                                    /*  4 个数据寄存器              */
    pregctx->REG_uiEBX = 0xebebebeb;
    pregctx->REG_uiECX = 0xecececec;
    pregctx->REG_uiEDX = 0xedededed;

    pregctx->REG_uiESI = 0xe0e0e0e0;                                    /*  2 个变址和指针寄存器        */
    pregctx->REG_uiEDI = 0xe1e1e1e1;

    pregctx->REG_uiEBP = (ARCH_REG_T)pfpctx;                            /*  栈帧指针寄存器              */
    pregctx->REG_uiESP = (ARCH_REG_T)pfpctx;                            /*  堆栈指针寄存器              */

    pregctx->REG_uiError  = 0x00000000;                                 /*  ERROR CODE                  */
    pregctx->REG_uiEIP    = (ARCH_REG_T)pfuncTask;                      /*  指令指针寄存器(EIP)         */

    pregctx->REG_uiCS     = X86_CS_KERNEL;                              /*  代码段寄存器(CS)            */
    pregctx->REG_uiSS     = X86_DS_KERNEL;                              /*  代码段寄存器(SS)            */
    pregctx->REG_uiEFLAGS = X86_EFLAGS_IF;                              /*  标志寄存器设置中断使能位    */

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

    pregctxDest->REG_uiEBP = (ARCH_REG_T)pregctxSrc->REG_uiEBP;
    pfpctx->FP_uiRetAddr   = (ARCH_REG_T)pregctxSrc->REG_uiEIP;
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
    *pregSp = pregctx->REG_uiESP;

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
** 注  意  : 不修改段寄存器, EBP, ESP
*********************************************************************************************************/
VOID  archTaskRegsSet (ARCH_REG_CTX  *pregctxDest, const ARCH_REG_CTX  *pregctxSrc)
{
    pregctxDest->REG_uiEAX = pregctxSrc->REG_uiEAX;
    pregctxDest->REG_uiEBX = pregctxSrc->REG_uiEBX;
    pregctxDest->REG_uiECX = pregctxSrc->REG_uiECX;
    pregctxDest->REG_uiEDX = pregctxSrc->REG_uiEDX;

    pregctxDest->REG_uiESI = pregctxSrc->REG_uiESI;
    pregctxDest->REG_uiEDI = pregctxSrc->REG_uiEDI;

    pregctxDest->REG_uiEIP    = pregctxSrc->REG_uiEIP;
    pregctxDest->REG_uiEFLAGS = pregctxSrc->REG_uiEFLAGS;
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
        fdprintf(iFd, "\n");
        fdprintf(iFd, "EFLAGS = 0x%08x\n", pregctx->REG_uiEFLAGS);

        fdprintf(iFd, "EIP = 0x%08x\n", pregctx->REG_uiEIP);

        fdprintf(iFd, "CS  = 0x%08x  ", pregctx->REG_uiCS);
        fdprintf(iFd, "SS  = 0x%08x\n", pregctx->REG_uiSS);

        fdprintf(iFd, "EAX = 0x%08x  ", pregctx->REG_uiEAX);
        fdprintf(iFd, "EBX = 0x%08x\n", pregctx->REG_uiEBX);
        fdprintf(iFd, "ECX = 0x%08x  ", pregctx->REG_uiECX);
        fdprintf(iFd, "EDX = 0x%08x\n", pregctx->REG_uiEDX);

        fdprintf(iFd, "ESI = 0x%08x  ", pregctx->REG_uiESI);
        fdprintf(iFd, "EDI = 0x%08x\n", pregctx->REG_uiEDI);

        fdprintf(iFd, "EBP = 0x%08x  ", pregctx->REG_uiEBP);
        fdprintf(iFd, "ESP = 0x%08x\n", pregctx->REG_uiESP);

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

        stOft = bnprintf(pvBuffer, stSize, stOft, "EFLAGS = 0x%08x\n", pregctx->REG_uiEFLAGS);

        stOft = bnprintf(pvBuffer, stSize, stOft, "EIP = 0x%08x\n", pregctx->REG_uiEIP);

        stOft = bnprintf(pvBuffer, stSize, stOft, "CS  = 0x%08x  ", pregctx->REG_uiCS);
        stOft = bnprintf(pvBuffer, stSize, stOft, "SS  = 0x%08x\n", pregctx->REG_uiSS);

        stOft = bnprintf(pvBuffer, stSize, stOft, "EAX = 0x%08x  ", pregctx->REG_uiEAX);
        stOft = bnprintf(pvBuffer, stSize, stOft, "EBX = 0x%08x\n", pregctx->REG_uiEBX);
        stOft = bnprintf(pvBuffer, stSize, stOft, "ECX = 0x%08x  ", pregctx->REG_uiECX);
        stOft = bnprintf(pvBuffer, stSize, stOft, "EDX = 0x%08x\n", pregctx->REG_uiEDX);

        stOft = bnprintf(pvBuffer, stSize, stOft, "ESI = 0x%08x  ", pregctx->REG_uiESI);
        stOft = bnprintf(pvBuffer, stSize, stOft, "EDI = 0x%08x\n", pregctx->REG_uiEDI);

        stOft = bnprintf(pvBuffer, stSize, stOft, "EBP = 0x%08x  ", pregctx->REG_uiEBP);
        stOft = bnprintf(pvBuffer, stSize, stOft, "ESP = 0x%08x\n", pregctx->REG_uiESP);

    } else {
        _PrintFormat("\r\n");

        _PrintFormat("EFLAGS = 0x%08x\r\n", pregctx->REG_uiEFLAGS);

        _PrintFormat("EIP = 0x%08x\r\n", pregctx->REG_uiEIP);

        _PrintFormat("CS  = 0x%08x  ",   pregctx->REG_uiCS);
        _PrintFormat("SS  = 0x%08x\r\n", pregctx->REG_uiSS);

        _PrintFormat("EAX = 0x%08x  ",   pregctx->REG_uiEAX);
        _PrintFormat("EBX = 0x%08x\r\n", pregctx->REG_uiEBX);
        _PrintFormat("ECX = 0x%08x  ",   pregctx->REG_uiECX);
        _PrintFormat("EDX = 0x%08x\r\n", pregctx->REG_uiEDX);

        _PrintFormat("ESI = 0x%08x  ",   pregctx->REG_uiESI);
        _PrintFormat("EDI = 0x%08x\r\n", pregctx->REG_uiEDI);

        _PrintFormat("EBP = 0x%08x  ",   pregctx->REG_uiEBP);
        _PrintFormat("ESP = 0x%08x\r\n", pregctx->REG_uiESP);
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
    ARCH_REG_T     iD0, iD1, iD2;

#define COPY_CTX(to, from, cnt)                                             \
    __asm__ __volatile__ ("cld\n\t"                                         \
                          "rep ; movsl"                                     \
                          : "=&c" (iD0), "=&D" (iD1), "=&S" (iD2)           \
                          : "0" (cnt), "1" ((LONG)to), "2" ((LONG)from)     \
                          : "memory")

    if (pcpu->CPU_ulInterNesting == 1) {
        pregctx = (ARCH_REG_CTX *)reg0;
        if (pregctx->REG_uiCS == X86_CS_USER) {
            COPY_CTX(&pcpu->CPU_ptcbTCBCur->TCB_archRegCtx, pregctx, ARCH_REG_CTX_WORD_SIZE);

        } else {
            COPY_CTX(&pcpu->CPU_ptcbTCBCur->TCB_archRegCtx, pregctx, ARCH_REG_CTX_WORD_SIZE - 2);
            pregctx = &pcpu->CPU_ptcbTCBCur->TCB_archRegCtx;
            pregctx->REG_uiSS  = X86_DS_KERNEL;
            pregctx->REG_uiESP = reg0 + sizeof(ARCH_REG_CTX) - 2 * sizeof(ARCH_REG_T);
        }
    }
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
    return  ((PLW_STACK)pregctx->REG_uiESP);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
