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
** 文   件   名: k_cpu.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2009 年 04 月 07 日
**
** 描        述: CPU 信息描述.
**
** BUG:
2013.07.18  去掉 cpu 信息中 优先级的记录, 1.0.0-rc40 以后的 SylixOS 不再使用. 
2015.04.24  核间中断加入专有的清除函数.
*********************************************************************************************************/

#ifndef __K_CPU_H
#define __K_CPU_H

/*********************************************************************************************************
  CPU 工作状态 (目前只支持 ACTIVE 模式)
*********************************************************************************************************/

#define LW_CPU_STATUS_ACTIVE            0x80000000                      /*  CPU 激活操作系统可执行调度  */
#define LW_CPU_STATUS_RUNNING           0x40000000                      /*  CPU 已经开始运转            */

/*********************************************************************************************************
  CPU 结构 (要求 CPU ID 编号从 0 开始并由连续数字组成) 
  TODO: CPU_cand, CPU_slIpi 独立组成数组 CACHE 效率高?
*********************************************************************************************************/
#ifdef  __SYLIXOS_KERNEL

typedef struct __lw_cpu {
    /*
     *  运行线程情况
     */
    PLW_CLASS_TCB            CPU_ptcbTCBCur;                            /*  当前 TCB                    */
    PLW_CLASS_TCB            CPU_ptcbTCBHigh;                           /*  需要运行的高优先 TCB        */
    
#if LW_CFG_COROUTINE_EN > 0
    /*
     *  协程切换信息
     */
    PLW_CLASS_COROUTINE      CPU_pcrcbCur;                              /*  当前协程                    */
    PLW_CLASS_COROUTINE      CPU_pcrcbNext;                             /*  需要切换的目标协程          */
#else
    PVOID                    CPU_pvNull[2];                             /*  保证下面成员地址偏移量一致  */
#endif                                                                  /*  LW_CFG_COROUTINE_EN > 0     */

    /*
     *  当前发生调度的调度方式
     */
    BOOL                     CPU_bIsIntSwitch;                          /*  是否为中断调度              */

    /*
     *  候选运行结构
     */
    LW_CLASS_CAND            CPU_cand;                                  /*  候选运行的线程              */

    /*
     *  内核锁定状态
     */
    INT                      CPU_iKernelCounter;                        /*  内核状态计数器              */

    /*
     *  当前核就绪表
     */
#if LW_CFG_SMP_EN > 0
    LW_CLASS_PCBBMAP         CPU_pcbbmapReady;                          /*  当前 CPU 就绪表             */
    BOOL                     CPU_bOnlyAffinity;                         /*  是否仅运行亲和线程          */
    
#if LW_CFG_CACHE_EN > 0
    volatile BOOL            CPU_bCacheBarrier;                         /*  CACHE 同步点                */
#endif                                                                  /*  LW_CFG_CACHE_EN > 0         */
    
    /*
     *  核间中断待处理标志, 这里最多有 ULONG 位数个核间中断类型, 和 CPU 硬件中断向量原理相同
     */
    LW_SPINLOCK_DEFINE      (CPU_slIpi);                                /*  核间中断锁                  */
    PLW_LIST_RING            CPU_pringMsg;                              /*  自定义核间中断参数链        */
    volatile UINT            CPU_uiMsgCnt;                              /*  自定义核间中断数量          */
    
    ULONG                    CPU_ulIPIVector;                           /*  核间中断向量                */
    FUNCPTR                  CPU_pfuncIPIClear;                         /*  核间中断清除函数            */
    PVOID                    CPU_pvIPIArg;                              /*  核间中断清除参数            */
    
    INT64                    CPU_iIPICnt;                               /*  核间中断次数                */
    atomic_t                 CPU_iIPIPend;                              /*  核间中断标志码              */

#define LW_IPI_NOP              0                                       /*  测试用核间中断向量          */
#define LW_IPI_SCHED            1                                       /*  调度请求                    */
#define LW_IPI_DOWN             2                                       /*  CPU 停止工作                */
#define LW_IPI_PERF             3                                       /*  性能分析                    */
#define LW_IPI_CALL             4                                       /*  自定义调用 (有参数可选等待) */

#define LW_IPI_NOP_MSK          (1 << LW_IPI_NOP)
#define LW_IPI_SCHED_MSK        (1 << LW_IPI_SCHED)
#define LW_IPI_DOWN_MSK         (1 << LW_IPI_DOWN)
#define LW_IPI_PERF_MSK         (1 << LW_IPI_PERF)
#define LW_IPI_CALL_MSK         (1 << LW_IPI_CALL)

#ifdef __LW_SPINLOCK_BUG_TRACE_EN
    ULONG                    CPU_ulSpinNesting;                         /*  spinlock 加锁数量           */
#endif                                                                  /*  __LW_SPINLOCK_BUG_TRACE_EN  */
    volatile UINT            CPU_uiLockQuick;                           /*  是否在 Lock Quick 中        */
    
    /*
     *  CPU 基本信息
     */
#if LW_CFG_CPU_ARCH_SMT > 0
             ULONG           CPU_ulPhyId;                               /*  Physical CPU Id             */
#endif                                                                  /*  LW_CFG_CPU_ARCH_SMT         */
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */

             ULONG           CPU_ulCPUId;                               /*  CPU ID 号                   */
    volatile ULONG           CPU_ulStatus;                              /*  CPU 工作状态                */

    /*
     *  中断信息
     */
    PLW_STACK                CPU_pstkInterBase;                         /*  中断堆栈基址                */
    ULONG                    CPU_ulInterNesting;                        /*  中断嵌套计数器              */
    ULONG                    CPU_ulInterNestingMax;                     /*  中断嵌套最大值              */
    ULONG                    CPU_ulInterError[LW_CFG_MAX_INTER_SRC];    /*  中断错误信息                */
    
#if (LW_CFG_CPU_FPU_EN > 0) && (LW_CFG_INTER_FPU > 0)
    /*
     *  中断时使用的 FPU 上下文. 
     *  只有 LW_KERN_FPU_EN_GET() 有效时才进行中断状态的 FPU 上下文操作.
     */
    LW_FPU_CONTEXT           CPU_fpuctxContext[LW_CFG_MAX_INTER_SRC];   /*  中断时使用的 FPU 上下文     */
#endif                                                                  /*  LW_CFG_CPU_FPU_EN > 0       */
                                                                        /*  LW_CFG_INTER_FPU > 0        */

#if (LW_CFG_CPU_DSP_EN > 0) && (LW_CFG_INTER_DSP > 0)
    /*
     *  中断时使用的 DSP 上下文.
     *  只有 LW_KERN_DSP_EN_GET() 有效时才进行中断状态的 DSP 上下文操作.
     */
    LW_DSP_CONTEXT           CPU_dspctxContext[LW_CFG_MAX_INTER_SRC];   /*  中断时使用的 DSP 上下文     */
#endif                                                                  /*  LW_CFG_CPU_DSP_EN > 0       */
                                                                        /*  LW_CFG_INTER_DSP > 0        */
} LW_CLASS_CPU;
typedef LW_CLASS_CPU        *PLW_CLASS_CPU;

