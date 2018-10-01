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
** 描        述: MIPS 体系架构 DSP.
*********************************************************************************************************/

#ifndef __ARCH_MIPSDSP_H
#define __ARCH_MIPSDSP_H

/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_DSP_EN > 0

/*********************************************************************************************************
  MIPS dsp 操作函数
*********************************************************************************************************/

typedef struct {
    VOIDFUNCPTR     MDSP_pfuncEnable;
    VOIDFUNCPTR     MDSP_pfuncDisable;
    BOOLFUNCPTR     MDSP_pfuncIsEnable;
    VOIDFUNCPTR     MDSP_pfuncSave;
    VOIDFUNCPTR     MDSP_pfuncRestore;
    VOIDFUNCPTR     MDSP_pfuncCtxShow;
    VOIDFUNCPTR     MDSP_pfuncEnableTask;
} MIPS_DSP_OP;
typedef MIPS_DSP_OP *PMIPS_DSP_OP;

/*********************************************************************************************************
  MIPS dsp 基本操作
*********************************************************************************************************/

#define MIPS_DSP_ENABLE(op)              op->MDSP_pfuncEnable()
#define MIPS_DSP_DISABLE(op)             op->MDSP_pfuncDisable()
#define MIPS_DSP_ISENABLE(op)            op->MDSP_pfuncIsEnable()
#define MIPS_DSP_SAVE(op, ctx)           op->MDSP_pfuncSave((ctx))
#define MIPS_DSP_RESTORE(op, ctx)        op->MDSP_pfuncRestore((ctx))
#define MIPS_DSP_CTXSHOW(op, fd, ctx)    op->MDSP_pfuncCtxShow((fd), (ctx))
#define MIPS_DSP_ENABLE_TASK(op, tcb)    op->MDSP_pfuncEnableTask((tcb))

#endif                                                                  /*  LW_CFG_CPU_DSP_EN > 0       */
#endif                                                                  /*  __ARCH_MIPSDSP_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
