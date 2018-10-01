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
** 文   件   名: selectType.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2007 年 11 月 07 日
**
** 描        述:  IO 系统 select 子系统数据结构定义.

** BUG
2008.03.16  LW_HANDLE 改为 LW_OBJECT_HANDLE.
*********************************************************************************************************/

#ifndef __SELECTTYPE_H
#define __SELECTTYPE_H

/*********************************************************************************************************
  等待节点类型
*********************************************************************************************************/

typedef enum {
    SELREAD,                                                            /*  读阻塞                      */
    SELWRITE,                                                           /*  写阻塞                      */
    SELEXCEPT                                                           /*  异常阻塞                    */
} LW_SEL_TYPE;                                                          /*  等待类型                    */

#define LW_SEL_TYPE_FLAG_READ   0x01
#define LW_SEL_TYPE_FLAG_WRITE  0x02
#define LW_SEL_TYPE_FLAG_EXCEPT 0x04

/*********************************************************************************************************
  等待链表节点.
*********************************************************************************************************/

typedef struct {
    LW_LIST_LINE            SELWUN_lineManage;                          /*  管理链表                    */
    UINT32                  SELWUN_uiFlags;
#define LW_SELWUN_FLAG_DFREE    0x1                                     /*  此节点不需要释放            */

#define LW_SELWUN_CLEAR_DFREE(node) \
        ((node)->SELWUN_uiFlags &= ~LW_SELWUN_FLAG_DFREE)
#define LW_SELWUN_SET_DFREE(node)   \
        ((node)->SELWUN_uiFlags |= LW_SELWUN_FLAG_DFREE)
#define LW_SELWUN_IS_DFREE(node)    \
        ((node)->SELWUN_uiFlags & LW_SELWUN_FLAG_DFREE)

#define LW_SELWUN_FLAG_READY   0x2                                      /*  此节点数据准备好            */

#define LW_SELWUN_CLEAR_READY(node) \
        ((node)->SELWUN_uiFlags &= ~LW_SELWUN_FLAG_READY)
#define LW_SELWUN_SET_READY(node)   \
        ((node)->SELWUN_uiFlags |= LW_SELWUN_FLAG_READY)
#define LW_SELWUN_IS_READY(node)    \
        ((node)->SELWUN_uiFlags & LW_SELWUN_FLAG_READY)

    LW_OBJECT_HANDLE        SELWUN_hThreadId;                           /*  创建节点的线程句柄          */
    INT                     SELWUN_iFd;                                 /*  链接点的文件描述符          */
    LW_SEL_TYPE             SELWUN_seltypType;                          /*  等待类型                    */
} LW_SEL_WAKEUPNODE;
typedef LW_SEL_WAKEUPNODE  *PLW_SEL_WAKEUPNODE;

/*********************************************************************************************************
  等待链表头控制结构, 支持 select() 能力的所有文件必须拥有以下结构, 包括 socket 文件.
*********************************************************************************************************/

typedef struct {
    LW_OBJECT_HANDLE        SELWUL_hListLock;                           /*  操作锁                      */
    LW_SEL_WAKEUPNODE       SELWUL_selwunFrist;                         /*  通常情况下,只产生一个节点   */
                                                                        /*  使用这个变量可以避免过多的  */
                                                                        /*  动态内存管理                */
    PLW_LIST_LINE           SELWUL_plineHeader;                         /*  等待链表头                  */
    UINT                    SELWUL_ulWakeCounter;                       /*  等待的数量                  */
} LW_SEL_WAKEUPLIST;
typedef LW_SEL_WAKEUPLIST  *PLW_SEL_WAKEUPLIST;

/*********************************************************************************************************
  每个线程都拥有的私有结构
*********************************************************************************************************/

typedef struct {
    LW_OBJECT_HANDLE        SELCTX_hSembWakeup;                         /*  唤醒信号量                  */
    BOOL                    SELCTX_bPendedOnSelect;                     /*  是否阻塞在 select() 上      */
    BOOL                    SELCTX_bBadFd;                              /*  文件描述符错误              */
    
    fd_set                  SELCTX_fdsetOrigReadFds;                    /*  原始的读文件集              */
    fd_set                  SELCTX_fdsetOrigWriteFds;                   /*  原始的写文件集              */
    fd_set                  SELCTX_fdsetOrigExceptFds;                  /*  原始的异常文件集            */
    
    INT                     SELCTX_iWidth;                              /*  select() 第一个参数         */
} LW_SEL_CONTEXT;
typedef LW_SEL_CONTEXT     *PLW_SEL_CONTEXT;

/*********************************************************************************************************
  内部函数
*********************************************************************************************************/

INT   __selDoIoctls(fd_set  *pfdset, fd_set  *pfdsetUpdate,
                    INT  iFdSetWidth, INT  iFunc,
                    PLW_SEL_WAKEUPNODE pselwun, BOOL  bStopOnErr);

#endif                                                                  /*  __SELECTLIB_H               */
/*********************************************************************************************************
  END
*********************************************************************************************************/
