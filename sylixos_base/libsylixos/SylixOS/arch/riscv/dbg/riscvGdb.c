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
** 文   件   名: riscvGdb.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: RISC-V 体系构架 GDB 调试接口.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁减配置
*********************************************************************************************************/
#if LW_CFG_GDB_EN > 0
#include "dtrace.h"
#include "arch/arch_gdb.h"
#include "arch/riscv/backtrace/riscvBacktrace.h"
/*********************************************************************************************************
  Xfer:features:read:arch-core.xml 回应包
*********************************************************************************************************/
#if LW_CFG_CPU_WORD_LENGHT == 32
#define WORD_LENGHT "32"
#else
#define WORD_LENGHT "64"
#endif

static const CHAR   _G_cRiscvCoreXml[] = \
    "<?xml version=\"1.0\"?>"
    "<!-- Copyright (C) 2006-2017 ACOINFO co.,ltd."
         "Copying and distribution of this file, with or without modification,"
         "are permitted in any medium without royalty provided the copyright"
         "notice and this notice are preserved.  -->"
    "<!DOCTYPE feature SYSTEM \"gdb-target.dtd\">"
    "<feature name=\"org.gnu.gdb.riscv.core\">"
    "<reg name=\"x0\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x1\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x2\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x3\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x4\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x5\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x6\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x7\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x8\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x9\"  bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x10\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x11\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x12\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x13\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x14\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x15\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x16\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x17\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x18\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x19\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x20\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x21\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x22\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x23\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x24\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x25\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x26\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x27\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x28\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x29\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x30\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"x31\" bitsize=\""WORD_LENGHT"\" group=\"general\"/>"
    "<reg name=\"pc\"  bitsize=\""WORD_LENGHT"\" type=\"code_ptr\" group=\"general\"/>"
    "</feature>";
/*********************************************************************************************************
  Xfer:features:read:target.xml 回应包
*********************************************************************************************************/
static const CHAR   _G_cRiscvTargetXml[] = \
    "l<?xml version=\"1.0\"?>"
    "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">"
    "<target>"
        "<xi:include href=\"arch-core.xml\"/>"
    "</target>";
