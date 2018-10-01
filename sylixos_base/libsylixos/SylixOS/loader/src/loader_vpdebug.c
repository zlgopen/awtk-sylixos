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
** 文   件   名: loader_vpdebug.c
**
** 创   建   人: Han.hui (韩辉)
**
** 文件创建日期: 2018 年 05 月 24 日
**
** 描        述: 模块对 VPROCESS 进程调试支持.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#if LW_CFG_GDB_EN > 0 && LW_CFG_NET_EN > 0
#include "lwip/tcpip.h"
#endif                                                                  /*  GDB_EN && NET_EN            */
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_MODULELOADER_EN > 0
#include "../include/loader_lib.h"
#if LW_CFG_VMM_EN > 0
#include "../SylixOS/kernel/vmm/pageTable.h"
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
** 函数名称: vprocDebugCriResLock
** 功能描述: 关键性资源锁定
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
#if LW_CFG_GDB_EN > 0

static VOID  vprocDebugCriResLock (VOID)
{
#if LW_CFG_VMM_EN > 0
    __VMM_LOCK();
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
#if LW_CFG_NET_EN > 0
    LOCK_TCPIP_CORE();
#endif
    API_SemaphoreMPend(_K_pheapSystem->HEAP_ulLock, LW_OPTION_WAIT_INFINITE);
}
/*********************************************************************************************************
** 函数名称: vprocDebugCriResUnlock
** 功能描述: 关键性资源解锁
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  vprocDebugCriResUnlock (VOID)
{
#if LW_CFG_VMM_EN > 0
    __VMM_UNLOCK();
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
#if LW_CFG_NET_EN > 0
    UNLOCK_TCPIP_CORE();
#endif
    API_SemaphoreMPost(_K_pheapSystem->HEAP_ulLock);
}
/*********************************************************************************************************
** 函数名称: vprocDebugStop
** 功能描述: 停止进程内的所有线程
** 输　入  : pvVProc    进程控制块指针
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : LW_VP_LOCK() 保证了进程内任务不可被删除性.
*********************************************************************************************************/
VOID  vprocDebugStop (PVOID  pvVProc)
{
    LW_LD_VPROC    *pvproc = (LW_LD_VPROC *)pvVProc;
    PLW_LIST_LINE   plineTemp;
    PLW_CLASS_TCB   ptcb;
    
    LW_VP_LOCK(pvproc);                                                 /*  锁定当前进程                */
    vprocDebugCriResLock();                                             /*  等待调试关键性资源          */
    for (plineTemp  = pvproc->VP_plineThread;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
    
        ptcb = _LIST_ENTRY(plineTemp, LW_CLASS_TCB, TCB_lineProcess);
        if (ptcb->TCB_iDeleteProcStatus == LW_TCB_DELETE_PROC_NONE) {
            __KERNEL_ENTER();                                           /*  进入内核                    */
            _ThreadStop(ptcb);
            __KERNEL_EXIT();                                            /*  退出内核                    */
                                                                        /*  必须保证线程被完全停止      */
            __KERNEL_ENTER();                                           /*  进入内核                    */
            _ThreadDebugUnpendSem(ptcb);
            __KERNEL_EXIT();                                            /*  退出内核                    */
        }
    }
    vprocDebugCriResUnlock();                                           /*  释放调试关键性资源          */
    LW_VP_UNLOCK(pvproc);                                               /*  解锁当前进程                */
}
/*********************************************************************************************************
** 函数名称: vprocDebugContinue
** 功能描述: 恢复进程内的所有停止的线程
** 输　入  : pvVProc    进程控制块指针
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  vprocDebugContinue (PVOID  pvVProc)
{
    LW_LD_VPROC    *pvproc = (LW_LD_VPROC *)pvVProc;
    PLW_LIST_LINE   plineTemp;
    PLW_CLASS_TCB   ptcb;
    
    LW_VP_LOCK(pvproc);                                                 /*  锁定当前进程                */
    for (plineTemp  = pvproc->VP_plineThread;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
    
        ptcb = _LIST_ENTRY(plineTemp, LW_CLASS_TCB, TCB_lineProcess);
        __KERNEL_ENTER();                                               /*  进入内核                    */
        _ThreadContinue(ptcb, LW_FALSE);
        __KERNEL_EXIT();                                                /*  退出内核                    */
    }
    LW_VP_UNLOCK(pvproc);                                               /*  解锁当前进程                */
}
/*********************************************************************************************************
** 函数名称: vprocDebugThreadStop
** 功能描述: 停止指定的线程
** 输　入  : pvVProc    进程控制块指针
**           ulId       线程句柄
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : LW_VP_LOCK() 保证了进程内任务不可被删除性.
*********************************************************************************************************/
VOID  vprocDebugThreadStop (PVOID  pvVProc, LW_OBJECT_HANDLE  ulId)
{
             LW_LD_VPROC   *pvproc = (LW_LD_VPROC *)pvVProc;
    REGISTER UINT16         usIndex;
    REGISTER PLW_CLASS_TCB  ptcb;
    
    usIndex = _ObjectGetIndex(ulId);
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_THREAD)) {                        /*  检查 ID 类型有效性          */
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return;
    }
    if (usIndex >= LW_CFG_MAX_THREADS) {                                /*  允许获得空闲线程堆栈信息    */
        _ErrorHandle(ERROR_THREAD_NULL);
        return;
    }
