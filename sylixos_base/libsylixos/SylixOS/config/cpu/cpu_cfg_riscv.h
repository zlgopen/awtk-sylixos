/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: cpu_cfg_riscv.h
**
** ��   ��   ��: Jiao.JinXing (������)
**
** �ļ���������: 2018 �� 03 �� 20 ��
**
** ��        ��: RISC-V CPU �����빦������.
*********************************************************************************************************/

#ifndef __CPU_CFG_RISCV_H
#define __CPU_CFG_RISCV_H

/*********************************************************************************************************
  CPU ��ϵ�ṹ
*********************************************************************************************************/

#define LW_CFG_CPU_ARCH_RISCV           1                               /*  CPU �ܹ�                    */
#define LW_CFG_CPU_ARCH_FAMILY          "RISC-V(R)"                     /*  RISC-V family               */

/*********************************************************************************************************
  SMT ͬ�����̵߳����Ż�
*********************************************************************************************************/

#define LW_CFG_CPU_ARCH_SMT             0                               /*  ͬ�����߳��Ż�              */

/*********************************************************************************************************
  CACHE LINE ����
*********************************************************************************************************/

#define LW_CFG_CPU_ARCH_CACHE_LINE      64                              /*  cache ����ж�������        */

/*********************************************************************************************************
  CPU ��ϵ�ṹ����
*********************************************************************************************************/

/*********************************************************************************************************
  CPU �ֳ������ʹ�С�˶���
*********************************************************************************************************/

#define LW_CFG_CPU_ENDIAN               0                               /*  0: С��  1: ���            */
#define LW_CFG_CPU_WORD_LENGHT          __riscv_xlen                    /*  CPU �ֳ�                    */

#if LW_CFG_CPU_WORD_LENGHT == 32
#define LW_CFG_CPU_PHYS_ADDR_64BIT      0                               /*  ������ַ 64bit ����         */
#else
#define LW_CFG_CPU_PHYS_ADDR_64BIT      1
#endif

/*********************************************************************************************************
  RISC-V ����
*********************************************************************************************************/

#define LW_CFG_RISCV_M_LEVEL            0                               /*  �Ƿ�ʹ�� Machine level      */

#if LW_CFG_CPU_WORD_LENGHT == 64
#define LW_CFG_RISCV_MMU_SV39           1                               /*  1: SV39 MMU, 0: SV48 MMU    */
#endif

#define LW_CFG_RISCV_GOT_SIZE           (8 * LW_CFG_KB_SIZE)            /*  �ں�ģ�� GOT ��С           */
#define LW_CFG_RISCV_HI20_SIZE          (8 * LW_CFG_KB_SIZE)            /*  �ں�ģ�� HI20 ��Ϣ��С      */

#define LW_CFG_RISCV_MPU_EN             0                               /*  CPU �Ƿ�ӵ�� MPU            */

/*********************************************************************************************************
  �������㵥Ԫ
*********************************************************************************************************/

#define LW_CFG_CPU_FPU_EN               1                               /*  CPU �Ƿ�ӵ�� FPU            */

/*********************************************************************************************************
  DSP �����źŴ�����
*********************************************************************************************************/

#define LW_CFG_CPU_DSP_EN               0                               /*  CPU �Ƿ�ӵ�� DSP            */

/*********************************************************************************************************
  ATOMIC
*********************************************************************************************************/

#ifdef __riscv_atomic
#define LW_CFG_CPU_ATOMIC_EN            1
#else
#define LW_CFG_CPU_ATOMIC_EN            0
#endif                                                                  /*  defined(__riscv_atomic)     */

#endif                                                                  /*  __CPU_CFG_RISCV_H           */
/*********************************************************************************************************
  END
*********************************************************************************************************/