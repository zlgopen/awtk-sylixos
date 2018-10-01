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
** 文   件   名: KernelMisc.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2013 年 03 月 30 日
**
** 描        述: 系统内核杂项功能
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** 函数名称: API_KernelNop
** 功能描述: 内核空函数, 当前未使用
** 输　入  : pcArg                         字串参数
**           lArg                          长整形参数
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
VOID  API_KernelNop (CPCHAR  pcArg, LONG  lArg)
{
    (VOID)pcArg;
    (VOID)lArg;
}
/*********************************************************************************************************
** 函数名称: API_KernelIsCpuIdle
** 功能描述: 指定 CPU 是否空闲
** 输　入  : ulCPUId    CPU ID
** 输　出  : 是否空闲
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
BOOL  API_KernelIsCpuIdle (ULONG  ulCPUId)
{
    BOOL            bRet;
    PLW_CLASS_CPU   pcpu;

    if (ulCPUId >= LW_NCPUS) {
        return  (LW_TRUE);
    }
    
    __KERNEL_ENTER();
    pcpu = LW_CPU_GET(ulCPUId);
    if (LW_CPU_IS_ACTIVE(pcpu)) {
        if (pcpu->CPU_ptcbTCBCur->TCB_ucPriority == LW_PRIO_IDLE) {
            bRet = LW_TRUE;

        } else {
            bRet = LW_FALSE;
        }
        
    } else {
        bRet = LW_TRUE;
    }
    __KERNEL_EXIT();

    return  (bRet);
}
/*********************************************************************************************************
** 函数名称: API_KernelIsSystemIdle
** 功能描述: 所有 CPU 是否空闲
** 输　入  : NONE
** 输　出  : 是否空闲
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
BOOL  API_KernelIsSystemIdle (VOID)
{
    INT             i;
    BOOL            bRet = LW_TRUE;
    PLW_CLASS_CPU   pcpu;
    
    __KERNEL_ENTER();
    LW_CPU_FOREACH_ACTIVE (i) {
        pcpu = LW_CPU_GET(i);
        if (pcpu->CPU_ptcbTCBCur->TCB_ucPriority != LW_PRIO_IDLE) {
            bRet = LW_FALSE;
            break;
        }
    }
    __KERNEL_EXIT();

    return  (bRet);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