/*********************************************************************************************************
  Physical CPU (要求物理 CPU ID 编号从 0 开始并由连续数字组成)
*********************************************************************************************************/
#if (LW_CFG_SMP_EN > 0) && (LW_CFG_CPU_ARCH_SMT > 0)

typedef struct __lw_phycpu {
    UINT                    PHYCPU_uiLogic;                             /*  当前拥有多少个在线逻辑 CPU  */
    UINT                    PHYCPU_uiNonIdle;                           /*  当前运行的有效任务数量      */
} LW_CLASS_PHYCPU;
typedef LW_CLASS_PHYCPU    *PLW_CLASS_PHYCPU;

#endif                                                                  /*  LW_CFG_CPU_ARCH_SMT > 0     */
#endif                                                                  /*  __SYLIXOS_KERNEL            */
/*********************************************************************************************************
  CPU 集合
*********************************************************************************************************/

#define LW_CPU_SETSIZE      2048
#define LW_NCPUBITS         (sizeof(ULONG) * 8)                         /*  每一个单位掩码的位数        */
#define LW_NCPUULONG        (LW_CPU_SETSIZE / LW_NCPUBITS)

typedef struct {
    ULONG                   cpus_bits[LW_NCPUULONG];
} LW_CLASS_CPUSET;
typedef LW_CLASS_CPUSET    *PLW_CLASS_CPUSET;

