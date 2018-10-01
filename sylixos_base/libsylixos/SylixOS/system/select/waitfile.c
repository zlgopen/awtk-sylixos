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
** 文   件   名: waitfile.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2007 年 11 月 07 日
**
** 描        述:  IO 系统 select 子系统变种 API. 提供对单个文件的快速操作.

** BUG
2007.11.20  将 pselctx->SELCTX_bPendedOnSelect = LW_TRUE; 提前, 以使 FIOSELECT 进行到中途线程被删除时
            可以使 delete hook 发挥作用, 将产生的 WAKE NODE 删除.
2007.11.20  加入对 context 中 Orig??? 文件集的操作, 以保证 delete hook 可以发挥作用.
2007.12.11  产生错误时,返回值应该为 -1.
2007.12.22  整理注释, 修改 _DebugHandle() 错误的字符串.
2008.03.01  wait???() 函数组对文件描述符的判断有漏洞.
2009.07.17  保存最大文件号操作应该提前, 保证 delete hook 能够正常的删除产生的节点.
2011.03.11  确保超时可以返回对应的 errno 编号.
2011.08.15  重大决定: 将所有 posix 定义的函数以函数方式(非宏)引出.
2017.08.18  等待时间更加精确.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  裁减控制
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_SELECT_EN > 0)
#include "select.h"
/*********************************************************************************************************
** 函数名称: waitread
** 功能描述: select() 变种,等待单个文件可读.
** 输　入  : iFd               文件描述符
**           ptmvalTO          等待超时时间, LW_NULL 表示永远等待.
** 输　出  : 正常等待到的文件数, 为 1 表示文件可以读, 为 0 表示超时 ,错误返回 PX_ERROR.
**           errno == ERROR_IO_SELECT_UNSUPPORT_IN_DRIVER       驱动程序不支持
**           errno == ERROR_IO_SELECT_CONTEXT                   线程不存在 context
**           errno == ERROR_THREAD_WAIT_TIMEOUT                 等待超时
**           errno == ERROR_KERNEL_IN_ISR                       在中断中调用
**           errno == ERROR_IOS_INVALID_FILE_DESCRIPTOR         文件描述符无效.
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
INT     waitread (INT  iFd, struct timeval   *ptmvalTO)
{
    REGISTER INT                 iWidth = iFd + 1;                      /*  iFd + 1                     */
    REGISTER INT                 iWidthInBytes;                         /*  需要检测的位数占多少字节    */
    REGISTER ULONG               ulWaitTime;                            /*  等待时间                    */
    REGISTER LW_SEL_CONTEXT     *pselctx;
             PLW_CLASS_TCB       ptcbCur;
             struct timespec     tvTO;

    volatile LW_SEL_WAKEUPNODE   selwunNode;                            /*  生成的 NODE 模板            */
             ULONG               ulError;
             BOOL                bBadFd, bReady;
             
    if (LW_CPU_GET_CUR_NESTING()) {
        _ErrorHandle(ERROR_KERNEL_IN_ISR);                              /*  不能在中断中调用            */
        return  (PX_ERROR);
    }
    
    if (iFd > (FD_SETSIZE - 1) || iFd < 0) {                            /*  文件号错误                  */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "file descriptor invalidate..\r\n");
        _ErrorHandle(ERROR_IOS_INVALID_FILE_DESCRIPTOR);                /*  文件描述符无效              */
        return  (PX_ERROR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    pselctx = ptcbCur->TCB_pselctxContext;
    
    if (!pselctx) {                                                     /*  没有 select context         */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "no select context.\r\n");
        _ErrorHandle(ERROR_IO_SELECT_CONTEXT);
        return  (PX_ERROR);
    }

    iWidthInBytes = __HOWMANY(iWidth, NFDBITS) * sizeof(fd_mask);       /*  需要检测的位数占多少字节    */
    
    lib_bzero(&pselctx->SELCTX_fdsetOrigReadFds, iWidthInBytes);
    FD_SET(iFd, &pselctx->SELCTX_fdsetOrigReadFds);

    if (!ptmvalTO) {                                                    /*  计算等待时间                */
        ulWaitTime = LW_OPTION_WAIT_INFINITE;                           /*  无限等待                    */

    } else {
        LW_TIMEVAL_TO_TIMESPEC(ptmvalTO, &tvTO);
        ulWaitTime = LW_TS_TIMEOUT_TICK(LW_TRUE, &tvTO);
    }

    API_SemaphoreBClear(pselctx->SELCTX_hSembWakeup);                   /*  清除信号量                  */
    
    selwunNode.SELWUN_hThreadId  = API_ThreadIdSelf();
    selwunNode.SELWUN_seltypType = SELREAD;
    selwunNode.SELWUN_iFd        = iFd;
    LW_SELWUN_CLEAR_READY(&selwunNode);
    
    LW_THREAD_SAFE();                                                   /*  进入安全模式                */
    
    pselctx->SELCTX_iWidth          = iWidth;                           /*  记录最大文件号              */
    pselctx->SELCTX_bPendedOnSelect = LW_TRUE;                          /*  需要 delete hook 清除 NODE  */
    pselctx->SELCTX_bBadFd          = LW_FALSE;
    
    if (ioctl(iFd, FIOSELECT, &selwunNode)) {                           /*  FIOSELECT                   */
        ioctl(iFd, FIOUNSELECT, &selwunNode);                           /*  FIOUNSELECT                 */
        
        bBadFd = pselctx->SELCTX_bBadFd;
        pselctx->SELCTX_bPendedOnSelect = LW_FALSE;                     /*  自行清理完毕                */
        
        LW_THREAD_UNSAFE();                                             /*  退出安全模式                */
        if (bBadFd) {
            _ErrorHandle(EBADF);
        }
        return  (PX_ERROR);                                             /*  错误                        */
    
    } else {
        if (LW_SELWUN_IS_READY(&selwunNode)) {
            LW_SELWUN_CLEAR_READY(&selwunNode);
            bReady = LW_TRUE;
        
        } else {
            bReady = LW_FALSE;
        }
    }

    LW_THREAD_UNSAFE();                                                 /*  退出安全模式                */

    ulError = API_SemaphoreBPend(pselctx->SELCTX_hSembWakeup,
                                 ulWaitTime);                           /*  开始等待                    */
    
    LW_THREAD_SAFE();                                                   /*  进入安全模式                */
    
    if (ioctl(iFd, FIOUNSELECT, &selwunNode) == ERROR_NONE) {           /*  FIOUNSELECT                 */
        if (!bReady && LW_SELWUN_IS_READY(&selwunNode)) {
            bReady = LW_TRUE;
        }
    }
    
    bBadFd = pselctx->SELCTX_bBadFd;
    pselctx->SELCTX_bPendedOnSelect = LW_FALSE;                         /*  自行清理完毕                */
    
    LW_THREAD_UNSAFE();                                                 /*  退出安全模式                */
    
    if (bBadFd) {                                                       /*  出现错误                    */
        _ErrorHandle(EBADF);
        return  (PX_ERROR);
    }
    
    _ErrorHandle(ulError);
    return  (bReady ? 1 : 0);                                           /*  检查文件是否可读            */
}
/*********************************************************************************************************
** 函数名称: waitwrite
** 功能描述: select() 变种,等待单个文件可写.
** 输　入  : iFd               文件描述符
**           ptmvalTO          等待超时时间, LW_NULL 表示永远等待.
** 输　出  : 正常等待到的文件数, 为 1 表示文件可以读, 为 0 表示超时 ,错误返回 PX_ERROR.
**           errno == ERROR_IO_SELECT_UNSUPPORT_IN_DRIVER       驱动程序不支持
**           errno == ERROR_IO_SELECT_CONTEXT                   线程不存在 context
**           errno == ERROR_THREAD_WAIT_TIMEOUT                 等待超时
**           errno == ERROR_KERNEL_IN_ISR                       在中断中调用
**           errno == ERROR_IOS_INVALID_FILE_DESCRIPTOR         文件描述符无效.
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
INT     waitwrite (INT  iFd, struct timeval   *ptmvalTO)
{
    REGISTER INT                 iWidth = iFd + 1;                      /*  iFd + 1                     */
    REGISTER INT                 iWidthInBytes;                         /*  需要检测的位数占多少字节    */
    REGISTER ULONG               ulWaitTime;                            /*  等待时间                    */
    REGISTER LW_SEL_CONTEXT     *pselctx;
             PLW_CLASS_TCB       ptcbCur;
             struct timespec     tvTO;
    
             LW_SEL_WAKEUPNODE   selwunNode;                            /*  生成的 NODE 模板            */
             ULONG               ulError;
             BOOL                bBadFd, bReady;
             
    if (LW_CPU_GET_CUR_NESTING()) {
        _ErrorHandle(ERROR_KERNEL_IN_ISR);                              /*  不能在中断中调用            */
        return  (PX_ERROR);
    }
    
    if (iFd > (FD_SETSIZE - 1) || iFd < 0) {                            /*  文件号错误                  */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "file descriptor invalidate..\r\n");
        _ErrorHandle(ERROR_IOS_INVALID_FILE_DESCRIPTOR);                /*  文件描述符无效              */
        return  (PX_ERROR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    pselctx = ptcbCur->TCB_pselctxContext;
    
    if (!pselctx) {                                                     /*  没有 select context         */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "no select context.\r\n");
        _ErrorHandle(ERROR_IO_SELECT_CONTEXT);
        return  (PX_ERROR);
    }

    iWidthInBytes = __HOWMANY(iWidth, NFDBITS) * sizeof(fd_mask);       /*  需要检测的位数占多少字节    */
    
    lib_bzero(&pselctx->SELCTX_fdsetOrigWriteFds, iWidthInBytes);
    FD_SET(iFd, &pselctx->SELCTX_fdsetOrigWriteFds);                    /*  指定文件置位                */
    
    if (!ptmvalTO) {                                                    /*  计算等待时间                */
        ulWaitTime = LW_OPTION_WAIT_INFINITE;                           /*  无限等待                    */

    } else {
        LW_TIMEVAL_TO_TIMESPEC(ptmvalTO, &tvTO);
        ulWaitTime = LW_TS_TIMEOUT_TICK(LW_TRUE, &tvTO);
    }
    
    API_SemaphoreBClear(pselctx->SELCTX_hSembWakeup);                   /*  清除信号量                  */
    
    selwunNode.SELWUN_hThreadId  = API_ThreadIdSelf();
    selwunNode.SELWUN_seltypType = SELWRITE;
    selwunNode.SELWUN_iFd        = iFd;
    LW_SELWUN_CLEAR_READY(&selwunNode);
    
    LW_THREAD_SAFE();                                                   /*  进入安全模式                */
    
    pselctx->SELCTX_iWidth          = iWidth;                           /*  记录最大文件号              */
    pselctx->SELCTX_bPendedOnSelect = LW_TRUE;                          /*  需要 delete hook 清除 NODE  */
    pselctx->SELCTX_bBadFd          = LW_FALSE;
    
    if (ioctl(iFd, FIOSELECT, &selwunNode)) {                           /*  FIOSELECT                   */
        ioctl(iFd, FIOUNSELECT, &selwunNode);                           /*  FIOUNSELECT                 */
        
        bBadFd = pselctx->SELCTX_bBadFd;
        pselctx->SELCTX_bPendedOnSelect = LW_FALSE;                     /*  自行清理完毕                */
        
        LW_THREAD_UNSAFE();                                             /*  退出安全模式                */
        if (bBadFd) {
            _ErrorHandle(EBADF);
        }
        return  (PX_ERROR);                                             /*  错误                        */
    
    } else {
        if (LW_SELWUN_IS_READY(&selwunNode)) {
            LW_SELWUN_CLEAR_READY(&selwunNode);
            bReady = LW_TRUE;
        
        } else {
            bReady = LW_FALSE;
        }
    }

    LW_THREAD_UNSAFE();                                                 /*  退出安全模式                */
    
    ulError = API_SemaphoreBPend(pselctx->SELCTX_hSembWakeup,
                                 ulWaitTime);                           /*  开始等待                    */
    
    LW_THREAD_SAFE();                                                   /*  进入安全模式                */
    
    if (ioctl(iFd, FIOUNSELECT, &selwunNode) == ERROR_NONE) {           /*  FIOUNSELECT                 */
        if (!bReady && LW_SELWUN_IS_READY(&selwunNode)) {
            bReady = LW_TRUE;
        }
    }
    
    bBadFd = pselctx->SELCTX_bBadFd;
    pselctx->SELCTX_bPendedOnSelect = LW_FALSE;                         /*  自行清理完毕                */
    
    LW_THREAD_UNSAFE();                                                 /*  退出安全模式                */
    
    if (bBadFd) {                                                       /*  出现错误                    */
        _ErrorHandle(EBADF);
        return  (PX_ERROR);
    }
    
    _ErrorHandle(ulError);
    return  (bReady ? 1 : 0);                                           /*  检查文件是否可读            */
}
/*********************************************************************************************************
** 函数名称: waitexcept
** 功能描述: select() 变种,等待单个文件异常.
** 输　入  : iFd               文件描述符
**           ptmvalTO          等待超时时间, LW_NULL 表示永远等待.
** 输　出  : 正常等待到的文件数, 为 1 表示文件可以读, 为 0 表示超时 ,错误返回 PX_ERROR.
**           errno == ERROR_IO_SELECT_UNSUPPORT_IN_DRIVER       驱动程序不支持
**           errno == ERROR_IO_SELECT_CONTEXT                   线程不存在 context
**           errno == ERROR_THREAD_WAIT_TIMEOUT                 等待超时
**           errno == ERROR_KERNEL_IN_ISR                       在中断中调用
**           errno == ERROR_IOS_INVALID_FILE_DESCRIPTOR         文件描述符无效.
** 全局变量: 
** 调用模块: 
                                           API 函数
*********************************************************************************************************/
LW_API  
INT     waitexcept (INT  iFd, struct timeval   *ptmvalTO)
{
    REGISTER INT                 iWidth = iFd + 1;                      /*  iFd + 1                     */
    REGISTER INT                 iWidthInBytes;                         /*  需要检测的位数占多少字节    */
    REGISTER ULONG               ulWaitTime;                            /*  等待时间                    */
    REGISTER LW_SEL_CONTEXT     *pselctx;
             PLW_CLASS_TCB       ptcbCur;
             struct timespec     tvTO;
    
             LW_SEL_WAKEUPNODE   selwunNode;                            /*  生成的 NODE 模板            */
             ULONG               ulError;
             BOOL                bBadFd, bReady;
             
    if (LW_CPU_GET_CUR_NESTING()) {
        _ErrorHandle(ERROR_KERNEL_IN_ISR);                              /*  不能在中断中调用            */
        return  (PX_ERROR);
    }
    
    if (iFd > (FD_SETSIZE - 1) || iFd < 0) {                            /*  文件号错误                  */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "file descriptor invalidate..\r\n");
        _ErrorHandle(ERROR_IOS_INVALID_FILE_DESCRIPTOR);                /*  文件描述符无效              */
        return  (PX_ERROR);
    }
    
    LW_TCB_GET_CUR_SAFE(ptcbCur);
    
    pselctx = ptcbCur->TCB_pselctxContext;
    
    if (!pselctx) {                                                     /*  没有 select context         */
        _DebugHandle(__ERRORMESSAGE_LEVEL, "no select context.\r\n");
        _ErrorHandle(ERROR_IO_SELECT_CONTEXT);
        return  (PX_ERROR);
    }

    iWidthInBytes = __HOWMANY(iWidth, NFDBITS) * sizeof(fd_mask);       /*  需要检测的位数占多少字节    */
    
    lib_bzero(&pselctx->SELCTX_fdsetOrigExceptFds, iWidthInBytes);
    FD_SET(iFd, &pselctx->SELCTX_fdsetOrigExceptFds);                   /*  指定文件置位                */
    
    if (!ptmvalTO) {                                                    /*  计算等待时间                */
        ulWaitTime = LW_OPTION_WAIT_INFINITE;                           /*  无限等待                    */

    } else {
        LW_TIMEVAL_TO_TIMESPEC(ptmvalTO, &tvTO);
        ulWaitTime = LW_TS_TIMEOUT_TICK(LW_TRUE, &tvTO);
    }
    
    API_SemaphoreBClear(pselctx->SELCTX_hSembWakeup);                   /*  清除信号量                  */
    
    selwunNode.SELWUN_hThreadId  = API_ThreadIdSelf();
    selwunNode.SELWUN_seltypType = SELEXCEPT;
    selwunNode.SELWUN_iFd        = iFd;
    LW_SELWUN_CLEAR_READY(&selwunNode);
    
    LW_THREAD_SAFE();                                                   /*  进入安全模式                */
    
    pselctx->SELCTX_iWidth          = iWidth;                           /*  记录最大文件号              */
    pselctx->SELCTX_bPendedOnSelect = LW_TRUE;                          /*  需要 delete hook 清除 NODE  */
    pselctx->SELCTX_bBadFd          = LW_FALSE;
    
    if (ioctl(iFd, FIOSELECT, &selwunNode)) {                           /*  FIOSELECT                   */
        ioctl(iFd, FIOUNSELECT, &selwunNode);                           /*  FIOUNSELECT                 */
        
        bBadFd = pselctx->SELCTX_bBadFd;
        pselctx->SELCTX_bPendedOnSelect = LW_FALSE;                     /*  自行清理完毕                */
        
        LW_THREAD_UNSAFE();                                             /*  退出安全模式                */
        if (bBadFd) {
            _ErrorHandle(EBADF);
        }
        return  (PX_ERROR);                                             /*  错误                        */
    
    } else {
        if (LW_SELWUN_IS_READY(&selwunNode)) {
            LW_SELWUN_CLEAR_READY(&selwunNode);
            bReady = LW_TRUE;
        
        } else {
            bReady = LW_FALSE;
        }
    }

    LW_THREAD_UNSAFE();                                                 /*  退出安全模式                */
    
    ulError = API_SemaphoreBPend(pselctx->SELCTX_hSembWakeup,
                                 ulWaitTime);                           /*  开始等待                    */
    
    LW_THREAD_SAFE();                                                   /*  进入安全模式                */
    
    if (ioctl(iFd, FIOUNSELECT, &selwunNode) == ERROR_NONE) {           /*  FIOUNSELECT                 */
        if (!bReady && LW_SELWUN_IS_READY(&selwunNode)) {
            bReady = LW_TRUE;
        }
    }
    
    bBadFd = pselctx->SELCTX_bBadFd;
    pselctx->SELCTX_bPendedOnSelect = LW_FALSE;                         /*  自行清理完毕                */
    
    LW_THREAD_UNSAFE();                                                 /*  退出安全模式                */
    
    if (bBadFd) {                                                       /*  出现错误                    */
        _ErrorHandle(EBADF);
        return  (PX_ERROR);
    }
    
    _ErrorHandle(ulError);
    return  (bReady ? 1 : 0);                                           /*  检查文件是否可读            */
}

#endif                                                                  /*  LW_CFG_DEVICE_EN > 0        */
                                                                        /*  LW_CFG_SELECT_EN > 0        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
