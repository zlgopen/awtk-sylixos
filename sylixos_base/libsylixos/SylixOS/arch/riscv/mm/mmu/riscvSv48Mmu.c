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
** 文   件   名: riscvSv48Mmu.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 04 月 11 日
**
** 描        述: RISC-V64 体系构架 Sv48 MMU 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_VMM_EN > 0
#if (LW_CFG_CPU_WORD_LENGHT == 64) && (LW_CFG_RISCV_MMU_SV39 == 0)
#include "arch/riscv/inc/sbi.h"
#include "riscvMmu.h"
/*********************************************************************************************************
  PTE 位定义
*********************************************************************************************************/
#define SV48_MMU_V                  (1)                                 /*  映射有效                    */
#define SV48_MMU_V_SHIFT            (0)

#define SV48_MMU_R                  (1)                                 /*  可读                        */
#define SV48_MMU_R_NO               (0)
#define SV48_MMU_R_SHIFT            (1)

#define SV48_MMU_W                  (1)                                 /*  可写                        */
#define SV48_MMU_W_NO               (0)
#define SV48_MMU_W_SHIFT            (2)

#define SV48_MMU_X                  (1)                                 /*  可执行                      */
#define SV48_MMU_X_NO               (0)
#define SV48_MMU_X_SHIFT            (3)

#define SV48_MMU_U                  (1)                                 /*  用户模式可访问              */
#define SV48_MMU_U_NO               (0)
#define SV48_MMU_U_SHIFT            (4)

#define SV48_MMU_G                  (1)                                 /*  全局                        */
#define SV48_MMU_G_NO               (0)
#define SV48_MMU_G_SHIFT            (5)

#define SV48_MMU_RSW_ZERO           (0)
#define SV48_MMU_RSW_CACHE          (1)                                 /*  可 CACHE                    */
#define SV48_MMU_RSW_BUFFER         (2)                                 /*  可 BUFFER                   */
#define SV48_MMU_RSW_SHIFT          (8)

#define SV48_MMU_A_SHIFT            (6)
#define SV48_MMU_D_SHIFT            (7)

