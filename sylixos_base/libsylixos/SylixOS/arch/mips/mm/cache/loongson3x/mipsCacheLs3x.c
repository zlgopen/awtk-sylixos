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
** 文   件   名: mipsCacheLs3x.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 11 月 02 日
**
** 描        述: Loongson-3x 体系构架 CACHE 驱动.
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
extern VOID  mipsCacheR4kEnableHw(VOID);
/*********************************************************************************************************
  L1 CACHE 状态
*********************************************************************************************************/
static INT  _G_iCacheStatus = L1_CACHE_DIS;
/*********************************************************************************************************
** 函数名称: ls3xCacheEnableHw
** 功能描述: 使能 CACHE
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ls3xCacheEnableHw (VOID)
{
    mipsCacheR4kEnableHw();
}
/*********************************************************************************************************
** 函数名称: ls3xBranchPredictionDisable
** 功能描述: 禁能分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ls3xBranchPredictionDisable (VOID)
{
    UINT32  uiDiag = mipsCp0DiagRead();

    uiDiag |= 1 << 0;                                                   /*  置 1 时禁用 RAS             */
    mipsCp0DiagWrite(uiDiag);
}
/*********************************************************************************************************
** 函数名称: ls3xBranchPredictionEnable
** 功能描述: 使能分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ls3xBranchPredictionEnable (VOID)
{
    UINT32  uiDiag = mipsCp0DiagRead();

    uiDiag &= ~(1 << 0);
    mipsCp0DiagWrite(uiDiag);
}
/*********************************************************************************************************
** 函数名称: ls3xBranchPredictorInvalidate
** 功能描述: 无效分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ls3xBranchPredictorInvalidate (VOID)
{
    UINT32  uiDiag = mipsCp0DiagRead();

    uiDiag |= 1 << 1;                                                   /*  写入 1 清空 BRBTB 和 BTAC   */
    mipsCp0DiagWrite(uiDiag);
}
/*********************************************************************************************************
** 函数名称: ls3aR1CacheFlushAll
** 功能描述: Loongson-3A R1 回写所有 CACHE
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ls3aR1CacheFlushAll (VOID)
{
    REGISTER PVOID  pvAddr;

    __asm__ __volatile__(
        "   .set push                     \n"
        "   .set noreorder                \n"
        "   li %[addr], 0x80000000        \n"                           /*  KSEG0                       */
        "1: cache 0, 0(%[addr])           \n"                           /*  Flush L1 ICACHE             */
        "   cache 0, 1(%[addr])           \n"
        "   cache 0, 2(%[addr])           \n"
        "   cache 0, 3(%[addr])           \n"
        "   cache 1, 0(%[addr])           \n"                           /*  Flush L1 DCACHE             */
        "   cache 1, 1(%[addr])           \n"
        "   cache 1, 2(%[addr])           \n"
        "   cache 1, 3(%[addr])           \n"
        "   addiu %[sets], %[sets], -1    \n"
        "   bnez  %[sets], 1b             \n"
        "   addiu %[addr], %[addr], 0x20  \n"
        "   sync                          \n"
        "   .set pop                      \n"
        : [addr] "=&r" (pvAddr)
        : [sets] "r" (_G_DCache.CACHE_uiSetNr));
}
/*********************************************************************************************************
** 函数名称: ls3aR2CacheFlushAll
** 功能描述: Loongson-3A R2 回写所有 CACHE
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ls3aR2CacheFlushAll (VOID)
{
    REGISTER PVOID  pvAddr;

    __asm__ __volatile__(
        "   .set push                     \n"
        "   .set noreorder                \n"
        "   li %[addr], 0x80000000        \n"                           /*  KSEG0                       */
        "1: cache 0, 0(%[addr])           \n"                           /*  Flush L1 ICACHE             */
        "   cache 0, 1(%[addr])           \n"
        "   cache 0, 2(%[addr])           \n"
        "   cache 0, 3(%[addr])           \n"
        "   cache 1, 0(%[addr])           \n"                           /*  Flush L1 DCACHE             */
        "   cache 1, 1(%[addr])           \n"
        "   cache 1, 2(%[addr])           \n"
        "   cache 1, 3(%[addr])           \n"
        "   addiu %[sets], %[sets], -1    \n"
        "   bnez  %[sets], 1b             \n"
        "   addiu %[addr], %[addr], 0x40  \n"
        "   li %[addr], 0x80000000        \n"                           /*  KSEG0                       */
        "2: cache 2, 0(%[addr])           \n"                           /*  Flush L1 VCACHE             */
        "   cache 2, 1(%[addr])           \n"
        "   cache 2, 2(%[addr])           \n"
        "   cache 2, 3(%[addr])           \n"
        "   cache 2, 4(%[addr])           \n"
        "   cache 2, 5(%[addr])           \n"
        "   cache 2, 6(%[addr])           \n"
        "   cache 2, 7(%[addr])           \n"
        "   cache 2, 8(%[addr])           \n"
        "   cache 2, 9(%[addr])           \n"
        "   cache 2, 10(%[addr])          \n"
        "   cache 2, 11(%[addr])          \n"
        "   cache 2, 12(%[addr])          \n"
        "   cache 2, 13(%[addr])          \n"
        "   cache 2, 14(%[addr])          \n"
        "   cache 2, 15(%[addr])          \n"
        "   addiu %[vsets], %[vsets], -1  \n"
        "   bnez  %[vsets], 2b            \n"
        "   addiu %[addr], %[addr], 0x40  \n"
        "   sync                          \n"
        "   .set pop                      \n"
        : [addr] "=&r" (pvAddr)
        : [sets] "r" (_G_DCache.CACHE_uiSetNr),
          [vsets] "r" (_G_VCache.CACHE_uiSetNr));
}
/*********************************************************************************************************
** 函数名称: ls3bCacheFlushAll
** 功能描述: Loongson-3B 回写所有 CACHE
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ls3bCacheFlushAll (VOID)
{
    REGISTER PVOID  pvAddr;

    __asm__ __volatile__(
        "   .set push                     \n"
        "   .set noreorder                \n"
        "   li %[addr], 0x80000000        \n"                           /*  KSEG0                       */
        "1: cache 0, 0(%[addr])           \n"                           /*  Flush L1 ICACHE             */
        "   cache 0, 1(%[addr])           \n"
        "   cache 0, 2(%[addr])           \n"
        "   cache 0, 3(%[addr])           \n"
        "   cache 1, 0(%[addr])           \n"                           /*  Flush L1 DCACHE             */
        "   cache 1, 1(%[addr])           \n"
        "   cache 1, 2(%[addr])           \n"
        "   cache 1, 3(%[addr])           \n"
        "   addiu %[sets], %[sets], -1    \n"
        "   bnez  %[sets], 1b             \n"
        "   addiu %[addr], %[addr], 0x20  \n"
        "   sync                          \n"
        "   .set pop                      \n"
        : [addr] "=&r" (pvAddr)
        : [sets] "r" (_G_DCache.CACHE_uiSetNr));
}
/*********************************************************************************************************
** 函数名称: ls3xCacheFlushAll
** 功能描述: 回写所有 CACHE
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ls3xCacheFlushAll (VOID)
{
    mipsCpuProbe(MIPS_MACHINE_LS3X);                                    /*  MIPS CPU 探测               */
    mipsCacheProbe(MIPS_MACHINE_LS3X);                                  /*  CACHE 探测                  */

    switch (_G_uiMipsPridRev) {

    case PRID_REV_LOONGSON3A_R2:
    case PRID_REV_LOONGSON3A_R3_0:
    case PRID_REV_LOONGSON3A_R3_1:
        ls3aR2CacheFlushAll();
        break;

    case PRID_REV_LOONGSON3B_R1:
    case PRID_REV_LOONGSON3B_R2:
        ls3bCacheFlushAll();
        break;

    case PRID_REV_LOONGSON3A_R1:                                        /*  use default case            */
    case PRID_REV_LOONGSON2K_R1:
    case PRID_REV_LOONGSON2K_R2:
    default:
        ls3aR1CacheFlushAll();
        break;
    }
}
/*********************************************************************************************************
** 函数名称: ls3xCacheEnable
** 功能描述: 使能 CACHE
** 输　入  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ls3xCacheEnable (LW_CACHE_TYPE  cachetype)
{
    if (cachetype == INSTRUCTION_CACHE) {
        if (LW_CPU_GET_CUR_ID() == 0) {
            _G_iCacheStatus |= L1_CACHE_I_EN;
        }
    } else {
        if (LW_CPU_GET_CUR_ID() == 0) {
            _G_iCacheStatus |= L1_CACHE_D_EN;
        }
    }

    if (_G_iCacheStatus == L1_CACHE_EN) {
        ls3xCacheEnableHw();
        ls3xBranchPredictionEnable();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheDisable
** 功能描述: 禁能 CACHE
** 输　入  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ls3xCacheDisable (LW_CACHE_TYPE  cachetype)
{
    if (cachetype == INSTRUCTION_CACHE) {
        if (LW_CPU_GET_CUR_ID() == 0) {
            _G_iCacheStatus &= ~L1_CACHE_I_EN;
        }
    } else {
        if (LW_CPU_GET_CUR_ID() == 0) {
            _G_iCacheStatus &= ~L1_CACHE_D_EN;
        }
    }

    if (_G_iCacheStatus == L1_CACHE_DIS) {
        /*
         * 不能关闭 CACHE
         */
        ls3xBranchPredictionDisable();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheFlushNone
