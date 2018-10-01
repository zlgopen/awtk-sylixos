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
** 文   件   名: riscvCache.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 04 月 12 日
**
** 描        述: RISC-V 体系构架 CACHE 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0
#include "riscvCache.h"
#include "arch/riscv/inc/sbi.h"
/*********************************************************************************************************
** 函数名称: riscvLocalICacheFlushAll
** 功能描述: 无效本地所有 I-CACHE
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE  VOID  riscvLocalICacheFlushAll (VOID)
{
#if LW_CFG_RISCV_M_LEVEL > 0
    __asm__ __volatile__ ("fence.i" : : : "memory");
#else
    sbi_remote_fence_i(0);
#endif                                                                  /*  LW_CFG_RISCV_M_LEVEL > 0    */
}
/*********************************************************************************************************
** 函数名称: riscvCacheEnable
** 功能描述: 使能 CACHE
** 输　入  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  riscvCacheEnable (LW_CACHE_TYPE  cachetype)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheDisable
** 功能描述: 禁能 CACHE (伪函数)
** 输　入  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  riscvCacheDisable (LW_CACHE_TYPE  cachetype)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheFlush
** 功能描述: CACHE 脏数据回写 (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  riscvCacheFlush (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheFlushPage
** 功能描述: CACHE 脏数据回写 (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  riscvCacheFlushPage (LW_CACHE_TYPE  cachetype,
                                 PVOID          pvAdrs,
                                 PVOID          pvPdrs,
                                 size_t         stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheInvalidate
** 功能描述: 指定类型的 CACHE 使部分无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  riscvCacheInvalidate (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    if (cachetype == INSTRUCTION_CACHE) {
        riscvLocalICacheFlushAll();
    }
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheInvalidatePage
** 功能描述: 指定类型的 CACHE 使部分无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  riscvCacheInvalidatePage (LW_CACHE_TYPE  cachetype,
                                      PVOID          pvAdrs,
                                      PVOID          pvPdrs,
                                      size_t         stBytes)
{
    if (cachetype == INSTRUCTION_CACHE) {
        riscvLocalICacheFlushAll();
    }
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheClear
** 功能描述: 指定类型的 CACHE 使部分或全部清空(回写内存)并无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  riscvCacheClear (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    if (cachetype == INSTRUCTION_CACHE) {
        riscvLocalICacheFlushAll();
    }
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheClearPage
** 功能描述: 指定类型的 CACHE 使部分或全部清空(回写内存)并无效(访问不命中)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  riscvCacheClearPage (LW_CACHE_TYPE  cachetype,
                                 PVOID          pvAdrs,
                                 PVOID          pvPdrs,
                                 size_t         stBytes)
{
    if (cachetype == INSTRUCTION_CACHE) {
        riscvLocalICacheFlushAll();
    }
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheLock
** 功能描述: 锁定指定类型的 CACHE 
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT	riscvCacheLock (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: riscvCacheUnlock
** 功能描述: 解锁指定类型的 CACHE 
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT	riscvCacheUnlock (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: riscvCacheTextUpdate
** 功能描述: 清空(回写内存) D CACHE 无效(访问不命中) I CACHE
** 输　入  : pvAdrs                        虚拟地址
**           stBytes                       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : L2 cache 为统一 CACHE 所以 text update 不需要操作 L2 cache.
*********************************************************************************************************/
static INT	riscvCacheTextUpdate (PVOID  pvAdrs, size_t  stBytes)
{
    riscvLocalICacheFlushAll();
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheDataUpdate
** 功能描述: 回写 D CACHE (仅回写 CPU 独享级 CACHE)
** 输　入  : pvAdrs                        虚拟地址
**           stBytes                       长度
**           bInv                          是否为回写无效
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : L2 cache 为统一 CACHE 所以 data update 不需要操作 L2 cache.
*********************************************************************************************************/
static INT  riscvCacheDataUpdate (PVOID  pvAdrs, size_t  stBytes, BOOL  bInv)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: riscvCacheInit
** 功能描述: 初始化 CACHE 
** 输　入  : pcacheop       CACHE 操作函数集
**           uiInstruction  指令 CACHE 参数
**           uiData         数据 CACHE 参数
**           pcMachineName  机器名称
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  riscvCacheInit (LW_CACHE_OP  *pcacheop,
                      CACHE_MODE    uiInstruction,
                      CACHE_MODE    uiData,
                      CPCHAR        pcMachineName)
{
#if LW_CFG_RISCV_M_LEVEL > 0
    pcacheop->CACHEOP_ulOption = CACHE_TEXT_UPDATE_MP;
#else
    pcacheop->CACHEOP_ulOption = 0;
#endif                                                                  /*  LW_CFG_RISCV_M_LEVEL > 0    */

    pcacheop->CACHEOP_iILoc = CACHE_LOCATION_VIPT;                      /*  VIPT                        */
    pcacheop->CACHEOP_iDLoc = CACHE_LOCATION_VIPT;
    
    pcacheop->CACHEOP_iICacheLine = LW_CFG_CPU_ARCH_CACHE_LINE;
    pcacheop->CACHEOP_iDCacheLine = LW_CFG_CPU_ARCH_CACHE_LINE;

    pcacheop->CACHEOP_iICacheWaySize = LW_CFG_VMM_PAGE_SIZE;
    pcacheop->CACHEOP_iDCacheWaySize = LW_CFG_VMM_PAGE_SIZE;

    pcacheop->CACHEOP_pfuncEnable         = riscvCacheEnable;
    pcacheop->CACHEOP_pfuncDisable        = riscvCacheDisable;
    
    pcacheop->CACHEOP_pfuncFlush          = riscvCacheFlush;
    pcacheop->CACHEOP_pfuncFlushPage      = riscvCacheFlushPage;
    pcacheop->CACHEOP_pfuncInvalidate     = riscvCacheInvalidate;
    pcacheop->CACHEOP_pfuncInvalidatePage = riscvCacheInvalidatePage;
    pcacheop->CACHEOP_pfuncClear          = riscvCacheClear;
    pcacheop->CACHEOP_pfuncClearPage      = riscvCacheClearPage;
    pcacheop->CACHEOP_pfuncTextUpdate     = riscvCacheTextUpdate;
    pcacheop->CACHEOP_pfuncDataUpdate     = riscvCacheDataUpdate;
    
    pcacheop->CACHEOP_pfuncLock           = riscvCacheLock;             /*  暂时不支持锁定操作          */
    pcacheop->CACHEOP_pfuncUnlock         = riscvCacheUnlock;
    
#if LW_CFG_VMM_EN > 0
    pcacheop->CACHEOP_pfuncDmaMalloc      = API_VmmDmaAlloc;
    pcacheop->CACHEOP_pfuncDmaMallocAlign = API_VmmDmaAllocAlign;
    pcacheop->CACHEOP_pfuncDmaFree        = API_VmmDmaFree;

#elif LW_CFG_RISCV_MPU_EN > 0
    pcacheop->CACHEOP_pfuncDmaMalloc      = riscvMpuDmaAlloc;
    pcacheop->CACHEOP_pfuncDmaMallocAlign = riscvMpuDmaAllocAlign;
    pcacheop->CACHEOP_pfuncDmaFree        = riscvMpuDmaFree;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
** 函数名称: riscvCacheReset
** 功能描述: 复位 CACHE 
** 输　入  : pcMachineName  机器名称
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  riscvCacheReset (CPCHAR  pcMachineName)
{
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
