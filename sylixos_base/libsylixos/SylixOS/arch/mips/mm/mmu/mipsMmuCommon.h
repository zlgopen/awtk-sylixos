/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: mipsMmuCommon.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 12 月 25 日
**
** 描        述: MIPS 体系构架 MMU 通用接口.
*********************************************************************************************************/

#ifndef __ARCH_MIPSMMUCOMMON_H
#define __ARCH_MIPSMMUCOMMON_H

/*********************************************************************************************************
  全局变量声明
*********************************************************************************************************/
extern BOOL                 _G_bMmuHasXI;                               /*  是否有 XI 位                */
extern UINT32               _G_uiMmuTlbSize;                            /*  TLB 数组大小                */
extern UINT32               _G_uiMmuEntryLoUnCache;                     /*  非高速缓存                  */
extern UINT32               _G_uiMmuEntryLoCache;                       /*  一致性高速缓存              */
/*********************************************************************************************************
  MMU 特性
*********************************************************************************************************/
#define MIPS_MMU_HAS_XI                 _G_bMmuHasXI                    /*  是否有 XI 位                */
#define MIPS_MMU_TLB_SIZE               _G_uiMmuTlbSize                 /*  TLB 数组大小                */
#define MIPS_MMU_ENTRYLO_UNCACHE        _G_uiMmuEntryLoUnCache          /*  非高速缓存                  */
#define MIPS_MMU_ENTRYLO_CACHE          _G_uiMmuEntryLoCache            /*  一致性高速缓存              */
/*********************************************************************************************************
  PAGE 掩码
*********************************************************************************************************/
#if   LW_CFG_VMM_PAGE_SIZE == (4  * LW_CFG_KB_SIZE)
#define MIPS_MMU_PAGE_MASK              PM_4K
#elif LW_CFG_VMM_PAGE_SIZE == (16 * LW_CFG_KB_SIZE)
#define MIPS_MMU_PAGE_MASK              PM_16K
#elif LW_CFG_VMM_PAGE_SIZE == (64 * LW_CFG_KB_SIZE)
#define MIPS_MMU_PAGE_MASK              PM_64K
#else
#error  LW_CFG_VMM_PAGE_SIZE must be (4K, 16K, 64K)!
#endif                                                                  /*  LW_CFG_VMM_PAGE_SIZE        */
/*********************************************************************************************************
  TLB 操作
*********************************************************************************************************/
#define MIPS_MMU_TLB_WRITE_INDEX()      tlb_write_indexed()             /*  写指定索引 TLB              */
#define MIPS_MMU_TLB_WRITE_RANDOM()     tlb_write_random()              /*  写随机 TLB                  */
#define MIPS_MMU_TLB_READ()             tlb_read()                      /*  读 TLB                      */
#define MIPS_MMU_TLB_PROBE()            tlb_probe()                     /*  探测 TLB                    */

VOID   mipsMmuInit(LW_MMU_OP  *pmmuop, CPCHAR  pcMachineName);
ULONG  mipsMmuTlbLoadStoreExcHandle(addr_t  ulAbortAddr, BOOL  bStore);

#endif                                                                  /*  __ARCH_MIPSMMUCOMMON_H      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
