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
** 文   件   名: config.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 08 月 27 日
**
** 描        述: PowerPC 体系构架非对齐配置.
*********************************************************************************************************/

#ifndef __ARCH_PPC_UNALIGNED_CONFIG_H
#define __ARCH_PPC_UNALIGNED_CONFIG_H

#if LW_CFG_CPU_WORD_LENGHT == 32
#define CONFIG_PPC32
#else
#define CONFIG_PPC64
#ifndef __powerpc64__
#define __powerpc64__
#endif
#endif

#if LW_CFG_CPU_FPU_EN >0
#define CONFIG_PPC_FPU
#define CONFIG_SPE
#endif

#if LW_CFG_CPU_DSP_EN >0
#define CONFIG_ALTIVEC
#endif

#if LW_CFG_CPU_ENDIAN == 0
#define __LITTLE_ENDIAN__
#endif

/*********************************************************************************************************
  VSX (Vector Scalar Extension)
  https://en.wikipedia.org/wiki/AltiVec
  现在不支持 VSX
*********************************************************************************************************/
#undef CONFIG_VSX

#define L1_CACHE_BYTES      LW_CFG_CPU_ARCH_CACHE_LINE

#endif                                                                  /* __ARCH_PPC_UNALIGNED_CONFIG_H*/
/*********************************************************************************************************
  END
*********************************************************************************************************/
