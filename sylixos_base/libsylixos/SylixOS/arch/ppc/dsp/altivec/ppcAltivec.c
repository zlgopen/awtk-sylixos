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
** 文   件   名: ppcAltivec.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 05 月 04 日
**
** 描        述: PowerPC 体系架构 ALTIVEC 支持.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_DSP_EN > 0
#include "../ppcDsp.h"
#define  __SYLIXOS_PPC_HAVE_ALTIVEC 1
#include "arch/ppc/arch_604.h"
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static PPC_DSP_OP   _G_dspopAltivec;
/*********************************************************************************************************
  实现函数
*********************************************************************************************************/
extern VOID     ppcAltivecEnable(VOID);
extern VOID     ppcAltivecDisable(VOID);
extern BOOL     ppcAltivecIsEnable(VOID);
extern VOID     ppcAltivecSave(PVOID  pvDspCtx);
extern VOID     ppcAltivecRestore(PVOID  pvDspCtx);
/*********************************************************************************************************
  VSCR 相关定义
*********************************************************************************************************/
#define ALTIVEC_VSCR_CONFIG_WORD    3                                   /*  VSCR word with below bits   */
#define ALTIVEC_VSCR_NJ             0x00010000                          /*  Non-Java mode               */
#define ALTIVEC_VSCR_SAT            0x00000001                          /*  Vector Saturation           */
/*********************************************************************************************************
** 函数名称: ppcAltivecCtxShow
** 功能描述: 显示 ALTIVEC 上下文
** 输　入  : iFd       输出文件描述符
**           pvDspCtx  ALTIVEC 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcAltivecCtxShow (INT  iFd, PVOID  pvDspCtx)
{
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)
    LW_DSP_CONTEXT *pdspctx    = (LW_DSP_CONTEXT *)pvDspCtx;
    ARCH_DSP_CTX   *pcpudspctx = &pdspctx->DSPCTX_dspctxContext;
    UINT32         *puiValue;
    INT             i;

    fdprintf(iFd, "VRSAVE = 0x%08x VSCR = 0x%08x\n",
             pcpudspctx->VECCTX_uiVrsave,
             pcpudspctx->VECCTX_uiVscr[ALTIVEC_VSCR_CONFIG_WORD]);

    for (i = 0; i < ALTIVEC_REG_NR; i++) {
        puiValue = (UINT32 *)&pcpudspctx->VECCTX_regs[i];
        fdprintf(iFd, "AR%02d: 0x%08x_0x%08x_0x%08x_0x%08x\n",
                 i,
                 puiValue[0], puiValue[1], puiValue[2], puiValue[3]);
    }
#endif
}
/*********************************************************************************************************
** 函数名称: ppcAltivecEnableTask
** 功能描述: 系统发生 undef 异常时, 使能任务的 ALTIVEC
** 输　入  : ptcbCur    当前任务控制块
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ppcAltivecEnableTask (PLW_CLASS_TCB  ptcbCur)
{
    ARCH_REG_CTX  *pregctx;
    ARCH_DSP_CTX  *pdspctx;

    pregctx = &ptcbCur->TCB_archRegCtx;
    pregctx->REG_uiSrr1 |= ARCH_PPC_MSR_VEC;

    pdspctx = &ptcbCur->TCB_dspctxContext.DSPCTX_dspctxContext;
    pdspctx->VECCTX_uiVrsave = 0ul;
    pdspctx->VECCTX_uiVscr[ALTIVEC_VSCR_CONFIG_WORD] = 0ul;
}
/*********************************************************************************************************
** 函数名称: ppcAltivecPrimaryInit
** 功能描述: 获取 ALTIVEC 控制器操作函数集
** 输　入  : pcMachineName 机器名
**           pcDspName     DSP 运算器名
** 输　出  : 操作函数集
** 全局变量:
** 调用模块:
*********************************************************************************************************/
PPPC_DSP_OP  ppcAltivecPrimaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    _G_dspopAltivec.PDSP_pfuncEnable     = ppcAltivecEnable;
    _G_dspopAltivec.PDSP_pfuncDisable    = ppcAltivecDisable;
    _G_dspopAltivec.PDSP_pfuncIsEnable   = ppcAltivecIsEnable;
    _G_dspopAltivec.PDSP_pfuncSave       = ppcAltivecSave;
    _G_dspopAltivec.PDSP_pfuncRestore    = ppcAltivecRestore;
    _G_dspopAltivec.PDSP_pfuncCtxShow    = ppcAltivecCtxShow;
    _G_dspopAltivec.PDSP_pfuncEnableTask = ppcAltivecEnableTask;

    return  (&_G_dspopAltivec);
}
/*********************************************************************************************************
** 函数名称: ppcAltivecSecondaryInit
** 功能描述: 初始化 ALTIVEC 控制器
** 输　入  : pcMachineName 机器名
**           pcDspName     DSP 运算器名
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ppcAltivecSecondaryInit (CPCHAR  pcMachineName, CPCHAR  pcDspName)
{
    (VOID)pcMachineName;
    (VOID)pcDspName;
}

#endif                                                                  /*  LW_CFG_CPU_DSP_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
