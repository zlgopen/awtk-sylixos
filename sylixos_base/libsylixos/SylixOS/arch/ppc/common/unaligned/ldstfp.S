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
;** 文   件   名: ldstfp.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2015 年 12 月 15 日
;**
;** 描        述: PowerPC 体系构架浮点 load 和 store 指令模拟.
;*********************************************************************************************************/
/*
 * Floating-point, VMX/Altivec and VSX loads and stores
 * for use in instruction emulation.
 *
 * Copyright 2010 Paul Mackerras, IBM Corp. <paulus@au1.ibm.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#ifndef SYLIXOS
#include <asm/processor.h>
#include <asm/ppc_asm.h>
#include <asm/ppc-opcode.h>
#include <asm/reg.h>
#include <asm/asm-offsets.h>
#include <linux/errno.h>
#else
#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include "porting.h"

    FILE_BEGIN()

    EXPORT_LABEL(get_fpr)
    EXPORT_LABEL(put_fpr)
    EXPORT_LABEL(get_vr)
    EXPORT_LABEL(put_vr)
    EXPORT_LABEL(get_vsr)
    EXPORT_LABEL(put_vsr)
    EXPORT_LABEL(load_vsrn)
    EXPORT_LABEL(store_vsrn)
    EXPORT_LABEL(conv_sp_to_dp)
    EXPORT_LABEL(conv_dp_to_sp)

/*
 * General Purpose Registers (GPRs)
 *
 * The lower case r0-r31 should be used in preference to the upper
 * case R0-R31 as they provide more error checking in the assembler.
 * Use R0-31 only when really nessesary.
 */
#define r0      %r0
#define r1      %r1
#define r2      %r2
#define r3      %r3
#define r4      %r4
#define r5      %r5
#define r6      %r6
#define r7      %r7
#define r8      %r8
#define r9      %r9
#define r10     %r10
#define r11     %r11
#define r12     %r12
#define r13     %r13
#define r14     %r14
#define r15     %r15
#define r16     %r16
#define r17     %r17
#define r18     %r18
#define r19     %r19
#define r20     %r20
#define r21     %r21
#define r22     %r22
#define r23     %r23
#define r24     %r24
#define r25     %r25
#define r26     %r26
#define r27     %r27
#define r28     %r28
#define r29     %r29
#define r30     %r30
#define r31     %r31

/*
 * Floating Point Registers (FPRs)
 */
#define fr0     0
#define fr1     1
#define fr2     2
#define fr3     3
#define fr4     4
#define fr5     5
#define fr6     6
#define fr7     7
#define fr8     8
#define fr9     9
#define fr10    10
#define fr11    11
#define fr12    12
#define fr13    13
#define fr14    14
#define fr15    15
#define fr16    16
#define fr17    17
#define fr18    18
#define fr19    19
#define fr20    20
#define fr21    21
#define fr22    22
#define fr23    23
#define fr24    24
#define fr25    25
#define fr26    26
#define fr27    27
#define fr28    28
#define fr29    29
#define fr30    30
#define fr31    31

#endif

#ifdef CONFIG_PPC_FPU

#define STKFRM  (PPC_MIN_STKFRM + 16)

/* Get the contents of frN into *p; N is in r3 and p is in r4. */
FUNC_DEF(get_fpr)
    mflr    r0
    mfmsr   r6
    ori r7, r6, MSR_FP
    MTMSRD(r7)
    isync
    rlwinm  r3,r3,3,0xf8
    bcl 20,31,1f
reg = 0
    .rept   32
    stfd    reg, 0(r4)
    b   2f
reg = reg + 1
    .endr
1:  mflr    r5
    add r5,r3,r5
    mtctr   r5
    mtlr    r0
    bctr
2:  MTMSRD(r6)
    isync
    blr
    FUNC_END()

/* Put the contents of *p into frN; N is in r3 and p is in r4. */
FUNC_DEF(put_fpr)
    mflr    r0
    mfmsr   r6
    ori r7, r6, MSR_FP
    MTMSRD(r7)
    isync
    rlwinm  r3,r3,3,0xf8
    bcl 20,31,1f
reg = 0
    .rept   32
    lfd reg, 0(r4)
    b   2f
reg = reg + 1
    .endr
1:  mflr    r5
    add r5,r3,r5
    mtctr   r5
    mtlr    r0
    bctr
2:  MTMSRD(r6)
    isync
    blr
    FUNC_END()

#ifdef CONFIG_ALTIVEC
/* Get the contents of vrN into *p; N is in r3 and p is in r4. */
FUNC_DEF(get_vr)
    mflr    r0
    mfmsr   r6
    oris    r7, r6, MSR_VEC@h
    MTMSRD(r7)
    isync
    rlwinm  r3,r3,3,0xf8
    bcl 20,31,1f
reg = 0
    .rept   32
    stvx    reg, 0, r4
    b   2f
reg = reg + 1
    .endr
