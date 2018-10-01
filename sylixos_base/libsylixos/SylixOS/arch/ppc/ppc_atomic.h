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
** 文   件   名: ppc_atomic.h
**
** 创   建   人: Jiao.Jinxing (焦进星)
**
** 文件创建日期: 2018 年 07 月 27 日
**
** 描        述: PowerPC 体系构架 ATOMIC 接口.
*********************************************************************************************************/

#ifndef __ARCH_PPC_ATOMIC_H
#define __ARCH_PPC_ATOMIC_H

/*********************************************************************************************************
  ATOMIC
*********************************************************************************************************/
#if LW_CFG_CPU_ATOMIC_EN > 0

#define ATOMIC_OP_RETURN(op, c_op, asm_op)                                  \
static LW_INLINE INT  archAtomic##op (INT  i, atomic_t  *v)                 \
{                                                                           \
    INT  iResult;                                                           \
                                                                            \
    __asm__ __volatile__(                                                   \
        "1: lwarx       %0, 0,  %3      \n"                                 \
            #asm_op "   %0, %2, %0      \n"                                 \
        "   stwcx.      %0, 0,  %3      \n"                                 \
        "   bne-        1b              \n"                                 \
            : "=&r" (iResult), "+m" (v->counter)                            \
            : "r" (i), "r" (&v->counter)                                    \
            : "cc");                                                        \
                                                                            \
    return  (iResult);                                                      \
}

ATOMIC_OP_RETURN(Add,  +=,  add)
ATOMIC_OP_RETURN(Sub,  -=,  subf)
ATOMIC_OP_RETURN(And,  &=,  and)
ATOMIC_OP_RETURN(Or,   |=,  or)
ATOMIC_OP_RETURN(Xor,  ^=,  xor)

static LW_INLINE INT  archAtomicGet (atomic_t  *v)
{
    INT  iValue;

    __asm__ __volatile__("lwz%U1%X1   %0, %1" : "=r"(iValue) : "m"(v->counter));

    return  (iValue);
}

static LW_INLINE VOID  archAtomicSet (INT  i, atomic_t  *v)
{
    __asm__ __volatile__("stw%U0%X0   %1, %0" : "=m"(v->counter) : "r"(i));
}

#if LW_CFG_SMP_EN > 0
#define PPC_ATOMIC_ENTRY_BARRIER    "      sync    \n"
#define PPC_ATOMIC_EXIT_BARRIER     "      sync    \n"
#else
#define PPC_ATOMIC_ENTRY_BARRIER
#define PPC_ATOMIC_EXIT_BARRIER
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */

/*********************************************************************************************************
  atomic cas op
*********************************************************************************************************/

static LW_INLINE INT  archAtomicCas (atomic_t  *v, INT  iOld, INT  iNew)
{
    INT  iOldValue;

    __asm__ __volatile__ (
            PPC_ATOMIC_ENTRY_BARRIER
        "1: lwarx   %0, 0,  %2   \n"
        "   cmpw    0,  %0, %3   \n"
        "   bne-    2f           \n"
        "   stwcx.  %4, 0,  %2   \n"
        "   bne-    1b           \n"
            PPC_ATOMIC_EXIT_BARRIER
        "2:"
            : "=&r" (iOldValue), "+m" (v->counter)
            : "r" (&v->counter), "r" (iOld), "r" (iNew)
            : "cc", "memory");

    return  (iOldValue);
}

static LW_INLINE addr_t  archAtomicAddrCas (volatile addr_t *p, addr_t  ulOld, addr_t  ulNew)
{
    addr_t  ulOldValue;

#if LW_CFG_CPU_WORD_LENGHT == 64
    __asm__ __volatile__ (
            PPC_ATOMIC_ENTRY_BARRIER
        "1: ldarx   %0, 0,  %2   \n"
        "   cmpd    0,  %0, %3   \n"
        "   bne-    2f           \n"
        "   stdcx.  %4, 0,  %2   \n"
        "   bne-    1b           \n"
            PPC_ATOMIC_EXIT_BARRIER
        "2:"
            : "=&r" (ulOldValue), "+m" (*p)
            : "r" (p), "r" (ulOld), "r" (ulNew)
            : "cc", "memory");

#else                                                                   /*  LW_CFG_CPU_WORD_LENGHT 64   */
    __asm__ __volatile__ (
            PPC_ATOMIC_ENTRY_BARRIER
        "1: lwarx   %0, 0,  %2   \n"
        "   cmpw    0,  %0, %3   \n"
        "   bne-    2f           \n"
        "   stwcx.  %4, 0,  %2   \n"
        "   bne-    1b           \n"
            PPC_ATOMIC_EXIT_BARRIER
        "2:"
            : "=&r" (ulOldValue), "+m" (*p)
            : "r" (p), "r" (ulOld), "r" (ulNew)
            : "cc", "memory");
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT 32   */

    return  (ulOldValue);
}

#endif                                                                  /*  LW_CFG_CPU_ATOMIC_EN        */
#endif                                                                  /*  __ARCH_PPC_ATOMIC_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
