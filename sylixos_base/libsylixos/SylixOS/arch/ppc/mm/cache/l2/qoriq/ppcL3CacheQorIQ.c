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
** 文   件   名: ppcL3CacheQorIQ.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 08 月 07 日
**
** 描        述: QorIQ 体系构架 L3 CACHE 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁减配置
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0 && LW_CFG_PPC_CACHE_L2 > 0
#include "../ppcL2.h"
#include "ppcL3CacheQorIQ.h"
/*********************************************************************************************************
  L3 Cache controller registers defines
*********************************************************************************************************/
#define CPCCSR0                 0x0000
#define CPCCFG0                 0x0008
#define CPCEWCR0                0x0010
#define CPCEWBAR0               0x0014
#define CPCEWCR1                0x0020
#define CPCEWBAR1               0x0024
#define CPCSRCR1                0x0100
#define CPCSRCR0                0x0104
#define CPCPAR(n)               (0x208 + (n) * 0x10)
#define CPCPAR_NUM              16

#define CPC_ENABLE              0x80000000                              /*  CPCCSR0_CPCE                */
#define CPC_PE                  0x40000000                              /*  CPCCSR0_CCPE                */
#define CPC_WT                  0x00800000                              /*  CPCWT                       */
#define CPC_FLUSH               0x00000800                              /*  CPCCSR0_CPCFL               */
#define CPC_INVALIDATE          0x00200400                              /*  CPCCSR0_CPCFI|CPCCSR0_CPCLFC*/
#define CPC_CLEAR_LOCKS         0x00000400                              /*  CPCCSR0_CPCLFC              */
#define CPC_ERRATA_A00N         0x01400000                              /*  CPC-A002, CPC-A003          */

#define CPCSIZE_MASK            0x00003fff                              /*  Cache size mask             */
#define CPCSIZE_UNIT            0x10000                                 /*  Cache size unit in bytes    */
#define CPCPAR_RESET_VALUE      0x000003fb
/*********************************************************************************************************
  Reg op
*********************************************************************************************************/
#define L3REG_READ(i, reg)          read32(_G_l3Config.CFG_ulBase[i] + (reg))
#define L3REG_WRITE(i, reg, data)   write32(data, _G_l3Config.CFG_ulBase[i] + (reg))
/*********************************************************************************************************
  L3 CACHE 配置(默认不存在 L3 CACHE，其它值为配置示例)
*********************************************************************************************************/
static PPC_QORIQ_L3CACHE_CONFIG     _G_l3Config = {
    .CFG_bPresent = LW_FALSE,
};
/*********************************************************************************************************
** 函数名称: ppcQorIQL3CacheConfig
** 功能描述: L3 CACHE 配置
** 输　入  : pL3Config         L3 CACHE 配置
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ppcQorIQL3CacheConfig (PPC_QORIQ_L3CACHE_CONFIG  *pL3Config)
{
    if (pL3Config) {
        _G_l3Config = *pL3Config;
    }
}
/*********************************************************************************************************
** 函数名称: ppcQorIQL3CacheEnable
** 功能描述: L3 CACHE 使能
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcQorIQL3CacheEnable (VOID)
{
    INT  i;

    for (i = 0; i < _G_l3Config.CFG_uiNum; i++) {
        if (!(L3REG_READ(i, CPCCSR0) & CPC_ENABLE)) {
            L3REG_WRITE(i, CPCCSR0, L3REG_READ(i, CPCCSR0) | CPC_ENABLE);
        }
    }
}
/*********************************************************************************************************
** 函数名称: ppcQorIQL3CacheDisable
** 功能描述: L3 CACHE 禁能
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcQorIQL3CacheDisable (VOID)
{
    INT     i;
    UINT32  uiAllocation;

    for (i = 0; i < _G_l3Config.CFG_uiNum; i++) {
        /*
         * Check whether the L3 Cache has been enabled
         */
        if (L3REG_READ(i, CPCCSR0) & CPC_ENABLE) {
            for (uiAllocation = 0; uiAllocation < CPCPAR_NUM; uiAllocation++) {
                L3REG_WRITE(i, CPCPAR(uiAllocation), CPCPAR_RESET_VALUE);
            }

            if (!(L3REG_READ(i, CPCCSR0) & CPC_WT)) {
                /*
                 * If the L3 Cache works at none write through mode, it needed
                 * to be flushed at first.
                 */
                L3REG_WRITE(i, CPCCSR0, L3REG_READ(i, CPCCSR0) | CPC_FLUSH);
                while (L3REG_READ(i, CPCCSR0) & CPC_FLUSH) {
                }
            }

            /*
             * Clearing of lock bits
             */
            L3REG_WRITE(i, CPCCSR0, L3REG_READ(i, CPCCSR0) | CPC_CLEAR_LOCKS);
            while (L3REG_READ(i, CPCCSR0) & CPC_CLEAR_LOCKS) {
            }

            /*
             * Flash invalidate the L3 cache
             */
            L3REG_WRITE(i, CPCCSR0, L3REG_READ(i, CPCCSR0) & CPC_INVALIDATE);
            while (L3REG_READ(i, CPCCSR0) & CPC_INVALIDATE) {
            }

            /*
             * Disable the L3 cache
             */
            L3REG_WRITE(i, CPCCSR0,
                        L3REG_READ(i, CPCCSR0) & ~(CPC_ENABLE | CPC_PE));
        }
    }
}
/*********************************************************************************************************
** 函数名称: ppcQorIQL3CacheInit
** 功能描述: 初始化 L3 CACHE 控制器
** 输　入  : pl3cdrv            驱动结构
**           uiInstruction      指令 CACHE 类型
**           uiData             数据 CACHE 类型
**           pcMachineName      机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ppcQorIQL3CacheInit (L2C_DRVIER  *pl3cdrv,
                           CACHE_MODE   uiInstruction,
                           CACHE_MODE   uiData,
                           CPCHAR       pcMachineName)
{
    if (_G_l3Config.CFG_bPresent) {
        _DebugFormat(__LOGMESSAGE_LEVEL, "%s %s L3 cache controller initialization.\r\n",
                     LW_CFG_CPU_ARCH_FAMILY, "QorIQ");

        pl3cdrv->L2CD_pcName             = "QorIQ";
        pl3cdrv->L2CD_stSize             = (L3REG_READ(0, CPCCFG0) * CPCSIZE_MASK) * CPCSIZE_UNIT;
        pl3cdrv->L2CD_pfuncEnable        = ppcQorIQL3CacheEnable;
        pl3cdrv->L2CD_pfuncDisable       = ppcQorIQL3CacheDisable;
        pl3cdrv->L2CD_pfuncIsEnable      = LW_NULL;
        pl3cdrv->L2CD_pfuncSync          = LW_NULL;
        pl3cdrv->L2CD_pfuncFlush         = LW_NULL;
        pl3cdrv->L2CD_pfuncFlushAll      = LW_NULL;
        pl3cdrv->L2CD_pfuncInvalidate    = LW_NULL;
        pl3cdrv->L2CD_pfuncInvalidateAll = LW_NULL;
        pl3cdrv->L2CD_pfuncClear         = LW_NULL;
        pl3cdrv->L2CD_pfuncClearAll      = LW_NULL;

        /*
         * Reset the L3 Cache to a known status
         */
        ppcQorIQL3CacheDisable();
    }
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
                                                                        /*  LW_CFG_PPC_CACHE_L2 > 0     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