/*********************************************************************************************************
  寄存器在 GDB_REG_SET 结构中的索引
*********************************************************************************************************/
#define GDB_REG_INDEX_GREG(n)   (n)                                     /*  32 个通用目的寄存器         */
#define GDB_REG_INDEX_PC        (32)                                    /*  程序计数器寄存器            */
#define GDB_REG_INDEX_FP(n)     (32 + 1 + (n))                          /*  32 个浮点数据寄存器         */
#define GDB_REG_NR              (33)                                    /*  寄存器总数                  */
/*********************************************************************************************************
** 函数名称: archGdbTargetXml
** 功能描述: 获得 Xfer:features:read:target.xml 回复 XML
** 输　入  : NONE
** 输　出  : 回复 XML
** 全局变量:
** 调用模块:
*********************************************************************************************************/
CPCHAR  archGdbTargetXml (VOID)
{
    return  (_G_cRiscvTargetXml);
}
/*********************************************************************************************************
** 函数名称: archGdbCoreXml
** 功能描述: 获得 Xfer:features:read:arch-core.xml 回复 XML
** 输　入  : NONE
** 输　出  : 回复 XML
** 全局变量:
** 调用模块:
*********************************************************************************************************/
CPCHAR  archGdbCoreXml (VOID)
{
    return  (_G_cRiscvCoreXml);
}
/*********************************************************************************************************
** 函数名称: archGdbRegsGet
** 功能描述: 获取寄存器值
** 输　入  : pvDtrace       dtrace 节点
**           ulThread       线程句柄
**           pregset        gdb 寄存器结构
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  archGdbRegsGet (PVOID  pvDtrace, LW_OBJECT_HANDLE  ulThread, GDB_REG_SET  *pregset)
{
    ARCH_REG_CTX    regctx;
    ARCH_REG_T      regSp;

    API_DtraceGetRegs(pvDtrace, ulThread, &regctx, &regSp);

    lib_bzero(pregset, sizeof(GDB_REG_SET));

    pregset->regArr[GDB_REG_INDEX_GREG( 0)].GDBRA_ulValue = 0;          /*  ZERO                        */
    pregset->regArr[GDB_REG_INDEX_GREG( 1)].GDBRA_ulValue = regctx.REG_ulReg[ 1];
    pregset->regArr[GDB_REG_INDEX_GREG( 2)].GDBRA_ulValue = regSp;      /*  SP                          */
    pregset->regArr[GDB_REG_INDEX_GREG( 3)].GDBRA_ulValue = regctx.REG_ulReg[ 3];
    pregset->regArr[GDB_REG_INDEX_GREG( 4)].GDBRA_ulValue = regctx.REG_ulReg[ 4];
    pregset->regArr[GDB_REG_INDEX_GREG( 5)].GDBRA_ulValue = regctx.REG_ulReg[ 5];
    pregset->regArr[GDB_REG_INDEX_GREG( 6)].GDBRA_ulValue = regctx.REG_ulReg[ 6];
    pregset->regArr[GDB_REG_INDEX_GREG( 7)].GDBRA_ulValue = regctx.REG_ulReg[ 7];
    pregset->regArr[GDB_REG_INDEX_GREG( 8)].GDBRA_ulValue = regctx.REG_ulReg[ 8];
    pregset->regArr[GDB_REG_INDEX_GREG( 9)].GDBRA_ulValue = regctx.REG_ulReg[ 9];
    pregset->regArr[GDB_REG_INDEX_GREG(10)].GDBRA_ulValue = regctx.REG_ulReg[10];
    pregset->regArr[GDB_REG_INDEX_GREG(11)].GDBRA_ulValue = regctx.REG_ulReg[11];
    pregset->regArr[GDB_REG_INDEX_GREG(12)].GDBRA_ulValue = regctx.REG_ulReg[12];
    pregset->regArr[GDB_REG_INDEX_GREG(13)].GDBRA_ulValue = regctx.REG_ulReg[13];
    pregset->regArr[GDB_REG_INDEX_GREG(14)].GDBRA_ulValue = regctx.REG_ulReg[14];
    pregset->regArr[GDB_REG_INDEX_GREG(15)].GDBRA_ulValue = regctx.REG_ulReg[15];
    pregset->regArr[GDB_REG_INDEX_GREG(16)].GDBRA_ulValue = regctx.REG_ulReg[16];
    pregset->regArr[GDB_REG_INDEX_GREG(17)].GDBRA_ulValue = regctx.REG_ulReg[17];
    pregset->regArr[GDB_REG_INDEX_GREG(18)].GDBRA_ulValue = regctx.REG_ulReg[18];
    pregset->regArr[GDB_REG_INDEX_GREG(19)].GDBRA_ulValue = regctx.REG_ulReg[19];
    pregset->regArr[GDB_REG_INDEX_GREG(20)].GDBRA_ulValue = regctx.REG_ulReg[20];
    pregset->regArr[GDB_REG_INDEX_GREG(21)].GDBRA_ulValue = regctx.REG_ulReg[21];
    pregset->regArr[GDB_REG_INDEX_GREG(22)].GDBRA_ulValue = regctx.REG_ulReg[22];
    pregset->regArr[GDB_REG_INDEX_GREG(23)].GDBRA_ulValue = regctx.REG_ulReg[23];
    pregset->regArr[GDB_REG_INDEX_GREG(24)].GDBRA_ulValue = regctx.REG_ulReg[24];
    pregset->regArr[GDB_REG_INDEX_GREG(25)].GDBRA_ulValue = regctx.REG_ulReg[25];
    pregset->regArr[GDB_REG_INDEX_GREG(26)].GDBRA_ulValue = regctx.REG_ulReg[26];
    pregset->regArr[GDB_REG_INDEX_GREG(27)].GDBRA_ulValue = regctx.REG_ulReg[27];
    pregset->regArr[GDB_REG_INDEX_GREG(28)].GDBRA_ulValue = regctx.REG_ulReg[28];
    pregset->regArr[GDB_REG_INDEX_GREG(29)].GDBRA_ulValue = regctx.REG_ulReg[29];
    pregset->regArr[GDB_REG_INDEX_GREG(30)].GDBRA_ulValue = regctx.REG_ulReg[30];
    pregset->regArr[GDB_REG_INDEX_GREG(31)].GDBRA_ulValue = regctx.REG_ulReg[31];

    pregset->regArr[GDB_REG_INDEX_PC].GDBRA_ulValue       = regctx.REG_ulEpc;

    pregset->GDBR_iRegCnt = GDB_REG_NR;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: archGdbRegsSet
