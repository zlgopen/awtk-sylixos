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
** 文   件   名: mipsDsp.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 01 月 10 日
**
** 描        述: PowerPC 体系架构 DSP.
*********************************************************************************************************/

#ifndef __ARCH_PPCDSP_H
#define __ARCH_PPCDSP_H

/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_DSP_EN > 0

/*********************************************************************************************************
  PowerPC dsp 操作函数
*********************************************************************************************************/

typedef struct {
    VOIDFUNCPTR     PDSP_pfuncEnable;
    VOIDFUNCPTR     PDSP_pfuncDisable;
    BOOLFUNCPTR     PDSP_pfuncIsEnable;
    VOIDFUNCPTR     PDSP_pfuncSave;
    VOIDFUNCPTR     PDSP_pfuncRestore;
    VOIDFUNCPTR     PDSP_pfuncCtxShow;
    VOIDFUNCPTR     PDSP_pfuncEnableTask;
} PPC_DSP_OP;
typedef PPC_DSP_OP *PPPC_DSP_OP;

/*********************************************************************************************************
  PowerPC dsp 基本操作
*********************************************************************************************************/

#define PPC_DSP_ENABLE(op)              op->PDSP_pfuncEnable()
#define PPC_DSP_DISABLE(op)             op->PDSP_pfuncDisable()
#define PPC_DSP_ISENABLE(op)            op->PDSP_pfuncIsEnable()
#define PPC_DSP_SAVE(op, ctx)           op->PDSP_pfuncSave((ctx))
#define PPC_DSP_RESTORE(op, ctx)        op->PDSP_pfuncRestore((ctx))
#define PPC_DSP_CTXSHOW(op, fd, ctx)    op->PDSP_pfuncCtxShow((fd), (ctx))
#define PPC_DSP_ENABLE_TASK(op, tcb)    op->PDSP_pfuncEnableTask((tcb))

#endif                                                                  /*  LW_CFG_CPU_DSP_EN > 0       */
#endif                                                                  /*  __ARCH_PPCDSP_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
