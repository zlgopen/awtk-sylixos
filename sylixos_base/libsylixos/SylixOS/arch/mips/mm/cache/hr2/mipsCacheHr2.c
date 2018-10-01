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
** 文   件   名: mipsCacheHr2.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 14 日
**
** 描        述: 华睿 2 号处理器 CACHE 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0
#include "arch/mips/common/cp0/mipsCp0.h"
#include "arch/mips/common/mipsCpuProbe.h"
#include "arch/mips/mm/cache/mipsCacheCommon.h"
/*********************************************************************************************************
  外部函数声明
*********************************************************************************************************/
extern VOID  hr2CacheEnableHw(VOID);
/*********************************************************************************************************
  L1 CACHE 状态
*********************************************************************************************************/
static INT  _G_iCacheStatus = L1_CACHE_DIS;
/*********************************************************************************************************
** 函数名称: hr2BranchPredictionDisable
** 功能描述: 禁能分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  hr2BranchPredictionDisable (VOID)
{
}
/*********************************************************************************************************
** 函数名称: hr2BranchPredictionEnable
** 功能描述: 使能分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  hr2BranchPredictionEnable (VOID)
{
}
/*********************************************************************************************************
** 函数名称: hr2BranchPredictorInvalidate
** 功能描述: 无效分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  hr2BranchPredictorInvalidate (VOID)
{
}
/*********************************************************************************************************
** 函数名称: hr2CacheFlushAll
** 功能描述: 回写所有 CACHE
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  hr2CacheFlushAll (VOID)
{
    /*
     * Not in opensource version
     */
}
/*********************************************************************************************************
** 函数名称: hr2CacheEnable
** 功能描述: 使能 CACHE
** 输　入  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  hr2CacheEnable (LW_CACHE_TYPE  cachetype)
{
    /*
     * Not in opensource version
     */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheDisable
