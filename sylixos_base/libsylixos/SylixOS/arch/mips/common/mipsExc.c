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
** 文   件   名: mipsExc.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 09 月 01 日
**
** 描        述: MIPS 体系架构异常处理.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "dtrace.h"
#include "arch/mips/common/cp0/mipsCp0.h"
#include "arch/mips/common/unaligned/mipsUnaligned.h"
#if LW_CFG_VMM_EN > 0
#include "arch/mips/mm/mmu/mipsMmuCommon.h"
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
#if LW_CFG_CPU_FPU_EN > 0
#include "arch/mips/fpu/emu/mipsFpuEmu.h"
#include "arch/mips/fpu/fpu32/mipsVfp32.h"
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
/*********************************************************************************************************
  MIPS 异常处理函数类型
*********************************************************************************************************/
typedef VOID  (*MIPS_EXCEPT_HANDLE)(addr_t         ulRetAddr,           /*  返回地址                    */
                                    addr_t         ulAbortAddr,         /*  终止地址                    */
                                    ARCH_REG_CTX  *pregctx);            /*  寄存器上下文                */
/*********************************************************************************************************
  向量使能与禁能锁
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
#define VECTOR_OP_LOCK()    LW_SPIN_LOCK_IGNIRQ(&_K_slcaVectorTable.SLCA_sl)
#define VECTOR_OP_UNLOCK()  LW_SPIN_UNLOCK_IGNIRQ(&_K_slcaVectorTable.SLCA_sl)
#else
#define VECTOR_OP_LOCK()
#define VECTOR_OP_UNLOCK()
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
** 函数名称: bspCpuExcHook
** 功能描述: 处理器异常回调
** 输　入  : ptcb       异常上下文
**           ulRetAddr  异常返回地址
**           ulExcAddr  异常地址
**           iExcType   异常类型
**           iExcInfo   体系结构相关异常信息
** 输　出  : 0
** 全局变量:
** 调用模块:
*********************************************************************************************************/
#if LW_CFG_CPU_EXC_HOOK_EN > 0

LW_WEAK INT  bspCpuExcHook (PLW_CLASS_TCB   ptcb,
                            addr_t          ulRetAddr,
                            addr_t          ulExcAddr,
                            INT             iExcType,
                            INT             iExcInfo)
{
    return  (0);
}