** 功能描述: 设置寄存器值
** 输　入  : pvDtrace       dtrace 节点
**           ulThread       线程句柄
**           pregset        gdb 寄存器结构
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  archGdbRegsSet (PVOID  pvDtrace, LW_OBJECT_HANDLE  ulThread, GDB_REG_SET  *pregset)
{
    ARCH_REG_CTX  regctx;

    regctx.REG_ulReg[ 1] = pregset->regArr[GDB_REG_INDEX_GREG( 1)].GDBRA_ulValue;
    regctx.REG_ulReg[ 2] = pregset->regArr[GDB_REG_INDEX_GREG( 2)].GDBRA_ulValue;
    regctx.REG_ulReg[ 3] = pregset->regArr[GDB_REG_INDEX_GREG( 3)].GDBRA_ulValue;
    regctx.REG_ulReg[ 4] = pregset->regArr[GDB_REG_INDEX_GREG( 4)].GDBRA_ulValue;
    regctx.REG_ulReg[ 5] = pregset->regArr[GDB_REG_INDEX_GREG( 5)].GDBRA_ulValue;
    regctx.REG_ulReg[ 6] = pregset->regArr[GDB_REG_INDEX_GREG( 6)].GDBRA_ulValue;
    regctx.REG_ulReg[ 7] = pregset->regArr[GDB_REG_INDEX_GREG( 7)].GDBRA_ulValue;
    regctx.REG_ulReg[ 8] = pregset->regArr[GDB_REG_INDEX_GREG( 8)].GDBRA_ulValue;
    regctx.REG_ulReg[ 9] = pregset->regArr[GDB_REG_INDEX_GREG( 9)].GDBRA_ulValue;
    regctx.REG_ulReg[10] = pregset->regArr[GDB_REG_INDEX_GREG(10)].GDBRA_ulValue;
    regctx.REG_ulReg[11] = pregset->regArr[GDB_REG_INDEX_GREG(11)].GDBRA_ulValue;
    regctx.REG_ulReg[12] = pregset->regArr[GDB_REG_INDEX_GREG(12)].GDBRA_ulValue;
    regctx.REG_ulReg[13] = pregset->regArr[GDB_REG_INDEX_GREG(13)].GDBRA_ulValue;
    regctx.REG_ulReg[14] = pregset->regArr[GDB_REG_INDEX_GREG(14)].GDBRA_ulValue;
    regctx.REG_ulReg[15] = pregset->regArr[GDB_REG_INDEX_GREG(15)].GDBRA_ulValue;
    regctx.REG_ulReg[16] = pregset->regArr[GDB_REG_INDEX_GREG(16)].GDBRA_ulValue;
    regctx.REG_ulReg[17] = pregset->regArr[GDB_REG_INDEX_GREG(17)].GDBRA_ulValue;
    regctx.REG_ulReg[18] = pregset->regArr[GDB_REG_INDEX_GREG(18)].GDBRA_ulValue;
    regctx.REG_ulReg[19] = pregset->regArr[GDB_REG_INDEX_GREG(19)].GDBRA_ulValue;
    regctx.REG_ulReg[20] = pregset->regArr[GDB_REG_INDEX_GREG(20)].GDBRA_ulValue;
    regctx.REG_ulReg[21] = pregset->regArr[GDB_REG_INDEX_GREG(21)].GDBRA_ulValue;
    regctx.REG_ulReg[22] = pregset->regArr[GDB_REG_INDEX_GREG(22)].GDBRA_ulValue;
    regctx.REG_ulReg[23] = pregset->regArr[GDB_REG_INDEX_GREG(23)].GDBRA_ulValue;
    regctx.REG_ulReg[24] = pregset->regArr[GDB_REG_INDEX_GREG(24)].GDBRA_ulValue;
    regctx.REG_ulReg[25] = pregset->regArr[GDB_REG_INDEX_GREG(25)].GDBRA_ulValue;
    regctx.REG_ulReg[26] = pregset->regArr[GDB_REG_INDEX_GREG(26)].GDBRA_ulValue;
    regctx.REG_ulReg[27] = pregset->regArr[GDB_REG_INDEX_GREG(27)].GDBRA_ulValue;
    regctx.REG_ulReg[28] = pregset->regArr[GDB_REG_INDEX_GREG(28)].GDBRA_ulValue;
    regctx.REG_ulReg[29] = pregset->regArr[GDB_REG_INDEX_GREG(29)].GDBRA_ulValue;
    regctx.REG_ulReg[30] = pregset->regArr[GDB_REG_INDEX_GREG(30)].GDBRA_ulValue;
    regctx.REG_ulReg[31] = pregset->regArr[GDB_REG_INDEX_GREG(31)].GDBRA_ulValue;
    regctx.REG_ulEpc     = pregset->regArr[GDB_REG_INDEX_PC].GDBRA_ulValue;

    API_DtraceSetRegs(pvDtrace, ulThread, &regctx);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: archGdbRegSetPc
** 功能描述: 设置 PC 寄存器值
** 输　入  : pvDtrace       dtrace 节点
**           ulThread       线程句柄
**           ulPC           PC 寄存器值
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  archGdbRegSetPc (PVOID  pvDtrace, LW_OBJECT_HANDLE  ulThread, ULONG  ulPc)
{
    ARCH_REG_CTX  regctx;
    ARCH_REG_T    regSp;

    API_DtraceGetRegs(pvDtrace, ulThread, &regctx, &regSp);

    regctx.REG_ulEpc = (ARCH_REG_T)ulPc;

    API_DtraceSetRegs(pvDtrace, ulThread, &regctx);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: archGdbRegGetPc
** 功能描述: 获取 PC 寄存器值
** 输　入  : pRegs       寄存器数组
** 输　出  : PC 寄存器值
** 全局变量:
** 调用模块:
*********************************************************************************************************/
ULONG  archGdbRegGetPc (GDB_REG_SET  *pRegs)
{
    return  (pRegs->regArr[GDB_REG_INDEX_PC].GDBRA_ulValue);
}
/*********************************************************************************************************
** 函数名称: archGdbGetNextPc
** 功能描述: 获取下一条指令地址，含分支预测
** 输　入  : pRegs       寄存器数组
** 输　出  : 下一条指令地址
** 全局变量:
** 调用模块:
*********************************************************************************************************/
ULONG  archGdbGetNextPc (PVOID  pvDtrace, LW_OBJECT_HANDLE  ulThread, GDB_REG_SET  *pRegs)
{
    ULONG   ulPc     = pRegs->regArr[GDB_REG_INDEX_PC].GDBRA_ulValue;
    UINT16  uiInst16 = *(UINT16 *)ulPc;
    ULONG   ulNextPc;

    if ((uiInst16 & 0x3) != 3) {                                        /*  压缩指令                    */
        switch (uiInst16 & 0x3) {

        case 0b01:                                                      /*  C1                          */
            switch ((uiInst16 >> 13) & 0x7) {

#if LW_CFG_CPU_WORD_LENGHT == 32                                        /*  RV32C 才有                  */
            case 0b001:                                                 /*  C.JAL                       */
#endif
            case 0b101: {                                               /*  C.J                         */
                UINT32  uiIMM11  = (uiInst16 & (1 << 12)) >> (12 - 11);
                UINT32  uiIMM4   = (uiInst16 & (1 << 11)) >> (11 - 4);
                UINT32  uiIMM9_8 = (uiInst16 & (0x3 << 9)) >> (9 - 8);
                UINT32  uiIMM10  = (uiInst16 & (1 << 8)) << (10 - 8);
                UINT32  uiIMM6   = (uiInst16 & (1 << 7)) >> (7 - 6);
                UINT32  uiIMM7   = (uiInst16 & (1 << 6)) << (7 - 6);
                UINT32  uiIMM3_1 = (uiInst16 & (0x7 << 3)) >> (3 - 1);
                UINT32  uiIMM5   = (uiInst16 & (1 << 2)) << (5 - 2);
                ULONG   ulOffset = uiIMM10 | uiIMM9_8 | uiIMM7 | uiIMM6 | uiIMM5 | uiIMM4 | uiIMM3_1;

                if (uiIMM11) {
                    ulOffset |= ~0x7ffUL;
                }
                ulNextPc = ulPc + ulOffset;
            }
                break;

            case 0b110:                                                 /*  C.BEQZ                      */
            case 0b111: {                                               /*  C.BNEZ                      */
                UINT32  uiIMM8   = (uiInst16 & (1 << 12)) >> (12 - 8);
                UINT32  uiIMM4_3 = (uiInst16 & (0x3 << 10)) >> (10 - 3);
                UINT32  uiIMM7_6 = (uiInst16 & (0x3 << 5)) << (6 - 5);
                UINT32  uiIMM2_1 = (uiInst16 & (0x3 << 3)) >> (3 - 1);
                UINT32  uiIMM5   = (uiInst16 & (1 << 2)) << (5 - 2);
                ULONG   ulOffset = uiIMM7_6 | uiIMM5 | uiIMM4_3 | uiIMM2_1;
                UINT32  uiRs1    = (uiInst16 & (0x7 << 7)) >> 7;
                ULONG   ulRs1    = pRegs->regArr[uiRs1 + 8].GDBRA_ulValue;

                if (uiIMM8) {
                    ulOffset |= ~0xffUL;
                }

                if (((uiInst16 >> 13) & 0x7) == 0b110) {                /*  C.BEQZ                      */
                    if (ulRs1 == 0) {
                        ulNextPc = ulPc + ulOffset;
                    } else {
                        ulNextPc = ulPc + 2;
                    }
                } else {
                    if (ulRs1 != 0) {                                   /*  C.BNEZ                      */
                        ulNextPc = ulPc + ulOffset;
                    } else {
                        ulNextPc = ulPc + 2;
                    }
                }
            }
                break;

            default:
                ulNextPc = ulPc + 2;
                break;
            }
            break;

        case 0b10:                                                      /*  C2                          */
            switch ((uiInst16 >> 13) & 0x7) {

            case 0b100:
                if (((uiInst16 & (0x1f << 2)) == 0) &&                  /*  C.JALR                      */
                     (uiInst16 & (0x1f << 7))) {                        /*  C.JR                        */
                    UINT32  uiRs1 = (uiInst16 & (0x1f << 7)) >> (7);
                    ULONG   ulRs1 = pRegs->regArr[uiRs1 + 8].GDBRA_ulValue;

                    ulNextPc = ulRs1;
                } else {
                    ulNextPc = ulPc + 2;
                }
                break;

            default:
                ulNextPc = ulPc + 2;
                break;
            }

        default:
            ulNextPc = ulPc + 2;
            break;
        }

    } else {                                                            /*  普通指令                    */
        UINT32  uiInst32 = *(UINT32 *)ulPc;

        switch (uiInst32 & 0x7f) {

        case 0b1101111: {                                               /*  JAL                         */
            UINT32  uiIMM20    = (uiInst32 & (1 << 31)) >> (31 - 20);
            UINT32  uiIMM10_1  = (uiInst32 & (0x3ff << 21)) >> (21 - 1);
            UINT32  uiIMM11    = (uiInst32 & (1 << 20)) >> (20 - 11);
            UINT32  uiIMM19_12 = (uiInst32 & (0xff << 12)) >> (12 - 12);
            ULONG   ulOffset   = uiIMM19_12 | uiIMM11 | uiIMM10_1;

            if (uiIMM20) {
                ulOffset |= ~0xfffffUL;
            }

            ulNextPc = ulPc + ulOffset;
        }
            break;

        case 0b1100111: {                                               /*  JALR                        */
            UINT32  uiIMM11   = (uiInst32 & (1 << 31)) >> (31 - 11);
            UINT32  uiIMM10_0 = (uiInst32 & (0x7ff << 20)) >> (20 - 0);
            ULONG   ulOffset  = uiIMM10_0;
            UINT32  uiRs      = (uiInst32 & (0x1f << 15)) >> (15);
            ULONG   ulRs      = pRegs->regArr[uiRs].GDBRA_ulValue;

            if (uiIMM11) {
                ulOffset |= ~0x7ffUL;
            }

            ulNextPc = ulRs + ulOffset;
        }
            break;

        case 0b1100011: {                                               /*  BRANCH                      */
            UINT32  uiIMM12   = (uiInst32 & (1 << 31)) >> (31 - 12);
            UINT32  uiIMM10_5 = (uiInst32 & (0x3f << 25)) >> (25 - 5);
            UINT32  uiIMM4_1  = (uiInst32 & (0xf << 8)) >> (8 - 1);
            UINT32  uiIMM11   = (uiInst32 & (1 << 7)) << (11 - 7);
            ULONG   ulOffset  = uiIMM11 | uiIMM10_5 | uiIMM4_1;
            UINT32  uiFunct3  = (uiInst32 & (0x7 << 12)) >> (12);
            UINT32  uiRs2     = (uiInst32 & (0x1f << 20)) >> (20);
            UINT32  uiRs1     = (uiInst32 & (0x1f << 15)) >> (15);
            ULONG   ulRs2     = pRegs->regArr[uiRs2].GDBRA_ulValue;
            ULONG   ulRs1     = pRegs->regArr[uiRs1].GDBRA_ulValue;

            if (uiIMM12) {
                ulOffset |= ~0xfffUL;
            }

            switch (uiFunct3) {

            case 0b000:                                                 /*  BEQ                         */
                if (ulRs2 == ulRs1) {
                    ulNextPc = ulPc + ulOffset;
                } else {
                    ulNextPc = ulPc + 4;
                }
                break;

            case 0b001:                                                 /*  BNE                         */
                if (ulRs2 != ulRs1) {
                    ulNextPc = ulPc + ulOffset;
                } else {
                    ulNextPc = ulPc + 4;
                }
                break;

            case 0b100:                                                 /*  BLT                         */
                if ((LONG)ulRs1 < (LONG)ulRs2) {
                    ulNextPc = ulPc + ulOffset;
                } else {
                    ulNextPc = ulPc + 4;
                }
                break;

            case 0b110:                                                 /*  BLTU                        */
                if (ulRs1 < ulRs2) {
                    ulNextPc = ulPc + ulOffset;
                } else {
                    ulNextPc = ulPc + 4;
                }
                break;

            case 0b101:                                                 /*  BGE                         */
                if ((LONG)ulRs1 >= (LONG)ulRs2) {
                    ulNextPc = ulPc + ulOffset;
                } else {
                    ulNextPc = ulPc + 4;
                }
                break;

            case 0b111:                                                 /*  BGEU                        */
                if (ulRs1 >= ulRs2) {
                    ulNextPc = ulPc + ulOffset;
                } else {
                    ulNextPc = ulPc + 4;
                }
                break;

            default:
                ulNextPc = ulPc + 4;
                break;
            }
        }
             break;

        default:
            ulNextPc = ulPc + 4;
            break;
        }
    }

    return  (ulNextPc);
}

#endif                                                                  /*  LW_CFG_GDB_EN > 0           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
