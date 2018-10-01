/**
 * @file
 * Lwip platform independent driver interface.
 * This set of driver interface shields the netif details, 
 * as much as possible compatible with different versions of LwIP
 * Verification using sylixos(tm) real-time operating system
 */

/*
 * Copyright (c) 2006-2018 SylixOS Group.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 * 4. This code has been or is applying for intellectual property protection 
 *    and can only be used with acoinfo software products.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 * 
 * Author: Han.hui <hanhui@acoinfo.com>
 *
 */

#define  __SYLIXOS_KERNEL
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/inet.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"

#if LW_CFG_NET_NETDEV_MIP_EN > 0

#include "string.h"
#include "netdev.h"

/* add a IP to netdev init callback */
static err_t netdev_mipif_init (struct netif *mipif)
{
  netdev_t *netdev = (netdev_t *)(mipif->state);
  struct netif *netif = (struct netif *)netdev->sys;
  
#if LWIP_NETIF_HOSTNAME
  mipif->hostname = netif->hostname;
#endif /* LWIP_NETIF_HOSTNAME */
  
  mipif->name[0] = 'm';
  mipif->name[1] = 'i';
  
  /* no ipv6, no multicast, no promisc */
  mipif->flags = (u8_t)(netif->flags & ~(NETIF_FLAG_IGMP | NETIF_FLAG_MLD6));
  
  mipif->output = netif->output;
  mipif->linkoutput = netif->linkoutput;
  
  mipif->mtu = netif->mtu;
  mipif->link_speed = netif->link_speed;
  mipif->chksum_flags = netif->chksum_flags;

  mipif->hwaddr_len = netif->hwaddr_len;
  MEMCPY(mipif->hwaddr, netif->hwaddr, netif->hwaddr_len);

  netif_set_tcp_ack_freq(mipif, netif->tcp_ack_freq);
  netif_set_tcp_wnd(mipif, netif->tcp_wnd);

  /* link to list */
  mipif->mipif = netif->mipif;
  netif->mipif = mipif;
  mipif->masterif = netif;
  
  return (ERR_OK);
}

/* add a IP to netdev (use slave interface) */
int netdev_mipif_add (netdev_t *netdev, const ip4_addr_t *ip4, 
                      const ip4_addr_t *netmask4, const ip4_addr_t *gw4)
{
  struct netif *netif, *mipif;
  
  if (!netdev || (netdev->magic_no != NETDEV_MAGIC)) {
    errno = EINVAL;
    return (-1);
  }
  
  if (ip4_addr_isany(ip4)) {
    errno = EINVAL;
    return (-1);
  }
  
  netif = (struct netif *)netdev->sys;
  
  if (ip4_addr_cmp(netif_ip4_addr(netif), ip4)) {
    errno = EADDRINUSE;
    return (-1);
  }
  
  NETIF_MIPIF_FOREACH(netif, mipif) {
    if (ip4_addr_cmp(netif_ip4_addr(mipif), ip4)) {
      errno = EADDRINUSE;
      return (-1);
    }
  }
  
  mipif = (struct netif *)mem_malloc(sizeof(struct netif));
  if (!mipif) {
    errno = ENOMEM;
    return (-1);
  }
  lib_bzero(mipif, sizeof(struct netif));
  
  if (netifapi_netif_add(mipif, ip4, netmask4, gw4, netdev, netdev_mipif_init, tcpip_input)) {
    errno = ENOSPC;
    return (-1);
  }
  
  return (0);
}

/* delete a IP from netdev (use slave interface) */
int netdev_mipif_delete (netdev_t *netdev, const ip4_addr_t *ip4)
{
  struct netif *netif, *mipif, *tmp;
  
  if (!netdev || (netdev->magic_no != NETDEV_MAGIC)) {
    errno = EINVAL;
    return (-1);
  }
  
  if (ip4_addr_isany(ip4)) {
    errno = EINVAL;
    return (-1);
  }
  
  netif = (struct netif *)netdev->sys;
  if (!netif->mipif) {
    errno = EINVAL;
    return (-1);
  }
  
  mipif = netif->mipif;
  
  if (ip4_addr_cmp(netif_ip4_addr(mipif), ip4)) {
    netif->mipif = mipif->mipif;
    
  } else {
    tmp = mipif;
    for (mipif = mipif->mipif; mipif != NULL; mipif = mipif->mipif) {
      if (ip4_addr_cmp(netif_ip4_addr(mipif), ip4)) {
        tmp->mipif = mipif->mipif;
        break;
      }
      tmp = mipif;
    }
  }
  
  if (mipif) {
    netifapi_netif_remove(mipif);
    mem_free(mipif);
    return (0);
  }
  
  errno = EINVAL;
  return (-1);
}

/* clean all slave interface */
void netdev_mipif_clean (netdev_t *netdev)
{
  struct netif *netif, *mipif, *tmp;

  if (!netdev || (netdev->magic_no != NETDEV_MAGIC)) {
    return;
  }
  
  netif = (struct netif *)netdev->sys;
  mipif = netif->mipif;
  
  while (mipif) {
    tmp = mipif->mipif;
    netifapi_netif_remove(mipif);
    mem_free(mipif);
    mipif = tmp;
  }
}

