;/*********************************************************************************************************
;**
;**                                    中国软件开源组织
;**
;**                                   嵌入式实时操作系统
;**
;**                                       SylixOS(TM)
;**
;**                               Copyright  All Rights Reserved
;**
;**--------------文件信息--------------------------------------------------------------------------------
;**
;** 文   件   名: mipsCacheR4kAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2015 年 12 月 01 日
;**
;** 描        述: MIPS R4K 体系构架 CACHE 驱动.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/cache_cfg.h>
#include <config/cpu/cpu_cfg_mips.h>

#if LW_CFG_CACHE_EN > 0

    EXPORT_LABEL(mipsCacheR4kDisableHw)
    EXPORT_LABEL(mipsCacheR4kEnableHw)

    EXPORT_LABEL(mipsDCacheR4kLineFlush)
    EXPORT_LABEL(mipsDCacheR4kLineClear)
    EXPORT_LABEL(mipsDCacheR4kLineInvalidate)
    EXPORT_LABEL(mipsDCacheR4kIndexClear)
    EXPORT_LABEL(mipsDCacheR4kIndexStoreTag)

    EXPORT_LABEL(mipsICacheR4kLineInvalidate)
    EXPORT_LABEL(mipsICacheR4kIndexInvalidate)
    EXPORT_LABEL(mipsICacheR4kFill)
    EXPORT_LABEL(mipsICacheR4kIndexStoreTag)

    IMPORT_LABEL(mipsICacheR4kInvalidateAll)
    IMPORT_LABEL(mipsDCacheR4kClearAll)
#if LW_CFG_MIPS_CACHE_L2 > 0
    IMPORT_LABEL(mipsL2R4kClearAll)
#endif

    FILE_BEGIN()

;/*********************************************************************************************************
;  禁能 CACHE
;*********************************************************************************************************/

FUNC_DEF(mipsCacheR4kDisableHw)
    MFC0_EHB(T0, CP0_CONFIG)
    ANDI        T0 , T0 , CONF_CM_CMASK
    LI          T1 , CONF_CM_UNCACHED
    BEQ         T0 , T1 , 1f
    NOP

    REG_SUBU    SP , ARCH_STK_VAR_SIZE
    REG_S       RA , ARCH_STK_OFF_VAR(0)(SP)
    REG_S       FP , ARCH_STK_OFF_VAR(1)(SP)
    MOV         FP , SP

    JAL         mipsDCacheR4kClearAll
    NOP

    JAL         mipsICacheR4kInvalidateAll
    NOP

#if LW_CFG_MIPS_CACHE_L2 > 0
    JAL         mipsL2R4kClearAll
    NOP
#endif

    MOV         SP , FP
    REG_L       RA , ARCH_STK_OFF_VAR(0)(SP)
    REG_L       FP , ARCH_STK_OFF_VAR(1)(SP)
    REG_ADDU    SP , ARCH_STK_VAR_SIZE

    MFC0_EHB(T0, CP0_CONFIG)
    AND         T0 , T0 , ~CONF_CM_CMASK
    ORI         T0 , T0 , CONF_CM_UNCACHED
    MTC0_EHB(T0, CP0_CONFIG)

1:
    JR          RA
    NOP
    FUNC_END(mipsCacheR4kDisableHw)

;/*********************************************************************************************************
;  使能 CACHE
;*********************************************************************************************************/

FUNC_DEF(mipsCacheR4kEnableHw)
    MFC0_EHB(T0, CP0_CONFIG)
    ANDI        T0 , T0 , CONF_CM_CMASK
    LI          T1 , CONF_CM_CACHABLE_NONCOHERENT
    BEQ         T0 , T1 , 1f
    NOP

    MFC0_EHB(T0, CP0_CONFIG)
    AND         T0 , T0 , ~CONF_CM_CMASK
    ORI         T0 , CONF_CM_CACHABLE_NONCOHERENT
    MTC0_EHB(T0, CP0_CONFIG)

1:
    JR          RA
    NOP
    FUNC_END(mipsCacheR4kEnableHw)

;/*********************************************************************************************************
;  回写一个 DCACHE 行
;*********************************************************************************************************/

FUNC_DEF(mipsDCacheR4kLineFlush)
    CACHE       Hit_Writeback_D , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsDCacheR4kLineFlush)

;/*********************************************************************************************************
;  回写并无效一个 DCACHE 行
;*********************************************************************************************************/

FUNC_DEF(mipsDCacheR4kLineClear)
    CACHE       Hit_Writeback_Inv_D , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsDCacheR4kLineClear)

;/*********************************************************************************************************
;  无效一个 DCACHE 行
;*********************************************************************************************************/

FUNC_DEF(mipsDCacheR4kLineInvalidate)
    CACHE       Hit_Invalidate_D , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsDCacheR4kLineInvalidate)

;/*********************************************************************************************************
;  通过 CACHE 存储器的地址索引回写并无效一个 DCACHE 行
;*********************************************************************************************************/

FUNC_DEF(mipsDCacheR4kIndexClear)
    CACHE       Index_Writeback_Inv_D , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsDCacheR4kIndexClear)

;/*********************************************************************************************************
;  无效一个 ICACHE 行
;*********************************************************************************************************/

FUNC_DEF(mipsICacheR4kLineInvalidate)
    CACHE       Hit_Invalidate_I , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsICacheR4kLineInvalidate)

;/*********************************************************************************************************
;  通过 CACHE 存储器的地址索引回写并无效一个 ICACHE 行
;*********************************************************************************************************/

FUNC_DEF(mipsICacheR4kIndexInvalidate)
    CACHE       Index_Invalidate_I , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsICacheR4kIndexInvalidate)

;/*********************************************************************************************************
;  FILL ICACHE
;*********************************************************************************************************/

FUNC_DEF(mipsICacheR4kFill)
    CACHE       Fill , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsICacheR4kFill)

;/*********************************************************************************************************
;  设置 DCACHE 的 TAG
;*********************************************************************************************************/

FUNC_DEF(mipsDCacheR4kIndexStoreTag)
    CACHE       Index_Store_Tag_D , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsDCacheR4kIndexStoreTag)

;/*********************************************************************************************************
;  设置 ICACHE 的 TAG
;*********************************************************************************************************/

FUNC_DEF(mipsICacheR4kIndexStoreTag)
    CACHE       Index_Store_Tag_I , 0(A0)
    JR          RA
    NOP
    FUNC_END(mipsICacheR4kIndexStoreTag)

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