** 功能描述: 禁能 CACHE
** 输　入  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  hr2CacheDisable (LW_CACHE_TYPE  cachetype)
{
    /*
     * Not in opensource version
     */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheFlushNone
** 功能描述: CACHE 脏数据回写 (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  hr2CacheFlushNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheFlushPageNone
** 功能描述: CACHE 脏数据回写 (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  hr2CacheFlushPageNone (LW_CACHE_TYPE  cachetype,
                                   PVOID          pvAdrs,
                                   PVOID          pvPdrs,
                                   size_t         stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheInvalidateNone
** 功能描述: 指定类型的 CACHE 使部分无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  hr2CacheInvalidateNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheInvalidatePageNone
** 功能描述: 指定类型的 CACHE 使部分无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  hr2CacheInvalidatePageNone (LW_CACHE_TYPE  cachetype,
                                        PVOID          pvAdrs,
                                        PVOID          pvPdrs,
                                        size_t         stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheClearNone
** 功能描述: 指定类型的 CACHE 使部分或全部清空(回写内存)并无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  hr2CacheClearNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheClearPageNone
** 功能描述: 指定类型的 CACHE 使部分或全部清空(回写内存)并无效(访问不命中)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  hr2CacheClearPageNone (LW_CACHE_TYPE  cachetype,
                                   PVOID          pvAdrs,
                                   PVOID          pvPdrs,
                                   size_t         stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheLock
** 功能描述: 锁定指定类型的 CACHE 
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  hr2CacheLockNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: hr2CacheUnlock
** 功能描述: 解锁指定类型的 CACHE 
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  hr2CacheUnlockNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: hr2CacheTextUpdate
** 功能描述: 清空(回写内存) D CACHE 无效(访问不命中) I CACHE
** 输　入  : pvAdrs                        虚拟地址
**           stBytes                       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : L2 cache 为统一 CACHE 所以 text update 不需要操作 L2 cache.
*********************************************************************************************************/
static INT  hr2CacheTextUpdateNone (PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: hr2CacheDataUpdate
** 功能描述: 回写 D CACHE (仅回写 CPU 独享级 CACHE)
** 输　入  : pvAdrs                        虚拟地址
**           stBytes                       长度
**           bInv                          是否为回写无效
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
** 注  意  : L2 cache 为统一 CACHE 所以 data update 不需要操作 L2 cache.
*********************************************************************************************************/
static INT  hr2CacheDataUpdateNone (PVOID  pvAdrs, size_t  stBytes, BOOL  bInv)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheHr2Init
** 功能描述: 初始化 CACHE
** 输　入  : pcacheop       CACHE 操作函数集
**           uiInstruction  指令 CACHE 参数
**           uiData         数据 CACHE 参数
**           pcMachineName  机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  mipsCacheHr2Init (LW_CACHE_OP  *pcacheop,
                        CACHE_MODE    uiInstruction,
                        CACHE_MODE    uiData,
                        CPCHAR        pcMachineName)
{
    mipsCacheProbe(pcMachineName);                                      /*  CACHE 探测                  */
    mipsCacheInfoShow();                                                /*  打印 CACHE 信息             */
    /*
     * 不能关闭 CACHE
     */
    hr2BranchPredictorInvalidate();                                     /*  无效分支预测                */

    pcacheop->CACHEOP_ulOption = 0ul;                                   /*  无须 TEXT_UPDATE_MP 选项    */

    pcacheop->CACHEOP_iILoc = CACHE_LOCATION_VIPT;                      /*  VIPT                        */
    pcacheop->CACHEOP_iDLoc = CACHE_LOCATION_VIPT;

    pcacheop->CACHEOP_iICacheLine = _G_ICache.CACHE_uiLineSize;
    pcacheop->CACHEOP_iDCacheLine = _G_DCache.CACHE_uiLineSize;

    pcacheop->CACHEOP_iICacheWaySize = _G_ICache.CACHE_uiWaySize;
    pcacheop->CACHEOP_iDCacheWaySize = _G_DCache.CACHE_uiWaySize;

    pcacheop->CACHEOP_pfuncEnable  = hr2CacheEnable;
    pcacheop->CACHEOP_pfuncDisable = hr2CacheDisable;
    /*
     * CETC-HR2 实现了各级 CACHE 的硬件一致性，所有 CACHE 函数都是 NONE 函数
     */
    pcacheop->CACHEOP_pfuncFlush          = hr2CacheFlushNone;
    pcacheop->CACHEOP_pfuncFlushPage      = hr2CacheFlushPageNone;
    pcacheop->CACHEOP_pfuncInvalidate     = hr2CacheInvalidateNone;
    pcacheop->CACHEOP_pfuncInvalidatePage = hr2CacheInvalidatePageNone;
    pcacheop->CACHEOP_pfuncClear          = hr2CacheClearNone;
    pcacheop->CACHEOP_pfuncClearPage      = hr2CacheClearPageNone;
    pcacheop->CACHEOP_pfuncTextUpdate     = hr2CacheTextUpdateNone;
    pcacheop->CACHEOP_pfuncDataUpdate     = hr2CacheDataUpdateNone;
    
    pcacheop->CACHEOP_pfuncLock           = hr2CacheLockNone;           /*  暂时不支持锁定操作          */
    pcacheop->CACHEOP_pfuncUnlock         = hr2CacheUnlockNone;
    
#if LW_CFG_VMM_EN > 0
    pcacheop->CACHEOP_pfuncDmaMalloc      = API_VmmDmaAlloc;
    pcacheop->CACHEOP_pfuncDmaMallocAlign = API_VmmDmaAllocAlign;
    pcacheop->CACHEOP_pfuncDmaFree        = API_VmmDmaFree;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
** 函数名称: mipsCacheHr2Reset
** 功能描述: 复位 CACHE 
** 输　入  : pcMachineName  机器名称
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  mipsCacheHr2Reset (CPCHAR  pcMachineName)
{
    mipsCacheProbe(pcMachineName);                                      /*  CACHE 探测                  */
    /*
     * 不能关闭 CACHE
     */
    hr2BranchPredictorInvalidate();                                     /*  无效分支预测                */
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
