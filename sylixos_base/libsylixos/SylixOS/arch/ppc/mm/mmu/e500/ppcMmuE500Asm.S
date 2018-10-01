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
;** 文   件   名: ppcMmuE500Asm.S
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2016 年 05 月 04 日
;**
;** 描        述: PowerPC E500 体系构架 MMU 驱动.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/vmm_cfg.h>

#if LW_CFG_VMM_EN > 0

#include "arch/ppc/arch_e500.h"

    FILE_BEGIN()

    EXPORT_LABEL(ppcE500MmuInvalidateTLB)
    EXPORT_LABEL(ppcE500MmuInvalidateTLBEA)
    EXPORT_LABEL(archE500DataTLBErrorEntry)
    EXPORT_LABEL(archE500InstructionTLBErrorEntry)

;/*********************************************************************************************************
;  定义
;*********************************************************************************************************/

#define INV_TLB0    0x0

;/*********************************************************************************************************
;  无效所有 TLB
;*********************************************************************************************************/

FUNC_DEF(ppcE500MmuInvalidateTLB)
    LIS         R3 , HIADJ(ARCH_PPC_MMUCSR0_L2TLB0_FI)
    ADDI        R3 , R3 , LO(ARCH_PPC_MMUCSR0_L2TLB0_FI)
    ISYNC
    MTSPR       MMUCSR0 , R3
    ISYNC
    TLBSYNC
    ISYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  无效指定 EA 的 TLB
;*********************************************************************************************************/

FUNC_DEF(ppcE500MmuInvalidateTLBEA)
    ORI         R3 , R3 , INV_TLB0
    ISYNC
    TLBIVAX     R0 , R3
    ISYNC
    MSYNC
    TLBSYNC
    ISYNC
    BLR
    FUNC_END()

;/*********************************************************************************************************
;  异常处理程序都放在 .vector 节区
;*********************************************************************************************************/

    SECTION(.vector)

;/*********************************************************************************************************
;  TLB 错误异常入口宏
;*********************************************************************************************************/

MACRO_DEF(E500_TLB_ERROR_ENTRY, addr_reg, exc_entry)
    MTSPR       SPRG4_W , R3                ;/*  用 SPRG4, 5, 6, 7 备份 R3 , R4 , R5, CR                 */
    MTSPR       SPRG5_W , R4
    MTSPR       SPRG6_W , R5
    MFCR        R3
    MTSPR       SPRG7_W , R3

    MFSPR       R3 , \addr_reg              ;/*  R3 = ulMissAddr;                                        */

    RLWINM      R3 , R3 , 20 , 12 , 31      ;/*  R3 = uiEPN = ulMissAddr >> LW_CFG_VMM_PAGE_SHIFT;       */

#if LW_CFG_CPU_PHYS_ADDR_64BIT > 0
    RLWINM      R3 , R3 , 3  , 0  , 28      ;/*  R3 = uiEPN * sizeof(LW_PTE_TRANSENTRY);                 */
#else
    RLWINM      R3 , R3 , 2  , 0  , 29      ;/*  R3 = uiEPN * sizeof(LW_PTE_TRANSENTRY);                 */
#endif

    MFSPR       R4 , SPRG3                  ;/*  R4 = _G_ulPTETable;                                     */

    ADD         R3 , R4 , R3                ;/*  R3 = p_pteentry = (_G_ulPTETable +                      */
                                            ;/*  uiEPN * sizeof(LW_PTE_TRANSENTRY));                     */

    MFSPR       R4 , SPRG2                  ;/*  R4 = _G_bHasMAS7;                                       */
    CMPWI       CR7 , R4 , 0                ;/*  if (!_G_bHasMAS7)                                       */
    BEQ         CR7 , 1f                    ;/*  then goto 1;                                            */

#if LW_CFG_CPU_PHYS_ADDR_64BIT > 0
    LWZ         R4 , 4(R3)                  ;/*  R4 = *(p_pteentry + 4);                                 */
#else
    LI          R4 , 0                      ;/*  R4 = 0;                                                 */
#endif
    MTSPR       MAS7 , R4                   ;/*  MAS7 = R4;                                              */

LINE_LABEL(1)
    LWZ         R3 , 0(R3)                  ;/*  R3 = *p_pteentry;                                       */

    RLWINM      R4  , R3 , 0 , 22 , 22      ;/*  R4 = pteentry.MAS3_bValid;                              */

    CMPWI       CR7 , R4 , 0                ;/*  if (!pteentry.MAS3_bValid)                              */
    BEQ         CR7 , 2f                    ;/*  then goto 2;                                            */

    MFSPR       R4 , MAS2                   ;/*  R4 = MAS2;                                              */

    RLWINM      R4 , R4 , 0  , 0  , 26      ;/*  去掉 MAS2 MAS2_bWT 及后面的内容                         */

    RLWINM      R5 , R3 , 28 , 27 , 29      ;/*  PTE 右移 4 位, 并取出 bWT bUnCache bMemCoh              */

    OR          R4 , R5 , R4                ;/*  合并成真正的 MAS2                                       */

    MTSPR       MAS2 , R4                   ;/*  设置 MAS2                                               */

    MTSPR       MAS3 , R3                   ;/*  设置 MAS3                                               */

    ISYNC
    SYNC
    TLBWE
    ISYNC

    MFSPR       R3 , SPRG7_R                ;/*  从 SPRG4, 5, 6 恢复 R3 , R4 , R5                        */
    MTCR        R3
    MFSPR       R5 , SPRG6_R
    MFSPR       R4 , SPRG5_R
    MFSPR       R3 , SPRG4_R

    RFI                                     ;/*  从 SRR0 返回，同时 MSR=SRR1                             */

LINE_LABEL(2)
    MFSPR       R3 , SPRG7_R                ;/*  从 SPRG4, 5, 6, 7 恢复 R3 , R4 , R5, CR                 */
    MTCR        R3
    MFSPR       R5 , SPRG6_R
    MFSPR       R4 , SPRG5_R
    MFSPR       R3 , SPRG4_R

    B           \exc_entry                  ;/*  PTE 无效, 跳转到异常处理                                */
    MACRO_END()

;/*********************************************************************************************************
;  数据 TLB 错误异常入口(Vector IVOR13)
;*********************************************************************************************************/

EXCE_DEF(archE500DataTLBErrorEntry)
    E500_TLB_ERROR_ENTRY    DEAR , archE500DataStorageExceptionEntry
    EXCE_END()

;/*********************************************************************************************************
;  指令 TLB 错误异常入口(Vector IVOR14)
;*********************************************************************************************************/

EXCE_DEF(archE500InstructionTLBErrorEntry)
    E500_TLB_ERROR_ENTRY    SRR0 , archE500InstructionStorageExceptionEntry
    EXCE_END()

    FILE_END()

#endif
;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
