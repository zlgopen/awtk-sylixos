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
** 文   件   名: assembler.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: MIPS 汇编相关.
*********************************************************************************************************/

#ifndef __ASMMIPS_ASSEMBLER_H
#define __ASMMIPS_ASSEMBLER_H

#include "archprob.h"
#include "arch/mips/arch_def.h"

/*********************************************************************************************************
  mips architecture assembly special code
*********************************************************************************************************/

#if defined(__ASSEMBLY__) || defined(ASSEMBLY)

#ifndef __MP_CFG_H
#include "../SylixOS/config/mp/mp_cfg.h"
#endif

/*********************************************************************************************************
  assembler define
*********************************************************************************************************/

#define EXPORT_LABEL(label)       .global label

#define IMPORT_LABEL(label)       .extern label

#define FUNC_LABEL(func)          func:
#define LINE_LABEL(line)          line:

#define FUNC_DEF(name)                  \
        .text;                          \
        .balign     4;                  \
        .type       symbol, @function;  \
        .ent        name;               \
        .set        push;               \
        .set        noreorder;          \
        .set        volatile;          \
name:

#define FUNC_END(name)                  \
        .set        pop;                \
        .size       name, .-name;       \
        .end        name

#define MACRO_DEF(mfunc...)             \
        .macro      mfunc

#define MACRO_END()                     \
        .endm

#define FILE_BEGIN()                    \
        .set        noreorder;          \
        .balign     4;

#define FILE_END()

#define SECTION(sec)                    \
        .section    sec

#define WEAK(name)                      \
        .weakext    name;               \
        .balign     4;

/*********************************************************************************************************
  Macros to handle different pointer/register sizes for 32/64-bit code
*********************************************************************************************************/

/*********************************************************************************************************
  Size of a register
*********************************************************************************************************/

#ifdef __mips64
#define SZREG       8
#else
#define SZREG       4
#endif

/*********************************************************************************************************
  Use the following macros in assemblercode to load/store registers, pointers etc.
*********************************************************************************************************/

#if (_MIPS_SIM == _MIPS_SIM_ABI32)
#define REG_S       sw
#define REG_L       lw
#define REG_SUBU    subu
#define REG_ADDU    addu
#endif
#if (_MIPS_SIM == _MIPS_SIM_NABI32) || (_MIPS_SIM == _MIPS_SIM_ABI64)
#define REG_S       sd
#define REG_L       ld
#define REG_SUBU    dsubu
#define REG_ADDU    daddu
#endif

/*********************************************************************************************************
  How to add/sub/load/store/shift C int variables.
*********************************************************************************************************/

#if (_MIPS_SZINT == 32)
#define INT_ADD     add
#define INT_ADDU    addu
#define INT_ADDI    addi
#define INT_ADDIU   addiu
#define INT_SUB     sub
#define INT_SUBU    subu
#define INT_L       lw
#define INT_S       sw
#define INT_SLL     sll
#define INT_SLLV    sllv
#define INT_SRL     srl
#define INT_SRLV    srlv
#define INT_SRA     sra
#define INT_SRAV    srav
#endif

#if (_MIPS_SZINT == 64)
#define INT_ADD     dadd
#define INT_ADDU    daddu
#define INT_ADDI    daddi
#define INT_ADDIU   daddiu
#define INT_SUB     dsub
#define INT_SUBU    dsubu
#define INT_L       ld
#define INT_S       sd
#define INT_SLL     dsll
#define INT_SLLV    dsllv
#define INT_SRL     dsrl
#define INT_SRLV    dsrlv
#define INT_SRA     dsra
#define INT_SRAV    dsrav
#endif

/*********************************************************************************************************
  How to add/sub/load/store/shift C long variables.
*********************************************************************************************************/

#if (_MIPS_SZLONG == 32)
#define LONG_ADD    add
#define LONG_ADDU   addu
#define LONG_ADDI   addi
#define LONG_ADDIU  addiu
#define LONG_SUB    sub
#define LONG_SUBU   subu
#define LONG_L      lw
#define LONG_S      sw
#define LONG_SP     swp
#define LONG_SLL    sll
#define LONG_SLLV   sllv
#define LONG_SRL    srl
#define LONG_SRLV   srlv
#define LONG_SRA    sra
#define LONG_SRAV   srav

