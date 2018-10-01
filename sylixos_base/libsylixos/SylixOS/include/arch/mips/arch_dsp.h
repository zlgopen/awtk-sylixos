/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: arch_dsp.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 01 月 10 日
**
** 描        述: MIPS DSP 相关.
*********************************************************************************************************/

#ifndef __MIPS_ARCH_DSP_H
#define __MIPS_ARCH_DSP_H

#if defined(__SYLIXOS_KERNEL) || defined(__ASSEMBLY__) || defined(ASSEMBLY)

/*********************************************************************************************************
  汇编代码不包含以下内容
*********************************************************************************************************/

#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))

/*********************************************************************************************************
  华睿 2 号向量运算单元上下文
*********************************************************************************************************/

#define HR2_VECTOR_REG_NR               32                              /*  HR2 向量数据寄存器的总数    */
#define HR2_VECTOR_REG_WIDTH            256                             /*  HR2 向量数据寄存器的位宽    */
#define HR2_VECTOR_REG_SIZE             (HR2_VECTOR_REG_WIDTH / 8)      /*  HR2 向量数据寄存器的大小    */
#define HR2_VECTOR_CTX_ALIGN            HR2_VECTOR_REG_SIZE             /*  HR2 向量上下文对齐大小      */

typedef union hr2_vector_reg {                                          /*  HR2 向量数据寄存器类型      */
    UINT32              val32[HR2_VECTOR_REG_WIDTH / 32];
    UINT64              val64[HR2_VECTOR_REG_WIDTH / 64];
} HR2_VECTOR_REG;

typedef struct hr2_vector_ctx {                                         /*  HR2 向量运算单元上下文      */
    HR2_VECTOR_REG      HR2VECCTX_vectorRegs[HR2_VECTOR_REG_NR];        /*  HR2 向量数据寄存器          */
    UINT32              HR2VECCTX_uiVccr;                               /*  HR2 向量目的寄存器          */
    UINT32              HR2VECCTX_uiPad;
} HR2_VECTOR_CTX;

/*********************************************************************************************************
  MIPS 标准 DSP 上下文
*********************************************************************************************************/

#define MIPS_DSP_REGS_NR                6                               /*  MIPS DSP 数据寄存器的总数   */

typedef UINT32  MIPS_DSP_REG;                                           /*  MIPS DSP 数据寄存器类型     */

typedef struct mips_dsp_ctx {                                           /*  MIPS 标准 DSP 上下文        */
    MIPS_DSP_REG        MIPSDSPCTX_dspRegs[MIPS_DSP_REGS_NR];           /*  MIPS DSP 数据寄存器         */
    UINT32              MIPSDSPCTX_dspCtrl;                             /*  MIPS DSP 控制寄存器         */
} MIPS_DSP_CTX;

/*********************************************************************************************************
  DSP 上下文
*********************************************************************************************************/

#define ARCH_DSP_CTX_ALIGN              HR2_VECTOR_CTX_ALIGN            /*  DSP 上下文对齐大小          */

union arch_dsp_ctx {                                                    /*  DSP 上下文                  */
    MIPS_DSP_CTX        DSPCTX_dspCtx;
    HR2_VECTOR_CTX      DSPCTX_hr2VectorCtx;
} __attribute__ ((aligned(ARCH_DSP_CTX_ALIGN)));

typedef union arch_dsp_ctx ARCH_DSP_CTX;                                /*  DSP 上下文                  */

#endif                                                                  /*  __ASSEMBLY__                */
#endif                                                                  /*  __SYLIXOS_KERNEL            */
#endif                                                                  /*  __MIPS_ARCH_DSP_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
