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
** 创   建   人: Xu.Guizhou (徐贵洲)
**
** 文件创建日期: 2017 年 05 月 15 日
**
** 描        述: SPARC 寄存器相关.
**
*********************************************************************************************************/

#ifndef __SPARC_ARCH_REGS_H
#define __SPARC_ARCH_REGS_H

/*********************************************************************************************************
  定义
*********************************************************************************************************/

#define ARCH_GREG_NR            32                                      /*  通用寄存器数目              */

#define ARCH_REG_CTX_WORD_SIZE  36                                      /*  寄存器上下文字数            */
#define ARCH_STK_MIN_WORD_SIZE  512                                     /*  堆栈最小字数                */

#define ARCH_REG_SIZE           4                                       /*  寄存器大小                  */
#define ARCH_REG_CTX_SIZE       (ARCH_REG_CTX_WORD_SIZE * ARCH_REG_SIZE)/*  寄存器上下文大小            */

#define ARCH_STK_ALIGN_SIZE     8                                       /*  堆栈对齐要求                */
#define ARCH_STK_FRAME_SIZE     96                                      /*  最小栈帧大小                */

#define ASM_STACK_FRAME_SIZE    ARCH_STK_FRAME_SIZE                     /*  老 BSP 兼容                 */

#define ARCH_JMP_BUF_WORD_SIZE  62                                      /*  跳转缓冲字数(向后兼容)      */

/*********************************************************************************************************
  寄存器在 ARCH_REG_CTX 中的偏移量
*********************************************************************************************************/

#define REG_GLOBAL(x)           (((x) + 0)  * ARCH_REG_SIZE)
#define REG_OUTPUT(x)           (((x) + 8)  * ARCH_REG_SIZE)
#define REG_LOCAL(x)            (((x) + 16) * ARCH_REG_SIZE)
#define REG_INPUT(x)            (((x) + 24) * ARCH_REG_SIZE)
#define REG_PSR                 ((ARCH_GREG_NR + 0) * ARCH_REG_SIZE)
#define REG_PC                  ((ARCH_GREG_NR + 1) * ARCH_REG_SIZE)
#define REG_NPC                 ((ARCH_GREG_NR + 2) * ARCH_REG_SIZE)
#define REG_Y                   ((ARCH_GREG_NR + 3) * ARCH_REG_SIZE)

/*********************************************************************************************************
  寄存器在 ARCH_FP_CTX (栈帧结构)中的偏移量
*********************************************************************************************************/

#define SF_LOCAL(x)             (((x) + 0) * ARCH_REG_SIZE)
#define SF_INPUT(x)             (((x) + 8) * ARCH_REG_SIZE)

/*********************************************************************************************************
  寄存器表
*********************************************************************************************************/
#if !defined(__ASSEMBLY__) && !defined(ASSEMBLY)

typedef UINT32      ARCH_REG_T;

struct arch_reg_ctx {
    ARCH_REG_T      REG_uiGlobal[8];                                    /*  Global regs                 */
    ARCH_REG_T      REG_uiOutput[8];                                    /*  Output regs                 */
    ARCH_REG_T      REG_uiLocal[8];                                     /*  Local regs                  */
    ARCH_REG_T      REG_uiInput[8];                                     /*  Input regs                  */
    ARCH_REG_T      REG_uiPsr;                                          /*  Psr reg                     */
    ARCH_REG_T      REG_uiPc;                                           /*  Pc reg                      */
    ARCH_REG_T      REG_uiNPc;                                          /*  NPc reg                     */
    ARCH_REG_T      REG_uiY;                                            /*  Y reg                       */

#define REG_uiFp    REG_uiInput[6]
#define REG_uiRet   REG_uiInput[7]
#define REG_uiSp    REG_uiOutput[6]
} __attribute__ ((aligned(8)));                                         /*  Use STD & LDD ins           */

typedef struct arch_reg_ctx  ARCH_REG_CTX;

/*********************************************************************************************************
  最小栈帧结构
*********************************************************************************************************/

typedef struct {
    ARCH_REG_T      FP_uiLocal[8];
    ARCH_REG_T      FP_uiInput[6];
    ARCH_REG_T      FP_uiFp;
    ARCH_REG_T      FP_uiRetAddr;
    ARCH_REG_T      FP_uiStructPtr;
    ARCH_REG_T      FP_uiXArgs[6];
    ARCH_REG_T      FP_uiXXArgs[1];
} ARCH_FP_CTX;

/*********************************************************************************************************
  从上下文中获取信息
*********************************************************************************************************/

#define ARCH_REG_CTX_GET_PC(ctx)    ((void *)(ctx).REG_uiPc)
#define ARCH_REG_CTX_GET_FRAME(ctx) ((void *)(ctx).REG_uiFp)
#define ARCH_REG_CTX_GET_STACK(ctx) ((void *)&(ctx))                    /*  不准确, 仅为了兼容性设计    */

#endif                                                                  /*  !defined __ASSEMBLY__       */
#endif                                                                  /*  __SPARC_ARCH_REGS_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