#define LONG        .word
#define LONGSIZE    4
#define LONGMASK    3
#define LONGLOG     2
#endif

#if (_MIPS_SZLONG == 64)
#define LONG_ADD    dadd
#define LONG_ADDU   daddu
#define LONG_ADDI   daddi
#define LONG_ADDIU  daddiu
#define LONG_SUB    dsub
#define LONG_SUBU   dsubu
#define LONG_L      ld
#define LONG_S      sd
#define LONG_SP     sdp
#define LONG_SLL    dsll
#define LONG_SLLV   dsllv
#define LONG_SRL    dsrl
#define LONG_SRLV   dsrlv
#define LONG_SRA    dsra
#define LONG_SRAV   dsrav

#define LONG        .dword
#define LONGSIZE    8
#define LONGMASK    7
#define LONGLOG     3
#endif

/*********************************************************************************************************
  How to add/sub/load/store/shift pointers.
*********************************************************************************************************/

#if (_MIPS_SZPTR == 32)
#define PTR_ADD     add
#define PTR_ADDU    addu
#define PTR_ADDI    addi
#define PTR_ADDIU   addiu
#define PTR_SUB     sub
#define PTR_SUBU    subu
#define PTR_L       lw
#define PTR_S       sw
#define PTR_LA      la
#define PTR_LI      li
#define PTR_SLL     sll
#define PTR_SLLV    sllv
#define PTR_SRL     srl
#define PTR_SRLV    srlv
#define PTR_SRA     sra
#define PTR_SRAV    srav

#define PTR_SCALESHIFT  2

#define PTR         .word
#define PTRSIZE     4
#define PTRLOG      2
#endif

#if (_MIPS_SZPTR == 64)
#define PTR_ADD     dadd
#define PTR_ADDU    daddu
#define PTR_ADDI    daddi
#define PTR_ADDIU   daddiu
#define PTR_SUB     dsub
#define PTR_SUBU    dsubu
#define PTR_L       ld
#define PTR_S       sd
#define PTR_LA      dla
#define PTR_LI      dli
#define PTR_SLL     dsll
#define PTR_SLLV    dsllv
#define PTR_SRL     dsrl
#define PTR_SRLV    dsrlv
#define PTR_SRA     dsra
#define PTR_SRAV    dsrav

#define PTR_SCALESHIFT  3

#define PTR         .dword
#define PTRSIZE     8
#define PTRLOG      3
#endif

/*********************************************************************************************************
  macros define
*********************************************************************************************************/

#define MTC0_EHB(src, dst)              \
    MTC0    src , dst;                  \
    EHB

#define MFC0_EHB(dst, src)              \
    MFC0    dst , src;                  \
    EHB

#define CTC1_EHB(src, dst)              \
    CTC1    src , dst;                  \
    EHB

#define CFC1_EHB(dst, src)              \
    CFC1    dst , src;                  \
    EHB

#define CTC2_EHB(src, dst)              \
    CTC2    src , dst;                  \
    EHB

#define CFC2_EHB(dst, src)              \
    CFC2    dst , src;                  \
    EHB

#if LW_CFG_CPU_WORD_LENGHT == 32

#define MTC0_LONG(src, dst)             \
    MTC0    src , dst

#define MFC0_LONG(dst, src)             \
    MFC0    dst , src

#define MTC0_LONG_EHB(src, dst)         \
    MTC0    src , dst;                  \
    EHB

#define MFC0_LONG_EHB(dst, src)         \
    MFC0    dst , src;                  \
    EHB

#else

#define MTC0_LONG(src, dst)             \
    DMTC0    src , dst

#define MFC0_LONG(dst, src)             \
    DMFC0    dst , src;

#define MTC0_LONG_EHB(src, dst)         \
    DMTC0    src , dst;                 \
    EHB

#define MFC0_LONG_EHB(dst, src)         \
    DMFC0    dst , src;                 \
    EHB

#endif

#define MOV             MOVE

/*********************************************************************************************************
  兼容老的 BSP 而定义的宏(新 BSP 请用 MTC0_EHB 和 MFC0_EHB)
*********************************************************************************************************/

