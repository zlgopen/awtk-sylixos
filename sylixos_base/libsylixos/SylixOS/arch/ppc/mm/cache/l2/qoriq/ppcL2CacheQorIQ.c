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
** 文   件   名: ppcL2CacheQorIQ.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 08 月 07 日
**
** 描        述: QorIQ 体系构架 L2 CACHE 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁减配置
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0 && LW_CFG_PPC_CACHE_L2 > 0
#include "../ppcL2.h"
#include "ppcL2CacheQorIQ.h"
/*********************************************************************************************************
  L2CTL
*********************************************************************************************************/
#define E500_L2CTL                  (0)

#define E500_L2CEWARn(n)            ((0x010) + (n * 0x10))
#define E500_L2CEWCRn(n)            ((0x018) + (n * 0x10))
#define E500_L2_EXTWR_REGION_NUM    4

#define E500_L2SRBAR0               (0x100)
#define E500_L2SRBAR1               (0x108)
#define E500_L2ERRINJHI             (0xe00)
#define E500_L2ERRINJLO             (0xe04)
#define E500_L2ERRINJCTL            (0xe08)
#define E500_L2ERRCAPTDATAHI        (0xe20)
#define E500_L2ERRCAPTDATALO        (0x20e24)
#define E500_L2ERRCAPTECC           (0x20e28)
#define E500_L2ERRDET               (0x20e40)
#define E500_L2ERRDIS               (0x20e44)
#define E500_L2ERRINTEN             (0x20e48)
#define E500_L2ERRATTR              (0x20e4c)
#define E500_L2ERRADDR              (0x20e50)
#define E500_L2ERRCTL               (0x20e58)
/*********************************************************************************************************
  L2CTL BIT MASKS and BIT SHIFTS
*********************************************************************************************************/
#define E500_L2CTL_L2E_MSK          0x80000000
#define E500_L2CTL_L2E_BIT          31
#define E500_L2CTL_L2I_MSK          0x40000000
#define E500_L2CTL_L2I_BIT          30
#define E500_L2CTL_L2SIZ_MSK        0x30000000
#define E500_L2CTL_L2SIZ_BIT        28

