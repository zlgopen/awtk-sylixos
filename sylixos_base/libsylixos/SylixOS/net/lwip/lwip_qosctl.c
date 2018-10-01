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
** 文   件   名: lwip_qosctl.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2018 年 07 月 17 日
**
** 描        述: ioctl QoS 支持.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪控制
*********************************************************************************************************/
#if LW_CFG_NET_EN > 0 && LW_CFG_LWIP_IPQOS > 0
#include "netinet/ip_qos.h"
#include "netinet6/ip6_qos.h"
#include "sys/socket.h"
#include "lwip/tcpip.h"
/*********************************************************************************************************
** 函数名称: __qosIoctlInet
** 功能描述: SIOCSETIPQOS / SIOCGETIPQOS 命令处理接口
** 输　入  : iFamily    AF_INET / AF_INET6
**           iCmd       SIOCSETIPQOS / SIOCGETIPQOS
**           pvArg      struct ipqosreq / struct ip6qosreq
** 输　出  : ERROR or OK
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  __qosIoctlInet (INT  iFamily, INT  iCmd, PVOID  pvArg)
{
    INT  iRet = PX_ERROR;

    if (!pvArg) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }
    
    switch (iCmd) {
    
    case SIOCSETIPQOS:
        if (iFamily == AF_INET) {
            struct ipqosreq *pipqosreq = (struct ipqosreq *)pvArg;
            
            if (pipqosreq->ip_qos_en) {
                tcpip_qos_set(1);
            
            } else {
                tcpip_qos_set(0);
            }
            
            iRet = ERROR_NONE;
        }
#if LWIP_IPV6
          else {
            struct ip6qosreq *pip6qosreq = (struct ip6qosreq *)pvArg;
            
            if (pip6qosreq->ip6_qos_en) {
                tcpip_qos_set(1);
            
            } else {
                tcpip_qos_set(0);
            }
            
            iRet = ERROR_NONE;
        }
#endif
        break;
        
    case SIOCGETIPQOS:
        if (iFamily == AF_INET) {
            struct ipqosreq *pipqosreq = (struct ipqosreq *)pvArg;
            
            pipqosreq->ip_qos_en = tcpip_qos_get();
            pipqosreq->ip_qos_reserved[0] = 0;
            pipqosreq->ip_qos_reserved[1] = 0;
            pipqosreq->ip_qos_reserved[2] = 0;
            
            iRet = ERROR_NONE;
        } 
#if LWIP_IPV6
          else {
            struct ip6qosreq *pip6qosreq = (struct ip6qosreq *)pvArg;
            
            pip6qosreq->ip6_qos_en = tcpip_qos_get();
            pip6qosreq->ip6_qos_reserved[0] = 0;
            pip6qosreq->ip6_qos_reserved[1] = 0;
            pip6qosreq->ip6_qos_reserved[2] = 0;
            
            iRet = ERROR_NONE;
        }
#endif
        break;
    
    default:
        _ErrorHandle(ENOSYS);
        break;
    }
    
    return  (iRet);
}

#endif                                                                  /*  LW_CFG_NET_EN               */
                                                                        /*  LW_CFG_LWIP_IPQOS           */
/*********************************************************************************************************
  END
*********************************************************************************************************/
