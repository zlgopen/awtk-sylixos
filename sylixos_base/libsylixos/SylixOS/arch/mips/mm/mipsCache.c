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
** 文   件   名: mipsCache.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: MIPS 体系架构 CACHE 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0
#include "cache/r4k/mipsCacheR4k.h"
#include "cache/loongson3x/mipsCacheLs3x.h"
#include "cache/hr2/mipsCacheHr2.h"
#include "arch/mips/common/mipsCpuProbe.h"
/*********************************************************************************************************
** 函数名称: archCacheInit
** 功能描述: 初始化 CACHE
** 输　入  : uiInstruction  指令 CACHE 参数
**           uiData         数据 CACHE 参数
**           pcMachineName  机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archCacheInit (CACHE_MODE  uiInstruction, CACHE_MODE  uiData, CPCHAR  pcMachineName)
{
    LW_CACHE_OP  *pcacheop = API_CacheGetLibBlock();

    _DebugFormat(__LOGMESSAGE_LEVEL, "%s %s L1 cache controller initialization.\r\n", 
                 LW_CFG_CPU_ARCH_FAMILY, pcMachineName);

    mipsCpuProbe(pcMachineName);                                        /*  MIPS CPU 探测               */

    if ((_G_uiMipsCpuType == CPU_LOONGSON3) ||                          /*  Loongson-3x/2G/2H           */
        (_G_uiMipsCpuType == CPU_LOONGSON2K)) {                         /*  Loongson-2K                 */
        mipsCacheLs3xInit(pcacheop, uiInstruction, uiData, pcMachineName);

    } else if (_G_uiMipsCpuType == CPU_CETC_HR2) {                      /*  CETC-HR2                    */
        mipsCacheHr2Init(pcacheop, uiInstruction, uiData, pcMachineName);

    } else if ((lib_strcmp(pcMachineName, MIPS_MACHINE_LS1X)   == 0) ||
               (lib_strcmp(pcMachineName, MIPS_MACHINE_LS2X)   == 0) || /*  Loongson-2E/2F              */
               (lib_strcmp(pcMachineName, MIPS_MACHINE_24KF)   == 0) ||
               (lib_strcmp(pcMachineName, MIPS_MACHINE_JZ47XX) == 0)) {
        mipsCacheR4kInit(pcacheop, uiInstruction, uiData, pcMachineName);

    } else {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "unknown machine name.\r\n");
    }
}
/*********************************************************************************************************
** 函数名称: archCacheReset
** 功能描述: 复位 CACHE, MMU 初始化时需要调用此函数
** 输　入  : pcMachineName  机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archCacheReset (CPCHAR  pcMachineName)
{
    mipsCpuProbe(pcMachineName);                                        /*  MIPS CPU 探测               */

    if ((_G_uiMipsCpuType == CPU_LOONGSON3) ||                          /*  Loongson-3x/2G/2H           */
        (_G_uiMipsCpuType == CPU_LOONGSON2K)) {                         /*  Loongson-2K                 */
        mipsCacheLs3xReset(pcMachineName);

    } else if (_G_uiMipsCpuType == CPU_CETC_HR2) {                      /*  CETC-HR2                    */
        mipsCacheHr2Reset(pcMachineName);

    } else if ((lib_strcmp(pcMachineName, MIPS_MACHINE_LS1X)   == 0) ||
               (lib_strcmp(pcMachineName, MIPS_MACHINE_LS2X)   == 0) || /*  Loongson-2E/2F              */
               (lib_strcmp(pcMachineName, MIPS_MACHINE_24KF)   == 0) ||
               (lib_strcmp(pcMachineName, MIPS_MACHINE_JZ47XX) == 0)) {
        mipsCacheR4kReset(pcMachineName);

    } else {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "unknown machine name.\r\n");
    }
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
