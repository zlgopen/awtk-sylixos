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
** 文   件   名: arch_regs.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: RISC-V 寄存器相关.
*********************************************************************************************************/

#ifndef __RISCV_ARCH_REGS_H
#define __RISCV_ARCH_REGS_H

/*********************************************************************************************************
  定义
*********************************************************************************************************/

#if LW_CFG_CPU_WORD_LENGHT == 32
#define ARCH_REG_SIZE           4                                       /*  寄存器大小                  */
#define ARCH_STK_ALIGN_SIZE     8                                       /*  堆栈对齐要求                */
#define ARCH_STK_MIN_WORD_SIZE  256                                     /*  堆栈最小字数                */
#else
#define ARCH_REG_SIZE           8                                       /*  寄存器大小                  */
#define ARCH_STK_ALIGN_SIZE     16                                      /*  堆栈对齐要求                */
#define ARCH_STK_MIN_WORD_SIZE  512                                     /*  堆栈最小字数                */
#endif

#define ARCH_REG_CTX_WORD_SIZE  38                                      /*  寄存器上下文字数            */
#define ARCH_REG_CTX_SIZE       (ARCH_REG_CTX_WORD_SIZE * ARCH_REG_SIZE)/*  寄存器上下文大小            */
#define ARCH_JMP_BUF_WORD_SIZE  ARCH_REG_CTX_WORD_SIZE                  /*  跳转缓冲字数                */

/*********************************************************************************************************
  寄存器在 ARCH_REG_CTX 中的偏移量
*********************************************************************************************************/

#define CTXTYPE_OFFSET  (0  * ARCH_REG_SIZE)
#define ZERO_OFFSET     (1  * ARCH_REG_SIZE)
#define RA_OFFSET       (2  * ARCH_REG_SIZE)
#define SP_OFFSET       (3  * ARCH_REG_SIZE)
#define GP_OFFSET       (4  * ARCH_REG_SIZE)
#define TP_OFFSET       (5  * ARCH_REG_SIZE)
#define T0_OFFSET       (6  * ARCH_REG_SIZE)
#define T1_OFFSET       (7  * ARCH_REG_SIZE)
#define T2_OFFSET       (8  * ARCH_REG_SIZE)
#define S0_OFFSET       (9  * ARCH_REG_SIZE)
#define S1_OFFSET       (10 * ARCH_REG_SIZE)
#define A0_OFFSET       (11 * ARCH_REG_SIZE)
#define A1_OFFSET       (12 * ARCH_REG_SIZE)
#define A2_OFFSET       (13 * ARCH_REG_SIZE)
#define A3_OFFSET       (14 * ARCH_REG_SIZE)
#define A4_OFFSET       (15 * ARCH_REG_SIZE)
#define A5_OFFSET       (16 * ARCH_REG_SIZE)
#define A6_OFFSET       (17 * ARCH_REG_SIZE)
#define A7_OFFSET       (18 * ARCH_REG_SIZE)
#define S2_OFFSET       (19 * ARCH_REG_SIZE)
#define S3_OFFSET       (20 * ARCH_REG_SIZE)
#define S4_OFFSET       (21 * ARCH_REG_SIZE)
#define S5_OFFSET       (22 * ARCH_REG_SIZE)
#define S6_OFFSET       (23 * ARCH_REG_SIZE)
#define S7_OFFSET       (24 * ARCH_REG_SIZE)
#define S8_OFFSET       (25 * ARCH_REG_SIZE)
#define S9_OFFSET       (26 * ARCH_REG_SIZE)
#define S10_OFFSET      (27 * ARCH_REG_SIZE)
#define S11_OFFSET      (28 * ARCH_REG_SIZE)
#define T3_OFFSET       (29 * ARCH_REG_SIZE)
#define T4_OFFSET       (30 * ARCH_REG_SIZE)
#define T5_OFFSET       (31 * ARCH_REG_SIZE)
#define T6_OFFSET       (32 * ARCH_REG_SIZE)
#define STATUS_OFFSET   (33 * ARCH_REG_SIZE)
#define EPC_OFFSET      (34 * ARCH_REG_SIZE)
#define TVAL_OFFSET     (35 * ARCH_REG_SIZE)
#define CAUSE_OFFSET    (36 * ARCH_REG_SIZE)

#define CPUID_OFFSET    (0  * ARCH_REG_SIZE)

/*********************************************************************************************************
  汇编代码不包含以下内容
*********************************************************************************************************/

#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))

/*********************************************************************************************************
  寄存器表
*********************************************************************************************************/

#if LW_CFG_CPU_WORD_LENGHT == 32
typedef UINT32      ARCH_REG_T;
#else
typedef UINT64      ARCH_REG_T;
#endif

typedef struct __ARCH_REG_CTX {
    ARCH_REG_T      REG_ulSmallCtx;
    ARCH_REG_T      REG_ulReg[32];
    ARCH_REG_T      REG_ulStatus;
    ARCH_REG_T      REG_ulEpc;
    ARCH_REG_T      REG_ulTrapVal;
    ARCH_REG_T      REG_ulCause;
    ARCH_REG_T      REG_ulPad;

#define REG_ZERO    0
#define REG_RA      1
#define REG_SP      2
#define REG_GP      3
#define REG_TP      4
#define REG_T0      5
#define REG_T1      6
#define REG_T2      7
#define REG_S0      8
#define REG_S1      9
#define REG_A0      10
#define REG_A1      11
#define REG_A2      12
#define REG_A3      13
#define REG_A4      14
#define REG_A5      15
#define REG_A6      16
#define REG_A7      17
#define REG_S2      18
#define REG_S3      19
#define REG_S4      20
#define REG_S5      21
#define REG_S6      22
#define REG_S7      23
#define REG_S8      24
#define REG_S9      25
#define REG_S10     26
#define REG_S11     27
#define REG_T3      28
#define REG_T4      29
#define REG_T5      30
#define REG_T6      31
#define REG_FP      REG_S0
} ARCH_REG_CTX;

/*********************************************************************************************************
  调用回溯堆栈表
*********************************************************************************************************/

typedef struct {
    ARCH_REG_T      FP_ulFp;
    ARCH_REG_T      FP_ulLr;
} ARCH_FP_CTX;

/*********************************************************************************************************
  从上下文中获取信息
*********************************************************************************************************/

#define ARCH_REG_CTX_GET_PC(ctx)    ((PVOID)(ctx).REG_ulEpc)
#define ARCH_REG_CTX_GET_FRAME(ctx) ((PVOID)(ctx).REG_ulReg[REG_FP])
#define ARCH_REG_CTX_GET_STACK(ctx) ((PVOID)&(ctx))                     /*  不准确, 仅为了兼容性设计    */

#endif                                                                  /*  !defined(__ASSEMBLY__)      */
#endif                                                                  /*  __RISCV_ARCH_REGS_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
