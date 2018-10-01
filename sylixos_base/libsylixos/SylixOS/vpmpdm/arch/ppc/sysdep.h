/* Copyright (C) 1999-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/*
 * sysdep.h
 *
 *  Created on: May 9, 2018
 *      Author: Jiao.JinXing
 */

#ifndef __PPC_SYSDEP_H
#define __PPC_SYSDEP_H

#define C_SYMBOL_NAME(name) name

#define ASM_LINE_SEP        ;

#define C_LABEL(name)       name:

#define L(x)                .L##x

/* This seems to always be the case on PPC.  */
#define ALIGNARG(log2)      log2

#define ASM_SIZE_DIRECTIVE(name) .size name,.-name

#define cfi_startproc
#define cfi_endproc

#define CALL_MCOUNT

#define libc_hidden_builtin_def(name)
#define libc_hidden_def(name)

#define EALIGN_W_0  /* No words to insert.  */
#define EALIGN_W_1  nop
#define EALIGN_W_2  nop;nop
#define EALIGN_W_3  nop;nop;nop
#define EALIGN_W_4  EALIGN_W_3;nop
#define EALIGN_W_5  EALIGN_W_4;nop
#define EALIGN_W_6  EALIGN_W_5;nop
#define EALIGN_W_7  EALIGN_W_6;nop

#define ENTRY(name)                                 \
  .globl C_SYMBOL_NAME(name);                       \
  .type C_SYMBOL_NAME(name),@function;              \
  .align ALIGNARG(2);                               \
  C_LABEL(name)                                     \
  cfi_startproc;                                    \
  CALL_MCOUNT

#define END(name)                                   \
  cfi_endproc;                                      \
  ASM_SIZE_DIRECTIVE(name)

# define EALIGN(name, alignt, words)                \
  .globl C_SYMBOL_NAME(name);                       \
  .type C_SYMBOL_NAME(name),@function;              \
  .align ALIGNARG(alignt);                          \
  EALIGN_W_##words;                                 \
  C_LABEL(name)                                     \
  cfi_startproc;

#define weak_alias(original, alias)                 \
  .weak C_SYMBOL_NAME (alias) ASM_LINE_SEP          \
  C_SYMBOL_NAME (alias) = C_SYMBOL_NAME (original)

#define weak_extern(symbol)                         \
  .weak C_SYMBOL_NAME (symbol)

/* Integer registers.  */
#define r0  0
#define r1  1
#define r2  2
#define r3  3
#define r4  4
#define r5  5
#define r6  6
#define r7  7
#define r8  8
#define r9  9
#define r10 10
#define r11 11
#define r12 12
#define r13 13
#define r14 14
#define r15 15
#define r16 16
#define r17 17
#define r18 18
#define r19 19
#define r20 20
#define r21 21
#define r22 22
#define r23 23
#define r24 24
#define r25 25
#define r26 26
#define r27 27
#define r28 28
#define r29 29
#define r30 30
#define r31 31

        .text
        .balign 4

#endif /* __PPC_SYSDEP_H */
