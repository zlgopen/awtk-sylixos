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
** 文   件   名: x86CpuId.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2016 年 08 月 03 日
**
** 描        述: x86 体系构架处理器 ID 探测.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "x86CpuId.h"
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define X86_CPUID_PRINT_FEATURE(state, info)        \
    if (state) {                                    \
        printf("    ( X ) ");                       \
    } else {                                        \
        printf("    (   ) ");                       \
    }                                               \
    printf(" %s\n", info)

#define X86_CPUID_PRINT_FEATURE_INV(state, info)    \
    if (!state) {                                   \
        printf("    ( X ) ");                       \
    } else {                                        \
        printf("    (   ) ");                       \
    }                                               \
    printf(" %s\n", info)

#define NOT_EOS(arg)                                \
    (*arg != 0 && (*arg != (CHAR)EOF))

#define SKIP_SPACE(arg)                             \
    while (NOT_EOS(arg) && (lib_isspace((INT)*arg)))\
        arg++
/*********************************************************************************************************
  全局变量定义
*********************************************************************************************************/
static UINT64           _G_ui64X86CpuIdFreq = 1000000000ULL;            /*  CPU 主频, 缺省为 1GHz       */

static X86_CPUID_ENTRY  _G_x86CpuIdTable[] = {                          /*  CPUID 条目表                */
    {X86_CPUID_PENTIUM,        X86_FAMILY_PENTIUM},
    {X86_CPUID_PENTIUM4,       X86_FAMILY_PENTIUM4},
    {X86_CPUID_CORE,           X86_FAMILY_CORE},
    {X86_CPUID_CORE2,          X86_FAMILY_CORE},
    {X86_CPUID_CORE2_DUO,      X86_FAMILY_CORE},
    {X86_CPUID_XEON_5400,      X86_FAMILY_NEHALEM},
    {X86_CPUID_XEON_7400,      X86_FAMILY_NEHALEM},
    {X86_CPUID_XEON_5500,      X86_FAMILY_NEHALEM},
    {X86_CPUID_XEON_C5500,     X86_FAMILY_NEHALEM},
    {X86_CPUID_XEON_5600,      X86_FAMILY_NEHALEM},
    {X86_CPUID_XEON_7500,      X86_FAMILY_NEHALEM},
    {X86_CPUID_COREI5_I7M,     X86_FAMILY_NEHALEM},
    {X86_CPUID_XEON_32NM,      X86_FAMILY_NEHALEM},
    {X86_CPUID_ATOM,           X86_FAMILY_ATOM},
    {X86_CPUID_SANDYBRIDGE,    X86_FAMILY_SANDYBRIDGE},
    {X86_CPUID_CEDARVIEW,      X86_FAMILY_ATOM},
    {X86_CPUID_SILVERMONT,     X86_FAMILY_ATOM},

    {X86_CPUID_HASWELL_CLIENT, X86_FAMILY_HASWELL},
    {X86_CPUID_HASWELL_SERVER, X86_FAMILY_HASWELL},
    {X86_CPUID_HASWELL_ULT,    X86_FAMILY_HASWELL},
    {X86_CPUID_CRYSTAL_WELL,   X86_FAMILY_HASWELL},

    {X86_CPUID_MINUTEIA,       X86_FAMILY_MINUTEIA},

    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY},
    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY},
    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY},
    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY},
    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY},
    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY},
    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY},
    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY},
    {X86_CPUID_DUMMY,          X86_CPUID_DUMMY}};

static INT              _G_iX86CpuEntriesNr = \
        sizeof(_G_x86CpuIdTable) / sizeof(X86_CPUID_ENTRY);             /*  CPUID 条目数                */

const static CHAR      *_G_pcX86CpuFamilyNames[] = {                    /*  CPU 家族名字表              */
    "Not Supported",                                                    /*  0                           */
    "Not Supported",                                                    /*  1                           */
    "Pentium",                                                          /*  2                           */
    "Not Supported",                                                    /*  3                           */
    "Not Supported",                                                    /*  4                           */
    "Pentium 4",                                                        /*  5                           */
    "Core",                                                             /*  6                           */
    "Atom",                                                             /*  7                           */
    "Nehalem",                                                          /*  8                           */
    "Sandy Bridge",                                                     /*  9                           */
    "Haswell",                                                          /*  10                          */
    "Quark"                                                             /*  11                          */
};

const static INT        _G_iCpuFamilyNr = \
        sizeof(_G_pcX86CpuFamilyNames) / sizeof(CHAR *);                /*  CPU 家族名字数              */

const static CHAR      *_G_pcX86CacheTypes[] = {                        /*  CACHE 类型表                */
    "Null",                                                             /*  0                           */
    "Data",                                                             /*  1                           */
    "Instruction",                                                      /*  2                           */
    "Unified",                                                          /*  3                           */
};

const static CHAR      *_G_pcX86CacheShortTypes[] = {                   /*  CACHE 短类型表              */
    "Null",                                                             /*  0                           */
    "D",                                                                /*  1                           */
    "I",                                                                /*  2                           */
    "U",                                                                /*  3                           */
};

const static INT        _G_iX86CacheTypeNr = \
        sizeof(_G_pcX86CacheTypes) / sizeof(CHAR *);                    /*  CACHE 类型数                */

const static CHAR      *_G_x86L2CacheAssoc[] = {                        /*  L2 CACHE 相联度表           */
    "Disabled",                                                         /*  0                           */
    "Direct Mapped",                                                    /*  1                           */
    "2-Way",                                                            /*  2                           */
    "Unsupported",                                                      /*  3                           */
    "4-Way",                                                            /*  4                           */
    "Unsupported",                                                      /*  5                           */
    "8-Way",                                                            /*  6                           */
    "Unsupported",                                                      /*  7                           */
    "16-Way",                                                           /*  8                           */
    "Unsupported",                                                      /*  9                           */
    "Unsupported",                                                      /*  10                          */
    "Unsupported",                                                      /*  11                          */
    "Unsupported",                                                      /*  12                          */
    "Unsupported",                                                      /*  13                          */
    "Unsupported",                                                      /*  14                          */
    "Fully associative"                                                 /*  15                          */
};

static X86_CPUID        _G_x86CpuId;                                    /*  全局的 CPUID 结构           */

