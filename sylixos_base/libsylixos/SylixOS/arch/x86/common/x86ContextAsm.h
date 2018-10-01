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
** 文   件   名: x86ContextAsm.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 07 月 04 日
**
** 描        述: x86 体系构架上下文切换.
*********************************************************************************************************/

#ifndef __ARCH_X86CONTEXTASM_H
#define __ARCH_X86CONTEXTASM_H

/*********************************************************************************************************
  头文件
*********************************************************************************************************/

#include "arch/x86/arch_regs.h"
#include "x86Segment.h"

/*********************************************************************************************************

  高地址: +-----------------+
          |        SS       |
          +-----------------+
          |       ESP       |
          +-----------------+
          |      EFLAGS     |
          +-----------------+
          |       CS        |
          +-----------------+
          |       EIP       |
          +-----------------+
          |      ERROR      |
          +-----------------+
          |       PAD       |
          +-----------------+
          |       EBP       |
          +-----------------+
          |       EDI       |
          +-----------------+
          |       ESI       |
          +-----------------+
          |       EDX       |
          +-----------------+
          |       ECX       |
          +-----------------+
          |       EBX       |
          +-----------------+
          |       EAX       |
  低地址: +-----------------+

*********************************************************************************************************/

/*********************************************************************************************************
  保存内核态任务寄存器(参数 %EAX: ARCH_REG_CTX 地址) 会破坏 %ECX
*********************************************************************************************************/

MACRO_DEF(KERN_SAVE_REGS)
    MOVL    %EAX , XEAX(%EAX)
    MOVL    %EBX , XEBX(%EAX)
    MOVL    %ECX , XECX(%EAX)
    MOVL    %EDX , XEDX(%EAX)

    MOVL    %ESI , XESI(%EAX)
    MOVL    %EDI , XEDI(%EAX)

    MOVL    %EBP , XEBP(%EAX)
    MOVL    %ESP , XESP(%EAX)

    MOVL    $archResumePc , XEIP(%EAX)

    MOVW    %CS  , XCS(%EAX)
    MOVW    %SS  , XSS(%EAX)

    PUSHF
    POPL    %ECX
    MOVL    %ECX , XEFLAGS(%EAX)
    MACRO_END()

/*********************************************************************************************************
  恢复内核态任务寄存器(参数 %EAX: ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(KERN_RESTORE_REGS)
    MOVL    XEBX(%EAX) , %EBX
    MOVL    XECX(%EAX) , %ECX
    MOVL    XEDX(%EAX) , %EDX

    MOVL    XESI(%EAX) , %ESI
    MOVL    XEDI(%EAX) , %EDI

    MOVL    XEBP(%EAX) , %EBP
    MOVL    XESP(%EAX) , %ESP

    PUSHL   XEFLAGS(%EAX)
    PUSHL   XCS(%EAX)
    PUSHL   XEIP(%EAX)

    MOVL    XEAX(%EAX) , %EAX

    IRET                                                                /*  弹出 EIP CS EFLAGS          */
    MACRO_END()

/*********************************************************************************************************
  恢复用户态任务寄存器(参数 %EAX: ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(USER_RESTORE_REGS)
    MOVW    XSS(%EAX) , %BX
    MOVW    %BX , %FS
    MOVW    %BX , %GS
    MOVW    %BX , %ES
    MOVW    %BX , %DS

    MOVL    XEBX(%EAX) , %EBX
    MOVL    XECX(%EAX) , %ECX
    MOVL    XEDX(%EAX) , %EDX

    MOVL    XESI(%EAX) , %ESI
    MOVL    XEDI(%EAX) , %EDI

    MOVL    XEBP(%EAX) , %EBP

    PUSHL   XSS(%EAX)
    PUSHL   XESP(%EAX)
    PUSHL   XEFLAGS(%EAX)
    PUSHL   XCS(%EAX)
    PUSHL   XEIP(%EAX)

    MOVL    XEAX(%EAX) , %EAX

    IRET                                                                /*  弹出 EIP CS EFLAGS ESP SS   */
    MACRO_END()

/*********************************************************************************************************
  异常/中断上下文保存
*********************************************************************************************************/

MACRO_DEF(INT_SAVE_REGS_HW_ERRNO)
    /*
     * RING0 :        EFLAGS CS EIP ERRNO 已经 PUSH
     * RING3 : SS ESP EFLAGS CS EIP ERRNO 已经 PUSH
     */
    CLI

    SUBL    $(8 * ARCH_REG_SIZE) , %ESP

    MOVL    %EAX , XEAX(%ESP)
    MOVL    %EBX , XEBX(%ESP)
    MOVL    %ECX , XECX(%ESP)
    MOVL    %EDX , XEDX(%ESP)

    MOVL    %ESI , XESI(%ESP)
    MOVL    %EDI , XEDI(%ESP)

    MOVL    %EBP , XEBP(%ESP)
    MACRO_END()

/*********************************************************************************************************
  异常/中断上下文保存
*********************************************************************************************************/

MACRO_DEF(INT_SAVE_REGS_FAKE_ERRNO)
    CLI

    PUSHL   $0                                                          /*  PUSH FAKE ERROR             */

    INT_SAVE_REGS_HW_ERRNO
    MACRO_END()

/*********************************************************************************************************
  恢复中断嵌套寄存器(参数 %ESP: ARCH_REG_CTX 地址)
*********************************************************************************************************/

MACRO_DEF(INT_NESTING_RESTORE_REGS)
    POPL    %EAX
    POPL    %EBX
    POPL    %ECX
    POPL    %EDX

    POPL    %ESI
    POPL    %EDI

    POPL    %EBP

    ADDL    $(2 * ARCH_REG_SIZE) , %ESP                                 /*  POP PAD ERROR               */

    IRET                                                                /*  弹出 EIP CS EFLAGS          */
    MACRO_END()

#endif                                                                  /*  __ARCH_X86CONTEXTASM_H      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
