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
** 文   件   名: ip_qos.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2018 年 07 月 17 日
**
** 描        述: IPv4 QoS support.
*********************************************************************************************************/

#ifndef __NETINET_IP_QOS_H
#define __NETINET_IP_QOS_H

#include "sys/types.h"
#include "sys/ioctl.h"

#include "lwip/opt.h"
#include "lwip/def.h"

#if LWIP_IPV4

#include "lwip/inet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************************************
  Configuration structure for SIOCSETIPQOS and SIOCGETIPQOS ioctls.
*********************************************************************************************************/

struct ipqosreq {
    u_int    ip_qos_en;
    u_int    ip_qos_reserved[3];                                /*  reserved for furture                */
};

#define SIOCSETIPQOS    _IOWR('i', 60, struct ipqosreq)
#define SIOCGETIPQOS    _IOWR('i', 61, struct ipqosreq)

#ifdef __cplusplus
}
#endif

#endif                                                          /*  LWIP_IPV4                           */
#endif                                                          /*  __NETINET_IP_QOS_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
