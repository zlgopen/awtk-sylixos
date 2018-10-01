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
** 文   件   名: armSpinlock.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2013 年 12 月 09 日
**
** 描        述: ARM 体系构架自旋锁驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  spinlock 状态
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
/*********************************************************************************************************
  L1 cache 同步请参考: http://www.cnblogs.com/jiayy/p/3246133.html
*********************************************************************************************************/
/*********************************************************************************************************
** 函数名称: armSpinLock
** 功能描述: ARM spin lock
** 输　入  : psld       spinlock data 指针
**           pfuncPoll  循环等待时调用函数
**           pvArg      回调参数
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : 自旋结束时, 操作系统会调用内存屏障, 所以这里不需要调用.
*********************************************************************************************************/
static VOID  armSpinLock (SPINLOCKTYPE *psld, VOIDFUNCPTR  pfuncPoll, PVOID  pvArg)
{
#if __SYLIXOS_ARM_ARCH__ >= 6
    UINT32          uiTemp;
    UINT32          uiNewVal;
    SPINLOCKTYPE    sldVal;

    ARM_PREFETCH_W(&psld->SLD_uiLock);

    __asm__ __volatile__(
        "1: ldrex   %[oldvalue], [%[slock]]                 \n"
        "   add     %[newvalue], %[oldvalue], %[tshift]     \n"
        "   strex   %[temp],     %[newvalue], [%[slock]]    \n"
        "   teq     %[temp],     #0                         \n"
        "   bne     1b"
        : [oldvalue] "=&r" (sldVal), [newvalue] "=&r" (uiNewVal), [temp] "=&r" (uiTemp)
        : [slock] "r" (&psld->SLD_uiLock), [tshift] "I" (1 << LW_SPINLOCK_TICKET_SHIFT)
        : "cc");

    while (sldVal.SLD_usTicket != sldVal.SLD_usSvcNow) {
        if (pfuncPoll) {
            pfuncPoll(pvArg);
        } else {
            __asm__ __volatile__(ARM_WFE(""));
        }
        sldVal.SLD_usSvcNow = LW_ACCESS_ONCE(UINT16, psld->SLD_usSvcNow);
    }
#endif                                                                  /*  __SYLIXOS_ARM_ARCH__ >= 6   */
}
/*********************************************************************************************************
** 函数名称: armSpinTryLock
** 功能描述: ARM spin trylock
** 输　入  : psld       spinlock data 指针
** 输　出  : 1: busy 0: ok
** 全局变量:
** 调用模块:
** 注  意  : 自旋结束时, 操作系统会调用内存屏障, 所以这里不需要调用.
*********************************************************************************************************/
static UINT32  armSpinTryLock (SPINLOCKTYPE *psld)
{
#if __SYLIXOS_ARM_ARCH__ >= 6
    UINT32  uiCont, uiRes, uiLock;

    ARM_PREFETCH_W(&psld->SLD_uiLock);

    do {
        __asm__ __volatile__(
            "   ldrex   %[lock], [%[slock]]                 \n"
            "   mov     %[res],  #0                         \n"
            "   subs    %[cont], %[lock], %[lock], ror #16  \n"
            "   addeq   %[lock], %[lock], %[tshift]         \n"
            "   strexeq %[res],  %[lock], [%[slock]]"
            : [lock] "=&r" (uiLock), [cont] "=&r" (uiCont), [res] "=&r" (uiRes)
            : [slock] "r" (&psld->SLD_uiLock), [tshift] "I" (1 << LW_SPINLOCK_TICKET_SHIFT)
            : "cc");
    } while (uiRes);

    if (uiCont) {
        return  (1);
    }
#endif                                                                  /*  __SYLIXOS_ARM_ARCH__ >= 6   */

    return  (0);
}
/*********************************************************************************************************
** 函数名称: armSpinUnlock
** 功能描述: ARM spin unlock
** 输　入  : psld       spinlock data 指针
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  armSpinUnlock (SPINLOCKTYPE *psld)
{
#if __SYLIXOS_ARM_ARCH__ >= 6
    psld->SLD_usSvcNow++;
    armDsb();
    __asm__ __volatile__(ARM_SEV);
#endif                                                                  /*  __SYLIXOS_ARM_ARCH__ >= 6   */
}
/*********************************************************************************************************
** 函数名称: armSpinLockDummy
** 功能描述: 空操作
** 输　入  : psl        spinlock 指针
**           pfuncPoll  循环等待时调用函数
**           pvArg      回调参数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  armSpinLockDummy (SPINLOCKTYPE  *psl, VOIDFUNCPTR  pfuncPoll, PVOID  pvArg)
{
}
/*********************************************************************************************************
** 函数名称: armSpinTryLockDummy
** 功能描述: 空操作
** 输　入  : psl        spinlock 指针
** 输　出  : 0
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static UINT32  armSpinTryLockDummy (SPINLOCKTYPE  *psl)
{
    return  (0);
}
/*********************************************************************************************************
** 函数名称: armSpinUnlockDummy
** 功能描述: 空操作
** 输　入  : psl        spinlock 指针
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  armSpinUnlockDummy (SPINLOCKTYPE  *psl)
{
}
/*********************************************************************************************************
  spin lock cache 依赖处理
*********************************************************************************************************/
static VOID    (*pfuncArmSpinLock)(SPINLOCKTYPE *, VOIDFUNCPTR, PVOID) = armSpinLock;
static UINT32  (*pfuncArmSpinTryLock)(SPINLOCKTYPE *)                  = armSpinTryLock;
static VOID    (*pfuncArmSpinUnlock)(SPINLOCKTYPE *)                   = armSpinUnlock;
/*********************************************************************************************************
** 函数名称: archSpinBypass
** 功能描述: spinlock 函数不起效
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archSpinBypass (VOID)
{
    pfuncArmSpinLock    = armSpinLockDummy;
    pfuncArmSpinTryLock = armSpinTryLockDummy;
    pfuncArmSpinUnlock  = armSpinUnlockDummy;
}
/*********************************************************************************************************
** 函数名称: archSpinWork
** 功能描述: spinlock 函数起效
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : 主核开启 CACHE 后, BSP 应立即调用此函数, 使 spinlock 生效,
             从核启动到开启 CACHE 过程中, 不得操作 spinlock.
*********************************************************************************************************/
VOID  archSpinWork (VOID)
{
    pfuncArmSpinUnlock  = armSpinUnlock;
    pfuncArmSpinTryLock = armSpinTryLock;
    pfuncArmSpinLock    = armSpinLock;
}
/*********************************************************************************************************
** 函数名称: archSpinInit
** 功能描述: 初始化一个 spinlock
** 输　入  : psl        spinlock 指针
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  archSpinInit (spinlock_t  *psl)
{
    psl->SL_sltData.SLD_uiLock = 0;                                     /*  0: 未锁定状态  1: 锁定状态  */
    psl->SL_pcpuOwner          = LW_NULL;
    psl->SL_ulCounter          = 0;
    psl->SL_pvReserved         = LW_NULL;
    KN_SMP_WMB();
}
/*********************************************************************************************************
** 函数名称: archSpinDelay
** 功能描述: 等待事件
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  archSpinDelay (VOID)
{
    volatile INT  i;

    for (i = 0; i < 3; i++) {
    }
}
/*********************************************************************************************************
** 函数名称: archSpinNotify
** 功能描述: 发送 spin 事件
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  archSpinNotify (VOID)
{
}
/*********************************************************************************************************
** 函数名称: archSpinLock
** 功能描述: spinlock 上锁
** 输　入  : psl        spinlock 指针
**           pfuncPoll  循环等待时调用函数
**           pvArg      回调参数
** 输　出  : 0: 没有获取
**           1: 正常加锁
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  archSpinLock (spinlock_t  *psl, VOIDFUNCPTR  pfuncPoll, PVOID  pvArg)
{
    if (psl->SL_pcpuOwner == LW_CPU_GET_CUR()) {
        psl->SL_ulCounter++;
        _BugFormat((psl->SL_ulCounter > 10), LW_TRUE, 
                   "spinlock RECURSIVE %lu!\r\n", psl->SL_ulCounter);
        return  (1);                                                    /*  重复调用                    */
    }
    
    pfuncArmSpinLock(&psl->SL_sltData, pfuncPoll, pvArg);
    
    psl->SL_pcpuOwner = LW_CPU_GET_CUR();                               /*  保存当前 CPU                */
    
    return  (1);                                                        /*  加锁成功                    */
}
/*********************************************************************************************************
** 函数名称: archSpinTryLock
** 功能描述: spinlock 试图上锁
** 输　入  : psl        spinlock 指针
** 输　出  : 0: 没有获取
**           1: 正常加锁
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  archSpinTryLock (spinlock_t  *psl)
{
    if (psl->SL_pcpuOwner == LW_CPU_GET_CUR()) {
        psl->SL_ulCounter++;
        _BugFormat((psl->SL_ulCounter > 10), LW_TRUE, 
                   "spinlock RECURSIVE %lu!\r\n", psl->SL_ulCounter);
        return  (1);                                                    /*  重复调用                    */
    }
    
    if (pfuncArmSpinTryLock(&psl->SL_sltData)) {                        /*  尝试加锁                    */
        return  (0);
    }
    
    psl->SL_pcpuOwner = LW_CPU_GET_CUR();                               /*  保存当前 CPU                */
    
    return  (1);
}
/*********************************************************************************************************
** 函数名称: archSpinUnlock
** 功能描述: spinlock 解锁
** 输　入  : psl        spinlock 指针
** 输　出  : 0: 没有获取
**           1: 正常解锁
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  archSpinUnlock (spinlock_t  *psl)
{
    if (psl->SL_pcpuOwner != LW_CPU_GET_CUR()) {
        return  (0);                                                    /*  没有权利释放                */
    }
    
    if (psl->SL_ulCounter) {
        psl->SL_ulCounter--;                                            /*  减少重复调用次数            */
        return  (1);
    }

    psl->SL_pcpuOwner = LW_NULL;                                        /*  没有 CPU 获取               */
    KN_SMP_WMB();
    
    pfuncArmSpinUnlock(&psl->SL_sltData);                               /*  解锁                        */

    return  (1);
}
/*********************************************************************************************************
** 函数名称: archSpinLockRaw
** 功能描述: spinlock 上锁 (不进行重入判断)
** 输　入  : psl        spinlock 指针
** 输　出  : 0: 没有获取
**           1: 正常加锁
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  archSpinLockRaw (spinlock_t  *psl)
{
    pfuncArmSpinLock(&psl->SL_sltData, LW_NULL, LW_NULL);

    return  (1);                                                        /*  加锁成功                    */
}
/*********************************************************************************************************
** 函数名称: archSpinTryLockRaw
** 功能描述: spinlock 试图上锁 (不进行重入判断)
** 输　入  : psl        spinlock 指针
** 输　出  : 0: 没有获取
**           1: 正常加锁
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  archSpinTryLockRaw (spinlock_t  *psl)
{
    if (pfuncArmSpinTryLock(&psl->SL_sltData)) {                        /*  尝试加锁                    */
        return  (0);
    }
    
    return  (1);                                                        /*  加锁成功                    */
}
/*********************************************************************************************************
** 函数名称: archSpinUnlockRaw
** 功能描述: spinlock 解锁
** 输　入  : psl        spinlock 指针
** 输　出  : 0: 没有获取
**           1: 正常解锁
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  archSpinUnlockRaw (spinlock_t  *psl)
{
    pfuncArmSpinUnlock(&psl->SL_sltData);
    
    return  (1);
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
