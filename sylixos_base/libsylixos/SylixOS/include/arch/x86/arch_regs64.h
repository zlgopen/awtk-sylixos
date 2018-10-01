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
** 文件创建日期: 2017 年 06 月 05 日
**
** 描        述: x86-64 寄存器相关.
*********************************************************************************************************/

#ifndef __X86_ARCH_REGS64_H
#define __X86_ARCH_REGS64_H

/*********************************************************************************************************
  定义
*********************************************************************************************************/

#define ARCH_REG_CTX_WORD_SIZE  22                                      /*  寄存器上下文字数            */
#define ARCH_STK_MIN_WORD_SIZE  512                                     /*  堆栈最小字数                */

#define ARCH_REG_SIZE           8                                       /*  寄存器大小                  */
#define ARCH_REG_CTX_SIZE       (ARCH_REG_CTX_WORD_SIZE * ARCH_REG_SIZE)/*  寄存器上下文大小            */

#define ARCH_STK_ALIGN_SIZE     16                                      /*  堆栈对齐要求                */

#define ARCH_JMP_BUF_WORD_SIZE  22                                      /*  跳转缓冲字数(向后兼容)      */

/*********************************************************************************************************
  寄存器在 ARCH_REG_CTX 中的偏移量
*********************************************************************************************************/

#define XRAX                    (0  * ARCH_REG_SIZE)
#define XRBX                    (1  * ARCH_REG_SIZE)
#define XRCX                    (2  * ARCH_REG_SIZE)
#define XRDX                    (3  * ARCH_REG_SIZE)
#define XRSI                    (4  * ARCH_REG_SIZE)
#define XRDI                    (5  * ARCH_REG_SIZE)
#define XR8                     (6  * ARCH_REG_SIZE)
#define XR9                     (7  * ARCH_REG_SIZE)
#define XR10                    (8  * ARCH_REG_SIZE)
#define XR11                    (9  * ARCH_REG_SIZE)
#define XR12                    (10 * ARCH_REG_SIZE)
#define XR13                    (11 * ARCH_REG_SIZE)
#define XR14                    (12 * ARCH_REG_SIZE)
#define XR15                    (13 * ARCH_REG_SIZE)
#define XRBP                    (14 * ARCH_REG_SIZE)
#define XPAD                    (15 * ARCH_REG_SIZE)
#define XERROR                  (16 * ARCH_REG_SIZE)
#define XRIP                    (17 * ARCH_REG_SIZE)
#define XCS                     (18 * ARCH_REG_SIZE)
#define XRFLAGS                 (19 * ARCH_REG_SIZE)
#define XRSP                    (20 * ARCH_REG_SIZE)
#define XSS                     (21 * ARCH_REG_SIZE)

/*********************************************************************************************************
  汇编代码不包含以下内容
*********************************************************************************************************/

#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))

/*********************************************************************************************************
  寄存器表
*********************************************************************************************************/

typedef UINT64          ARCH_REG_T;

struct __ARCH_REG_CTX {
    ARCH_REG_T          REG_ulRAX;                                      /*  4 个数据寄存器              */
    ARCH_REG_T          REG_ulRBX;
    ARCH_REG_T          REG_ulRCX;
    ARCH_REG_T          REG_ulRDX;

    ARCH_REG_T          REG_ulRSI;                                      /*  2 个变址和指针寄存器        */
    ARCH_REG_T          REG_ulRDI;

    ARCH_REG_T          REG_ulR8;                                       /*  8 个数据寄存器              */
    ARCH_REG_T          REG_ulR9;
    ARCH_REG_T          REG_ulR10;
    ARCH_REG_T          REG_ulR11;
    ARCH_REG_T          REG_ulR12;
    ARCH_REG_T          REG_ulR13;
    ARCH_REG_T          REG_ulR14;
    ARCH_REG_T          REG_ulR15;

    ARCH_REG_T          REG_ulRBP;                                      /*  栈帧指针寄存器              */
    ARCH_REG_T          REG_uiPad;                                      /*  PAD                         */

    ARCH_REG_T          REG_ulError;                                    /*  ERROR CODE                  */
    ARCH_REG_T          REG_ulRIP;                                      /*  指令指针寄存器(RIP)         */
    ARCH_REG_T          REG_ulCS;                                       /*  代码段寄存器(CS)            */
    ARCH_REG_T          REG_ulRFLAGS;                                   /*  标志寄存器(RFLAGS)          */
    ARCH_REG_T          REG_ulRSP;                                      /*  原栈指针寄存器(RSP)         */
    ARCH_REG_T          REG_ulSS;                                       /*  栈段寄存器(SS)              */

#define REG_ERROR       REG_ulError
#define REG_XIP         REG_ulRIP
#define REG_XFLAGS      REG_ulRFLAGS
} __attribute__ ((packed));

typedef struct __ARCH_REG_CTX   ARCH_REG_CTX;

/*********************************************************************************************************
  调用回溯堆栈表
*********************************************************************************************************/

typedef struct {
    ARCH_REG_T          FP_ulRetAddr;
    ARCH_REG_T          FP_ulSavedRBP;
} ARCH_FP_CTX;

/*********************************************************************************************************
  从上下文中获取信息
*********************************************************************************************************/

#define ARCH_REG_CTX_GET_PC(ctx)    ((PVOID)(ctx).REG_ulRIP)
#define ARCH_REG_CTX_GET_FRAME(ctx) ((PVOID)(ctx).REG_ulRBP)
#define ARCH_REG_CTX_GET_STACK(ctx) ((PVOID)&(ctx))                    /*  不准确, 仅为了兼容性设计    */

#endif                                                                  /*  !defined(ASSEMBLY)          */
#endif                                                                  /*  __X86_ARCH_REGS64_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
