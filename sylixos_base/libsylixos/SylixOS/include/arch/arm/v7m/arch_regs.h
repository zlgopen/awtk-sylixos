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
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2013 年 05 月 04 日
**
** 描        述: ARMv7M 寄存器相关.
**
** BUG:
2015.08.19 将 CPU 字长定义移动到这里.
*********************************************************************************************************/

#ifndef __ARMV7M_ARCH_REGS_H
#define __ARMV7M_ARCH_REGS_H

/*********************************************************************************************************
  定义
*********************************************************************************************************/

#define ARCH_REG_CTX_WORD_SIZE  19                                      /*  寄存器上下文字数            */
#define ARCH_STK_MIN_WORD_SIZE  256                                     /*  堆栈最小字数                */

#define ARCH_REG_SIZE           4                                       /*  寄存器大小                  */
#define ARCH_HW_SAVE_CTX_SIZE   (8  * ARCH_REG_SIZE)                    /*  硬件自动保存上下文大小      */
#define ARCH_SW_SAVE_CTX_SIZE   (11 * ARCH_REG_SIZE)                    /*  软件手动保存上下文大小      */
#define ARCH_REG_CTX_SIZE       (ARCH_REG_CTX_WORD_SIZE * ARCH_REG_SIZE)/*  寄存器上下文大小            */

#define ARCH_STK_ALIGN_SIZE     8                                       /*  堆栈对齐要求                */

#define ARCH_JMP_BUF_WORD_SIZE  18                                      /*  跳转缓冲字数(向后兼容)      */

/*********************************************************************************************************
  寄存器表
*********************************************************************************************************/

#if !defined(__ASSEMBLY__) && !defined(ASSEMBLY)

typedef UINT32      ARCH_REG_T;

typedef struct {
    ARCH_REG_T      REG_uiR0;
    ARCH_REG_T      REG_uiR1;
    ARCH_REG_T      REG_uiR2;
    ARCH_REG_T      REG_uiR3;
    ARCH_REG_T      REG_uiR12;
    ARCH_REG_T      REG_uiR14;
    ARCH_REG_T      REG_uiR15;
    ARCH_REG_T      REG_uiXpsr;
} ARCH_HW_SAVE_REG_CTX;

typedef struct {
    ARCH_REG_T      REG_uiR13;
    ARCH_REG_T      REG_uiBASEPRI;
    ARCH_REG_T      REG_uiR4;
    ARCH_REG_T      REG_uiR5;
    ARCH_REG_T      REG_uiR6;
    ARCH_REG_T      REG_uiR7;
    ARCH_REG_T      REG_uiR8;
    ARCH_REG_T      REG_uiR9;
    ARCH_REG_T      REG_uiR10;
    ARCH_REG_T      REG_uiR11;
    ARCH_REG_T      REG_uiExcRet;
} ARCH_SW_SAVE_REG_CTX;

typedef struct {
    ARCH_REG_T      REG_uiR13;
    ARCH_REG_T      REG_uiBASEPRI;
    ARCH_REG_T      REG_uiR4;
    ARCH_REG_T      REG_uiR5;
    ARCH_REG_T      REG_uiR6;
    ARCH_REG_T      REG_uiR7;
    ARCH_REG_T      REG_uiR8;
    ARCH_REG_T      REG_uiR9;
    ARCH_REG_T      REG_uiR10;
    ARCH_REG_T      REG_uiR11;
    ARCH_REG_T      REG_uiExcRet;

    ARCH_REG_T      REG_uiR0;
    ARCH_REG_T      REG_uiR1;
    ARCH_REG_T      REG_uiR2;
    ARCH_REG_T      REG_uiR3;
    ARCH_REG_T      REG_uiR12;
    ARCH_REG_T      REG_uiR14;
    ARCH_REG_T      REG_uiR15;
    ARCH_REG_T      REG_uiXpsr;

#define REG_uiFp    REG_uiR11
#define REG_uiIp    REG_uiR12
#define REG_uiSp    REG_uiR13
#define REG_uiLr    REG_uiR14
#define REG_uiPc    REG_uiR15
} ARCH_REG_CTX;

/*********************************************************************************************************
  EABI 标准调用回溯堆栈表
*********************************************************************************************************/

typedef struct {
    ARCH_REG_T      FP_uiFp;
    ARCH_REG_T      FP_uiLr;
} ARCH_FP_CTX;

/*********************************************************************************************************
  从上下文中获取信息
*********************************************************************************************************/

#define ARCH_REG_CTX_GET_PC(ctx)    ((void *)(ctx).REG_uiPc)
#define ARCH_REG_CTX_GET_FRAME(ctx) ((void *)(ctx).REG_uiFp)
#define ARCH_REG_CTX_GET_STACK(ctx) ((void *)&(ctx))                    /*  不准确, 仅为了兼容性设计    */

#endif                                                                  /*  !defined(__ASSEMBLY__)      */
#endif                                                                  /*  __ARMV7M_ARCH_REGS_H        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
