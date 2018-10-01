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
** 文   件   名: arch_def.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2018 年 07 月 25 日
**
** 描        述: ARM 相关定义.
*********************************************************************************************************/

#ifndef __ARM_ARCH_DEF_H
#define __ARM_ARCH_DEF_H

#include "asm/archprob.h"

/*********************************************************************************************************
  SMP UP
*********************************************************************************************************/
#ifdef __SYLIXOS_KERNEL

#ifndef __MP_CFG_H
#include "../SylixOS/config/mp/mp_cfg.h"
#endif

#if LW_CFG_SMP_EN > 0
#define __ALT_SMP_ASM(smp, up)  smp
#else                                                                   /*  SMP                         */
#define __ALT_SMP_ASM(smp, up)  up
#endif                                                                  /*  UP                          */

/*********************************************************************************************************
  PREFETCH
*********************************************************************************************************/

#if __SYLIXOS_ARM_ARCH__ >= 5
#define ARM_PREFETCH(ptr)   __asm__ __volatile__("pld  %a0" :: "p" (ptr))
#else
#define ARM_PREFETCH(ptr)
#endif                                                                  /*  __SYLIXOS_ARM_ARCH__ >= 5   */

#if __SYLIXOS_ARM_ARCH__ >= 7
#if (LW_CFG_SMP_EN > 0) && (LW_CFG_ARM_PREFETCH_W > 0)
#define ARM_PREFETCH_W(ptr) __asm__ __volatile__("pldw %a0" :: "p" (ptr))
#else
#define ARM_PREFETCH_W(ptr) ARM_PREFETCH(ptr)
#endif
#else
#define ARM_PREFETCH_W(ptr) ARM_PREFETCH(ptr)
#endif                                                                  /*  __SYLIXOS_ARM_ARCH__ >= 7   */

/*********************************************************************************************************
  WFE SEV
*********************************************************************************************************/

#ifdef __SYLIXOS_ARM_ARCH_M__
#define ARM_WFE(cond)       __ALT_SMP_ASM("it " cond "\n\t" \
                                          "wfe" cond ".n",  \
                                          "nop.w")
#define ARM_SEV             __ALT_SMP_ASM("sev.w", "nop.w")
#else                                                                   /*  Thumb-2                     */
#define ARM_WFE(cond)       __ALT_SMP_ASM("wfe" cond, "nop")
#define ARM_SEV             __ALT_SMP_ASM("sev", "nop")
#endif                                                                  /*  ARM                         */

#endif                                                                  /*  defined(__SYLIXOS_KERNEL)   */
#endif                                                                  /*  __ARM_ARCH_DEF_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