#define LW_CPU_SET(n, p)    ((p)->cpus_bits[(n) / LW_NCPUBITS] |= (ULONG)( (1u << ((n) % LW_NCPUBITS))))
#define LW_CPU_CLR(n, p)    ((p)->cpus_bits[(n) / LW_NCPUBITS] &= (ULONG)(~(1u << ((n) % LW_NCPUBITS))))
#define LW_CPU_ISSET(n, p)  ((p)->cpus_bits[(n) / LW_NCPUBITS] &  (ULONG)( (1u << ((n) % LW_NCPUBITS))))
#define LW_CPU_ZERO(p)      lib_bzero((PVOID)(p), sizeof(*(p)))

/*********************************************************************************************************
  当前 CPU 信息 LW_NCPUS 绝不能大于 LW_CFG_MAX_PROCESSORS
*********************************************************************************************************/
#ifdef  __SYLIXOS_KERNEL

#if LW_CFG_SMP_EN > 0
ULONG   archMpCur(VOID);
#define LW_CPU_GET_CUR_ID()  archMpCur()                                /*  获得当前 CPU ID             */
#define LW_NCPUS             (_K_ulNCpus)
#if LW_CFG_CPU_ARCH_SMT > 0
#define LW_NPHYCPUS          (_K_ulNPhyCpus)
#endif                                                                  /*  LW_CFG_CPU_ARCH_SMT > 0     */
#else
#define LW_CPU_GET_CUR_ID()  0
#define LW_NCPUS             1
#endif                                                                  /*  LW_CFG_SMP_EN               */

/*********************************************************************************************************
  CPU 表操作
*********************************************************************************************************/

extern LW_CLASS_CPU          _K_cpuTable[];                             /*  CPU 表                      */
#define LW_CPU_GET_CUR()     (&_K_cpuTable[LW_CPU_GET_CUR_ID()])        /*  获得当前 CPU 结构           */
#define LW_CPU_GET(id)       (&_K_cpuTable[(id)])                       /*  获得指定 CPU 结构           */
#define LW_CPU_GET_ID(pcpu)  (pcpu->CPU_ulCPUId)                        /*  获得 CPU ID                 */

#if (LW_CFG_SMP_EN > 0) && (LW_CFG_CPU_ARCH_SMT > 0)
extern LW_CLASS_PHYCPU       _K_phycpuTable[];                          /*  物理 CPU 表                 */
#define LW_PHYCPU_GET_CUR()  (&_K_phycpuTable[LW_CPU_GET_CUR()->CPU_ulPhyId])
#define LW_PHYCPU_GET(phyid) (&_K_phycpuTable[(phyid)])
#endif                                                                  /*  LW_CFG_CPU_ARCH_SMT > 0     */

/*********************************************************************************************************
  CPU 遍历
*********************************************************************************************************/

#if LW_CFG_SMP_REVERSE_FOREACH > 0
#define LW_CPU_FOREACH_LOOP(i)      for (i = (LW_NCPUS - 1); (i >= 0) && (i < LW_NCPUS); i--)
#define LW_PHYCPU_FOREACH_LOOP(i)   for (i = (LW_NPHYCPUS - 1); (i >= 0) && (i < LW_NPHYCPUS); i--)
#else
#define LW_CPU_FOREACH_LOOP(i)      for (i = 0; i < LW_NCPUS; i++)
#define LW_PHYCPU_FOREACH_LOOP(i)   for (i = 0; i < LW_NPHYCPUS; i++)
#endif

#define LW_CPU_FOREACH(i)                       \
        LW_CPU_FOREACH_LOOP(i)

#define LW_CPU_FOREACH_ACTIVE(i)                \
        LW_CPU_FOREACH_LOOP(i)                  \
        if (LW_CPU_IS_ACTIVE(LW_CPU_GET(i)))

#define LW_CPU_FOREACH_EXCEPT(i, id)            \
        LW_CPU_FOREACH_LOOP(i)                  \
        if ((i) != (id))

#define LW_CPU_FOREACH_ACTIVE_EXCEPT(i, id)     \
        LW_CPU_FOREACH_LOOP(i)                  \
        if ((i) != (id))                        \
        if (LW_CPU_IS_ACTIVE(LW_CPU_GET(i)))

#if (LW_CFG_SMP_EN > 0) && (LW_CFG_CPU_ARCH_SMT > 0)
#define LW_PHYCPU_FOREACH(i)                    \
        LW_PHYCPU_FOREACH_LOOP(i)
