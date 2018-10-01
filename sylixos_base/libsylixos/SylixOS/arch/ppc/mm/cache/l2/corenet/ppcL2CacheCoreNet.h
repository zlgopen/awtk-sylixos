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
** 文   件   名: ppcL2CacheCoreNet.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 08 月 07 日
**
** 描        述: CoreNet 体系构架 L2 CACHE 驱动.
*********************************************************************************************************/

#ifndef __ARCH_PPCL2CACHECORENET_H
#define __ARCH_PPCL2CACHECORENET_H

#define PPC_CORENET_L2_CACHE_MAX_NR 3

typedef struct {
    BOOL        CFG_bPresent;                                           /*  是否存在 L2 CACHE           */
    UINT32      CFG_uiL2CacheCsr1;                                      /*  L2 CACHE CSR1 配置值        */
    UINT        CFG_uiNum;                                              /*  L2 CACHE 数目               */
    addr_t      CFG_ulBase[PPC_CORENET_L2_CACHE_MAX_NR];                /*  L2 CACHE 控制器基地址       */
} PPC_CORENET_L2CACHE_CONFIG;

VOID  ppcCoreNetL2CacheConfig(PPC_CORENET_L2CACHE_CONFIG  *pL2Config);

#endif                                                                  /*  __ARCH_PPCL2CACHECORENET_H  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