#define E500_L2CTL_L2DO_MSK         0x00400000
#define E500_L2CTL_L2DO_BIT         22
#define E500_L2CTL_L2IO_MSK         0x00200000
#define E500_L2CTL_L2IO_BIT         21
#define E500_L2CTL_L2MEXTDIS_MSK    0x00100000
#define E500_L2CTL_L2MEXTDIS_BIT    20
#define E500_L2CTL_L2INTDIS_MSK     0x00080000
#define E500_L2CTL_L2INTDIS_BIT     19
#define E500_L2CTL_L2SRAM_MSK       0x00070000
#define E500_L2CTL_L2SRAM_BIT       16
#define E500_L2CTL_L2LO_MSK         0x00002000
#define E500_L2CTL_L2LO_BIT         13
#define E500_L2CTL_L2SLC_MSK        0x00001000
#define E500_L2CTL_L2SLC_BIT        12
#define E500_L2CTL_L2LFR_MSK        0x00000400
#define E500_L2CTL_L2LFR_BIT        10
#define E500_L2CTL_L2LFRID_MSK      0x00000300
#define E500_L2CTL_L2LFRID_BIT      8
/*********************************************************************************************************
  L2CWAR BIT MASKS
*********************************************************************************************************/
#define E500_L2CEWAR_ADDR_MSK       0xffffff00
/*********************************************************************************************************
  L2CEWCR BIT MASKS and BIT SHIFTS
*********************************************************************************************************/
#define E500_L2CEWCR_E_MSK          0x80000000
#define E500_L2CEWCR_E_BIT          31
#define E500_L2CEWCR_LOCK_MSK       0x40000000
#define E500_L2CEWCR_LOCK_BIT       30
#define E500_L2CEWCR_SIZMASK_MSK    0x00ffffff
#define E500_L2CEWCR_SIZMASK_BIT    0
/*********************************************************************************************************
  L2SRBAR BIT MASKS and BIT SHIFTS
*********************************************************************************************************/
#define E500_L2SRBAR_ADDR_MSK       0xffffc000
#define E500_L2SRBAR_ADDR_BIT       14
/*********************************************************************************************************
  Size vs bit value
*********************************************************************************************************/
#define L2SIZ_1024KB                3
#define L2SIZ_512KB                 2
#define L2SIZ_256KB                 1
#define L2SIZ_0KB                   0
/*********************************************************************************************************
  Reg op
*********************************************************************************************************/
#define L2REG_READ(reg)             read32(_G_l2Config.CFG_ulBase + (reg))
#define L2REG_WRITE(reg, data)      write32(data, _G_l2Config.CFG_ulBase + (reg))
/*********************************************************************************************************
  外部变量声明
*********************************************************************************************************/
extern UINT32  PPC_E500_DCACHE_ALIGN_SIZE;
extern UINT32  PPC_E500_DCACHE_FLUSH_NUM;
/*********************************************************************************************************
  L2 CACHE 配置(默认不存在 L2 CACHE，其它值为配置示例)
*********************************************************************************************************/
static PPC_QORIQ_L2CACHE_CONFIG     _G_l2Config = {
    .CFG_bPresent   = LW_FALSE,
    .CFG_stSize     = 256 * LW_CFG_KB_SIZE,
    .CFG_ulBase     = 0,
};
/*********************************************************************************************************
** 函数名称: ppcQorIQL2CacheConfig
** 功能描述: L2 CACHE 配置
** 输　入  : pL2Config         L2 CACHE 配置
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ppcQorIQL2CacheConfig (PPC_QORIQ_L2CACHE_CONFIG  *pL2Config)
{
    if (pL2Config) {
        _G_l2Config = *pL2Config;
    }
}
/*********************************************************************************************************
** 函数名称: ppcQorIQL2CacheEnable
** 功能描述: L2 CACHE 使能
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcQorIQL2CacheEnable (VOID)
{
    UINT32  uiL2CtrlVal = L2REG_READ(E500_L2CTL);

    uiL2CtrlVal |= E500_L2CTL_L2E_MSK;
    L2REG_WRITE(E500_L2CTL, uiL2CtrlVal);
}
/*********************************************************************************************************
** 函数名称: ppcQorIQL2CacheDisable
** 功能描述: L2 CACHE 禁能
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcQorIQL2CacheDisable (VOID)
{
    UINT32 uiL2CtrlVal = L2REG_READ(E500_L2CTL);

    uiL2CtrlVal &= ~((UINT32)E500_L2CTL_L2E_MSK);
    L2REG_WRITE(E500_L2CTL, uiL2CtrlVal);
}
/*********************************************************************************************************
** 函数名称: ppcQorIQL2CacheDisable
** 功能描述: L2 CACHE 禁能
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  ppcQorIQL2CacheIsEnable (VOID)
{
    UINT32  uiL2CtrlVal = L2REG_READ(E500_L2CTL);

    return  ((uiL2CtrlVal & E500_L2CTL_L2E_MSK) ? (LW_TRUE) : (LW_FALSE));
}
/*********************************************************************************************************
** 函数名称: ppcQorIQL2CacheInvalidateAll
** 功能描述: L2 CACHE 无效
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcQorIQL2CacheInvalidateAll (VOID)
{
    volatile UINT32  uiL2CtrlVal;

    uiL2CtrlVal  = L2REG_READ(E500_L2CTL);
    uiL2CtrlVal |= E500_L2CTL_L2I_MSK;                                  /*  无效整个 L2 CACHE           */
    L2REG_WRITE(E500_L2CTL, uiL2CtrlVal);
    while (L2REG_READ(E500_L2CTL) & E500_L2CTL_L2I_MSK) {
    }
}
/*********************************************************************************************************
** 函数名称: ppcQorIQL2CacheInit
** 功能描述: 初始化 L2 CACHE 控制器
** 输　入  : pl2cdrv            驱动结构
**           uiInstruction      指令 CACHE 类型
**           uiData             数据 CACHE 类型
**           pcMachineName      机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ppcQorIQL2CacheInit (L2C_DRVIER  *pl2cdrv,
                           CACHE_MODE   uiInstruction,
                           CACHE_MODE   uiData,
                           CPCHAR       pcMachineName)
{
    if (_G_l2Config.CFG_bPresent) {
        INT  i;

        _DebugFormat(__LOGMESSAGE_LEVEL, "%s %s L2 cache controller initialization.\r\n",
                     LW_CFG_CPU_ARCH_FAMILY, "QorIQ");

        pl2cdrv->L2CD_pcName             = "QorIQ";
        pl2cdrv->L2CD_stSize             = _G_l2Config.CFG_stSize;
        pl2cdrv->L2CD_pfuncEnable        = ppcQorIQL2CacheEnable;
        pl2cdrv->L2CD_pfuncDisable       = ppcQorIQL2CacheDisable;
        pl2cdrv->L2CD_pfuncIsEnable      = ppcQorIQL2CacheIsEnable;
        pl2cdrv->L2CD_pfuncSync          = LW_NULL;
        pl2cdrv->L2CD_pfuncFlush         = LW_NULL;
        pl2cdrv->L2CD_pfuncFlushAll      = LW_NULL;
        pl2cdrv->L2CD_pfuncInvalidate    = LW_NULL;
        pl2cdrv->L2CD_pfuncInvalidateAll = LW_NULL;
        pl2cdrv->L2CD_pfuncClear         = LW_NULL;
        pl2cdrv->L2CD_pfuncClearAll      = LW_NULL;

        /*
         * Initialize L2CTL register to reset value
         */
        L2REG_WRITE(E500_L2CTL, 0);

        PPC_E500_DCACHE_FLUSH_NUM = (_G_l2Config.CFG_stSize * 3) / (2 * PPC_E500_DCACHE_ALIGN_SIZE);

        /*
         * Clean external write region registers
         */
        for (i = 0; i < E500_L2_EXTWR_REGION_NUM; i++) {
            L2REG_WRITE(E500_L2CEWARn(i), 0);
            L2REG_WRITE(E500_L2CEWCRn(i), 0);
        }

        ppcQorIQL2CacheInvalidateAll();
    }
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
                                                                        /*  LW_CFG_PPC_CACHE_L2 > 0     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
