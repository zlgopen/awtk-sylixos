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
** 文   件   名: ppcL2CacheE500mc.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 08 月 07 日
**
** 描        述: E500MC 体系构架 L2 CACHE 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁减配置
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0 && LW_CFG_PPC_CACHE_L2 > 0
#include "../ppcL2.h"
/*********************************************************************************************************
  L2CSR0 BIT MASKS and BIT SHIFTS
*********************************************************************************************************/
#define L2CSR0_L2E          0x80000000                              /*  L2 Cache Enable                 */
#define L2CSR0_L2PE         0x40000000                              /*  L2 Cache Parity/ECC Enable      */
#define L2CSR0_L2WP         0x1c000000                              /*  L2 I/D Way Partioning           */
#define L2CSR0_L2CM         0x03000000                              /*  L2 Cache Coherency Mode         */
#define L2CSR0_L2FI         0x00200000                              /*  L2 Cache Flash Invalidate       */
#define L2CSR0_L2IO         0x00100000                              /*  L2 Cache Instruction Only       */
#define L2CSR0_L2DO         0x00010000                              /*  L2 Cache Data Only              */
#define L2CSR0_L2REP        0x00003000                              /*  L2 Line Replacement Algo        */
#define L2CSR0_L2FL         0x00000800                              /*  L2 Cache Flush                  */
#define L2CSR0_L2LFC        0x00000400                              /*  L2 Cache Lock Flash Clear       */
#define L2CSR0_L2LOA        0x00000080                              /*  L2 Cache Lock Overflow Allocate */
#define L2CSR0_L2LO         0x00000020                              /*  L2 Cache Lock Overflow          */

#define L2CSR0_L2E_BIT      0                                       /*  L2 Enable                       */
#define L2CSR0_L2PE_BIT     1                                       /*  L2 Parity Enable                */
#define L2CSR0_L2WP_BIT     3                                       /*  L2 Way Partitioning (3)         */
#define L2CSR0_L2CM_BIT     6                                       /*  L2 Coherency Mode (2)           */
#define L2CSR0_L2FI_BIT     10                                      /*  L2 Flash Invalidate             */
#define L2CSR0_L2IO_BIT     11                                      /*  L2 Instruction Only             */
#define L2CSR0_L2DO_BIT     15                                      /*  L2 Data Only                    */
#define L2CSR0_L2REP_BIT    18                                      /*  L2 Line Replcmnt Algo. (2)      */
#define L2CSR0_L2FL_BIT     20                                      /*  L2 Flush                        */
#define L2CSR0_L2LFC_BIT    21                                      /*  L2 Lock Flush Clear             */
#define L2CSR0_L2LOA_BIT    24                                      /*  L2 Lock Overflow Allocate       */
#define L2CSR0_L2LO_BIT     26                                      /*  L2 Lock Overflow                */

#define L2CSR0_L2E_MSK      (1U << (31 - L2CSR0_L2E_BIT))
#define L2CSR0_L2PE_MSK     (1U << (31 - L2CSR0_L2PE_BIT))
#define L2CSR0_L2WP_MSK     (7U << (31 - L2CSR0_L2WP_BIT - 2))
#define L2CSR0_L2CM_MSK     (3U << (31 - L2CSR0_L2CM_BIT - 1))
#define L2CSR0_L2FI_MSK     (1U << (31 - L2CSR0_L2FI_BIT))
#define L2CSR0_L2IO_MSK     (1U << (31 - L2CSR0_L2IO_BIT))
#define L2CSR0_L2DO_MSK     (1U << (31 - L2CSR0_L2DO_BIT))
#define L2CSR0_L2REP_MSK    (3U << (31 - L2CSR0_L2REP_BIT))
#define L2CSR0_L2FL_MSK     (1U << (31 - L2CSR0_L2FL_BIT))
#define L2CSR0_L2LFC_MSK    (1U << (31 - L2CSR0_L2LFC_BIT))
#define L2CSR0_L2LOA_MSK    (1U << (31 - L2CSR0_L2LOA_BIT))
#define L2CSR0_L2LO_MSK     (1U << (31 - L2CSR0_L2LO_BIT))

