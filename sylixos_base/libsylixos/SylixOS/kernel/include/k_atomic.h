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
** 文   件   名: k_atomic.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2009 年 09 月 30 日
**
** 描        述: 系统内核原子操作.
*********************************************************************************************************/

#ifndef __K_ATOMIC_H
#define __K_ATOMIC_H

/*********************************************************************************************************
  根据系统配置, 选择锁类型
*********************************************************************************************************/

#define __LW_ATOMIC_LOCK(iregInterLevel)    \
        { LW_SPIN_LOCK_RAW(&_K_slcaAtomic.SLCA_sl, &iregInterLevel); }
#define __LW_ATOMIC_UNLOCK(iregInterLevel)  \
        { LW_SPIN_UNLOCK_RAW(&_K_slcaAtomic.SLCA_sl, iregInterLevel); }

/*********************************************************************************************************
  汇编实现
*********************************************************************************************************/
#if LW_CFG_CPU_ATOMIC_EN > 0

static LW_INLINE INT  __LW_ATOMIC_ADD (INT  iVal, atomic_t  *patomic)
{
    return  (archAtomicAdd(iVal, patomic));
}

static LW_INLINE INT  __LW_ATOMIC_SUB (INT  iVal, atomic_t  *patomic)
{
    return  (archAtomicSub(iVal, patomic));
}

static LW_INLINE INT  __LW_ATOMIC_AND (INT  iVal, atomic_t  *patomic)
{
    return  (archAtomicAnd(iVal, patomic));
}

static LW_INLINE INT  __LW_ATOMIC_OR (INT  iVal, atomic_t  *patomic)
{
    return  (archAtomicOr(iVal, patomic));
}

static LW_INLINE INT  __LW_ATOMIC_XOR (INT  iVal, atomic_t  *patomic)
{
    return  (archAtomicXor(iVal, patomic));
}

static LW_INLINE VOID  __LW_ATOMIC_SET (INT  iVal, atomic_t  *patomic)
{
    archAtomicSet(iVal, patomic);
}

static LW_INLINE INT  __LW_ATOMIC_GET (atomic_t  *patomic)
{
    return  (archAtomicGet(patomic));
}

static LW_INLINE INT  __LW_ATOMIC_SWP (INT  iVal, atomic_t  *patomic)
{
    INT   iOldVal;
    
    for (;;) {
        iOldVal = archAtomicGet(patomic);
        if (archAtomicCas(patomic, iOldVal, iVal) == iOldVal) {
            break;
        }
    }
    
    return  (iOldVal);
}

static LW_INLINE INT  __LW_ATOMIC_CAS (atomic_t  *patomic, INT  iOldVal, INT  iNewVal)
{
    return  (archAtomicCas(patomic, iOldVal, iNewVal));
}

static LW_INLINE addr_t  __LW_ATOMIC_ADDR_CAS (volatile addr_t *p, addr_t  ulOld, addr_t  ulNew)
{
    return  (archAtomicAddrCas(p, ulOld, ulNew));
}

#else
/*********************************************************************************************************
  软件实现
*********************************************************************************************************/

static LW_INLINE INT  __LW_ATOMIC_ADD (INT  iVal, atomic_t  *patomic)
{
             INTREG  iregInterLevel;
    REGISTER INT     iRet;
    
    __LW_ATOMIC_LOCK(iregInterLevel);
    patomic->counter += iVal;
    iRet = patomic->counter;
    __LW_ATOMIC_UNLOCK(iregInterLevel);
    
    return  (iRet);
}

static LW_INLINE INT  __LW_ATOMIC_SUB (INT  iVal, atomic_t  *patomic)
{
             INTREG  iregInterLevel;
    REGISTER INT     iRet;
    
    __LW_ATOMIC_LOCK(iregInterLevel);
    patomic->counter -= iVal;
    iRet = patomic->counter;
    __LW_ATOMIC_UNLOCK(iregInterLevel);
    
    return  (iRet);
}