1:  mflr    r5
    add r5,r3,r5
    mtctr   r5
    mtlr    r0
    bctr
2:  MTMSRD(r6)
    isync
    blr
    FUNC_END()

/* Put the contents of *p into vrN; N is in r3 and p is in r4. */
FUNC_DEF(put_vr)
    mflr    r0
    mfmsr   r6
    oris    r7, r6, MSR_VEC@h
    MTMSRD(r7)
    isync
    rlwinm  r3,r3,3,0xf8
    bcl 20,31,1f
reg = 0
    .rept   32
    lvx reg, 0, r4
    b   2f
reg = reg + 1
    .endr
1:  mflr    r5
    add r5,r3,r5
    mtctr   r5
    mtlr    r0
    bctr
2:  MTMSRD(r6)
    isync
    blr
    FUNC_END()
#endif /* CONFIG_ALTIVEC */

#ifdef CONFIG_VSX
/* Get the contents of vsN into vs0; N is in r3. */
FUNC_DEF(get_vsr)
    mflr    r0
    rlwinm  r3,r3,3,0x1f8
    bcl 20,31,1f
    blr         /* vs0 is already in vs0 */
    nop
reg = 1
    .rept   63
    XXLOR(0,reg,reg)
    blr
reg = reg + 1
    .endr
1:  mflr    r5
    add r5,r3,r5
    mtctr   r5
    mtlr    r0
    bctr
    FUNC_END()

/* Put the contents of vs0 into vsN; N is in r3. */
FUNC_DEF(put_vsr)
    mflr    r0
    rlwinm  r3,r3,3,0x1f8
    bcl 20,31,1f
    blr         /* v0 is already in v0 */
    nop
reg = 1
    .rept   63
    XXLOR(reg,0,0)
    blr
reg = reg + 1
    .endr
1:  mflr    r5
    add r5,r3,r5
    mtctr   r5
    mtlr    r0
    bctr
    FUNC_END()

/* Load VSX reg N from vector doubleword *p.  N is in r3, p in r4. */
FUNC_DEF(load_vsrn)
    PPC_STLU r1,-STKFRM(r1)
    mflr    r0
    PPC_STL r0,STKFRM+PPC_LR_STKOFF(r1)
    mfmsr   r6
    oris    r7,r6,MSR_VSX@h
    cmpwi   cr7,r3,0
    li  r8,STKFRM-16
    MTMSRD(r7)
    isync
    beq cr7,1f
    STXVD2X(0,R1,R8)
1:  LXVD2X(0,R0,R4)
#ifdef __LITTLE_ENDIAN__
    XXSWAPD(0,0)
#endif
    beq cr7,4f
    bl  put_vsr
    LXVD2X(0,R1,R8)
4:  PPC_LL  r0,STKFRM+PPC_LR_STKOFF(r1)
    mtlr    r0
    MTMSRD(r6)
    isync
    addi    r1,r1,STKFRM
    blr
    FUNC_END()

/* Store VSX reg N to vector doubleword *p.  N is in r3, p in r4. */
FUNC_DEF(store_vsrn)
    PPC_STLU r1,-STKFRM(r1)
    mflr    r0
    PPC_STL r0,STKFRM+PPC_LR_STKOFF(r1)
    mfmsr   r6
    oris    r7,r6,MSR_VSX@h
    li  r8,STKFRM-16
    MTMSRD(r7)
    isync
    STXVD2X(0,R1,R8)
    bl  get_vsr
#ifdef __LITTLE_ENDIAN__
    XXSWAPD(0,0)
#endif
    STXVD2X(0,R0,R4)
    LXVD2X(0,R1,R8)
    PPC_LL  r0,STKFRM+PPC_LR_STKOFF(r1)
    mtlr    r0
    MTMSRD(r6)
    isync
    mr  r3,r9
    addi    r1,r1,STKFRM
    blr
    FUNC_END()
#endif /* CONFIG_VSX */

/* Convert single-precision to double, without disturbing FPRs. */
/* conv_sp_to_dp(float *sp, double *dp) */
FUNC_DEF(conv_sp_to_dp)
    mfmsr   r6
    ori r7, r6, MSR_FP
    MTMSRD(r7)
    isync
    stfd    fr0, -16(r1)
    lfs fr0, 0(r3)
    stfd    fr0, 0(r4)
    lfd fr0, -16(r1)
    MTMSRD(r6)
    isync
    blr
    FUNC_END()

/* Convert single-precision to double, without disturbing FPRs. */
/* conv_sp_to_dp(double *dp, float *sp) */
FUNC_DEF(conv_dp_to_sp)
    mfmsr   r6
    ori r7, r6, MSR_FP
    MTMSRD(r7)
    isync
    stfd    fr0, -16(r1)
    lfd fr0, 0(r3)
    stfs    fr0, 0(r4)
    lfd fr0, -16(r1)
    MTMSRD(r6)
    isync
    blr
    FUNC_END()

#endif  /* CONFIG_PPC_FPU */

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
