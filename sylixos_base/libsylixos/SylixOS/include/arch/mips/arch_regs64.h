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
** 文   件   名: arch_regs64.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: MIPS64 寄存器相关.
*********************************************************************************************************/

#ifndef __MIPS_ARCH_REGS64_H
#define __MIPS_ARCH_REGS64_H

/*********************************************************************************************************
  定义
*********************************************************************************************************/

#define ARCH_GREG_NR            32                                      /*  通用寄存器数目              */

#define ARCH_REG_CTX_WORD_SIZE  38                                      /*  寄存器上下文字数            */
#define ARCH_STK_MIN_WORD_SIZE  512                                     /*  堆栈最小字数                */

#define ARCH_REG_SIZE           8                                       /*  寄存器大小                  */
#define ARCH_REG_CTX_SIZE       (ARCH_REG_CTX_WORD_SIZE * ARCH_REG_SIZE)/*  寄存器上下文大小            */

#if defined(_MIPS_ARCH_HR2)
#define ARCH_STK_ALIGN_SIZE     32                                      /*  堆栈对齐要求                */
#else
#define ARCH_STK_ALIGN_SIZE     16                                      /*  堆栈对齐要求                */
#endif                                                                  /*  defined(_MIPS_ARCH_HR2)     */

#define ARCH_JMP_BUF_WORD_SIZE  38                                      /*  跳转缓冲字数(向后兼容)      */

/*********************************************************************************************************
  寄存器在 ARCH_REG_CTX 中的偏移量
*********************************************************************************************************/

#define XGREG(n)                ((n) * ARCH_REG_SIZE)
#define XLO                     ((ARCH_GREG_NR + 0) * ARCH_REG_SIZE)
#define XHI                     ((ARCH_GREG_NR + 1) * ARCH_REG_SIZE)
#define XCAUSE                  ((ARCH_GREG_NR + 2) * ARCH_REG_SIZE)
#define XSR                     ((ARCH_GREG_NR + 3) * ARCH_REG_SIZE)
#define XEPC                    ((ARCH_GREG_NR + 4) * ARCH_REG_SIZE)
#define XBADVADDR               ((ARCH_GREG_NR + 5) * ARCH_REG_SIZE)

/*********************************************************************************************************
  寄存器表
*********************************************************************************************************/

#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))

typedef UINT64      ARCH_REG_T;

typedef struct {
    ARCH_REG_T  REG_ulReg[ARCH_GREG_NR];                                /*  32 个通用目的寄存器         */
    ARCH_REG_T  REG_ulCP0DataLO;                                        /*  除数低位寄存器              */
    ARCH_REG_T  REG_ulCP0DataHI;                                        /*  除数高位寄存器              */
    ARCH_REG_T  REG_ulCP0Cause;                                         /*  产生中断或者异常查看的寄存器*/
    ARCH_REG_T  REG_ulCP0Status;                                        /*  CP0 协处理器状态寄存器      */
    ARCH_REG_T  REG_ulCP0EPC;                                           /*  程序计数器寄存器            */
    ARCH_REG_T  REG_ulCP0BadVAddr;                                      /*  出错地址寄存器              */

/*********************************************************************************************************
  MIPS64 N64 ABI 的寄存器索引
*********************************************************************************************************/
#define REG_ZERO                0                                       /*  wired zero                  */
#define REG_AT                  1                                       /*  assembler temp              */
#define REG_V0                  2                                       /*  return reg 0                */
#define REG_V1                  3                                       /*  return reg 1                */
#define REG_A0                  4                                       /*  arg reg 0                   */
#define REG_A1                  5                                       /*  arg reg 1                   */
#define REG_A2                  6                                       /*  arg reg 2                   */
#define REG_A3                  7                                       /*  arg reg 3                   */
#define REG_A4                  8                                       /*  arg reg 4                   */
#define REG_A5                  9                                       /*  arg reg 5                   */
#define REG_A6                  10                                      /*  arg reg 6                   */
#define REG_A7                  11                                      /*  arg reg 7                   */
#define REG_T0                  12                                      /*  caller saved 0              */
#define REG_T1                  13                                      /*  caller saved 1              */
#define REG_T2                  14                                      /*  caller saved 2              */
#define REG_T3                  15                                      /*  caller saved 3              */
#define REG_S0                  16                                      /*  callee saved 0              */
#define REG_S1                  17                                      /*  callee saved 1              */
#define REG_S2                  18                                      /*  callee saved 2              */
#define REG_S3                  19                                      /*  callee saved 3              */
#define REG_S4                  20                                      /*  callee saved 4              */
#define REG_S5                  21                                      /*  callee saved 5              */
#define REG_S6                  22                                      /*  callee saved 6              */
#define REG_S7                  23                                      /*  callee saved 7              */
#define REG_T8                  24                                      /*  caller saved 8              */
#define REG_T9                  25                                      /*  caller saved 9              */
#define REG_K0                  26                                      /*  kernel temp 0               */
#define REG_K1                  27                                      /*  kernel temp 1               */
#define REG_GP                  28                                      /*  global pointer              */
#define REG_SP                  29                                      /*  stack pointer               */
#define REG_S8                  30                                      /*  callee saved 8              */
#define REG_FP                  REG_S8                                  /*  callee saved 8              */
#define REG_RA                  31                                      /*  return address              */
} ARCH_REG_CTX;

/*********************************************************************************************************
  调用回溯堆栈表
*********************************************************************************************************/

typedef struct {
#define ARCH_ARG_REG_NR         4
    ARCH_REG_T  FP_ulArg[ARCH_ARG_REG_NR];                              /*  N64 ABI 不需要为参数预留栈区*/
} ARCH_FP_CTX;

/*********************************************************************************************************
  从上下文中获取信息
*********************************************************************************************************/

#define ARCH_REG_CTX_GET_PC(ctx)    ((void *)(ctx).REG_ulCP0EPC)

#endif                                                                  /*  !defined(__ASSEMBLY__)      */
#endif                                                                  /*  __MIPS_ARCH_REGS64_H        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
