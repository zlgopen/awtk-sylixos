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
** 文   件   名: riscvExc.c
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2018 年 03 月 20 日
**
** 描        述: RISC-V 体系构架异常处理.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "dtrace.h"
#if LW_CFG_VMM_EN > 0
#include "arch/riscv/mm/mmu/riscvMmu.h"
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
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
    REGISTER irqreturn_t  irqret;

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
** 函数名称: archMmuAbortType
** 功能描述: 获得访问终止类型
** 输　入  : ulAddr        终止地址
**           uiMethod      访问方法(LW_VMM_ABORT_METHOD_XXX)
** 输　出  : 访问终止类型
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE ULONG  archMmuAbortType (addr_t  ulAddr, UINT  uiMethod)
{
#if LW_CFG_VMM_EN > 0
    return  (riscvMmuAbortType(ulAddr, uiMethod));
#else
    return  (LW_VMM_ABORT_TYPE_PERM);
#endif                                                                  /*  LW_CFG_VMM_EN > 0           */
}
/*********************************************************************************************************
** 函数名称: archDefaultTrapHandle
** 功能描述: 缺省异常处理
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archDefaultTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = 0;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archInstAddrMisalignTrapHandle
** 功能描述: Instruction address misaligned
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archInstAddrMisalignTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = BUS_ADRERR;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_BUS;
    if (abtInfo.VMABT_uiType) {
        API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
    }
}
/*********************************************************************************************************
** 函数名称: archInstAccessTrapHandle
** 功能描述: Instruction access fault
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archInstAccessTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archIllegalInstTrapHandle
** 功能描述: Illegal instruction
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archIllegalInstTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB   ptcbCur;
    LW_VMM_ABORT    abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

#if LW_CFG_CPU_FPU_EN > 0
    if (archFpuUndHandle(ptcbCur) == ERROR_NONE) {                      /*  进行 FPU 指令探测           */
        return;
    }
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */

    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_UNDEF;
    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;

    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulEpc, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archBreakpointTrapHandle
** 功能描述: Breakpoint
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archBreakpointTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

#if LW_CFG_GDB_EN > 0
    addr_t         ulAddr = pregctx->REG_ulEpc;

    UINT    uiBpType = archDbgTrapType(ulAddr, LW_NULL);                /*  断点指令探测                */
    if (uiBpType) {
        if (API_DtraceBreakTrap(ulAddr, uiBpType) == ERROR_NONE) {      /*  进入调试接口断点处理        */
            return;
        }
    }
#endif                                                                  /*  LW_CFG_GDB_EN > 0           */

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_BREAK;
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulEpc, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archLoadAccessTrapHandle
** 功能描述: Load access fault
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archLoadAccessTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_READ;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archLoadAddrMisalignTrapHandle
** 功能描述: Load address misaligned
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
#if LW_CFG_RISCV_M_LEVEL > 0

static VOID  archLoadAddrMisalignTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = BUS_ADRERR;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_BUS;
    if (abtInfo.VMABT_uiType) {
        API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
    }
}