#define SV48_MMU_PPN_MASK           (0x3ffffffffffc00ul)                /*  [53:10]                     */
#define SV48_MMU_PPN_SHIFT          (10)
#define SV48_MMU_PA(ulTemp)         ((ulTemp & SV48_MMU_PPN_MASK) << (LW_CFG_VMM_PAGE_SHIFT - SV48_MMU_PPN_SHIFT))
#define SV48_MMU_PPN(pa)            (((pa) >> (LW_CFG_VMM_PAGE_SHIFT - SV48_MMU_PPN_SHIFT)) & SV48_MMU_PPN_MASK)
/*********************************************************************************************************
  页面数大于其 1/2 时, 全清 TLB
*********************************************************************************************************/
#define SV48_MMU_TLB_NR             (128)                               /*  TLB 数目                    */
/*********************************************************************************************************
  虚拟地址的大小
*********************************************************************************************************/
#define SV48_MMU_VIRT_ADDR_SIZE     (48)                                /*  48 位虚拟地址               */
/*********************************************************************************************************
  全局变量定义
*********************************************************************************************************/
static LW_OBJECT_HANDLE     _G_hPGDPartition;                           /*  系统目前仅使用一个 PGD      */
static LW_OBJECT_HANDLE     _G_hPMDPartition;                           /*  PMD 缓冲区                  */
static LW_OBJECT_HANDLE     _G_hPTSPartition;                           /*  PTS 缓冲区                  */
static LW_OBJECT_HANDLE     _G_hPTEPartition;                           /*  PTE 缓冲区                  */
/*********************************************************************************************************
** 函数名称: sv48MmuEnable
** 功能描述: 使能 MMU
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  sv48MmuEnable (VOID)
{
    ULONG  ulSPTBR = read_csr("sptbr");

    ulSPTBR &= ~(SATP64_MODE);
    ulSPTBR |=  (9ULL << 60);                                           /*  Sv48                        */

    write_csr("sptbr", ulSPTBR);
}
/*********************************************************************************************************
** 函数名称: sv48MmuDisable
** 功能描述: 关闭 MMU
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  sv48MmuDisable (VOID)
{
    ULONG  ulSPTBR = read_csr("sptbr");

    ulSPTBR &= ~(SATP64_MODE);                                          /*  Bare                        */

    write_csr("sptbr", ulSPTBR);
}
/*********************************************************************************************************
** 函数名称: sv48MmuInvalidateTLBMVA
** 功能描述: 无效指定地址的 TLB
** 输　入  : pvAddr        地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  sv48MmuInvalidateTLBMVA (PVOID  pvAddr)
{
    __asm__ __volatile__ ("sfence.vma %0" : : "r" (pvAddr) : "memory");
}
/*********************************************************************************************************
** 函数名称: sv48MmuInvalidateTLB
** 功能描述: 无效所有 TLB
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  sv48MmuInvalidateTLB (VOID)
{
    __asm__ __volatile__ ("sfence.vma" : : : "memory");
}
/*********************************************************************************************************
** 函数名称: sv48MmuFlags2Attr
** 功能描述: 根据 SylixOS 权限标志, 生成 sv48 MMU 权限标志
** 输　入  : ulFlag                  SylixOS 权限标志
**           pucV                    是否有效
**           pucR                    是否可读
**           pucW                    是否可写
**           pucX                    是否执行
**           pucU                    是否用户能访问
**           pucG                    是否全局
**           pucRSW                  RSW
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  sv48MmuFlags2Attr (ULONG   ulFlag,
                               UINT8  *pucV,
                               UINT8  *pucR,
                               UINT8  *pucW,
                               UINT8  *pucX,
                               UINT8  *pucU,
                               UINT8  *pucG,
                               UINT8  *pucRSW)
{
    if (!(ulFlag & LW_VMM_FLAG_VALID)) {                                /*  无效的映射关系              */
        return  (PX_ERROR);
    }

    *pucV = SV48_MMU_V;                                                 /*  有效                        */
    *pucU = SV48_MMU_U_NO;                                              /*  User 不能访问               */
    *pucG = SV48_MMU_G;                                                 /*  全局映射                    */

    if (ulFlag & LW_VMM_FLAG_ACCESS) {                                  /*  是否可访问                  */
        *pucR = SV48_MMU_R;

    } else {
        *pucR = SV48_MMU_R_NO;
    }

    if (ulFlag & LW_VMM_FLAG_WRITABLE) {                                /*  是否可写                    */
        *pucW = SV48_MMU_W;

    } else {
        *pucW = SV48_MMU_W_NO;
    }

    if (ulFlag & LW_VMM_FLAG_EXECABLE) {                                /*  是否可执行                  */
        *pucX = SV48_MMU_X;

    } else {
        *pucX = SV48_MMU_X_NO;
    }

    if ((ulFlag & LW_VMM_FLAG_CACHEABLE) &&
        (ulFlag & LW_VMM_FLAG_BUFFERABLE)) {                            /*  CACHE 与 BUFFER 控制        */
        *pucRSW = SV48_MMU_RSW_CACHE | SV48_MMU_RSW_BUFFER;

    } else if (ulFlag & LW_VMM_FLAG_CACHEABLE) {
        *pucRSW = SV48_MMU_RSW_CACHE;

    } else if (ulFlag & LW_VMM_FLAG_BUFFERABLE) {
        *pucRSW = SV48_MMU_RSW_BUFFER;

    } else {
        *pucRSW = SV48_MMU_RSW_ZERO;
    }

    return (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuAttr2Flags
** 功能描述: 根据 sv48 MMU 权限标志, 生成 SylixOS 权限标志
** 输　入  : ucV                     是否有效
**           ucR                     是否可读
**           ucW                     是否可写
**           ucX                     是否执行
**           ucU                     是否用户能访问
**           ucG                     是否全局
**           ucRSW                   RSW
**           pulFlag                 SylixOS 权限标志
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  sv48MmuAttr2Flags (UINT8   ucV,
                               UINT8   ucR,
                               UINT8   ucW,
                               UINT8   ucX,
                               UINT8   ucU,
                               UINT8   ucG,
                               UINT8   ucRSW,
                               ULONG  *pulFlag)
{
    *pulFlag = LW_VMM_FLAG_VALID;

    if (ucR == SV48_MMU_R) {
        *pulFlag |= LW_VMM_FLAG_ACCESS;
    }

    if (ucW == SV48_MMU_W) {
        *pulFlag |= LW_VMM_FLAG_WRITABLE;
    }

    if (ucX == SV48_MMU_X) {
        *pulFlag |= LW_VMM_FLAG_EXECABLE;
    }

    if (ucRSW & SV48_MMU_RSW_BUFFER) {
        *pulFlag |= LW_VMM_FLAG_BUFFERABLE;
    }

    if (ucRSW & SV48_MMU_RSW_CACHE) {
        *pulFlag |= LW_VMM_FLAG_CACHEABLE;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuBuildPgdEntry
** 功能描述: 生成一个一级描述符 (PGD 描述符)
** 输　入  : ulBaseAddr              基地址     (二级页表基地址)
**           ucV                     是否有效
**           ucR                     是否可读
**           ucW                     是否可写
**           ucX                     是否执行
**           ucU                     是否用户能访问
**           ucG                     是否全局
**           ucRSW                   RSW
** 输　出  : 一级描述符
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static LW_INLINE LW_PGD_TRANSENTRY  sv48MmuBuildPgdEntry (addr_t  ulBaseAddr,
                                                          UINT8   ucV,
                                                          UINT8   ucR,
                                                          UINT8   ucW,
                                                          UINT8   ucX,
                                                          UINT8   ucU,
                                                          UINT8   ucG,
                                                          UINT8   ucRSW)
{
    LW_PGD_TRANSENTRY  ulDescriptor;

    ulDescriptor = (SV48_MMU_PPN(ulBaseAddr))
                 | (ucR << SV48_MMU_A_SHIFT)
                 | (ucW << SV48_MMU_D_SHIFT)
                 | (ucV << SV48_MMU_V_SHIFT)
                 | (ucR << SV48_MMU_R_SHIFT)
                 | (ucW << SV48_MMU_W_SHIFT)
                 | (ucX << SV48_MMU_X_SHIFT)
                 | (ucU << SV48_MMU_U_SHIFT)
                 | (ucG << SV48_MMU_G_SHIFT)
                 | ((ULONG)ucRSW << SV48_MMU_RSW_SHIFT);

    return  (ulDescriptor);
}
/*********************************************************************************************************
** 函数名称: sv48MmuBuildPmdEntry
** 功能描述: 生成一个二级描述符 (PMD 描述符)
** 输　入  : ulBaseAddr              基地址     (三级页表基地址)
**           ucV                     是否有效
**           ucR                     是否可读
**           ucW                     是否可写
**           ucX                     是否执行
**           ucU                     是否用户能访问
**           ucG                     是否全局
**           ucRSW                   RSW
** 输　出  : 二级描述符
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE LW_PMD_TRANSENTRY  sv48MmuBuildPmdEntry (addr_t  ulBaseAddr,
                                                          UINT8   ucV,
                                                          UINT8   ucR,
                                                          UINT8   ucW,
                                                          UINT8   ucX,
                                                          UINT8   ucU,
                                                          UINT8   ucG,
                                                          UINT8   ucRSW)
{
    LW_PMD_TRANSENTRY  ulDescriptor;

    ulDescriptor = (SV48_MMU_PPN(ulBaseAddr))
                 | (ucR << SV48_MMU_A_SHIFT)
                 | (ucW << SV48_MMU_D_SHIFT)
                 | (ucV << SV48_MMU_V_SHIFT)
                 | (ucR << SV48_MMU_R_SHIFT)
                 | (ucW << SV48_MMU_W_SHIFT)
                 | (ucX << SV48_MMU_X_SHIFT)
                 | (ucU << SV48_MMU_U_SHIFT)
                 | (ucG << SV48_MMU_G_SHIFT)
                 | ((ULONG)ucRSW << SV48_MMU_RSW_SHIFT);

    return  (ulDescriptor);
}
/*********************************************************************************************************
** 函数名称: sv48MmuBuildPtsEntry
** 功能描述: 生成一个三级描述符 (PTS 描述符)
** 输　入  : ulBaseAddr              基地址     (四级页表基地址)
**           ucV                     是否有效
**           ucR                     是否可读
**           ucW                     是否可写
**           ucX                     是否执行
**           ucU                     是否用户能访问
**           ucG                     是否全局
**           ucRSW                   RSW
** 输　出  : 三级描述符
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE LW_PTS_TRANSENTRY  sv48MmuBuildPtsEntry (addr_t  ulBaseAddr,
                                                          UINT8   ucV,
                                                          UINT8   ucR,
                                                          UINT8   ucW,
                                                          UINT8   ucX,
                                                          UINT8   ucU,
                                                          UINT8   ucG,
                                                          UINT8   ucRSW)
{
    LW_PTS_TRANSENTRY  ulDescriptor;

    ulDescriptor = (SV48_MMU_PPN(ulBaseAddr))
                 | (ucR << SV48_MMU_A_SHIFT)
                 | (ucW << SV48_MMU_D_SHIFT)
                 | (ucV << SV48_MMU_V_SHIFT)
                 | (ucR << SV48_MMU_R_SHIFT)
                 | (ucW << SV48_MMU_W_SHIFT)
                 | (ucX << SV48_MMU_X_SHIFT)
                 | (ucU << SV48_MMU_U_SHIFT)
                 | (ucG << SV48_MMU_G_SHIFT)
                 | ((ULONG)ucRSW << SV48_MMU_RSW_SHIFT);

    return  (ulDescriptor);
}
/*********************************************************************************************************
** 函数名称: sv48MmuBuildPteEntry
** 功能描述: 生成一个四级描述符 (PTE 描述符)
** 输　入  : ulBaseAddr              基地址     (页基地址)
**           ucV                     是否有效
**           ucR                     是否可读
**           ucW                     是否可写
**           ucX                     是否执行
**           ucU                     是否用户能访问
**           ucG                     是否全局
**           ucRSW                   RSW
** 输　出  : 四级描述符
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE LW_PTE_TRANSENTRY  sv48MmuBuildPteEntry (addr_t  ulBaseAddr,
                                                          UINT8   ucV,
                                                          UINT8   ucR,
                                                          UINT8   ucW,
                                                          UINT8   ucX,
                                                          UINT8   ucU,
                                                          UINT8   ucG,
                                                          UINT8   ucRSW)
{
    LW_PTE_TRANSENTRY  ulDescriptor;

    ulDescriptor = (SV48_MMU_PPN(ulBaseAddr))
                 | (ucR << SV48_MMU_A_SHIFT)
                 | (ucW << SV48_MMU_D_SHIFT)
                 | (ucR << SV48_MMU_V_SHIFT)
                 | (ucR << SV48_MMU_R_SHIFT)
                 | (ucW << SV48_MMU_W_SHIFT)
                 | (ucX << SV48_MMU_X_SHIFT)
                 | (ucU << SV48_MMU_U_SHIFT)
                 | (ucG << SV48_MMU_G_SHIFT)
                 | ((ULONG)ucRSW << SV48_MMU_RSW_SHIFT);

    return  (ulDescriptor);
}
/*********************************************************************************************************
** 函数名称: sv48MmuMemInit
** 功能描述: 初始化 MMU 页表内存区
** 输　入  : pmmuctx        mmu 上下文
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  sv48MmuMemInit (PLW_MMU_CONTEXT  pmmuctx)
{
#define PGD_BLOCK_SIZE  (4 * LW_CFG_KB_SIZE)
#define PMD_BLOCK_SIZE  (4 * LW_CFG_KB_SIZE)
#define PTS_BLOCK_SIZE  (4 * LW_CFG_KB_SIZE)
#define PTE_BLOCK_SIZE  (4 * LW_CFG_KB_SIZE)

    PVOID  pvPgdTable;
    PVOID  pvPmdTable;
    PVOID  pvPtsTable;
    PVOID  pvPteTable;
    
    ULONG  ulPgdNum = bspMmuPgdMaxNum();
    ULONG  ulPmdNum = bspMmuPmdMaxNum();
    ULONG  ulPtsNum = bspMmuPtsMaxNum();
    ULONG  ulPteNum = bspMmuPteMaxNum();
    
    pvPgdTable = __KHEAP_ALLOC_ALIGN((size_t)ulPgdNum * PGD_BLOCK_SIZE, PGD_BLOCK_SIZE);
    pvPmdTable = __KHEAP_ALLOC_ALIGN((size_t)ulPmdNum * PMD_BLOCK_SIZE, PMD_BLOCK_SIZE);
    pvPtsTable = __KHEAP_ALLOC_ALIGN((size_t)ulPtsNum * PTS_BLOCK_SIZE, PTS_BLOCK_SIZE);
    pvPteTable = __KHEAP_ALLOC_ALIGN((size_t)ulPteNum * PTE_BLOCK_SIZE, PTE_BLOCK_SIZE);
    
    if (!pvPgdTable || !pvPmdTable || !pvPtsTable || !pvPteTable) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "can not allocate page table.\r\n");
        return  (PX_ERROR);
    }
    
    _G_hPGDPartition = API_PartitionCreate("pgd_pool", pvPgdTable, ulPgdNum, PGD_BLOCK_SIZE,
                                           LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    _G_hPMDPartition = API_PartitionCreate("pmd_pool", pvPmdTable, ulPmdNum, PMD_BLOCK_SIZE,
                                           LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    _G_hPTSPartition = API_PartitionCreate("pts_pool", pvPtsTable, ulPtsNum, PTS_BLOCK_SIZE,
                                           LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    _G_hPTEPartition = API_PartitionCreate("pte_pool", pvPteTable, ulPteNum, PTE_BLOCK_SIZE,
                                           LW_OPTION_OBJECT_GLOBAL, LW_NULL);
                                           
    if (!_G_hPGDPartition || !_G_hPMDPartition || !_G_hPTSPartition || !_G_hPTEPartition) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "can not allocate page pool.\r\n");
        return  (PX_ERROR);
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuGlobalInit
** 功能描述: 调用 BSP 对 MMU 初始化
** 输　入  : pcMachineName     使用的机器名称
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  sv48MmuGlobalInit (CPCHAR  pcMachineName)
{
    archCacheReset(pcMachineName);

    sv48MmuInvalidateTLB();
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPgdOffset
** 功能描述: 通过虚拟地址计算 PGD 项
** 输　入  : pmmuctx        mmu 上下文
**           ulAddr         虚拟地址
** 输　出  : 对应的 PGD 表项地址
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static LW_PGD_TRANSENTRY  *sv48MmuPgdOffset (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulAddr)
{
    REGISTER LW_PGD_TRANSENTRY  *p_pgdentry = pmmuctx->MMUCTX_pgdEntry;
    REGISTER ULONG               ulPgdNum;

    if (ulAddr & (~((1ULL << SV48_MMU_VIRT_ADDR_SIZE) - 1))) {          /*  不在合法的虚拟地址空间内    */
        return  (LW_NULL);
    }

    ulAddr    &= LW_CFG_VMM_PGD_MASK;
    ulPgdNum   = ulAddr >> LW_CFG_VMM_PGD_SHIFT;                        /*  计算 PGD 号                 */

    p_pgdentry = (LW_PGD_TRANSENTRY *)((addr_t)p_pgdentry |
                 (ulPgdNum * sizeof(LW_PGD_TRANSENTRY)));               /*  获得一级页表描述符地址      */
               
    return  (p_pgdentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPmdOffset
** 功能描述: 通过虚拟地址计算 PMD 项
** 输　入  : p_pgdentry     pgd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 对应的 PMD 表项地址
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static LW_PMD_TRANSENTRY  *sv48MmuPmdOffset (LW_PGD_TRANSENTRY  *p_pgdentry, addr_t  ulAddr)
{
    REGISTER LW_PMD_TRANSENTRY  *p_pmdentry;
    REGISTER LW_PGD_TRANSENTRY   ulTemp;
    REGISTER ULONG               ulPmdNum;

    ulTemp = (LW_PGD_TRANSENTRY)(*p_pgdentry);                          /*  获得一级页表描述符          */

    p_pmdentry = (LW_PMD_TRANSENTRY *)(SV48_MMU_PA(ulTemp));            /*  获得二级页表基地址          */

    ulAddr    &= LW_CFG_VMM_PMD_MASK;
    ulPmdNum   = ulAddr >> LW_CFG_VMM_PMD_SHIFT;                        /*  计算 PMD 号                 */

    p_pmdentry = (LW_PMD_TRANSENTRY *)((addr_t)p_pmdentry |
                 (ulPmdNum * sizeof(LW_PMD_TRANSENTRY)));               /*  获得二级页表描述符地址      */

    return  (p_pmdentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPtsOffset
** 功能描述: 通过虚拟地址计算 PTS 项
** 输　入  : p_pmdentry     pmd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 对应的 PTS 表项地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PTS_TRANSENTRY  *sv48MmuPtsOffset (LW_PMD_TRANSENTRY  *p_pmdentry, addr_t  ulAddr)
{
    REGISTER LW_PTS_TRANSENTRY  *p_ptsentry;
    REGISTER LW_PMD_TRANSENTRY   ulTemp;
    REGISTER ULONG               ulPtsNum;

    ulTemp = (LW_PMD_TRANSENTRY)(*p_pmdentry);                          /*  获得二级页表描述符          */

    p_ptsentry = (LW_PTS_TRANSENTRY *)(SV48_MMU_PA(ulTemp));            /*  获得三级页表基地址          */

    ulAddr    &= LW_CFG_VMM_PTS_MASK;
    ulPtsNum   = ulAddr >> LW_CFG_VMM_PTS_SHIFT;                        /*  计算 PTS 号                 */

    p_ptsentry = (LW_PTS_TRANSENTRY *)((addr_t)p_ptsentry |
                 (ulPtsNum * sizeof(LW_PTS_TRANSENTRY)));               /*  获得三级页表描述符地址      */

    return  (p_ptsentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPteOffset
** 功能描述: 通过虚拟地址计算 PTE 项
** 输　入  : p_ptsentry     pts 入口地址
**           ulAddr         虚拟地址
** 输　出  : 对应的 PTE 表项地址
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static  LW_PTE_TRANSENTRY  *sv48MmuPteOffset (LW_PTS_TRANSENTRY  *p_ptsentry, addr_t  ulAddr)
{
    REGISTER LW_PTE_TRANSENTRY  *p_pteentry;
    REGISTER LW_PTS_TRANSENTRY   ulTemp;
    REGISTER ULONG               ulPageNum;

    ulTemp = (LW_PTS_TRANSENTRY)(*p_ptsentry);                          /*  获得三级页表描述符          */
    
    p_pteentry = (LW_PTE_TRANSENTRY *)(SV48_MMU_PA(ulTemp));            /*  获得四级页表基地址          */

    ulAddr    &= 0x1fful << LW_CFG_VMM_PAGE_SHIFT;                      /*  不要使用LW_CFG_VMM_PAGE_MASK*/
    ulPageNum  = ulAddr >> LW_CFG_VMM_PAGE_SHIFT;                       /*  计算段内页号                */

    p_pteentry = (LW_PTE_TRANSENTRY *)((addr_t)p_pteentry |
                 (ulPageNum * sizeof(LW_PTE_TRANSENTRY)));              /*  获得虚拟地址页表描述符地址  */

    return  (p_pteentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPgdIsOk
** 功能描述: 判断 PGD 项的描述符是否正确
** 输　入  : pgdentry       PGD 项描述符
** 输　出  : 是否正确
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static BOOL  sv48MmuPgdIsOk (LW_PGD_TRANSENTRY  pgdentry)
{
    return  ((pgdentry & (SV48_MMU_V << SV48_MMU_V_SHIFT)) ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPmdIsOk
** 功能描述: 判断 PMD 项的描述符是否正确
** 输　入  : pmdentry       PMD 项描述符
** 输　出  : 是否正确
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  sv48MmuPmdIsOk (LW_PMD_TRANSENTRY  pmdentry)
{
    return  ((pmdentry & (SV48_MMU_V << SV48_MMU_V_SHIFT)) ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPtsIsOk
** 功能描述: 判断 PTS 项的描述符是否正确
** 输　入  : ptsentry       PTS 项描述符
** 输　出  : 是否正确
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  sv48MmuPtsIsOk (LW_PTS_TRANSENTRY  ptsentry)
{
    return  ((ptsentry & (SV48_MMU_V << SV48_MMU_V_SHIFT)) ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPteIsOk
** 功能描述: 判断 PTE 项的描述符是否正确
** 输　入  : pteentry       PTE 项描述符
** 输　出  : 是否正确
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static BOOL  sv48MmuPteIsOk (LW_PTE_TRANSENTRY  pteentry)
{
    return  ((pteentry & (SV48_MMU_V << SV48_MMU_V_SHIFT)) ? LW_TRUE : LW_FALSE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPgdAlloc
** 功能描述: 分配 PGD 项
** 输　入  : pmmuctx        mmu 上下文
**           ulAddr         虚拟地址 (参数 0 即偏移量为 0 , 需要返回页表基地址)
** 输　出  : 分配 PGD 地址
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static LW_PGD_TRANSENTRY *sv48MmuPgdAlloc (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulAddr)
{
    REGISTER LW_PGD_TRANSENTRY  *p_pgdentry;
    REGISTER ULONG               ulPgdNum;
    
    if (ulAddr & (~((1ULL << SV48_MMU_VIRT_ADDR_SIZE) - 1))) {          /*  不在合法的虚拟地址空间内    */
        return  (LW_NULL);
    }

    p_pgdentry = (LW_PGD_TRANSENTRY *)API_PartitionGet(_G_hPGDPartition);
    if (!p_pgdentry) {
        return  (LW_NULL);
    }
    
    lib_bzero(p_pgdentry, PGD_BLOCK_SIZE);

    ulAddr    &= LW_CFG_VMM_PGD_MASK;
    ulPgdNum   = ulAddr >> LW_CFG_VMM_PGD_SHIFT;                        /*  计算 PGD 号                 */

    p_pgdentry = (LW_PGD_TRANSENTRY *)((addr_t)p_pgdentry |
                 (ulPgdNum * sizeof(LW_PGD_TRANSENTRY)));               /*  获得一级页表描述符地址      */
               
    return  (p_pgdentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPgdFree
** 功能描述: 释放 PGD 项
** 输　入  : p_pgdentry     pgd 入口地址
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sv48MmuPgdFree (LW_PGD_TRANSENTRY  *p_pgdentry)
{
    p_pgdentry = (LW_PGD_TRANSENTRY *)((addr_t)p_pgdentry & (~(PGD_BLOCK_SIZE - 1)));
    
    API_PartitionPut(_G_hPGDPartition, (PVOID)p_pgdentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPmdAlloc
** 功能描述: 分配 PMD 项
** 输　入  : pmmuctx        mmu 上下文
**           p_pgdentry     pgd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 分配 PMD 地址
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static LW_PMD_TRANSENTRY *sv48MmuPmdAlloc (PLW_MMU_CONTEXT     pmmuctx,
                                           LW_PGD_TRANSENTRY  *p_pgdentry,
                                           addr_t              ulAddr)
{
    LW_PMD_TRANSENTRY  *p_pmdentry = (LW_PMD_TRANSENTRY *)API_PartitionGet(_G_hPMDPartition);

    if (!p_pmdentry) {
        return  (LW_NULL);
    }

    lib_bzero(p_pmdentry, PMD_BLOCK_SIZE);

    *p_pgdentry = sv48MmuBuildPgdEntry((addr_t)p_pmdentry,
                                       SV48_MMU_V,
                                       SV48_MMU_R_NO,
                                       SV48_MMU_W_NO,
                                       SV48_MMU_X_NO,
                                       SV48_MMU_U_NO,
                                       SV48_MMU_G_NO,
                                       SV48_MMU_RSW_ZERO);              /*  设置一级页表描述符          */

    return  (sv48MmuPmdOffset(p_pgdentry, ulAddr));
}
/*********************************************************************************************************
** 函数名称: sv48MmuPmdFree
** 功能描述: 释放 PMD 项
** 输　入  : p_pmdentry     pmd 入口地址
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sv48MmuPmdFree (LW_PMD_TRANSENTRY  *p_pmdentry)
{
    p_pmdentry = (LW_PMD_TRANSENTRY *)((addr_t)p_pmdentry & (~(PMD_BLOCK_SIZE - 1)));

    API_PartitionPut(_G_hPMDPartition, (PVOID)p_pmdentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPtsAlloc
** 功能描述: 分配 PTS 项
** 输　入  : pmmuctx        mmu 上下文
**           p_pmdentry     pmd 入口地址
**           ulAddr         虚拟地址
** 输　出  : 分配 PTS 地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_PTS_TRANSENTRY *sv48MmuPtsAlloc (PLW_MMU_CONTEXT     pmmuctx,
                                           LW_PMD_TRANSENTRY  *p_pmdentry,
                                           addr_t              ulAddr)
{
    LW_PTS_TRANSENTRY  *p_ptsentry = (LW_PTS_TRANSENTRY *)API_PartitionGet(_G_hPTSPartition);

    if (!p_ptsentry) {
        return  (LW_NULL);
    }

    lib_bzero(p_ptsentry, PTS_BLOCK_SIZE);

    *p_pmdentry = sv48MmuBuildPmdEntry((addr_t)p_ptsentry,
                                       SV48_MMU_V,
                                       SV48_MMU_R_NO,
                                       SV48_MMU_W_NO,
                                       SV48_MMU_X_NO,
                                       SV48_MMU_U_NO,
                                       SV48_MMU_G_NO,
                                       SV48_MMU_RSW_ZERO);              /*  设置二级页表描述符          */

    return  (sv48MmuPtsOffset(p_pmdentry, ulAddr));
}
/*********************************************************************************************************
** 函数名称: sv48MmuPtsFree
** 功能描述: 释放 PTS 项
** 输　入  : p_ptsentry     pts 入口地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  sv48MmuPtsFree (LW_PTS_TRANSENTRY  *p_ptsentry)
{
    p_ptsentry = (LW_PTS_TRANSENTRY *)((addr_t)p_ptsentry & (~(PTS_BLOCK_SIZE - 1)));

    API_PartitionPut(_G_hPTSPartition, (PVOID)p_ptsentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPteAlloc
** 功能描述: 分配 PTE 项
** 输　入  : pmmuctx        mmu 上下文
**           p_ptsentry     pts 入口地址
**           ulAddr         虚拟地址
** 输　出  : 分配 PTE 地址
** 全局变量: 
** 调用模块: VMM 这里没有关闭中断, 回写 CACHE 时, 需要手动关中断, SylixOS 映射完毕会自动清快表, 所以
             这里不用清除快表.
*********************************************************************************************************/
static LW_PTE_TRANSENTRY  *sv48MmuPteAlloc (PLW_MMU_CONTEXT     pmmuctx,
                                            LW_PTS_TRANSENTRY  *p_ptsentry,
                                            addr_t              ulAddr)
{
    LW_PTE_TRANSENTRY  *p_pteentry = (LW_PTE_TRANSENTRY *)API_PartitionGet(_G_hPTEPartition);
    
    if (!p_pteentry) {
        return  (LW_NULL);
    }
    
    lib_bzero(p_pteentry, PTE_BLOCK_SIZE);
    
    *p_ptsentry = sv48MmuBuildPtsEntry((addr_t)p_pteentry,
                                       SV48_MMU_V,
                                       SV48_MMU_R_NO,
                                       SV48_MMU_W_NO,
                                       SV48_MMU_X_NO,
                                       SV48_MMU_U_NO,
                                       SV48_MMU_G_NO,
                                       SV48_MMU_RSW_ZERO);              /*  设置三级页表描述符          */

    return  (sv48MmuPteOffset(p_ptsentry, ulAddr));
}
/*********************************************************************************************************
** 函数名称: sv48MmuPteFree
** 功能描述: 释放 PTE 项
** 输　入  : p_pteentry     pte 入口地址
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sv48MmuPteFree (LW_PTE_TRANSENTRY  *p_pteentry)
{
    p_pteentry = (LW_PTE_TRANSENTRY *)((addr_t)p_pteentry & (~(PTE_BLOCK_SIZE - 1)));
    
    API_PartitionPut(_G_hPTEPartition, (PVOID)p_pteentry);
}
/*********************************************************************************************************
** 函数名称: sv48MmuPtePhysGet
** 功能描述: 通过 PTE 表项, 查询物理地址
** 输　入  : pteentry           pte 表项
**           pulPhysicalAddr    获得的物理地址
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static INT  sv48MmuPtePhysGet (LW_PTE_TRANSENTRY  pteentry, addr_t  *pulPhysicalAddr)
{
    *pulPhysicalAddr = (addr_t)(SV48_MMU_PA(pteentry));                 /*  获得物理地址                */
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: sv48MmuFlagGet
** 功能描述: 获得指定虚拟地址的 SylixOS 权限标志
** 输　入  : pmmuctx        mmu 上下文
**           ulAddr         虚拟地址
** 输　出  : SylixOS 权限标志
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static ULONG  sv48MmuFlagGet (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulAddr)
{
    LW_PGD_TRANSENTRY  *p_pgdentry = sv48MmuPgdOffset(pmmuctx, ulAddr); /*  获得一级描述符地址          */

    if (p_pgdentry && sv48MmuPgdIsOk(*p_pgdentry)) {                    /*  一级描述符有效              */
        LW_PMD_TRANSENTRY  *p_pmdentry = sv48MmuPmdOffset(p_pgdentry,
                                                          ulAddr);      /*  获得二级描述符地址          */

        if (sv48MmuPmdIsOk(*p_pmdentry)) {                              /*  二级描述符有效              */
            LW_PTS_TRANSENTRY  *p_ptsentry = sv48MmuPtsOffset(p_pmdentry,
                                                              ulAddr);  /*  获得三级描述符地址          */

            if (sv48MmuPtsIsOk(*p_ptsentry)) {                          /*  三级描述符有效              */
                LW_PTE_TRANSENTRY  *p_pteentry = sv48MmuPteOffset(p_ptsentry,
                                                                ulAddr);/*  获得四级描述符地址          */
                LW_PTE_TRANSENTRY   pteentry = *p_pteentry;             /*  获得四级描述符              */

                if (sv48MmuPteIsOk(pteentry)) {                         /*  四级描述符有效              */
                    UINT8  ucV, ucR, ucW, ucX, ucU, ucG, ucRSW;
                    ULONG  ulFlag;

                    ucV   = (UINT8)((pteentry >> SV48_MMU_V_SHIFT)   & 0x01);
                    ucR   = (UINT8)((pteentry >> SV48_MMU_R_SHIFT)   & 0x01);
                    ucW   = (UINT8)((pteentry >> SV48_MMU_W_SHIFT)   & 0x01);
                    ucX   = (UINT8)((pteentry >> SV48_MMU_X_SHIFT)   & 0x01);
                    ucU   = (UINT8)((pteentry >> SV48_MMU_U_SHIFT)   & 0x01);
                    ucG   = (UINT8)((pteentry >> SV48_MMU_G_SHIFT)   & 0x01);
                    ucRSW = (UINT8)((pteentry >> SV48_MMU_RSW_SHIFT) & 0x03);

                    sv48MmuAttr2Flags(ucV, ucR, ucW, ucX, ucU, ucG, ucRSW, &ulFlag);

                    return  (ulFlag);
                }
            }
        }
    }

    return  (LW_VMM_FLAG_UNVALID);
}
/*********************************************************************************************************
** 函数名称: sv48MmuFlagSet
** 功能描述: 设置指定虚拟地址的 flag 标志
** 输　入  : pmmuctx        mmu 上下文
**           ulAddr         虚拟地址
**           ulFlag         flag 标志
** 输　出  : ERROR CODE
** 全局变量: 
** 调用模块: 
** 注  意  : 这里不需要清除快表 TLB, 因为 VMM 自身会作此操作.
*********************************************************************************************************/
static INT  sv48MmuFlagSet (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulAddr, ULONG  ulFlag)
{
    UINT8  ucV, ucR, ucW, ucX, ucU, ucG, ucRSW;

    if (sv48MmuFlags2Attr(ulFlag, &ucV, &ucR, &ucW,
                          &ucX, &ucU, &ucG, &ucRSW) != ERROR_NONE) {    /*  无效的映射关系              */
        return  (PX_ERROR);
    }

    LW_PGD_TRANSENTRY  *p_pgdentry = sv48MmuPgdOffset(pmmuctx, ulAddr); /*  获得一级描述符地址          */

    if (p_pgdentry && sv48MmuPgdIsOk(*p_pgdentry)) {                    /*  一级描述符有效              */
        LW_PMD_TRANSENTRY  *p_pmdentry = sv48MmuPmdOffset(p_pgdentry,
                                                          ulAddr);      /*  获得二级描述符地址          */

        if (sv48MmuPmdIsOk(*p_pmdentry)) {                              /*  二级描述符有效              */
            LW_PTS_TRANSENTRY  *p_ptsentry = sv48MmuPtsOffset(p_pmdentry,
                                                              ulAddr);  /*  获得三级描述符地址          */

            if (sv48MmuPtsIsOk(*p_ptsentry)) {                          /*  三级描述符有效              */
                LW_PTE_TRANSENTRY  *p_pteentry = sv48MmuPteOffset(p_ptsentry,
                                                                ulAddr);/*  获得四级描述符地址          */
                LW_PTE_TRANSENTRY   pteentry = *p_pteentry;             /*  获得四级描述符              */

                if (sv48MmuPteIsOk(pteentry)) {                         /*  四级描述符有效              */
                    addr_t  ulPhysicalAddr = (addr_t)(SV48_MMU_PA(pteentry));

                    *p_pteentry = sv48MmuBuildPteEntry(ulPhysicalAddr,
                                                       ucV, ucR, ucW, ucX, ucU, ucG, ucRSW);

                    return  (ERROR_NONE);
                }
            }
        }
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: sv48MmuMakeTrans
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
static VOID  sv48MmuMakeTrans (PLW_MMU_CONTEXT     pmmuctx,
                               LW_PTE_TRANSENTRY  *p_pteentry,
                               addr_t              ulVirtualAddr,
                               phys_addr_t         paPhysicalAddr,
                               ULONG               ulFlag)
{
    UINT8  ucV, ucR, ucW, ucX, ucU, ucG, ucRSW;
    
    if (sv48MmuFlags2Attr(ulFlag, &ucV, &ucR, &ucW,
                          &ucX, &ucU, &ucG, &ucRSW) != ERROR_NONE) {    /*  无效的映射关系              */
        return;
    }

    *p_pteentry = sv48MmuBuildPteEntry((addr_t)paPhysicalAddr,
                                       ucV, ucR, ucW, ucX, ucU, ucG, ucRSW);
}
/*********************************************************************************************************
** 函数名称: sv48MmuMakeCurCtx
** 功能描述: 设置 MMU 当前上下文
** 输　入  : pmmuctx        mmu 上下文
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  sv48MmuMakeCurCtx (PLW_MMU_CONTEXT  pmmuctx)
{
    ULONG  ulSPTBR = read_csr("sptbr");

    ulSPTBR = ((ULONG)pmmuctx->MMUCTX_pgdEntry) >> LW_CFG_VMM_PAGE_SHIFT;

    write_csr("sptbr", ulSPTBR);
}
/*********************************************************************************************************
** 函数名称: sv48MmuInvTLB
** 功能描述: 无效当前 CPU TLB
** 输　入  : pmmuctx        mmu 上下文
**           ulPageAddr     页面虚拟地址
**           ulPageNum      页面个数
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  sv48MmuInvTLB (PLW_MMU_CONTEXT  pmmuctx, addr_t  ulPageAddr, ULONG  ulPageNum)
{
    if (ulPageNum > (SV48_MMU_TLB_NR >> 1)) {
        sv48MmuInvalidateTLB();                                         /*  全部清除 TLB                */

    } else {
        ULONG  i;

        for (i = 0; i < ulPageNum; i++) {
            sv48MmuInvalidateTLBMVA((PVOID)ulPageAddr);                 /*  逐个页面清除 TLB            */
            ulPageAddr += LW_CFG_VMM_PAGE_SIZE;
        }
    }
}
/*********************************************************************************************************
** 函数名称: riscvMmuAbortType
** 功能描述: 获得访问终止类型
** 输　入  : ulAddr        终止地址
**           uiMethod      访问方法(LW_VMM_ABORT_METHOD_XXX)
** 输　出  : 访问终止类型
** 全局变量:
** 调用模块:
*********************************************************************************************************/
ULONG  riscvMmuAbortType (addr_t  ulAddr, UINT  uiMethod)
{
    PLW_MMU_CONTEXT     pmmuctx    = __vmmGetCurCtx();
    LW_PGD_TRANSENTRY  *p_pgdentry = sv48MmuPgdOffset(pmmuctx, ulAddr); /*  获得一级描述符地址          */

    if (p_pgdentry && sv48MmuPgdIsOk(*p_pgdentry)) {                    /*  一级描述符有效              */
        LW_PMD_TRANSENTRY  *p_pmdentry = sv48MmuPmdOffset(p_pgdentry,
                                                          ulAddr);      /*  获得二级描述符地址          */

        if (sv48MmuPmdIsOk(*p_pmdentry)) {                              /*  二级描述符有效              */
            LW_PTS_TRANSENTRY  *p_ptsentry = sv48MmuPtsOffset(p_pmdentry,
                                                              ulAddr);  /*  获得三级描述符地址          */

            if (sv48MmuPtsIsOk(*p_ptsentry)) {                          /*  三级描述符有效              */
                LW_PTE_TRANSENTRY  *p_pteentry = sv48MmuPteOffset(p_ptsentry,
                                                                ulAddr);/*  获得四级描述符地址          */
                LW_PTE_TRANSENTRY   pteentry = *p_pteentry;             /*  获得四级描述符              */

                if (sv48MmuPteIsOk(pteentry)) {                         /*  四级描述符有效              */
                    addr_t  ulPhysicalAddr = (addr_t)(SV48_MMU_PA(pteentry));

                    if (ulPhysicalAddr) {
                        if (uiMethod == LW_VMM_ABORT_METHOD_READ) {
                            UINT8  ucR;

                            ucR = (UINT8)((pteentry >> SV48_MMU_R_SHIFT) & 0x01);
                            if (!ucR) {
                                return  (LW_VMM_ABORT_TYPE_PERM);
                            }

                        } else if (uiMethod == LW_VMM_ABORT_METHOD_WRITE) {
                            UINT8  ucW;

                            ucW = (UINT8)((pteentry >> SV48_MMU_W_SHIFT) & 0x01);
                            if (!ucW) {
                                return  (LW_VMM_ABORT_TYPE_PERM);
                            }

                        } else {
                            UINT8  ucX;

                            ucX = (UINT8)((pteentry >> SV48_MMU_X_SHIFT) & 0x01);
                            if (!ucX) {
                                return  (LW_VMM_ABORT_TYPE_PERM);
                            }
                        }
                    }
                }
            }
        }
    }

    return  (LW_VMM_ABORT_TYPE_MAP);
}
/*********************************************************************************************************
** 函数名称: riscvMmuInit
** 功能描述: MMU 系统初始化
** 输　入  : pmmuop            MMU 操作函数集
**           pcMachineName     使用的机器名称
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  riscvMmuInit (LW_MMU_OP  *pmmuop, CPCHAR  pcMachineName)
{
    pmmuop->MMUOP_ulOption = LW_VMM_MMU_FLUSH_TLB_MP;

    pmmuop->MMUOP_pfuncMemInit    = sv48MmuMemInit;
    pmmuop->MMUOP_pfuncGlobalInit = sv48MmuGlobalInit;
    
    pmmuop->MMUOP_pfuncPGDAlloc = sv48MmuPgdAlloc;
    pmmuop->MMUOP_pfuncPGDFree  = sv48MmuPgdFree;
    pmmuop->MMUOP_pfuncPMDAlloc = sv48MmuPmdAlloc;
    pmmuop->MMUOP_pfuncPMDFree  = sv48MmuPmdFree;
    pmmuop->MMUOP_pfuncPTSAlloc = sv48MmuPtsAlloc;
    pmmuop->MMUOP_pfuncPTSFree  = sv48MmuPtsFree;
    pmmuop->MMUOP_pfuncPTEAlloc = sv48MmuPteAlloc;
    pmmuop->MMUOP_pfuncPTEFree  = sv48MmuPteFree;
    
    pmmuop->MMUOP_pfuncPGDIsOk = sv48MmuPgdIsOk;
    pmmuop->MMUOP_pfuncPMDIsOk = sv48MmuPmdIsOk;
    pmmuop->MMUOP_pfuncPTSIsOk = sv48MmuPtsIsOk;
    pmmuop->MMUOP_pfuncPTEIsOk = sv48MmuPteIsOk;
    
    pmmuop->MMUOP_pfuncPGDOffset = sv48MmuPgdOffset;
    pmmuop->MMUOP_pfuncPMDOffset = sv48MmuPmdOffset;
    pmmuop->MMUOP_pfuncPTSOffset = sv48MmuPtsOffset;
    pmmuop->MMUOP_pfuncPTEOffset = sv48MmuPteOffset;
    
    pmmuop->MMUOP_pfuncPTEPhysGet = sv48MmuPtePhysGet;
    
    pmmuop->MMUOP_pfuncFlagGet = sv48MmuFlagGet;
    pmmuop->MMUOP_pfuncFlagSet = sv48MmuFlagSet;
    
    pmmuop->MMUOP_pfuncMakeTrans     = sv48MmuMakeTrans;
    pmmuop->MMUOP_pfuncMakeCurCtx    = sv48MmuMakeCurCtx;
    pmmuop->MMUOP_pfuncInvalidateTLB = sv48MmuInvTLB;
    pmmuop->MMUOP_pfuncSetEnable     = sv48MmuEnable;
    pmmuop->MMUOP_pfuncSetDisable    = sv48MmuDisable;
}

#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 64*/
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
