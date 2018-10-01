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
** 文   件   名: riscvSpinlock.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: RISC-V 体系构架自旋锁驱动.
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
** 函数名称: riscvSpinLock
** 功能描述: RISC-V spin lock
** 输　入  : psld       spinlock data 指针
**           pfuncPoll  循环等待时调用函数
**           pvArg      回调参数
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : 自旋结束时, 操作系统会调用内存屏障, 所以这里不需要调用.
*********************************************************************************************************/
static LW_INLINE VOID  riscvSpinLock (SPINLOCKTYPE *psld, VOIDFUNCPTR  pfuncPoll, PVOID  pvArg)
{
    UINT32          uiNewVal;
    UINT32          uiInc = 1 << LW_SPINLOCK_TICKET_SHIFT;
    SPINLOCKTYPE    sldVal;

    __asm__ __volatile__(
        "1: lr.w        %[oldvalue], %[slock]                   \n"
        "   add         %[newvalue], %[oldvalue], %[tshift]     \n"
        "   sc.w.aq     %[newvalue], %[newvalue], %[slock]      \n"
        "   bnez        %[newvalue], 1b                         \n"
        : [oldvalue] "=&r" (sldVal), [newvalue] "=&r" (uiNewVal), [slock] "+A" (psld->SLD_uiLock)
        : [tshift] "r" (uiInc)
        : "memory");

    while (sldVal.SLD_usTicket != sldVal.SLD_usSvcNow) {
        if (pfuncPoll) {
            pfuncPoll(pvArg);
        }
        sldVal.SLD_usSvcNow = LW_ACCESS_ONCE(UINT16, psld->SLD_usSvcNow);
    }
}
/*********************************************************************************************************
** 函数名称: riscvSpinTryLock
** 功能描述: RISC-V spin trylock
** 输　入  : psld       spinlock data 指针
** 输　出  : 1: busy 0: ok
** 全局变量:
** 调用模块:
** 注  意  : 自旋结束时, 操作系统会调用内存屏障, 所以这里不需要调用.
*********************************************************************************************************/
static LW_INLINE UINT32  riscvSpinTryLock (SPINLOCKTYPE *psld)
{
    UINT32  uiRes, uiTemp1, uiTemp2;
    UINT32  uiInc = 1 << LW_SPINLOCK_TICKET_SHIFT;

    __asm__ __volatile__ (
        "1: lr.w    %[ticket],                    %[slock]  \n"
        "   li      %[myticket],                  0xffff    \n"
        "   and     %[nowserving], %[ticket],     %[myticket]\n"
        "   srli    %[myticket],   %[ticket],     16        \n"
        "   bne     %[myticket],   %[nowserving], 3f        \n"
        "   add     %[ticket],     %[ticket],     %[inc]    \n"
        "   sc.w.aq %[ticket],     %[ticket],     %[slock]  \n"
        "   bnez    %[ticket],     1b                       \n"
        "   li      %[ticket],     0                        \n"
        "2:                                                 \n"
        "   .subsection 2                                   \n"
        "3: li      %[ticket],     1                        \n"
        "   j       2b                                      \n"
        "   .previous                                       \n"
        : [ticket] "=&r" (uiRes), [myticket] "=&r" (uiTemp1), [nowserving] "=&r" (uiTemp2),
          [slock] "+A"  (psld->SLD_uiLock)
        : [inc] "r" (uiInc)
        : "memory");

    return  (uiRes);
}
/*********************************************************************************************************
** 函数名称: riscvSpinUnlock
** 功能描述: RISC-V spin unlock
** 输　入  : psld       spinlock data 指针
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE VOID  riscvSpinUnlock (SPINLOCKTYPE *psld)
{
    psld->SLD_usSvcNow++;
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

    riscvSpinLock(&psl->SL_sltData, pfuncPoll, pvArg);

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

    if (riscvSpinTryLock(&psl->SL_sltData)) {
        return  (0);
    }

    psl->SL_pcpuOwner = LW_CPU_GET_CUR();                               /*  保存当前 CPU                */

    return  (1);                                                        /*  加锁成功                    */
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

    riscvSpinUnlock(&psl->SL_sltData);                                  /*  汇编实现中带内存屏障        */

    return  (1);                                                        /*  解锁成功                    */
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
    riscvSpinLock(&psl->SL_sltData, LW_NULL, LW_NULL);

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
    if (riscvSpinTryLock(&psl->SL_sltData)) {
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
    riscvSpinUnlock(&psl->SL_sltData);

    return  (1);                                                        /*  解锁成功                    */
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
