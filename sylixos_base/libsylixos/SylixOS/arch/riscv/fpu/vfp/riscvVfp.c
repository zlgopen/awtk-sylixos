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
** 文   件   名: riscvVfp.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: RISC-V 体系架构 FPU 支持.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_FPU_EN > 0
#include "../riscvFpu.h"
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static RISCV_FPU_OP     _G_fpuopVfp;
/*********************************************************************************************************
  实现函数
*********************************************************************************************************/
extern VOID     riscvVfpEnable(VOID);
extern VOID     riscvVfpDisable(VOID);
extern BOOL     riscvVfpIsEnable(VOID);

extern VOID     riscvVfpSaveSp(PVOID  pvFpuCtx);
extern VOID     riscvVfpRestoreSp(PVOID  pvFpuCtx);

extern VOID     riscvVfpSaveDp(PVOID  pvFpuCtx);
extern VOID     riscvVfpRestoreDp(PVOID  pvFpuCtx);

extern VOID     riscvVfpSaveQp(PVOID  pvFpuCtx);
extern VOID     riscvVfpRestoreQp(PVOID  pvFpuCtx);
/*********************************************************************************************************
** 函数名称: riscvVfpCtxShowSp
** 功能描述: 显示 Single-Precision VFP 上下文
** 输　入  : iFd       输出文件描述符
**           pvFpuCtx  VFP 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  riscvVfpCtxShowSp (INT  iFd, PVOID  pvFpuCtx)
{
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)
    LW_FPU_CONTEXT  *pfpuCtx    = (LW_FPU_CONTEXT *)pvFpuCtx;
    ARCH_FPU_CTX    *pcpufpuCtx = &pfpuCtx->FPUCTX_fpuctxContext;
    INT              i;

    fdprintf(iFd, "FCSR = 0x%08x\n", pcpufpuCtx->f.FPUCTX_uiFcsr);

    for (i = 0; i < FPU_REG_NR; i++) {
        fdprintf(iFd, "FP%02d = 0x%08x\n", i, pcpufpuCtx->f.FPUCTX_reg[i]);
    }
#endif
}
/*********************************************************************************************************
** 函数名称: riscvVfpCtxShowDp
** 功能描述: 显示 Double-Precision VFP 上下文
** 输　入  : iFd       输出文件描述符
**           pvFpuCtx  VFP 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  riscvVfpCtxShowDp (INT  iFd, PVOID  pvFpuCtx)
{
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)
    LW_FPU_CONTEXT  *pfpuCtx    = (LW_FPU_CONTEXT *)pvFpuCtx;
    ARCH_FPU_CTX    *pcpufpuCtx = &pfpuCtx->FPUCTX_fpuctxContext;
    INT              i;

    fdprintf(iFd, "FCSR = 0x%08x\n", pcpufpuCtx->d.FPUCTX_uiFcsr);

    for (i = 0; i < FPU_REG_NR; i++) {
        fdprintf(iFd, "FP%02d = 0x%016lx\n", i, pcpufpuCtx->d.FPUCTX_reg[i]);
    }
#endif
}
/*********************************************************************************************************
** 函数名称: riscvVfpCtxShowQp
** 功能描述: 显示 Quad-Precision VFP 上下文
** 输　入  : iFd       输出文件描述符
**           pvFpuCtx  VFP 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  riscvVfpCtxShowQp (INT  iFd, PVOID  pvFpuCtx)
{
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_FIO_LIB_EN > 0)
    LW_FPU_CONTEXT  *pfpuCtx    = (LW_FPU_CONTEXT *)pvFpuCtx;
    ARCH_FPU_CTX    *pcpufpuCtx = &pfpuCtx->FPUCTX_fpuctxContext;
    INT              i;

    fdprintf(iFd, "FCSR = 0x%08x\n", pcpufpuCtx->q.FPUCTX_uiFcsr);

    for (i = 0; i < (FPU_REG_NR * 2); i += 2) {
        fdprintf(iFd, "FP%02d = 0x%016lx%016lx\n", i,
                 pcpufpuCtx->q.FPUCTX_reg[i], pcpufpuCtx->q.FPUCTX_reg[i + 1]);
    }
#endif
}
/*********************************************************************************************************
** 函数名称: riscvVfpEnableTask
** 功能描述: 系统发生 undef 异常时, 使能任务的 VFP
** 输　入  : ptcbCur    当前任务控制块
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  riscvVfpEnableTask (PLW_CLASS_TCB  ptcbCur)
{
    ARCH_REG_CTX  *pregctx;

    pregctx = &ptcbCur->TCB_archRegCtx;
    pregctx->REG_ulStatus |= SSTATUS_FS;
}
/*********************************************************************************************************
** 函数名称: riscvVfpPrimaryInit
** 功能描述: 初始化并获取 VFP 控制器操作函数集
** 输　入  : pcMachineName 机器名
**           pcFpuName     浮点运算器名
** 输　出  : 操作函数集
** 全局变量:
** 调用模块:
*********************************************************************************************************/
PRISCV_FPU_OP  riscvVfpPrimaryInit (CPCHAR  pcMachineName, CPCHAR  pcFpuName)
{
    if (lib_strcmp(pcFpuName, RISCV_FPU_SP) == 0) {
        _G_fpuopVfp.SFPU_pfuncSave    = riscvVfpSaveSp;
        _G_fpuopVfp.SFPU_pfuncRestore = riscvVfpRestoreSp;
        _G_fpuopVfp.SFPU_pfuncCtxShow = riscvVfpCtxShowSp;

    } else if (lib_strcmp(pcFpuName, RISCV_FPU_DP) == 0) {
        _G_fpuopVfp.SFPU_pfuncSave    = riscvVfpSaveDp;
        _G_fpuopVfp.SFPU_pfuncRestore = riscvVfpRestoreDp;
        _G_fpuopVfp.SFPU_pfuncCtxShow = riscvVfpCtxShowDp;

    } else {
        _G_fpuopVfp.SFPU_pfuncSave    = riscvVfpSaveQp;
        _G_fpuopVfp.SFPU_pfuncRestore = riscvVfpRestoreQp;
        _G_fpuopVfp.SFPU_pfuncCtxShow = riscvVfpCtxShowQp;
    }

    _G_fpuopVfp.SFPU_pfuncEnable     = riscvVfpEnable;
    _G_fpuopVfp.SFPU_pfuncDisable    = riscvVfpDisable;
    _G_fpuopVfp.SFPU_pfuncIsEnable   = riscvVfpIsEnable;
    _G_fpuopVfp.SFPU_pfuncEnableTask = riscvVfpEnableTask;

    return  (&_G_fpuopVfp);
}
/*********************************************************************************************************
** 函数名称: riscvVfpSecondaryInit
** 功能描述: 初始化 VFP 控制器
** 输　入  : pcMachineName 机器名
**           pcFpuName     浮点运算器名
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  riscvVfpSecondaryInit (CPCHAR  pcMachineName, CPCHAR  pcFpuName)
{
    (VOID)pcMachineName;
    (VOID)pcFpuName;
}

#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
