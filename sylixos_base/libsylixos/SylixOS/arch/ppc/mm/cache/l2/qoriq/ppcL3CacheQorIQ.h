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
** 文   件   名: ppcL3CacheQorIQ.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 08 月 07 日
**
** 描        述: QorIQ 体系构架 L3 CACHE 驱动.
*********************************************************************************************************/

#ifndef __ARCH_PPCL3CACHEQORIQ_H
#define __ARCH_PPCL3CACHEQORIQ_H

#define PPC_QORIQ_L3_CACHE_MAX_NR  4

typedef struct {
    BOOL        CFG_bPresent;                                           /*  是否存在 L3 CACHE           */
    UINT        CFG_uiNum;                                              /*  L3 CACHE 数目               */
    addr_t      CFG_ulBase[PPC_QORIQ_L3_CACHE_MAX_NR];                  /*  L3 CACHE 控制器基地址       */
} PPC_QORIQ_L3CACHE_CONFIG;

VOID  ppcQorIQL3CacheConfig(PPC_QORIQ_L3CACHE_CONFIG  *pL3Config);

#endif                                                                  /*  __ARCH_PPCL3CACHEQORIQ_H    */
/*********************************************************************************************************
  END
*********************************************************************************************************/