#endif                                                                  /*  LW_CFG_CPU_EXC_HOOK_EN      */
/*********************************************************************************************************
** 函数名称: archIntHandle
** 功能描述: bspIntHandle 需要调用此函数处理中断 (关闭中断情况被调用)
** 输　入  : ulVector         中断向量
**           bPreemptive      中断是否可抢占
** 输　出  : NONE
** 全局变量:
** 调用模块:
** 注  意  : 此函数退出时必须为中断关闭状态.
*********************************************************************************************************/
LW_WEAK VOID  archIntHandle (ULONG  ulVector, BOOL  bPreemptive)
{
    REGISTER irqreturn_t irqret;

    if (_Inter_Vector_Invalid(ulVector)) {
        return;                                                         /*  向量号不正确                */
    }

    if (LW_IVEC_GET_FLAG(ulVector) & LW_IRQ_FLAG_PREEMPTIVE) {
        bPreemptive = LW_TRUE;
    }

    if (bPreemptive) {
        VECTOR_OP_LOCK();
        __ARCH_INT_VECTOR_DISABLE(ulVector);                            /*  屏蔽 vector 中断            */
        VECTOR_OP_UNLOCK();
        KN_INT_ENABLE_FORCE();                                          /*  允许中断                    */
    }

    irqret = API_InterVectorIsr(ulVector);                              /*  调用中断服务程序            */

    KN_INT_DISABLE();                                                   /*  禁能中断                    */

    if (bPreemptive) {
        if (irqret != LW_IRQ_HANDLED_DISV) {
            VECTOR_OP_LOCK();
            __ARCH_INT_VECTOR_ENABLE(ulVector);                         /*  允许 vector 中断            */
            VECTOR_OP_UNLOCK();
        }

    } else if (irqret == LW_IRQ_HANDLED_DISV) {
        VECTOR_OP_LOCK();
        __ARCH_INT_VECTOR_DISABLE(ulVector);                            /*  屏蔽 vector 中断            */
        VECTOR_OP_UNLOCK();
    }
}
/*********************************************************************************************************
** 函数名称: archCacheErrorHandle
** 功能描述: Cache 错误处理
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
#if LW_CFG_CACHE_EN > 0

VOID  archCacheErrorHandle (ARCH_REG_CTX  *pregctx)
{
    REGISTER UINT32         uiFiled  = 2 * sizeof(UINT32);
    REGISTER ARCH_REG_T     ulRegVal;
             PLW_CLASS_TCB  ptcbCur;
             LW_VMM_ABORT   abtInfo;

    ulRegVal = mipsCp0ConfigRead();
    mipsCp0ConfigWrite((ulRegVal & ~M_ConfigK0) | MIPS_UNCACHED);

    _PrintFormat("Cache error exception:\r\n");
    _PrintFormat("cp0_error epc == %lx\r\n", uiFiled, mipsCp0ERRPCRead());

    ulRegVal = mipsCp0CacheErrRead();

    _PrintFormat("cp0_cache error == 0x%08x\r\n", ulRegVal);
    _PrintFormat("Decoded cp0_cache error: %s cache fault in %s reference.\r\n",
                 (ulRegVal & M_CacheLevel) ? "secondary" : "primary",
                 (ulRegVal & M_CacheType)  ? "d-cache"   : "i-cache");

    _PrintFormat("Error bits: %s%s%s%s%s%s%s\r\n",
                 (ulRegVal & M_CacheData) ? "ED " : "",
                 (ulRegVal & M_CacheTag)  ? "ET " : "",
                 (ulRegVal & M_CacheECC)  ? "EE " : "",
                 (ulRegVal & M_CacheBoth) ? "EB " : "",
                 (ulRegVal & M_CacheEI)   ? "EI " : "",
                 (ulRegVal & M_CacheE1)   ? "E1 " : "",
                 (ulRegVal & M_CacheE0)   ? "E0 " : "");

    _PrintFormat("IDX: 0x%08x\r\n", ulRegVal & (M_CacheE0 - 1));

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = 0;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_FATAL_ERROR;
    API_VmmAbortIsr(pregctx->REG_ulCP0EPC, pregctx->REG_ulCP0EPC, &abtInfo, ptcbCur);
}

#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
/*********************************************************************************************************
** 函数名称: archTlbModExceptHandle
** 功能描述: TLB modified 异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archTlbModExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    /*
     * A TLB modify exception occurs when a TLB entry matches a store reference to a mapped address,
     * but the matched entry is not dirty.
     */

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_WRITE;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_PERM;
    API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archTlbLoadExceptHandle
** 功能描述: TLB load or ifetch 异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archTlbLoadExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_READ;
#if LW_CFG_VMM_EN > 0
    abtInfo.VMABT_uiType   = mipsMmuTlbLoadStoreExcHandle(ulAbortAddr, LW_FALSE);
#else
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
    if (abtInfo.VMABT_uiType) {
        API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
    }
}
/*********************************************************************************************************
** 函数名称: archTlbStoreExceptHandle
** 功能描述: TLB store 异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archTlbStoreExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_WRITE;
#if LW_CFG_VMM_EN > 0
    abtInfo.VMABT_uiType   = mipsMmuTlbLoadStoreExcHandle(ulAbortAddr, LW_TRUE);
#else
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
    if (abtInfo.VMABT_uiType) {
        API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
    }
}
/*********************************************************************************************************
** 函数名称: archAddrLoadExceptHandle
** 功能描述: Address load or ifetch 异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
**           pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archAddrLoadExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr, ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    /*
     *  An address error exception occurs in following cases:
     *  1. An instruction fetch or a data load/store accesses a word from a misaligned word address
     *     boundary.
     *  2. A data load/store accesses a half-word from a misaligned half-word address boundary.
     *  3. Reference kernel address space in user mode.
     *
     *  TODO: 由于现在未使用用户模式, 所以第三种情况暂不用处理
     */

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_READ;
    abtInfo.VMABT_uiType   = mipsUnalignedHandle(pregctx, ulAbortAddr);
    if (abtInfo.VMABT_uiType) {
        API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
    }
}
/*********************************************************************************************************
** 函数名称: archAddrStoreExceptHandle
** 功能描述: Address store 异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
**           pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archAddrStoreExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr, ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_WRITE;
    abtInfo.VMABT_uiType   = mipsUnalignedHandle(pregctx, ulAbortAddr);
    if (abtInfo.VMABT_uiType) {
        API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
    }
}
/*********************************************************************************************************
** 函数名称: archInstBusExceptHandle
** 功能描述: Instruction Bus 异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archInstBusExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

#if LW_CFG_CPU_EXC_HOOK_EN > 0
    if (bspCpuExcHook(ptcbCur, ulRetAddr, ulAbortAddr, ARCH_BUS_EXCEPTION, 0)) {
        return;
    }
#endif                                                                  /*  LW_CFG_CPU_EXC_HOOK_EN > 0  */

    abtInfo.VMABT_uiMethod = BUS_ADRERR;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_BUS;
    API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archDataBusExceptHandle
