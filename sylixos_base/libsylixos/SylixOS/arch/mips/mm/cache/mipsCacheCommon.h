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
** 文   件   名: mipsCacheCommon.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 07 月 18 日
**
** 描        述: MIPS 体系构架 CACHE 驱动.
*********************************************************************************************************/

#ifndef __ARCH_MIPSCACHECOMMON_H
#define __ARCH_MIPSCACHECOMMON_H

/*********************************************************************************************************
  CACHE 信息
*********************************************************************************************************/
typedef struct {
    BOOL        CACHE_bPresent;                                         /*  是否存在 CACHE              */
    UINT32      CACHE_uiSize;                                           /*  CACHE 大小                  */
    UINT32      CACHE_uiLineSize;                                       /*  CACHE 行大小                */
    UINT32      CACHE_uiSetNr;                                          /*  组数                        */
    UINT32      CACHE_uiWayNr;                                          /*  路数                        */
    UINT32      CACHE_uiWaySize;                                        /*  路大小                      */
    UINT32      CACHE_uiWayBit;                                         /*  索引选路偏移                */
} MIPS_CACHE;
/*********************************************************************************************************
  全局变量声明
*********************************************************************************************************/
extern MIPS_CACHE   _G_ICache, _G_DCache;                               /*  ICACHE 和 DCACHE 信息       */
extern MIPS_CACHE   _G_VCache, _G_SCache;                               /*  VCACHE 和 SCACHE 信息       */
extern BOOL         _G_bHaveHitWritebackS;                              /*  是否有 HitWritebackS 操作   */
extern BOOL         _G_bHaveHitWritebackD;                              /*  是否有 HitWritebackD 操作   */
extern BOOL         _G_bHaveFillI;                                      /*  是否有 FillI 操作           */
extern BOOL         _G_bHaveTagHi;                                      /*  是否有 TagHi 寄存器         */
extern BOOL         _G_bHaveECC;                                        /*  是否有 ECC 寄存器           */
extern UINT32       _G_uiEccValue;                                      /*  ECC 寄存器的值              */
/*********************************************************************************************************
  CACHE 特性
*********************************************************************************************************/
#define MIPS_CACHE_HAS_L2           _G_SCache.CACHE_bPresent            /*  是否有 L2CACHE              */
#define MIPS_CACHE_HAS_HIT_WB_S     _G_bHaveHitWritebackS               /*  是否有 HitWritebackS 操作   */
#define MIPS_CACHE_HAS_HIT_WB_D     _G_bHaveHitWritebackD               /*  是否有 HitWritebackD 操作   */
#define MIPS_CACHE_HAS_FILL_I       _G_bHaveFillI                       /*  是否有 FillI 操作           */
#define MIPS_CACHE_HAS_TAG_HI       _G_bHaveTagHi                       /*  是否有 TagHi 寄存器         */
#define MIPS_CACHE_HAS_ECC          _G_bHaveECC                         /*  是否有 ECC 寄存器           */
#define MIPS_CACHE_ECC_VALUE        _G_uiEccValue                       /*  ECC 寄存器的值              */
/*********************************************************************************************************
  CACHE 状态
*********************************************************************************************************/
#define L1_CACHE_I_EN       0x01
#define L1_CACHE_D_EN       0x02
#define L1_CACHE_EN         (L1_CACHE_I_EN | L1_CACHE_D_EN)
#define L1_CACHE_DIS        0x00
/*********************************************************************************************************
  CACHE 获得 pvAdrs 与 pvEnd 位置
*********************************************************************************************************/
#define MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, uiLineSize)              \
        do {                                                                \
            ulEnd  = (addr_t)((size_t)pvAdrs + stBytes);                    \
            pvAdrs = (PVOID)((addr_t)pvAdrs & ~((addr_t)uiLineSize - 1));   \
        } while (0)
/*********************************************************************************************************
  CACHE 回写管线
*********************************************************************************************************/
#define MIPS_PIPE_FLUSH()       KN_SYNC()
/*********************************************************************************************************
  This macro return a properly sign-extended address suitable as base address
  for indexed cache operations.  Two issues here:

   - The MIPS32 and MIPS64 specs permit an implementation to directly derive
     the index bits from the virtual address.  This breaks with tradition
     set by the R4000.  To keep unpleasant surprises from happening we pick
     an address in KSEG0 / CKSEG0.
   - We need a properly sign extended address for 64-bit code.  To get away
     without ifdefs we let the compiler do it by a type cast.
*********************************************************************************************************/
#define MIPS_CACHE_INDEX_BASE   CKSEG0

VOID  mipsCacheInfoShow(VOID);
VOID  mipsCacheProbe(CPCHAR  pcMachineName);

#endif                                                                  /*  __ARCH_MIPSCACHECOMMON_H    */
/*********************************************************************************************************
  END
*********************************************************************************************************/
