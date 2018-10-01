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
** 文   件   名: mipsContext.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: MIPS 体系架构上下文处理.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "arch/mips/common/cp0/mipsCp0.h"
/*********************************************************************************************************
  外部函数声明
*********************************************************************************************************/
extern ARCH_REG_T  archGetGP(VOID);
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
    ARCH_REG_T          ulCP0Status;
    INT                 i;

    ulCP0Status  = mipsCp0StatusRead();                                 /*  获得当前的 CP0 STATUS 寄存器*/
    ulCP0Status |= bspIntInitEnableStatus() | ST0_IE;                   /*  使能中断                    */
    ulCP0Status |=  ST0_CU0;                                            /*  使能 CU0                    */
    ulCP0Status &= ~ST0_CU1;                                            /*  禁能 CU1(FPU)               */
    ulCP0Status &= ~ST0_CU2;                                            /*  禁能 CU2                    */
    ulCP0Status &= ~ST0_CU3;                                            /*  禁能 CU3                    */
    ulCP0Status &= ~ST0_MX;                                             /*  禁能 MDMX ASE 或 DSP        */

    pstkTop = (PLW_STACK)ROUND_DOWN(pstkTop, ARCH_STK_ALIGN_SIZE);      /*  保证出栈后 SP 8/16 字节对齐 */

    pfpctx  = (ARCH_FP_CTX *)((PCHAR)pstkTop - sizeof(ARCH_FP_CTX));

    /*
     * 初始化预留的参数寄存器栈空间
     */
    for (i = 0; i < ARCH_ARG_REG_NR; i++) {
        pfpctx->FP_ulArg[i] = i;
    }

    /*
     * 初始化寄存器上下文
     */
    for (i = 0; i < ARCH_GREG_NR; i++) {
        pregctx->REG_ulReg[i] = i;
    }

    pregctx->REG_ulReg[REG_A0] = (ARCH_REG_T)pvArg;
    pregctx->REG_ulReg[REG_GP] = (ARCH_REG_T)archGetGP();               /*  获得 GP 寄存器的值          */
    pregctx->REG_ulReg[REG_FP] = (ARCH_REG_T)pfpctx;
    pregctx->REG_ulReg[REG_RA] = (ARCH_REG_T)0x0;
    pregctx->REG_ulReg[REG_SP] = (ARCH_REG_T)pfpctx;

    pregctx->REG_ulCP0Status   = (ARCH_REG_T)ulCP0Status;
    pregctx->REG_ulCP0EPC      = (ARCH_REG_T)pfuncTask;
    pregctx->REG_ulCP0Cause    = (ARCH_REG_T)0x0;
    pregctx->REG_ulCP0DataLO   = (ARCH_REG_T)0x0;
    pregctx->REG_ulCP0DataHI   = (ARCH_REG_T)0x0;
    pregctx->REG_ulCP0BadVAddr = (ARCH_REG_T)0x0;

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
    pregctxDest->REG_ulReg[REG_FP] = (ARCH_REG_T)pregctxSrc->REG_ulReg[REG_SP];
    pregctxDest->REG_ulReg[REG_RA] = (ARCH_REG_T)pregctxSrc->REG_ulCP0EPC;
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
    *pregSp = pregctx->REG_ulReg[REG_SP];

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
    /*
     * $0(ZERO)不设置
     */
    pregctxDest->REG_ulReg[1]  = pregctxSrc->REG_ulReg[1];
    pregctxDest->REG_ulReg[2]  = pregctxSrc->REG_ulReg[2];
    pregctxDest->REG_ulReg[3]  = pregctxSrc->REG_ulReg[3];
    pregctxDest->REG_ulReg[4]  = pregctxSrc->REG_ulReg[4];
    pregctxDest->REG_ulReg[5]  = pregctxSrc->REG_ulReg[5];
    pregctxDest->REG_ulReg[6]  = pregctxSrc->REG_ulReg[6];
    pregctxDest->REG_ulReg[7]  = pregctxSrc->REG_ulReg[7];
    pregctxDest->REG_ulReg[8]  = pregctxSrc->REG_ulReg[8];
    pregctxDest->REG_ulReg[9]  = pregctxSrc->REG_ulReg[9];
    pregctxDest->REG_ulReg[10] = pregctxSrc->REG_ulReg[10];
    pregctxDest->REG_ulReg[11] = pregctxSrc->REG_ulReg[11];
    pregctxDest->REG_ulReg[12] = pregctxSrc->REG_ulReg[12];
    pregctxDest->REG_ulReg[13] = pregctxSrc->REG_ulReg[13];
    pregctxDest->REG_ulReg[14] = pregctxSrc->REG_ulReg[14];
    pregctxDest->REG_ulReg[15] = pregctxSrc->REG_ulReg[15];
    pregctxDest->REG_ulReg[16] = pregctxSrc->REG_ulReg[16];
    pregctxDest->REG_ulReg[17] = pregctxSrc->REG_ulReg[17];
    pregctxDest->REG_ulReg[18] = pregctxSrc->REG_ulReg[18];
    pregctxDest->REG_ulReg[19] = pregctxSrc->REG_ulReg[19];
    pregctxDest->REG_ulReg[20] = pregctxSrc->REG_ulReg[20];
    pregctxDest->REG_ulReg[21] = pregctxSrc->REG_ulReg[21];
    pregctxDest->REG_ulReg[22] = pregctxSrc->REG_ulReg[22];
    pregctxDest->REG_ulReg[23] = pregctxSrc->REG_ulReg[23];
    pregctxDest->REG_ulReg[24] = pregctxSrc->REG_ulReg[24];
    pregctxDest->REG_ulReg[25] = pregctxSrc->REG_ulReg[25];
    /*
     * $26 $27(K0 K1)不设置
     * $28 $29 $30(GP SP FP)不设置, 保持原值
     */
    pregctxDest->REG_ulReg[31] = pregctxSrc->REG_ulReg[31];

    pregctxDest->REG_ulCP0DataLO = pregctxSrc->REG_ulCP0DataLO;         /*  除数低位寄存器              */
    pregctxDest->REG_ulCP0DataHI = pregctxSrc->REG_ulCP0DataHI;         /*  除数高位寄存器              */
    pregctxDest->REG_ulCP0Status = pregctxSrc->REG_ulCP0Status;         /*  CP0 STATUS 寄存器           */
    pregctxDest->REG_ulCP0EPC    = pregctxSrc->REG_ulCP0EPC;            /*  程序计数器寄存器            */
    /*
     * CP0 的 Cause BadVAddr 寄存器不设置, 保持原值
     */
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
    ARCH_REG_T  ulCP0Status = pregctx->REG_ulCP0Status;