#endif

    LW_VP_LOCK(pvproc);                                                 /*  锁定当前进程                */
    vprocDebugCriResLock();                                             /*  等待调试关键性资源          */
    
    __KERNEL_ENTER();                                                   /*  进入内核                    */
    if (_Thread_Invalid(usIndex)) {
        goto    __error;
    }
    
    ptcb = _K_ptcbTCBIdTable[usIndex];
    
    if (__LW_VP_GET_TCB_PROC(ptcb) != (LW_LD_VPROC *)pvVProc) {         /*  进程控制块错误              */
        goto    __error;
    }
    
    if (ptcb->TCB_iDeleteProcStatus) {
        goto    __error;
    }
    
    _ThreadStop(ptcb);
    __KERNEL_EXIT();                                                    /*  退出内核                    */
                                                                        /*  必须保证线程被完全停止      */
    __KERNEL_ENTER();                                                   /*  进入内核                    */
    _ThreadDebugUnpendSem(ptcb);
    __KERNEL_EXIT();                                                    /*  退出内核                    */
    
    vprocDebugCriResUnlock();                                           /*  释放调试关键性资源          */
    LW_VP_UNLOCK(pvproc);                                               /*  解锁当前进程                */
    return;
    
__error:
    __KERNEL_EXIT();                                                    /*  退出内核                    */
    vprocDebugCriResUnlock();                                           /*  释放调试关键性资源          */
    LW_VP_UNLOCK(pvproc);                                               /*  解锁当前进程                */
    _ErrorHandle(ERROR_THREAD_NULL);
}
/*********************************************************************************************************
** 函数名称: vprocDebugThreadContinue
** 功能描述: 恢复指定的线程
** 输　入  : pvVProc    进程控制块指针
**           ulId       线程句柄
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  vprocDebugThreadContinue (PVOID  pvVProc, LW_OBJECT_HANDLE  ulId)
{
    REGISTER UINT16         usIndex;
    REGISTER PLW_CLASS_TCB  ptcb;
    
    usIndex = _ObjectGetIndex(ulId);
    
#if LW_CFG_ARG_CHK_EN > 0
    if (!_ObjectClassOK(ulId, _OBJECT_THREAD)) {                        /*  检查 ID 类型有效性          */
        _ErrorHandle(ERROR_KERNEL_HANDLE_NULL);
        return;
    }
    if (usIndex >= LW_CFG_MAX_THREADS) {                                /*  允许获得空闲线程堆栈信息    */
        _ErrorHandle(ERROR_THREAD_NULL);
        return;
    }
#endif

    __KERNEL_ENTER();                                                   /*  进入内核                    */
    if (_Thread_Invalid(usIndex)) {
        goto    __error;
    }
    
    ptcb = _K_ptcbTCBIdTable[usIndex];
    
    if (__LW_VP_GET_TCB_PROC(ptcb) != (LW_LD_VPROC *)pvVProc) {         /*  进程控制块错误              */
        goto    __error;
    }
    
    _ThreadContinue(ptcb, LW_FALSE);
    __KERNEL_EXIT();                                                    /*  退出内核                    */
    return;

__error:
    __KERNEL_EXIT();                                                    /*  退出内核                    */
    _ErrorHandle(ERROR_THREAD_NULL);
}
/*********************************************************************************************************
** 函数名称: vprocDebugThreadGet
** 功能描述: 获得进程内的所有线程句柄
** 输　入  : pvVProc        进程控制块指针
**           ulId           线程表
**           uiTableNum     表容量
** 输　出  : 总线程数
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT  vprocDebugThreadGet (PVOID  pvVProc, LW_OBJECT_HANDLE  ulId[], UINT   uiTableNum)
{
    LW_LD_VPROC    *pvproc = (LW_LD_VPROC *)pvVProc;
    PLW_LIST_LINE   plineTemp;
    PLW_CLASS_TCB   ptcb;
    UINT            uiNum = 0;
    
    __KERNEL_ENTER();                                                   /*  进入内核                    */
    for (plineTemp  = pvproc->VP_plineThread;
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {
    
        ptcb = _LIST_ENTRY(plineTemp, LW_CLASS_TCB, TCB_lineProcess);
        if (uiNum < uiTableNum) {
            ulId[uiNum] = ptcb->TCB_ulId;
            uiNum++;
        }
    }
    __KERNEL_EXIT();                                                    /*  退出内核                    */

    return  (uiNum);
}

#endif                                                                  /*  LW_CFG_GDB_EN > 0           */
#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
