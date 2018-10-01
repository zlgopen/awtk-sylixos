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
** 文   件   名: mipsCp0.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 10 月 13 日
**
** 描        述: MIPS CP0 函数库.
*********************************************************************************************************/

#ifndef __ARCH_MIPSCP0_H
#define __ARCH_MIPSCP0_H

/*********************************************************************************************************
  CP0 寄存器操作
*********************************************************************************************************/

#define mipsCp0StatusRead       read_c0_status
#define mipsCp0StatusWrite      write_c0_status

#define mipsCp0CauseRead        read_c0_cause
#define mipsCp0CauseWrite       write_c0_cause

#define mipsCp0EPCRead          read_c0_epc
#define mipsCp0EPCWrite         write_c0_epc

#define mipsCp0ERRPCRead        read_c0_errorepc
#define mipsCp0ERRPCWrite       write_c0_errorepc

#define mipsCp0BadVAddrRead     read_c0_badvaddr
#define mipsCp0BadVAddrWrite    write_c0_badvaddr

#define mipsCp0CountRead        read_c0_count
#define mipsCp0CountWrite       write_c0_count

#define mipsCp0CompareRead      read_c0_compare
#define mipsCp0CompareWrite     write_c0_compare

#define mipsCp0PRIdRead         read_c0_prid

#define mipsCp0ConfigRead       read_c0_config
#define mipsCp0ConfigWrite      write_c0_config

#define mipsCp0Config1Read      read_c0_config1
#define mipsCp0Config1Write     write_c0_config1

#define mipsCp0Config2Read      read_c0_config2
#define mipsCp0Config2Write     write_c0_config2

#define mipsCp0Config3Read      read_c0_config3
#define mipsCp0Config3Write     write_c0_config3

#define mipsCp0Config4Read      read_c0_config4
#define mipsCp0Config4Write     write_c0_config4

#define mipsCp0Config5Read      read_c0_config5
#define mipsCp0Config5Write     write_c0_config5

#define mipsCp0Config6Read      read_c0_config6
#define mipsCp0Config6Write     write_c0_config6

#define mipsCp0Config7Read      read_c0_config7
#define mipsCp0Config7Write     write_c0_config7

#define mipsCp0EBaseRead        read_c0_ebase
#define mipsCp0EBase7Write      write_c0_ebase

#define mipsCp0IntCtlRead       read_c0_intctl
#define mipsCp0IntCtlWrite      write_c0_intctl

#define mipsCp0LLAddrRead       read_c0_lladdr
#define mipsCp0LLAddrWrite      write_c0_lladdr

#define mipsCp0ECCRead          read_c0_ecc
#define mipsCp0ECCWrite         write_c0_ecc

#define mipsCp0CacheErrRead     read_c0_cacheerr
#define mipsCp0CacheErrWrite    write_c0_cacheerr

#define mipsCp0IndexRead        read_c0_index
#define mipsCp0IndexWrite       write_c0_index

#define mipsCp0WiredRead        read_c0_wired
#define mipsCp0WiredWrite       write_c0_wired

#define mipsCp0RandomRead       read_c0_random
#define mipsCp0RandomWrite      write_c0_random

#define mipsCp0EntryLo0Read     read_c0_entrylo0
#define mipsCp0EntryLo0Write    write_c0_entrylo0

#define mipsCp0EntryLo1Read     read_c0_entrylo1
#define mipsCp0EntryLo1Write    write_c0_entrylo1

#define mipsCp0ContextRead      read_c0_context
#define mipsCp0ContextWrite     write_c0_context

#define mipsCp0XContextRead     read_c0_xcontext
#define mipsCp0XContextWrite    write_c0_xcontext

#define mipsCp0PageMaskRead     read_c0_pagemask
#define mipsCp0PageMaskWrite    write_c0_pagemask

#define mipsCp0EntryHiRead      read_c0_entryhi
#define mipsCp0EntryHiWrite     write_c0_entryhi

#define mipsCp0TagLoRead        read_c0_taglo
#define mipsCp0TagLoWrite       write_c0_taglo

#define mipsCp0TagHiRead        read_c0_taghi
#define mipsCp0TagHiWrite       write_c0_taghi

#define mipsCp0DiagRead         read_c0_diag
#define mipsCp0DiagWrite        write_c0_diag

#define mipsCp0PageGrainRead    read_c0_pagegrain
#define mipsCp0PageGrainWrite   write_c0_pagegrain

#define mipsCp0EBaseRead        read_c0_ebase
#define mipsCp0EBaseWrite       write_c0_ebase

/*********************************************************************************************************
  Loongson-1x/2x/3x CP0 寄存器操作
*********************************************************************************************************/

#define mipsCp0GSConfigRead     read_c0_config6
#define mipsCp0GSConfigWrite    write_c0_config6

#endif                                                                  /*  __ARCH_MIPSCP0_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