#endif                                                                  /*  LW_CFG_CPU_ARCH_SMT > 0     */

/*********************************************************************************************************
  CPU LOCK QUICK 记录
*********************************************************************************************************/

#if LW_CFG_SMP_EN > 0
#define LW_CPU_LOCK_QUICK_INC(pcpu)             ((pcpu)->CPU_uiLockQuick++)
#define LW_CPU_LOCK_QUICK_DEC(pcpu)             ((pcpu)->CPU_uiLockQuick--)
#define LW_CPU_LOCK_QUICK_GET(pcpu)             ((pcpu)->CPU_uiLockQuick)
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */

/*********************************************************************************************************
  CPU 强制运行亲和度线程
*********************************************************************************************************/

#if LW_CFG_SMP_EN > 0
#define LW_CPU_ONLY_AFFINITY_SET(pcpu, val)     ((pcpu)->CPU_bOnlyAffinity = val)
#define LW_CPU_ONLY_AFFINITY_GET(pcpu)          ((pcpu)->CPU_bOnlyAffinity)
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */

/*********************************************************************************************************
  CPU 就绪表
*********************************************************************************************************/

#if LW_CFG_SMP_EN > 0
#define LW_CPU_RDY_PCBBMAP(pcpu)        (&((pcpu)->CPU_pcbbmapReady))
#define LW_CPU_RDY_BMAP(pcpu)           (&((pcpu)->CPU_pcbbmapReady.PCBM_bmap))
#define LW_CPU_RDY_PPCB(pcpu, prio)     (&((pcpu)->CPU_pcbbmapReady.PCBM_pcb[prio]))
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */

/*********************************************************************************************************
  CPU spin nesting
*********************************************************************************************************/

#if LW_CFG_SMP_EN > 0
#ifdef __LW_SPINLOCK_BUG_TRACE_EN
#define LW_CPU_SPIN_NESTING_GET(pcpu)   ((pcpu)->CPU_ulSpinNesting)
#define LW_CPU_SPIN_NESTING_INC(pcpu)   ((pcpu)->CPU_ulSpinNesting++)
#define LW_CPU_SPIN_NESTING_DEC(pcpu)   ((pcpu)->CPU_ulSpinNesting--)
#else
#define LW_CPU_SPIN_NESTING_GET(pcpu)   (0)
#define LW_CPU_SPIN_NESTING_INC(pcpu)
#define LW_CPU_SPIN_NESTING_DEC(pcpu)
#endif                                                                  /*  __LW_SPINLOCK_BUG_TRACE_EN  */
#endif                                                                  /*  LW_CFG_SMP_EN               */

/*********************************************************************************************************
  CPU 状态
*********************************************************************************************************/

#define LW_CPU_IS_ACTIVE(pcpu)  \
        ((pcpu)->CPU_ulStatus & LW_CPU_STATUS_ACTIVE)

#define LW_CPU_IS_RUNNING(pcpu) \
        ((pcpu)->CPU_ulStatus & LW_CPU_STATUS_RUNNING)

/*********************************************************************************************************
  CPU 核间中断 (如果 CPU 支持 atomic 则使用 atomic 指令, CPU 不支持使用 CPU spinlock)
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

#if LW_CFG_CPU_ATOMIC_EN > 0
#define LW_CPU_ADD_IPI_PEND(id, ipi_msk)    \
        __LW_ATOMIC_OR(ipi_msk, &_K_cpuTable[(id)].CPU_iIPIPend)        /*  加入指定 CPU 核间中断 pend  */
#define LW_CPU_CLR_IPI_PEND(id, ipi_msk)    \
        __LW_ATOMIC_AND(~ipi_msk, &_K_cpuTable[(id)].CPU_iIPIPend)      /*  清除指定 CPU 核间中断 pend  */
#define LW_CPU_GET_IPI_PEND(id)             \
        __LW_ATOMIC_GET(&_K_cpuTable[(id)].CPU_iIPIPend)                /*  获取指定 CPU 核间中断 pend  */
        
#define LW_CPU_ADD_IPI_PEND2(pcpu, ipi_msk)    \
        __LW_ATOMIC_OR(ipi_msk, &pcpu->CPU_iIPIPend)                    /*  加入指定 CPU 核间中断 pend  */
