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
** 文   件   名: mipsElfCommon.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: 实现 MIPS 体系结构的 ELF 文件重定位.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#if defined(LW_CFG_CPU_ARCH_MIPS)                                       /*  MIPS 体系结构               */
/*********************************************************************************************************
  裁剪支持
*********************************************************************************************************/
#if LW_CFG_MODULELOADER_EN > 0
#include "../SylixOS/loader/elf/elf_type.h"
#include "../SylixOS/loader/elf/elf_arch.h"
#include "../SylixOS/loader/include/loader_lib.h"
#include "../SylixOS/loader/include/loader_symbol.h"
#include "./mipsElfCommon.h"
/*********************************************************************************************************
** 函数名称: mipsElfREL32RelocateRel
** 功能描述: 重定位 R_MIPS_REL32 类型的重定位项
** 输  入  : pmodule      模块
**           pRelocAdrs   重定位地址
**           SymIndex     重定位符表索引值
** 输  出  : ERROR_NONE 表示没有错误, PX_ERROR 表示错误
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  mipsElfREL32RelocateRel (LW_LD_EXEC_MODULE  *pmodule,
                              Elf_Addr           *pRelocAdrs,
                              Elf_Addr            symIndex)
{
    ELF_DYN_DIR  *pdyndir = (ELF_DYN_DIR *)(pmodule->EMOD_pvFormatInfo);

    if (symIndex) {
        if (symIndex < pdyndir->ulMIPSGotSymIdx) {
            *pRelocAdrs += (Elf_Addr)pmodule->EMOD_pvBaseAddr +
                            pdyndir->psymTable[symIndex].st_value;
        } else {
            *pRelocAdrs += pdyndir->ulPltGotAddr[symIndex + pdyndir->ulMIPSLocalGotNumIdx -
                                                 pdyndir->ulMIPSGotSymIdx];
        }

    } else {
        *pRelocAdrs += (Elf_Addr)pmodule->EMOD_pvBaseAddr;
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: archElfGotInit
** 功能描述: MIPS GOT 重定位
** 输　入  : pmodule       模块
** 输　出  : ERROR_NONE 表示没有错误, PX_ERROR 表示错误
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  archElfGotInit (PVOID  pmodule)
{
    Elf_Addr           *pMipsGotEntry;
    Elf_Sym            *pMipsSym;
    CHAR               *pchStrTab;
    Elf_Addr            addrSymVal;
    ULONG               ulTemp = 0;
    INT                 iIndex = 2;
    LW_LD_EXEC_MODULE  *pprivmodule = (LW_LD_EXEC_MODULE *)pmodule;
    ELF_DYN_DIR        *pprivdyndir = (ELF_DYN_DIR *)pprivmodule->EMOD_pvFormatInfo;

    pMipsGotEntry = pprivdyndir->ulPltGotAddr + iIndex;
    for (; iIndex < pprivdyndir->ulMIPSLocalGotNumIdx; iIndex++, pMipsGotEntry++) {
        *pMipsGotEntry += (Elf_Addr)pprivmodule->EMOD_pvBaseAddr;
    }

    pMipsGotEntry = pprivdyndir->ulPltGotAddr + pprivdyndir->ulMIPSLocalGotNumIdx;
    pMipsSym      = (Elf_Sym *)pprivdyndir->psymTable + pprivdyndir->ulMIPSGotSymIdx;
    pchStrTab     = (CHAR *)pprivdyndir->pcStrTable;
    ulTemp        = pprivdyndir->ulMIPSSymNumIdx - pprivdyndir->ulMIPSGotSymIdx;

    while (ulTemp--) {
        if (pMipsSym->st_shndx == SHN_UNDEF || pMipsSym->st_shndx == SHN_COMMON) {
            BOOL  bWeak = (STB_WEAK == ELF_ST_BIND(pMipsSym->st_info));

            if (__moduleSymGetValue(pprivmodule,
                                    bWeak,
                                    pchStrTab + pMipsSym->st_name,
                                    &addrSymVal,
                                    LW_LD_SYM_ANY) < 0) {
                return  (PX_ERROR);
            }
            *pMipsGotEntry = addrSymVal;

        } else if (ELF_ST_TYPE(pMipsSym->st_info) == STT_SECTION) {
            if (pMipsSym->st_other == 0) {
                *pMipsGotEntry += (Elf_Addr)pprivmodule->EMOD_pvBaseAddr;
            }

        } else {
            addrSymVal     = LW_LD_V2PADDR(pprivdyndir->addrMin,
                                           pprivmodule->EMOD_pvBaseAddr,
                                           pMipsSym->st_value);
            *pMipsGotEntry = addrSymVal;
        }

        pMipsGotEntry++;
        pMipsSym++;
    }

    return (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: archElfRGetJmpBuffItemLen
** 功能描述: 返回跳转表项长度
** 输  入  : pmodule       模块
** 输  出  : 跳转表项长度
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  archElfRGetJmpBuffItemLen (PVOID  pmodule)
{
    return  (JMP_TABLE_ITEMLEN);
}

#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
#endif                                                                  /*  LW_CFG_CPU_ARCH_MIPS        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
