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
** 文   件   名: sched_cpu.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2018 年 06 月 07 日
**
** 描        述: CPU 强亲和度调度 (posix 扩展).
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../include/px_sched.h"                                        /*  已包含操作系统头文件        */
/*********************************************************************************************************
** 函数名称: sched_cpuaffinity_enable_np
** 功能描述: 使能 CPU 强亲和度调度.
** 输　入  : setsize       CPU 集合大小
**           set           CPU 集合
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
#if (LW_CFG_SMP_EN > 0) && (LW_CFG_POSIX_EN > 0) && (LW_CFG_POSIXEX_EN > 0)

LW_API 
int  sched_cpuaffinity_enable_np (size_t setsize, const cpu_set_t *set)
{
    ULONG      i, num;
    cpu_set_t  cpuset_cur;
    
    if (!setsize || !set) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    if (API_CpuGetSchedAffinity(sizeof(cpu_set_t), &cpuset_cur)) {
        return  (PX_ERROR);
    }
    
    num = ((ULONG)setsize << 3);
    num = (num > LW_NCPUS) ? LW_NCPUS : num;
    
    for (i = 1; i < num; i++) {                                         /*  CPU 0 不能设置              */
        if (LW_CPU_ISSET(i, set)) {
            LW_CPU_SET(i, &cpuset_cur);
        }
    }
    
    if (API_CpuSetSchedAffinity(sizeof(cpu_set_t), &cpuset_cur)) {
        return  (PX_ERROR);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: sched_cpuaffinity_disable_np
** 功能描述: 禁能 CPU 强亲和度调度.
** 输　入  : setsize       CPU 集合大小
**           set           CPU 集合
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
int  sched_cpuaffinity_disable_np (size_t setsize, const cpu_set_t *set)
{
    ULONG      i, num;
    cpu_set_t  cpuset_cur;
    
    if (!setsize || !set) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    if (API_CpuGetSchedAffinity(sizeof(cpu_set_t), &cpuset_cur)) {
        return  (PX_ERROR);
    }
    
    num = ((ULONG)setsize << 3);
    num = (num > LW_NCPUS) ? LW_NCPUS : num;
    
    for (i = 1; i < num; i++) {                                         /*  CPU 0 不能设置              */
        if (LW_CPU_ISSET(i, set)) {
            LW_CPU_CLR(i, &cpuset_cur);
        }
    }
    
    if (API_CpuSetSchedAffinity(sizeof(cpu_set_t), &cpuset_cur)) {
        return  (PX_ERROR);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: sched_cpuaffinity_set_np
** 功能描述: 设置 CPU 强亲和度调度.
** 输　入  : setsize       CPU 集合大小
**           set           CPU 集合
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
int  sched_cpuaffinity_set_np (size_t setsize, const cpu_set_t *set)
{
    if (!setsize || !set) {
        errno = EINVAL;
        return  (PX_ERROR);
    }

    if (API_CpuSetSchedAffinity(setsize, (PLW_CLASS_CPUSET)set)) {
        return  (PX_ERROR);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: sched_cpuaffinity_get_np
** 功能描述: 获取 CPU 强亲和度调度.
** 输　入  : setsize       CPU 集合大小
**           set           CPU 集合
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API 
int  sched_cpuaffinity_get_np (size_t setsize, cpu_set_t *set)
{
    if (!setsize || !set) {
        errno = EINVAL;
        return  (PX_ERROR);
    }

    if (API_CpuGetSchedAffinity(setsize, set)) {
        return  (PX_ERROR);
    }
    
    return  (ERROR_NONE);
}

#endif                                                                  /*  (LW_CFG_SMP_EN > 0)         */
                                                                        /*  (LW_CFG_POSIX_EN > 0)       */
                                                                        /*  (LW_CFG_POSIXEX_EN > 0)     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
