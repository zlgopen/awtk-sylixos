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
** 文   件   名: _CandTable.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2011 年 12 月 05 日
**
** 描        述: 这是系统内核候选运行表操作。(将 k_sched.h 中的 inline 函数抽象到此)

** BUG:
2013.07.29  此文件改名为 _CandTable.c 表示候选运行表操作.
2014.01.10  将候选表放入 CPU 结构中.
2015.04.22  加入 _CandTableResel() 提高运算速度.
2017.10.31  当 _CandTableUpdate() 从一个核中移除一个任务时, 这个任务没有绑定 CPU, 则应该在其他 CPU 上尝试
            调度.
2018.06.06  支持强亲和度调度.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** 函数名称: _CandTableSeek
** 功能描述: 调度器查询有就绪线程的最高优先级内联函数.
** 输　入  : pcpu              CPU
**           pucPriority       优先级返回值
** 输　出  : 就绪表.
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static LW_INLINE PLW_CLASS_PCBBMAP  _CandTableSeek (PLW_CLASS_CPU  pcpu, UINT8 *pucPriority)
{
#if LW_CFG_SMP_EN > 0
    REGISTER UINT8  ucGlobal;
    
    if (_BitmapIsEmpty(LW_CPU_RDY_BMAP(pcpu))) {
        if (LW_CPU_ONLY_AFFINITY_GET(pcpu) ||
            _BitmapIsEmpty(LW_GLOBAL_RDY_BMAP())) {                     /*  就绪表为空                  */
            return  (LW_NULL);
        }
        
        *pucPriority = _BitmapHigh(LW_GLOBAL_RDY_BMAP());
        return  (LW_GLOBAL_RDY_PCBBMAP());                              /*  从全局就绪表选择            */
    
    } else {
        *pucPriority = _BitmapHigh(LW_CPU_RDY_BMAP(pcpu));              /*  本地就绪表最高优先级获取    */
        
        if (LW_CPU_ONLY_AFFINITY_GET(pcpu) || 
            _BitmapIsEmpty(LW_GLOBAL_RDY_BMAP())) {
            return  (LW_CPU_RDY_PCBBMAP(pcpu));                         /*  选择本地就绪任务            */
        }
        
        ucGlobal = _BitmapHigh(LW_GLOBAL_RDY_BMAP());
        if (LW_PRIO_IS_HIGH_OR_EQU(*pucPriority, ucGlobal)) {           /*  同优先级, 优先执行 local    */
            return  (LW_CPU_RDY_PCBBMAP(pcpu));
        
        } else {
            *pucPriority = ucGlobal;
            return  (LW_GLOBAL_RDY_PCBBMAP());
        }
    }
    
#else
    if (_BitmapIsEmpty(LW_GLOBAL_RDY_BMAP())) {                         /*  就绪表中无任务              */
        return  (LW_NULL);
    
    } else {
        *pucPriority = _BitmapHigh(LW_GLOBAL_RDY_BMAP());
        return  (LW_GLOBAL_RDY_PCBBMAP());
    }
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
}
/*********************************************************************************************************
** 函数名称: _CandTableNext
** 功能描述: 从就绪表中确定一个最需运行的线程.
** 输　入  : ppcbbmap          就绪表
**           ucPriority        优先级
** 输　出  : 在就绪表中最需要运行的线程.
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static PLW_CLASS_TCB  _CandTableNext (PLW_CLASS_PCBBMAP  ppcbbmap, UINT8  ucPriority)
{
    REGISTER PLW_CLASS_PCB      ppcb;
    REGISTER PLW_CLASS_TCB      ptcb;
    
    ppcb = &ppcbbmap->PCBM_pcb[ucPriority];
    ptcb = _LIST_ENTRY(ppcb->PCB_pringReadyHeader, 
                       LW_CLASS_TCB, 
                       TCB_ringReady);                                  /*  从就绪环中取出一个线程      */
    
    if (ptcb->TCB_ucSchedPolicy == LW_OPTION_SCHED_FIFO) {              /*  如果是 FIFO 直接运行        */
        return  (ptcb);
    
    } else if (ptcb->TCB_usSchedCounter == 0) {                         /*  缺少时间片                  */
        ptcb->TCB_usSchedCounter = ptcb->TCB_usSchedSlice;              /*  补充时间片                  */
        _list_ring_next(&ppcb->PCB_pringReadyHeader);                   /*  下一个                      */
        ptcb = _LIST_ENTRY(ppcb->PCB_pringReadyHeader, 
                           LW_CLASS_TCB, 
                           TCB_ringReady);
    }
    
    return  (ptcb);
}
/*********************************************************************************************************
** 函数名称: _CandTableFill
** 功能描述: 选择一个最该执行线程放入候选表.
** 输　入  : pcpu          CPU 结构
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  _CandTableFill (PLW_CLASS_CPU   pcpu)
{
    REGISTER PLW_CLASS_TCB      ptcb;
    REGISTER PLW_CLASS_PCB      ppcb;
    REGISTER PLW_CLASS_PCBBMAP  ppcbbmap;
             UINT8              ucPriority;

    ppcbbmap = _CandTableSeek(pcpu, &ucPriority);
    _BugHandle((ppcbbmap == LW_NULL), LW_TRUE, "serious error!\r\n");   /*  至少存在一个任务            */
    
    ptcb = _CandTableNext(ppcbbmap, ucPriority);                        /*  确定可以候选运行的线程      */
    ppcb = &ppcbbmap->PCBM_pcb[ucPriority];
    
    LW_CAND_TCB(pcpu) = ptcb;                                           /*  保存新的可执行线程          */
    ptcb->TCB_ulCPUId = LW_CPU_GET_ID(pcpu);                            /*  记录 CPU 号                 */
    ptcb->TCB_bIsCand = LW_TRUE;                                        /*  进入候选运行表              */
    _DelTCBFromReadyRing(ptcb, ppcb);                                   /*  从就绪环中退出              */
    
    if (_PcbIsEmpty(ppcb)) {
        __DEL_RDY_MAP(ptcb);                                            /*  从就绪表中删除              */
    }
}
/*********************************************************************************************************
** 函数名称: _CandTableResel
** 功能描述: 选择一个最该执行线程放入候选表.
** 输　入  : pcpu          CPU 结构
**           ppcbbmap      优先级位图
**           ucPriority    需要运行任务的优先级
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  _CandTableResel (PLW_CLASS_CPU   pcpu, PLW_CLASS_PCBBMAP  ppcbbmap, UINT8  ucPriority)
{
    REGISTER PLW_CLASS_TCB      ptcb;
    REGISTER PLW_CLASS_PCB      ppcb;
    
    ptcb = _CandTableNext(ppcbbmap, ucPriority);                        /*  确定可以候选运行的线程      */
    ppcb = &ppcbbmap->PCBM_pcb[ucPriority];
    
    LW_CAND_TCB(pcpu) = ptcb;                                           /*  保存新的可执行线程          */
    ptcb->TCB_ulCPUId = LW_CPU_GET_ID(pcpu);                            /*  记录 CPU 号                 */
    ptcb->TCB_bIsCand = LW_TRUE;                                        /*  进入候选运行表              */
    _DelTCBFromReadyRing(ptcb, ppcb);                                   /*  从就绪环中退出              */
    
    if (_PcbIsEmpty(ppcb)) {
        __DEL_RDY_MAP(ptcb);                                            /*  从就绪表中删除              */
    }
}
/*********************************************************************************************************
** 函数名称: _CandTableEmpty
** 功能描述: 将候选表中的候选线程放入就绪表.
** 输　入  : pcpu      CPU 结构
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  _CandTableEmpty (PLW_CLASS_CPU   pcpu)
{
    REGISTER PLW_CLASS_TCB  ptcb;
    REGISTER PLW_CLASS_PCB  ppcb;
    
    ptcb = LW_CAND_TCB(pcpu);
    ppcb = _GetPcb(ptcb);
    
    ptcb->TCB_bIsCand = LW_FALSE;
    _AddTCBToReadyRing(ptcb, ppcb, LW_TRUE);                            /*  重新加入就绪环              */
                                                                        /*  插入就绪环头, 下次优先调度  */
    if (_PcbIsOne(ppcb)) {
        __ADD_RDY_MAP(ptcb);                                            /*  当前线程被抢占, 重回就绪表  */
    }
    
    LW_CAND_TCB(pcpu) = LW_NULL;
}
/*********************************************************************************************************
** 函数名称: _CandTableTryAdd
** 功能描述: 试图将一个就绪线程装入候选线程表.
** 输　入  : ptcb          就绪的线程
**           ppcb          对应的优先级控制块
** 输　出  : 是否加入了候选运行表
** 全局变量: 
** 调用模块: 
** 说  明  : 单核系统可不需要使用软绕标志, 立即更换候选表.
*********************************************************************************************************/
BOOL  _CandTableTryAdd (PLW_CLASS_TCB  ptcb, PLW_CLASS_PCB  ppcb)
{
    REGISTER PLW_CLASS_CPU   pcpu;
    REGISTER PLW_CLASS_TCB   ptcbCand;
    
#if LW_CFG_SMP_EN > 0                                                   /*  SMP 多核                    */
    REGISTER ULONG           i;
    REGISTER BOOL            bRotIdle;

    if (ptcb->TCB_bCPULock) {                                           /*  任务锁定 CPU                */
        pcpu = LW_CPU_GET(ptcb->TCB_ulCPULock);
        if (!LW_CPU_IS_ACTIVE(pcpu)) {
            goto    __can_not_cand;
        }
        
        ptcbCand = LW_CAND_TCB(pcpu);
        if (LW_UNLIKELY(ptcbCand == LW_NULL)) {                         /*  候选表为空                  */
            LW_CAND_TCB(pcpu) = ptcb;
            ptcb->TCB_ulCPUId = ptcb->TCB_ulCPULock;                    /*  记录 CPU 号                 */
            ptcb->TCB_bIsCand = LW_TRUE;                                /*  进入候选运行表              */
            return  (LW_TRUE);
            
        } else if (LW_PRIO_IS_HIGH(ptcb->TCB_ucPriority, 
                                   ptcbCand->TCB_ucPriority)) {         /*  优先级高于当前候选线程      */
            LW_CAND_ROT(pcpu) = LW_TRUE;                                /*  产生优先级卷绕              */
        }
    
    } else {                                                            /*  所有 CPU 均可运行此任务     */
        bRotIdle = LW_FALSE;
        pcpu     = LW_CPU_GET(ptcb->TCB_ulCPUId);                       /*  保持 CACHE 热度             */

#if LW_CFG_SMP_CPU_DOWN_EN > 0
        if (LW_CPU_IS_ACTIVE(pcpu)) 
#endif                                                                  /*  LW_CFG_SMP_CPU_DOWN_EN      */
        {
            if (!LW_CPU_ONLY_AFFINITY_GET(pcpu)) {                      /*  没有设置强亲和度调度        */
                ptcbCand = LW_CAND_TCB(pcpu);
                if (LW_UNLIKELY(ptcbCand == LW_NULL)) {
                    LW_CAND_TCB(pcpu) = ptcb;
                    ptcb->TCB_bIsCand = LW_TRUE;                        /*  进入候选运行表              */
                    return  (LW_TRUE);
                }
            
                if (!LW_CAND_ROT(pcpu) && 
                    (LW_PRIO_IS_EQU(LW_PRIO_LOWEST, 
                                    ptcbCand->TCB_ucPriority))) {       /*  之前运行的 CPU 为 idle      */
                    LW_CAND_ROT(pcpu) = LW_TRUE;                        /*  产生优先级卷绕              */
                    bRotIdle          = LW_TRUE;
                }
            }
        }
        
        if (!bRotIdle) {
            LW_CPU_FOREACH_ACTIVE_EXCEPT (i, ptcb->TCB_ulCPUId) {       /*  CPU 必须为激活状态          */
                pcpu = LW_CPU_GET(i);
                if (LW_CPU_ONLY_AFFINITY_GET(pcpu)) {                   /*  设置了强亲和度调度          */
                    continue;
                }
                
                ptcbCand = LW_CAND_TCB(pcpu);
                if (LW_UNLIKELY(ptcbCand == LW_NULL)) {                 /*  候选表为空                  */
                    LW_CAND_TCB(pcpu) = ptcb;
                    ptcb->TCB_ulCPUId = i;                              /*  记录 CPU 号                 */
                    ptcb->TCB_bIsCand = LW_TRUE;                        /*  进入候选运行表              */
                    return  (LW_TRUE);
                
                } else if (!LW_CAND_ROT(pcpu) && 
                           (LW_PRIO_IS_EQU(LW_PRIO_LOWEST, 
                                           ptcbCand->TCB_ucPriority))) {/*  运行 idle 任务且无标注      */
                    LW_CAND_ROT(pcpu) = LW_TRUE;
                    bRotIdle          = LW_TRUE;
                    break;                                              /*  只标注一个 CPU 即可         */
                }
            }
            
            if (!bRotIdle) {
                LW_CPU_FOREACH_ACTIVE (i) {                             /*  CPU 必须为激活状态          */
                    pcpu = LW_CPU_GET(i);
                    if (LW_CPU_ONLY_AFFINITY_GET(pcpu)) {               /*  设置了强亲和度调度          */
                        continue;
                    }
                    
                    ptcbCand = LW_CAND_TCB(pcpu);
                    if (LW_UNLIKELY(ptcbCand == LW_NULL)) {             /*  候选表为空                  */
                        LW_CAND_TCB(pcpu) = ptcb;
                        ptcb->TCB_ulCPUId = i;                          /*  记录 CPU 号                 */
                        ptcb->TCB_bIsCand = LW_TRUE;                    /*  进入候选运行表              */
                        return  (LW_TRUE);
                    
                    } else if (LW_PRIO_IS_HIGH(ptcb->TCB_ucPriority,
                                               ptcbCand->TCB_ucPriority)) {
                        LW_CAND_ROT(pcpu) = LW_TRUE;                    /*  产生优先级卷绕              */
                    }
                }
            }
        }
    }
    
#else                                                                   /*  单核情况                    */
    pcpu = LW_CPU_GET(0);
    if (!LW_CPU_IS_ACTIVE(pcpu)) {                                      /*  CPU 必须为激活状态          */
        goto    __can_not_cand;
    }
    
    ptcbCand = LW_CAND_TCB(pcpu);
    if (LW_UNLIKELY(ptcbCand == LW_NULL)) {                             /*  候选表为空                  */
__can_cand:
        LW_CAND_TCB(pcpu) = ptcb;
        ptcb->TCB_ulCPUId = 0;                                          /*  记录 CPU 号                 */
        ptcb->TCB_bIsCand = LW_TRUE;                                    /*  进入候选运行表              */
        return  (LW_TRUE);                                              /*  直接加入候选运行表          */
        
    } else if (LW_PRIO_IS_HIGH(ptcb->TCB_ucPriority, 
                               ptcbCand->TCB_ucPriority)) {             /*  优先级比当前候选线程高      */
        if (__THREAD_LOCK_GET(ptcbCand)) {
            LW_CAND_ROT(pcpu) = LW_TRUE;                                /*  产生优先级卷绕              */
        
        } else {
            _CandTableEmpty(pcpu);                                      /*  清空候选表                  */
            goto    __can_cand;
        }
    }
#endif                                                                  /*  LW_CFG_SMP_EN               */

__can_not_cand:
    if (_PcbIsEmpty(ppcb)) {
        __ADD_RDY_MAP(ptcb);                                            /*  将位图的相关位置一          */
    }
    
    return  (LW_FALSE);                                                 /*  无法加入候选运行表          */
}
/*********************************************************************************************************
** 函数名称: _CandTableTryDel
** 功能描述: 试图从将一个不再就绪的候选线程从候选表中退出
** 输　入  : ptcb          就绪的线程
**           ppcb          对应的优先级控制块
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID _CandTableTryDel (PLW_CLASS_TCB  ptcb, PLW_CLASS_PCB  ppcb)
{
    REGISTER PLW_CLASS_CPU   pcpu = LW_CPU_GET(ptcb->TCB_ulCPUId);
    
    if (LW_CAND_TCB(pcpu) == ptcb) {                                    /*  在候选表中                  */
        ptcb->TCB_bIsCand = LW_FALSE;                                   /*  退出候选运行表              */
        _CandTableFill(pcpu);                                           /*  重新填充一个候选线程        */
        LW_CAND_ROT(pcpu) = LW_FALSE;                                   /*  清除优先级卷绕标志          */
    
    } else {                                                            /*  没有在候选表中              */
        if (_PcbIsEmpty(ppcb)) {
            __DEL_RDY_MAP(ptcb);                                        /*  将位图的相关位清零          */
        }
    }
}
/*********************************************************************************************************
** 函数名称: _CandTableNotify
** 功能描述: 通知其他 CPU 进行调度查看. (这里仅需要设置卷绕标志, 不需要发送核间中断, 在任务切换间隙会发送)
** 输　入  : pcpu      当前 CPU 结构
**           ptcbCand  当前 CPU 候选线程
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0

static VOID _CandTableNotify (PLW_CLASS_CPU   pcpu, PLW_CLASS_TCB  ptcbCand)
{
    INT             i;
    ULONG           ulCPUId;
    PLW_CLASS_CPU   pcpuOther;
    PLW_CLASS_TCB   ptcbOther;
    BOOL            bRotIdle;
    
    bRotIdle = LW_FALSE;
    ulCPUId  = LW_CPU_GET_ID(pcpu);
    
    LW_CPU_FOREACH_ACTIVE_EXCEPT (i, ulCPUId) {                         /*  CPU 必须是激活状态          */
        pcpuOther = LW_CPU_GET(i);
        ptcbOther = LW_CAND_TCB(pcpuOther);
        
        if (LW_CPU_ONLY_AFFINITY_GET(pcpuOther)) {                      /*  仅运行亲和度任务            */
            continue;
        }
        
        if (!LW_CAND_ROT(pcpuOther) && 
            (LW_PRIO_IS_EQU(LW_PRIO_LOWEST, 
                            ptcbOther->TCB_ucPriority))) {              /*  运行 idle 任务且无标注      */
            LW_CAND_ROT(pcpuOther) = LW_TRUE;
            bRotIdle               = LW_TRUE;
            break;                                                      /*  只标注一个 CPU 即可         */
        }
    }
    
    if (!bRotIdle) {
        LW_CPU_FOREACH_ACTIVE_EXCEPT (i, ulCPUId) {                     /*  CPU 必须是激活状态          */
            pcpuOther = LW_CPU_GET(i);
            ptcbOther = LW_CAND_TCB(pcpuOther);
            
            if (LW_CPU_ONLY_AFFINITY_GET(pcpuOther)) {                  /*  仅运行亲和度任务            */
                continue;
            }
            
            if (LW_PRIO_IS_HIGH(ptcbCand->TCB_ucPriority,
                                ptcbOther->TCB_ucPriority)) {
                LW_CAND_ROT(pcpuOther) = LW_TRUE;
            }
        }
    }
}

