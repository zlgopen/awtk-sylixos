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
** 文   件   名: mipsIdle.c
**
** 创   建   人: Han.hui (韩辉)
**
** 文件创建日期: 2017 年 04 月 14 日
**
** 描        述: MIPS 体系架构 IDLE 节能程序.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "arch/mips/common/cp0/mipsCp0.h"
/*********************************************************************************************************
  汇编函数
*********************************************************************************************************/
extern VOID  mipsWaitInstruction(VOID);
/*********************************************************************************************************
** 函数名称: mipsIdleHookGet
** 功能描述: 获得 IDLE 节能程序
** 输　入  : pcMachineName     机器名字
** 输　出  : IDLE 节能程序
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOIDFUNCPTR  mipsIdleHookGet (CPCHAR  pcMachineName)
{
    if ((lib_strcmp(pcMachineName, MIPS_MACHINE_LS1X)   == 0) ||
        (lib_strcmp(pcMachineName, MIPS_MACHINE_24KF)   == 0) ||
        (lib_strcmp(pcMachineName, MIPS_MACHINE_JZ47XX) == 0)) {
        return  (mipsWaitInstruction);

    } else if ((lib_strcmp(pcMachineName, MIPS_MACHINE_LS3X) == 0)) {
        ARCH_REG_T  ulPrid = mipsCp0PRIdRead();

        if (((ulPrid & 0xf) == PRID_REV_LOONGSON2K_R1) ||
            ((ulPrid & 0xf) == PRID_REV_LOONGSON2K_R2)) {
            return  (mipsWaitInstruction);

        } else if ((ulPrid & 0xf) != PRID_REV_LOONGSON3A_R1) {
            return  (mipsWaitInstruction);
        }

    } else if ((lib_strcmp(pcMachineName, MIPS_MACHINE_HR2) == 0)) {
        return  (mipsWaitInstruction);
    }

    return  (LW_NULL);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
