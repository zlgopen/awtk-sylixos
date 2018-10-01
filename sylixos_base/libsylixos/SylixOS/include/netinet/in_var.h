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
** 文   件   名: in_var.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2013 年 01 月 19 日
**
** 描        述: include/netinet/in_var .
*********************************************************************************************************/

#ifndef __NETINET_IN_VAR_H
#define __NETINET_IN_VAR_H

#include "sys/socket.h"
#include "net/if.h"

struct in_aliasreq {
    char                ifra_name[IFNAMSIZ];                            /*  if name, e.g. "en1"         */
    struct sockaddr_in  ifra_addr;
    struct sockaddr_in  ifra_broadaddr;
#define ifra_dstaddr    ifra_broadaddr
    struct sockaddr_in  ifra_mask;
};

#endif                                                                  /*  __NETINET_IN_VAR_H          */
/*********************************************************************************************************
  END
*********************************************************************************************************/
