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
** 文   件   名: mipsCacheHr2.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 14 日
**
** 描        述: 华睿 2 号处理器 CACHE 驱动.
*********************************************************************************************************/

#ifndef __ARCH_MIPSCACHEHR2_H
#define __ARCH_MIPSCACHEHR2_H

VOID  hr2CacheFlushAll(VOID);
VOID  hr2CacheEnableHw(VOID);

VOID  mipsCacheHr2Init(LW_CACHE_OP *pcacheop,
                       CACHE_MODE   uiInstruction,
                       CACHE_MODE   uiData,
                       CPCHAR       pcMachineName);
VOID  mipsCacheHr2Reset(CPCHAR  pcMachineName);

#endif                                                                  /*  __ARCH_MIPSCACHEHR2_H       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
