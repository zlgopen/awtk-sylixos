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
** 文   件   名: arch_mmu64.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: RISC-V64 内存管理相关.
*********************************************************************************************************/

#ifndef __RISCV_ARCH_MMU64_H
#define __RISCV_ARCH_MMU64_H

/*********************************************************************************************************
  L4 微内核虚拟机 MMU
*********************************************************************************************************/

#define LW_CFG_VMM_L4_HYPERVISOR_EN           0                         /*  是否使用 L4 虚拟机 MMU      */

/*********************************************************************************************************
  是否需要内核超过 3 级页表支持
*********************************************************************************************************/

#if LW_CFG_RISCV_MMU_SV39 > 0
#define LW_CFG_VMM_PAGE_4L_EN                 0                         /*  sv39 需要 3 级页表支持      */
#else
#define LW_CFG_VMM_PAGE_4L_EN                 1                         /*  sv48 需要 4 级页表支持      */
#endif

/*********************************************************************************************************
  虚拟内存页表相关配置
*********************************************************************************************************/

#define LW_CFG_VMM_PAGE_SHIFT                 12                        /*  2^12 = 4096                 */
#define LW_CFG_VMM_PAGE_SIZE                  (1ul << LW_CFG_VMM_PAGE_SHIFT)
#define LW_CFG_VMM_PAGE_MASK                  (~(LW_CFG_VMM_PAGE_SIZE - 1))

#if LW_CFG_RISCV_MMU_SV39 > 0
#define LW_CFG_VMM_PMD_SHIFT                  21
#define LW_CFG_VMM_PMD_SIZE                   (1ul << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_MASK                   (0x1fful << LW_CFG_VMM_PMD_SHIFT)

#define LW_CFG_VMM_PGD_SHIFT                  30
#define LW_CFG_VMM_PGD_SIZE                   (1ul << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_MASK                   (0x1fful << LW_CFG_VMM_PGD_SHIFT)
#else
#define LW_CFG_VMM_PTS_SHIFT                  21
#define LW_CFG_VMM_PTS_SIZE                   (1ul << LW_CFG_VMM_PTS_SHIFT)
#define LW_CFG_VMM_PTS_MASK                   (0x1fful << LW_CFG_VMM_PTS_SHIFT)

#define LW_CFG_VMM_PMD_SHIFT                  30
#define LW_CFG_VMM_PMD_SIZE                   (1ul << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_MASK                   (0x1fful << LW_CFG_VMM_PMD_SHIFT)

#define LW_CFG_VMM_PGD_SHIFT                  39
#define LW_CFG_VMM_PGD_SIZE                   (1ul << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_MASK                   (0x1fful << LW_CFG_VMM_PGD_SHIFT)
#endif

/*********************************************************************************************************
  内存分组数量
*********************************************************************************************************/
#ifdef __SYLIXOS_KERNEL

#define LW_CFG_VMM_ZONE_NUM                   16                        /*  物理分区数                  */
#define LW_CFG_VMM_VIR_NUM                    8                         /*  虚拟分区数                  */

/*********************************************************************************************************
  MMU 转换条目类型
*********************************************************************************************************/
#if !defined(__ASSEMBLY__) && !defined(ASSEMBLY)

typedef UINT64  LW_PGD_TRANSENTRY;                                      /*  PGD 页目录类型              */
typedef UINT64  LW_PMD_TRANSENTRY;                                      /*  PMD 页目录类型              */
#if LW_CFG_RISCV_MMU_SV39 == 0
typedef UINT64  LW_PTS_TRANSENTRY;                                      /*  PTS 页目录类型              */
#endif                                                                  /*  LW_CFG_RISCV_MMU_SV39 == 0  */
typedef UINT64  LW_PTE_TRANSENTRY;                                      /*  页表条目类型                */

#endif
#endif                                                                  /*  __SYLIXOS_KERNEL            */
#endif                                                                  /*  __RISCV_ARCH_MMU64_H        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
