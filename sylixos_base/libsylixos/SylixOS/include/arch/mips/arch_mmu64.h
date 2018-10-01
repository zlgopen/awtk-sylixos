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
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: MIPS64 内存管理相关.
*********************************************************************************************************/

#ifndef __MIPS_ARCH_MMU64_H
#define __MIPS_ARCH_MMU64_H

/*********************************************************************************************************
  L4 微内核虚拟机 MMU
*********************************************************************************************************/

#define LW_CFG_VMM_L4_HYPERVISOR_EN           0                         /*  是否使用 L4 虚拟机 MMU      */

/*********************************************************************************************************
  是否需要内核超过 3 级页表支持
*********************************************************************************************************/

#define LW_CFG_VMM_PAGE_4L_EN                 1                         /*  是否需要 4 级页表支持       */

/*********************************************************************************************************
  虚拟内存页表相关配置
*********************************************************************************************************/

#define LW_CFG_VMM_PAGE_SHIFT                 LW_CFG_MIPS_PAGE_SHIFT    /*  2^n                         */
#define LW_CFG_VMM_PAGE_SIZE                  (__CONST64(1) << LW_CFG_VMM_PAGE_SHIFT)
#define LW_CFG_VMM_PAGE_MASK                  (~(LW_CFG_VMM_PAGE_SIZE - 1))

#define LW_CFG_VMM_PTE_SHIFT                  LW_CFG_VMM_PAGE_SHIFT
#define LW_CFG_VMM_PTE_SIZE                   LW_CFG_VMM_PAGE_SIZE
#define LW_CFG_VMM_PTE_BLKSIZE                LW_CFG_VMM_PAGE_SIZE

/*********************************************************************************************************
 * 2^9 = 512 , PTE BLOCK SIZE = 512 * sizeof(PTE) = 512 * 8 = 4096 = PAGE SIZE
 *
 * PTE BLOCK SIZE = PAGE SIZE, 方便将虚拟页表页面映射到 PTE BLOCK
 *
 * +------------+------------+------------+------------+------------+
 * |6          4|4          3|3          2|2          1|1          0|
 * |3    15b   9|8    14b   5|4   14b    1|0    9b    2|1    12b   0|
 * +----------------------------------------------------------------+
 * |   PGD      |    PMD     |    PTS     |    PTE     |   OFFSET   |
 * +------------+------------+------------+------------+------------+
*********************************************************************************************************/
#if LW_CFG_MIPS_PAGE_SHIFT == 12

#define LW_CFG_VMM_PTE_MASK                   (__CONST64(0x1ff) << LW_CFG_VMM_PAGE_SHIFT)

#define LW_CFG_VMM_PTS_SHIFT                  21
#define LW_CFG_VMM_PTS_SIZE                   (__CONST64(1) << LW_CFG_VMM_PTS_SHIFT)
#define LW_CFG_VMM_PTS_MASK                   (__CONST64(0x3fff) << LW_CFG_VMM_PTS_SHIFT)
#define LW_CFG_VMM_PTS_BLKSIZE                (128 * LW_CFG_KB_SIZE)

#define LW_CFG_VMM_PMD_SHIFT                  35
#define LW_CFG_VMM_PMD_SIZE                   (__CONST64(1) << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_MASK                   (__CONST64(0x3fff) << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_BLKSIZE                (128 * LW_CFG_KB_SIZE)

#define LW_CFG_VMM_PGD_SHIFT                  50
#define LW_CFG_VMM_PGD_SIZE                   (__CONST64(1) << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_MASK                   (__CONST64(0x7fff) << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_BLKSIZE                (256 * LW_CFG_KB_SIZE)

/*********************************************************************************************************
 * 2^11 = 2048 , PTE BLOCK SIZE = 2048 * sizeof(PTE) = 2048 * 8 = 16384 = PAGE SIZE
 *
 * +------------+------------+------------+------------+------------+
 * |6          5|5          3|3          2|2          1|1          0|
 * |3    13b   1|0    13b   8|7   13b    5|4    11b   4|3    14b   0|
 * +----------------------------------------------------------------+
 * |   PGD      |    PMD     |    PTS     |    PTE     |   OFFSET   |
 * +------------+------------+------------+------------+------------+
*********************************************************************************************************/
#elif LW_CFG_MIPS_PAGE_SHIFT == 14

