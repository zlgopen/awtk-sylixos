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
** 文   件   名: selectCtl.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2007 年 11 月 07 日
**
** 描        述:  IO 系统 select 子系统 ioctl 控制部分.

** BUG
2008.05.31  将参数改为 long 型, 支持 64 位系统.
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
** 函数名称: __selDoIoctls
** 功能描述: 执行 pfdset 文件集中所有的 ioctl 命令.
** 输　入  : pfdset              文件集
**           pfdsetUpdate        需要升级的文件集
**           iFdSetWidth         最大的文件号
**           iFunc               ioctl 命令
**           pselwun             sel 节点
**           bStopOnErr          发生错误时是否立即结束.
** 输　出  : 正常时返回 ERROR_NONE, 有错误发生时返回 PX_ERROR
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT   __selDoIoctls (fd_set             *pfdset, 
                     fd_set             *pfdsetUpdate,
                     INT                 iFdSetWidth, 
                     INT                 iFunc,
                     PLW_SEL_WAKEUPNODE  pselwun, 
                     BOOL                bStopOnErr)
{
    REGISTER INT        iIsOk = ERROR_NONE;
    
    REGISTER INT        iFdTemp;                                        /*  临时文件描述符              */
    REGISTER ULONG      ulPartMask;                                     /*  区域掩码                    */
    
    volatile PLW_SEL_WAKEUPNODE  pselwunNode = (volatile PLW_SEL_WAKEUPNODE)pselwun;
                                                                        /*  volatile for 数据准备好判断 */
    LW_SELWUN_CLEAR_READY(pselwunNode);
    
    for (iFdTemp = 0; iFdTemp < iFdSetWidth; iFdTemp++) {               /*  检查所有可执行读操作的文件  */
        ulPartMask = pfdset->fds_bits[((unsigned)iFdTemp) / NFDBITS];   /*  获得 iFdTemp 所在的掩码组   */
        if (ulPartMask == 0) {                                          /*  这个组中与这个文件无关      */
            iFdTemp += NFDBITS - 1;                                     /*  进行下一个掩码组判断        */
        
        } else if (ulPartMask & (ULONG)(1ul << (((unsigned)iFdTemp) % NFDBITS))) {
            pselwunNode->SELWUN_iFd = iFdTemp;
            
            if (ioctl(iFdTemp, iFunc, pselwunNode)) {                   /*  ioctl() 失败                */
                iIsOk = PX_ERROR;
                if (bStopOnErr) {                                       /*  遇到错误立即停止?           */
                    break;
                }
            
            } else {
                if (pfdsetUpdate && LW_SELWUN_IS_READY(pselwunNode)) {  /*  更新文件描述符集            */
                    FD_SET(iFdTemp, pfdsetUpdate);
                    LW_SELWUN_CLEAR_READY(pselwunNode);
                }
            }
        }
    }
    
    return  (iIsOk);
}

#endif                                                                  /*  LW_CFG_DEVICE_EN > 0        */
                                                                        /*  LW_CFG_SELECT_EN > 0        */
/*********************************************************************************************************
  END
*********************************************************************************************************/
