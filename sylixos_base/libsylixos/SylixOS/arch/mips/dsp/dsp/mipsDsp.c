/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: mipsDsp.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 01 月 10 日
**
** 描        述: MIPS 体系架构 DSP 支持.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_DSP_EN > 0
#include "../mipsDsp.h"
#include "arch/mips/common/cp0/mipsCp0.h"
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static MIPS_DSP_OP      _G_dspopDsp;

#define DSP_DEFAULT     0x00000000
#define DSP_MASK        0x3f
/*********************************************************************************************************
** 函数名称: mipsDspEnable
** 功能描述: 使能 DSP
** 输　入  :
** 输　出  :
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  mipsDspEnable (VOID)
{
    mipsCp0StatusWrite(mipsCp0StatusRead() | ST0_MX);
}
/*********************************************************************************************************
** 函数名称: mipsDspDisable
** 功能描述: 禁能 DSP
** 输　入  :
** 输　出  :
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  mipsDspDisable (VOID)
{
    mipsCp0StatusWrite(mipsCp0StatusRead() & (~ST0_MX));
}
/*********************************************************************************************************
** 函数名称: mipsDspIsEnable
** 功能描述: 是否使能了 DSP
** 输　入  :
** 输　出  : 是否使能
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static BOOL  mipsDspIsEnable (VOID)
{
    return  ((mipsCp0StatusRead() & ST0_MX) ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: mipsDspSave
** 功能描述: 保存 DSP 上下文
** 输　入  : pvDspCtx  DSP 上下文
** 输　出  :
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  mipsDspSave (ARCH_DSP_CTX  *pdspctx)
{
    UINT32  uiStatus = mipsCp0StatusRead();

    mipsCp0StatusWrite(uiStatus | ST0_MX);

    pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[0] = mfhi1();
    pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[1] = mflo1();
    pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[2] = mfhi2();
    pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[3] = mflo2();
    pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[4] = mfhi3();
    pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[5] = mflo3();
    pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspCtrl    = rddsp(DSP_MASK);

    mipsCp0StatusWrite(uiStatus);
}
/*********************************************************************************************************
** 函数名称: mipsDspRestore
** 功能描述: 恢复 DSP 上下文
** 输　入  : pvDspCtx  DSP 上下文
** 输　出  :
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  mipsDspRestore (ARCH_DSP_CTX  *pdspctx)
{
    UINT32  uiStatus = mipsCp0StatusRead();

    mipsCp0StatusWrite(uiStatus | ST0_MX);

    mthi1(pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[0]);
    mtlo1(pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[1]);
    mthi2(pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[2]);
    mtlo2(pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[3]);
    mthi3(pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[4]);
    mtlo3(pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[5]);
    wrdsp(pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspCtrl, DSP_MASK);

    mipsCp0StatusWrite(uiStatus);
}
/*********************************************************************************************************
** 函数名称: mipsDspCtxShow
** 功能描述: 显示 DSP 上下文
** 输　入  : iFd       输出文件描述符
**           pvDspCtx  DSP 上下文
** 输　出  :
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  mipsDspCtxShow (INT  iFd, ARCH_DSP_CTX  *pdspctx)
{
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)
    fdprintf(iFd, "hi1 = 0x%x, lo1 = 0x%x\n", pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[0],
                                              pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[1]);
    fdprintf(iFd, "hi2 = 0x%x, lo2 = 0x%x\n", pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[2],
                                              pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[3]);
    fdprintf(iFd, "hi3 = 0x%x, lo3 = 0x%x\n", pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[4],
                                              pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspRegs[5]);
    fdprintf(iFd, "dsp control = 0x%x\n",     pdspctx->DSPCTX_dspCtx.MIPSDSPCTX_dspCtrl);
#endif
}
/*********************************************************************************************************
** 函数名称: mipsDspEnableTask
** 功能描述: 系统发生 DSP 不可用异常时, 使能任务的 DSP
** 输　入  : ptcbCur    当前任务控制块
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsDspEnableTask (PLW_CLASS_TCB  ptcbCur)
{
    ARCH_REG_CTX  *pregctx;

    pregctx = &ptcbCur->TCB_archRegCtx;
    pregctx->REG_ulCP0Status |= ST0_MX;
}
/*********************************************************************************************************
** 函数名称: mipsDspPrimaryInit
** 功能描述: 获取 DSP 控制器操作函数集
** 输　入  : pcMachineName 机器名
**           pcDspName     DSP 运算器名
** 输　出  : 操作函数集
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  mipsDspInit (VOID)
{
    UINT32  uiStatus = mipsCp0StatusRead();

    mipsCp0StatusWrite(uiStatus | ST0_MX);

    mthi1(0);
    mtlo1(0);
    mthi2(0);
    mtlo2(0);
    mthi3(0);
    mtlo3(0);
    wrdsp(DSP_DEFAULT, DSP_MASK);

    mipsCp0StatusWrite(uiStatus);
}
/*********************************************************************************************************
** 函数名称: mipsDspPrimaryInit
** 功能描述: 获取 DSP 控制器操作函数集
** 输　入  : pcMachineName 机器名
**           pcDspName     DSP 运算器名
** 输　出  : 操作函数集
** 全局变量:
** 调用模块:
*********************************************************************************************************/
PMIPS_DSP_OP  mipsDspPrimaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    mipsDspInit();

    _G_dspopDsp.MDSP_pfuncEnable     = mipsDspEnable;
    _G_dspopDsp.MDSP_pfuncDisable    = mipsDspDisable;
    _G_dspopDsp.MDSP_pfuncIsEnable   = mipsDspIsEnable;
    _G_dspopDsp.MDSP_pfuncSave       = mipsDspSave;
    _G_dspopDsp.MDSP_pfuncRestore    = mipsDspRestore;
    _G_dspopDsp.MDSP_pfuncCtxShow    = mipsDspCtxShow;
    _G_dspopDsp.MDSP_pfuncEnableTask = mipsDspEnableTask;

    return  (&_G_dspopDsp);
}
/*********************************************************************************************************
** 函数名称: mipsDspSecondaryInit
** 功能描述: 初始化 DSP 控制器
** 输　入  : pcMachineName 机器名
**           pcDspName     DSP 运算器名
** 输　出  :
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  mipsDspSecondaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    mipsDspInit();
}

#endif                                                                  /*  LW_CFG_CPU_DSP_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
