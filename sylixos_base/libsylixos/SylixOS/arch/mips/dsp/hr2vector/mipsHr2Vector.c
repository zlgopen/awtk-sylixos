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
** 文   件   名: mipsHr2Vector.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 02 月 24 日
**
** 描        述: 华睿 2 号处理器向量运算单元支持.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if defined(_MIPS_ARCH_HR2) && LW_CFG_CPU_DSP_EN > 0
#include "../mipsDsp.h"
#include "arch/mips/common/cp0/mipsCp0.h"
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static MIPS_DSP_OP      _G_dspopHr2Vector;
/*********************************************************************************************************
  外部函数声明
*********************************************************************************************************/
extern VOID  mipsHr2VectorInit(VOID);
extern VOID  mipsHr2VectorEnable(VOID);
extern VOID  mipsHr2VectorDisable(VOID);
extern BOOL  mipsHr2VectorIsEnable(VOID);
extern VOID  mipsHr2VectorSave(ARCH_DSP_CTX  *pdspctx);
extern VOID  mipsHr2VectorRestore(ARCH_DSP_CTX  *pdspctx);
/*********************************************************************************************************
** 函数名称: mipsHr2VectorCtxShow
** 功能描述: 显示 DSP 上下文
** 输　入  : iFd       输出文件描述符
**           pvDspCtx  DSP 上下文
** 输　出  :
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  mipsHr2VectorCtxShow (INT  iFd, ARCH_DSP_CTX  *pdspctx)
{
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)
    /*
     * Not in opensoure version
     */
#endif
}
/*********************************************************************************************************
** 函数名称: mipsHr2VectorEnableTask
** 功能描述: 系统发生 DSP 不可用异常时, 使能任务的 DSP
** 输　入  : ptcbCur    当前任务控制块
** 输　出  :
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsHr2VectorEnableTask (PLW_CLASS_TCB  ptcbCur)
{
    /*
     * Not in opensoure version
     */
}
/*********************************************************************************************************
** 函数名称: mipsHr2VectorPrimaryInit
** 功能描述: 获取 DSP 控制器操作函数集
** 输　入  : pcMachineName 机器名
**           pcDspName     DSP 运算器名
** 输　出  : 操作函数集
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
PMIPS_DSP_OP  mipsHr2VectorPrimaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    mipsHr2VectorInit();

    _G_dspopHr2Vector.MDSP_pfuncEnable     = mipsHr2VectorEnable;
    _G_dspopHr2Vector.MDSP_pfuncDisable    = mipsHr2VectorDisable;
    _G_dspopHr2Vector.MDSP_pfuncIsEnable   = mipsHr2VectorIsEnable;
    _G_dspopHr2Vector.MDSP_pfuncSave       = mipsHr2VectorSave;
    _G_dspopHr2Vector.MDSP_pfuncRestore    = mipsHr2VectorRestore;
    _G_dspopHr2Vector.MDSP_pfuncCtxShow    = mipsHr2VectorCtxShow;
    _G_dspopHr2Vector.MDSP_pfuncEnableTask = mipsHr2VectorEnableTask;

    return  (&_G_dspopHr2Vector);
}
/*********************************************************************************************************
** 函数名称: mipsHr2VectorSecondaryInit
** 功能描述: 初始化 DSP 控制器
** 输　入  : pcMachineName 机器名
**           pcDspName     DSP 运算器名
** 输　出  :
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  mipsHr2VectorSecondaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    mipsHr2VectorInit();
}

#endif                                                                  /*  defined(_MIPS_ARCH_HR2)     */
                                                                        /*  LW_CFG_CPU_DSP_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
