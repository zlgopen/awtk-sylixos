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
** 文   件   名: mipsFpu.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 11 月 17 日
**
** 描        述: MIPS 体系架构硬件浮点运算器 (VFP).
*********************************************************************************************************/

#ifndef __ARCH_MIPSFPU_H
#define __ARCH_MIPSFPU_H

/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_FPU_EN > 0

/*********************************************************************************************************
  MIPS fpu 操作函数
*********************************************************************************************************/

typedef struct {
    VOIDFUNCPTR     MFPU_pfuncEnable;
    VOIDFUNCPTR     MFPU_pfuncDisable;
    BOOLFUNCPTR     MFPU_pfuncIsEnable;
    VOIDFUNCPTR     MFPU_pfuncSave;
    VOIDFUNCPTR     MFPU_pfuncRestore;
    VOIDFUNCPTR     MFPU_pfuncCtxShow;
    ULONGFUNCPTR    MFPU_pfuncGetFIR;
    VOIDFUNCPTR     MFPU_pfuncEnableTask;
} MIPS_FPU_OP;
typedef MIPS_FPU_OP *PMIPS_FPU_OP;

/*********************************************************************************************************
  MIPS fpu 基本操作
*********************************************************************************************************/

#define MIPS_VFP_ENABLE(op)              op->MFPU_pfuncEnable()
#define MIPS_VFP_DISABLE(op)             op->MFPU_pfuncDisable()
#define MIPS_VFP_ISENABLE(op)            op->MFPU_pfuncIsEnable()
#define MIPS_VFP_SAVE(op, ctx)           op->MFPU_pfuncSave((ctx))
#define MIPS_VFP_RESTORE(op, ctx)        op->MFPU_pfuncRestore((ctx))
#define MIPS_VFP_CTXSHOW(op, fd, ctx)    op->MFPU_pfuncCtxShow((fd), (ctx))
#define MIPS_VFP_GETFIR(op)              op->MFPU_pfuncGetFIR()
#define MIPS_VFP_ENABLE_TASK(op, tcb)    op->MFPU_pfuncEnableTask((tcb))

#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
#endif                                                                  /*  __ARCH_MIPSFPU_H            */
/*********************************************************************************************************
  END
*********************************************************************************************************/