#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
/*********************************************************************************************************
** 函数名称: _CandTableUpdate
** 功能描述: 尝试将最高优先级就绪任务装入候选表. 
** 输　入  : pcpu      CPU 结构
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
** 注  意  : 如果新的候选任务为锁定当前 CPU 则其他核没有设置过卷绕标志, 如果同时被换出的任务可以运行在其他
             CPU 上, 则需要设置其他 CPU 的卷绕标志.
*********************************************************************************************************/
VOID _CandTableUpdate (PLW_CLASS_CPU   pcpu)
{
             UINT8              ucPriority;
    REGISTER PLW_CLASS_TCB      ptcbCand;
             PLW_CLASS_PCBBMAP  ppcbbmap;
             BOOL               bNeedRotate = LW_FALSE;
             
#if LW_CFG_SMP_EN > 0                                                   /*  SMP 多核                    */
             PLW_CLASS_TCB      ptcbNew;
#endif

    if (!LW_CPU_IS_ACTIVE(pcpu)) {                                      /*  CPU 必须为激活状态          */
        return;
    }
    
    ptcbCand = LW_CAND_TCB(pcpu);
    if (ptcbCand == LW_NULL) {                                          /*  当前没有候选线程            */
        _CandTableFill(pcpu);
        goto    __update_done;
    }
    
    ppcbbmap = _CandTableSeek(pcpu, &ucPriority);                       /*  当前就绪表中最高优先级      */
    if (ppcbbmap == LW_NULL) {
        LW_CAND_ROT(pcpu) = LW_FALSE;                                   /*  清除优先级卷绕标志          */
        return;
    }
    
#if LW_CFG_SMP_EN > 0
    if (LW_CPU_ONLY_AFFINITY_GET(pcpu) && !ptcbCand->TCB_bCPULock) {    /*  强制运行亲和度任务          */
        bNeedRotate = LW_TRUE;                                          /*  当前普通任务需要让出 CPU    */
    
    } else 
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
    {
        if (ptcbCand->TCB_usSchedCounter == 0) {                        /*  已经没有时间片了            */
            if (LW_PRIO_IS_HIGH_OR_EQU(ucPriority, 
                                       ptcbCand->TCB_ucPriority)) {     /*  是否需要轮转                */
                bNeedRotate = LW_TRUE;
            }
        
        } else {
            if (LW_PRIO_IS_HIGH(ucPriority, 
                                ptcbCand->TCB_ucPriority)) {
                bNeedRotate = LW_TRUE;
            }
        }
    }
    
    if (bNeedRotate) {                                                  /*  存在更需要运行的线程        */
        _CandTableEmpty(pcpu);                                          /*  清空候选表                  */
        _CandTableResel(pcpu, ppcbbmap, ucPriority);                    /*  重新选择任务执行            */
        
#if LW_CFG_SMP_EN > 0                                                   /*  SMP 多核                    */
        ptcbNew = LW_CAND_TCB(pcpu);
        if ((ptcbNew != ptcbCand) && !ptcbCand->TCB_bCPULock) {         /*  是否需要尝试标记其他 CPU    */
            _CandTableNotify(pcpu, ptcbCand);                           /*  通知其他 CPU 进行调度查看   */
        }
#endif                                                                  /*  LW_CFG_SMP_EN > 0           */
    }
    
__update_done:
    LW_CAND_ROT(pcpu) = LW_FALSE;                                       /*  清除优先级卷绕标志          */
}
/*********************************************************************************************************
** 函数名称: _CandTableRemove
** 功能描述: 将一个 CPU 对应的候选表清除 
** 输　入  : pcpu      CPU 结构
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
#if LW_CFG_SMP_CPU_DOWN_EN > 0

VOID _CandTableRemove (PLW_CLASS_CPU   pcpu)
{
    if (LW_CPU_IS_ACTIVE(pcpu)) {                                       /*  CPU 必须为非激活状态        */
        return;
    }
    
    if (LW_CAND_TCB(pcpu)) {
        _CandTableEmpty(pcpu);
    }
}

#endif                                                                  /*  LW_CFG_SMP_CPU_DOWN_EN > 0  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
