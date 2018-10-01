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
** 文件创建日期: 2015 年 11 月 26 日
**
** 描        述: PowerPC 寄存器相关.
*********************************************************************************************************/

#ifndef __PPC_ARCH_REGS_H
#define __PPC_ARCH_REGS_H

/*********************************************************************************************************
  定义
*********************************************************************************************************/

#define ARCH_GREG_NR            32                                      /*  通用寄存器数目              */

#define ARCH_REG_CTX_WORD_SIZE  40                                      /*  寄存器上下文字数            */
#define ARCH_STK_MIN_WORD_SIZE  256                                     /*  堆栈最小字数                */

#define ARCH_REG_SIZE           4                                       /*  寄存器大小                  */
#define ARCH_REG_CTX_SIZE       (ARCH_REG_CTX_WORD_SIZE * ARCH_REG_SIZE)/*  寄存器上下文大小            */

#define ARCH_STK_ALIGN_SIZE     8                                       /*  堆栈对齐要求                */

#define ARCH_JMP_BUF_WORD_SIZE  44                                      /*  跳转缓冲字数(向后兼容)      */

/*********************************************************************************************************
  寄存器在 ARCH_REG_CTX 中的偏移量
*********************************************************************************************************/

#define XR(n)                   ((n) * ARCH_REG_SIZE)
#define XSRR0                   ((ARCH_GREG_NR + 0) * ARCH_REG_SIZE)
#define XSRR1                   ((ARCH_GREG_NR + 1) * ARCH_REG_SIZE)
#define XCTR                    ((ARCH_GREG_NR + 2) * ARCH_REG_SIZE)
#define XXER                    ((ARCH_GREG_NR + 3) * ARCH_REG_SIZE)
#define XCR                     ((ARCH_GREG_NR + 4) * ARCH_REG_SIZE)
#define XLR                     ((ARCH_GREG_NR + 5) * ARCH_REG_SIZE)
#define XDAR                    ((ARCH_GREG_NR + 6) * ARCH_REG_SIZE)

/*********************************************************************************************************
  汇编代码不包含以下内容
*********************************************************************************************************/

#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))

/*********************************************************************************************************
  寄存器表
*********************************************************************************************************/

typedef UINT32      ARCH_REG_T;

typedef struct {
    union {
        ARCH_REG_T          REG_uiReg[32];
        struct {
            ARCH_REG_T      REG_uiR0;
            ARCH_REG_T      REG_uiR1;
            ARCH_REG_T      REG_uiR2;
            ARCH_REG_T      REG_uiR3;
            ARCH_REG_T      REG_uiR4;
            ARCH_REG_T      REG_uiR5;
            ARCH_REG_T      REG_uiR6;
            ARCH_REG_T      REG_uiR7;
            ARCH_REG_T      REG_uiR8;
            ARCH_REG_T      REG_uiR9;
            ARCH_REG_T      REG_uiR10;
            ARCH_REG_T      REG_uiR11;
            ARCH_REG_T      REG_uiR12;
            ARCH_REG_T      REG_uiR13;
            ARCH_REG_T      REG_uiR14;
            ARCH_REG_T      REG_uiR15;
            ARCH_REG_T      REG_uiR16;
            ARCH_REG_T      REG_uiR17;
            ARCH_REG_T      REG_uiR18;
            ARCH_REG_T      REG_uiR19;
            ARCH_REG_T      REG_uiR20;
            ARCH_REG_T      REG_uiR21;
            ARCH_REG_T      REG_uiR22;
            ARCH_REG_T      REG_uiR23;
            ARCH_REG_T      REG_uiR24;
            ARCH_REG_T      REG_uiR25;
            ARCH_REG_T      REG_uiR26;
            ARCH_REG_T      REG_uiR27;
            ARCH_REG_T      REG_uiR28;
            ARCH_REG_T      REG_uiR29;
            ARCH_REG_T      REG_uiR30;
            ARCH_REG_T      REG_uiR31;
        };
    };

    ARCH_REG_T              REG_uiSrr0;
    ARCH_REG_T              REG_uiSrr1;
    ARCH_REG_T              REG_uiCtr;
    ARCH_REG_T              REG_uiXer;
    ARCH_REG_T              REG_uiCr;
    ARCH_REG_T              REG_uiLr;
    ARCH_REG_T              REG_uiDar;
    ARCH_REG_T              REG_uiPad;
#define REG_uiSp            REG_uiR1
#define REG_uiFp            REG_uiR31
#define REG_uiPc            REG_uiSrr0
#define REG_uiMsr           REG_uiSrr1
} ARCH_REG_CTX;

/*********************************************************************************************************
  EABI 标准调用回溯堆栈表
*********************************************************************************************************/

typedef struct {
    ARCH_REG_T              FP_uiFp;
    ARCH_REG_T              FP_uiLr;
} ARCH_FP_CTX;

/*********************************************************************************************************
  从上下文中获取信息
*********************************************************************************************************/

#define ARCH_REG_CTX_GET_PC(ctx)    ((void *)(ctx).REG_uiPc)
#define ARCH_REG_CTX_GET_FRAME(ctx) ((void *)(ctx).REG_uiFp)
#define ARCH_REG_CTX_GET_STACK(ctx) ((void *)&(ctx))                    /*  不准确, 仅为了兼容性设计    */

#endif                                                                  /*  !defined(__ASSEMBLY__)      */
#endif                                                                  /*  __PPC_ARCH_REGS_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