static LW_INLINE INT  __LW_ATOMIC_AND (INT  iVal, atomic_t  *patomic)
{
             INTREG  iregInterLevel;
    REGISTER INT     iRet;
    
    __LW_ATOMIC_LOCK(iregInterLevel);
    iRet = patomic->counter & iVal;
    patomic->counter = iRet;
    __LW_ATOMIC_UNLOCK(iregInterLevel);
    
    return  (iRet);
}

static LW_INLINE INT  __LW_ATOMIC_OR (INT  iVal, atomic_t  *patomic)
{
             INTREG  iregInterLevel;
    REGISTER INT     iRet;
    
    __LW_ATOMIC_LOCK(iregInterLevel);
    iRet = patomic->counter | iVal;
    patomic->counter = iRet;
    __LW_ATOMIC_UNLOCK(iregInterLevel);
    
    return  (iRet);
}

static LW_INLINE INT  __LW_ATOMIC_XOR (INT  iVal, atomic_t  *patomic)
{
             INTREG  iregInterLevel;
    REGISTER INT     iRet;
    
    __LW_ATOMIC_LOCK(iregInterLevel);
    iRet = patomic->counter ^ iVal;
    patomic->counter = iRet;
    __LW_ATOMIC_UNLOCK(iregInterLevel);
    
    return  (iRet);
}

static LW_INLINE VOID  __LW_ATOMIC_SET (INT  iVal, atomic_t  *patomic)
{
    INTREG  iregInterLevel;
    
    __LW_ATOMIC_LOCK(iregInterLevel);
    patomic->counter = iVal;
    __LW_ATOMIC_UNLOCK(iregInterLevel);
}

static LW_INLINE INT  __LW_ATOMIC_GET (atomic_t  *patomic)
{
    return  (patomic->counter);
}

static LW_INLINE INT  __LW_ATOMIC_SWP (INT  iVal, atomic_t  *patomic)
{
             INTREG  iregInterLevel;
    REGISTER INT     iRet;
    
    __LW_ATOMIC_LOCK(iregInterLevel);
    iRet = patomic->counter;
    patomic->counter = iVal;
    __LW_ATOMIC_UNLOCK(iregInterLevel);
    
    return  (iRet);
}

static LW_INLINE INT  __LW_ATOMIC_CAS (atomic_t  *patomic, INT  iOldVal, INT  iNewVal)
{
             INTREG  iregInterLevel;
    REGISTER INT     iRet;
    
    __LW_ATOMIC_LOCK(iregInterLevel);
    iRet = patomic->counter;
    if (iRet == iOldVal) {
        patomic->counter = iNewVal;
    }
    __LW_ATOMIC_UNLOCK(iregInterLevel);
    
    return  (iRet);
}

static LW_INLINE addr_t  __LW_ATOMIC_ADDR_CAS (volatile addr_t *p, addr_t  ulOld, addr_t  ulNew)
{
             INTREG  iregInterLevel;
    REGISTER addr_t  ulRet;

    __LW_ATOMIC_LOCK(iregInterLevel);
    ulRet = *p;
    if (ulRet == ulOld) {
        *p = ulNew;
    }
    __LW_ATOMIC_UNLOCK(iregInterLevel);

    return  (ulRet);
}

#endif                                                                  /*  LW_CFG_CPU_ATOMIC_EN        */
/*********************************************************************************************************
  自我实现
*********************************************************************************************************/

static LW_INLINE INT  __LW_ATOMIC_INC (atomic_t  *patomic)
{
    return  (__LW_ATOMIC_ADD(1, patomic));
}

static LW_INLINE INT  __LW_ATOMIC_DEC (atomic_t  *patomic)
{
    return  (__LW_ATOMIC_SUB(1, patomic));
}

static LW_INLINE INT  __LW_ATOMIC_NAND (INT  iVal, atomic_t  *patomic)
{
    return  (__LW_ATOMIC_AND(~iVal, patomic));
}

#endif                                                                  /*  __K_ATOMIC_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
