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
** 文   件   名: riscvElf.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: 实现 RISC-V 体系结构的 ELF 文件重定位.
*********************************************************************************************************/

#ifndef __ARCH_RISCV_ELF_H
#define __ARCH_RISCV_ELF_H

#ifdef LW_CFG_CPU_ARCH_RISCV                                            /*  RISC-V 体系结构             */

#if LW_CFG_CPU_WORD_LENGHT == 32
#define ELF_CLASS       ELFCLASS32
#define ELF_ARCH        EM_RISCV
#else
#define ELF_CLASS       ELFCLASS64
#define ELF_ARCH        EM_RISCV
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/

#endif                                                                  /*  LW_CFG_CPU_ARCH_RISCV       */
#endif                                                                  /*  __ARCH_RISCV_ELF_H          */
/*********************************************************************************************************
  END
*********************************************************************************************************/