#if LW_CFG_CPU_WORD_LENGHT == 32
#define LX_FMT      "0x%08x"
#else
#define LX_FMT      "0x%016lx"
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/

    if (iFd >= 0) {
        fdprintf(iFd, "\n");

        fdprintf(iFd, "EPC       = "LX_FMT"\n", pregctx->REG_ulCP0EPC);
        fdprintf(iFd, "BADVADDR  = "LX_FMT"\n", pregctx->REG_ulCP0BadVAddr);
        fdprintf(iFd, "CAUSE     = "LX_FMT"\n", pregctx->REG_ulCP0Cause);
        fdprintf(iFd, "LO        = "LX_FMT"\n", pregctx->REG_ulCP0DataLO);
        fdprintf(iFd, "HI        = "LX_FMT"\n", pregctx->REG_ulCP0DataHI);

        fdprintf(iFd, "$00(ZERO) = "LX_FMT"\n", pregctx->REG_ulReg[REG_ZERO]);
        fdprintf(iFd, "$01(AT)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_AT]);
        fdprintf(iFd, "$02(V0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_V0]);
        fdprintf(iFd, "$03(V1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_V1]);
        fdprintf(iFd, "$04(A0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A0]);
        fdprintf(iFd, "$05(A1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A1]);
        fdprintf(iFd, "$06(A2)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A2]);
        fdprintf(iFd, "$07(A3)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A3]);

#if LW_CFG_CPU_WORD_LENGHT == 32
        fdprintf(iFd, "$08(T0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T0]);
        fdprintf(iFd, "$09(T1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T1]);
        fdprintf(iFd, "$10(T2)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T2]);
        fdprintf(iFd, "$11(T3)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T3]);
        fdprintf(iFd, "$12(T4)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T4]);
        fdprintf(iFd, "$13(T5)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T5]);
        fdprintf(iFd, "$14(T6)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T6]);
        fdprintf(iFd, "$15(T7)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T7]);
#else
        fdprintf(iFd, "$08(A4)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A4]);
        fdprintf(iFd, "$09(A5)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A5]);
        fdprintf(iFd, "$10(A6)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A6]);
        fdprintf(iFd, "$11(A7)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A7]);
        fdprintf(iFd, "$12(T0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T0]);
        fdprintf(iFd, "$13(T1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T1]);
        fdprintf(iFd, "$14(T2)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T2]);
        fdprintf(iFd, "$15(T3)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T3]);
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/

        fdprintf(iFd, "$16(S0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S0]);
        fdprintf(iFd, "$17(S1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S1]);
        fdprintf(iFd, "$18(S2)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S2]);
        fdprintf(iFd, "$19(S3)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S3]);
        fdprintf(iFd, "$20(S4)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S4]);
        fdprintf(iFd, "$21(S5)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S5]);
        fdprintf(iFd, "$22(S6)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S6]);
        fdprintf(iFd, "$23(S7)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S7]);
        fdprintf(iFd, "$24(T8)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T8]);
        fdprintf(iFd, "$25(T9)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T9]);
        fdprintf(iFd, "$28(GP)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_GP]);
        fdprintf(iFd, "$29(SP)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_SP]);
        fdprintf(iFd, "$30(FP)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_FP]);
        fdprintf(iFd, "$31(RA)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_RA]);

        fdprintf(iFd, "CP0 Status Register:\n");
        fdprintf(iFd, "CU3 = %d  ", (ulCP0Status & M_StatusCU3) >> S_StatusCU3);
        fdprintf(iFd, "CU2 = %d\n", (ulCP0Status & M_StatusCU2) >> S_StatusCU2);
        fdprintf(iFd, "CU1 = %d  ", (ulCP0Status & M_StatusCU1) >> S_StatusCU1);
        fdprintf(iFd, "CU0 = %d\n", (ulCP0Status & M_StatusCU0) >> S_StatusCU0);
        fdprintf(iFd, "RP  = %d  ", (ulCP0Status & M_StatusRP)  >> S_StatusRP);
        fdprintf(iFd, "FR  = %d\n", (ulCP0Status & M_StatusFR)  >> S_StatusFR);
        fdprintf(iFd, "RE  = %d  ", (ulCP0Status & M_StatusRE)  >> S_StatusRE);
        fdprintf(iFd, "MX  = %d\n", (ulCP0Status & M_StatusMX)  >> S_StatusMX);
        fdprintf(iFd, "PX  = %d  ", (ulCP0Status & M_StatusPX)  >> S_StatusPX);
        fdprintf(iFd, "BEV = %d\n", (ulCP0Status & M_StatusBEV) >> S_StatusBEV);
        fdprintf(iFd, "TS  = %d  ", (ulCP0Status & M_StatusTS)  >> S_StatusTS);
        fdprintf(iFd, "SR  = %d\n", (ulCP0Status & M_StatusSR)  >> S_StatusSR);
        fdprintf(iFd, "NMI = %d  ", (ulCP0Status & M_StatusNMI) >> S_StatusNMI);
        fdprintf(iFd, "IM7 = %d\n", (ulCP0Status & M_StatusIM7) >> S_StatusIM7);
        fdprintf(iFd, "IM6 = %d  ", (ulCP0Status & M_StatusIM6) >> S_StatusIM6);
        fdprintf(iFd, "IM5 = %d\n", (ulCP0Status & M_StatusIM5) >> S_StatusIM5);
        fdprintf(iFd, "IM4 = %d  ", (ulCP0Status & M_StatusIM4) >> S_StatusIM4);
        fdprintf(iFd, "IM3 = %d\n", (ulCP0Status & M_StatusIM3) >> S_StatusIM3);
        fdprintf(iFd, "IM2 = %d  ", (ulCP0Status & M_StatusIM2) >> S_StatusIM2);
        fdprintf(iFd, "IM1 = %d\n", (ulCP0Status & M_StatusIM1) >> S_StatusIM1);
        fdprintf(iFd, "IM0 = %d  ", (ulCP0Status & M_StatusIM0) >> S_StatusIM0);
        fdprintf(iFd, "KX  = %d\n", (ulCP0Status & M_StatusKX)  >> S_StatusKX);
        fdprintf(iFd, "SX  = %d  ", (ulCP0Status & M_StatusSX)  >> S_StatusSX);
        fdprintf(iFd, "UX  = %d\n", (ulCP0Status & M_StatusUX)  >> S_StatusUX);
        fdprintf(iFd, "KSU = %d  ", (ulCP0Status & M_StatusKSU) >> S_StatusKSU);
        fdprintf(iFd, "UM  = %d\n", (ulCP0Status & M_StatusUM)  >> S_StatusUM);
        fdprintf(iFd, "SM  = %d  ", (ulCP0Status & M_StatusSM)  >> S_StatusSM);
        fdprintf(iFd, "ERL = %d\n", (ulCP0Status & M_StatusERL) >> S_StatusERL);
        fdprintf(iFd, "EXL = %d  ", (ulCP0Status & M_StatusEXL) >> S_StatusEXL);
        fdprintf(iFd, "IE  = %d\n", (ulCP0Status & M_StatusIE)  >> S_StatusIE);

    } else {
        archTaskCtxPrint(LW_NULL, 0, pregctx);
    }

#undef LX_FMT
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
    ARCH_REG_T  ulCP0Status = pregctx->REG_ulCP0Status;

    if (pvBuffer && stSize) {
#if LW_CFG_CPU_WORD_LENGHT == 32
#define LX_FMT      "0x%08x"
#else
#define LX_FMT      "0x%016lx"
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/
        size_t  stOft = 0;

        stOft = bnprintf(pvBuffer, stSize, stOft, "EPC       = "LX_FMT"\n", pregctx->REG_ulCP0EPC);
        stOft = bnprintf(pvBuffer, stSize, stOft, "BADVADDR  = "LX_FMT"\n", pregctx->REG_ulCP0BadVAddr);
        stOft = bnprintf(pvBuffer, stSize, stOft, "CAUSE     = "LX_FMT"\n", pregctx->REG_ulCP0Cause);
        stOft = bnprintf(pvBuffer, stSize, stOft, "LO        = "LX_FMT"\n", pregctx->REG_ulCP0DataLO);
        stOft = bnprintf(pvBuffer, stSize, stOft, "HI        = "LX_FMT"\n", pregctx->REG_ulCP0DataHI);

        stOft = bnprintf(pvBuffer, stSize, stOft, "$00(ZERO) = "LX_FMT"\n", pregctx->REG_ulReg[REG_ZERO]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$01(AT)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_AT]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$02(V0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_V0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$03(V1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_V1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$04(A0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$05(A1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$06(A2)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A2]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$07(A3)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A3]);
#if LW_CFG_CPU_WORD_LENGHT == 32
        stOft = bnprintf(pvBuffer, stSize, stOft, "$08(T0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$09(T1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$10(T2)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T2]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$11(T3)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T3]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$12(T4)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T4]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$13(T5)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T5]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$14(T6)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T6]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$15(T7)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T7]);
#else
        stOft = bnprintf(pvBuffer, stSize, stOft, "$08(A4)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A4]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$09(A5)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A5]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$10(A6)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A6]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$11(A7)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_A7]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$12(T0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$13(T1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$14(T2)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T2]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$15(T3)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T3]);
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/
        stOft = bnprintf(pvBuffer, stSize, stOft, "$16(S0)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S0]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$17(S1)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S1]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$18(S2)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S2]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$19(S3)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S3]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$20(S4)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S4]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$21(S5)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S5]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$22(S6)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S6]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$23(S7)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_S7]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$24(T8)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T8]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$25(T9)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_T9]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$28(GP)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_GP]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$29(SP)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_SP]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$30(FP)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_FP]);
        stOft = bnprintf(pvBuffer, stSize, stOft, "$31(RA)   = "LX_FMT"\n", pregctx->REG_ulReg[REG_RA]);

        stOft = bnprintf(pvBuffer, stSize, stOft, "CP0 Status Register:\n");
        stOft = bnprintf(pvBuffer, stSize, stOft, "CU3 = %d  ", (ulCP0Status & M_StatusCU3) >> S_StatusCU3);
        stOft = bnprintf(pvBuffer, stSize, stOft, "CU2 = %d\n", (ulCP0Status & M_StatusCU2) >> S_StatusCU2);
        stOft = bnprintf(pvBuffer, stSize, stOft, "CU1 = %d  ", (ulCP0Status & M_StatusCU1) >> S_StatusCU1);
        stOft = bnprintf(pvBuffer, stSize, stOft, "CU0 = %d\n", (ulCP0Status & M_StatusCU0) >> S_StatusCU0);
        stOft = bnprintf(pvBuffer, stSize, stOft, "RP  = %d  ", (ulCP0Status & M_StatusRP)  >> S_StatusRP);
        stOft = bnprintf(pvBuffer, stSize, stOft, "FR  = %d\n", (ulCP0Status & M_StatusFR)  >> S_StatusFR);
        stOft = bnprintf(pvBuffer, stSize, stOft, "RE  = %d  ", (ulCP0Status & M_StatusRE)  >> S_StatusRE);
        stOft = bnprintf(pvBuffer, stSize, stOft, "MX  = %d\n", (ulCP0Status & M_StatusMX)  >> S_StatusMX);
        stOft = bnprintf(pvBuffer, stSize, stOft, "PX  = %d  ", (ulCP0Status & M_StatusPX)  >> S_StatusPX);
        stOft = bnprintf(pvBuffer, stSize, stOft, "BEV = %d\n", (ulCP0Status & M_StatusBEV) >> S_StatusBEV);
        stOft = bnprintf(pvBuffer, stSize, stOft, "TS  = %d  ", (ulCP0Status & M_StatusTS)  >> S_StatusTS);
        stOft = bnprintf(pvBuffer, stSize, stOft, "SR  = %d\n", (ulCP0Status & M_StatusSR)  >> S_StatusSR);
        stOft = bnprintf(pvBuffer, stSize, stOft, "NMI = %d  ", (ulCP0Status & M_StatusNMI) >> S_StatusNMI);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IM7 = %d\n", (ulCP0Status & M_StatusIM7) >> S_StatusIM7);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IM6 = %d  ", (ulCP0Status & M_StatusIM6) >> S_StatusIM6);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IM5 = %d\n", (ulCP0Status & M_StatusIM5) >> S_StatusIM5);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IM4 = %d  ", (ulCP0Status & M_StatusIM4) >> S_StatusIM4);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IM3 = %d\n", (ulCP0Status & M_StatusIM3) >> S_StatusIM3);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IM2 = %d  ", (ulCP0Status & M_StatusIM2) >> S_StatusIM2);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IM1 = %d\n", (ulCP0Status & M_StatusIM1) >> S_StatusIM1);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IM0 = %d  ", (ulCP0Status & M_StatusIM0) >> S_StatusIM0);
        stOft = bnprintf(pvBuffer, stSize, stOft, "KX  = %d\n", (ulCP0Status & M_StatusKX)  >> S_StatusKX);
        stOft = bnprintf(pvBuffer, stSize, stOft, "SX  = %d  ", (ulCP0Status & M_StatusSX)  >> S_StatusSX);
        stOft = bnprintf(pvBuffer, stSize, stOft, "UX  = %d\n", (ulCP0Status & M_StatusUX)  >> S_StatusUX);
        stOft = bnprintf(pvBuffer, stSize, stOft, "KSU = %d  ", (ulCP0Status & M_StatusKSU) >> S_StatusKSU);
        stOft = bnprintf(pvBuffer, stSize, stOft, "UM  = %d\n", (ulCP0Status & M_StatusUM)  >> S_StatusUM);
        stOft = bnprintf(pvBuffer, stSize, stOft, "SM  = %d  ", (ulCP0Status & M_StatusSM)  >> S_StatusSM);
        stOft = bnprintf(pvBuffer, stSize, stOft, "ERL = %d\n", (ulCP0Status & M_StatusERL) >> S_StatusERL);
        stOft = bnprintf(pvBuffer, stSize, stOft, "EXL = %d  ", (ulCP0Status & M_StatusEXL) >> S_StatusEXL);
        stOft = bnprintf(pvBuffer, stSize, stOft, "IE  = %d\n", (ulCP0Status & M_StatusIE)  >> S_StatusIE);

#undef LX_FMT
    } else {
#if LW_CFG_CPU_WORD_LENGHT == 32
#define LX_FMT      "0x%08x"
#else
#define LX_FMT      "0x%016qx"
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/

        _PrintFormat("\r\n");

        _PrintFormat("EPC       = "LX_FMT"\r\n", pregctx->REG_ulCP0EPC);
        _PrintFormat("BADVADDR  = "LX_FMT"\r\n", pregctx->REG_ulCP0BadVAddr);
        _PrintFormat("CAUSE     = "LX_FMT"\r\n", pregctx->REG_ulCP0Cause);
        _PrintFormat("LO        = "LX_FMT"\r\n", pregctx->REG_ulCP0DataLO);
        _PrintFormat("HI        = "LX_FMT"\r\n", pregctx->REG_ulCP0DataHI);

        _PrintFormat("$00(ZERO) = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_ZERO]);
        _PrintFormat("$01(AT)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_AT]);
        _PrintFormat("$02(V0)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_V0]);
        _PrintFormat("$03(V1)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_V1]);
        _PrintFormat("$04(A0)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_A0]);
        _PrintFormat("$05(A1)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_A1]);
        _PrintFormat("$06(A2)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_A2]);
        _PrintFormat("$07(A3)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_A3]);

#if LW_CFG_CPU_WORD_LENGHT == 32
        _PrintFormat("$08(T0)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T0]);
        _PrintFormat("$09(T1)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T1]);
        _PrintFormat("$10(T2)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T2]);
        _PrintFormat("$11(T3)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T3]);
        _PrintFormat("$12(T4)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T4]);
        _PrintFormat("$13(T5)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T5]);
        _PrintFormat("$14(T6)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T6]);
        _PrintFormat("$15(T7)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T7]);
#else
        _PrintFormat("$08(A4)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_A4]);
        _PrintFormat("$09(A5)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_A5]);
        _PrintFormat("$10(A6)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_A6]);
        _PrintFormat("$11(A7)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_A7]);
        _PrintFormat("$12(T0)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T0]);
        _PrintFormat("$13(T1)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T1]);
        _PrintFormat("$14(T2)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T2]);
        _PrintFormat("$15(T3)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T3]);
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/

        _PrintFormat("$16(S0)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_S0]);
        _PrintFormat("$17(S1)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_S1]);
        _PrintFormat("$18(S2)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_S2]);
        _PrintFormat("$19(S3)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_S3]);
        _PrintFormat("$20(S4)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_S4]);
        _PrintFormat("$21(S5)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_S5]);
        _PrintFormat("$22(S6)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_S6]);
        _PrintFormat("$23(S7)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_S7]);
        _PrintFormat("$24(T8)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T8]);
        _PrintFormat("$25(T9)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_T9]);
        _PrintFormat("$28(GP)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_GP]);
        _PrintFormat("$29(SP)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_SP]);
        _PrintFormat("$30(FP)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_FP]);
        _PrintFormat("$31(RA)   = "LX_FMT"\r\n", pregctx->REG_ulReg[REG_RA]);

        _PrintFormat("CP0 Status Register:\r\n");
        _PrintFormat("CU3 = %d  ",   (ulCP0Status & M_StatusCU3) >> S_StatusCU3);
        _PrintFormat("CU2 = %d\r\n", (ulCP0Status & M_StatusCU2) >> S_StatusCU2);
        _PrintFormat("CU1 = %d  ",   (ulCP0Status & M_StatusCU1) >> S_StatusCU1);
        _PrintFormat("CU0 = %d\r\n", (ulCP0Status & M_StatusCU0) >> S_StatusCU0);
        _PrintFormat("RP  = %d  ",   (ulCP0Status & M_StatusRP)  >> S_StatusRP);
        _PrintFormat("FR  = %d\r\n", (ulCP0Status & M_StatusFR)  >> S_StatusFR);
        _PrintFormat("RE  = %d  ",   (ulCP0Status & M_StatusRE)  >> S_StatusRE);
        _PrintFormat("MX  = %d\r\n", (ulCP0Status & M_StatusMX)  >> S_StatusMX);
        _PrintFormat("PX  = %d  ",   (ulCP0Status & M_StatusPX)  >> S_StatusPX);
        _PrintFormat("BEV = %d\r\n", (ulCP0Status & M_StatusBEV) >> S_StatusBEV);
        _PrintFormat("TS  = %d  ",   (ulCP0Status & M_StatusTS)  >> S_StatusTS);
        _PrintFormat("SR  = %d\r\n", (ulCP0Status & M_StatusSR)  >> S_StatusSR);
        _PrintFormat("NMI = %d  ",   (ulCP0Status & M_StatusNMI) >> S_StatusNMI);
        _PrintFormat("IM7 = %d\r\n", (ulCP0Status & M_StatusIM7) >> S_StatusIM7);
        _PrintFormat("IM6 = %d  ",   (ulCP0Status & M_StatusIM6) >> S_StatusIM6);
        _PrintFormat("IM5 = %d\r\n", (ulCP0Status & M_StatusIM5) >> S_StatusIM5);
        _PrintFormat("IM4 = %d  ",   (ulCP0Status & M_StatusIM4) >> S_StatusIM4);
        _PrintFormat("IM3 = %d\r\n", (ulCP0Status & M_StatusIM3) >> S_StatusIM3);
        _PrintFormat("IM2 = %d  ",   (ulCP0Status & M_StatusIM2) >> S_StatusIM2);
        _PrintFormat("IM1 = %d\r\n", (ulCP0Status & M_StatusIM1) >> S_StatusIM1);
        _PrintFormat("IM0 = %d  ",   (ulCP0Status & M_StatusIM0) >> S_StatusIM0);
        _PrintFormat("KX  = %d\r\n", (ulCP0Status & M_StatusKX)  >> S_StatusKX);
        _PrintFormat("SX  = %d  ",   (ulCP0Status & M_StatusSX)  >> S_StatusSX);
        _PrintFormat("UX  = %d\r\n", (ulCP0Status & M_StatusUX)  >> S_StatusUX);
        _PrintFormat("KSU = %d  ",   (ulCP0Status & M_StatusKSU) >> S_StatusKSU);
        _PrintFormat("UM  = %d\r\n", (ulCP0Status & M_StatusUM)  >> S_StatusUM);
        _PrintFormat("SM  = %d  ",   (ulCP0Status & M_StatusSM)  >> S_StatusSM);
        _PrintFormat("ERL = %d\r\n", (ulCP0Status & M_StatusERL) >> S_StatusERL);
        _PrintFormat("EXL = %d  ",   (ulCP0Status & M_StatusEXL) >> S_StatusEXL);
        _PrintFormat("IE  = %d\r\n", (ulCP0Status & M_StatusIE)  >> S_StatusIE);

#undef LX_FMT
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
        pregctx = &pcpu->CPU_ptcbTCBCur->TCB_archRegCtx;

    } else {
        pregctx = (ARCH_REG_CTX *)(((ARCH_REG_CTX *)reg0)->REG_ulReg[REG_SP] - ARCH_REG_CTX_SIZE);
    }

    archTaskCtxCopy(pregctx, (ARCH_REG_CTX *)reg0);
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
    return  ((PLW_STACK)pregctx->REG_ulReg[REG_SP]);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
