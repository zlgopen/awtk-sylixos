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
;** 文   件   名: ppcL2CacheE500mcAsm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2018 年 08 月 07 日
;**
;** 描        述: E500MC 体系构架 L2 CACHE 驱动.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/cache_cfg.h>

#if LW_CFG_CACHE_EN > 0 && LW_CFG_PPC_CACHE_L2 > 0

#define __SYLIXOS_PPC_E500MC__
#include "arch/ppc/arch_e500.h"

    FILE_BEGIN()

    EXPORT_LABEL(ppcE500mcGetL2Csr0)
    EXPORT_LABEL(ppcE500mcSetL2Csr0)

;/*********************************************************************************************************
;  SPR GET SET 函数
;*********************************************************************************************************/

MACRO_DEF(SET_SPR, spr)
    MTSPR       \spr , R3
    ISYNC
    BLR
    MACRO_END()

MACRO_DEF(GET_SPR, spr)
    MFSPR       R3 , \spr
    ISYNC
    BLR
    MACRO_END()

FUNC_DEF(ppcE500mcGetL2Csr0)
    GET_SPR     L2CSR0
    FUNC_END()

FUNC_DEF(ppcE500mcSetL2Csr0)
    SET_SPR     L2CSR0
    FUNC_END()

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
