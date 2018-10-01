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
** 文   件   名: _MsgQueueLib.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2007 年 01 月 07 日
**
** 描        述: 消息队列内部操作库。

** BUG:
2009.06.19  修改注释.
2018.07.16  使用双优先级队列模式.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
** 函数名称: _MsgQueueClear
** 功能描述: 消息队列清空
** 输　入  : pmsgqueue     消息队列控制块
**           ulMsgTotal    总消息个数
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
#if (LW_CFG_MSGQUEUE_EN > 0) && (LW_CFG_MAX_MSGQUEUES > 0)

VOID  _MsgQueueClear (PLW_CLASS_MSGQUEUE    pmsgqueue,
                      ULONG                 ulMsgTotal)
{
    ULONG               i;
    size_t              stMaxByteAlign;
    PLW_CLASS_MSGNODE   pmsgnode;

    pmsgqueue->MSGQUEUE_pmonoFree = LW_NULL;
    pmsgqueue->MSGQUEUE_uiMap     = 0;
    
    for (i = 0; i < EVENT_MSG_Q_PRIO; i++) {
        pmsgqueue->MSGQUEUE_pmonoHeader[i] = LW_NULL;
        pmsgqueue->MSGQUEUE_pmonoTail[i]   = LW_NULL;
    }
    
    stMaxByteAlign = ROUND_UP(pmsgqueue->MSGQUEUE_stMaxBytes, sizeof(size_t));
    pmsgnode       = (PLW_CLASS_MSGNODE)pmsgqueue->MSGQUEUE_pvBuffer;
    
    for (i = 0; i < ulMsgTotal; i++) {
        _list_mono_free(&pmsgqueue->MSGQUEUE_pmonoFree, &pmsgnode->MSGNODE_monoManage);
        pmsgnode = (PLW_CLASS_MSGNODE)((UINT8 *)pmsgnode + sizeof(LW_CLASS_MSGNODE) + stMaxByteAlign);
    }
}
/*********************************************************************************************************
** 函数名称: _MsgQueueGet
** 功能描述: 从消息队列中获得一个消息
** 输　入  : pmsgqueue     消息队列控制块
**         : pvMsgBuffer   接收缓冲区
**         : stMaxByteSize 缓冲区大小
**         : pstMsgLen     获得消息的长度
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _MsgQueueGet (PLW_CLASS_MSGQUEUE    pmsgqueue,
                    PVOID                 pvMsgBuffer,
                    size_t                stMaxByteSize,
                    size_t               *pstMsgLen)
{
    INT                 iQ;
    PLW_CLASS_MSGNODE   pmsgnode;
    
    iQ = archFindLsb(pmsgqueue->MSGQUEUE_uiMap) - 1;                    /*  计算优先级                  */
    
    _BugHandle(!pmsgqueue->MSGQUEUE_pmonoHeader[iQ], LW_TRUE, "buffer is empty!\r\n");
    
    pmsgnode = (PLW_CLASS_MSGNODE)_list_mono_allocate_seq(&pmsgqueue->MSGQUEUE_pmonoHeader[iQ],
                                                          &pmsgqueue->MSGQUEUE_pmonoTail[iQ]);
    
    if (pmsgqueue->MSGQUEUE_pmonoHeader[iQ] == LW_NULL) {
        pmsgqueue->MSGQUEUE_uiMap &= ~(1 << iQ);                        /*  清除优先级位图              */
    }
    
    *pstMsgLen = (stMaxByteSize < pmsgnode->MSGNODE_stMsgLen) ? 
                 (stMaxByteSize) : (pmsgnode->MSGNODE_stMsgLen);        /*  确定拷贝信息数量            */
                 
    lib_memcpy(pvMsgBuffer, (PVOID)(pmsgnode + 1), *pstMsgLen);         /*  拷贝消息                    */
    
    _list_mono_free(&pmsgqueue->MSGQUEUE_pmonoFree, &pmsgnode->MSGNODE_monoManage);
}
/*********************************************************************************************************
** 函数名称: _MsgQueuePut
** 功能描述: 向消息队列中写入一个消息
** 输　入  : pmsgqueue     消息队列控制块 
**         : pvMsgBuffer   消息缓冲区
**         : stMsgLen      消息长度
**         : uiPrio        消息优先级
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _MsgQueuePut (PLW_CLASS_MSGQUEUE    pmsgqueue,
                    PVOID                 pvMsgBuffer,
                    size_t                stMsgLen, 
                    UINT                  uiPrio)
{
    PLW_CLASS_MSGNODE  pmsgnode;
    
    pmsgnode = (PLW_CLASS_MSGNODE)_list_mono_allocate(&pmsgqueue->MSGQUEUE_pmonoFree);
    
    pmsgnode->MSGNODE_stMsgLen = stMsgLen;
    
    lib_memcpy((PVOID)(pmsgnode + 1), pvMsgBuffer, stMsgLen);           /*  拷贝消息                    */
    
    if (pmsgqueue->MSGQUEUE_pmonoHeader[uiPrio] == LW_NULL) {
        pmsgqueue->MSGQUEUE_uiMap |= (1 << uiPrio);                     /*  设置优先级位图              */
    }
    
    _list_mono_free_seq(&pmsgqueue->MSGQUEUE_pmonoHeader[uiPrio], 
                        &pmsgqueue->MSGQUEUE_pmonoTail[uiPrio], 
                        &pmsgnode->MSGNODE_monoManage);
}
/*********************************************************************************************************
** 函数名称: _MsgQueueMsgLen
** 功能描述: 从消息队列中获得一个消息的长度
** 输　入  : pmsgqueue     消息队列控制块
**           pstMsgLen     消息长度
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  _MsgQueueMsgLen (PLW_CLASS_MSGQUEUE  pmsgqueue, size_t  *pstMsgLen)
{
    INT                 iQ;
    PLW_CLASS_MSGNODE   pmsgnode;
    
    iQ = archFindLsb(pmsgqueue->MSGQUEUE_uiMap) - 1;                    /*  计算优先级                  */
    
    _BugHandle(!pmsgqueue->MSGQUEUE_pmonoHeader[iQ], LW_TRUE, "buffer is empty!\r\n");

    pmsgnode = (PLW_CLASS_MSGNODE)pmsgqueue->MSGQUEUE_pmonoHeader[iQ];
    
    *pstMsgLen = pmsgnode->MSGNODE_stMsgLen;
}

#endif                                                                  /*  (LW_CFG_MSGQUEUE_EN > 0)    */
                                                                        /*  (LW_CFG_MAX_MSGQUEUES > 0)  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
