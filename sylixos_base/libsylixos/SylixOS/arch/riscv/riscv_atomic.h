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
** 文   件   名: riscv_atomic.h
**
** 创   建   人: Jiao.Jinxing (焦进星)
**
** 文件创建日期: 2018 年 07 月 27 日
**
** 描        述: RISC-V 体系构架 ATOMIC 接口.
*********************************************************************************************************/

#ifndef __ARCH_RISCV_ATOMIC_H
#define __ARCH_RISCV_ATOMIC_H

/*********************************************************************************************************
  ATOMIC
*********************************************************************************************************/
#if LW_CFG_CPU_ATOMIC_EN > 0

#define ATOMIC_OP_RETURN(op, asm_op, c_op, I)                   \
static LW_INLINE INT  archAtomic##op (INT  i, atomic_t  *v)     \
{                                                               \
    REGISTER INT  iRet;                                         \
                                                                \
    __asm__ __volatile__ (                                      \
        "   amo" #asm_op ".w.aqrl  %1, %2, %0"                  \
        : "+A" (v->counter), "=r" (iRet)                        \
        : "r" (I)                                               \
        : "memory");                                            \
    return  (iRet c_op I);                                      \
}

ATOMIC_OP_RETURN(Add, add, +,  i)
ATOMIC_OP_RETURN(Sub, add, +, -i)
ATOMIC_OP_RETURN(And, and, &,  i)
ATOMIC_OP_RETURN(Or,  or,  |,  i)
ATOMIC_OP_RETURN(Xor, xor, ^,  i)

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
             INT    iResult;
    REGISTER UINT   uiTemp;

    __asm__ __volatile__ (
        "0: lr.w    %0, %2              \n"
        "   bne     %0, %z3, 1f         \n"
        "   sc.w.rl %1, %z4, %2         \n"
        "   bnez    %1, 0b              \n"
        "   fence   rw, rw              \n"
        "1:                             \n"
        : "=&r" (iResult), "=&r" (uiTemp), "+A" (v->counter)
        : "rJ" (iOld), "rJ" (iNew)
        : "memory");

    return  (iResult);
}

static LW_INLINE addr_t  archAtomicAddrCas (volatile addr_t *p, addr_t  ulOld, addr_t  ulNew)
{
             addr_t ulResult;
    REGISTER UINT   uiTemp;

#if LW_CFG_CPU_WORD_LENGHT == 64
    __asm__ __volatile__ (
        "0: lr.d    %0, %2              \n"
        "   bne     %0, %z3, 1f         \n"
        "   sc.d.rl %1, %z4, %2         \n"
        "   bnez    %1, 0b              \n"
        "   fence   rw, rw              \n"
        "1:                             \n"
        : "=&r" (ulResult), "=&r" (uiTemp), "+A" (*p)
        : "rJ" (ulOld), "rJ" (ulNew)
        : "memory");

#else                                                                   /*  LW_CFG_CPU_WORD_LENGHT 64   */
    __asm__ __volatile__ (
        "0: lr.w    %0, %2              \n"
        "   bne     %0, %z3, 1f         \n"
        "   sc.w.rl %1, %z4, %2         \n"
        "   bnez    %1, 0b              \n"
        "   fence   rw, rw              \n"
        "1:                             \n"
        : "=&r" (ulResult), "=&r" (uiTemp), "+A" (*p)
        : "rJ" (ulOld), "rJ" (ulNew)
        : "memory");
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT 32   */

    return  (ulResult);
}

#endif                                                                  /*  LW_CFG_CPU_ATOMIC_EN        */
#endif                                                                  /*  __ARCH_RISCV_ATOMIC_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
