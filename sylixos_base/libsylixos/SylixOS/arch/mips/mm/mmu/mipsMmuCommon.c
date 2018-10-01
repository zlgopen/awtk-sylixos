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
** 文   件   名: mipsMmuCommon.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 10 月 12 日
**
** 描        述: MIPS 体系构架 MMU 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0
#include "mipsMmuCommon.h"
#include "arch/mips/common/cp0/mipsCp0.h"
#include "arch/mips/common/mipsCpuProbe.h"
#include "arch/mips/inc/addrspace.h"
#include "mips32/mips32Mmu.h"
#include "mips64/mips64Mmu.h"
/*********************************************************************************************************
  UNIQUE ENTRYHI(按 512KB 步进, 提升兼容性, TLB 数目最多 64 个, 不会上溢到 CKSEG1)
*********************************************************************************************************/
#define MIPS_UNIQUE_ENTRYHI(idx)        (CKSEG0 + ((idx) << (18 + 1)))
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
BOOL         _G_bMmuHasXI           = LW_FALSE;                         /*  是否有 XI 位                */
UINT32       _G_uiMmuTlbSize        = 0;                                /*  TLB 数组大小                */
UINT32       _G_uiMmuEntryLoUnCache = CONF_CM_UNCACHED;                 /*  非高速缓存                  */
UINT32       _G_uiMmuEntryLoCache   = CONF_CM_CACHABLE_NONCOHERENT;     /*  一致性高速缓存              */
/*********************************************************************************************************
** 函数名称: mipsMmuEnable
** 功能描述: 使能 MMU
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsMmuEnable (VOID)
{
}
/*********************************************************************************************************
** 函数名称: mipsMmuDisable
** 功能描述: 禁能 MMU
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsMmuDisable (VOID)
{
}
/*********************************************************************************************************
** 函数名称: mipsMmuInvalidateMicroTLB
** 功能描述: 无效 Micro TLB
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  mipsMmuInvalidateMicroTLB (VOID)
{
    switch (_G_uiMipsCpuType) {

    case CPU_LOONGSON2:
        mipsCp0DiagWrite(LOONGSON_DIAG_ITLB);                           /*  无效 ITLB                   */
        break;

    case CPU_LOONGSON3:
    case CPU_LOONGSON2K:
        mipsCp0DiagWrite((LOONGSON_DIAG_DTLB) |
                         (LOONGSON_DIAG_ITLB));                         /*  无效 ITLB DTLB              */
        break;

    default:
        /*
         * CETC-HR2 和君正的 CPU, ITLB 和 DTLB 对软件透明
         */
        break;
    }
}
/*********************************************************************************************************
** 函数名称: mipsMmuInvalidateTLB
** 功能描述: 无效 TLB
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : 内部会无效 Micro TLB
*********************************************************************************************************/
static VOID  mipsMmuInvalidateTLB (VOID)
{
    ULONG   ulEntryHiBak = mipsCp0EntryHiRead();
    INT     i;

    for (i = 0; i < MIPS_MMU_TLB_SIZE; i++) {
        mipsCp0IndexWrite(i);
        mipsCp0EntryLo0Write(0);
        mipsCp0EntryLo1Write(0);
        mipsCp0EntryHiWrite(MIPS_UNIQUE_ENTRYHI(i));
        MIPS_MMU_TLB_WRITE_INDEX();
    }

    mipsCp0EntryHiWrite(ulEntryHiBak);

    mipsMmuInvalidateMicroTLB();                                        /*  无效 Micro TLB              */
}
/*********************************************************************************************************
** 函数名称: mipsMmuInvalidateTLBMVA
** 功能描述: 无效指定 MVA 的 TLB
** 输　入  : ulAddr            指定 MVA
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : 内部不会无效 Micro TLB, 外部操作完成后必须无效 Micro TLB
*********************************************************************************************************/
VOID  mipsMmuInvalidateTLBMVA (addr_t  ulAddr)
{
    ULONG   ulEntryHiBak = mipsCp0EntryHiRead();
    ULONG   ulEntryHi    = ulAddr & (LW_CFG_VMM_PAGE_MASK << 1);
    INT32   iIndex;
    INT     iReTry;

    for (iReTry = 0; iReTry < 2; iReTry++) {                            /*  不会出现两条一样的 TLB 条目 */
        mipsCp0EntryHiWrite(ulEntryHi);
        MIPS_MMU_TLB_PROBE();
        iIndex = mipsCp0IndexRead();
        if (iIndex >= 0) {
            mipsCp0IndexWrite(iIndex);
            mipsCp0EntryLo0Write(0);
            mipsCp0EntryLo1Write(0);
            mipsCp0EntryHiWrite(MIPS_UNIQUE_ENTRYHI(iIndex));
            MIPS_MMU_TLB_WRITE_INDEX();
        } else {
            break;
        }
    }

    mipsCp0EntryHiWrite(ulEntryHiBak);
}
/*********************************************************************************************************
** 函数名称: mipsMmuDumpTLB
** 功能描述: Dump TLB
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  mipsMmuDumpTLB (VOID)
{
    ULONG   ulEntryHiBak = mipsCp0EntryHiRead();
    ULONG   ulEntryLo0;
    ULONG   ulEntryLo1;
    ULONG   ulEntryHi;
    ULONG   ulPageMask;
    INT     i;

#if LW_CFG_CPU_WORD_LENGHT == 32
#define LX_FMT      "0x%08x"
#else
#define LX_FMT      "0x%016lx"
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/

    for (i = 0; i < MIPS_MMU_TLB_SIZE; i++) {
        mipsCp0IndexWrite(i);
        MIPS_MMU_TLB_READ();
        ulEntryLo0 = mipsCp0EntryLo0Read();
        ulEntryLo1 = mipsCp0EntryLo1Read();
        ulEntryHi  = mipsCp0EntryHiRead();
        ulPageMask = mipsCp0PageMaskRead();

        _PrintFormat("TLB[%02d]: EntryLo0="LX_FMT", EntryLo1=0x%"LX_FMT", "
                     "EntryHi="LX_FMT", PageMask="LX_FMT"\r\n",
                     i, ulEntryLo0, ulEntryLo1, ulEntryHi, ulPageMask);
    }

    mipsCp0EntryHiWrite(ulEntryHiBak);
}
/*********************************************************************************************************
** 函数名称: mipsMmuGlobalInit
** 功能描述: 调用 BSP 对 MMU 初始化
** 输　入  : pcMachineName     使用的机器名称
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  mipsMmuGlobalInit (CPCHAR  pcMachineName)
{
    archCacheReset(pcMachineName);                                      /*  复位 CACHE                  */

    mipsCp0PageMaskWrite(MIPS_MMU_PAGE_MASK);                           /*  PAGE MASK                   */
    mipsCp0EntryHiWrite(0);                                             /*  ASID = 0                    */
    mipsCp0WiredWrite(0);                                               /*  全部允许随机替换            */
    mipsMmuInvalidateTLB();                                             /*  无效 TLB                    */

    if (_G_bMmuHasXI) {                                                 /*  有执行阻止位                */
        mipsCp0PageGrainWrite(
#if LW_CFG_CPU_WORD_LENGHT == 64
                              PG_ELPA |                                 /*  支持 48 位物理地址          */
#endif
                              PG_XIE);                                  /*  使能 EntryLo 执行阻止位     */
                                                                        /*  执行阻止例外复用 TLBL 例外  */
    }

    if ((_G_uiMipsCpuType == CPU_LOONGSON3) ||                          /*  Loongson-3x/2G/2H           */
        (_G_uiMipsCpuType == CPU_LOONGSON2K)) {                         /*  Loongson-2K                 */
        UINT32  uiGSConfig = mipsCp0GSConfigRead();
        uiGSConfig &= ~(1 <<  3);                                       /*  Store 操作也进行硬件自动预取*/
        uiGSConfig &= ~(1 << 15);                                       /*  使用 VCache                 */
        uiGSConfig |=  (1 <<  8) |                                      /*  Store 操作自动写合并        */
                       (1 <<  1) |                                      /*  取指操作进行硬件自动预取    */
                       (1 <<  0) |                                      /*  数据访存操作进行硬件自动预取*/
                       MIPS_CONF6_FTLBDIS;                              /*  只用 VTLB, 不用 FTLB        */
        __asm__ __volatile__("sync" : : : "memory");
        mipsCp0GSConfigWrite(uiGSConfig);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsMmuTlbLoadStoreExcHandle
** 功能描述: MMU TLB 加载/存储异常处理
** 输　入  : ulAbortAddr       终止地址
** 输　出  : 终止类型
** 全局变量:
** 调用模块:
*********************************************************************************************************/
ULONG  mipsMmuTlbLoadStoreExcHandle (addr_t  ulAbortAddr, BOOL  bStore)
{
    ULONG  ulAbortType  = LW_VMM_ABORT_TYPE_MAP;
    ULONG  ulEntryHiBak = mipsCp0EntryHiRead();
    ULONG  ulEntryLo;
    INT32  iIndex;
    BOOL   bIsEntryLo1;

    mipsCp0EntryHiWrite(ulAbortAddr & (LW_CFG_VMM_PAGE_MASK << 1));
    MIPS_MMU_TLB_PROBE();
    iIndex = mipsCp0IndexRead();
    if (iIndex >= 0) {
        MIPS_MMU_TLB_READ();
        bIsEntryLo1 = !!(ulAbortAddr & (1 << LW_CFG_VMM_PAGE_SHIFT));
        if (bIsEntryLo1) {
            ulEntryLo = mipsCp0EntryLo1Read();
        } else {
            ulEntryLo = mipsCp0EntryLo0Read();
        }

        if (ulEntryLo & ENTRYLO_V) {
            /*
             * TLB 中有映射条目, 并且映射有效, 理论上不应该出现这种情况,
             * 但在 Loongson-1B 处理器上偶尔会出现(QEMU 并不出现),
             * 可能这是 Loongson-1B 处理器的 BUG, 但不响应程序继续运行
             */
            ulAbortType = 0;
        } else {
            /*
             * TLB 无效异常(正确情况)
             */
            ulAbortType = LW_VMM_ABORT_TYPE_MAP;
        }
    } else {
        /*
         * 需要 TLB 重填, 但设计的 TLB 重填机制并不会由通用异常入口来处理
         */
        ULONG  ulNesting = LW_CPU_GET_CUR_NESTING();
        if (ulNesting > 1) {
            /*
             * 如果 TLB 重填异常发生在异常里, 直接终止
             */
            _DebugHandle(__ERRORMESSAGE_LEVEL, "TLB refill error.\r\n");
            ulAbortType = LW_VMM_ABORT_TYPE_FATAL_ERROR;
        } else {
            /*
             * 如果 TLB 重填异常不是发生异常里, 忽略之, 也不进行 TLB 重填
             * QEMU 运行 Qt 时会出现, Loongson-1B 处理器并不出现
             */
            ulAbortType = 0;
        }
    }

    mipsCp0EntryHiWrite(ulEntryHiBak);

    return  (ulAbortType);
}
/*********************************************************************************************************
** 函数名称: mipsMmuInvTLB
** 功能描述: 无效当前 CPU TLB
** 输　入  : pmmuctx        mmu 上下文
**           ulPageAddr     页面虚拟地址
**           ulPageNum      页面个数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsMmuInvTLB (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulPageAddr, ULONG  ulPageNum)
{
    REGISTER ULONG   i;

    if (ulPageNum > (MIPS_MMU_TLB_SIZE >> 1)) {
        mipsMmuInvalidateTLB();                                         /*  全部清除 TLB                */

    } else {
        for (i = 0; i < ulPageNum; i++) {
            mipsMmuInvalidateTLBMVA(ulPageAddr);                        /*  逐个页面清除 TLB            */
            ulPageAddr += LW_CFG_VMM_PAGE_SIZE;
        }

        mipsMmuInvalidateMicroTLB();                                    /*  无效 Micro TLB              */
    }
}
/*********************************************************************************************************
** 函数名称: mipsMmuInit
** 功能描述: MMU 系统初始化
** 输　入  : pmmuop            MMU 操作函数集
**           pcMachineName     使用的机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  mipsMmuInit (LW_MMU_OP  *pmmuop, CPCHAR  pcMachineName)
{
    UINT32  uiConfig;
    UINT32  uiMT;

    uiConfig = mipsCp0ConfigRead();                                     /*  读 Config0                  */
    uiMT = uiConfig & MIPS_CONF_MT;
    if ((uiMT != MIPS_CONF_MT_TLB) && (uiMT != MIPS_CONF_MT_FTLB)) {    /*  Config0 MT 域 != 1，没有 MMU*/
        _DebugFormat(__PRINTMESSAGE_LEVEL,
                     "Warning: Config register MMU type is not standard: %d!\r\n", uiMT);
    }

    if (uiConfig & MIPS_CONF_M) {                                       /*  有 Config1                  */
        uiConfig        = mipsCp0Config1Read();                         /*  读 Config1                  */
        _G_uiMmuTlbSize = ((uiConfig & MIPS_CONF1_TLBS) >> 25) + 1;     /*  获得 MMUSize 域             */

    } else {
        _G_uiMmuTlbSize = 64;                                           /*  按最大算                    */
    }

    _DebugFormat(__LOGMESSAGE_LEVEL, "%s MMU TLB size = %d.\r\n", pcMachineName, MIPS_MMU_TLB_SIZE);

    switch (_G_uiMipsCpuType) {

    case CPU_LOONGSON1:                                                 /*  Loongson-1x/2x/3x 和        */
    case CPU_LOONGSON2:
    case CPU_LOONGSON3:
    case CPU_LOONGSON2K:
    case CPU_JZRISC:                                                    /*  君正 CPU 都有执行阻止位     */
        _G_bMmuHasXI           = LW_TRUE;
        _G_uiMmuEntryLoUnCache = CONF_CM_UNCACHED;                      /*  非高速缓存                  */
        _G_uiMmuEntryLoCache   = CONF_CM_CACHABLE_NONCOHERENT;          /*  一致性高速缓存              */
        break;

    case CPU_CETC_HR2:                                                  /*  CETC-HR2                    */
        _G_bMmuHasXI           = LW_TRUE;
        _G_uiMmuEntryLoUnCache = CONF_CM_UNCACHED;                      /*  非高速缓存                  */
        _G_uiMmuEntryLoCache   = 6;                                     /*  一致性高速缓存              */
        break;

    default:
        _G_uiMmuEntryLoUnCache = CONF_CM_UNCACHED;                      /*  非高速缓存                  */
        _G_uiMmuEntryLoCache   = CONF_CM_CACHABLE_NONCOHERENT;          /*  一致性高速缓存              */
        break;
    }

#if LW_CFG_SMP_EN > 0
    pmmuop->MMUOP_ulOption = LW_VMM_MMU_FLUSH_TLB_MP;
#else
    pmmuop->MMUOP_ulOption = 0ul;
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */

    pmmuop->MMUOP_pfuncGlobalInit    = mipsMmuGlobalInit;
    pmmuop->MMUOP_pfuncInvalidateTLB = mipsMmuInvTLB;
    pmmuop->MMUOP_pfuncSetEnable     = mipsMmuEnable;
    pmmuop->MMUOP_pfuncSetDisable    = mipsMmuDisable;

#if LW_CFG_CPU_WORD_LENGHT == 32
    mips32MmuInit(pmmuop, pcMachineName);
#else
    mips64MmuInit(pmmuop, pcMachineName);
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/
}

#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