#define L2CFG0_L2CSIZE_MSK  0x3fff
/*********************************************************************************************************
  外部函数声明
*********************************************************************************************************/
extern UINT32  ppcE500mcGetL2Csr0(VOID);
extern VOID    ppcE500mcSetL2Csr0(UINT32);
/*********************************************************************************************************
  外部变量声明
*********************************************************************************************************/
extern UINT32  PPC_E500_DCACHE_ALIGN_SIZE;
extern UINT32  PPC_E500_DCACHE_FLUSH_NUM;
/*********************************************************************************************************
** 函数名称: ppcE500mcL2CacheEnable
** 功能描述: L2 CACHE 使能
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcE500mcL2CacheEnable (VOID)
{
    UINT32  uiL2Csr0Val = ppcE500mcGetL2Csr0();

    /*
     * Check whether the L2 cache had been enabled, if it not, then
     * flash invalidate and lock flash clear it. If enabled, then skip.
     */
    if ((uiL2Csr0Val & L2CSR0_L2E_MSK) == 0) {
        uiL2Csr0Val |= (L2CSR0_L2FI_MSK | L2CSR0_L2LFC_MSK);

        do {
            uiL2Csr0Val = ppcE500mcGetL2Csr0();
        } while (uiL2Csr0Val & (L2CSR0_L2FI_MSK | L2CSR0_L2LFC_MSK));

        uiL2Csr0Val |= L2CSR0_L2E_MSK;
    }

    ppcE500mcSetL2Csr0(uiL2Csr0Val);
}
/*********************************************************************************************************
** 函数名称: ppcE500mcL2CacheDisable
** 功能描述: L2 CACHE 禁能
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcE500mcL2CacheDisable (VOID)
{
    UINT32  uiL2Csr0Val = ppcE500mcGetL2Csr0();

    /*
     * only handle if the L2 has been enabled
     */
    if (uiL2Csr0Val & L2CSR0_L2E_MSK) {
        ppcE500mcSetL2Csr0(uiL2Csr0Val | L2CSR0_L2FL_MSK);

        /*
         * wait until complete
         */
        while (ppcE500mcGetL2Csr0() & L2CSR0_L2FL_MSK) {
        }

        uiL2Csr0Val  = ppcE500mcGetL2Csr0();
        uiL2Csr0Val &= ~((UINT32)(L2CSR0_L2E_MSK | L2CSR0_L2PE_MSK));
        ppcE500mcSetL2Csr0(uiL2Csr0Val);
    }
}
/*********************************************************************************************************
** 函数名称: ppcE500mcL2CacheDisable
** 功能描述: L2 CACHE 禁能
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  ppcE500mcL2CacheIsEnable (VOID)
{
    UINT32  uiL2Csr0Val = ppcE500mcGetL2Csr0();

    return  ((uiL2Csr0Val & L2CSR0_L2E_MSK) ? (LW_TRUE) : (LW_FALSE));
}
/*********************************************************************************************************
** 函数名称: ppcL2CacheE500mcInit
** 功能描述: 初始化 L2 CACHE 控制器
** 输　入  : pl2cdrv            驱动结构
**           uiInstruction      指令 CACHE 类型
**           uiData             数据 CACHE 类型
**           pcMachineName      机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ppcE500mcL2CacheInit (L2C_DRVIER  *pl2cdrv,
                            CACHE_MODE   uiInstruction,
                            CACHE_MODE   uiData,
                            CPCHAR       pcMachineName)
{
    UINT  uil2CacheSize;

    _DebugFormat(__LOGMESSAGE_LEVEL, "%s %s L2 cache controller initialization.\r\n",
                 LW_CFG_CPU_ARCH_FAMILY, "E500MC");

    pl2cdrv->L2CD_pcName             = "E500MC";
    pl2cdrv->L2CD_stSize             = 128 * LW_CFG_KB_SIZE;
    pl2cdrv->L2CD_pfuncEnable        = ppcE500mcL2CacheEnable;
    pl2cdrv->L2CD_pfuncDisable       = ppcE500mcL2CacheDisable;
    pl2cdrv->L2CD_pfuncIsEnable      = ppcE500mcL2CacheIsEnable;
    pl2cdrv->L2CD_pfuncSync          = LW_NULL;
    pl2cdrv->L2CD_pfuncFlush         = LW_NULL;
    pl2cdrv->L2CD_pfuncFlushAll      = LW_NULL;
    pl2cdrv->L2CD_pfuncInvalidate    = LW_NULL;
    pl2cdrv->L2CD_pfuncInvalidateAll = LW_NULL;
    pl2cdrv->L2CD_pfuncClear         = LW_NULL;
    pl2cdrv->L2CD_pfuncClearAll      = LW_NULL;

    uil2CacheSize = (ppcE500mcGetL2Csr0() & L2CFG0_L2CSIZE_MSK) * 0x10000;

    PPC_E500_DCACHE_FLUSH_NUM = (uil2CacheSize * 3) / (2 * PPC_E500_DCACHE_ALIGN_SIZE);
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
                                                                        /*  LW_CFG_PPC_CACHE_L2 > 0     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