#define MTC0(src, dst)                  \
    MTC0    src , dst;                  \
    EHB

#define MFC0(dst, src)                  \
    MFC0    dst , src;                  \
    EHB

/*********************************************************************************************************
  寄存器定义(MIPS32 用 O32 ABI, MIPS64 用 N64 ABI)
*********************************************************************************************************/

#define ZERO            $0                                              /*  wired zero                  */
#define AT              $at                                             /*  assembler temp              */

#define V0              $2                                              /*  return reg 0                */
#define V1              $3                                              /*  return reg 1                */

#define A0              $4                                              /*  arg reg 0                   */
#define A1              $5                                              /*  arg reg 1                   */
#define A2              $6                                              /*  arg reg 2                   */
#define A3              $7                                              /*  arg reg 3                   */

#if (_MIPS_SIM == _MIPS_SIM_ABI32)
#define T0              $8                                              /*  caller saved 0              */
#define T1              $9                                              /*  caller saved 1              */
#define T2              $10                                             /*  caller saved 2              */
#define T3              $11                                             /*  caller saved 3              */
#define T4              $12                                             /*  caller saved 4              */
#define T5              $13                                             /*  caller saved 5              */
#define T6              $14                                             /*  caller saved 6              */
#define T7              $15                                             /*  caller saved 7              */
#else
#define A4              $8                                              /*  arg reg 4                   */
#define A5              $9                                              /*  arg reg 5                   */
#define A6              $10                                             /*  arg reg 6                   */
#define A7              $11                                             /*  arg reg 7                   */

#define T0              $12                                             /*  caller saved 0              */
#define T1              $13                                             /*  caller saved 1              */
#define T2              $14                                             /*  caller saved 2              */
#define T3              $15                                             /*  caller saved 3              */
#endif

#define T8              $24                                             /*  caller saved 8              */
#define T9              $25                                             /*  caller saved 9              */

#define S0              $16                                             /*  callee saved 0              */
#define S1              $17                                             /*  callee saved 1              */
#define S2              $18                                             /*  callee saved 2              */
#define S3              $19                                             /*  callee saved 3              */
#define S4              $20                                             /*  callee saved 4              */
#define S5              $21                                             /*  callee saved 5              */
#define S6              $22                                             /*  callee saved 6              */
#define S7              $23                                             /*  callee saved 7              */

#define K0              $26                                             /*  kernel temp 0               */
#define K1              $27                                             /*  kernel temp 1               */

#define GP              $28                                             /*  global pointer              */
#define SP              $29                                             /*  stack pointer               */
#define S8              $30                                             /*  callee saved 8              */
#define FP              S8                                              /*  callee saved 8              */
#define RA              $31                                             /*  return address              */

/*********************************************************************************************************
  RDHWR 寄存器定义
*********************************************************************************************************/

#define HWR_CPUNUM      $0                                              /*  CPU number                  */
#define HWR_SYNCISTEP   $1                                              /*  SYNCI step size             */
#define HWR_CC          $2                                              /*  Cycle counter               */
#define HWR_CCRES       $3                                              /*  Cycle counter resolution    */
#define HWR_ULR         $29                                             /*  UserLocal                   */
#define HWR_IMPL1       $30                                             /*  Implementation dependent    */
#define HWR_IMPL2       $31                                             /*  Implementation dependent    */

/*********************************************************************************************************
  汇编调用 C 函数临时变量保存与参数预留栈大小
*********************************************************************************************************/

#if LW_CFG_CPU_WORD_LENGHT == 32
#define ARCH_STK_VAR_SIZE       32                                      /*  O32 ABI 4 个参数寄存器      */
                                                                        /*  4 个临时变量保存区          */
#else
#define ARCH_STK_VAR_SIZE       32                                      /*  N64 ABI 不需要预留参数栈    */
                                                                        /*  4 个临时变量保存区          */
#endif

#define ARCH_STK_OFF_VAR(n)     (ARCH_STK_VAR_SIZE - ((n) + 1) * SZREG)

#endif                                                                  /*  __ASSEMBLY__                */
#endif                                                                  /*  __ASMMIPS_ASSEMBLER_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