** 功能描述: Data Bus 异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archDataBusExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

#if LW_CFG_CPU_EXC_HOOK_EN > 0
    if (bspCpuExcHook(ptcbCur, ulRetAddr, ulAbortAddr, ARCH_BUS_EXCEPTION, 1)) {
        return;
    }
#endif                                                                  /*  LW_CFG_CPU_EXC_HOOK_EN > 0  */

    abtInfo.VMABT_uiMethod = BUS_ADRERR;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_BUS;
    API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archSysCallHandle
** 功能描述: System Call 处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archSysCallHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = 0;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_SYS;
    API_VmmAbortIsr(ulRetAddr, ulRetAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archBreakPointHandle
** 功能描述: Break Point 处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
**           pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archBreakPointHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr, ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;
#if LW_CFG_GDB_EN > 0
    UINT           uiBpType;
#endif                                                                  /*  LW_CFG_GDB_EN > 0           */

    LW_TCB_GET_CUR(ptcbCur);

#if LW_CFG_CPU_FPU_EN > 0
    if (*(MIPS_INSTRUCTION *)ulRetAddr == BRK_MEMU) {
        if (do_dsemulret(ptcbCur, pregctx) == ERROR_NONE) {
            return;                                                     /*  FPU 模拟返回                */
        }
    }
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */

#if LW_CFG_GDB_EN > 0
    uiBpType = archDbgTrapType(ulRetAddr, LW_NULL);                     /*  断点指令探测                */
    if (uiBpType) {
        if (API_DtraceBreakTrap(ulRetAddr, uiBpType) == ERROR_NONE) {
            return;                                                     /*  进入调试接口断点处理        */
        }
    }
#endif                                                                  /*  LW_CFG_GDB_EN > 0           */

    abtInfo.VMABT_uiMethod = 0;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_BREAK;
    API_VmmAbortIsr(ulRetAddr, ulRetAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archTrapInstHandle
** 功能描述: Trap instruction 处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
**           pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archTrapInstHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr, ARCH_REG_CTX  *pregctx)
{
    /*
     * 当 TGE、TGUE、TLT、TLTU、TEQ、TNE、TGEI、TGEUI、TLTI、TLTUI、TEQI、TNEI 指令执行，
     * 条件结果为真时，触发陷阱例外
     */
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = 0;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
    API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archResvInstHandle
** 功能描述: Reserved instruction 处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archResvInstHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_UNDEF;
    API_VmmAbortIsr(ulRetAddr, ulRetAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archFloatPointExceptHandle
** 功能描述: 浮点异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
**           pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archFloatPointExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr, ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;
#if LW_CFG_CPU_FPU_EN > 0
    UINT32         uiFCSR;
    UINT32         uiFEXR;
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiType = LW_VMM_ABORT_TYPE_FPE;                       /*  终止类型默认为 FPU 异常     */

#if LW_CFG_CPU_FPU_EN > 0
#define FEXR_MASK       ((0x3f << 12) | (0x1f << 2))
#define FENR_MASK       ((0x1f <<  7) | (0x07 << 0))
#define FCCR_MASK       ((0xff <<  0))

    uiFCSR = mipsVfp32GetFCSR();
    uiFEXR = uiFCSR & FEXR_MASK;                                        /*  获得浮点异常类型            */
    if (uiFEXR & (1 << 17)) {                                           /*  未实现异常                  */
        PVOID           pvFaultAddr;
        INT             iSignal;
        ARCH_FPU_CTX   *pfpuctx;

        pfpuctx = &_G_mipsFpuCtx[LW_CPU_GET_CUR_ID()];
        __ARCH_FPU_SAVE(pfpuctx);                                       /*  保存当前 FPU CTX            */

        iSignal = fpu_emulator_cop1Handler(pregctx, pfpuctx,
                                           LW_TRUE, &pvFaultAddr);      /*  FPU 模拟                    */
        switch (iSignal) {

        case 0:                                                         /*  成功模拟                    */
            abtInfo.VMABT_uiType = 0;
            __ARCH_FPU_RESTORE(pfpuctx);                                /*  恢复当前 FPU CTX            */
            break;

        case SIGILL:                                                    /*  未定义指令                  */
            abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;
            abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_UNDEF;
            ulAbortAddr            = ulRetAddr;
            break;

        case SIGBUS:                                                    /*  总线错误                    */
            abtInfo.VMABT_uiMethod = BUS_ADRERR;
            abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_BUS;
            break;

        case SIGSEGV:                                                   /*  地址不合法                  */
            abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_WRITE;
            abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
            ulAbortAddr = (addr_t)pvFaultAddr;
            break;

        case SIGFPE:                                                    /*  FPU 错误                    */
        default:
            abtInfo.VMABT_uiMethod = 0;
            break;
        }

    } else if (uiFEXR & (1 << 16)) {                                    /*  非法操作异常                */
        abtInfo.VMABT_uiMethod = FPE_FLTINV;

    } else if (uiFEXR & (1 << 15)) {                                    /*  除零异常                    */
        abtInfo.VMABT_uiMethod = FPE_FLTDIV;

    } else if (uiFEXR & (1 << 14)) {                                    /*  上溢异常                    */
        abtInfo.VMABT_uiMethod = FPE_FLTOVF;

    } else if (uiFEXR & (1 << 13)) {
        abtInfo.VMABT_uiMethod = FPE_FLTUND;                            /*  下溢异常                    */

    } else if (uiFEXR & (1 << 12)) {                                    /*  不精确异常                  */
        abtInfo.VMABT_uiMethod = FPE_FLTRES;

    } else {
        abtInfo.VMABT_uiMethod = 0;
    }

    mipsVfp32SetFCSR(uiFCSR & (~FEXR_MASK));                            /*  清除浮点异常                */
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */

    if (abtInfo.VMABT_uiType) {
        API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
    }
}
/*********************************************************************************************************
** 函数名称: archCoProc2ExceptHandle
** 功能描述: 协处理器 2 异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
#if defined(_MIPS_ARCH_HR2)

static VOID  archCoProc2ExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = 0;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_DSPE;
    API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
}

#endif                                                                  /*  defined(_MIPS_ARCH_HR2)     */
/*********************************************************************************************************
** 函数名称: archCoProcUnusableExceptHandle
** 功能描述: 协处理器不可用异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
**           pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archCoProcUnusableExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr, ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    switch (((pregctx->REG_ulCP0Cause & CAUSEF_CE) >> CAUSEB_CE)) {

#if LW_CFG_CPU_FPU_EN > 0
    case 1:
        if (archFpuUndHandle(ptcbCur) == ERROR_NONE) {                  /*  进行 FPU 指令探测           */
            return;
        }
        break;
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */

#if defined(_MIPS_ARCH_HR2) && (LW_CFG_CPU_DSP_EN > 0)
    case 2:
        if (archDspUndHandle(ptcbCur) == ERROR_NONE) {                  /*  进行 DSP 指令探测           */
            return;
        }
        break;
#endif                                                                  /*  defined(_MIPS_ARCH_HR2)     */
                                                                        /*  LW_CFG_CPU_DSP_EN > 0       */
    default:
        break;
    }

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_UNDEF;
    API_VmmAbortIsr(ulRetAddr, ulRetAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archDspDisableExceptHandle
** 功能描述: DSP 关闭异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archDspDisableExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

#if LW_CFG_CPU_DSP_EN > 0
    if (archDspUndHandle(ptcbCur) == ERROR_NONE) {                      /*  进行 DSP 指令探测           */
        return;
    }
#endif                                                                  /*  LW_CFG_CPU_DSP_EN > 0       */

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_UNDEF;
    API_VmmAbortIsr(ulRetAddr, ulRetAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archDefaultExceptHandle
** 功能描述: 缺省的异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archDefaultExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = 0;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
    API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archMachineCheckExceptHandle
** 功能描述: 机器检查异常处理
** 输　入  : ulRetAddr     返回地址
**           ulAbortAddr   终止地址
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archMachineCheckExceptHandle (addr_t  ulRetAddr, addr_t  ulAbortAddr)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    /*
     * The machine check exception occurs when executing of TLBWI or TLBWR instruction
     * detects multiple matching entries in the JTLB.
     */

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = 0;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_FATAL_ERROR;
    API_VmmAbortIsr(ulRetAddr, ulAbortAddr, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
  MIPS 异常处理函数数组
*********************************************************************************************************/
static MIPS_EXCEPT_HANDLE   _G_mipsExceptHandle[32] = {
    [EXCCODE_INT]      = (PVOID)bspIntHandle,                           /*  Interrupt pending           */
    [EXCCODE_MOD]      = (PVOID)archTlbModExceptHandle,                 /*  TLB modified fault          */
    [EXCCODE_TLBL]     = (PVOID)archTlbLoadExceptHandle,                /*  TLB miss on load or ifetch  */
    [EXCCODE_TLBS]     = (PVOID)archTlbStoreExceptHandle,               /*  TLB miss on a store         */
    [EXCCODE_ADEL]     = (PVOID)archAddrLoadExceptHandle,               /*  Address error(load or ifetch*/
    [EXCCODE_ADES]     = (PVOID)archAddrStoreExceptHandle,              /*  Address error(store)        */
    [EXCCODE_IBE]      = (PVOID)archInstBusExceptHandle,                /*  Instruction bus error       */
    [EXCCODE_DBE]      = (PVOID)archDataBusExceptHandle,                /*  Data bus error              */
    [EXCCODE_SYS]      = (PVOID)archSysCallHandle,                      /*  System call                 */
    [EXCCODE_BP]       = (PVOID)archBreakPointHandle,                   /*  Breakpoint                  */
    [EXCCODE_RI]       = (PVOID)archResvInstHandle,                     /*  Reserved instruction        */
    [EXCCODE_CPU]      = (PVOID)archCoProcUnusableExceptHandle,         /*  Coprocessor unusable        */
    [EXCCODE_OV]       = (PVOID)archDefaultExceptHandle,                /*  Arithmetic overflow         */
    [EXCCODE_TR]       = (PVOID)archTrapInstHandle,                     /*  Trap instruction            */
    [EXCCODE_MSAFPE]   = (PVOID)archDefaultExceptHandle,                /*  MSA floating point exception*/
    [EXCCODE_FPE]      = (PVOID)archFloatPointExceptHandle,             /*  Floating point exception    */
    [EXCCODE_GSEXC]    = (PVOID)archDefaultExceptHandle,                /*  Loongson cpu exception      */
#if defined(_MIPS_ARCH_HR2)
    [EXCCODE_C2E]      = (PVOID)archCoProc2ExceptHandle,                /*  Coprocessor 2 exception     */
#endif                                                                  /*  defined(_MIPS_ARCH_HR2)     */
    /*
     * 读阻止例外没使能, 执行阻止例外复用 TLBL 例外
     */
    [EXCCODE_TLBRI]    = (PVOID)archDefaultExceptHandle,                /*  TLB Read-Inhibit exception  */
    [EXCCODE_TLBXI]    = (PVOID)archDefaultExceptHandle,                /*  TLB Exec-Inhibit exception  */
    [EXCCODE_MSADIS]   = (PVOID)archDefaultExceptHandle,                /*  MSA disabled exception      */
    [EXCCODE_MDMX]     = (PVOID)archDefaultExceptHandle,                /*  MDMX unusable exception     */
    [EXCCODE_WATCH]    = (PVOID)archDefaultExceptHandle,                /*  Watch address reference     */
    [EXCCODE_MCHECK]   = (PVOID)archMachineCheckExceptHandle,           /*  Machine check exception     */
    [EXCCODE_THREAD]   = (PVOID)archDefaultExceptHandle,                /*  Thread exception (MT)       */
    [EXCCODE_DSPDIS]   = (PVOID)archDspDisableExceptHandle,             /*  DSP disabled exception      */
    [EXCCODE_CACHEERR] = (PVOID)archCacheErrorHandle,                   /*  Cache error exception       */
};
/*********************************************************************************************************
** 函数名称: archExceptionHandle
** 功能描述: 通用异常处理
** 输　入  : NONE
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archExceptionHandle (ARCH_REG_CTX  *pregctx)
{
    ULONG               ulExcCode = ((pregctx->REG_ulCP0Cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE);
    MIPS_EXCEPT_HANDLE  pfuncExceptHandle;

    if (ulExcCode == EXCCODE_INT) {                                     /*  优化普通中断处理            */
        bspIntHandle();
        return;
    }

    pfuncExceptHandle = _G_mipsExceptHandle[ulExcCode];
    if (pfuncExceptHandle == LW_NULL) {
        _BugFormat(LW_TRUE, LW_TRUE, "Unknow exception: %d\r\n", ulExcCode);
    } else {
        pfuncExceptHandle(pregctx->REG_ulCP0EPC, pregctx->REG_ulCP0BadVAddr, pregctx);
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
