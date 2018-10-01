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
** 文   件   名: arch_regs32.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 06 月 25 日
**
** 描        述: x86 寄存器相关.
*********************************************************************************************************/

#ifndef __X86_ARCH_REGS32_H
#define __X86_ARCH_REGS32_H

/*********************************************************************************************************
  定义
*********************************************************************************************************/

#define ARCH_REG_CTX_WORD_SIZE  14                                      /*  寄存器上下文字数            */
#define ARCH_STK_MIN_WORD_SIZE  256                                     /*  堆栈最小字数                */

#define ARCH_REG_SIZE           4                                       /*  寄存器大小                  */
#define ARCH_REG_CTX_SIZE       (ARCH_REG_CTX_WORD_SIZE * ARCH_REG_SIZE)/*  寄存器上下文大小            */

#define ARCH_STK_ALIGN_SIZE     8                                       /*  堆栈对齐要求                */

#define ARCH_JMP_BUF_WORD_SIZE  14                                      /*  跳转缓冲字数(向后兼容)      */

/*********************************************************************************************************
  寄存器在 ARCH_REG_CTX 中的偏移量
*********************************************************************************************************/

#define XEAX                    (0  * ARCH_REG_SIZE)
#define XEBX                    (1  * ARCH_REG_SIZE)
#define XECX                    (2  * ARCH_REG_SIZE)
#define XEDX                    (3  * ARCH_REG_SIZE)
#define XESI                    (4  * ARCH_REG_SIZE)
#define XEDI                    (5  * ARCH_REG_SIZE)
#define XEBP                    (6  * ARCH_REG_SIZE)
#define XPAD                    (7  * ARCH_REG_SIZE)
#define XERROR                  (8  * ARCH_REG_SIZE)
#define XEIP                    (9  * ARCH_REG_SIZE)
#define XCS                     (10 * ARCH_REG_SIZE)
#define XEFLAGS                 (11 * ARCH_REG_SIZE)
#define XESP                    (12 * ARCH_REG_SIZE)
#define XSS                     (13 * ARCH_REG_SIZE)

/*********************************************************************************************************
  汇编代码不包含以下内容
*********************************************************************************************************/

#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))

/*********************************************************************************************************
  寄存器表
*********************************************************************************************************/

typedef UINT32      ARCH_REG_T;

struct __ARCH_REG_CTX {
    ARCH_REG_T      REG_uiEAX;                                          /*  4 个数据寄存器              */
    ARCH_REG_T      REG_uiEBX;
    ARCH_REG_T      REG_uiECX;
    ARCH_REG_T      REG_uiEDX;

    ARCH_REG_T      REG_uiESI;                                          /*  2 个变址和指针寄存器        */
    ARCH_REG_T      REG_uiEDI;

    ARCH_REG_T      REG_uiEBP;                                          /*  栈帧指针寄存器              */
    ARCH_REG_T      REG_uiPad;                                          /*  PAD                         */

    ARCH_REG_T      REG_uiError;                                        /*  ERROR CODE                  */
    ARCH_REG_T      REG_uiEIP;                                          /*  指令指针寄存器(EIP)         */
    ARCH_REG_T      REG_uiCS;                                           /*  代码段寄存器(CS)            */
    ARCH_REG_T      REG_uiEFLAGS;                                       /*  标志寄存器(EFLAGS)          */
    ARCH_REG_T      REG_uiESP;                                          /*  堆栈指针寄存器              */
    ARCH_REG_T      REG_uiSS;                                           /*  代码段寄存器(CS)            */

#define REG_ERROR   REG_uiError
#define REG_XIP     REG_uiEIP
#define REG_XFLAGS  REG_uiEFLAGS
} __attribute__ ((packed));

typedef struct __ARCH_REG_CTX   ARCH_REG_CTX;

/*********************************************************************************************************
  调用回溯堆栈表
*********************************************************************************************************/

typedef struct {
    ARCH_REG_T      FP_uiRetAddr;
    ARCH_REG_T      FP_uiArg;
} ARCH_FP_CTX;

/*********************************************************************************************************
  从上下文中获取信息
*********************************************************************************************************/

#define ARCH_REG_CTX_GET_PC(ctx)    ((PVOID)(ctx).REG_uiEIP)
#define ARCH_REG_CTX_GET_FRAME(ctx) ((PVOID)(ctx).REG_uiEBP)
#define ARCH_REG_CTX_GET_STACK(ctx) ((PVOID)&(ctx))                     /*  不准确, 仅为了兼容性设计    */

#endif                                                                  /*  !defined(__ASSEMBLY__)      */
#endif                                                                  /*  __X86_ARCH_REGS32_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
