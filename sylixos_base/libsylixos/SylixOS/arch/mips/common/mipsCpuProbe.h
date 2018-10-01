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
** 文   件   名: mipsCpuProbe.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 07 月 18 日
**
** 描        述: MIPS 体系架构 CPU 探测.
*********************************************************************************************************/

#ifndef __ARCH_MIPSCPUPROBE_H
#define __ARCH_MIPSCPUPROBE_H

/*********************************************************************************************************
  全局变量声明
*********************************************************************************************************/

extern UINT32         _G_uiMipsProcessorId;                             /*  处理器 ID                   */
extern UINT32         _G_uiMipsPridRev;                                 /*  处理器 ID Revision          */
extern UINT32         _G_uiMipsPridImp;                                 /*  处理器 ID implementation    */
extern UINT32         _G_uiMipsPridComp;                                /*  处理器 ID Company           */
extern UINT32         _G_uiMipsPridOpt;                                 /*  处理器 ID Option            */
extern MIPS_CPU_TYPE  _G_uiMipsCpuType;                                 /*  处理器类型                  */
extern UINT32         _G_uiMipsMachineType;                             /*  机器类型                    */

VOID  mipsCpuProbe(CPCHAR  pcMachineName);

#endif                                                                  /*  __ARCH_MIPSCPUPROBE_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