#define LW_CPU_CLR_IPI_PEND2(pcpu, ipi_msk)    \
        __LW_ATOMIC_AND(~ipi_msk, &pcpu->CPU_iIPIPend)                  /*  清除指定 CPU 核间中断 pend  */
#define LW_CPU_GET_IPI_PEND2(pcpu)             \
        __LW_ATOMIC_GET(&pcpu->CPU_iIPIPend)                            /*  获取指定 CPU 核间中断 pend  */

#else                                                                   /*  LW_CFG_CPU_ATOMIC_EN        */
#define LW_CPU_ADD_IPI_PEND(id, ipi_msk)    \
        (_K_cpuTable[(id)].CPU_iIPIPend.counter |= (ipi_msk))           /*  加入指定 CPU 核间中断 pend  */
#define LW_CPU_CLR_IPI_PEND(id, ipi_msk)    \
        (_K_cpuTable[(id)].CPU_iIPIPend.counter &= ~(ipi_msk))          /*  清除指定 CPU 核间中断 pend  */
#define LW_CPU_GET_IPI_PEND(id)             \
        (_K_cpuTable[(id)].CPU_iIPIPend.counter)                        /*  获取指定 CPU 核间中断 pend  */
        
#define LW_CPU_ADD_IPI_PEND2(pcpu, ipi_msk)    \
        (pcpu->CPU_iIPIPend.counter |= (ipi_msk))                       /*  加入指定 CPU 核间中断 pend  */
#define LW_CPU_CLR_IPI_PEND2(pcpu, ipi_msk)    \
        (pcpu->CPU_iIPIPend.counter &= ~(ipi_msk))                      /*  清除指定 CPU 核间中断 pend  */
#define LW_CPU_GET_IPI_PEND2(pcpu)             \
        (pcpu->CPU_iIPIPend.counter)                                    /*  获取指定 CPU 核间中断 pend  */
#endif                                                                  /*  !LW_CFG_CPU_ATOMIC_EN       */

/*********************************************************************************************************
  CPU 清除调度请求中断 (关中断情况下被调用)
*********************************************************************************************************/

#if LW_CFG_CPU_ATOMIC_EN > 0
#define LW_CPU_CLR_SCHED_IPI_PEND(pcpu)                             \
        do {                                                        \
            LW_CPU_CLR_IPI_PEND2(pcpu, LW_IPI_SCHED_MSK);           \
            LW_SPINLOCK_NOTIFY();                                   \
        } while (0)

#else                                                                   /*  LW_CFG_CPU_ATOMIC_EN        */
#define LW_CPU_CLR_SCHED_IPI_PEND(pcpu)                             \
        do {                                                        \
            if (LW_CPU_GET_IPI_PEND2(pcpu) & LW_IPI_SCHED_MSK) {    \
                LW_SPIN_LOCK_IGNIRQ(&pcpu->CPU_slIpi);              \
                LW_CPU_CLR_IPI_PEND2(pcpu, LW_IPI_SCHED_MSK);       \
                LW_SPIN_UNLOCK_IGNIRQ(&pcpu->CPU_slIpi);            \
            }                                                       \
            LW_SPINLOCK_NOTIFY();                                   \
        } while (0)
#endif                                                                  /*  !LW_CFG_CPU_ATOMIC_EN       */

/*********************************************************************************************************
  CPU 核间中断数量
*********************************************************************************************************/

#define LW_CPU_GET_IPI_CNT(id)          (_K_cpuTable[(id)].CPU_iIPICnt)

#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
  CPU 中断信息
*********************************************************************************************************/

#define LW_CPU_GET_NESTING(id)          (_K_cpuTable[(id)].CPU_ulInterNesting)
#define LW_CPU_GET_NESTING_MAX(id)      (_K_cpuTable[(id)].CPU_ulInterNestingMax)

ULONG  _CpuGetNesting(VOID);
ULONG  _CpuGetMaxNesting(VOID);

#define LW_CPU_GET_CUR_NESTING()        _CpuGetNesting()
#define LW_CPU_GET_CUR_NESTING_MAX()    _CpuGetMaxNesting()

#endif                                                                  /*  __SYLIXOS_KERNEL            */
#endif                                                                  /*  __K_CPU_H                   */
/*********************************************************************************************************
  END
*********************************************************************************************************/