#define LW_CFG_VMM_PTE_MASK                   (__CONST64(0x7ff) << LW_CFG_VMM_PAGE_SHIFT)

#define LW_CFG_VMM_PTS_SHIFT                  25
#define LW_CFG_VMM_PTS_SIZE                   (__CONST64(1) << LW_CFG_VMM_PTS_SHIFT)
#define LW_CFG_VMM_PTS_MASK                   (__CONST64(0x1fff) << LW_CFG_VMM_PTS_SHIFT)
#define LW_CFG_VMM_PTS_BLKSIZE                (64 * LW_CFG_KB_SIZE)

#define LW_CFG_VMM_PMD_SHIFT                  38
#define LW_CFG_VMM_PMD_SIZE                   (__CONST64(1) << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_MASK                   (__CONST64(0x1fff) << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_BLKSIZE                (64 * LW_CFG_KB_SIZE)

#define LW_CFG_VMM_PGD_SHIFT                  51
#define LW_CFG_VMM_PGD_SIZE                   (__CONST64(1) << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_MASK                   (__CONST64(0x1fff) << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_BLKSIZE                (64 * LW_CFG_KB_SIZE)

/*********************************************************************************************************
 * 2^13 = 8192 , PTE BLOCK SIZE = 8192 * sizeof(PTE) = 8192 * 8 = 65536 = PAGE SIZE
 *
 * +------------+------------+------------+------------+------------+
 * |6          5|5          4|3          2|2          1|1          0|
 * |3    12b   2|1    12b   0|9   11b    9|8    13b   6|5    16b   0|
 * +----------------------------------------------------------------+
 * |   PGD      |    PMD     |    PTS     |    PTE     |   OFFSET   |
 * +------------+------------+------------+------------+------------+
*********************************************************************************************************/
#elif LW_CFG_MIPS_PAGE_SHIFT == 16

#define LW_CFG_VMM_PTE_MASK                   (__CONST64(0x1fff) << LW_CFG_VMM_PAGE_SHIFT)

#define LW_CFG_VMM_PTS_SHIFT                  29
#define LW_CFG_VMM_PTS_SIZE                   (__CONST64(1) << LW_CFG_VMM_PTS_SHIFT)
#define LW_CFG_VMM_PTS_MASK                   (__CONST64(0x7ff) << LW_CFG_VMM_PTS_SHIFT)
#define LW_CFG_VMM_PTS_BLKSIZE                (16 * LW_CFG_KB_SIZE)

#define LW_CFG_VMM_PMD_SHIFT                  40
#define LW_CFG_VMM_PMD_SIZE                   (__CONST64(1) << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_MASK                   (__CONST64(0xfff) << LW_CFG_VMM_PMD_SHIFT)
#define LW_CFG_VMM_PMD_BLKSIZE                (32 * LW_CFG_KB_SIZE)

#define LW_CFG_VMM_PGD_SHIFT                  52
#define LW_CFG_VMM_PGD_SIZE                   (__CONST64(1) << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_MASK                   (__CONST64(0xfff) << LW_CFG_VMM_PGD_SHIFT)
#define LW_CFG_VMM_PGD_BLKSIZE                (32 * LW_CFG_KB_SIZE)

#else
#error  LW_CFG_VMM_PAGE_SIZE must be (4K, 16K, 64K)!
#endif

/*********************************************************************************************************
  内存分组数量
*********************************************************************************************************/
#ifdef __SYLIXOS_KERNEL

#define LW_CFG_VMM_ZONE_NUM                   8                         /*  物理分区数                  */
#define LW_CFG_VMM_VIR_NUM                    8                         /*  虚拟分区数                  */

/*********************************************************************************************************
  MMU 转换条目类型
*********************************************************************************************************/
#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))

typedef UINT64  LW_PGD_TRANSENTRY;                                      /*  页目录类型                  */
typedef UINT64  LW_PMD_TRANSENTRY;                                      /*  中间页目录类型              */
typedef UINT64  LW_PTS_TRANSENTRY;                                      /*  中间页目录类型              */
typedef UINT64  LW_PTE_TRANSENTRY;                                      /*  页表条目类型                */

#endif
#endif                                                                  /*  __SYLIXOS_KERNEL            */
#endif                                                                  /*  __MIPS_ARCH_MMU64_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
