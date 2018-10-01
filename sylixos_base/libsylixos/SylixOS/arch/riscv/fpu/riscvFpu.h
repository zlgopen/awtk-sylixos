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
** 文   件   名: riscvFpu.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: RISC-V 体系架构硬件浮点运算器 (VFP).
*********************************************************************************************************/

#ifndef __ARCH_RISCV_FPU_H
#define __ARCH_RISCV_FPU_H

/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_CPU_FPU_EN > 0

/*********************************************************************************************************
  RISC-V fpu 操作函数
*********************************************************************************************************/

typedef struct {
    ULONGFUNCPTR    SFPU_pfuncHwSid;
    VOIDFUNCPTR     SFPU_pfuncEnable;
    VOIDFUNCPTR     SFPU_pfuncDisable;
    BOOLFUNCPTR     SFPU_pfuncIsEnable;
    VOIDFUNCPTR     SFPU_pfuncSave;
    VOIDFUNCPTR     SFPU_pfuncRestore;
    VOIDFUNCPTR     SFPU_pfuncCtxShow;
    VOIDFUNCPTR     SFPU_pfuncEnableTask;
} RISCV_FPU_OP;
typedef RISCV_FPU_OP *PRISCV_FPU_OP;

/*********************************************************************************************************
  RISC-V fpu 基本操作
*********************************************************************************************************/

#define RISCV_VFP_ENABLE(op)              op->SFPU_pfuncEnable()
#define RISCV_VFP_DISABLE(op)             op->SFPU_pfuncDisable()
#define RISCV_VFP_ISENABLE(op)            op->SFPU_pfuncIsEnable()
#define RISCV_VFP_SAVE(op, ctx)           op->SFPU_pfuncSave((ctx))
#define RISCV_VFP_RESTORE(op, ctx)        op->SFPU_pfuncRestore((ctx))
#define RISCV_VFP_CTXSHOW(op, fd, ctx)    op->SFPU_pfuncCtxShow((fd), (ctx))
#define RISCV_VFP_ENABLE_TASK(op, tcb)    op->SFPU_pfuncEnableTask((tcb))

#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
#endif                                                                  /*  __ARCH_RISCV_FPU_H          */
/*********************************************************************************************************
  END
*********************************************************************************************************/
