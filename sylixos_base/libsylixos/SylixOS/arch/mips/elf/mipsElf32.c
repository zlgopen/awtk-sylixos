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
** 文   件   名: mipsElf32.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: 实现 MIPS32 体系结构的 ELF 文件重定位.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#if defined(LW_CFG_CPU_ARCH_MIPS)                                       /*  MIPS 体系结构               */
#if LW_CFG_CPU_WORD_LENGHT == 32
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
** 函数名称: mipsElfHI16RelocateRel
** 功能描述: 重定位 R_MIPS_HI16类型的重定位项
** 输  入  : pmodule      模块
**           pRelocAdrs   重定位地址
**           addrSymVal   重定位符号的值
** 输  出  : ERROR_NONE 表示没有错误, PX_ERROR 表示错误
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  mipsElfHI16RelocateRel (LW_LD_EXEC_MODULE  *pmodule,
                                    Elf_Addr           *pRelocAdrs,
                                    Elf_Addr            addrSymVal)
{
    PMIPS_HI16_RELOC_INFO  pHi16Info;

    pHi16Info = LW_LD_SAFEMALLOC(sizeof(MIPS_HI16_RELOC_INFO));
    if (!pHi16Info) {
        return  (PX_ERROR);
    }

    pHi16Info->HI16_pAddr    	= (Elf_Addr *)pRelocAdrs;
    pHi16Info->HI16_valAddr  	= addrSymVal;
    pHi16Info->HI16_pNext    	= pmodule->EMOD_pMIPSHi16List;        	/*  增加一个 List 节点          */
    pmodule->EMOD_pMIPSHi16List = pHi16Info;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: mipsElfFreeHI16Relocatelist
** 功能描述: 重定位 R_MIPS_LO16类型的重定位项
** 输  入  : pHi16Info   HI16_RELOC_INFO描述符
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  mipsElfFreeHI16Relocatelist (PMIPS_HI16_RELOC_INFO  pHi16Info)
{
    PMIPS_HI16_RELOC_INFO  pNext;

    while (pHi16Info != NULL) {
        pNext = pHi16Info->HI16_pNext;
        LW_LD_SAFEFREE(pHi16Info);
        pHi16Info = pNext;
    }
}
/*********************************************************************************************************
** 函数名称: mipsElfLO16RelocateRel
** 功能描述: 重定位 R_MIPS_LO16类型的重定位项
** 输  入  : pmodule      模块
**           pRelocAdrs   重定位地址
**           addrSymVal   重定位符号的值
** 输  出  : ERROR_NONE 表示没有错误, PX_ERROR 表示错误
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  mipsElfLO16RelocateRel (LW_LD_EXEC_MODULE  *pmodule,
                                    Elf_Addr           *pRelocAdrs,
                                    Elf_Addr            addrSymVal)
{
    PMIPS_HI16_RELOC_INFO  pHi16Info;
    Elf_Addr               addrVal, addrValLO;
    Elf_Addr               addrInsnLO = *pRelocAdrs;

    addrValLO = SIGN_LOW16_VALUE(addrInsnLO);

    if (pmodule->EMOD_pMIPSHi16List != NULL) {
        pHi16Info = pmodule->EMOD_pMIPSHi16List;
        while (pHi16Info != NULL) {
            PMIPS_HI16_RELOC_INFO  pNext;
            Elf_Addr               addrInsn;

            /*
             * 检查 HI16 和 LO16 的重定位 offset 是否一致
             */
            if (addrSymVal != pHi16Info->HI16_valAddr) {
                mipsElfFreeHI16Relocatelist(pHi16Info);
                pmodule->EMOD_pMIPSHi16List = NULL;
                return  (PX_ERROR);
            }

            /*
             * 计算总的地址
             */
            addrInsn  = *pHi16Info->HI16_pAddr;
            addrVal   = (LOW16_VALUE(addrInsn) << 16) + addrValLO;
            addrVal  += addrSymVal;

            /*
             * 检查 BIT15 的符号值(sign extension)
             */
            addrVal   = ((addrVal >> 16) + ((addrVal & 0x8000) != 0)) & 0xffff;

            addrInsn  = (addrInsn & ~0xffff) | addrVal;
            *pHi16Info->HI16_pAddr = addrInsn;

            pNext     = pHi16Info->HI16_pNext;
            LW_LD_SAFEFREE(pHi16Info);
            pHi16Info = pNext;
        }

        pmodule->EMOD_pMIPSHi16List = NULL;
    }

    addrVal     = addrSymVal + addrValLO;
    addrInsnLO  = (addrInsnLO & ~0xffff) | LOW16_VALUE(addrVal);
    *pRelocAdrs = addrInsnLO;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: archElfRelocateRel
** 功能描述: 重定位 REL 类型的重定位项
** 输  入  : pmodule      模块 
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

    switch (ELF_R_TYPE(prel->r_info)) {

    case R_MIPS_NONE:
        break;

    case R_MIPS_32:
        *paddrWhere += (Elf_Addr)addrSymVal;
        break;

    case R_MIPS_REL32:
        mipsElfREL32RelocateRel((LW_LD_EXEC_MODULE *)pmodule,
                                paddrWhere,
                                ELF_R_SYM(prel->r_info));
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
        mipsElfHI16RelocateRel((LW_LD_EXEC_MODULE *)pmodule,
                               paddrWhere,
                               addrSymVal);
        break;

    case R_MIPS_LO16:
        mipsElfLO16RelocateRel((LW_LD_EXEC_MODULE *)pmodule,
                               paddrWhere,
                               addrSymVal);
        break;

    case R_MIPS_JUMP_SLOT:
        *paddrWhere = (Elf_Addr)addrSymVal;
        break;

    default:
        _DebugFormat(__ERRORMESSAGE_LEVEL, "unknown relocate type %d.\r\n", ELF_R_TYPE(prel->r_info));
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
    return  (PX_ERROR);
}

#endif                                                                  /*  LW_CFG_MODULELOADER_EN > 0  */
#endif                                                                  /*  LW_CFG_CPU_WORD_LENGHT == 32*/
#endif                                                                  /*  LW_CFG_CPU_ARCH_MIPS        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