/* set all slave interface update mtu, linkup, updown */
void netdev_mipif_update (netdev_t *netdev)
{
  struct netif *netif, *mipif;
  
  if (!netdev || (netdev->magic_no != NETDEV_MAGIC)) {
    return;
  }
  
  netif = (struct netif *)netdev->sys;
  
  NETIF_MIPIF_FOREACH(netif, mipif) {
    mipif->mtu = netif->mtu;
    mipif->link_speed = netif->link_speed;
    if ((mipif->flags & NETIF_FLAG_UP) && !(netif->flags & NETIF_FLAG_UP)) {
      netif_set_down(mipif);
    } else if (!(mipif->flags & NETIF_FLAG_UP) && (netif->flags & NETIF_FLAG_UP)) {
      netif_set_up(mipif);
    }
    mipif->flags = (u8_t)(netif->flags & ~(NETIF_FLAG_IGMP | NETIF_FLAG_MLD6));
  }
}

/* set all slave interface update tcp ack freq, tcp wnd */
void netdev_mipif_tcpupd (netdev_t *netdev)
{
  struct netif *netif, *mipif;
  
  if (!netdev || (netdev->magic_no != NETDEV_MAGIC)) {
    return;
  }
  
  netif = (struct netif *)netdev->sys;
  
  NETIF_MIPIF_FOREACH(netif, mipif) {
    netif_set_tcp_ack_freq(mipif, netif->tcp_ack_freq);
    netif_set_tcp_wnd(mipif, netif->tcp_wnd);
  }
}

/* set all slave interface hwaddr */
void netdev_mipif_hwaddr (netdev_t *netdev)
{
  struct netif *netif, *mipif;
  
  if (!netdev || (netdev->magic_no != NETDEV_MAGIC)) {
    return;
  }
  
  netif = (struct netif *)netdev->sys;
  
  NETIF_MIPIF_FOREACH(netif, mipif) {
    MEMCPY(mipif->hwaddr, netif->hwaddr, netif->hwaddr_len);
  }
}

/* set all slave interface find */
struct netif *netdev_mipif_search (netdev_t *netdev, struct pbuf *p)
{
  u16_t next_offset;
  u16_t type;
  ip4_addr_t destip;
  struct netif *netif, *mipif;
  
  netif = (struct netif *)netdev->sys;
  
  destip.addr = IPADDR_ANY;
  
  if (netif->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET)) {
    struct eth_hdr *ethhdr = (struct eth_hdr *)p->payload;
    next_offset = SIZEOF_ETH_HDR;
    if (LW_UNLIKELY(p->len < SIZEOF_ETH_HDR)) {
      return (netif);
    }
    
    type = ethhdr->type;
    if (type == PP_HTONS(ETHTYPE_VLAN)) {
      struct eth_vlan_hdr *vlan = (struct eth_vlan_hdr *)(((char *)ethhdr) + SIZEOF_ETH_HDR);
      if (LW_UNLIKELY(p->len < SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR)) {
        return (netif);
      }
      next_offset = SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR;
      type = vlan->tpid;
    }
    
    switch (type) {
    
    case PP_HTONS(ETHTYPE_ARP):
    case PP_HTONS(ETHTYPE_RARP): {
        struct etharp_hdr *arphdr = (struct etharp_hdr *)((u8_t *)p->payload + next_offset);
        if (LW_UNLIKELY(p->len < next_offset + SIZEOF_ETHARP_HDR)) {
          return (netif);
        }
#if BYTE_ORDER == BIG_ENDIAN
        destip.addr = (arphdr->dipaddr.addrw[0] << 16) | arphdr->dipaddr.addrw[1];
#else
        destip.addr = (arphdr->dipaddr.addrw[1] << 16) | arphdr->dipaddr.addrw[0];
#endif
      }
      break;
      
    case PP_HTONS(ETHTYPE_IP): {
        struct ip_hdr *iphdr = (struct ip_hdr *)((char *)p->payload + next_offset);
        if (LW_UNLIKELY(p->len < next_offset + IP_HLEN)) {
          return (netif);
        }
        if (IPH_V(iphdr) != 4) {
          return (netif);
        }
        destip.addr = iphdr->dest.addr;
      }
      break;
      
    default:
      return (netif);
    }
  
  } else {
    struct ip_hdr *iphdr = (struct ip_hdr *)((char *)p->payload);
    if (LW_UNLIKELY(p->len < IP_HLEN)) {
      return (netif);
    }
    if (IPH_V(iphdr) != 4) {
      return (netif);
    }
    destip.addr = iphdr->dest.addr;
  }
  
  if (ip4_addr_cmp(netif_ip4_addr(netif), &destip)) {
    return (netif);
  }
  
  NETIF_MIPIF_FOREACH(netif, mipif) {
    if (ip4_addr_cmp(netif_ip4_addr(mipif), &destip)) {
      return (mipif);
    }
  }
  
  return (netif);
}

#endif /* LW_CFG_NET_NETDEV_MIP_EN */
/*
 * end
 */
