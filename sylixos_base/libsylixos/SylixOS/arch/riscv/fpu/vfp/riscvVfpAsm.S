;/*********************************************************************************************************
;**
;**                                    中国软件开源组织
;**
;**                                   嵌入式实时操作系统
;**
;**                                       SylixOS(TM)
;**
;**                               Copyright  All Rights Reserved
;**
;**--------------文件信息--------------------------------------------------------------------------------
;**
;** 文   件   名: riscvVfpAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2018 年 03 月 20 日
;**
;** 描        述: RISC-V 体系架构 FPU 支持.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>

#if LW_CFG_CPU_FPU_EN > 0
#include "arch/riscv/arch_float.h"

    FILE_BEGIN()

    EXPORT_LABEL(riscvVfpEnable)
    EXPORT_LABEL(riscvVfpDisable)
    EXPORT_LABEL(riscvVfpIsEnable)
    EXPORT_LABEL(riscvVfpSaveSp)
    EXPORT_LABEL(riscvVfpRestoreSp)
    EXPORT_LABEL(riscvVfpSaveDp)
    EXPORT_LABEL(riscvVfpRestoreDp)
    EXPORT_LABEL(riscvVfpSaveQp)
    EXPORT_LABEL(riscvVfpRestoreQp)

;/*********************************************************************************************************
;  使能 FPU 宏
;*********************************************************************************************************/

MACRO_DEF(ENABLE_VFP)
    LI      T1 , XSTATUS_FS
    CSRS    XSTATUS , T1
    MACRO_END()

;/*********************************************************************************************************
;  使能 FPU
;*********************************************************************************************************/

FUNC_DEF(riscvVfpEnable)
    ENABLE_VFP
    RET
    FUNC_END(riscvVfpEnable)

;/*********************************************************************************************************
;  关闭 FPU
;*********************************************************************************************************/

FUNC_DEF(riscvVfpDisable)
    LI      T1 , XSTATUS_FS
    CSRC    XSTATUS , T1
    RET
    FUNC_END(riscvVfpDisable)

;/*********************************************************************************************************
;  判断 FPU 是否使能
;*********************************************************************************************************/

FUNC_DEF(riscvVfpIsEnable)
    LI      T1  , XSTATUS_FS
    CSRR    T0  , XSTATUS
    AND     T0  , T0 , T1
    SRL     RV0 , T0 , 14                                               ;/*  仅看第 14 位                */
    RET
    FUNC_END(riscvVfpIsEnable)

;/*********************************************************************************************************
;  FLD FSD 指令
;*********************************************************************************************************/

#define RISCV_FLD(rd, offset, rs1) \
    .word   ((offset << 20) | (rs1 << 15) | (0x3 << 12) | (rd << 7) | 0x7)

