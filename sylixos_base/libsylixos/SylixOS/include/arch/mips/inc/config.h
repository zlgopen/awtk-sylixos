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
** 文件创建日期: 2017 年 11 月 28 日
**
** 描        述: MIPS 配置.
*********************************************************************************************************/

#ifndef __ARCH_MIPSCONFIG_H
#define __ARCH_MIPSCONFIG_H

#if LW_CFG_CPU_WORD_LENGHT == 32
#define CONFIG_32BIT                    32                              /*  使用 32 位                  */
#undef  CONFIG_64BIT
#else
#define CONFIG_64BIT                    64                              /*  使用 64 位                  */
#undef  CONFIG_32BIT
#endif

#define CONFIG_DEBUG_FPU_EMU                                            /*  FPU 模拟使能调试            */

#define MIPS_CONFIG_EVA                 0                               /*  暂不支持增强虚拟地址(EVA)   */
#undef  CONFIG_EVA                                                      /*  暂不支持增强虚拟地址(EVA)   */

#if LW_CFG_CPU_WORD_LENGHT == 32
#define CONFIG_MIPS32_O32               1                               /*  O32 ABI                     */
#define CONFIG_MIPS_O32_FP64_SUPPORT    0
#else
#define CONFIG_MIPS32_O32               0                               /*  N64 ABI                     */
#define CONFIG_MIPS_O32_FP64_SUPPORT    0
#endif

#undef  CONFIG_SYS_SUPPORTS_MIPS16                                      /*  暂不支持 MIPS16             */
#undef  CONFIG_SYS_SUPPORTS_MICROMIPS                                   /*  暂不支持 MICROMIPS          */
#undef  CONFIG_CPU_MICROMIPS                                            /*  暂不支持 MICROMIPS          */
#undef  CONFIG_CPU_CAVIUM_OCTEON                                        /*  暂不支持 CAVIUM OCTEON      */

#if defined(_MIPS_ARCH_MIPS64R2) || \
    defined(_MIPS_ARCH_MIPS64R3) || \
    defined(_MIPS_ARCH_MIPS64R5) || \
    defined(_MIPS_ARCH_MIPS64R6) || \
    defined(_MIPS_ARCH_MIPS32R2) || \
    defined(_MIPS_ARCH_MIPS32R3) || \
    defined(_MIPS_ARCH_MIPS32R5) || \
    defined(_MIPS_ARCH_MIPS32R6) || \
    defined(_MIPS_ARCH_HR2)
#define NO_R6EMU                        0                               /*  支持 MIPSR2-R6 模拟         */
#else
#define NO_R6EMU                        1                               /*  不支持 MIPSR2-R6 模拟       */
#endif

#if defined(_MIPS_ARCH_MIPS64R6) || \
    defined(_MIPS_ARCH_MIPS32R6)
#define CONFIG_CPU_MIPSR6               1                               /*  支持 MIPSR6                 */
#else
#undef  CONFIG_CPU_MIPSR6                                               /*  不支持 MIPSR6               */
#endif

#endif                                                                  /*  __ARCH_MIPSCONFIG_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
