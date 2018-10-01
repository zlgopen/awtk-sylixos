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
** 文   件   名: if_param.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2015 年 09 月 20 日
**
** 描        述: 网络接口配置参数获取.
*********************************************************************************************************/

#ifndef __IF_PARAM_H
#define __IF_PARAM_H

#include "netinet/in.h"
/*********************************************************************************************************
  裁剪控制
*********************************************************************************************************/
#if LW_CFG_NET_EN > 0

/*********************************************************************************************************
  本配置系统, 供网卡驱动程序初始化部分使用, 具体行为需要网卡驱动程序实现.
  
  网络参数文件格式范例 /etc/ifparam.ini

  [dm9000a]
  enable=1
  ipaddr=192.168.1.2
  netmask=255.255.255.0
  gateway=192.168.1.1
  default=1
  mac=00:11:22:33:44:55   # 除非网卡没有 MAC 地址, 否则不建议设置 MAC
  ipv6_auto_cfg=1         # 如果将 SylixOS 作为 IPv6 路由器, 则 ipv6_auto_cfg=0
  tcp_ack_freq=2          # TCP Delay ACK 响应频率 (2~127), 默认为 2, 
                            既接收两个总和大于 MSS 长度数据包立即发送 ACK
  tcp_wnd=8192            # TCP window (tcp_wnd > 2 * MSS) && (tcp_wnd < (0xffffu << TCP_RCV_SCALE))
  
  mipaddr=10.0.0.2        # 添加一个辅助 IP 地址
  mnetmask=255.0.0.0
  mgateway=10.0.0.1
  
  mipaddr=172.168.0.2     # 添加一个辅助 IP 地址
  mnetmask=255.255.0.0
  mgateway=172.168.0.2
  
  或者
  
  [dm9000a]
  enable=1
  dhcp=1
  dhcp6=1
  mac=00:11:22:33:44:55   # 除非网卡没有 MAC 地址, 否则不建议设置 MAC
*********************************************************************************************************/
/*********************************************************************************************************
  resolver 类库配置文件范例 /etc/resolv.conf

  nameserver x.x.x.x
*********************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif                                                                  /*  __cplusplus                 */

LW_API void  *if_param_load(const char *name);
LW_API void   if_param_unload(void *pifparam);
LW_API int    if_param_getenable(void *pifparam, int *enable);
LW_API int    if_param_getdefault(void *pifparam, int *def);
LW_API int    if_param_getdhcp(void *pifparam, int *dhcp);
LW_API int    if_param_getdhcp6(void *pifparam, int *dhcp);
LW_API int    if_param_ipv6autocfg(void *pifparam, int *autocfg);
LW_API int    if_param_tcpackfreq(void *pifparam, int *tcpaf);
LW_API int    if_param_tcpwnd(void *pifparam, int *tcpwnd);
LW_API int    if_param_getipaddr(void *pifparam, ip4_addr_t *ipaddr);
LW_API int    if_param_getinaddr(void *pifparam, struct in_addr *inaddr);
LW_API int    if_param_getnetmask(void *pifparam, ip4_addr_t *mask);
LW_API int    if_param_getinnetmask(void *pifparam, struct in_addr *mask);
LW_API int    if_param_getgw(void *pifparam, ip4_addr_t *gw);
LW_API int    if_param_getingw(void *pifparam, struct in_addr *gw);
LW_API int    if_param_getmac(void *pifparam, char *mac, size_t  sz);
LW_API void   if_param_syncdns(void);

#if LW_CFG_NET_NETDEV_MIP_EN > 0
LW_API int    if_param_getmipaddr(void *pifparam, int  idx, ip4_addr_t *ipaddr);
LW_API int    if_param_getmnetmask(void *pifparam, int  idx, ip4_addr_t *mask);
LW_API int    if_param_getmgw(void *pifparam, int  idx, ip4_addr_t *gw);
#endif                                                                  /*  LW_CFG_NET_NETDEV_MIP_EN    */

#ifdef __cplusplus
}
#endif                                                                  /*  __cplusplus                 */

#endif                                                                  /*  LW_CFG_NET_EN               */
#endif                                                                  /*  __IF_ETHER_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
