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
** 文   件   名: assembler.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: RISC-V 汇编相关.
*********************************************************************************************************/

#ifndef __ASMRISCV_ASSEMBLER_H
#define __ASMRISCV_ASSEMBLER_H

#include "arch/riscv/arch_def.h"

/*********************************************************************************************************
  RISC-V architecture assembly special code
*********************************************************************************************************/

#if defined(__ASSEMBLY__) || defined(ASSEMBLY)

#ifndef __MP_CFG_H
#include "../SylixOS/config/mp/mp_cfg.h"
#endif

/*********************************************************************************************************
  assembler define
*********************************************************************************************************/

#ifdef __GNUC__
#  define EXPORT_LABEL(label)       .global label
#  define IMPORT_LABEL(label)       .extern label

#  define FUNC_LABEL(func)          func:
#  define LINE_LABEL(line)          line:

#  define FUNC_DEF(func)  \
        .type   func, %function;  \
func:

#  define FUNC_END(func)    \
        .size   func, . - func
        
#  define MACRO_DEF(mfunc...)   \
        .macro  mfunc
        
#  define MACRO_END()   \
        .endm

#  define FILE_BEGIN()  \
        .text

#  define FILE_END()    \
        .end
        
#  define SECTION(sec)  \
        .section sec

#  define WEAK(sym)     \
        .weak   sym

#  define EXCE_DEF(func)  \
        .balign 16;     \
        .type func, %function;  \
func:

#else                                                                   /*  __GNUC__                    */

#endif                                                                  /*  !__GNUC__                   */

/*********************************************************************************************************
  size define
*********************************************************************************************************/

#ifndef LW_CFG_KB_SIZE
#define LW_CFG_KB_SIZE          (1024)
#define LW_CFG_MB_SIZE          (1024 * LW_CFG_KB_SIZE)
#define LW_CFG_GB_SIZE          (1024 * LW_CFG_MB_SIZE)
#endif

/*********************************************************************************************************
  汇编器不允许大写字母寄存器, 定义大写字母的寄存器
*********************************************************************************************************/

#define X0          x0
#define X1          x1
#define X2          x2
#define X3          x3
#define X4          x4
#define X5          x5
#define X6          x6
#define X7          x7
#define X8          x8
#define X9          x9
#define X10         x10
#define X11         x11
#define X12         x12
#define X13         x13
#define X14         x14
#define X15         x15
#define X16         x16
#define X17         x17
#define X18         x18
#define X19         x19
#define X20         x20
#define X21         x21
#define X22         x22
#define X23         x23
#define X24         x24
#define X25         x25
#define X26         x26
#define X27         x27
#define X28         x28
#define X29         x29
#define X30         x30
#define X31         x31

#define F0          f0
#define F1          f1
#define F2          f2
#define F3          f3
#define F4          f4
#define F5          f5
#define F6          f6
#define F7          f7
#define F8          f8
#define F9          f9
#define F10         f10
#define F11         f11
#define F12         f12
#define F13         f13
#define F14         f14
#define F15         f15
#define F16         f16
#define F17         f17
#define F18         f18
#define F19         f19
#define F20         f20
#define F21         f21
#define F22         f22
#define F23         f23
#define F24         f24
#define F25         f25
#define F26         f26
#define F27         f27
#define F28         f28
#define F29         f29
#define F30         f30
#define F31         f31

/*********************************************************************************************************
  按 ABI 定义寄存器
*********************************************************************************************************/

#define ZERO        x0
#define RA          x1
#define SP          x2
#define GP          x3
#define TP          x4
#define T0          x5
#define T1          x6
#define T2          x7
#define S0          x8
#define S1          x9
#define A0          x10
#define A1          x11
#define A2          x12
#define A3          x13
#define A4          x14
#define A5          x15
#define A6          x16
#define A7          x17
#define S2          x18
#define S3          x19
#define S4          x20
#define S5          x21
#define S6          x22
#define S7          x23
#define S8          x24
#define S9          x25
#define S10         x26
#define S11         x27
#define T3          x28
#define T4          x29
#define T5          x30
#define T6          x31

#define FP          S0
#define RV0         A0
#define RV1         A1

/*********************************************************************************************************
  32 位 64 位适配宏
*********************************************************************************************************/

#define __ASM_STR(x)    x

#if __riscv_xlen == 64
#define __REG_SEL(a, b) __ASM_STR(a)
#elif __riscv_xlen == 32
#define __REG_SEL(a, b) __ASM_STR(b)
#else
#error "Unexpected __riscv_xlen"
#endif

#define REG_L           __REG_SEL(ld, lw)
#define REG_S           __REG_SEL(sd, sw)
#define SZREG           __REG_SEL(8, 4)
#define LGREG           __REG_SEL(3, 2)

#endif                                                                  /*  __ASSEMBLY__                */

#endif                                                                  /*  __ASMRISCV_ASSEMBLER_H      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
