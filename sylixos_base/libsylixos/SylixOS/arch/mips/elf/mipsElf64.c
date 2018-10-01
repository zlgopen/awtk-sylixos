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
** 文   件   名: mipsElf64.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: 实现 MIPS64 体系结构的 ELF 文件重定位.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#if defined(LW_CFG_CPU_ARCH_MIPS)                                       /*  MIPS 体系结构               */
#if LW_CFG_CPU_WORD_LENGHT == 64
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
** 函数名称: archElfRelocateRel
** 功能描述: 重定位 REL 类型的重定位项
** 输  入  : module       模块 
**           prel         REL 表项
**           psym         符号
**           addrSymVal   重定位符号的值
**           pcTargetSec  重定位目目标节区
**           pcBuffer     跳转表起始地址
**           stBuffLen    跳转表长度
** 输  出  : ERROR_NONE 表示没有错误, PX_ERROR 表示错误
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  archElfRelocateRel (PVOID        pmodule,
                         Elf_Rel     *prel,
                         Elf_Sym     *psym,
                         Elf_Addr     addrSymVal,
                         PCHAR        pcTargetSec,
                         PCHAR        pcBuffer,
                         size_t       stBuffLen)
{
    Elf_Addr  *paddrWhere;

    paddrWhere = (Elf_Addr *)((size_t)pcTargetSec + prel->r_offset);    /*  计算重定位目标地址          */

    switch (ELF_MIPS_R_TYPE(prel)) {

    case R_MIPS_REL32:
        mipsElfREL32RelocateRel((LW_LD_EXEC_MODULE *)pmodule,
                                paddrWhere,
                                ELF_MIPS_R_SYM(prel));
        break;

    case R_MIPS_NONE:
        break;

    default:
        _DebugFormat(__ERRORMESSAGE_LEVEL, "unknown relocate type %d.\r\n", ELF_MIPS_R_TYPE(prel));
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsElfRelocateRela
** 功能描述: 重定位 RELA 类型的重定位项
** 输  入  : module       模块 
**           prela        RELA 表项
**           psym         符号
**           addrSymVal   重定位符号的值
**           pcTargetSec  重定位目目标节区
**           pcBuffer     跳转表起始地址
**           stBuffLen    跳转表长度
** 输  出  : ERROR_NONE 表示没有错误, PX_ERROR 表示错误
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  archElfRelocateRela (PVOID       pmodule,
                          Elf_Rela   *prela,
                          Elf_Sym    *psym,
                          Elf_Addr    addrSymVal,
                          PCHAR       pcTargetSec,
                          PCHAR       pcBuffer,
                          size_t      stBuffLen)
{
    UINT32  *paddrWhere;

    paddrWhere = (UINT32 *)((size_t)pcTargetSec + prela->r_offset);     /*  计算重定位目标地址          */

    addrSymVal += prela->r_addend;

    switch (ELF_MIPS_R_TYPE(prela)) {

    case R_MIPS_NONE:
        break;

    case R_MIPS_32:
        (*(UINT32 *)paddrWhere) += (UINT32)addrSymVal;
        break;

    case R_MIPS_64:
        (*(UINT64 *)paddrWhere) += (UINT64)addrSymVal;
        break;

    case R_MIPS_HIGHER:
        *paddrWhere = (*paddrWhere & 0xffff0000) |
                      ((((INT64)addrSymVal + 0x80008000LL) >> 32) & 0xffff);
        break;

    case R_MIPS_HIGHEST:
        *paddrWhere = (*paddrWhere & 0xffff0000) |
                      ((((INT64)addrSymVal + 0x800080008000LL) >> 48) & 0xffff);
        break;

    case R_MIPS_REL32:
        mipsElfREL32RelocateRel((LW_LD_EXEC_MODULE *)pmodule,
                                (Elf_Addr *)paddrWhere,
                                ELF_MIPS_R_SYM(prela));
        break;

    case R_MIPS_26:
        if (addrSymVal & 0x03) {
            return  (PX_ERROR);
        }
        if ((addrSymVal & 0xf0000000) != (((Elf_Addr)paddrWhere + 4) & 0xf0000000)) {
            return  (PX_ERROR);
        }
        *paddrWhere = (*paddrWhere & ~0x03ffffff) |
                      ((*paddrWhere + (addrSymVal >> 2)) & 0x03ffffff);
        break;

    case R_MIPS_HI16:
        *paddrWhere = (*paddrWhere & 0xffff0000) |
                      ((((INT64)addrSymVal + 0x8000LL) >> 16) & 0xffff);
        break;

    case R_MIPS_LO16:
        *paddrWhere = (*paddrWhere & 0xffff0000) | (addrSymVal & 0xffff);
        break;

    case R_MIPS_JUMP_SLOT:
        *paddrWhere = (Elf_Addr)addrSymVal;
        break;

    default:
        _DebugFormat(__ERRORMESSAGE_LEVEL, "unknown relocate type %d.\r\n", ELF_MIPS_R_TYPE(prela));
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 64*/
#endif                                                                  /*  LW_CFG_CPU_ARCH_MIPS64      */
/*********************************************************************************************************
  END
*********************************************************************************************************/
