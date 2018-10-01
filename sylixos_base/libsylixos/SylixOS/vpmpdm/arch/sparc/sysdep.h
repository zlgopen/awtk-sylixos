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

#ifndef __SPARC_SYSDEP_H
#define __SPARC_SYSDEP_H

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

#define weak_alias(original, alias)                 \
  .weak C_SYMBOL_NAME (alias) ASM_LINE_SEP          \
  C_SYMBOL_NAME (alias) = C_SYMBOL_NAME (original)

#define weak_extern(symbol)                         \
  .weak C_SYMBOL_NAME (symbol)

        .text
        .balign 4

#endif /* __SPARC_SYSDEP_H */
