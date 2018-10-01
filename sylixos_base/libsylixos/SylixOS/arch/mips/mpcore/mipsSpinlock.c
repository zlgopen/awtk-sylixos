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
** 文   件   名: mipsSpinlock.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: MIPS 体系架构自旋锁驱动.
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
#ifdef LW_CFG_CPU_ARCH_MICROMIPS
#define GCC_OFF_SMALL_ASM()     "ZC"
#else
#define GCC_OFF_SMALL_ASM()     "R"
#endif
/*********************************************************************************************************
** 函数名称: mipsSpinLock
** 功能描述: MIPS spin lock
** 输　入  : psld       spinlock data 指针
**           pfuncPoll  循环等待时调用函数
**           pvArg      回调参数
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : 自旋结束时, 操作系统会调用内存屏障, 所以这里不需要调用.
*********************************************************************************************************/
static LW_INLINE VOID  mipsSpinLock (SPINLOCKTYPE *psld, VOIDFUNCPTR  pfuncPoll, PVOID  pvArg)
{
    UINT32          uiNewVal;
    UINT32          uiInc = 1 << LW_SPINLOCK_TICKET_SHIFT;
    SPINLOCKTYPE    sldVal;

    __asm__ __volatile__(
        "   .set push                                   \n"
        "   .set noreorder                              \n"
        "1: ll      %[oldvalue], %[slock]               \n"
        "   addu    %[newvalue], %[oldvalue], %[inc]    \n"
        "   sc      %[newvalue], %[slock]               \n"
        "   beqz    %[newvalue], 1b                     \n"
        "   nop                                         \n"
        "   .set pop"
        : [slock] "+" GCC_OFF_SMALL_ASM() (psld->SLD_uiLock),
          [oldvalue] "=&r" (sldVal),
          [newvalue] "=&r" (uiNewVal)
        : [inc] "r" (uiInc));

    while (sldVal.SLD_usTicket != sldVal.SLD_usSvcNow) {
        if (pfuncPoll) {
            pfuncPoll(pvArg);
        }
        sldVal.SLD_usSvcNow = LW_ACCESS_ONCE(UINT16, psld->SLD_usSvcNow);
    }
}
/*********************************************************************************************************
** 函数名称: mipsSpinTryLock
** 功能描述: MIPS spin trylock
** 输　入  : psld       spinlock data 指针
** 输　出  : 1: busy 0: ok
** 全局变量:
** 调用模块:
** 注  意  : 自旋结束时, 操作系统会调用内存屏障, 所以这里不需要调用.
*********************************************************************************************************/
static LW_INLINE UINT32  mipsSpinTryLock (SPINLOCKTYPE *psld)
{
    UINT32  uiRes, uiTemp1, uiTemp2;
    UINT32  uiInc = 1 << LW_SPINLOCK_TICKET_SHIFT;

    __asm__ __volatile__ (
        "   .set push                                       \n"
        "   .set noreorder                                  \n"
        "                                                   \n"
        "1: ll      %[ticket],     %[slock]                 \n"
        "   srl     %[myticket],   %[ticket],     16        \n"
        "   andi    %[nowserving], %[ticket],     0xffff    \n"
        "   bne     %[myticket],   %[nowserving], 3f        \n"
        "   addu    %[ticket],     %[ticket],     %[inc]    \n"
        "   sc      %[ticket],     %[slock]                 \n"
        "   beqz    %[ticket],     1b                       \n"
        "   li      %[ticket],     0                        \n"
        "2:                                                 \n"
        "   .subsection 2                                   \n"
        "3: b       2b                                      \n"
        "   li      %[ticket],     1                        \n"
        "   .previous                                       \n"
        "   .set pop                                        \n"
        : [slock] "+" GCC_OFF_SMALL_ASM() (psld->SLD_uiLock),
          [ticket] "=&r" (uiRes),
          [myticket] "=&r" (uiTemp1),
          [nowserving] "=&r" (uiTemp2)
        : [inc] "r" (uiInc));

    return  (uiRes);
}
/*********************************************************************************************************
** 函数名称: mipsSpinUnlock
** 功能描述: MIPS spin unlock
** 输　入  : psld       spinlock data 指针
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE VOID  mipsSpinUnlock (SPINLOCKTYPE *psld)
{
    psld->SLD_usSvcNow++;
    KN_SMP_WMB();
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

    mipsSpinLock(&psl->SL_sltData, pfuncPoll, pvArg);

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

    if (mipsSpinTryLock(&psl->SL_sltData)) {                            /*  尝试加锁                    */
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

    mipsSpinUnlock(&psl->SL_sltData);                                   /*  解锁                        */

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
    mipsSpinLock(&psl->SL_sltData, LW_NULL, LW_NULL);

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
    if (mipsSpinTryLock(&psl->SL_sltData)) {                            /*  尝试加锁                    */
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
    mipsSpinUnlock(&psl->SL_sltData);                                   /*  解锁                        */

    return  (1);
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