X86_CPU_FEATURE         _G_x86CpuFeature = {                            /*  全局的 CPU 特性结构         */
    .CPUF_iICacheWaySize    = 4096,                                     /*  I-Cache way size            */
    .CPUF_iDCacheWaySize    = 4096,                                     /*  D-Cache way size            */
    .CPUF_stCacheFlushBytes = X86_CLFLUSH_DEF_BYTES,                    /*  CLFLUSH 字节数              */
    .CPUF_bHasCLFlush       = LW_FALSE,                                 /*  Has CLFLUSH inst?           */
    .CPUF_bHasAPIC          = LW_FALSE,                                 /*  Has APIC on chip?           */
    .CPUF_uiProcessorFamily = X86_FAMILY_UNSUPPORTED,                   /*  Processor Family            */
    .CPUF_bHasX87FPU        = LW_FALSE,                                 /*  Has X87 FPU?                */
    .CPUF_bHasSSE           = LW_FALSE,                                 /*  Has SSE?                    */
    .CPUF_bHasSSE2          = LW_FALSE,                                 /*  Has SSE?                    */
    .CPUF_bHasFXSR          = LW_FALSE,                                 /*  Has FXSR?                   */
    .CPUF_bHasXSAVE         = LW_FALSE,                                 /*  Has XSAVE?                  */
    .CPUF_stXSaveCtxSize    = 0,                                        /*  XSAVE context size          */
    .CPUF_bHasAVX           = LW_FALSE,                                 /*  Has AVX?                    */
    .CPUF_bHasMMX           = LW_FALSE,                                 /*  Has MMX?                    */
    .CPUF_bHasMTRR          = LW_FALSE,                                 /*  Has MTRR?                   */
    .CPUF_bHasPAT           = LW_FALSE,                                 /*  Has PAT?                    */
    .CPUF_pcCpuInfo         = "<unknow>",                               /*  CPU info                    */
    .CPUF_pcCacheInfo       = "<unknow>",                               /*  CACHE info                  */
};

size_t                  _G_stX86CacheFlushBytes = X86_CLFLUSH_DEF_BYTES;/*  CLFLUSH 字节数              */