#define RISCV_FSD(rs2, offset, rs1) \
    .word   (((offset >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (0x3 << 12) | ((offset & 0x1f) << 7) | 0x27)

;/*********************************************************************************************************
;  FLW FSW 指令
;*********************************************************************************************************/

#define RISCV_FLW(rd, offset, rs1) \
    .word   ((offset << 20) | (rs1 << 15) | (0x2 << 12) | (rd << 7) | 0x7)

#define RISCV_FSW(rs2, offset, rs1) \
    .word   (((offset >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (0x2 << 12) | ((offset & 0x1f) << 7) | 0x27)

;/*********************************************************************************************************
;  FLQ FSQ 指令
;*********************************************************************************************************/

#define RISCV_FLQ(rd, offset, rs1) \
    .word   ((offset << 20) | (rs1 << 15) | (0x4 << 12) | (rd << 7) | 0x7)

#define RISCV_FSQ(rs2, offset, rs1) \
    .word   (((offset >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (0x4 << 12) | ((offset & 0x1f) << 7) | 0x27)

;/*********************************************************************************************************
;  FRCSR_T0 FSCSR_T0 指令
;*********************************************************************************************************/

#define RISCV_FRCSR_T0 \
    .word   0x003022f3

#define RISCV_FSCSR_T0 \
    .word   0x00329073

;/*********************************************************************************************************
;  A0 寄存器的指令编码
;*********************************************************************************************************/

#define RISCV_A0    10

;/*********************************************************************************************************
;  保存寄存器
;*********************************************************************************************************/

#define SP_FPU_REG_WIDTH           32                                    /*  浮点数据寄存器的位宽        */
#define SP_FPU_REG_SIZE            (SP_FPU_REG_WIDTH / 8)                /*  浮点数据寄存器的大小        */

/*********************************************************************************************************
  定义 ARCH_FPU_CTX FPU 成员偏移
*********************************************************************************************************/

#define SP_FPU_OFFSET_REG(n)       ((n) * SP_FPU_REG_SIZE)               /*  浮点数据寄存器偏移          */
#define SP_FPU_OFFSET_FCSR         (SP_FPU_OFFSET_REG(FPU_REG_NR))       /*  FCSR 偏移                   */

;/*********************************************************************************************************
;  保存寄存器
;*********************************************************************************************************/

FUNC_DEF(riscvVfpSaveSp)
    ENABLE_VFP

    RISCV_FRCSR_T0

    RISCV_FSW(0 , SP_FPU_OFFSET_REG(0 ), RISCV_A0)
    RISCV_FSW(1 , SP_FPU_OFFSET_REG(1 ), RISCV_A0)
    RISCV_FSW(2 , SP_FPU_OFFSET_REG(2 ), RISCV_A0)
    RISCV_FSW(3 , SP_FPU_OFFSET_REG(3 ), RISCV_A0)
    RISCV_FSW(4 , SP_FPU_OFFSET_REG(4 ), RISCV_A0)
    RISCV_FSW(5 , SP_FPU_OFFSET_REG(5 ), RISCV_A0)
    RISCV_FSW(6 , SP_FPU_OFFSET_REG(6 ), RISCV_A0)
    RISCV_FSW(7 , SP_FPU_OFFSET_REG(7 ), RISCV_A0)
    RISCV_FSW(8 , SP_FPU_OFFSET_REG(8 ), RISCV_A0)
    RISCV_FSW(9 , SP_FPU_OFFSET_REG(9 ), RISCV_A0)
    RISCV_FSW(10, SP_FPU_OFFSET_REG(10), RISCV_A0)
    RISCV_FSW(11, SP_FPU_OFFSET_REG(11), RISCV_A0)
    RISCV_FSW(12, SP_FPU_OFFSET_REG(12), RISCV_A0)
    RISCV_FSW(13, SP_FPU_OFFSET_REG(13), RISCV_A0)
    RISCV_FSW(14, SP_FPU_OFFSET_REG(14), RISCV_A0)
    RISCV_FSW(15, SP_FPU_OFFSET_REG(15), RISCV_A0)
    RISCV_FSW(16, SP_FPU_OFFSET_REG(16), RISCV_A0)
    RISCV_FSW(17, SP_FPU_OFFSET_REG(17), RISCV_A0)
    RISCV_FSW(18, SP_FPU_OFFSET_REG(18), RISCV_A0)
    RISCV_FSW(19, SP_FPU_OFFSET_REG(19), RISCV_A0)
    RISCV_FSW(20, SP_FPU_OFFSET_REG(20), RISCV_A0)
    RISCV_FSW(21, SP_FPU_OFFSET_REG(21), RISCV_A0)
    RISCV_FSW(22, SP_FPU_OFFSET_REG(22), RISCV_A0)
    RISCV_FSW(23, SP_FPU_OFFSET_REG(23), RISCV_A0)
    RISCV_FSW(24, SP_FPU_OFFSET_REG(24), RISCV_A0)
    RISCV_FSW(25, SP_FPU_OFFSET_REG(25), RISCV_A0)
    RISCV_FSW(26, SP_FPU_OFFSET_REG(26), RISCV_A0)
    RISCV_FSW(27, SP_FPU_OFFSET_REG(27), RISCV_A0)
    RISCV_FSW(28, SP_FPU_OFFSET_REG(28), RISCV_A0)
    RISCV_FSW(29, SP_FPU_OFFSET_REG(29), RISCV_A0)
    RISCV_FSW(30, SP_FPU_OFFSET_REG(30), RISCV_A0)
    RISCV_FSW(31, SP_FPU_OFFSET_REG(31), RISCV_A0)

    SW      T0  , SP_FPU_OFFSET_FCSR(A0)

    RET
    FUNC_END(riscvVfpSaveSp)

;/*********************************************************************************************************
;  恢复寄存器
;*********************************************************************************************************/

FUNC_DEF(riscvVfpRestoreSp)
    ENABLE_VFP

    LW      T0  , SP_FPU_OFFSET_FCSR(A0)

    RISCV_FLW(0 , SP_FPU_OFFSET_REG(0 ), RISCV_A0)
    RISCV_FLW(1 , SP_FPU_OFFSET_REG(1 ), RISCV_A0)
    RISCV_FLW(2 , SP_FPU_OFFSET_REG(2 ), RISCV_A0)
    RISCV_FLW(3 , SP_FPU_OFFSET_REG(3 ), RISCV_A0)
    RISCV_FLW(4 , SP_FPU_OFFSET_REG(4 ), RISCV_A0)
    RISCV_FLW(5 , SP_FPU_OFFSET_REG(5 ), RISCV_A0)
    RISCV_FLW(6 , SP_FPU_OFFSET_REG(6 ), RISCV_A0)
    RISCV_FLW(7 , SP_FPU_OFFSET_REG(7 ), RISCV_A0)
    RISCV_FLW(8 , SP_FPU_OFFSET_REG(8 ), RISCV_A0)
    RISCV_FLW(9 , SP_FPU_OFFSET_REG(9 ), RISCV_A0)
    RISCV_FLW(10, SP_FPU_OFFSET_REG(10), RISCV_A0)
    RISCV_FLW(11, SP_FPU_OFFSET_REG(11), RISCV_A0)
    RISCV_FLW(12, SP_FPU_OFFSET_REG(12), RISCV_A0)
    RISCV_FLW(13, SP_FPU_OFFSET_REG(13), RISCV_A0)
    RISCV_FLW(14, SP_FPU_OFFSET_REG(14), RISCV_A0)
    RISCV_FLW(15, SP_FPU_OFFSET_REG(15), RISCV_A0)
    RISCV_FLW(16, SP_FPU_OFFSET_REG(16), RISCV_A0)
    RISCV_FLW(17, SP_FPU_OFFSET_REG(17), RISCV_A0)
    RISCV_FLW(18, SP_FPU_OFFSET_REG(18), RISCV_A0)
    RISCV_FLW(19, SP_FPU_OFFSET_REG(19), RISCV_A0)
    RISCV_FLW(20, SP_FPU_OFFSET_REG(20), RISCV_A0)
    RISCV_FLW(21, SP_FPU_OFFSET_REG(21), RISCV_A0)
    RISCV_FLW(22, SP_FPU_OFFSET_REG(22), RISCV_A0)
    RISCV_FLW(23, SP_FPU_OFFSET_REG(23), RISCV_A0)
    RISCV_FLW(24, SP_FPU_OFFSET_REG(24), RISCV_A0)
    RISCV_FLW(25, SP_FPU_OFFSET_REG(25), RISCV_A0)
    RISCV_FLW(26, SP_FPU_OFFSET_REG(26), RISCV_A0)
    RISCV_FLW(27, SP_FPU_OFFSET_REG(27), RISCV_A0)
    RISCV_FLW(28, SP_FPU_OFFSET_REG(28), RISCV_A0)
    RISCV_FLW(29, SP_FPU_OFFSET_REG(29), RISCV_A0)
    RISCV_FLW(30, SP_FPU_OFFSET_REG(30), RISCV_A0)
    RISCV_FLW(31, SP_FPU_OFFSET_REG(31), RISCV_A0)

    RISCV_FSCSR_T0

    RET
    FUNC_END(riscvVfpRestoreSp)

;/*********************************************************************************************************
;  保存寄存器
;*********************************************************************************************************/

#define DP_FPU_REG_WIDTH           64                                    /*  浮点数据寄存器的位宽        */
#define DP_FPU_REG_SIZE            (DP_FPU_REG_WIDTH / 8)                /*  浮点数据寄存器的大小        */

/*********************************************************************************************************
  定义 ARCH_FPU_CTX FPU 成员偏移
*********************************************************************************************************/

#define DP_FPU_OFFSET_REG(n)       ((n) * DP_FPU_REG_SIZE)               /*  浮点数据寄存器偏移          */
#define DP_FPU_OFFSET_FCSR         (DP_FPU_OFFSET_REG(FPU_REG_NR))       /*  FCSR 偏移                   */

;/*********************************************************************************************************
;  保存寄存器
;*********************************************************************************************************/

FUNC_DEF(riscvVfpSaveDp)
    ENABLE_VFP

    RISCV_FRCSR_T0

    RISCV_FSD(0 , DP_FPU_OFFSET_REG(0 ), RISCV_A0)
    RISCV_FSD(1 , DP_FPU_OFFSET_REG(1 ), RISCV_A0)
    RISCV_FSD(2 , DP_FPU_OFFSET_REG(2 ), RISCV_A0)
    RISCV_FSD(3 , DP_FPU_OFFSET_REG(3 ), RISCV_A0)
    RISCV_FSD(4 , DP_FPU_OFFSET_REG(4 ), RISCV_A0)
    RISCV_FSD(5 , DP_FPU_OFFSET_REG(5 ), RISCV_A0)
    RISCV_FSD(6 , DP_FPU_OFFSET_REG(6 ), RISCV_A0)
    RISCV_FSD(7 , DP_FPU_OFFSET_REG(7 ), RISCV_A0)
    RISCV_FSD(8 , DP_FPU_OFFSET_REG(8 ), RISCV_A0)
    RISCV_FSD(9 , DP_FPU_OFFSET_REG(9 ), RISCV_A0)
    RISCV_FSD(10, DP_FPU_OFFSET_REG(10), RISCV_A0)
    RISCV_FSD(11, DP_FPU_OFFSET_REG(11), RISCV_A0)
    RISCV_FSD(12, DP_FPU_OFFSET_REG(12), RISCV_A0)
    RISCV_FSD(13, DP_FPU_OFFSET_REG(13), RISCV_A0)
    RISCV_FSD(14, DP_FPU_OFFSET_REG(14), RISCV_A0)
    RISCV_FSD(15, DP_FPU_OFFSET_REG(15), RISCV_A0)
    RISCV_FSD(16, DP_FPU_OFFSET_REG(16), RISCV_A0)
    RISCV_FSD(17, DP_FPU_OFFSET_REG(17), RISCV_A0)
    RISCV_FSD(18, DP_FPU_OFFSET_REG(18), RISCV_A0)
    RISCV_FSD(19, DP_FPU_OFFSET_REG(19), RISCV_A0)
    RISCV_FSD(20, DP_FPU_OFFSET_REG(20), RISCV_A0)
    RISCV_FSD(21, DP_FPU_OFFSET_REG(21), RISCV_A0)
    RISCV_FSD(22, DP_FPU_OFFSET_REG(22), RISCV_A0)
    RISCV_FSD(23, DP_FPU_OFFSET_REG(23), RISCV_A0)
    RISCV_FSD(24, DP_FPU_OFFSET_REG(24), RISCV_A0)
    RISCV_FSD(25, DP_FPU_OFFSET_REG(25), RISCV_A0)
    RISCV_FSD(26, DP_FPU_OFFSET_REG(26), RISCV_A0)
    RISCV_FSD(27, DP_FPU_OFFSET_REG(27), RISCV_A0)
    RISCV_FSD(28, DP_FPU_OFFSET_REG(28), RISCV_A0)
    RISCV_FSD(29, DP_FPU_OFFSET_REG(29), RISCV_A0)
    RISCV_FSD(30, DP_FPU_OFFSET_REG(30), RISCV_A0)
    RISCV_FSD(31, DP_FPU_OFFSET_REG(31), RISCV_A0)

    SW      T0  , DP_FPU_OFFSET_FCSR(A0)

    RET
    FUNC_END(riscvVfpSaveDp)

;/*********************************************************************************************************
;  恢复寄存器
;*********************************************************************************************************/

FUNC_DEF(riscvVfpRestoreDp)
    ENABLE_VFP

    LW      T0  , DP_FPU_OFFSET_FCSR(A0)

    RISCV_FLD(0 , DP_FPU_OFFSET_REG(0 ), RISCV_A0)
    RISCV_FLD(1 , DP_FPU_OFFSET_REG(1 ), RISCV_A0)
    RISCV_FLD(2 , DP_FPU_OFFSET_REG(2 ), RISCV_A0)
    RISCV_FLD(3 , DP_FPU_OFFSET_REG(3 ), RISCV_A0)
    RISCV_FLD(4 , DP_FPU_OFFSET_REG(4 ), RISCV_A0)
    RISCV_FLD(5 , DP_FPU_OFFSET_REG(5 ), RISCV_A0)
    RISCV_FLD(6 , DP_FPU_OFFSET_REG(6 ), RISCV_A0)
    RISCV_FLD(7 , DP_FPU_OFFSET_REG(7 ), RISCV_A0)
    RISCV_FLD(8 , DP_FPU_OFFSET_REG(8 ), RISCV_A0)
    RISCV_FLD(9 , DP_FPU_OFFSET_REG(9 ), RISCV_A0)
    RISCV_FLD(10, DP_FPU_OFFSET_REG(10), RISCV_A0)
    RISCV_FLD(11, DP_FPU_OFFSET_REG(11), RISCV_A0)
    RISCV_FLD(12, DP_FPU_OFFSET_REG(12), RISCV_A0)
    RISCV_FLD(13, DP_FPU_OFFSET_REG(13), RISCV_A0)
    RISCV_FLD(14, DP_FPU_OFFSET_REG(14), RISCV_A0)
    RISCV_FLD(15, DP_FPU_OFFSET_REG(15), RISCV_A0)
    RISCV_FLD(16, DP_FPU_OFFSET_REG(16), RISCV_A0)
    RISCV_FLD(17, DP_FPU_OFFSET_REG(17), RISCV_A0)
    RISCV_FLD(18, DP_FPU_OFFSET_REG(18), RISCV_A0)
    RISCV_FLD(19, DP_FPU_OFFSET_REG(19), RISCV_A0)
    RISCV_FLD(20, DP_FPU_OFFSET_REG(20), RISCV_A0)
    RISCV_FLD(21, DP_FPU_OFFSET_REG(21), RISCV_A0)
    RISCV_FLD(22, DP_FPU_OFFSET_REG(22), RISCV_A0)
    RISCV_FLD(23, DP_FPU_OFFSET_REG(23), RISCV_A0)
    RISCV_FLD(24, DP_FPU_OFFSET_REG(24), RISCV_A0)
    RISCV_FLD(25, DP_FPU_OFFSET_REG(25), RISCV_A0)
    RISCV_FLD(26, DP_FPU_OFFSET_REG(26), RISCV_A0)
    RISCV_FLD(27, DP_FPU_OFFSET_REG(27), RISCV_A0)
    RISCV_FLD(28, DP_FPU_OFFSET_REG(28), RISCV_A0)
    RISCV_FLD(29, DP_FPU_OFFSET_REG(29), RISCV_A0)
    RISCV_FLD(30, DP_FPU_OFFSET_REG(30), RISCV_A0)
    RISCV_FLD(31, DP_FPU_OFFSET_REG(31), RISCV_A0)

    RISCV_FSCSR_T0

    RET
    FUNC_END(riscvVfpRestoreDp)

;/*********************************************************************************************************
;  保存寄存器
;*********************************************************************************************************/

#define QP_FPU_REG_WIDTH           128                                   /*  浮点数据寄存器的位宽        */
#define QP_FPU_REG_SIZE            (QP_FPU_REG_WIDTH / 8)                /*  浮点数据寄存器的大小        */

/*********************************************************************************************************
  定义 ARCH_FPU_CTX FPU 成员偏移
*********************************************************************************************************/

#define QP_FPU_OFFSET_REG(n)       ((n) * QP_FPU_REG_SIZE)               /*  浮点数据寄存器偏移          */
#define QP_FPU_OFFSET_FCSR         (QP_FPU_OFFSET_REG(FPU_REG_NR))       /*  FCSR 偏移                   */

;/*********************************************************************************************************
;  保存寄存器
;*********************************************************************************************************/

FUNC_DEF(riscvVfpSaveQp)
    ENABLE_VFP

    RISCV_FRCSR_T0

    RISCV_FSQ(0 , QP_FPU_OFFSET_REG(0 ), RISCV_A0)
    RISCV_FSQ(1 , QP_FPU_OFFSET_REG(1 ), RISCV_A0)
    RISCV_FSQ(2 , QP_FPU_OFFSET_REG(2 ), RISCV_A0)
    RISCV_FSQ(3 , QP_FPU_OFFSET_REG(3 ), RISCV_A0)
    RISCV_FSQ(4 , QP_FPU_OFFSET_REG(4 ), RISCV_A0)
    RISCV_FSQ(5 , QP_FPU_OFFSET_REG(5 ), RISCV_A0)
    RISCV_FSQ(6 , QP_FPU_OFFSET_REG(6 ), RISCV_A0)
    RISCV_FSQ(7 , QP_FPU_OFFSET_REG(7 ), RISCV_A0)
    RISCV_FSQ(8 , QP_FPU_OFFSET_REG(8 ), RISCV_A0)
    RISCV_FSQ(9 , QP_FPU_OFFSET_REG(9 ), RISCV_A0)
    RISCV_FSQ(10, QP_FPU_OFFSET_REG(10), RISCV_A0)
    RISCV_FSQ(11, QP_FPU_OFFSET_REG(11), RISCV_A0)
    RISCV_FSQ(12, QP_FPU_OFFSET_REG(12), RISCV_A0)
    RISCV_FSQ(13, QP_FPU_OFFSET_REG(13), RISCV_A0)
    RISCV_FSQ(14, QP_FPU_OFFSET_REG(14), RISCV_A0)
    RISCV_FSQ(15, QP_FPU_OFFSET_REG(15), RISCV_A0)
    RISCV_FSQ(16, QP_FPU_OFFSET_REG(16), RISCV_A0)
    RISCV_FSQ(17, QP_FPU_OFFSET_REG(17), RISCV_A0)
    RISCV_FSQ(18, QP_FPU_OFFSET_REG(18), RISCV_A0)
    RISCV_FSQ(19, QP_FPU_OFFSET_REG(19), RISCV_A0)
    RISCV_FSQ(20, QP_FPU_OFFSET_REG(20), RISCV_A0)
    RISCV_FSQ(21, QP_FPU_OFFSET_REG(21), RISCV_A0)
    RISCV_FSQ(22, QP_FPU_OFFSET_REG(22), RISCV_A0)
    RISCV_FSQ(23, QP_FPU_OFFSET_REG(23), RISCV_A0)
    RISCV_FSQ(24, QP_FPU_OFFSET_REG(24), RISCV_A0)
    RISCV_FSQ(25, QP_FPU_OFFSET_REG(25), RISCV_A0)
    RISCV_FSQ(26, QP_FPU_OFFSET_REG(26), RISCV_A0)
    RISCV_FSQ(27, QP_FPU_OFFSET_REG(27), RISCV_A0)
    RISCV_FSQ(28, QP_FPU_OFFSET_REG(28), RISCV_A0)
    RISCV_FSQ(29, QP_FPU_OFFSET_REG(29), RISCV_A0)
    RISCV_FSQ(30, QP_FPU_OFFSET_REG(30), RISCV_A0)
    RISCV_FSQ(31, QP_FPU_OFFSET_REG(31), RISCV_A0)

    SW      T0  , QP_FPU_OFFSET_FCSR(A0)

    RET
    FUNC_END(riscvVfpSaveQp)

;/*********************************************************************************************************
;  恢复寄存器
;*********************************************************************************************************/

FUNC_DEF(riscvVfpRestoreQp)
    ENABLE_VFP

    LW      T0  , QP_FPU_OFFSET_FCSR(A0)

    RISCV_FLQ(0 , QP_FPU_OFFSET_REG(0 ), RISCV_A0)
    RISCV_FLQ(1 , QP_FPU_OFFSET_REG(1 ), RISCV_A0)
    RISCV_FLQ(2 , QP_FPU_OFFSET_REG(2 ), RISCV_A0)
    RISCV_FLQ(3 , QP_FPU_OFFSET_REG(3 ), RISCV_A0)
    RISCV_FLQ(4 , QP_FPU_OFFSET_REG(4 ), RISCV_A0)
    RISCV_FLQ(5 , QP_FPU_OFFSET_REG(5 ), RISCV_A0)
    RISCV_FLQ(6 , QP_FPU_OFFSET_REG(6 ), RISCV_A0)
    RISCV_FLQ(7 , QP_FPU_OFFSET_REG(7 ), RISCV_A0)
    RISCV_FLQ(8 , QP_FPU_OFFSET_REG(8 ), RISCV_A0)
    RISCV_FLQ(9 , QP_FPU_OFFSET_REG(9 ), RISCV_A0)
    RISCV_FLQ(10, QP_FPU_OFFSET_REG(10), RISCV_A0)
    RISCV_FLQ(11, QP_FPU_OFFSET_REG(11), RISCV_A0)
    RISCV_FLQ(12, QP_FPU_OFFSET_REG(12), RISCV_A0)
    RISCV_FLQ(13, QP_FPU_OFFSET_REG(13), RISCV_A0)
    RISCV_FLQ(14, QP_FPU_OFFSET_REG(14), RISCV_A0)
    RISCV_FLQ(15, QP_FPU_OFFSET_REG(15), RISCV_A0)
    RISCV_FLQ(16, QP_FPU_OFFSET_REG(16), RISCV_A0)
    RISCV_FLQ(17, QP_FPU_OFFSET_REG(17), RISCV_A0)
    RISCV_FLQ(18, QP_FPU_OFFSET_REG(18), RISCV_A0)
    RISCV_FLQ(19, QP_FPU_OFFSET_REG(19), RISCV_A0)
    RISCV_FLQ(20, QP_FPU_OFFSET_REG(20), RISCV_A0)
    RISCV_FLQ(21, QP_FPU_OFFSET_REG(21), RISCV_A0)
    RISCV_FLQ(22, QP_FPU_OFFSET_REG(22), RISCV_A0)
    RISCV_FLQ(23, QP_FPU_OFFSET_REG(23), RISCV_A0)
    RISCV_FLQ(24, QP_FPU_OFFSET_REG(24), RISCV_A0)
    RISCV_FLQ(25, QP_FPU_OFFSET_REG(25), RISCV_A0)
    RISCV_FLQ(26, QP_FPU_OFFSET_REG(26), RISCV_A0)
    RISCV_FLQ(27, QP_FPU_OFFSET_REG(27), RISCV_A0)
    RISCV_FLQ(28, QP_FPU_OFFSET_REG(28), RISCV_A0)
    RISCV_FLQ(29, QP_FPU_OFFSET_REG(29), RISCV_A0)
    RISCV_FLQ(30, QP_FPU_OFFSET_REG(30), RISCV_A0)
    RISCV_FLQ(31, QP_FPU_OFFSET_REG(31), RISCV_A0)

    RISCV_FSCSR_T0

    RET
    FUNC_END(riscvVfpRestoreQp)

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
