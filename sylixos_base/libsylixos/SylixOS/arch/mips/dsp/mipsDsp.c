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
** 描        述: MIPS 体系架构 DSP.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_DSP_EN > 0
#include "mipsDsp.h"
#include "dsp/mipsDsp.h"
#include "dspnone/mipsDspNone.h"
#include "hr2vector/mipsHr2Vector.h"
#include "arch/mips/common/cp0/mipsCp0.h"
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static LW_DSP_CONTEXT   _G_dspCtxInit;
static PMIPS_DSP_OP     _G_pdspop;
/*********************************************************************************************************
** 函数名称: archDspPrimaryInit
** 功能描述: 主核 DSP 控制器初始化
** 输　入  : pcMachineName 机器名称
**           pcDspName     dsp 名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archDspPrimaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    _DebugFormat(__LOGMESSAGE_LEVEL, "%s %s %s DSP pri-core initialization.\r\n",
                 LW_CFG_CPU_ARCH_FAMILY, pcMachineName, pcDspName);

    if (lib_strcmp(pcDspName, MIPS_DSP_NONE) == 0) {                    /*  选择 DSP 架构               */
        _G_pdspop = mipsDspNonePrimaryInit(pcMachineName, pcDspName);

    } else if ((lib_strcmp(pcDspName, MIPS_DSP_V1) == 0) ||
               (lib_strcmp(pcDspName, MIPS_DSP_V2) == 0) ||
               (lib_strcmp(pcDspName, MIPS_DSP_V3) == 0)) {
        _G_pdspop = mipsDspPrimaryInit(pcMachineName, pcDspName);

#if defined(_MIPS_ARCH_HR2)
    } else if (lib_strcmp(pcDspName, MIPS_DSP_HR2_VECTOR) == 0) {
        _G_pdspop = mipsHr2VectorPrimaryInit(pcMachineName, pcDspName);
#endif                                                                  /*  defined(_MIPS_ARCH_HR2)     */

    } else {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "unknown dsp name.\r\n");
        return;
    }

    if (_G_pdspop == LW_NULL) {
        return;
    }

    lib_bzero(&_G_dspCtxInit, sizeof(LW_DSP_CONTEXT));

    MIPS_DSP_ENABLE(_G_pdspop);

    MIPS_DSP_SAVE(_G_pdspop, (PVOID)&_G_dspCtxInit);

    MIPS_DSP_DISABLE(_G_pdspop);
}
/*********************************************************************************************************
** 函数名称: archDspSecondaryInit
** 功能描述: 从核 DSP 控制器初始化
** 输　入  : pcMachineName 机器名称
**           pcDspName     dsp 名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

VOID  archDspSecondaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    _DebugFormat(__LOGMESSAGE_LEVEL, "%s %s %s DSP sec-core initialization.\r\n",
                 LW_CFG_CPU_ARCH_FAMILY, pcMachineName, pcDspName);

    if (lib_strcmp(pcDspName, MIPS_DSP_NONE) == 0) {                    /*  选择 DSP 架构               */
        mipsDspNoneSecondaryInit(pcMachineName, pcDspName);

    } else if ((lib_strcmp(pcDspName, MIPS_DSP_V1) == 0) ||
               (lib_strcmp(pcDspName, MIPS_DSP_V2) == 0) ||
               (lib_strcmp(pcDspName, MIPS_DSP_V3) == 0)) {
        mipsDspSecondaryInit(pcMachineName, pcDspName);

#if defined(_MIPS_ARCH_HR2)
    } else if (lib_strcmp(pcDspName, MIPS_DSP_HR2_VECTOR) == 0) {
        mipsHr2VectorSecondaryInit(pcMachineName, pcDspName);
#endif                                                                  /*  defined(_MIPS_ARCH_HR2)     */

    } else {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "unknown dsp name.\r\n");
        return;
    }

    MIPS_DSP_DISABLE(_G_pdspop);
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
** 函数名称: OSDspInitCtx
** 功能描述: 初始化一个 DSP 上下文控制块 (这里并没有使能 DSP)
** 输　入  : pvDspCtx   DSP 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archDspCtxInit (PVOID  pvDspCtx)
{
    lib_memcpy(pvDspCtx, &_G_dspCtxInit, sizeof(LW_DSP_CONTEXT));
}
/*********************************************************************************************************
** 函数名称: archDspEnable
** 功能描述: 使能 DSP.
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archDspEnable (VOID)
{
    MIPS_DSP_ENABLE(_G_pdspop);
}
/*********************************************************************************************************
** 函数名称: archDspDisable
** 功能描述: 禁能 DSP.
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archDspDisable (VOID)
{
    MIPS_DSP_DISABLE(_G_pdspop);
}
/*********************************************************************************************************
** 函数名称: archDspSave
** 功能描述: 保存 DSP 上下文.
** 输　入  : pvDspCtx  DSP 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archDspSave (PVOID  pvDspCtx)
{
    MIPS_DSP_SAVE(_G_pdspop, pvDspCtx);
}
/*********************************************************************************************************
** 函数名称: archDspRestore
** 功能描述: 恢复 DSP 上下文.
** 输　入  : pvDspCtx  DSP 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archDspRestore (PVOID  pvDspCtx)
{
    MIPS_DSP_RESTORE(_G_pdspop, pvDspCtx);
}
/*********************************************************************************************************
** 函数名称: archDspCtxShow
** 功能描述: 显示 DSP 上下文.
** 输　入  : iFd       文件描述符
**           pvDspCtx  DSP 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archDspCtxShow (INT  iFd, PVOID  pvDspCtx)
{
    MIPS_DSP_CTXSHOW(_G_pdspop, iFd, pvDspCtx);
}
/*********************************************************************************************************
** 函数名称: archDspUndHandle
** 功能描述: 系统发生 undef 异常时, 调用此函数.
**           只有某个任务或者中断, 真正使用 DSP 时 (即运行到 DSP 指令产生异常)
**           这时才可以打开 DSP.
** 输　入  : ptcbCur   当前任务 TCB
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  archDspUndHandle (PLW_CLASS_TCB  ptcbCur)
{
    if (MIPS_DSP_ISENABLE(_G_pdspop)) {                                 /*  如果当前上下文 DSP 使能     */
        return  (PX_ERROR);                                             /*  此未定义指令与 DSP 无关     */
    }

    MIPS_DSP_ENABLE_TASK(_G_pdspop, ptcbCur);                           /*  任务使能 DSP                */

    ptcbCur->TCB_ulOption |= LW_OPTION_THREAD_USED_DSP;
    MIPS_DSP_ENABLE(_G_pdspop);                                         /*  使能 DSP                    */

    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_CPU_DSP_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
