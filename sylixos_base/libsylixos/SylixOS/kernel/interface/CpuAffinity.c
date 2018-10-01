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
** 文   件   名: CpuAffinity.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2018 年 06 月 06 日
**
** 描        述: SMP 系统设置 CPU 为强亲和度调度模式.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  裁剪控制
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
/*********************************************************************************************************
** 函数名称: API_CpuSetSchedAffinity
** 功能描述: 设置指定 CPU 为强亲和度调度模式. (非 0 号 CPU)
** 输　入  : stSize        CPU 掩码集内存大小
**           pcpuset       CPU 掩码
** 输　出  : ERROR
** 全局变量: 
** 调用模块: 
** 注  意  : 不能设置
                                           API 函数
*********************************************************************************************************/
LW_API 
ULONG  API_CpuSetSchedAffinity (size_t  stSize, const PLW_CLASS_CPUSET  pcpuset)
{
    INTREG  iregInterLevel;
    ULONG   i;
    ULONG   ulNumChk;

    if (!LW_SYS_STATUS_IS_RUNNING()) {                                  /*  系统必须已经启动            */
        _ErrorHandle(ERROR_KERNEL_NOT_RUNNING);
        return  (ERROR_KERNEL_NOT_RUNNING);
    }
    
    if (!stSize || !pcpuset) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }

    ulNumChk = ((ULONG)stSize << 3);
    ulNumChk = (ulNumChk > LW_NCPUS) ? LW_NCPUS : ulNumChk;
    
    for (i = 1; i < ulNumChk; i++) {                                    /*  CPU 0 不能设置              */
        iregInterLevel = __KERNEL_ENTER_IRQ();                          /*  进入内核                    */
        if (LW_CPU_ISSET(i, pcpuset)) {
            _CpuSetSchedAffinity(i, LW_TRUE);
        
        } else {
            _CpuSetSchedAffinity(i, LW_FALSE);
        }
        __KERNEL_EXIT_IRQ(iregInterLevel);                              /*  退出内核                    */
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_CpuGetSchedAffinity
** 功能描述: 获取指定 CPU 是否为强亲和度调度模式.
** 输　入  : stSize        CPU 掩码集内存大小
**           pcpuset       CPU 掩码
** 输　出  : ERROR
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
ULONG  API_CpuGetSchedAffinity (size_t  stSize, PLW_CLASS_CPUSET  pcpuset)
{
    INTREG  iregInterLevel;
    BOOL    bEnable;
    ULONG   i;
    ULONG   ulNumChk;

    if (!LW_SYS_STATUS_IS_RUNNING()) {                                  /*  系统必须已经启动            */
        _ErrorHandle(ERROR_KERNEL_NOT_RUNNING);
        return  (ERROR_KERNEL_NOT_RUNNING);
    }
    
    if (!stSize || !pcpuset) {
        _ErrorHandle(EINVAL);
        return  (EINVAL);
    }

    ulNumChk = ((ULONG)stSize << 3);
    ulNumChk = (ulNumChk > LW_NCPUS) ? LW_NCPUS : ulNumChk;
    
    LW_CPU_ZERO(pcpuset);
    
    for (i = 1; i < ulNumChk; i++) {                                    /*  CPU 0 不能设置              */
        iregInterLevel = __KERNEL_ENTER_IRQ();                          /*  进入内核                    */
        _CpuGetSchedAffinity(i, &bEnable);
        if (bEnable) {
            LW_CPU_SET(i, pcpuset);
            
        } else {
            LW_CPU_CLR(i, pcpuset);
        }
        __KERNEL_EXIT_IRQ(iregInterLevel);                              /*  退出内核                    */
    }
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