#endif                                                                  /*  LW_CFG_RISCV_M_LEVEL > 0    */
/*********************************************************************************************************
** 函数名称: archStoreAmoAddrMisalignTrapHandle
** 功能描述: Store/AMO address misaligned
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archStoreAmoAddrMisalignTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = BUS_ADRERR;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_BUS;
    if (abtInfo.VMABT_uiType) {
        API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
    }
}
/*********************************************************************************************************
** 函数名称: archStoreAmoAccessTrapHandle
** 功能描述: Store/AMO access fault
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archStoreAmoAccessTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_WRITE;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_TERMINAL;
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archEnvironmentCallHandle
** 功能描述: Environment call
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archEnvironmentCallHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;
    abtInfo.VMABT_uiType   = LW_VMM_ABORT_TYPE_SYS;
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archInstPageTrapHandle
** 功能描述: Instruction page fault
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archInstPageTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_EXEC;
    abtInfo.VMABT_uiType   = archMmuAbortType(pregctx->REG_ulTrapVal, LW_VMM_ABORT_METHOD_EXEC);
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archLoadPageTrapHandle
** 功能描述: Load page fault
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archLoadPageTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_READ;
    abtInfo.VMABT_uiType   = archMmuAbortType(pregctx->REG_ulTrapVal, LW_VMM_ABORT_METHOD_READ);
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
** 函数名称: archStoreAmoPageTrapHandle
** 功能描述: Store/AMO page fault
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  archStoreAmoPageTrapHandle (ARCH_REG_CTX  *pregctx)
{
    PLW_CLASS_TCB  ptcbCur;
    LW_VMM_ABORT   abtInfo;

    LW_TCB_GET_CUR(ptcbCur);

    abtInfo.VMABT_uiMethod = LW_VMM_ABORT_METHOD_WRITE;
    abtInfo.VMABT_uiType   = archMmuAbortType(pregctx->REG_ulTrapVal, LW_VMM_ABORT_METHOD_WRITE);
    API_VmmAbortIsr(pregctx->REG_ulEpc, pregctx->REG_ulTrapVal, &abtInfo, ptcbCur);
}
/*********************************************************************************************************
  Machine cause register (mcause) values after trap
  Interrupt Exception Code Description
  1         0              User software interrupt
  1         1              Supervisor software interrupt
  1         2              Reserved
  1         3              Machine software interrupt
  1         4              User timer interrupt
  1         5              Supervisor timer interrupt
  1         6              Reserved
  1         7              Machine timer interrupt
  1         8              User external interrupt
  1         9              Supervisor external interrupt
  1         10             Reserved
  1         11             Machine external interrupt
  1         >=12           Reserved

  0         0              Instruction address misaligned
  0         1              Instruction access fault
  0         2              Illegal instruction
  0         3              Breakpoint
  0         4              Load address misaligned
  0         5              Load access fault
  0         6              Store/AMO address misaligned
  0         7              Store/AMO access fault
  0         8              Environment call from U-mode
  0         9              Environment call from S-mode
  0         10             Reserved
  0         11             Environment call from M-mode
  0         12             Instruction page fault
  0         13             Load page fault
  0         14             Reserved
  0         15             Store/AMO page fault
  0         >=16           Reserved
*********************************************************************************************************/
/*********************************************************************************************************
  Supervisor cause register (scause) values after trap.
  Interrupt Exception Code Description
  1         0              User software interrupt
  1         1              Supervisor software interrupt
  1         2-3            Reserved
  1         4              User timer interrupt
  1         5              Supervisor timer interrupt
  1         6-7            Reserved
  1         8              User external interrupt
  1         9              Supervisor external interrupt
  1         >=10           Reserved

  0         0              Instruction address misaligned
  0         1              Instruction access fault
  0         2              Illegal instruction
  0         3              Breakpoint
  0         4              Reserved
  0         5              Load access fault
  0         6              AMO address misaligned
  0         7              Store/AMO access fault
  0         8              Environment call
  0         9-11           Reserved
  0         12             Instruction page fault
  0         13             Load page fault
  0         14             Reserved
  0         15             Store/AMO page fault
  0         >=16           Reserved
*********************************************************************************************************/
/*********************************************************************************************************
  RISC-V 异常处理函数表
*********************************************************************************************************/
typedef VOID  (*RISCV_TRAP_HANDLE)(ARCH_REG_CTX  *pregctx);

static RISCV_TRAP_HANDLE   _G_riscvTrapHandle[32] = {
    [0]  = (PVOID)archInstAddrMisalignTrapHandle,                   /*  Instruction address misaligned  */
    [1]  = (PVOID)archInstAccessTrapHandle,                         /*  Instruction access fault        */
    [2]  = (PVOID)archIllegalInstTrapHandle,                        /*  Illegal instruction             */
    [3]  = (PVOID)archBreakpointTrapHandle,                         /*  Breakpoint                      */

#if LW_CFG_RISCV_M_LEVEL > 0
    [4]  = (PVOID)archLoadAddrMisalignTrapHandle,                   /*  Load address misaligned         */
#endif                                                              /*  LW_CFG_RISCV_M_LEVEL > 0        */

    [5]  = (PVOID)archLoadAccessTrapHandle,                         /*  Load access fault               */
    [6]  = (PVOID)archStoreAmoAddrMisalignTrapHandle,               /*  Store/AMO address misaligned    */
    [7]  = (PVOID)archStoreAmoAccessTrapHandle,                     /*  Store/AMO access fault          */
    [8]  = (PVOID)archEnvironmentCallHandle,                        /*  Environment call from U-mode    */

#if LW_CFG_RISCV_M_LEVEL > 0
    [9]  = (PVOID)archEnvironmentCallHandle,                        /*  Environment call from S-mode    */
    [11] = (PVOID)archEnvironmentCallHandle,                        /*  Environment call from M-mode    */
#endif                                                              /*  LW_CFG_RISCV_M_LEVEL > 0        */

    [12] = (PVOID)archInstPageTrapHandle,                           /*  Instruction page fault          */
    [13] = (PVOID)archLoadPageTrapHandle,                           /*  Load page fault                 */
    [15] = (PVOID)archStoreAmoPageTrapHandle,                       /*  Store/AMO page fault            */
};
/*********************************************************************************************************
** 函数名称: archTrapHandle
** 功能描述: 异常处理分派
** 输　入  : pregctx       寄存器上下文
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  archTrapHandle (ARCH_REG_CTX  *pregctx)
{
    if (pregctx->REG_ulCause & (1ULL << (LW_CFG_CPU_WORD_LENGHT - 1))) {
        bspIntHandle();

    } else {
        RISCV_TRAP_HANDLE  pfuncHandle = _G_riscvTrapHandle[pregctx->REG_ulCause];

        if (pfuncHandle) {
            pfuncHandle(pregctx);
        } else {
            archDefaultTrapHandle(pregctx);
        }
    }
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
