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
** ��   ��   ��: arch_compiler.h
**
** ��   ��   ��: Jiao.JinXing (������)
**
** �ļ���������: 2016 �� 06 �� 25 ��
**
** ��        ��: x86 ���������.
*********************************************************************************************************/

#ifndef __X86_ARCH_COMPILER_H
#define __X86_ARCH_COMPILER_H

/*********************************************************************************************************
  fast variable
*********************************************************************************************************/

#ifndef REGISTER
#define	REGISTER                register                                /*  �Ĵ�������                  */
#endif

/*********************************************************************************************************
  inline function
*********************************************************************************************************/

#ifdef __GNUC__
#define LW_INLINE               inline                                  /*  ������������                */
#define LW_ALWAYS_INLINE        __attribute__((always_inline))
#else
#define LW_INLINE               __inline
#define LW_ALWAYS_INLINE        __inline
#endif

/*********************************************************************************************************
  weak function
*********************************************************************************************************/

#ifdef __SYLIXOS_KERNEL
#ifdef __GNUC__
#define LW_WEAK                 __attribute__((weak))                   /*  ������                      */
#else
#define LW_WEAK
#endif
#endif                                                                  /*  __SYLIXOS_KERNEL            */

/*********************************************************************************************************
  shared object function call prepare
*********************************************************************************************************/

#define LW_SOFUNC_PREPARE(func)

/*********************************************************************************************************
  �������ṹ�����������
*********************************************************************************************************/

#define LW_STRUCT_PACK_FIELD(x)     x
#define LW_STRUCT_PACK_STRUCT       __attribute__((packed))
#define LW_STRUCT_PACK_BEGIN                                            /*  ���ֽ����Žṹ��            */
#define LW_STRUCT_PACK_END                                              /*  �������ֽ����Žṹ��        */

/*********************************************************************************************************
  ������ ABI �Ƿ������ں�ʹ�ø�������
*********************************************************************************************************/

#if LW_CFG_CPU_WORD_LENGHT == 64
#define LW_KERN_FLOATING            0
#else
#define LW_KERN_FLOATING            1                                   /*  �Ƿ������ں�ʹ�ø�������    */
#endif

#endif                                                                  /*  __X86_ARCH_COMPILER_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/