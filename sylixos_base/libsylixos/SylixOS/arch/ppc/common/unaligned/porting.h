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
** 文   件   名: porting.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 08 月 27 日
**
** 描        述: PowerPC 体系构架非对齐移植.
*********************************************************************************************************/

#ifndef __ARCH_PPC_UNALIGNED_PORTING_H
#define __ARCH_PPC_UNALIGNED_PORTING_H

#include "config.h"

/*********************************************************************************************************
  类型
*********************************************************************************************************/
#if !defined(__ASSEMBLY__) && !defined(ASSEMBLY)

typedef BOOL            bool;

typedef ALTIVEC_REG     __vector128;
typedef __vector128     vector128;
/*********************************************************************************************************
  定义
*********************************************************************************************************/
#define nokprobe_inline                 inline

#define NOKPROBE_SYMBOL(sym)
#define EXPORT_SYMBOL_GPL(sym)

#define preempt_disable()
#define preempt_enable()

#define access_ok(type, va, len)        1
#define user_mode(x)                    1

#define put_user(val, va)               (*va = val, 0)
#define __put_user(val, va)             (*va = val, 0)

#define get_user(val, va)               (val = *va, 0)
#define __get_user(val, va)             (val = *va, 0)

#define __get_user_inatomic(val, va)    (val = *va, 0)
#define __put_user_inatomic(val, va)    (*va = val, 0)

#define EX_TABLE(a, b)

#define FULL_REGS(reg)      1
#define CHECK_FULL_REGS(reg)

#define flush_spe_to_thread(tcb)

#define WARN_ON_ONCE(cond)              \
do {                                    \
    if (cond)                           \
        _PrintFormat("%s(): %d warning!\r\n", __func__, __LINE__); \
} while (0)

#define PPC_WARN_ALIGNMENT(a, b)

#define STACK_INT_FRAME_SIZE            0

#define swab64                          bswap64
#define swab32                          bswap32
#define swab16                          bswap16

static inline void prefetch(const void *x)
{
    if (unlikely(!x))
        return;

    __asm__ __volatile__ ("dcbt 0,%0" : : "r" (x));
}

static inline void prefetchw(const void *x)
{
    if (unlikely(!x))
        return;

    __asm__ __volatile__ ("dcbtst 0,%0" : : "r" (x));
}

#define mb()   __asm__ __volatile__ ("sync" : : : "memory")
#define rmb()  __asm__ __volatile__ ("sync" : : : "memory")
#define wmb()  __asm__ __volatile__ ("sync" : : : "memory")

static inline void eieio(void)
{
    __asm__ __volatile__ ("eieio" : : : "memory");
}

static inline void isync(void)
{
    __asm__ __volatile__ ("isync" : : : "memory");
}
/*********************************************************************************************************
  任务上下文的寄存器
*********************************************************************************************************/
#define gpr                 REG_uiReg
#define ctr                 REG_uiCtr
#define ccr                 REG_uiCr
#define xer                 REG_uiXer
#define msr                 REG_uiMsr
#define nip                 REG_uiPc
#define link                REG_uiLr
#define dar                 REG_uiDar

#endif
/*********************************************************************************************************
  MSR 定义
*********************************************************************************************************/
#if defined(__ASSEMBLY__) || defined(ASSEMBLY)
#define __MASK(X)           (1<<(X))
#else
#define __MASK(X)           (1UL<<(X))
#endif

#define MSR_VEC_LG          25                                      /*  Enable AltiVec                  */
#define MSR_VSX_LG          23                                      /*  Enable VSX                      */
#define MSR_PR_LG           14                                      /*  Problem State / Privilege Level */
#define MSR_FP_LG           13                                      /*  Floating Point enable           */
#define MSR_ME_LG           12                                      /*  Machine Check Enable            */
#define MSR_RI_LG           1                                       /*  Recoverable Exception           */
#define MSR_LE_LG           0                                       /*  Little Endian                   */

#define MSR_VEC             __MASK(MSR_VEC_LG)                      /*  Enable AltiVec                  */
#define MSR_VSX             __MASK(MSR_VSX_LG)                      /*  Enable VSX                      */
#define MSR_PR              __MASK(MSR_PR_LG)                       /*  Problem State / Privilege Level */
#define MSR_FP              __MASK(MSR_FP_LG)                       /*  Floating Point enable           */
#define MSR_ME              __MASK(MSR_ME_LG)                       /*  Machine Check Enable            */
#define MSR_RI              __MASK(MSR_RI_LG)                       /*  Recoverable Exception           */
#define MSR_LE              __MASK(MSR_LE_LG)                       /*  Little Endian                   */

#define MSR_KERNEL          0                                       /*  Big Endian                      */
/*********************************************************************************************************
  寄存器定义
*********************************************************************************************************/
#define SPRN_XER            0x001                                   /*  Fixed Point Exception Register  */
#define SPRN_LR             0x008                                   /*  Link Register                   */
#define SPRN_CTR            0x009                                   /*  Count Register                  */

#define SRR1_PROGPRIV       0x00040000                              /*  Privileged instruction          */
#define SRR1_PROGTRAP       0x00020000                              /*  Trap                            */
/*********************************************************************************************************
  指令定义
*********************************************************************************************************/
#define PPC_INST_COPY       0x7c20060c

#ifdef CONFIG_PPC_BOOK3S_64
#define RFI                 rfid
#define MTMSRD(r)           mtmsrd  r
#define MTMSR_EERI(reg)     mtmsrd  reg,1
#else
#ifndef CONFIG_40x
#define RFI                 rfi
#else
#define RFI                 rfi; b .                                /*  Prevent prefetch past rfi       */
#endif
#define MTMSRD(r)           mtmsr   r
#define MTMSR_EERI(reg)     mtmsr   reg
#endif

#endif                                                              /*  __ARCH_PPC_UNALIGNED_PORTING_H  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
