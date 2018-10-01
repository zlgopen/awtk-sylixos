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
** 文   件   名: KernelDsp.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 01 月 10 日
**
** 描        述: 初始化内核 DSP.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** 函数名称: API_KernelDspPrimaryInit
** 功能描述: 初始化 DSP (必须在 bsp 初始化回调中调用)
** 输　入  : pcMachineName 使用的处理器名称
**           pcDspName     使用的 DSP 名称.
** 输　出  : 
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
#if LW_CFG_CPU_DSP_EN > 0

LW_API
VOID  API_KernelDspPrimaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
#if LW_CFG_INTER_DSP > 0
    INT     i, j;
#endif                                                                  /*  LW_CFG_INTER_DSP > 0        */
    
    if (LW_SYS_STATUS_GET()) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "kernel already running.\r\n");
        return;
    }

    archDspPrimaryInit(pcMachineName, pcDspName);                       /*  初始化 DSP 单元             */
    
#if LW_CFG_INTER_DSP > 0
    if (LW_KERN_DSP_EN_GET()) {                                         /*  中断状态允许使用 DSP        */
        __ARCH_DSP_ENABLE();                                            /*  这里需要在当前 DSP 上下文中 */
                                                                        /*  使能 DSP, 操作系统内核需要  */
    } else 
#endif                                                                  /*  LW_CFG_INTER_DSP > 0        */
    {
        __ARCH_DSP_DISABLE();
    }
    
#if LW_CFG_INTER_DSP > 0
    for (i = 0; i < LW_CFG_MAX_PROCESSORS; i++) {
        for (j = 0; j < LW_CFG_MAX_INTER_SRC; j++) {                    /*  初始化, 但是不使能 DSP      */
            __ARCH_DSP_CTX_INIT((PVOID)&(LW_CPU_GET(i)->CPU_dspctxContext[j]));
        }
    }
#endif                                                                  /*  LW_CFG_INTER_DSP > 0        */
    
    _DebugHandle(__LOGMESSAGE_LEVEL, "DSP initilaized.\r\n");
}
/*********************************************************************************************************
** 函数名称: API_KernelDspSecondaryInit
** 功能描述: 初始化 DSP (必须在 bsp 初始化回调中调用)
** 输　入  : pcMachineName 使用的处理器名称
**           pcDspName     使用的 DSP 名称.
** 输　出  : 
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

LW_API
VOID  API_KernelDspSecondaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    archDspSecondaryInit(pcMachineName, pcDspName);
    
    _DebugHandle(__LOGMESSAGE_LEVEL, "secondary DSP initilaized.\r\n");
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
#endif                                                                  /*  LW_CFG_CPU_DSP_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
