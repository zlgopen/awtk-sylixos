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
** 文   件   名: mipsCacheR4k.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 12 月 01 日
**
** 描        述: MIPS R4K 体系构架 CACHE 驱动.
**
** BUG:
2016.04.06  Add Cache Init 对 CP0_ECC Register Init(Loongson-2H 支持)
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0
#include "arch/mips/common/cp0/mipsCp0.h"
#include "arch/mips/common/mipsCpuProbe.h"
#include "arch/mips/inc/addrspace.h"
#include "arch/mips/mm/cache/mipsCacheCommon.h"
#if LW_CFG_MIPS_CACHE_L2 > 0
#include "arch/mips/mm/cache/l2/mipsL2R4k.h"
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
/*********************************************************************************************************
  外部函数声明
*********************************************************************************************************/
extern VOID  mipsCacheR4kDisableHw(VOID);
extern VOID  mipsCacheR4kEnableHw(VOID);

extern VOID  mipsDCacheR4kLineFlush(PCHAR       pcAddr);
extern VOID  mipsDCacheR4kLineClear(PCHAR       pcAddr);
extern VOID  mipsDCacheR4kLineInvalidate(PCHAR  pcAddr);
extern VOID  mipsDCacheR4kIndexClear(PCHAR      pcAddr);
extern VOID  mipsDCacheR4kIndexStoreTag(PCHAR   pcAddr);

