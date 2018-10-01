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
** 文   件   名: mips64Mmu.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 11 月 30 日
**
** 描        述: MIPS64 体系构架 MMU 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0
#if LW_CFG_CPU_WORD_LENGHT == 64
#include "arch/mips/common/cp0/mipsCp0.h"
#include "../mipsMmuCommon.h"
#include "./mips64MmuAlgorithm.h"
/*********************************************************************************************************
  ENTRYLO PFN 掩码
*********************************************************************************************************/
#define MIPS64_ENTRYLO_PFN_MASK         (0xfffffffffull << MIPS_ENTRYLO_PFN_SHIFT)
/*********************************************************************************************************
  全局变量
*********************************************************************************************************/
static LW_OBJECT_HANDLE     _G_hPGDPartition;                           /*  系统目前仅使用一个 PGD      */
static LW_OBJECT_HANDLE     _G_hPMDPartition;                           /*  PMD 缓冲区                  */
static LW_OBJECT_HANDLE     _G_hPTSPartition;                           /*  PTS 缓冲区                  */
static LW_OBJECT_HANDLE     _G_hPTEPartition;                           /*  PTE 缓冲区                  */
/*********************************************************************************************************
** 函数名称: mips64MmuBuildPgdEntry
** 功能描述: 生成一个一级描述符 (PGD 描述符)
** 输　入  : ulBaseAddr              二级页表基地址
** 输　出  : 一级描述符
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE LW_PGD_TRANSENTRY  mips64MmuBuildPgdEntry (addr_t  ulBaseAddr)
{
    return  (ulBaseAddr);                                               /*  一级描述符就是二级页表基地址*/
}
/*********************************************************************************************************
** 函数名称: mips64MmuBuildPmdEntry
** 功能描述: 生成一个二级描述符 (PMD 描述符)
** 输　入  : ulBaseAddr              三级页表基地址
** 输　出  : 二级描述符
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE LW_PMD_TRANSENTRY  mips64MmuBuildPmdEntry (addr_t  ulBaseAddr)
{
    return  (ulBaseAddr);                                               /*  二级描述符就是三级页表基地址*/
}
/*********************************************************************************************************
** 函数名称: mips64MmuBuildPtsEntry
** 功能描述: 生成一个三级描述符 (PTS 描述符)
** 输　入  : ulBaseAddr              四级页表基地址
** 输　出  : 三级描述符
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE LW_PTS_TRANSENTRY  mips64MmuBuildPtsEntry (addr_t  ulBaseAddr)
{
    return  (ulBaseAddr);                                               /*  三级描述符就是四级页表基地址*/
}
/*********************************************************************************************************
** 函数名称: mips64MmuBuildPteEntry
** 功能描述: 生成一个四级描述符 (PTE 描述符)
** 输　入  : ulBaseAddr              物理页地址
**           ulFlag                  标志
** 输　出  : 四级描述符
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PTE_TRANSENTRY  mips64MmuBuildPteEntry (addr_t  ulBaseAddr, ULONG  ulFlag)
{
    LW_PTE_TRANSENTRY   pteentry;
    ULONG               ulPFN;

    if (ulFlag & LW_VMM_FLAG_ACCESS) {
        ulPFN = ulBaseAddr >> 12;                                       /*  计算 PFN                    */

        pteentry = ulPFN << MIPS_ENTRYLO_PFN_SHIFT;                     /*  填充 PFN                    */

        if (ulFlag & LW_VMM_FLAG_VALID) {
            pteentry |= ENTRYLO_V;                                      /*  填充 V 位                   */
        }

        if (ulFlag & LW_VMM_FLAG_WRITABLE) {
            pteentry |= ENTRYLO_D;                                      /*  填充 D 位                   */
        }

        pteentry |= ENTRYLO_G;                                          /*  填充 G 位                   */

        if (ulFlag & LW_VMM_FLAG_CACHEABLE) {                           /*  填充 C 位                   */
            pteentry |= MIPS_MMU_ENTRYLO_CACHE   << ENTRYLO_C_SHIFT;
        } else {
            pteentry |= MIPS_MMU_ENTRYLO_UNCACHE << ENTRYLO_C_SHIFT;
        }

        if (MIPS_MMU_HAS_XI) {
            if (!(ulFlag & LW_VMM_FLAG_EXECABLE)) {                     /*  不可执行                    */
                pteentry |= MIPS_ENTRYLO_XI;                            /*  填充 XI 位                  */
            }
        }
    } else {
        pteentry = 0;
    }

    return  (pteentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuMemInit
** 功能描述: 初始化 MMU 页表内存区
** 输　入  : pmmuctx        mmu 上下文
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  mips64MmuMemInit (PLW_MMU_CONTEXT  pmmuctx)
{
    PVOID  pvPgdTable;
    PVOID  pvPmdTable;
    PVOID  pvPtsTable;
    PVOID  pvPteTable;

    ULONG  ulPgdNum = bspMmuPgdMaxNum();
    ULONG  ulPmdNum = bspMmuPmdMaxNum();
    ULONG  ulPtsNum = bspMmuPtsMaxNum();
    ULONG  ulPteNum = bspMmuPteMaxNum();

    pvPgdTable = __KHEAP_ALLOC_ALIGN((size_t)ulPgdNum * LW_CFG_VMM_PGD_BLKSIZE, LW_CFG_VMM_PGD_BLKSIZE);
    pvPmdTable = __KHEAP_ALLOC_ALIGN((size_t)ulPmdNum * LW_CFG_VMM_PMD_BLKSIZE, LW_CFG_VMM_PMD_BLKSIZE);
    pvPtsTable = __KHEAP_ALLOC_ALIGN((size_t)ulPtsNum * LW_CFG_VMM_PTS_BLKSIZE, LW_CFG_VMM_PTS_BLKSIZE);
    pvPteTable = __KHEAP_ALLOC_ALIGN((size_t)ulPteNum * LW_CFG_VMM_PTE_BLKSIZE, LW_CFG_VMM_PTE_BLKSIZE);

    if (!pvPgdTable || !pvPmdTable || !pvPtsTable || !pvPteTable) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "can not allocate page table.\r\n");
        return  (PX_ERROR);
    }

    _G_hPGDPartition = API_PartitionCreate("pgd_pool", pvPgdTable, ulPgdNum, LW_CFG_VMM_PGD_BLKSIZE,
                                           LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    _G_hPMDPartition = API_PartitionCreate("pmd_pool", pvPmdTable, ulPmdNum, LW_CFG_VMM_PMD_BLKSIZE,
                                           LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    _G_hPTSPartition = API_PartitionCreate("pts_pool", pvPtsTable, ulPtsNum, LW_CFG_VMM_PTS_BLKSIZE,
                                           LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    _G_hPTEPartition = API_PartitionCreate("pte_pool", pvPteTable, ulPteNum, LW_CFG_VMM_PTE_BLKSIZE,
                                           LW_OPTION_OBJECT_GLOBAL, LW_NULL);

    if (!_G_hPGDPartition || !_G_hPMDPartition || !_G_hPTSPartition || !_G_hPTEPartition) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "can not allocate page pool.\r\n");
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPgdOffset
** 功能描述: 通过虚拟地址计算 PGD 项
** 输　入  : pmmuctx        mmu 上下文
**           ulAddr         虚拟地址
** 输　出  : 对应的 PGD 表项地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PGD_TRANSENTRY  *mips64MmuPgdOffset (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulAddr)
{
    REGISTER LW_PGD_TRANSENTRY  *p_pgdentry = pmmuctx->MMUCTX_pgdEntry;
    REGISTER ULONG               ulPgdNum;

    ulAddr    &= LW_CFG_VMM_PGD_MASK;
    ulPgdNum   = ulAddr >> LW_CFG_VMM_PGD_SHIFT;                        /*  计算 PGD 号                 */

    p_pgdentry = (LW_PGD_TRANSENTRY *)((addr_t)p_pgdentry |
                 (ulPgdNum * sizeof(LW_PGD_TRANSENTRY)));               /*  获得一级页表描述符地址      */

    return  (p_pgdentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPmdOffset
** 功能描述: 通过虚拟地址计算 PMD 项
** 输　入  : p_pgdentry     pgd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 对应的 PMD 表项地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PMD_TRANSENTRY  *mips64MmuPmdOffset (LW_PGD_TRANSENTRY  *p_pgdentry, addr_t  ulAddr)
{
    REGISTER LW_PMD_TRANSENTRY  *p_pmdentry;
    REGISTER LW_PGD_TRANSENTRY   ulTemp;
    REGISTER ULONG               ulPmdNum;

    ulTemp = *p_pgdentry;                                               /*  获得一级页表描述符          */

    p_pmdentry = (LW_PMD_TRANSENTRY *)(ulTemp);                         /*  获得二级页表基地址          */

    ulAddr    &= LW_CFG_VMM_PMD_MASK;
    ulPmdNum   = ulAddr >> LW_CFG_VMM_PMD_SHIFT;                        /*  计算 PMD 号                 */

    p_pmdentry = (LW_PMD_TRANSENTRY *)((addr_t)p_pmdentry |
                 (ulPmdNum * sizeof(LW_PMD_TRANSENTRY)));               /*  获得二级页表描述符地址      */

    return  (p_pmdentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPtsOffset
** 功能描述: 通过虚拟地址计算 PTS 项
** 输　入  : p_pmdentry     pmd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 对应的 PTS 表项地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PTS_TRANSENTRY  *mips64MmuPtsOffset (LW_PMD_TRANSENTRY  *p_pmdentry, addr_t  ulAddr)
{
    REGISTER LW_PTS_TRANSENTRY  *p_ptsentry;
    REGISTER LW_PMD_TRANSENTRY   ulTemp;
    REGISTER ULONG               ulPtsNum;

    ulTemp = *p_pmdentry;                                               /*  获得二级页表描述符          */

    p_ptsentry = (LW_PTS_TRANSENTRY *)(ulTemp);                         /*  获得三级页表基地址          */

    ulAddr    &= LW_CFG_VMM_PTS_MASK;
    ulPtsNum   = ulAddr >> LW_CFG_VMM_PTS_SHIFT;                        /*  计算 PTS 号                 */

    p_ptsentry = (LW_PTS_TRANSENTRY *)((addr_t)p_ptsentry |
                 (ulPtsNum * sizeof(LW_PTS_TRANSENTRY)));               /*  获得三级页表描述符地址      */

    return  (p_ptsentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPteOffset
** 功能描述: 通过虚拟地址计算 PTE 项
** 输　入  : p_ptsentry     pts 入口地址
**           ulAddr         虚拟地址
** 输　出  : 对应的 PTE 表项地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static  LW_PTE_TRANSENTRY  *mips64MmuPteOffset (LW_PTS_TRANSENTRY  *p_ptsentry, addr_t  ulAddr)
{
    REGISTER LW_PTE_TRANSENTRY  *p_pteentry;
    REGISTER LW_PTS_TRANSENTRY   ulTemp;
    REGISTER ULONG               ulPageNum;

    ulTemp = *p_ptsentry;                                               /*  获得三级页表描述符          */

    p_pteentry = (LW_PTE_TRANSENTRY *)(ulTemp);                         /*  获得四级页表基地址          */

    ulAddr    &= LW_CFG_VMM_PTE_MASK;                                   /*  不要使用LW_CFG_VMM_PAGE_MASK*/
    ulPageNum  = ulAddr >> LW_CFG_VMM_PTE_SHIFT;                        /*  计算段内页号                */

    p_pteentry = (LW_PTE_TRANSENTRY *)((addr_t)p_pteentry |
                 (ulPageNum * sizeof(LW_PTE_TRANSENTRY)));              /*  获得虚拟地址页表描述符地址  */

    return  (p_pteentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPgdIsOk
** 功能描述: 判断 PGD 项的描述符是否正确
** 输　入  : pgdentry       PGD 项描述符
** 输　出  : 是否正确
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  mips64MmuPgdIsOk (LW_PGD_TRANSENTRY  pgdentry)
{
    return  (pgdentry ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPmdIsOk
** 功能描述: 判断 PMD 项的描述符是否正确
** 输　入  : pmdentry       PMD 项描述符
** 输　出  : 是否正确
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  mips64MmuPmdIsOk (LW_PMD_TRANSENTRY  pmdentry)
{
    return  (pmdentry ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPtsIsOk
** 功能描述: 判断 PTS 项的描述符是否正确
** 输　入  : ptsentry       PTS 项描述符
** 输　出  : 是否正确
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  mips64MmuPtsIsOk (LW_PTS_TRANSENTRY  ptsentry)
{
    return  (ptsentry ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPteIsOk
** 功能描述: 判断 PTE 项的描述符是否正确
** 输　入  : pteentry       PTE 项描述符
** 输　出  : 是否正确
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  mips64MmuPteIsOk (LW_PTE_TRANSENTRY  pteentry)
{
    return  ((pteentry & ENTRYLO_V) ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPgdAlloc
** 功能描述: 分配 PGD 项
** 输　入  : pmmuctx        mmu 上下文
**           ulAddr         虚拟地址 (参数 0 即偏移量为 0 , 需要返回页表基地址)
** 输　出  : 分配 PGD 地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PGD_TRANSENTRY  *mips64MmuPgdAlloc (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulAddr)
{
    REGISTER LW_PGD_TRANSENTRY  *p_pgdentry;
    REGISTER ULONG               ulPgdNum;

    p_pgdentry = (LW_PGD_TRANSENTRY *)API_PartitionGet(_G_hPGDPartition);
    if (!p_pgdentry) {
        return  (LW_NULL);
    }

    lib_bzero(p_pgdentry, LW_CFG_VMM_PGD_BLKSIZE);

    ulAddr    &= LW_CFG_VMM_PGD_MASK;
    ulPgdNum   = ulAddr >> LW_CFG_VMM_PGD_SHIFT;                        /*  计算 PGD 号                 */

    p_pgdentry = (LW_PGD_TRANSENTRY *)((addr_t)p_pgdentry |
                 (ulPgdNum * sizeof(LW_PGD_TRANSENTRY)));               /*  获得一级页表描述符地址      */

    return  (p_pgdentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPgdFree
** 功能描述: 释放 PGD 项
** 输　入  : p_pgdentry     pgd 入口地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mips64MmuPgdFree (LW_PGD_TRANSENTRY  *p_pgdentry)
{
    p_pgdentry = (LW_PGD_TRANSENTRY *)((addr_t)p_pgdentry & (~(LW_CFG_VMM_PGD_BLKSIZE - 1)));

    API_PartitionPut(_G_hPGDPartition, (PVOID)p_pgdentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPmdAlloc
** 功能描述: 分配 PMD 项
** 输　入  : pmmuctx        mmu 上下文
**           p_pgdentry     pgd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 分配 PMD 地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PMD_TRANSENTRY  *mips64MmuPmdAlloc (PLW_MMU_CONTEXT     pmmuctx,
                                              LW_PGD_TRANSENTRY  *p_pgdentry,
                                              addr_t              ulAddr)
{
    LW_PMD_TRANSENTRY  *p_pmdentry = (LW_PMD_TRANSENTRY *)API_PartitionGet(_G_hPMDPartition);

    if (!p_pmdentry) {
        return  (LW_NULL);
    }

    lib_bzero(p_pmdentry, LW_CFG_VMM_PMD_BLKSIZE);

    *p_pgdentry = mips64MmuBuildPgdEntry((addr_t)p_pmdentry);           /*  设置一级页表描述符          */

    return  (mips64MmuPmdOffset(p_pgdentry, ulAddr));
}
/*********************************************************************************************************
** 函数名称: mips64MmuPmdFree
** 功能描述: 释放 PMD 项
** 输　入  : p_pmdentry     pmd 入口地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mips64MmuPmdFree (LW_PMD_TRANSENTRY  *p_pmdentry)
{
    p_pmdentry = (LW_PMD_TRANSENTRY *)((addr_t)p_pmdentry & (~(LW_CFG_VMM_PMD_BLKSIZE - 1)));

    API_PartitionPut(_G_hPMDPartition, (PVOID)p_pmdentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPtsAlloc
** 功能描述: 分配 PTS 项
** 输　入  : pmmuctx        mmu 上下文
**           p_pmdentry     pmd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 分配 PTS 地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PTS_TRANSENTRY  *mips64MmuPtsAlloc (PLW_MMU_CONTEXT     pmmuctx,
                                              LW_PMD_TRANSENTRY  *p_pmdentry,
                                              addr_t              ulAddr)
{
    LW_PTS_TRANSENTRY  *p_ptsentry = (LW_PTS_TRANSENTRY *)API_PartitionGet(_G_hPTSPartition);

    if (!p_ptsentry) {
        return  (LW_NULL);
    }

    lib_bzero(p_ptsentry, LW_CFG_VMM_PTS_BLKSIZE);

    *p_pmdentry = mips64MmuBuildPmdEntry((addr_t)p_ptsentry);           /*  设置二级页表描述符          */

    return  (mips64MmuPtsOffset(p_pmdentry, ulAddr));
}
/*********************************************************************************************************
** 函数名称: mips64MmuPtsFree
** 功能描述: 释放 PTS 项
** 输　入  : p_ptsentry     pts 入口地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mips64MmuPtsFree (LW_PTS_TRANSENTRY  *p_ptsentry)
{
    p_ptsentry = (LW_PTS_TRANSENTRY *)((addr_t)p_ptsentry & (~(LW_CFG_VMM_PTS_BLKSIZE - 1)));

    API_PartitionPut(_G_hPTSPartition, (PVOID)p_ptsentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPteAlloc
** 功能描述: 分配 PTE 项
** 输　入  : pmmuctx        mmu 上下文
**           p_pmdentry     pmd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 分配 PTE 地址
** 全局变量:
** 调用模块: VMM 这里没有关闭中断, 回写 CACHE 时, 需要手动关中断, SylixOS 映射完毕会自动清快表, 所以
             这里不用清除快表.
*********************************************************************************************************/
static LW_PTE_TRANSENTRY  *mips64MmuPteAlloc (PLW_MMU_CONTEXT     pmmuctx,
                                              LW_PTS_TRANSENTRY  *p_ptsentry,
                                              addr_t              ulAddr)
{
    LW_PTE_TRANSENTRY  *p_pteentry = (LW_PTE_TRANSENTRY *)API_PartitionGet(_G_hPTEPartition);

    if (!p_pteentry) {
        return  (LW_NULL);
    }

    lib_bzero(p_pteentry, LW_CFG_VMM_PTE_BLKSIZE);

    *p_ptsentry = mips64MmuBuildPtsEntry((addr_t)p_pteentry);           /*  设置三级页表描述符          */

    return  (mips64MmuPteOffset(p_ptsentry, ulAddr));
}
/*********************************************************************************************************
** 函数名称: mips64MmuPteFree
** 功能描述: 释放 PTE 项
** 输　入  : p_pteentry     pte 入口地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mips64MmuPteFree (LW_PTE_TRANSENTRY  *p_pteentry)
{
    p_pteentry = (LW_PTE_TRANSENTRY *)((addr_t)p_pteentry & (~(LW_CFG_VMM_PTE_BLKSIZE - 1)));

    API_PartitionPut(_G_hPTEPartition, (PVOID)p_pteentry);
}
/*********************************************************************************************************
** 函数名称: mips64MmuPtePhysGet
** 功能描述: 通过 PTE 表项, 查询物理地址
** 输　入  : pteentry           pte 表项
**           pulPhysicalAddr    获得的物理地址
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  mips64MmuPtePhysGet (LW_PTE_TRANSENTRY  pteentry, addr_t  *pulPhysicalAddr)
{
    ULONG  ulPFN = (pteentry & MIPS64_ENTRYLO_PFN_MASK) >>
                    MIPS_ENTRYLO_PFN_SHIFT;                             /*  获得物理页面号              */

    *pulPhysicalAddr = ulPFN << 12;                                     /*  计算页面物理地址            */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mips64MmuFlagGet
** 功能描述: 获得指定虚拟地址的 SylixOS 权限标志
** 输　入  : pmmuctx        mmu 上下文
**           ulAddr         虚拟地址
** 输　出  : SylixOS 权限标志
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static ULONG  mips64MmuFlagGet (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulAddr)
{
    LW_PGD_TRANSENTRY  *p_pgdentry;
    LW_PMD_TRANSENTRY  *p_pmdentry;
    LW_PTS_TRANSENTRY  *p_ptsentry;
    LW_PTE_TRANSENTRY  *p_pteentry;
    LW_PTE_TRANSENTRY   pteentry;

    p_pgdentry = mips64MmuPgdOffset(pmmuctx, ulAddr);                   /*  获得一级描述符地址          */
    if (p_pgdentry && mips64MmuPgdIsOk(*p_pgdentry)) {                  /*  一级描述符有效              */

        p_pmdentry = mips64MmuPmdOffset(p_pgdentry, ulAddr);            /*  获得二级描述符地址          */
        if (mips64MmuPmdIsOk(*p_pmdentry)) {                            /*  二级描述符有效              */

            p_ptsentry = mips64MmuPtsOffset(p_pmdentry, ulAddr);        /*  获得三级描述符地址          */
            if (mips64MmuPtsIsOk(*p_ptsentry)) {                        /*  三级描述符有效              */

                p_pteentry = mips64MmuPteOffset(p_ptsentry, ulAddr);    /*  获得四级描述符地址          */
                pteentry   = *p_pteentry;                               /*  获得四级描述符              */
                if (mips64MmuPteIsOk(pteentry)) {                       /*  四级描述符有效              */
                    ULONG   ulFlag = 0;
                    ULONG   ulCacheAttr;

                    if (pteentry & ENTRYLO_V) {                         /*  有效                        */
                        ulFlag |= LW_VMM_FLAG_VALID;                    /*  映射有效                    */
                    }

                    ulFlag |= LW_VMM_FLAG_ACCESS;                       /*  可以访问                    */

                    if (MIPS_MMU_HAS_XI) {
                        if (!(pteentry & MIPS_ENTRYLO_XI)) {
                            ulFlag |= LW_VMM_FLAG_EXECABLE;             /*  可以执行                    */
                        }
                    } else {
                        ulFlag |= LW_VMM_FLAG_EXECABLE;                 /*  可以执行                    */
                    }

                    if (pteentry & ENTRYLO_D) {                         /*  可写                        */
                        ulFlag |= LW_VMM_FLAG_WRITABLE;
                    }

                    ulCacheAttr = (pteentry & ENTRYLO_C)
                                  >> ENTRYLO_C_SHIFT;                   /*  获得 CACHE 属性             */
                    if (ulCacheAttr == MIPS_MMU_ENTRYLO_CACHE) {        /*  可以 CACHE                  */
                        ulFlag |= LW_VMM_FLAG_CACHEABLE;
                    }

                    return  (ulFlag);
                }
            }
        }
    }

    return  (LW_VMM_FLAG_UNVALID);
}
/*********************************************************************************************************
** 函数名称: mips64MmuFlagSet
** 功能描述: 设置指定虚拟地址的 flag 标志
** 输　入  : pmmuctx        mmu 上下文
**           ulAddr         虚拟地址
**           ulFlag         flag 标志
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
** 注  意  : 这里不需要清除快表 TLB, 因为 VMM 自身会作此操作.
*********************************************************************************************************/
static INT  mips64MmuFlagSet (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulAddr, ULONG  ulFlag)
{
    LW_PGD_TRANSENTRY  *p_pgdentry;
    LW_PMD_TRANSENTRY  *p_pmdentry;
    LW_PTS_TRANSENTRY  *p_ptsentry;
    LW_PTE_TRANSENTRY  *p_pteentry;
    LW_PTE_TRANSENTRY   pteentry;

    if (!(ulFlag & LW_VMM_FLAG_VALID)) {                                /*  无效的映射关系              */
        return  (PX_ERROR);
    }

    p_pgdentry = mips64MmuPgdOffset(pmmuctx, ulAddr);                   /*  获得一级描述符地址          */
    if (p_pgdentry && mips64MmuPgdIsOk(*p_pgdentry)) {                  /*  一级描述符有效              */

        p_pmdentry = mips64MmuPmdOffset(p_pgdentry, ulAddr);            /*  获得二级描述符地址          */
        if (mips64MmuPmdIsOk(*p_pmdentry)) {                            /*  二级描述符有效              */

            p_ptsentry = mips64MmuPtsOffset(p_pmdentry, ulAddr);        /*  获得三级描述符地址          */
            if (mips64MmuPtsIsOk(*p_ptsentry)) {                        /*  三级描述符有效              */

                p_pteentry = mips64MmuPteOffset(p_ptsentry, ulAddr);    /*  获得四级描述符地址          */
                pteentry   = *p_pteentry;                               /*  获得四级描述符              */
                if (mips64MmuPteIsOk(pteentry)) {                       /*  四级描述符有效              */
                    ULONG   ulPFN = (pteentry & MIPS64_ENTRYLO_PFN_MASK) >>
                                     MIPS_ENTRYLO_PFN_SHIFT;            /*  获得物理页面号              */
                    addr_t  ulPhysicalAddr = ulPFN << 12;               /*  计算页面物理地址            */

                    *p_pteentry = mips64MmuBuildPteEntry(ulPhysicalAddr, ulFlag);
                    return  (ERROR_NONE);
                }
            }
        }
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: mips64MmuMakeTrans
** 功能描述: 设置页面映射关系
** 输　入  : pmmuctx        mmu 上下文
**           p_pteentry     对应的页表项
**           ulVirtualAddr  虚拟地址
**           paPhysicalAddr 物理地址
**           ulFlag         对应的类型
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : 这里不需要清除快表 TLB, 因为 VMM 自身会作此操作.
*********************************************************************************************************/
static VOID  mips64MmuMakeTrans (PLW_MMU_CONTEXT     pmmuctx,
                                 LW_PTE_TRANSENTRY  *p_pteentry,
                                 addr_t              ulVirtualAddr,
                                 phys_addr_t         paPhysicalAddr,
                                 ULONG               ulFlag)
{
    if (!(ulFlag & LW_VMM_FLAG_VALID)) {                                /*  无效的映射关系              */
        return;
    }

    /*
     * 构建四级描述符并设置四级描述符
     */
    *p_pteentry = mips64MmuBuildPteEntry((addr_t)paPhysicalAddr, ulFlag);
}
/*********************************************************************************************************
** 函数名称: mips64MmuMakeCurCtx
** 功能描述: 设置 MMU 当前上下文
** 输　入  : pmmuctx        mmu 上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mips64MmuMakeCurCtx (PLW_MMU_CONTEXT  pmmuctx)
{
#if LW_CFG_MIPS_HAS_RDHWR_INSTR > 0
    extern CHAR    *_G_mips64MmuTlbRefillCtxMp;
    MIPS64_TLB_REFILL_CTX  *pCtx = (MIPS64_TLB_REFILL_CTX *)((addr_t)&_G_mips64MmuTlbRefillCtxMp +
                                    MIPS64_TLB_CTX_SIZE * LW_CPU_GET_CUR_ID());
#else
    extern CHAR    *_G_mips64MmuTlbRefillCtx;
    MIPS64_TLB_REFILL_CTX  *pCtx = (MIPS64_TLB_REFILL_CTX *)((addr_t)&_G_mips64MmuTlbRefillCtx);
#endif                                                                  /*  HAS_RDHWR_INSTR > 0         */
    pCtx->CTX_ulSpinLock = 0;
    pCtx->CTX_ulPGD      = (addr_t)pmmuctx->MMUCTX_pgdEntry;
    KN_WMB();

    mipsCp0ContextWrite(0);
    mipsCp0XContextWrite(0);
}
/*********************************************************************************************************
** 函数名称: mips64MmuInit
** 功能描述: MMU 系统初始化
** 输　入  : pmmuop            MMU 操作函数集
**           pcMachineName     使用的机器名称
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  mips64MmuInit (LW_MMU_OP  *pmmuop, CPCHAR  pcMachineName)
{
    pmmuop->MMUOP_pfuncMemInit = mips64MmuMemInit;

    pmmuop->MMUOP_pfuncPGDAlloc = mips64MmuPgdAlloc;
    pmmuop->MMUOP_pfuncPGDFree  = mips64MmuPgdFree;
    pmmuop->MMUOP_pfuncPMDAlloc = mips64MmuPmdAlloc;
    pmmuop->MMUOP_pfuncPMDFree  = mips64MmuPmdFree;
    pmmuop->MMUOP_pfuncPTSAlloc = mips64MmuPtsAlloc;
    pmmuop->MMUOP_pfuncPTSFree  = mips64MmuPtsFree;
    pmmuop->MMUOP_pfuncPTEAlloc = mips64MmuPteAlloc;
    pmmuop->MMUOP_pfuncPTEFree  = mips64MmuPteFree;

    pmmuop->MMUOP_pfuncPGDIsOk = mips64MmuPgdIsOk;
    pmmuop->MMUOP_pfuncPMDIsOk = mips64MmuPmdIsOk;
    pmmuop->MMUOP_pfuncPTSIsOk = mips64MmuPtsIsOk;
    pmmuop->MMUOP_pfuncPTEIsOk = mips64MmuPteIsOk;

    pmmuop->MMUOP_pfuncPGDOffset = mips64MmuPgdOffset;
    pmmuop->MMUOP_pfuncPMDOffset = mips64MmuPmdOffset;
    pmmuop->MMUOP_pfuncPTSOffset = mips64MmuPtsOffset;
    pmmuop->MMUOP_pfuncPTEOffset = mips64MmuPteOffset;

    pmmuop->MMUOP_pfuncPTEPhysGet = mips64MmuPtePhysGet;

    pmmuop->MMUOP_pfuncFlagGet = mips64MmuFlagGet;
    pmmuop->MMUOP_pfuncFlagSet = mips64MmuFlagSet;

    pmmuop->MMUOP_pfuncMakeTrans  = mips64MmuMakeTrans;
    pmmuop->MMUOP_pfuncMakeCurCtx = mips64MmuMakeCurCtx;
}

#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 64*/
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
