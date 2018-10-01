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
** 文   件   名: archprob.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2017 年 06 月 08 日
**
** 描        述: 平台编译探测.
*********************************************************************************************************/

#ifndef __ARCHPROB_H
#define __ARCHPROB_H

/*********************************************************************************************************
  mips architecture detect
*********************************************************************************************************/

#define __SYLIXOS_MIPS_ARCH_MIPS32      32
#define __SYLIXOS_MIPS_ARCH_MIPS32R2    33
#define __SYLIXOS_MIPS_ARCH_MIPS64      64
#define __SYLIXOS_MIPS_ARCH_MIPS64R2    65

#ifdef __GNUC__
#  if defined(_MIPS_ARCH_MIPS32)
#    define __SYLIXOS_MIPS_ARCH__   __SYLIXOS_MIPS_ARCH_MIPS32

#  elif defined(_MIPS_ARCH_MIPS32R2)
#    define __SYLIXOS_MIPS_ARCH__   __SYLIXOS_MIPS_ARCH_MIPS32R2

#  elif defined(_MIPS_ARCH_MIPS64)
#    define __SYLIXOS_MIPS_ARCH__   __SYLIXOS_MIPS_ARCH_MIPS64

#  elif defined(_MIPS_ARCH_MIPS64R2) || defined(_MIPS_ARCH_HR2)
#    define __SYLIXOS_MIPS_ARCH__   __SYLIXOS_MIPS_ARCH_MIPS64R2

#  endif                                                                /*  user define only            */

#else
#  define __SYLIXOS_MIPS_ARCH__     __SYLIXOS_MIPS_ARCH_MIPS32          /*  default MIPS32              */
#endif

#endif                                                                  /*  __ARCHPROB_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