extern VOID  mipsICacheR4kLineInvalidate(PCHAR  pcAddr);
extern VOID  mipsICacheR4kIndexInvalidate(PCHAR pcAddr);
extern VOID  mipsICacheR4kFill(PCHAR            pcAddr);
extern VOID  mipsICacheR4kIndexStoreTag(PCHAR   pcAddr);
/*********************************************************************************************************
  CACHE 状态
*********************************************************************************************************/
static INT      _G_iCacheStatus = L1_CACHE_DIS;
/*********************************************************************************************************
  CACHE 循环操作时允许的最大大小, 大于该大小时将使用 All 操作
*********************************************************************************************************/
#define MIPS_CACHE_LOOP_OP_MAX_SIZE     (8 * LW_CFG_KB_SIZE)
/*********************************************************************************************************
  内部函数
*********************************************************************************************************/
/*********************************************************************************************************
  在龙芯 2F 处理器上需要特别小心地使用 Likely 类转移指令。尽管 Likely 类
  转移指令也许对顺序标量处理器的简单的静态调度很有效， 但是它对现代高性能
  处理器并不是同样有效。因为现代高性能处理器的转移预测硬件是比较复杂，它
  们通常有 90%以上的正确预测率。 （比如说，龙芯 2F 能够正确预测 85%-100%，
  平均 95%的条件转移的转移方向） 在这种情况下， 编译器不应该使用预测率不太
  高的 Likely 类转移指令。事实上，我们发现带有-mno-branch-likely 选项的 GCC
  （3.3 版）通常会工作得更好
*********************************************************************************************************/
/*********************************************************************************************************
** 函数名称: mipsBranchPredictionDisable
** 功能描述: 禁能分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
LW_WEAK VOID  mipsBranchPredictionDisable (VOID)
{
    if (_G_uiMipsCpuType == CPU_LOONGSON2) {
        UINT32  uiDiag = mipsCp0DiagRead();

        uiDiag |= 1 << 0;                                               /*  置 1 时禁用 RAS             */
        mipsCp0DiagWrite(uiDiag);

    } else if (_G_uiMipsCpuType == CPU_JZRISC) {
        UINT32  uiConfig7 = mipsCp0Config7Read();

        uiConfig7 &= ~(1 << 0);
        mipsCp0DiagWrite(uiConfig7);
    }
}
/*********************************************************************************************************
** 函数名称: mipsBranchPredictionEnable
** 功能描述: 使能分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
LW_WEAK VOID  mipsBranchPredictionEnable (VOID)
{
    if (_G_uiMipsCpuType == CPU_LOONGSON2) {
        UINT32  uiDiag = mipsCp0DiagRead();

        uiDiag &= ~(1 << 0);
        mipsCp0DiagWrite(uiDiag);

    } else if (_G_uiMipsCpuType == CPU_LOONGSON1) {
        UINT32  uiConfig6 = mipsCp0GSConfigRead();

        uiConfig6 &= ~(3 << 0);                                         /*  分支预测方式:Gshare 索引 BHT*/
        mipsCp0GSConfigWrite(uiConfig6);

    } else if (_G_uiMipsCpuType == CPU_JZRISC) {
        UINT32  uiConfig7 = mipsCp0Config7Read();

        uiConfig7 |= (1 << 0);
        mipsCp0DiagWrite(uiConfig7);
    }
}
/*********************************************************************************************************
** 函数名称: mipsBranchPredictorInvalidate
** 功能描述: 无效分支预测
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
LW_WEAK VOID  mipsBranchPredictorInvalidate (VOID)
{
    if (_G_uiMipsCpuType == CPU_LOONGSON2) {
        UINT32  uiDiag = mipsCp0DiagRead();

        uiDiag |= 1 << 1;                                               /*  写入 1 清空 BTB             */
        mipsCp0DiagWrite(uiDiag);

    } else if (_G_uiMipsCpuType == CPU_JZRISC) {
        UINT32  uiConfig7 = mipsCp0Config7Read();

        uiConfig7 |= (1 << 1);                                          /*  写入 1 清空 BTB             */
        mipsCp0DiagWrite(uiConfig7);
    }
}
/*********************************************************************************************************
** 函数名称: mipsDCacheR4kClear
** 功能描述: D-CACHE 脏数据回写并无效
** 输　入  : pvStart        开始地址
**           pvEnd          结束地址
**           uiStep         步进
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsDCacheR4kClear (PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep)
{
    REGISTER PCHAR   pcAddr;

    for (pcAddr = (PCHAR)pvStart; pcAddr < (PCHAR)pvEnd; pcAddr += uiStep) {
        mipsDCacheR4kLineClear(pcAddr);
    }
    MIPS_PIPE_FLUSH();
}
/*********************************************************************************************************
** 函数名称: mipsDCacheR4kFlush
** 功能描述: D-CACHE 脏数据回写
** 输　入  : pvStart        开始地址
**           pvEnd          结束地址
**           uiStep         步进
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsDCacheR4kFlush (PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep)
{
    REGISTER PCHAR   pcAddr;

    if (MIPS_CACHE_HAS_HIT_WB_D) {
        for (pcAddr = (PCHAR)pvStart; pcAddr < (PCHAR)pvEnd; pcAddr += uiStep) {
            mipsDCacheR4kLineFlush(pcAddr);
        }
        MIPS_PIPE_FLUSH();

    } else {
        mipsDCacheR4kClear(pvStart, pvEnd, uiStep);
    }
}
/*********************************************************************************************************
** 函数名称: mipsDCacheR4kInvalidate
** 功能描述: D-CACHE 脏数据无效
** 输　入  : pvStart        开始地址
**           pvEnd          结束地址
**           uiStep         步进
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsDCacheR4kInvalidate (PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep)
{
    REGISTER PCHAR   pcAddr;

    for (pcAddr = (PCHAR)pvStart; pcAddr < (PCHAR)pvEnd; pcAddr += uiStep) {
        mipsDCacheR4kLineInvalidate(pcAddr);
    }
    MIPS_PIPE_FLUSH();
}
/*********************************************************************************************************
** 函数名称: mipsICacheR4kInvalidate
** 功能描述: I-CACHE 脏数据无效
** 输　入  : pvStart       开始地址
**           pvEnd         结束地址
**           uiStep        步进
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsICacheR4kInvalidate (PVOID  pvStart, PVOID  pvEnd, UINT32  uiStep)
{
    REGISTER PCHAR   pcAddr;

    for (pcAddr = (PCHAR)pvStart; pcAddr < (PCHAR)pvEnd; pcAddr += uiStep) {
        mipsICacheR4kLineInvalidate(pcAddr);
    }
    MIPS_PIPE_FLUSH();
}
/*********************************************************************************************************
** 函数名称: mipsDCacheR4kClearAll
** 功能描述: D-CACHE 所有数据回写并无效
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  mipsDCacheR4kClearAll (VOID)
{
    REGISTER INT     iWay;
    REGISTER PCHAR   pcLineAddr;
    REGISTER PCHAR   pcBaseAddr = (PCHAR)MIPS_CACHE_INDEX_BASE;
    REGISTER PCHAR   pcEndAddr  = (PCHAR)(MIPS_CACHE_INDEX_BASE + _G_DCache.CACHE_uiWaySize);

    for (pcLineAddr = pcBaseAddr; pcLineAddr < pcEndAddr; pcLineAddr += _G_DCache.CACHE_uiLineSize) {
        for (iWay = 0; iWay < _G_DCache.CACHE_uiWayNr; iWay++) {
            mipsDCacheR4kIndexClear(pcLineAddr + (iWay << _G_DCache.CACHE_uiWayBit));
        }
    }
    MIPS_PIPE_FLUSH();
}
/*********************************************************************************************************
** 函数名称: mipsDCacheR4kFlushAll
** 功能描述: D-CACHE 所有数据回写
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsDCacheR4kFlushAll (VOID)
{
    mipsDCacheR4kClearAll();
}
/*********************************************************************************************************
** 函数名称: mipsICacheR4kInvalidateAll
** 功能描述: I-CACHE 所有数据无效
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID    mipsICacheR4kInvalidateAll (VOID)
{
    REGISTER INT     iWay;
    REGISTER PCHAR   pcLineAddr;
    REGISTER PCHAR   pcBaseAddr = (PCHAR)MIPS_CACHE_INDEX_BASE;
    REGISTER PCHAR   pcEndAddr  = (PCHAR)(MIPS_CACHE_INDEX_BASE + _G_ICache.CACHE_uiSize);

    for (pcLineAddr = pcBaseAddr; pcLineAddr < pcEndAddr; pcLineAddr += _G_ICache.CACHE_uiLineSize) {
        for (iWay = 0; iWay < _G_ICache.CACHE_uiWayNr; iWay++) {
            mipsICacheR4kIndexInvalidate(pcLineAddr + (iWay << _G_ICache.CACHE_uiWayBit));
        }
    }
    MIPS_PIPE_FLUSH();
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kEnable
** 功能描述: 使能 CACHE 
** 输　入  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  mipsCacheR4kEnable (LW_CACHE_TYPE  cachetype)
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
        mipsCacheR4kEnableHw();
        mipsBranchPredictionEnable();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kDisable
** 功能描述: 禁能 CACHE 
** 输　入  : cachetype      INSTRUCTION_CACHE / DATA_CACHE
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  mipsCacheR4kDisable (LW_CACHE_TYPE  cachetype)
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
        mipsCacheR4kDisableHw();
        mipsBranchPredictionDisable();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kFlush
** 功能描述: CACHE 脏数据回写
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  mipsCacheR4kFlush (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (stBytes == 0) {
        return  (ERROR_NONE);
    }

    if (cachetype == DATA_CACHE) {
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (MIPS_CACHE_HAS_L2) {
            /*
             * Loongson2F_UM_CN_V1.5 PAGE91
             * 二级 CACHE 与数据 CACHE 和指令 CACHE 保持包含关系, 所以回写二级 CACHE 即可, 下同
             *
             * TODO: 君正的 CPU 是否也这样?
             */
            return  (mipsL2R4kFlush(pvAdrs, stBytes));
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
        if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
            mipsDCacheR4kFlushAll();                                    /*  全部回写                    */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_DCache.CACHE_uiLineSize);
            mipsDCacheR4kFlush(pvAdrs, (PVOID)ulEnd,                    /*  部分回写                    */
                               _G_DCache.CACHE_uiLineSize);
        }
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kFlushPage
** 功能描述: CACHE 脏数据回写
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块:
*********************************************************************************************************/
static INT  mipsCacheR4kFlushPage (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, PVOID  pvPdrs, size_t  stBytes)
{
    addr_t  ulEnd;
    
    if (stBytes == 0) {
        return  (ERROR_NONE);
    }

    if (cachetype == DATA_CACHE) {
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (MIPS_CACHE_HAS_L2) {
            return  (mipsL2R4kFlush(pvAdrs, stBytes));
        }
#endif
        if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
            mipsDCacheR4kFlushAll();                                    /*  全部回写                    */
        
        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_DCache.CACHE_uiLineSize);
            mipsDCacheR4kFlush(pvAdrs, (PVOID)ulEnd,                    /*  部分回写                    */
                               _G_DCache.CACHE_uiLineSize);
        }
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kInvalidate
** 功能描述: 指定类型的 CACHE 使部分无效(访问不命中)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  mipsCacheR4kInvalidate (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (stBytes == 0) {
        return  (ERROR_NONE);
    }

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
            mipsICacheR4kInvalidateAll();                               /*  ICACHE 全部无效             */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_ICache.CACHE_uiLineSize);
            mipsICacheR4kInvalidate(pvAdrs, (PVOID)ulEnd, _G_ICache.CACHE_uiLineSize);
        }

    } else {
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (MIPS_CACHE_HAS_L2) {
            return  (mipsL2R4kInvalidate(pvAdrs, stBytes));
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
        addr_t  ulStart = (addr_t)pvAdrs;
                ulEnd   = ulStart + stBytes;
            
        if (ulStart & ((addr_t)_G_DCache.CACHE_uiLineSize - 1)) {       /*  起始地址非 cache line 对齐  */
            ulStart &= ~((addr_t)_G_DCache.CACHE_uiLineSize - 1);
            mipsDCacheR4kClear((PVOID)ulStart, (PVOID)ulStart, _G_DCache.CACHE_uiLineSize);
            ulStart += _G_DCache.CACHE_uiLineSize;
        }
            
        if (ulEnd & ((addr_t)_G_DCache.CACHE_uiLineSize - 1)) {         /*  结束地址非 cache line 对齐  */
            ulEnd &= ~((addr_t)_G_DCache.CACHE_uiLineSize - 1);
            mipsDCacheR4kClear((PVOID)ulEnd, (PVOID)ulEnd, _G_DCache.CACHE_uiLineSize);
        }
                                                                        /*  仅无效对齐部分              */
        mipsDCacheR4kInvalidate((PVOID)ulStart, (PVOID)ulEnd, _G_DCache.CACHE_uiLineSize);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kInvalidatePage
** 功能描述: 指定类型的 CACHE 使部分无效(访问不命中)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块:
*********************************************************************************************************/
static INT  mipsCacheR4kInvalidatePage (LW_CACHE_TYPE cachetype, PVOID pvAdrs, PVOID pvPdrs, size_t stBytes)
{
    addr_t  ulEnd;
    
    if (stBytes == 0) {
        return  (ERROR_NONE);
    }

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
            mipsICacheR4kInvalidateAll();                               /*  ICACHE 全部无效             */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_ICache.CACHE_uiLineSize);
            mipsICacheR4kInvalidate(pvAdrs, (PVOID)ulEnd, _G_ICache.CACHE_uiLineSize);
        }

    } else {
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (MIPS_CACHE_HAS_L2) {
            return  (mipsL2R4kInvalidate(pvAdrs, stBytes));
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
        addr_t  ulStart = (addr_t)pvAdrs;
                ulEnd   = ulStart + stBytes;
                    
        if (ulStart & ((addr_t)_G_DCache.CACHE_uiLineSize - 1)) {       /*  起始地址非 cache line 对齐  */
            ulStart &= ~((addr_t)_G_DCache.CACHE_uiLineSize - 1);
            mipsDCacheR4kClear((PVOID)ulStart, (PVOID)ulStart, _G_DCache.CACHE_uiLineSize);
            ulStart += _G_DCache.CACHE_uiLineSize;
        }
            
        if (ulEnd & ((addr_t)_G_DCache.CACHE_uiLineSize - 1)) {         /*  结束地址非 cache line 对齐  */
            ulEnd &= ~((addr_t)_G_DCache.CACHE_uiLineSize - 1);
            mipsDCacheR4kClear((PVOID)ulEnd, (PVOID)ulEnd, _G_DCache.CACHE_uiLineSize);
        }
                                                                        /*  仅无效对齐部分              */
        mipsDCacheR4kInvalidate((PVOID)ulStart, (PVOID)ulEnd, _G_DCache.CACHE_uiLineSize);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kClear
** 功能描述: 指定类型的 CACHE 使部分或全部清空(回写内存)并无效(访问不命中)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  mipsCacheR4kClear (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;

    if (stBytes == 0) {
        return  (ERROR_NONE);
    }

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
            mipsICacheR4kInvalidateAll();                               /*  ICACHE 全部无效             */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_ICache.CACHE_uiLineSize);
            mipsICacheR4kInvalidate(pvAdrs, (PVOID)ulEnd, _G_ICache.CACHE_uiLineSize);
        }

    } else {
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (MIPS_CACHE_HAS_L2) {
            return  (mipsL2R4kClear(pvAdrs, stBytes));
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
        if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
            mipsDCacheR4kClearAll();                                    /*  全部回写并无效              */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_DCache.CACHE_uiLineSize);
            mipsDCacheR4kClear(pvAdrs, (PVOID)ulEnd,
                               _G_DCache.CACHE_uiLineSize);             /*  部分回写并无效              */
        }
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kClearPage
** 功能描述: 指定类型的 CACHE 使部分或全部清空(回写内存)并无效(访问不命中)
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           pvPdrs        物理地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  mipsCacheR4kClearPage (LW_CACHE_TYPE cachetype, PVOID pvAdrs, PVOID pvPdrs, size_t stBytes)
{
    addr_t  ulEnd;
    
    if (stBytes == 0) {
        return  (ERROR_NONE);
    }

    if (cachetype == INSTRUCTION_CACHE) {
        if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
            mipsICacheR4kInvalidateAll();                               /*  ICACHE 全部无效             */
            
        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_ICache.CACHE_uiLineSize);
            mipsICacheR4kInvalidate(pvAdrs, (PVOID)ulEnd, _G_ICache.CACHE_uiLineSize);
        }

    } else {
#if LW_CFG_MIPS_CACHE_L2 > 0
        if (MIPS_CACHE_HAS_L2) {
            return  (mipsL2R4kClear(pvAdrs, stBytes));
        }
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
        if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
            mipsDCacheR4kClearAll();                                    /*  全部回写并无效              */

        } else {
            MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_DCache.CACHE_uiLineSize);
            mipsDCacheR4kClear(pvAdrs, (PVOID)ulEnd,
                               _G_DCache.CACHE_uiLineSize);             /*  部分回写并无效              */
        }
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kLock
** 功能描述: 锁定指定类型的 CACHE 
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  mipsCacheR4kLock (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kUnlock
** 功能描述: 解锁指定类型的 CACHE 
** 输　入  : cachetype     CACHE 类型
**           pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  mipsCacheR4kUnlock (LW_CACHE_TYPE  cachetype, PVOID  pvAdrs, size_t  stBytes)
{
    _ErrorHandle(ENOSYS);
    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kTextUpdate
** 功能描述: 清空(回写内存) D CACHE 无效(访问不命中) I CACHE
** 输　入  : pvAdrs        虚拟地址
**           stBytes       长度
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
** 注  意  : L2 cache 为统一 CACHE 所以 text update 不需要操作 L2 cache.
*********************************************************************************************************/
static INT  mipsCacheR4kTextUpdate (PVOID  pvAdrs, size_t  stBytes)
{
    addr_t  ulEnd;
    
    if (stBytes == 0) {
        return  (ERROR_NONE);
    }

    if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {
        mipsDCacheR4kFlushAll();                                        /*  DCACHE 全部回写             */
        mipsICacheR4kInvalidateAll();                                   /*  ICACHE 全部无效             */
        
    } else {
        PVOID   pvAdrsBak = pvAdrs;

        MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_DCache.CACHE_uiLineSize);
        mipsDCacheR4kFlush(pvAdrs, (PVOID)ulEnd,
                           _G_DCache.CACHE_uiLineSize);                 /*  部分回写                    */

        MIPS_CACHE_GET_END(pvAdrsBak, stBytes, ulEnd, _G_ICache.CACHE_uiLineSize);
        mipsICacheR4kInvalidate(pvAdrsBak, (PVOID)ulEnd, _G_ICache.CACHE_uiLineSize);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kDataUpdate
** 功能描述: 回写 D CACHE (仅回写 CPU 独享级 CACHE)
** 输　入  : pvAdrs                        虚拟地址
**           stBytes                       长度
**           bInv                          是否为回写无效
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
** 注  意  : L2 cache 为统一 CACHE 所以 data update 不需要操作 L2 cache.
*********************************************************************************************************/
INT  mipsCacheR4kDataUpdate (PVOID  pvAdrs, size_t  stBytes, BOOL  bInv)
{
    addr_t  ulEnd;

    if (stBytes == 0) {
        return  (ERROR_NONE);
    }

    if (stBytes >= MIPS_CACHE_LOOP_OP_MAX_SIZE) {                       /*  全部回写                    */
        if (bInv) {
            mipsDCacheR4kClearAll();
        } else {
            mipsDCacheR4kFlushAll();
        }

    } else {                                                            /*  部分回写                    */
        MIPS_CACHE_GET_END(pvAdrs, stBytes, ulEnd, _G_DCache.CACHE_uiLineSize);
        if (bInv) {
            mipsDCacheR4kClear(pvAdrs, (PVOID)ulEnd, _G_DCache.CACHE_uiLineSize);
        } else {
            mipsDCacheR4kFlush(pvAdrs, (PVOID)ulEnd, _G_DCache.CACHE_uiLineSize);
        }
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kInitHw
** 功能描述: CACHE 硬件初始化
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsCacheR4kInitHw (VOID)
{
    REGISTER PCHAR   pcLineAddr;
    REGISTER PCHAR   pcBaseAddr = (PCHAR)MIPS_CACHE_INDEX_BASE;
    REGISTER PCHAR   pcEndAddr;
    REGISTER PCHAR   pcWayEndAddr;
    REGISTER CHAR    cTemp;
    REGISTER INT32   iWay;

    (VOID)cTemp;

    mipsCp0TagLoWrite(0);

    if (MIPS_CACHE_HAS_TAG_HI) {
        mipsCp0TagHiWrite(0);
    }

    if (MIPS_CACHE_HAS_ECC) {
        mipsCp0ECCWrite(0);
    }

    pcEndAddr    = (PCHAR)(MIPS_CACHE_INDEX_BASE + _G_ICache.CACHE_uiSize);
    pcWayEndAddr = (PCHAR)(MIPS_CACHE_INDEX_BASE + _G_ICache.CACHE_uiWaySize);

    /*
     * Clear tag to invalidate
     */
    for (pcLineAddr = pcBaseAddr; pcLineAddr < pcWayEndAddr; pcLineAddr += _G_ICache.CACHE_uiLineSize) {
        for (iWay = 0; iWay < _G_ICache.CACHE_uiWayNr; iWay ++) {
            mipsICacheR4kIndexStoreTag(pcLineAddr + (iWay << _G_ICache.CACHE_uiWayBit));
        }
    }

    if (MIPS_CACHE_HAS_FILL_I) {
        /*
         * Fill so data field parity is correct
         */
        for (pcLineAddr = pcBaseAddr; pcLineAddr < pcWayEndAddr; pcLineAddr += _G_ICache.CACHE_uiLineSize) {
            mipsICacheR4kFill(pcLineAddr);
        }
        /*
         * Invalidate again – prudent but not strictly necessay
         */
        for (pcLineAddr = pcBaseAddr; pcLineAddr < pcWayEndAddr; pcLineAddr += _G_ICache.CACHE_uiLineSize) {
            for (iWay = 0; iWay < _G_ICache.CACHE_uiWayNr; iWay ++) {
                mipsICacheR4kIndexStoreTag(pcLineAddr + (iWay << _G_ICache.CACHE_uiWayBit));
            }
        }
    }

    if (MIPS_CACHE_HAS_ECC) {
        mipsCp0ECCWrite(MIPS_CACHE_ECC_VALUE);
    }

    pcEndAddr    = (PCHAR)(MIPS_CACHE_INDEX_BASE + _G_DCache.CACHE_uiSize);
    pcWayEndAddr = (PCHAR)(MIPS_CACHE_INDEX_BASE + _G_DCache.CACHE_uiWaySize);

    /*
     * Clear all tags
     */
    for (pcLineAddr = pcBaseAddr; pcLineAddr < pcWayEndAddr; pcLineAddr += _G_DCache.CACHE_uiLineSize) {
        for (iWay = 0; iWay < _G_DCache.CACHE_uiWayNr; iWay ++) {
            mipsDCacheR4kIndexStoreTag(pcLineAddr + (iWay << _G_DCache.CACHE_uiWayBit));
        }
    }

    /*
     * Load from each line (in cached space)
     */
    for (pcLineAddr = pcBaseAddr; pcLineAddr < pcEndAddr; pcLineAddr += _G_DCache.CACHE_uiLineSize) {
        cTemp = *pcLineAddr;
    }

    /*
     * Clear all tags again
     */
    for (pcLineAddr = pcBaseAddr; pcLineAddr < pcWayEndAddr; pcLineAddr += _G_DCache.CACHE_uiLineSize) {
        for (iWay = 0; iWay < _G_DCache.CACHE_uiWayNr; iWay ++) {
            mipsDCacheR4kIndexStoreTag(pcLineAddr + (iWay << _G_DCache.CACHE_uiWayBit));
        }
    }

#if LW_CFG_MIPS_CACHE_L2 > 0
    mipsL2R4kInitHw();
#endif                                                                  /*  LW_CFG_MIPS_CACHE_L2 > 0    */
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kInit
** 功能描述: 初始化 CACHE
** 输　入  : pcacheop       CACHE 操作函数集
**           uiInstruction  指令 CACHE 参数
**           uiData         数据 CACHE 参数
**           pcMachineName  机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  mipsCacheR4kInit (LW_CACHE_OP  *pcacheop,
                        CACHE_MODE    uiInstruction,
                        CACHE_MODE    uiData,
                        CPCHAR        pcMachineName)
{
    mipsCacheProbe(pcMachineName);                                      /*  CACHE 探测                  */
    mipsCacheInfoShow();                                                /*  打印 CACHE 信息             */
    mipsCacheR4kDisableHw();                                            /*  关闭 CACHE                  */
    mipsBranchPredictorInvalidate();                                    /*  无效分支预测                */
    mipsCacheR4kInitHw();                                               /*  初始化 CACHE                */

#if LW_CFG_SMP_EN > 0
    pcacheop->CACHEOP_ulOption = CACHE_TEXT_UPDATE_MP;
#else
    pcacheop->CACHEOP_ulOption = 0ul;
#endif                                                                  /*  LW_CFG_SMP_EN               */

    pcacheop->CACHEOP_iILoc = CACHE_LOCATION_VIPT;
    pcacheop->CACHEOP_iDLoc = CACHE_LOCATION_VIPT;

    pcacheop->CACHEOP_iICacheLine = _G_ICache.CACHE_uiLineSize;
    pcacheop->CACHEOP_iDCacheLine = _G_DCache.CACHE_uiLineSize;

    pcacheop->CACHEOP_iICacheWaySize = _G_ICache.CACHE_uiWaySize;
    pcacheop->CACHEOP_iDCacheWaySize = _G_DCache.CACHE_uiWaySize;

    pcacheop->CACHEOP_pfuncEnable  = mipsCacheR4kEnable;
    pcacheop->CACHEOP_pfuncDisable = mipsCacheR4kDisable;
    
    pcacheop->CACHEOP_pfuncLock   = mipsCacheR4kLock;                   /*  暂时不支持锁定操作          */
    pcacheop->CACHEOP_pfuncUnlock = mipsCacheR4kUnlock;

    pcacheop->CACHEOP_pfuncFlush          = mipsCacheR4kFlush;
    pcacheop->CACHEOP_pfuncFlushPage      = mipsCacheR4kFlushPage;
    pcacheop->CACHEOP_pfuncInvalidate     = mipsCacheR4kInvalidate;
    pcacheop->CACHEOP_pfuncInvalidatePage = mipsCacheR4kInvalidatePage;
    pcacheop->CACHEOP_pfuncClear          = mipsCacheR4kClear;
    pcacheop->CACHEOP_pfuncClearPage      = mipsCacheR4kClearPage;
    pcacheop->CACHEOP_pfuncTextUpdate     = mipsCacheR4kTextUpdate;
    pcacheop->CACHEOP_pfuncDataUpdate     = mipsCacheR4kDataUpdate;
    
#if LW_CFG_VMM_EN > 0
    pcacheop->CACHEOP_pfuncDmaMalloc      = API_VmmDmaAlloc;
    pcacheop->CACHEOP_pfuncDmaMallocAlign = API_VmmDmaAllocAlign;
    pcacheop->CACHEOP_pfuncDmaFree        = API_VmmDmaFree;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
** 函数名称: mipsCacheR4kReset
** 功能描述: 复位 CACHE 
** 输　入  : pcMachineName  机器名称
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
** 注  意  : 如果有 lockdown 必须首先 unlock & invalidate 才能启动
*********************************************************************************************************/
VOID  mipsCacheR4kReset (CPCHAR  pcMachineName)
{
    mipsCacheProbe(pcMachineName);                                      /*  CACHE 探测                  */
    mipsCacheR4kDisableHw();                                            /*  关闭 CACHE                  */
    mipsBranchPredictorInvalidate();                                    /*  无效分支预测                */
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