const static CHAR      *_G_x86TlbCacheDesc[0x100] = {                   /*  TLB CACHE 描述信息          */
             LW_NULL,
             "Instruction TLB: 4 KB Pages, 4-way set associative, 32 entries",
             "Instruction TLB: 4 MB Pages, fully associative, 2 entries",
             "Data TLB: 4 KB Pages, 4-way set associative, 64 entries",
             "Data TLB: 4 MB Pages, 4-way set associative, 8 entries",
             "Data TLB: 4 MB Pages, 4-way set associative, 32 entries",
             "level-1 instruction cache: 8 KB, 4-way set associative, 32-byte line size",
    [8]    = "level-1 instruction cache: 16 KB, 4-way set associative, 32-byte line size",
    [0xa]  = "level-1 data cache: 8 KB, 2-way set associative, 32-byte line size",
    [0xc]  = "level-1 data cache: 16 KB, 4-way set associative, 32-byte line size",
    [0x22] = "level-3 cache: 512 KB, 4-way set associative, sectored cache, 64-byte line size",
             "level-3 cache: 1 MB, 8-way set associative, sectored cache, 64-byte line size",
             "level-3 cache: 2 MB, 8-way set associative, sectored cache, 64-byte line size",
    [0x29] = "level-3 cache: 4 MB, 8-way set associative, sectored cache, 64-byte line size",
    [0x2c] = "level-1 data cache: 32 KB, 8-way set associative, 64-byte line size",
    [0x30] = "level-1 instruction cache: 32 KB, 8-way set associative, 64-byte line size",
    [0x39] = "level-2 cache: 128 KB, 4-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 192 KB, 6-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 128 KB, 2-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 256 KB, 4-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 384 KB, 6-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 512 KB, 4-way set associative, sectored cache, 64-byte line size",
    [0x40]=  "No level-2 cache or, if processor contains a valid level-2 cache, no level-3 cache",
             "level-2 cache: 128 KB, 4-way set associative, 32-byte line size",
             "level-2 cache: 256 KB, 4-way set associative, 32-byte line size",
             "level-2 cache: 512 KB, 4-way set associative, 32-byte line size",
             "level-2 cache: 1 MB, 4-way set associative, 32-byte line size",
             "level-2 cache: 2 MB, 4-way set associative, 32-byte line size",
             "level-3 cache: 4 MB, 4-way set associative, 64-byte line size",
             "level-3 cache: 8 MB, 8-way set associative, 64-byte line size",
    [0x49] = "level-3 cache: 4 MB, 16-way set associative, 64-byte line size (Intel Xeon processor MP,Family F, Model 6)\n"
             "level-2 cache: 4 MB, 16-way set associative, 64-byte line size",
             "level-3 cache: 6 MB, 12-way set associative, 64-byte line size",
             "level-3 cache: 8 MB, 16-way set associative, 64-byte line size",
             "level-3 cache: 12 MB, 12-way set associative, 64-byte line size",
             "level-3 cache: 16 MB, 16-way set associative, 64-byte line size",
    [0x50] = "Instruction TLB: 4 KB, 2 MB or 4 MB pages, fully associative, 64 entries",
             "Instruction TLB: 4 KB, 2 MB or 4 MB pages, fully associative, 128 entries",
             "Instruction TLB: 4 KB, 2 MB or 4 MB pages, fully associative, 256 entries",
    [0x56] = "L0 Data TLB: 4 MB pages, 4-way set associative, 16 entries",
             "L0 Data TLB: 4 MB pages, 4-way set associative, 16 entries",
    [0x5b] = "Data TLB: 4 KB or 4 MB pages, fully associative, 64 entries",
             "Data TLB: 4 KB or 4 MB pages, fully associative, 128 entries",
             "Data TLB: 4 KB or 4 MB pages, fully associative, 256 entries",
    [0x60] = "level-1 data cache: 16 KB, 8-way set associative, sectored cache, 64-byte line size",
    [0x66] = "level-1 data cache: 8 KB, 4-way set associative, sectored cache, 64-byte line size",
             "level-1 data cache: 16 KB, 4-way set associative, sectored cache, 64-byte line size",
             "level-1 data cache: 32 KB, 4 way set associative, sectored cache, 64-byte line size",
    [0x70] = "Trace cache: 12 K-uops, 8-way set associative",
             "Trace cache: 16 K-uops, 8-way set associative",
             "Trace cache: 32 K-uops, 8-way set associative",
             "Trace cache: 64 K-uops, 8-way set associative",
    [0x78] = "level-2 cache: 1 MB, 4-way set associative, 64-byte line size",
             "level-2 cache: 128 KB, 8-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 256 KB, 8-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 512 KB, 8-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 1 MB, 8-way set associative, sectored cache, 64-byte line size",
             "level-2 cache: 2 MB, 8-way set associative, 64-byte line size",
    [0x7f] = "level-2 cache: 512 KB, 2-way set associative, 64-byte line size",
    [0x82] = "level-2 cache: 256 KB, 8-way set associative, 32-byte line size",
             "level-2 cache: 512 KB, 8-way set associative, 32-byte line size",
             "level-2 cache: 1 MB, 8-way set associative, 32-byte line size",
             "level-2 cache: 2 MB, 8-way set associative, 32-byte line size",
             "level-2 cache: 512 KB, 4-way set associative, 64-byte line size",
             "level-2 cache: 1 MB, 8-way set associative, 64-byte line size",
    [0xb0] = "Instruction TLB: 4 KB Pages, 4-way set associative, 128 entries",
             "Instruction TLB: 4 MB Pages, 4-way set associative, 4 entries\n"
             "Instruction TLB: 2 MB Pages, 4-way set associative, 8 entries",
    [0xb3] = "Data TLB: 4 KB Pages, 4-way set associative, 128 entries",
             "Data TLB: 4 KB Pages, 4-way set associative, 256 entries",
    [0xf0] = "64-byte Prefetching",
             "128-byte Prefetching",
};
/*********************************************************************************************************
** 函数名称: x86CpuIdGet
** 功能描述: 获得 CPU 特性集
** 输　入  : NONE
** 输　出  : CPU 特性集
** 全局变量:
** 调用模块:
*********************************************************************************************************/
X86_CPUID  *x86CpuIdGet (VOID)
{
    return   (&_G_x86CpuId);
}
/*********************************************************************************************************
** 函数名称: x86CpuIdAdd
** 功能描述: 增加一个新的 X86_CPUID_ENTRY 到支持的 CPUID 条目表
** 输　入  : pentry        CPUID 条目
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  x86CpuIdAdd (X86_CPUID_ENTRY  *pentry)
{
    X86_CPUID_ENTRY  *pcur = LW_NULL;
    INT               i;
    INT               iError = PX_ERROR;

    for (i = 0; i < _G_iX86CpuEntriesNr; i++) {
        pcur = &_G_x86CpuIdTable[i];
        if (pcur->signature == X86_CPUID_DUMMY) {
            pcur->signature = pentry->signature;
            pcur->family    = pentry->family;
            iError          = ERROR_NONE;
            break;
        }
    }

    return  (iError);
}
/*********************************************************************************************************
** 函数名称: x86CpuIdOverride
** 功能描述: 覆盖 CPU 特性
** 输　入  : pentries      覆盖的 CPUID 条目
**           iCount        条目个数
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  x86CpuIdOverride (X86_CPUID_OVERRIDE  *pentries, INT  iCount)
{
    X86_CPUID           *pcpuid = &_G_x86CpuId;
    X86_CPUID_OVERRIDE  *pentry;
    INT                  i, *piValue;
    INT                  iError = PX_ERROR;

    for (i = 0; i < iCount; i++) {
        pentry = &pentries[i];

        switch (pentry->type) {

        case X86_CPUID_FEATURES_EBX:
        case X86_CPUID_FEATURES_ECX:
        case X86_CPUID_FEATURES_EDX:
        case X86_CPUID_EXT_FEATURES_ECX:
        case X86_CPUID_EXT_FEATURES_EDX:
            piValue = (INT *)((INT *)pcpuid + pentry->type / sizeof(UINT));
            if (pentry->state == X86_CPUID_FEAT_ENABLE) {
                *piValue |= pentry->feat;
            } else if (pentry->state == X86_CPUID_FEAT_DISABLE) {
                *piValue &= ~pentry->feat;
            } else {
                break;
            }
            iError = ERROR_NONE;
            break;

        default:
            break;
        }
    }

    return  (iError);
}
/*********************************************************************************************************
** 函数名称: x86CpuIdProbe
** 功能描述: 探测 CPU 特性集
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  x86CpuIdProbe (VOID)
{
    X86_CPUID                      *pcpuid      = &_G_x86CpuId;
    X86_CPU_FEATURE                *pcpufeature = &_G_x86CpuFeature;
    X86_CPUID_EAX_CACHE_PARAMS      cacheEax[4];
    X86_CPUID_EBX_CACHE_PARAMS      cacheEbx[4];
    X86_CPUID_ECX_CACHE_PARAMS      cacheEcx[4];
    X86_CPUID_EDX_FEATURES          features;
    X86_CPUID_ECX_FEATURES          extendedFeatures;
    UINT                            uiCpuId;
    CHAR                           *pcLine;
    INT                             i;
    CHAR                            cTemp[256];

    x86CpuIdProbeHw(pcpuid);

    features.value         = pcpuid->std.featuresEdx;
    extendedFeatures.value = pcpuid->std.featuresEcx;

    /*
     * 识别 Intel 处理器家族类型码
     */
    uiCpuId = pcpuid->std.signature & (X86_CPUID_FAMILY | X86_CPUID_MODEL | X86_CPUID_EXT_MODEL);

    for (i = 0; i < _G_iX86CpuEntriesNr; i++) {
        if (_G_x86CpuIdTable[i].signature == uiCpuId) {
            pcpufeature->CPUF_uiProcessorFamily = _G_x86CpuIdTable[i].family;
            break;
        }
    }

    if (pcpufeature->CPUF_uiProcessorFamily == X86_FAMILY_UNSUPPORTED) {
        pcpufeature->CPUF_uiProcessorFamily  = X86_FAMILY_PENTIUM;
    }

    /*
     * 识别 CPU 名字
     */
    if (pcpuid->ext.highestExtValue >= 0x80000002) {
        pcLine = (CHAR *)pcpuid->std.brandString;
        SKIP_SPACE(pcLine);
        lib_strcpy(pcpufeature->CPUF_pcCpuInfo, pcLine);
    }

    /*
     * 识别 APIC
     */
    pcpufeature->CPUF_bHasAPIC = features.field.apic ? LW_TRUE : LW_FALSE;


    /*
     * 识别 CACHE 特性
     */
    if (pcpuid->std.featuresEdx & X86_CPUID_CLFLUSH) {
        pcpufeature->CPUF_bHasCLFlush = LW_TRUE;
        pcpufeature->CPUF_stCacheFlushBytes = (size_t)(pcpuid->std.featuresEbx & X86_CPUID_CHUNKS)
                                               >> X86_CPUID_CHUNKS_TO_BYTES_SHIFT;
        _G_stX86CacheFlushBytes = pcpufeature->CPUF_stCacheFlushBytes;
    }

    if (pcpuid->std.highestValue >= 4) {
        lib_strcpy(pcpufeature->CPUF_pcCacheInfo, "");

        for (i = 0; i < pcpuid->ext.cacheCount; i++) {
            cacheEax[i].value = pcpuid->ext.cacheParams[i][0];
            cacheEbx[i].value = pcpuid->ext.cacheParams[i][1];
            cacheEcx[i].sets  = pcpuid->ext.cacheParams[i][2];
        }

        for (i = 0; i < pcpuid->ext.cacheCount; i++) {
            size_t  stSize = (cacheEbx[i].field.assoc + 1) *
                             (cacheEbx[i].field.partitions + 1) *
                             (cacheEbx[i].field.line_size + 1) *
                             (cacheEcx[i].sets + 1);

            if (stSize >= LW_CFG_MB_SIZE) {
                stSize = stSize / LW_CFG_MB_SIZE;
                snprintf(cTemp, sizeof(cTemp), "L%d %s-CACHE %dMB ",
                         cacheEax[i].field.level, _G_pcX86CacheShortTypes[cacheEax[i].field.type],
                         (INT)stSize);

            } else {
                stSize = stSize / LW_CFG_KB_SIZE;
                snprintf(cTemp, sizeof(cTemp), "L%d %s-CACHE %dKB ",
                         cacheEax[i].field.level, _G_pcX86CacheShortTypes[cacheEax[i].field.type],
                         (INT)stSize);
            }

            lib_strcat(pcpufeature->CPUF_pcCacheInfo, cTemp);

            if (cacheEax[i].field.level == 1) {
                if (cacheEax[i].field.type == 1) {
                    pcpufeature->CPUF_iDCacheWaySize = (cacheEbx[i].field.partitions + 1) *
                                                       (cacheEbx[i].field.line_size  + 1) *
                                                       (cacheEcx[i].sets + 1);

                } else if (cacheEax[i].field.type == 2) {
                    pcpufeature->CPUF_iICacheWaySize = (cacheEbx[i].field.partitions + 1) *
                                                       (cacheEbx[i].field.line_size  + 1) *
                                                       (cacheEcx[i].sets + 1);
                }
            }
        }
    }

    /*
     * 识别 FPU MMX SSE AVX 特性
     */
    pcpufeature->CPUF_bHasX87FPU = features.field.fpu           ? LW_TRUE : LW_FALSE;
    pcpufeature->CPUF_bHasSSE    = features.field.sse           ? LW_TRUE : LW_FALSE;
    pcpufeature->CPUF_bHasSSE2   = features.field.sse2          ? LW_TRUE : LW_FALSE;
    pcpufeature->CPUF_bHasFXSR   = features.field.fxsr          ? LW_TRUE : LW_FALSE;
    pcpufeature->CPUF_bHasXSAVE  = extendedFeatures.field.xsave ? LW_TRUE : LW_FALSE;
    pcpufeature->CPUF_bHasAVX    = extendedFeatures.field.avx   ? LW_TRUE : LW_FALSE;
    pcpufeature->CPUF_bHasMMX    = features.field.mmx           ? LW_TRUE : LW_FALSE;

    if (pcpufeature->CPUF_bHasXSAVE && pcpufeature->CPUF_bHasAVX) {
        pcpufeature->CPUF_stXSaveCtxSize = pcpuid->ext.xsaveParamsEcx;
    }

    /*
     * 识别 MTRR PAT 特性
     */
    pcpufeature->CPUF_bHasMTRR = features.field.mtrr ? LW_TRUE : LW_FALSE;
    pcpufeature->CPUF_bHasPAT  = features.field.pat  ? LW_TRUE : LW_FALSE;
}
/*********************************************************************************************************
** 函数名称: x86CpuIdShow
** 功能描述: 显示探测到的 CPU 特性
** 输　入  : pcpuid        CPUID 结构
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  x86CpuIdShow (VOID)
{
    X86_CPUID                      *pcpuid = &_G_x86CpuId;
    X86_CPUID_ENTRY                *pentry;
    X86_CPUID_INFO                  info;
    X86_CPUID_VERSION               version;
    X86_CPUID_EDX_FEATURES          features;
    X86_CPUID_ECX_FEATURES          extendedFeatures;
    X86_CPUID_EDX_FEATURES_EXT      extendedFeaturesD;
    X86_CPUID_ECX_FEATURES_EXT      extendedFeaturesC;

    X86_CPUID_EAX_CACHE_PARAMS      cacheEax[4];
    X86_CPUID_EBX_CACHE_PARAMS      cacheEbx[4];
    X86_CPUID_ECX_CACHE_PARAMS      cacheEcx[4];
    X86_CPUID_EDX_CACHE_PARAMS      cacheEdx[4];

    X86_CPUID_EAX_MONITOR_PARAMS    monEax;
    X86_CPUID_EBX_MONITOR_PARAMS    monEbx;
    X86_CPUID_ECX_MONITOR_PARAMS    monEcx;
    X86_CPUID_EDX_MONITOR_PARAMS    monEdx;

    X86_CPUID_EAX_DTSPM_PARAMS      dtsEax;
    X86_CPUID_EBX_DTSPM_PARAMS      dtsEbx;
    X86_CPUID_ECX_DTSPM_PARAMS      dtsEcx;
    X86_CPUID_EDX_DTSPM_PARAMS      dtsEdx __unused;

    X86_CPUID_EAX_DCA_PARAMS        dcaEax;

    X86_CPUID_EAX_PMON_PARAMS       pmonEax;
    X86_CPUID_EBX_PMON_PARAMS       pmonEbx;
    X86_CPUID_EDX_PMON_PARAMS       pmonEdx __unused;

    X86_CPUID_EAX_PTOP_PARAMS       ptopEax[16];
    X86_CPUID_EBX_PTOP_PARAMS       ptopEbx[16];
    X86_CPUID_ECX_PTOP_PARAMS       ptopEcx[16];
    X86_CPUID_EDX_PTOP_PARAMS       ptopEdx[16];

    X86_CPUID_ECX_L2_PARAMS         l2cacheEcx;

    X86_CPUID_EDX_APM_PARAMS        apmEdx;

    X86_CPUID_EAX_VPADRSIZES_PARAMS vpadrEax;

    UINT64                          ulCpuSerial;
    CHAR                           *pcCpuTypeName = LW_NULL;
    CHAR                           *pcLine;
    UINT                            uiX86Processor = X86_FAMILY_UNSUPPORTED;
    INT                             i, iCpuIdMask;


    lib_bzero((VOID *)cacheEax, 4 * sizeof(X86_CPUID_EAX_CACHE_PARAMS));
    lib_bzero((VOID *)cacheEbx, 4 * sizeof(X86_CPUID_EAX_CACHE_PARAMS));
    lib_bzero((VOID *)cacheEcx, 4 * sizeof(X86_CPUID_EAX_CACHE_PARAMS));
    lib_bzero((VOID *)cacheEdx, 4 * sizeof(X86_CPUID_EAX_CACHE_PARAMS));

    lib_bzero((VOID *)ptopEax, 16 * sizeof(X86_CPUID_EAX_PTOP_PARAMS));
    lib_bzero((VOID *)ptopEbx, 16 * sizeof(X86_CPUID_EAX_PTOP_PARAMS));
    lib_bzero((VOID *)ptopEcx, 16 * sizeof(X86_CPUID_EAX_PTOP_PARAMS));
    lib_bzero((VOID *)ptopEdx, 16 * sizeof(X86_CPUID_EAX_PTOP_PARAMS));

    info.value              = pcpuid->std.featuresEbx;
    version.value           = pcpuid->std.signature;
    features.value          = pcpuid->std.featuresEdx;
    extendedFeatures.value  = pcpuid->std.featuresEcx;
    extendedFeaturesD.value = pcpuid->ext.featuresExtEdx;
    extendedFeaturesC.value = pcpuid->ext.featuresExtEcx;

    printf("\nX86 CPU probe report\n\n");
    printf("     signature: 0x%X\n", pcpuid->std.signature);
    printf("   featuresEbx: 0x%X\n", pcpuid->std.featuresEbx);
    printf("   featuresEcx: 0x%X\n", pcpuid->std.featuresEcx);
    printf("   featuresEdx: 0x%X\n", pcpuid->std.featuresEdx);
    printf("featuresEcxExt: 0x%X\n", pcpuid->ext.featuresExtEcx);
    printf("featuresEdxExt: 0x%X\n", pcpuid->ext.featuresExtEdx);

    if (pcpuid->std.highestValue >= 2) {
        printf("      cacheEax: 0x%X\n", pcpuid->std.cacheEax);
        printf("      cacheEbx: 0x%X\n", pcpuid->std.cacheEbx);
        printf("      cacheEcx: 0x%X\n", pcpuid->std.cacheEcx);
        printf("      cacheEdx: 0x%X\n", pcpuid->std.cacheEdx);
    }
    printf("    maxFuncEax: 0x%X\n", pcpuid->std.highestValue);
    printf(" maxFuncEaxExt: 0x%X\n", pcpuid->ext.highestExtValue);

    printf("\nCPU info:\n");

    if (pcpuid->ext.highestExtValue >= 0x80000002) {
        pcLine = (CHAR *)pcpuid->std.brandString;
        SKIP_SPACE(pcLine);
        printf("    %s\n", pcLine);
    }

    printf("    model=0x%X/%d  stepping=%d  family=%d/%d\n",
           version.field.model, version.field.modelExt,
           version.field.stepid,
           version.field.family, version.field.familyExt);

    switch ((INT)version.field.type) {

    case X86_CPUID_ORIG:
        pcCpuTypeName = "original OEM";
        break;

    case X86_CPUID_OVERD:
        pcCpuTypeName = "overdrive";
        break;

    case X86_CPUID_DUAL:
        pcCpuTypeName = "dual";
        break;

    default:
        pcCpuTypeName = "<unknown>";
        break;
    }

    iCpuIdMask = X86_CPUID_FAMILY | X86_CPUID_MODEL | X86_CPUID_EXT_MODEL;
    for (i = 0; i < _G_iX86CpuEntriesNr; i++) {
        pentry = &_G_x86CpuIdTable[i];
        if ((pentry->signature & iCpuIdMask) == (pcpuid->std.signature & iCpuIdMask)) {
            uiX86Processor = pentry->family;
            break;
        }
    }

    printf("    x86 processor architecture is %s, type %s\n",
           ((uiX86Processor <= _G_iCpuFamilyNr) && (uiX86Processor != X86_FAMILY_UNSUPPORTED)) ?
           _G_pcX86CpuFamilyNames[uiX86Processor] : "unsupported",
           pcCpuTypeName);

    if (features.field.psnum) {
        ulCpuSerial = (UINT64)pcpuid->std.serialNo64[0] << 32 | pcpuid->std.serialNo64[1];
        printf("    CPU serial number: %lld\n", ulCpuSerial);
    }

    printf("    Number of processors: %d\n", info.field.nproc);
    printf("    Local APIC id: %d\n", info.field.apicId);

    printf("\nCPU features:\n");

    /*
     * 特性位
     */
    X86_CPUID_PRINT_FEATURE(features.field.fpu, "FPU on chip");
    X86_CPUID_PRINT_FEATURE(features.field.vme, "Virtual 8086 mode enhancement");
    X86_CPUID_PRINT_FEATURE(features.field.de, "Debugging extensions");
    X86_CPUID_PRINT_FEATURE(features.field.pse, "Page size extension");
    X86_CPUID_PRINT_FEATURE(features.field.tsc, "Time stamp counter");
    X86_CPUID_PRINT_FEATURE(features.field.msr, "RDMSR and WRMSR support");
    X86_CPUID_PRINT_FEATURE(features.field.pae, "Physical address extensions");
    X86_CPUID_PRINT_FEATURE(features.field.mce, "Machine check exception");
    X86_CPUID_PRINT_FEATURE(features.field.cx8, "CMPXCHG8 inst");
    X86_CPUID_PRINT_FEATURE(features.field.apic, "APIC on chip");
    X86_CPUID_PRINT_FEATURE(features.field.sep, "Fast system calls");
    X86_CPUID_PRINT_FEATURE(features.field.mtrr, "MTRR");
    X86_CPUID_PRINT_FEATURE(features.field.pge, "PTE global bit");
    X86_CPUID_PRINT_FEATURE(features.field.mca, "Machine check architecture");
    X86_CPUID_PRINT_FEATURE(features.field.cmov, "Cond. move/cmp. inst");
    X86_CPUID_PRINT_FEATURE(features.field.pat, "Page attribute table");
    X86_CPUID_PRINT_FEATURE(features.field.pse36, "36 bit page size extension");
    X86_CPUID_PRINT_FEATURE(features.field.psnum, "Processor serial number");
    X86_CPUID_PRINT_FEATURE(features.field.clflush, "CLFLUSH inst supported");
    X86_CPUID_PRINT_FEATURE(features.field.dts, "Debug Store");
    X86_CPUID_PRINT_FEATURE(features.field.acpi, "ACPI registers in MSR space supported");
    X86_CPUID_PRINT_FEATURE(features.field.mmx, "MMX technology supported");
    X86_CPUID_PRINT_FEATURE(features.field.fxsr, "Fast FP save and restore");
    X86_CPUID_PRINT_FEATURE(features.field.sse, "SSE supported");
    X86_CPUID_PRINT_FEATURE(features.field.sse2, "SSE2 supported");
    X86_CPUID_PRINT_FEATURE(features.field.ss, "Self Snoop supported");
    X86_CPUID_PRINT_FEATURE((features.field.htt && (info.field.nproc > 1)),
                           "Hyper-Threading Technology/Core Multi-Processing supported");
    X86_CPUID_PRINT_FEATURE(features.field.tm, "Thermal Monitor supported");
    X86_CPUID_PRINT_FEATURE(features.field.pbe, "Pending break enable");

    /*
     * 扩展特性位
     */
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.sse3, "SSE3 supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.pclmuldq, "PCLMULDQ inst supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.dtes64, "64-bit debug store supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.mon, "MONITOR/MWAIT inst supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.ds_cpl, "CPL qualified debug store");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.vmx, "VT Technology");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.smx, "Safer mode extensions supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.est, "Enhanced Speedstep Technology");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.tm2, "Thermal Monitor 2 supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.ssse3, "SSSE3 supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.cid, "L1 Context ID supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.debugmsr, "IA32_DEBUG_INTERFACE_MSR supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.fma, "FMA Extensions Using XMM state supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.cx16, "CMPXCHG16B inst supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.xtpr, "xTPR Update Control supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.pdcm, "Performance and Debug capability");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.asid_pcid, "ASID-PCID supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.dca, "Direct Cache Access supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.sse41, "SSE4.1 supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.sse42, "SSE4.2 supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.x2apic, "x2APIC feature supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.movbe, "MOVBE inst supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.popcnt, "POPCNT inst supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.tsc_dline, "TSC-deadline Timer supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.aes, "AES inst supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.xsave, "XSAVE/XRSTOR States supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.osxsave,
            "OS-Enabled Extended State management supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.avx, "AVX instr extensions supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.f16c, "Float16 instructions supported");
    X86_CPUID_PRINT_FEATURE(extendedFeatures.field.rdrand, "Read Random Number instructions supported");

    if (pcpuid->ext.highestExtValue >= 0x80000001) {
        X86_CPUID_PRINT_FEATURE(extendedFeaturesD.field.scall_sret, "SYSCALL/SYSRET inst supported");
        X86_CPUID_PRINT_FEATURE(extendedFeaturesD.field.excdis, "Execute Disable supported");
        X86_CPUID_PRINT_FEATURE(extendedFeaturesD.field.gbpss, "1 GB Page Size Support supported");
        X86_CPUID_PRINT_FEATURE(extendedFeaturesD.field.rdtscp,
                "RDTSCP instruction and IA32_TSC_AUX_MSR supported");
        X86_CPUID_PRINT_FEATURE(extendedFeaturesD.field.em64t, "Intel 64 Arch extensions supported");
        X86_CPUID_PRINT_FEATURE(extendedFeaturesC.field.lahf, "LAHF/SAHF inst supported");
        X86_CPUID_PRINT_FEATURE(extendedFeaturesC.field.abm, "Advanced Bit Manipulation supported");
    }

    if (pcpuid->std.highestValue >= 4) {
        for (i = 0; i < pcpuid->ext.cacheCount; i++) {
            cacheEax[i].value = pcpuid->ext.cacheParams[i][0];
            cacheEbx[i].value = pcpuid->ext.cacheParams[i][1];
            cacheEcx[i].sets  = pcpuid->ext.cacheParams[i][2];
            cacheEdx[i].value = pcpuid->ext.cacheParams[i][3];
        }

        printf("\nDeterministic Cache Parameters:\n");

        for (i = 0; i < pcpuid->ext.cacheCount; i++) {
            printf("    L%d %s cache size 0x%x\n",
                   cacheEax[i].field.level, _G_pcX86CacheTypes[cacheEax[i].field.type],
                   (cacheEbx[i].field.assoc + 1) *
                   (cacheEbx[i].field.partitions + 1) *
                   (cacheEbx[i].field.line_size + 1) *
                   (cacheEcx[i].sets + 1) );
            printf("        self initializing cache level: %d\n", cacheEax[i].field.self_init);
            printf("        fully associative cache level: %d\n", cacheEax[i].field.assoc);
            printf("        max threads sharing cache: %d\n", cacheEax[i].field.threads + 1);
            printf("        num reserved APIC IDs: %d\n", cacheEax[i].field.apics + 1);

            printf("        system coherency line size: %d\n", cacheEbx[i].field.line_size + 1);
            printf("        physical line partitions: %d\n", cacheEbx[i].field.partitions + 1);
            printf("        ways of associativity: %d\n", cacheEbx[i].field.assoc + 1);
            printf("        number of sets: %d\n", cacheEcx[i].sets + 1);

            printf("        WBINDV/INDBV behavior on lower levels: %d\n", cacheEdx[i].field.wbindv);
            printf("        inclusive to lower levels: %d\n", cacheEdx[i].field.inclusive);
            printf("        uses complex function to index: %d\n", cacheEdx[i].field.complex_addr);
        }
    }

    if (pcpuid->std.highestValue >= 5) {
        if (extendedFeatures.field.mon) {
            monEax.value = pcpuid->ext.monitorParamsEax;
            monEbx.value = pcpuid->ext.monitorParamsEbx;
            monEcx.value = pcpuid->ext.monitorParamsEcx;
            monEdx.value = pcpuid->ext.monitorParamsEdx;

            printf("\nMONITOR/MWAIT Parameters:\n");
            printf("    smallest line size: %d bytes\n", monEax.field.smline);
            printf("    largest line size: %d bytes\n", monEbx.field.lgline);
            printf("    num C0 sub-states supported: %d\n", monEdx.field.c0);
            printf("    num C1 sub-states supported: %d\n", monEdx.field.c1);
            printf("    num C2 sub-states supported: %d\n", monEdx.field.c2);
            printf("    num C3/6 sub-states supported: %d\n", monEdx.field.c3_6);
            printf("    num C4/7 sub-states supported: %d\n", monEdx.field.c4_7);

            X86_CPUID_PRINT_FEATURE(monEcx.field.mwait, "MONTIOR/MWAIT extension supported");
            X86_CPUID_PRINT_FEATURE(monEcx.field.intr_break,
                    "interrupt break-events for MWAIT supported");
        }
    }

    if (pcpuid->std.highestValue >= 6) {
        dtsEax.value = pcpuid->ext.dtspmParamsEax;
        dtsEbx.value = pcpuid->ext.dtspmParamsEbx;
        dtsEcx.value = pcpuid->ext.dtspmParamsEcx;
        dtsEdx.value = pcpuid->ext.dtspmParamsEdx;

        printf("\nDigital Thermal Sensor and Power Management Parameters:\n");
        printf("    number of interrupt thresholds: %d\n", dtsEbx.field.intr);
        X86_CPUID_PRINT_FEATURE(dtsEax.field.dts, "digital thermal sensor capability");
        X86_CPUID_PRINT_FEATURE(dtsEax.field.turbo, "Turbo Boost technology");
        X86_CPUID_PRINT_FEATURE(dtsEax.field.invar_apic, "Invariant Apic Timer");
        X86_CPUID_PRINT_FEATURE(dtsEax.field.pln_at_core, "Power Limit Notification at Corei Level");
        X86_CPUID_PRINT_FEATURE(dtsEax.field.fgcm, "Fine Grained Clock Modulation");
        X86_CPUID_PRINT_FEATURE(dtsEax.field.thermint_stat,
                "Package Thermal Interrupt and Status support");
        X86_CPUID_PRINT_FEATURE(dtsEcx.field.hcfc, "hardware coordination feedback capability");
        X86_CPUID_PRINT_FEATURE(dtsEcx.field.acnt2, "ACNT2 Capability");
        X86_CPUID_PRINT_FEATURE(dtsEcx.field.efps, "Energy Efficient Policy support");
    }

    if ((pcpuid->std.highestValue >= 9) && extendedFeatures.field.dca) {
        dcaEax.value = pcpuid->ext.dcaParamsEax;

        printf("\nDirect Cache Access(DCA) Parameters:\n");
        printf("    PLATFORM_DCA_CAP MSR bits: 0x%x\n", dcaEax.field.dca_cap);
    }

    if (pcpuid->std.highestValue >= 0xa) {
        pmonEax.value = pcpuid->ext.pMonParamsEax;
        pmonEbx.value = pcpuid->ext.pMonParamsEbx;
        pmonEdx.value = pcpuid->ext.pMonParamsEdx;

        printf("\nPerformance Monitor Features:\n");
        printf("    PerfMon version: %d\n", pmonEax.field.ver);
        printf("    ncounters per processor: %d\n", pmonEax.field.ncounters);
        printf("    nbits per counter: %d\n", pmonEax.field.nbits);
        printf("    nevents per processor: %d\n", pmonEax.field.nevts);

        X86_CPUID_PRINT_FEATURE_INV(pmonEbx.field.core_cycles, "core cycles supported");
        X86_CPUID_PRINT_FEATURE_INV(pmonEbx.field.instr_ret, "instructions retired supported");
        X86_CPUID_PRINT_FEATURE_INV(pmonEbx.field.ref_cycles, "reference cycles supported");
        X86_CPUID_PRINT_FEATURE_INV(pmonEbx.field.cache_ref, "last level cache references supported");
        X86_CPUID_PRINT_FEATURE_INV(pmonEbx.field.cache_miss, "last level cache misses supported");
        X86_CPUID_PRINT_FEATURE_INV(pmonEbx.field.br_instr, "branch instructions retired supported");
        X86_CPUID_PRINT_FEATURE_INV(pmonEbx.field.br_mispr, "branch mispredicts retired supported");
    }

    if (pcpuid->std.highestValue >= 0xb) {
        for (i = 0; i < pcpuid->ext.pTopCount; i++) {
            ptopEax[i].value = pcpuid->ext.pTopParams[i][0];
            ptopEbx[i].value = pcpuid->ext.pTopParams[i][1];
            ptopEcx[i].value = pcpuid->ext.pTopParams[i][2];
            ptopEdx[i].value = pcpuid->ext.pTopParams[i][3];
        }

        printf("\nx2APIC Features / Processor Topology:\n");
        for (i = 0; i < pcpuid->ext.pTopCount; i++) {
            if (ptopEcx[i].field.lvl_num == 0) {
                printf("    Thread level topology:\n");
            } else if (ptopEcx[i].field.lvl_num == 1) {
                printf("    Core level topology:\n");
            } else {
                printf("    Package level topology:\n");
            }
            printf("        number of logical processors: %d\n", ptopEbx[i].field.nproc);
            printf("        extended APIC ID: %d\n", ptopEdx[i].field.xapic_id);
            printf("        bits to shift right for APIC ID: %d\n", ptopEax[i].field.nbits);
        }
    }

    if (pcpuid->std.highestValue >= 0xd) {
        printf("\nProcessor Extended State Enumeration (XSAVE):\n");
        printf("    XFEATURE_ENABLED_MASK valid bitfield lower 32: 0x%08x\n", pcpuid->ext.xsaveParamsEax);
        printf("    XFEATURE_ENABLED_MASK valid bitfield upper 32: 0x%08x\n", pcpuid->ext.xsaveParamsEdx);
        printf("    max size required by XFEATURE_ENABLED_MASK: %d bytes\n", pcpuid->ext.xsaveParamsEbx);
        printf("    max size of XSAVE/XRESTORE save area: %d bytes\n", pcpuid->ext.xsaveParamsEcx);
    }

    if (pcpuid->std.highestValue >= 2) {
        printf("\nCache Features:\n");

        UCHAR  *pcValue = (UCHAR *)&pcpuid->std.cacheEax;
        INT     j;

        for (j = 0; j < 4; j++) {
            for (i = 3; i >= 0; i--) {
                if (_G_x86TlbCacheDesc[pcValue[i]]) {
                    printf("    %s\n", _G_x86TlbCacheDesc[pcValue[i]]);
                }
            }
            pcValue += 4;
        }
    }

    if (pcpuid->ext.highestExtValue < 0x80000006) {
        return;
    }

    l2cacheEcx.value = pcpuid->ext.l2ParamsEcx;

    if (l2cacheEcx.value != 0) {
        printf("\nExtended L2 Cache Features:\n");
        printf("    L2 cache line size: %d bytes\n", l2cacheEcx.field.cache_line);
        printf("    L2 cache size: %d kbytes\n", l2cacheEcx.field.cache_size);
        printf("    L2 cache associativity: %s\n", _G_x86L2CacheAssoc[l2cacheEcx.field.assoc]);
    }

    if (pcpuid->ext.highestExtValue < 0x80000007) {
        return;
    }

    apmEdx.value = pcpuid->ext.apmParamsEdx;

    printf("\nAdvanced Power Management:\n");
    X86_CPUID_PRINT_FEATURE(apmEdx.field.tsc_inv, "TSC Invariance Available");

    if (pcpuid->ext.highestExtValue < 0x80000008) {
        return;
    }

    vpadrEax.value = pcpuid->ext.vpAdrSizesEax;

    printf("\nVirtual and Physical Address Sizes:\n");
    printf("    Physical address size: %d bits\n", vpadrEax.field.phys);
    printf("    Virtual address size: %d bits\n\n", vpadrEax.field.virt);
}
/*********************************************************************************************************
** 函数名称: x86CpuIdBitField
** 功能描述: extracts a Bit field from an 8 bit ID
** 输　入  : ucFullId           8 bit ID
**           ucMaxSubIdValue    determines the width of bit field to extract from ucFullId
**           ucShiftCount       specifies offset from the right most bit of the 8-bit ucFullId
** 输　出  : representing a subset of bits based on input
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT8  x86CpuIdBitField (UINT8  ucFullId, UINT8  ucMaxSubIdValue, UINT8  ucShiftCount)
{
    UINT   uiMaskWidth;
    UINT8  ucSubId, ucMaskBits;

    uiMaskWidth = x86CpuIdBitFieldWidth((UINT)ucMaxSubIdValue);

    ucMaskBits = (((UINT8)(0xff << ucShiftCount)) ^
                  ((UINT8)(0xff << (ucShiftCount + uiMaskWidth))));

    ucSubId = ucFullId & ucMaskBits;

    return  (ucSubId);
}
/*********************************************************************************************************
** 函数名称: x86CpuIdMaxNumLProcsPerCore
** 功能描述: 获得每个 Core 可以容纳的最大的逻辑处理器数
** 输　入  : NONE
** 输　出  : 每个 Core 可以容纳的最大的逻辑处理器数
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT  x86CpuIdMaxNumLProcsPerCore (VOID)
{
    return  ((UINT)(x86CpuIdMaxNumLProcsPerPkg() / x86CpuIdMaxNumCoresPerPkg()));
}
/*********************************************************************************************************
** 函数名称: x86CpuIdCalcFreq
** 功能描述: 从 CPUID 获得 CPU 频率
** 输　入  : NONE
** 输　出  : CPU 频率(Hz)
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT64  x86CpuIdCalcFreq (VOID)
{
    X86_CPUID  *pcpuid = &_G_x86CpuId;
    CHAR        cIntBuf[8] = { 0 };
    CHAR        cMinBuf[8] = { 0 };
    CHAR       *pcStr;
    CHAR       *pcDot;
    CHAR       *pcFreqStart;
    UINT64      ui64Multiple;
    UINT64      ui64TmpFreq;
    UINT64      ui64Freq;

    /*
     * Get frequency string from CPUID
     */
    pcStr = (CHAR *)&(pcpuid->std.brandString[0]);

    /*
     * Using pattern matching first
     */
    pcFreqStart = lib_strstr(pcStr, "Hz");
    if (pcFreqStart == LW_NULL) {
        return  (0);
    }

    /*
     * pcDot points to 'M' or 'G' of "MHz" or "GHz"
     */
    pcDot = --pcFreqStart;

    switch (*pcDot) {

    case 'M':
        ui64Multiple = 1000000;
        break;

    case 'T':
        ui64Multiple = 1000000000000;
        break;

    case 'G':
    default:
        ui64Multiple = 1000000000;
        break;
    }

    while (*pcFreqStart != ' ') {
        if (*pcFreqStart == '.') {
            lib_strncpy(cMinBuf, pcFreqStart + 1, pcDot - (pcFreqStart + 1));
            pcDot = pcFreqStart;
        }
        pcFreqStart--;
    }

    lib_strncpy(cIntBuf, pcFreqStart + 1, pcDot - (pcFreqStart + 1));

    /*
     * Get frequency in Hz, note: can not use floating compute, or it will
     * cause exception at initial stage.
     */
    ui64TmpFreq = lib_atoi(cIntBuf);
    ui64Freq    = ui64TmpFreq * ui64Multiple;

    /*
     * Skip the leading '0' after '.'
     */
    pcFreqStart = cMinBuf;
    while (*pcFreqStart == '0') {
        pcFreqStart++;
        ui64Multiple = ui64Multiple / 10;
    }
    ui64TmpFreq = lib_atoi(pcFreqStart);

    /*
     * Placing division first can make the posibility of overflow lower
     */
    ui64TmpFreq = (ui64Multiple / 10) * ui64TmpFreq;

    /*
     * The decimal part should be less than multiple
     */
    while (ui64TmpFreq >= ui64Multiple) {
        ui64TmpFreq = ui64TmpFreq / 10;
    }

    /*
     * Actually the frequency is multiplied by bus frequency, fix it:
     * the theory here is that the bus frequency is multiple of 33Mhz, or,
     * 100,000,000/3 Hz. for example, 100Mhz = 33Mhz * 3, 133 = 33Mhz * 4,
     * 167Mhz = 33Mhz * 5, etc.
     * The following expression get the number that is multiple of 33MHz
     * and closest to literal value in brand string.
     */

    /*
     * Get number that is the most near to the multiple of 33MHz
     */
    ui64Freq = ((ui64Freq + ui64TmpFreq + 16666667) / 33333333) * 33333333;

    _G_ui64X86CpuIdFreq = ui64Freq;

    return  (ui64Freq);
}
/*********************************************************************************************************
** 函数名称: x86CpuIdGetFreq
** 功能描述: 获得保存的 CPU 频率
** 输　入  : NONE
** 输　出  : CPU 频率(Hz)
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT64  x86CpuIdGetFreq (VOID)
{
    return  (_G_ui64X86CpuIdFreq);
}
/*********************************************************************************************************
** 函数名称: x86CpuIdSetFreq
** 功能描述: 设置保存的 CPU 频率
** 输　入  : ui64Freq      CPU 频率(Hz)
** 输　出  : ERROR CODE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  x86CpuIdSetFreq (UINT64  ui64Freq)
{
    _G_ui64X86CpuIdFreq = ui64Freq;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
