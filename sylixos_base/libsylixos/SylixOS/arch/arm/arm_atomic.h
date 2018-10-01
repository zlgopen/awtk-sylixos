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
** 文   件   名: arm_atomic.h
**
** 创   建   人: Jiao.Jinxing (焦进星)
**
** 文件创建日期: 2018 年 07 月 27 日
**
** 描        述: ARM 体系构架 ATOMIC 接口.
*********************************************************************************************************/

#ifndef __ARCH_ARM_ATOMIC_H
#define __ARCH_ARM_ATOMIC_H

/*********************************************************************************************************
  ATOMIC
*********************************************************************************************************/
#if LW_CFG_CPU_ATOMIC_EN > 0

#define ATOMIC_OP_RETURN(op, c_op, asm_op)                                  \
static LW_INLINE INT  archAtomic##op (INT  i, atomic_t  *v)                 \
{                                                                           \
    ULONG   ulTemp;                                                         \
    INT     iResult;                                                        \
                                                                            \
    ARM_PREFETCH_W(&v->counter);                                            \
                                                                            \
    __asm__ __volatile__(                                                   \
        "1: ldrex       %0, [%2]            \n"                             \
        "   " #asm_op " %0, %0, %3          \n"                             \
        "   strex       %1, %0, [%2]        \n"                             \
        "   teq         %1, #0              \n"                             \
        "   bne         1b"                                                 \
            : "=&r" (iResult), "=&r" (ulTemp)                               \
            : "r" (&v->counter), "Ir" (i)                                   \
            : "cc");                                                        \
                                                                            \
    return  (iResult);                                                      \
}

ATOMIC_OP_RETURN(Add,  +=,  add)
ATOMIC_OP_RETURN(Sub,  -=,  sub)
ATOMIC_OP_RETURN(And,  &=,  and)
ATOMIC_OP_RETURN(Or,   |=,  orr)
ATOMIC_OP_RETURN(Xor,  ^=,  eor)

/*********************************************************************************************************
  On ARM, ordinary assignment (str instruction) doesn't clear the local
  strex/ldrex monitor on some implementations. The reason we can use it for
  archAtomicSet() is the clrex or dummy strex done on every exception return.
*********************************************************************************************************/

static LW_INLINE VOID  archAtomicSet (INT  i, atomic_t  *v)
{
    LW_ACCESS_ONCE(INT, v->counter) = i;
}

static LW_INLINE INT  archAtomicGet (atomic_t  *v)
{
    return  (LW_ACCESS_ONCE(INT, v->counter));
}

/*********************************************************************************************************
  atomic cas op
*********************************************************************************************************/

static LW_INLINE INT  archAtomicCas (atomic_t  *v, INT  iOld, INT  iNew)
{
    INT    iOldVal;
    UINT   uiRes;

    ARM_PREFETCH_W(&v->counter);

    do {
        __asm__ __volatile__(
            "ldrex      %1, [%2]        \n"
            "mov        %0, #0          \n"
            "teq        %1, %3          \n"
#if defined(__SYLIXOS_ARM_ARCH_M__)
            "itt        eq              \n"
#endif
            "strexeq    %0, %4, [%2]    \n"
#if defined(__SYLIXOS_ARM_ARCH_M__)
            "nopeq                      \n"
#endif
                : "=&r" (uiRes), "=&r" (iOldVal)
                : "r" (&v->counter), "Ir" (iOld), "r" (iNew)
                : "cc");
    } while (uiRes);

    return  (iOldVal);
}

static LW_INLINE addr_t  archAtomicAddrCas (volatile addr_t *p, addr_t  ulOld, addr_t  ulNew)
{
    addr_t  ulOldVal;
    UINT    uiRes;

    ARM_PREFETCH_W(p);

    do {
        __asm__ __volatile__(
            "ldrex      %1, [%2]        \n"
            "mov        %0, #0          \n"
            "teq        %1, %3          \n"
#if defined(__SYLIXOS_ARM_ARCH_M__)
            "itt        eq              \n"
#endif
            "strexeq    %0, %4, [%2]    \n"
#if defined(__SYLIXOS_ARM_ARCH_M__)
            "nopeq                      \n"
#endif
                : "=&r" (uiRes), "=&r" (ulOldVal)
                : "r" (p), "Ir" (ulOld), "r" (ulNew)
                : "cc");
    } while (uiRes);

    return  (ulOldVal);
}

#endif                                                                  /*  LW_CFG_CPU_ATOMIC_EN        */
#endif                                                                  /*  __ARCH_ARM_ATOMIC_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