** 功能描述: CACHE 脏数据回写 (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ls3xCacheFlushNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheFlushPageNone
** 功能描述: CACHE 脏数据回写 (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ls3xCacheFlushPageNone (LW_CACHE_TYPE  cachetype,
                                    PVOID          pvAdrs,
                                    PVOID          pvPdrs,
                                    size_t         stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheInvalidateNone
** 功能描述: 指定类型的 CACHE 使部分无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ls3xCacheInvalidateNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheInvalidatePageNone
** 功能描述: 指定类型的 CACHE 使部分无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  ls3xCacheInvalidatePageNone (LW_CACHE_TYPE  cachetype,
                                         PVOID          pvAdrs,
                                         PVOID          pvPdrs,
                                         size_t         stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheClearNone
** 功能描述: 指定类型的 CACHE 使部分或全部清空(回写内存)并无效(访问不命中) (伪函数)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  ls3xCacheClearNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheClearPageNone
** 功能描述: 指定类型的 CACHE 使部分或全部清空(回写内存)并无效(访问不命中)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  ls3xCacheClearPageNone (LW_CACHE_TYPE  cachetype,
                                    PVOID          pvAdrs,
                                    PVOID          pvPdrs,
                                    size_t         stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheLock
** 功能描述: 锁定指定类型的 CACHE 
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  ls3xCacheLockNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheUnlock
** 功能描述: 解锁指定类型的 CACHE 
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  ls3xCacheUnlockNone (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheTextUpdate
** 功能描述: 清空(回写内存) D CACHE 无效(访问不命中) I CACHE
** 输　入  : pvAdrs                        虚拟地址
**           stBytes                       长度
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : L2 cache 为统一 CACHE 所以 text update 不需要操作 L2 cache.
*********************************************************************************************************/
static INT  ls3xCacheTextUpdateNone (PVOID  pvAdrs, size_t  stBytes)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ls3xCacheDataUpdate
** 功能描述: 回写 D CACHE (仅回写 CPU 独享级 CACHE)
** 输　入  : pvAdrs                        虚拟地址
**           stBytes                       长度
**           bInv                          是否为回写无效
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
** 注  意  : L2 cache 为统一 CACHE 所以 data update 不需要操作 L2 cache.
*********************************************************************************************************/
static INT  ls3xCacheDataUpdateNone (PVOID  pvAdrs, size_t  stBytes, BOOL  bInv)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheLs3xInit
** 功能描述: 初始化 CACHE
** 输　入  : pcacheop       CACHE 操作函数集
**           uiInstruction  指令 CACHE 参数
**           uiData         数据 CACHE 参数
**           pcMachineName  机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  mipsCacheLs3xInit (LW_CACHE_OP  *pcacheop,
                         CACHE_MODE    uiInstruction,
                         CACHE_MODE    uiData,
                         CPCHAR        pcMachineName)
{
    mipsCacheProbe(pcMachineName);                                      /*  CACHE 探测                  */
    mipsCacheInfoShow();                                                /*  打印 CACHE 信息             */
    /*
     * 不能关闭 CACHE
     */
    ls3xBranchPredictorInvalidate();                                    /*  无效分支预测                */

    pcacheop->CACHEOP_ulOption = 0ul;                                   /*  无须 TEXT_UPDATE_MP 选项    */

    pcacheop->CACHEOP_iILoc = CACHE_LOCATION_VIPT;                      /*  VIPT                        */
    pcacheop->CACHEOP_iDLoc = CACHE_LOCATION_VIPT;

    pcacheop->CACHEOP_iICacheLine = _G_ICache.CACHE_uiLineSize;
    pcacheop->CACHEOP_iDCacheLine = _G_DCache.CACHE_uiLineSize;

    pcacheop->CACHEOP_iICacheWaySize = _G_ICache.CACHE_uiWaySize;
    pcacheop->CACHEOP_iDCacheWaySize = _G_DCache.CACHE_uiWaySize;

    pcacheop->CACHEOP_pfuncEnable  = ls3xCacheEnable;
    pcacheop->CACHEOP_pfuncDisable = ls3xCacheDisable;
    /*
     * Loongson-3x 实现了各级 CACHE 的硬件一致性，所有 CACHE 函数都是 NONE 函数
     */
    pcacheop->CACHEOP_pfuncFlush          = ls3xCacheFlushNone;
    pcacheop->CACHEOP_pfuncFlushPage      = ls3xCacheFlushPageNone;
    pcacheop->CACHEOP_pfuncInvalidate     = ls3xCacheInvalidateNone;
    pcacheop->CACHEOP_pfuncInvalidatePage = ls3xCacheInvalidatePageNone;
    pcacheop->CACHEOP_pfuncClear          = ls3xCacheClearNone;
    pcacheop->CACHEOP_pfuncClearPage      = ls3xCacheClearPageNone;
    pcacheop->CACHEOP_pfuncTextUpdate     = ls3xCacheTextUpdateNone;
    pcacheop->CACHEOP_pfuncDataUpdate     = ls3xCacheDataUpdateNone;
    
    pcacheop->CACHEOP_pfuncLock           = ls3xCacheLockNone;          /*  暂时不支持锁定操作          */
    pcacheop->CACHEOP_pfuncUnlock         = ls3xCacheUnlockNone;
    
#if LW_CFG_VMM_EN > 0
    pcacheop->CACHEOP_pfuncDmaMalloc      = API_VmmDmaAlloc;
    pcacheop->CACHEOP_pfuncDmaMallocAlign = API_VmmDmaAllocAlign;
    pcacheop->CACHEOP_pfuncDmaFree        = API_VmmDmaFree;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
** 函数名称: mipsCacheLs3xReset
** 功能描述: 复位 CACHE 
** 输　入  : pcMachineName  机器名称
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  mipsCacheLs3xReset (CPCHAR  pcMachineName)
{
    mipsCacheProbe(pcMachineName);                                      /*  CACHE 探测                  */
    /*
     * 不能关闭 CACHE
     */
    ls3xBranchPredictorInvalidate();                                    /*  无效分支预测                */
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
