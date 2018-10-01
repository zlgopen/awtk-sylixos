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
** 文   件   名: lwip_npf.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2011 年 02 月 11 日
**
** 描        述: lwip net packet filter 工具.

** BUG:
2011.07.07  修正一些拼写错误.
2013.09.11  加入 /proc/net/netfilter 文件.
            UDP/TCP 滤波加入端口范围.
2013.09.12  使用 bnprintf 专用缓冲区打印函数.
2016.04.13  __npfInput() 自己释放数据包缓存.
2018.01.24  使用新的 firewall 接口.
2018.08.06  改为读写锁, 删除手动 attach detach 接口, 改为自动模式.
2018.08.17  支持 [in | out] [allow | deny] 选择, 实现较为完整的防火墙功能.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
#include "../SylixOS/net/include/net_net.h"
/*********************************************************************************************************
  裁剪控制
*********************************************************************************************************/
#if (LW_CFG_NET_EN > 0) && (LW_CFG_NET_NPF_EN > 0)
#include "net/if.h"
#include "net/if_lock.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"
#include "netif/etharp.h"
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define __NPF_NETIF_RULE_FAST       1                                   /*  快速检测模式                */
#define __NPF_NETIF_RULE_MAX        4                                   /*  规则表种类                  */
#define __NPF_NETIF_HASH_SIZE       16                                  /*  网络接口 hash 表大小        */
#define __NPF_NETIF_HASH_MASK       (__NPF_NETIF_HASH_SIZE - 1)         /*  hash 掩码                   */
/*********************************************************************************************************
  规则表操作锁
*********************************************************************************************************/
static LW_OBJECT_HANDLE             _G_ulNpfLock;

#define __NPF_LOCK_R()              API_SemaphoreRWPendR(_G_ulNpfLock, LW_OPTION_WAIT_INFINITE)
#define __NPF_LOCK_W()              API_SemaphoreRWPendW(_G_ulNpfLock, LW_OPTION_WAIT_INFINITE)
#define __NPF_UNLOCK()              API_SemaphoreRWPost(_G_ulNpfLock)
/*********************************************************************************************************
  统计信息 (缓存不足和规则阻止, 都会造成丢弃计数器加1)
*********************************************************************************************************/
static ULONG    _G_ulNpfDropPacketCounter  = 0;                         /*  丢弃数据报的个数            */
static ULONG    _G_ulNpfAllowPacketCounter = 0;                         /*  放行数据报的个数            */

#define __NPF_PACKET_DROP_INC()     (_G_ulNpfDropPacketCounter++)
#define __NPF_PACKET_ALLOW_INC()    (_G_ulNpfAllowPacketCounter++)
#define __NPF_PACKET_DROP_GET()     (_G_ulNpfDropPacketCounter)
#define __NPF_PACKET_ALLOW_GET()    (_G_ulNpfAllowPacketCounter)
/*********************************************************************************************************
  规则控制结构

  注意:
       如果需要禁用 UDP/TCP 某一端口, 起始和结束 IP 地址字段就设为 0.0.0.0 ~ FF.FF.FF.FF
       NPFR?_iRule 字段是为了从句柄获取规则的种类
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE            NPFRM_lineManage;                           /*  MAC 规则管理链表            */
    INT                     NPFRM_iRule;
    BOOL                    NPFRM_bAllow;
    u8_t                    NPFRM_ucMac[NETIF_MAX_HWADDR_LEN];          /*  不允许接收的 MAC 地址       */
} __NPF_RULE_MAC;
typedef __NPF_RULE_MAC     *__PNPF_RULE_MAC;

typedef struct {
    LW_LIST_LINE            NPFRI_lineManage;                           /*  IP 规则管理链表             */
    INT                     NPFRI_iRule;
    BOOL                    NPFRI_bAllow;
    ip4_addr_t              NPFRI_ipaddrHboS;                           /*  IP 段起始 IP 地址           */
    ip4_addr_t              NPFRI_ipaddrHboE;                           /*  IP 段结束 IP 地址           */
} __NPF_RULE_IP;
typedef __NPF_RULE_IP      *__PNPF_RULE_IP;

typedef struct {
    LW_LIST_LINE            NPFRU_lineManage;                           /*  UDP 规则管理链表            */
    INT                     NPFRU_iRule;
    BOOL                    NPFRU_bAllow;
    ip4_addr_t              NPFRU_ipaddrHboS;                           /*  IP 段起始 IP 地址           */
    ip4_addr_t              NPFRU_ipaddrHboE;                           /*  IP 段结束 IP 地址           */
    u16_t                   NPFRU_usPortHboS;                           /*  端口起始 主机序             */
    u16_t                   NPFRU_usPortHboE;                           /*  端口结束                    */
} __NPF_RULE_UDP;
typedef __NPF_RULE_UDP     *__PNPF_RULE_UDP;

typedef struct {
    LW_LIST_LINE            NPFRT_lineManage;                           /*  TCP 规则管理链表            */
    INT                     NPFRT_iRule;
    BOOL                    NPFRT_bAllow;
    ip4_addr_t              NPFRT_ipaddrHboS;                           /*  IP 段起始 IP 地址           */
    ip4_addr_t              NPFRT_ipaddrHboE;                           /*  IP 段结束 IP 地址           */
    u16_t                   NPFRT_usPortHboS;                           /*  端口起始 主机序             */
    u16_t                   NPFRT_usPortHboE;                           /*  端口结束                    */
} __NPF_RULE_TCP;
typedef __NPF_RULE_TCP     *__PNPF_RULE_TCP;
/*********************************************************************************************************
  过滤器网络接口结构
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE            NPFNI_lineHash;                             /*  hash 表                     */
    LW_LIST_LINE_HEADER     NPFNI_npfrnIn[__NPF_NETIF_RULE_MAX];        /*  输入规则表                  */
    LW_LIST_LINE_HEADER     NPFNI_npfrnOut[__NPF_NETIF_RULE_MAX];       /*  输出规则表                  */
    CHAR                    NPFNI_cName[IF_NAMESIZE];                   /*  网络接口名                  */
    BOOL                    NPFNI_bAttached;                            /*  是否已经连接                */
} __NPF_NETIF_CB;
typedef __NPF_NETIF_CB     *__PNPF_NETIF_CB;
/*********************************************************************************************************
  规则 hash 表
*********************************************************************************************************/
static LW_LIST_LINE_HEADER  _G_plineNpfHash[__NPF_NETIF_HASH_SIZE];     /*  通过 name & num 进行散列    */
static ULONG                _G_ulNpfCounter = 0ul;                      /*  规则计数器                  */
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
static INT    npf_netif_firewall(struct netif *pnetif, struct pbuf *p);
#if LW_CFG_SHELL_EN > 0
static INT    __tshellNetNpfShow(INT  iArgC, PCHAR  *ppcArgV);
static INT    __tshellNetNpfRuleAdd(INT  iArgC, PCHAR  *ppcArgV);
static INT    __tshellNetNpfRuleDel(INT  iArgC, PCHAR  *ppcArgV);
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
#if LW_CFG_PROCFS_EN > 0
static VOID   __procFsNpfInit(VOID);
#endif                                                                  /*  LW_CFG_PROCFS_EN > 0        */
/*********************************************************************************************************
** 函数名称: __npfNetifGet
** 功能描述: 获得指定的过滤器网络接口结构
** 输　入  : pnetif         网络接口
** 输　出  : 找到的过滤器网络接口结构
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static LW_INLINE __PNPF_NETIF_CB  __npfNetifGet (struct netif *pnetif)
{
    return  ((__PNPF_NETIF_CB)pnetif->inner_fw_stat);
}
/*********************************************************************************************************
** 函数名称: __npfNetifFind
** 功能描述: 查找指定的过滤器网络接口结构
** 输　入  : pcIfname       网络接口名字
** 输　出  : 找到的过滤器网络接口结构
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static __PNPF_NETIF_CB  __npfNetifFind (CPCHAR  pcIfname)
{
    REGISTER INT                    iIndex;
             PLW_LIST_LINE          plineTemp;
             __PNPF_NETIF_CB        pnpfniTemp;


    iIndex = (pcIfname[0] + pcIfname[1] + pcIfname[2]) & __NPF_NETIF_HASH_MASK;

    for (plineTemp  = _G_plineNpfHash[iIndex];
         plineTemp != LW_NULL;
         plineTemp  = _list_line_get_next(plineTemp)) {

        pnpfniTemp = _LIST_ENTRY(plineTemp, __NPF_NETIF_CB, NPFNI_lineHash);
        if (lib_strncmp(pnpfniTemp->NPFNI_cName, pcIfname, IF_NAMESIZE) == 0) {
            return  (pnpfniTemp);
        }
    }

    return  (LW_NULL);
}
/*********************************************************************************************************
** 函数名称: __npfNetifInsertHash
** 功能描述: 将指定的过滤器网络接口结构插入 hash 表
** 输　入  : pnpfni        过滤器网络接口结构
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  __npfNetifInsertHash (__PNPF_NETIF_CB  pnpfni)
{
    REGISTER INT                    iIndex;
    REGISTER LW_LIST_LINE_HEADER   *ppline;

    iIndex = (pnpfni->NPFNI_cName[0]
           +  pnpfni->NPFNI_cName[1]
           +  pnpfni->NPFNI_cName[2])
           & __NPF_NETIF_HASH_MASK;

    ppline = &_G_plineNpfHash[iIndex];

    _List_Line_Add_Ahead(&pnpfni->NPFNI_lineHash, ppline);
}
/*********************************************************************************************************
** 函数名称: __npfNetifDeleteHash
** 功能描述: 将指定的过滤器网络接口结构从 hash 表中移除
** 输　入  : pnpfni        过滤器网络接口结构
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  __npfNetifDeleteHash (__PNPF_NETIF_CB  pnpfni)
{
    REGISTER INT                    iIndex;
    REGISTER LW_LIST_LINE_HEADER   *ppline;

    iIndex = (pnpfni->NPFNI_cName[0]
           +  pnpfni->NPFNI_cName[1]
           +  pnpfni->NPFNI_cName[2])
           & __NPF_NETIF_HASH_MASK;

    ppline = &_G_plineNpfHash[iIndex];

    _List_Line_Del(&pnpfni->NPFNI_lineHash, ppline);
}
/*********************************************************************************************************
** 函数名称: __npfNetifCreate
** 功能描述: 创建一个过滤器网络接口结构 (如果存在则直接返回已有的)
** 输　入  : pcIfname         网络接口名
** 输　出  : 创建出的过滤器网络接口结构
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static __PNPF_NETIF_CB  __npfNetifCreate (CPCHAR  pcIfname)
{
    INT                    i;
    __PNPF_NETIF_CB        pnpfni;
    struct netif          *pnetif;

    pnpfni = __npfNetifFind(pcIfname);
    if (pnpfni == LW_NULL) {
        pnpfni =  (__PNPF_NETIF_CB)__SHEAP_ALLOC(sizeof(__NPF_NETIF_CB));
        if (pnpfni == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        lib_strlcpy(pnpfni->NPFNI_cName, pcIfname, IF_NAMESIZE);
        for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
            pnpfni->NPFNI_npfrnIn[i]  = LW_NULL;                        /*  没有任何规则                */
            pnpfni->NPFNI_npfrnOut[i] = LW_NULL;
        }
        __npfNetifInsertHash(pnpfni);
        
        pnetif = netif_find(pcIfname);
        if (pnetif) {
            pnetif->inner_fw_stat  = (void *)pnpfni;
            pnetif->inner_fw       = npf_netif_firewall;
            pnpfni->NPFNI_bAttached = LW_TRUE;
        
        } else {
            pnpfni->NPFNI_bAttached = LW_FALSE;
        }
    }

    return  (pnpfni);
}
/*********************************************************************************************************
** 函数名称: __npfNetifDelete
** 功能描述: 删除一个过滤器网络接口结构
** 输　入  : pnpfni        过滤器网络接口结构
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  __npfNetifDelete (__PNPF_NETIF_CB  pnpfni)
{
    __npfNetifDeleteHash(pnpfni);
    __SHEAP_FREE(pnpfni);
}
/*********************************************************************************************************
** 函数名称: __npfMacRuleCheck
** 功能描述: 检查 MAC 规则是否允许数据报通过
** 输　入  : pnpfni        过滤器网络接口
             bIn           IN or OUT
             pethhdr       以太包头
** 输　出  : 是否允许数据报通过
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  __npfMacRuleCheck (__PNPF_NETIF_CB  pnpfni, BOOL  bIn, struct eth_hdr *pethhdr)
{
    BOOL                bAllow = LW_TRUE;
    PLW_LIST_LINE       plineTemp;
    __PNPF_RULE_MAC     pnpfrm;
    
    plineTemp = bIn ? 
                pnpfni->NPFNI_npfrnIn[LWIP_NPF_RULE_MAC] : 
                pnpfni->NPFNI_npfrnOut[LWIP_NPF_RULE_MAC];

    while (plineTemp) {
        pnpfrm = _LIST_ENTRY(plineTemp, __NPF_RULE_MAC, NPFRM_lineManage);
        if (lib_memcmp(pnpfrm->NPFRM_ucMac,
                       pethhdr->src.addr,
                       ETHARP_HWADDR_LEN) == 0) {                       /*  比较 hwaddr                 */
            if (pnpfrm->NPFRM_bAllow) {
                return  (LW_TRUE);                                      /*  白名单, 立即放行            */
            }
            bAllow = LW_FALSE;                                          /*  禁止通过                    */
        }
        plineTemp = _list_line_get_next(plineTemp);
    }

    return  (bAllow);
}
/*********************************************************************************************************
** 函数名称: __npfIpRuleCheck
** 功能描述: 检查 IP 规则是否允许数据报通过
** 输　入  : pnpfni        过滤器网络接口
             bIn           IN or OUT
             piphdr        IP 包头
** 输　出  : 是否允许数据报通过
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  __npfIpRuleCheck (__PNPF_NETIF_CB  pnpfni, BOOL  bIn, struct ip_hdr *piphdr)
{
    BOOL                bAllow = LW_TRUE;
    PLW_LIST_LINE       plineTemp;
    __PNPF_RULE_IP      pnpfri;
    ip4_addr_t          ipaddrHbo;

    if (bIn) {
        plineTemp = pnpfni->NPFNI_npfrnIn[LWIP_NPF_RULE_IP];
        ipaddrHbo.addr = PP_NTOHL(piphdr->src.addr);
        
    } else {
        plineTemp = pnpfni->NPFNI_npfrnOut[LWIP_NPF_RULE_IP];
        ipaddrHbo.addr = PP_NTOHL(piphdr->dest.addr);
    }
    
    while (plineTemp) {
        pnpfri = _LIST_ENTRY(plineTemp, __NPF_RULE_IP, NPFRI_lineManage);
        if ((ipaddrHbo.addr >= pnpfri->NPFRI_ipaddrHboS.addr) &&
            (ipaddrHbo.addr <= pnpfri->NPFRI_ipaddrHboE.addr)) {
            if (pnpfri->NPFRI_bAllow) {
                return  (LW_TRUE);                                      /*  白名单, 立即放行            */
            }
            bAllow = LW_FALSE;                                          /*  禁止通过                    */
        }
        plineTemp = _list_line_get_next(plineTemp);
    }

    return  (bAllow);
}
/*********************************************************************************************************
** 函数名称: __npfUdpRuleCheck
** 功能描述: 检查 UDP 规则是否允许数据报通过
** 输　入  : pnpfni        过滤器网络接口
             bIn           IN or OUT
             piphdr        IP 包头
             pudphdr       UDP 包头
** 输　出  : 是否允许数据报通过
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  __npfUdpRuleCheck (__PNPF_NETIF_CB  pnpfni, BOOL  bIn, 
                                struct ip_hdr *piphdr, struct udp_hdr *pudphdr)
{
    BOOL                bAllow = LW_TRUE;
    PLW_LIST_LINE       plineTemp;
    __PNPF_RULE_UDP     pnpfru;
    ip4_addr_t          ipaddrHbo;
    u16_t               usPortHbo;
    
    if (bIn) {
        plineTemp = pnpfni->NPFNI_npfrnIn[LWIP_NPF_RULE_UDP];
        ipaddrHbo.addr = PP_NTOHL(piphdr->src.addr);
        usPortHbo      = PP_NTOHS(pudphdr->dest);
        
    } else {
        plineTemp = pnpfni->NPFNI_npfrnOut[LWIP_NPF_RULE_UDP];
        ipaddrHbo.addr = PP_NTOHL(piphdr->dest.addr);
        usPortHbo      = PP_NTOHS(pudphdr->dest);
    }

    while (plineTemp) {
        pnpfru = _LIST_ENTRY(plineTemp, __NPF_RULE_UDP, NPFRU_lineManage);
        if ((ipaddrHbo.addr >= pnpfru->NPFRU_ipaddrHboS.addr) &&
            (ipaddrHbo.addr <= pnpfru->NPFRU_ipaddrHboE.addr) &&
            (usPortHbo      >= pnpfru->NPFRU_usPortHboS)      &&
            (usPortHbo      <= pnpfru->NPFRU_usPortHboE)) {
            if (pnpfru->NPFRU_bAllow) {
                return  (LW_TRUE);                                      /*  白名单, 立即放行            */
            }
            bAllow = LW_FALSE;                                          /*  禁止通过                    */
        }
        plineTemp = _list_line_get_next(plineTemp);
    }

    return  (bAllow);
}
/*********************************************************************************************************
** 函数名称: __npfTcpRuleCheck
** 功能描述: 检查 TCP 规则是否允许数据报通过
** 输　入  : pnpfni        过滤器网络接口
             piphdr        IP 包头
             ptcphdr       TCP 包头
** 输　出  : 是否允许数据报通过
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static BOOL  __npfTcpRuleCheck (__PNPF_NETIF_CB  pnpfni, BOOL  bIn, 
                                struct ip_hdr *piphdr, struct tcp_hdr *ptcphdr)
{
    BOOL                bAllow = LW_TRUE;
    PLW_LIST_LINE       plineTemp;
    __PNPF_RULE_TCP     pnpfrt;
    ip4_addr_t          ipaddrHbo;
    u16_t               usPortHbo;
    
    if (bIn) {
        plineTemp = pnpfni->NPFNI_npfrnIn[LWIP_NPF_RULE_TCP];
        ipaddrHbo.addr = PP_NTOHL(piphdr->src.addr);
        usPortHbo      = PP_NTOHS(ptcphdr->dest);
        
    } else {
        plineTemp = pnpfni->NPFNI_npfrnOut[LWIP_NPF_RULE_TCP];
        ipaddrHbo.addr = PP_NTOHL(piphdr->dest.addr);
        usPortHbo      = PP_NTOHS(ptcphdr->dest);
    }

    while (plineTemp) {
        pnpfrt = _LIST_ENTRY(plineTemp, __NPF_RULE_TCP, NPFRT_lineManage);
        if ((ipaddrHbo.addr >= pnpfrt->NPFRT_ipaddrHboS.addr) &&
            (ipaddrHbo.addr <= pnpfrt->NPFRT_ipaddrHboE.addr) &&
            (usPortHbo      >= pnpfrt->NPFRT_usPortHboS)      &&
            (usPortHbo      <= pnpfrt->NPFRT_usPortHboE)) {
            if (pnpfrt->NPFRT_bAllow) {
                return  (LW_TRUE);                                      /*  白名单, 立即放行            */
            }
            bAllow = LW_FALSE;                                          /*  禁止通过                    */
        }
        plineTemp = _list_line_get_next(plineTemp);
    }

    return  (bAllow);
}
/*********************************************************************************************************
** 函数名称: npf_netif_firewall
** 功能描述: 过滤器检查, 符合条件的数据报将输入至协议栈
** 输　入  : pnetif        网络接口
**           p             数据包
** 输　出  : 是否被 eaten
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  npf_netif_firewall (struct netif *pnetif, struct pbuf *p)
{
    __PNPF_NETIF_CB     pnpfni;                                         /*  过滤器网络接口              */
    INT                 iOffset = 0;                                    /*  拷贝数据的起始偏移量        */

#if __NPF_NETIF_RULE_FAST > 0
    struct eth_hdr     *pethhdr;                                        /*  eth 头                      */
    struct ip_hdr      *piphdr;                                         /*  ip 头                       */
    struct udp_hdr     *pudphdr;                                        /*  udp 头                      */
    struct tcp_hdr     *ptcphdr;                                        /*  tcp 头                      */

    __NPF_LOCK_R();                                                     /*  锁定 NPF 表                 */
    pnpfni = __npfNetifGet(pnetif);                                     /*  过滤器网络接口              */
    if (pnpfni == LW_NULL) {                                            /*  没有找到对应的控制结构      */
        __NPF_PACKET_ALLOW_INC();
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */
        return  (0);
    }
    
    /*
     *  ethernet 过滤处理
     */
    if (pnetif->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET)) {    /*  以太网接口                  */
        if (p->len < sizeof(struct eth_hdr)) {
            goto    __allow_input;                                      /*  无法获取 eth hdr 允许输入   */
        }
        
        pethhdr = (struct eth_hdr *)((char *)p->payload + iOffset);
        if (__npfMacRuleCheck(pnpfni, LW_TRUE, pethhdr)) {              /*  开始检查相应的 MAC 过滤规则 */
            iOffset += sizeof(struct eth_hdr);                          /*  允许通过                    */
        
        } else {
            goto    __drop_input;
        }
    }
    
    /*
     *  ip 过滤处理
     */
    if (iOffset == sizeof(struct eth_hdr)) {                            /*  前面有 eth hdr 字段         */
        if (pethhdr->type == PP_HTONS(ETHTYPE_VLAN)) {
            struct eth_vlan_hdr *pvlanhdr;
            
            if (p->len < iOffset + sizeof(struct eth_vlan_hdr)) {
                goto    __allow_input;                                  /*  无法获取 vlan hdr 允许输入  */
            }
            
            pvlanhdr = (struct eth_vlan_hdr *)((char *)p->payload + iOffset);
            if (pvlanhdr->tpid != PP_HTONS(ETHTYPE_IP)) {
                goto    __allow_input;
            }
            iOffset += sizeof(struct eth_vlan_hdr);
        
        } else if (pethhdr->type != PP_HTONS(ETHTYPE_IP)) {
            goto    __allow_input;                                      /*  不是 ip 数据包, 放行        */
        }
    }
    
    piphdr = (struct ip_hdr *)((char *)p->payload + iOffset);
    if (IPH_V(piphdr) != 4) {                                           /*  非 ipv4 数据包              */
        goto    __allow_input;                                          /*  放行                        */
    }

    if (__npfIpRuleCheck(pnpfni, LW_TRUE, piphdr)) {                    /*  开始检查相应的 ip 过滤规则  */
        iOffset += (IPH_HL(piphdr) << 2);                               /*  允许通过                    */
    
    } else {
        goto    __drop_input;
    }
    
    if ((IPH_OFFSET(piphdr) & PP_HTONS(IP_OFFMASK)) != 0) {             /*  分片数据包                  */
        goto    __allow_input;
    }
    
    switch (IPH_PROTO(piphdr)) {                                        /*  检查 ip 数据报类型          */

    case IP_PROTO_UDP:                                                  /*  udp 过滤处理                */
    case IP_PROTO_UDPLITE:
        if (p->len < (iOffset + sizeof(struct udp_hdr))) {
            goto    __allow_input;
        }
        pudphdr = (struct udp_hdr *)((char *)p->payload + iOffset);
        if (__npfUdpRuleCheck(pnpfni, LW_TRUE, piphdr, pudphdr) == LW_FALSE) {
            goto    __drop_input;
        }
        break;

    case IP_PROTO_TCP:                                                  /*  tcp 过滤处理                */
        if (p->len < (iOffset + sizeof(struct tcp_hdr))) {
            goto    __allow_input;
        }
        ptcphdr = (struct tcp_hdr *)((char *)p->payload + iOffset);
        if (__npfTcpRuleCheck(pnpfni, LW_TRUE, piphdr, ptcphdr) == LW_FALSE) {
            goto    __drop_input;
        }
        break;

    default:
        break;
    }
    
#else                                                                   /*  __NPF_NETIF_RULE_FAST       */
    struct eth_hdr      ethhdrChk;                                      /*  eth 头                      */
    struct ip_hdr       iphdrChk;                                       /*  ip 头                       */
    struct udp_hdr      udphdrChk;                                      /*  udp 头                      */
    struct tcp_hdr      tcphdrChk;                                      /*  tcp 头                      */

    __NPF_LOCK_R();                                                     /*  锁定 NPF 表                 */
    pnpfni = __npfNetifGet();                                           /*  过滤器网络接口              */
    if (pnpfni == LW_NULL) {                                            /*  没有找到对应的控制结构      */
        __NPF_PACKET_ALLOW_INC();
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */
        return  (0);
    }

    /*
     *  ethernet 过滤处理
     */
    if (pnetif->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET)) {    /*  以太网接口                  */
        if (pbuf_copy_partial(p, (void *)&ethhdrChk,
                              sizeof(struct eth_hdr), (u16_t)iOffset) != sizeof(struct eth_hdr)) {
            goto    __allow_input;                                      /*  无法获取 eth hdr 允许输入   */
        }
        
        if (__npfMacRuleCheck(pnpfni, LW_TRUE, &ethhdrChk)) {           /*  开始检查相应的 MAC 过滤规则 */
            iOffset += sizeof(struct eth_hdr);                          /*  允许通过                    */
        
        } else {
            goto    __drop_input;
        }
    }

    /*
     *  ip 过滤处理
     */
    if (iOffset == sizeof(struct eth_hdr)) {                            /*  前面有 eth hdr 字段         */
        if (ethhdrChk.type == PP_HTONS(ETHTYPE_VLAN)) {
            struct eth_vlan_hdr vlanhdrChk;
            
            if (pbuf_copy_partial(p, (void *)&vlanhdrChk,
                                  sizeof(struct eth_vlan_hdr), (u16_t)iOffset) != 
                                  sizeof(struct eth_vlan_hdr)) {        /*  VLAN 包头                   */
                goto    __allow_input;
            }
            if (vlanhdrChk.tpid != PP_HTONS(ETHTYPE_IP)) {
                goto    __allow_input;
            }
            iOffset += sizeof(struct eth_vlan_hdr);
        
        } else if (ethhdrChk.type != PP_HTONS(ETHTYPE_IP)) {
            goto    __allow_input;                                      /*  不是 ip 数据包, 放行        */
        }
    }
    
    if (pbuf_copy_partial(p, (void *)&iphdrChk,
                          sizeof(struct ip_hdr), (u16_t)iOffset) != sizeof(struct ip_hdr)) {
        goto    __allow_input;                                          /*  无法获取 ip hdr 放行        */
    }
    
    if (IPH_V((&iphdrChk)) != 4) {                                      /*  非 ipv4 数据包              */
        goto    __allow_input;                                          /*  放行                        */
    }

    if (__npfIpRuleCheck(pnpfni, LW_TRUE, &iphdrChk)) {                 /*  开始检查相应的 ip 过滤规则  */
        iOffset += (IPH_HL((&iphdrChk)) * 4);                           /*  允许通过                    */
    
    } else {
        goto    __drop_input;
    }
    
    if ((IPH_OFFSET(&iphdrChk) & PP_HTONS(IP_OFFMASK)) != 0) {          /*  分片数据包                  */
        goto    __allow_input;
    }

    switch (IPH_PROTO((&iphdrChk))) {                                   /*  检查 ip 数据报类型          */

    case IP_PROTO_UDP:                                                  /*  udp 过滤处理                */
    case IP_PROTO_UDPLITE:
        if (pbuf_copy_partial(p, (void *)&udphdrChk,
                              sizeof(struct udp_hdr), (u16_t)iOffset) != sizeof(struct udp_hdr)) {
            goto    __allow_input;                                      /*  无法获取 udp hdr 放行       */
        }
        if (__npfUdpRuleCheck(pnpfni, LW_TRUE, &iphdrChk, &udphdrChk) == LW_FALSE) {
            goto    __drop_input;
        }
        break;

    case IP_PROTO_TCP:                                                  /*  tcp 过滤处理                */
        if (pbuf_copy_partial(p, (void *)&tcphdrChk,
                              sizeof(struct tcp_hdr), (u16_t)iOffset) != sizeof(struct tcp_hdr)) {
            goto    __allow_input;                                      /*  无法获取 tcp hdr 放行       */
        }
        if (__npfTcpRuleCheck(pnpfni, LW_TRUE, &iphdrChk, &tcphdrChk) == LW_FALSE) {
            goto    __drop_input;
        }
        break;

    default:
        break;
    }
#endif                                                                  /*  !__NPF_NETIF_RULE_FAST      */

__allow_input:
    __NPF_PACKET_ALLOW_INC();
    __NPF_UNLOCK();                                                     /*  解锁 NPF 表                 */
    return  (0);                                                        /*  允许输入                    */
    
__drop_input:
    __NPF_PACKET_DROP_INC();
    __NPF_UNLOCK();                                                     /*  解锁 NPF 表                 */
    pbuf_free(p);                                                       /*  释放数据包                  */
    return  (1);                                                        /*  eaten                       */
}
/*********************************************************************************************************
** 函数名称: npf_netif_attach
** 功能描述: 当网络接口添加时, 调用的 hook 函数
** 输　入  : pnetif       网络接口
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  npf_netif_attach (struct netif  *pnetif)
{
    CHAR                cIfname[IF_NAMESIZE];
    __PNPF_NETIF_CB     pnpfni;
    
    if (!_G_ulNpfLock) {
        return;
    }
    
    netif_get_name(pnetif, cIfname);
    
    __NPF_LOCK_R();                                                     /*  锁定 NPF 表                 */
    pnpfni = __npfNetifFind(cIfname);
    if (pnpfni == LW_NULL) {
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */
        return;
    }
    
    pnetif->inner_fw_stat   = (void *)pnpfni;
    pnetif->inner_fw        = npf_netif_firewall;
    pnpfni->NPFNI_bAttached = LW_TRUE;
    __NPF_UNLOCK();                                                     /*  解锁 NPF 表                 */
}
/*********************************************************************************************************
** 函数名称: npf_netif_detach
** 功能描述: 当网络接口删除时, 调用的 hook 函数
** 输　入  : pcNetifName       对应的网络接口名
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  npf_netif_detach (struct netif  *pnetif)
{
    INT                 i;
    __PNPF_NETIF_CB     pnpfni;

    __NPF_LOCK_W();                                                     /*  锁定 NPF 表                 */
    pnpfni = __npfNetifGet(pnetif);
    if (pnpfni == LW_NULL) {
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */
        return;
    }
    
    pnpfni->NPFNI_bAttached = LW_FALSE;
    pnetif->inner_fw_stat   = LW_NULL;
    pnetif->inner_fw        = LW_NULL;                                  /*  不再使用 npf 输入函数       */

    for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
        if (pnpfni->NPFNI_npfrnIn[i] || 
            pnpfni->NPFNI_npfrnOut[i]) {
            break;
        }
    }
    if (i >= __NPF_NETIF_RULE_MAX) {                                    /*  此网络接口控制结果没有规则  */
        __npfNetifDelete(pnpfni);
    }
    __NPF_UNLOCK();                                                     /*  解锁 NPF 表                 */
}
/*********************************************************************************************************
** 函数名称: API_INetNpfRuleAdd
** 功能描述: net packet filter 添加一条规则
** 输　入  : pcIfname          对应的网络接口名
**           iRule             对应的规则, MAC/IP/UDP/TCP/...
**           pucMac            禁止通信的 MAC 地址数组,
**           pcAddrStart       禁止通信 IP 地址起始, 为 IP 地址字符串, 格式为: ???.???.???.???
**           pcAddrEnd         禁止通信 IP 地址结束, 为 IP 地址字符串, 格式为: ???.???.???.???
**           usPortStart       禁止通信的本地起始端口号(网络字节序), 仅适用与 UDP/TCP 规则
**           usPortEnd         禁止通信的本地结束端口号(网络字节序), 仅适用与 UDP/TCP 规则
** 输　出  : 规则句柄
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
LW_API
PVOID  API_INetNpfRuleAdd (CPCHAR  pcIfname,
                           INT     iRule,
                           UINT8   pucMac[],
                           CPCHAR  pcAddrStart,
                           CPCHAR  pcAddrEnd,
                           UINT16  usPortStart,
                           UINT16  usPortEnd)
{
    return  (API_INetNpfRuleAddEx(pcIfname, iRule, LW_TRUE, LW_FALSE, pucMac, 
                                  pcAddrStart, pcAddrEnd, usPortStart, usPortEnd));
}
/*********************************************************************************************************
** 函数名称: API_INetNpfRuleAddEx
** 功能描述: net packet filter 添加一条规则
** 输　入  : pcIfname          对应的网络接口名
**           iRule             对应的规则, MAC/IP/UDP/TCP/...
**           bIn               TRUE: INPUT FALSE: OUTPUT
**           bAllow            TRUE: 允许 FALSE: 禁止
**           pucMac            通信的 MAC 地址数组,
**           pcAddrStart       通信 IP 地址起始, 为 IP 地址字符串, 格式为: ???.???.???.???
**           pcAddrEnd         通信 IP 地址结束, 为 IP 地址字符串, 格式为: ???.???.???.???
**           usPortStart       通信的本地起始端口号(网络字节序), 仅适用与 UDP/TCP 规则
**           usPortEnd         通信的本地结束端口号(网络字节序), 仅适用与 UDP/TCP 规则
** 输　出  : 规则句柄
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
LW_API
PVOID  API_INetNpfRuleAddEx (CPCHAR  pcIfname,
                             INT     iRule,
                             BOOL    bIn,
                             BOOL    bAllow,
                             UINT8   pucMac[],
                             CPCHAR  pcAddrStart,
                             CPCHAR  pcAddrEnd,
                             UINT16  usPortStart,
                             UINT16  usPortEnd)
{
    __PNPF_NETIF_CB       pnpfni;
    LW_LIST_LINE_HEADER  *pplineHeader;

    if (!pcIfname) {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }

    if (iRule == LWIP_NPF_RULE_MAC) {                                   /*  添加 MAC 规则               */
        __PNPF_RULE_MAC   pnpfrm;

        if (!pucMac) {
            _ErrorHandle(EINVAL);
            return  (LW_NULL);
        }

        pnpfrm = (__PNPF_RULE_MAC)__SHEAP_ALLOC(sizeof(__NPF_RULE_MAC));/*  创建规则                    */
        if (pnpfrm == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfrm->NPFRM_iRule  = iRule;
        pnpfrm->NPFRM_bAllow = bAllow;
        lib_memcpy(&pnpfrm->NPFRM_ucMac, pucMac, ETHARP_HWADDR_LEN);    /*  拷贝 6 字节                 */

        __NPF_LOCK_W();                                                 /*  锁定 NPF 表                 */
        pnpfni = __npfNetifCreate(pcIfname);                            /*  创建控制块                  */
        if (pnpfni == LW_NULL) {
            __NPF_UNLOCK();                                             /*  解锁 NPF 表                 */
            return  (LW_NULL);
        }
        pplineHeader = bIn ? 
                       &pnpfni->NPFNI_npfrnIn[iRule] :
                       &pnpfni->NPFNI_npfrnOut[iRule];
        _List_Line_Add_Ahead(&pnpfrm->NPFRM_lineManage, pplineHeader);
        _G_ulNpfCounter++;
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */

        return  ((PVOID)pnpfrm);

    } else if (iRule == LWIP_NPF_RULE_IP) {                             /*  添加 IP 规则                */
        __PNPF_RULE_IP      pnpfri;

        if (!pcAddrStart || !pcAddrEnd) {
            _ErrorHandle(EINVAL);
            return  (LW_NULL);
        }

        pnpfri = (__PNPF_RULE_IP)__SHEAP_ALLOC(sizeof(__NPF_RULE_IP));  /*  创建规则                    */
        if (pnpfri == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfri->NPFRI_iRule  = iRule;
        pnpfri->NPFRI_bAllow = bAllow;
        
        pnpfri->NPFRI_ipaddrHboS.addr = PP_NTOHL(ipaddr_addr(pcAddrStart));
        pnpfri->NPFRI_ipaddrHboE.addr = PP_NTOHL(ipaddr_addr(pcAddrEnd));

        __NPF_LOCK_W();                                                 /*  锁定 NPF 表                 */
        pnpfni = __npfNetifCreate(pcIfname);                            /*  创建控制块                  */
        if (pnpfni == LW_NULL) {
            __NPF_UNLOCK();                                             /*  解锁 NPF 表                 */
            return  (LW_NULL);
        }
        pplineHeader = bIn ? 
                       &pnpfni->NPFNI_npfrnIn[iRule] :
                       &pnpfni->NPFNI_npfrnOut[iRule];
        _List_Line_Add_Ahead(&pnpfri->NPFRI_lineManage, pplineHeader);
        _G_ulNpfCounter++;
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */

        return  ((PVOID)pnpfri);

    } else if (iRule == LWIP_NPF_RULE_UDP) {                            /*  添加 UDP 规则               */
        __PNPF_RULE_UDP   pnpfru;

        if (!pcAddrStart || !pcAddrEnd) {
            _ErrorHandle(EINVAL);
            return  (LW_NULL);
        }

        pnpfru = (__PNPF_RULE_UDP)__SHEAP_ALLOC(sizeof(__NPF_RULE_UDP));/*  创建规则                    */
        if (pnpfru == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfru->NPFRU_iRule  = iRule;
        pnpfru->NPFRU_bAllow = bAllow;
        
        pnpfru->NPFRU_ipaddrHboS.addr = PP_NTOHL(ipaddr_addr(pcAddrStart));
        pnpfru->NPFRU_ipaddrHboE.addr = PP_NTOHL(ipaddr_addr(pcAddrEnd));
        pnpfru->NPFRU_usPortHboS      = PP_NTOHS(usPortStart);
        pnpfru->NPFRU_usPortHboE      = PP_NTOHS(usPortEnd);

        __NPF_LOCK_W();                                                 /*  锁定 NPF 表                 */
        pnpfni = __npfNetifCreate(pcIfname);                            /*  创建控制块                  */
        if (pnpfni == LW_NULL) {
            __NPF_UNLOCK();                                             /*  解锁 NPF 表                 */
            return  (LW_NULL);
        }
        pplineHeader = bIn ? 
                       &pnpfni->NPFNI_npfrnIn[iRule] :
                       &pnpfni->NPFNI_npfrnOut[iRule];
        _List_Line_Add_Ahead(&pnpfru->NPFRU_lineManage, pplineHeader);
        _G_ulNpfCounter++;
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */

        return  ((PVOID)pnpfru);

    } else if (iRule == LWIP_NPF_RULE_TCP) {                            /*  添加 TCP 规则               */
        __PNPF_RULE_TCP   pnpfrt;

        if (!pcAddrStart || !pcAddrEnd) {
            _ErrorHandle(EINVAL);
            return  (LW_NULL);
        }

        pnpfrt = (__PNPF_RULE_TCP)__SHEAP_ALLOC(sizeof(__NPF_RULE_TCP));/*  创建规则                    */
        if (pnpfrt == LW_NULL) {
            _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
            _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
            return  (LW_NULL);
        }
        pnpfrt->NPFRT_iRule  = iRule;
        pnpfrt->NPFRT_bAllow = bAllow;
        
        pnpfrt->NPFRT_ipaddrHboS.addr = PP_NTOHL(ipaddr_addr(pcAddrStart));
        pnpfrt->NPFRT_ipaddrHboE.addr = PP_NTOHL(ipaddr_addr(pcAddrEnd));
        pnpfrt->NPFRT_usPortHboS      = PP_NTOHS(usPortStart);
        pnpfrt->NPFRT_usPortHboE      = PP_NTOHS(usPortEnd);

        __NPF_LOCK_W();                                                 /*  锁定 NPF 表                 */
        pnpfni = __npfNetifCreate(pcIfname);                            /*  创建控制块                  */
        if (pnpfni == LW_NULL) {
            __NPF_UNLOCK();                                             /*  解锁 NPF 表                 */
            return  (LW_NULL);
        }
        pplineHeader = bIn ? 
                       &pnpfni->NPFNI_npfrnIn[iRule] :
                       &pnpfni->NPFNI_npfrnOut[iRule];
        _List_Line_Add_Ahead(&pnpfrt->NPFRT_lineManage, pplineHeader);
        _G_ulNpfCounter++;
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */

        return  ((PVOID)pnpfrt);

    } else {
        _ErrorHandle(EINVAL);
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** 函数名称: API_INetNpfRuleDel
** 功能描述: net packet filter 删除一条规则
** 输　入  : pcIfname          对应的网络接口名
**           pvRule            规则句柄 (可以为 NULL, 为 NULL 时表示使用规则序列号)
**           iSeqNum           指定网络接口的规则序列号 (从 0 开始)
** 输　出  : ERROR_NONE or PX_ERROR
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
LW_API
INT  API_INetNpfRuleDel (CPCHAR  pcIfname,
                         PVOID   pvRule,
                         INT     iSeqNum)
{
    return  (API_INetNpfRuleDelEx(pcIfname, LW_TRUE, pvRule, iSeqNum));
}
/*********************************************************************************************************
** 函数名称: API_INetNpfRuleDelEx
** 功能描述: net packet filter 删除一条规则
** 输　入  : pcIfname          对应的网络接口名
**           bIn               TRUE: INPUT FALSE: OUTPUT
**           pvRule            规则句柄 (可以为 NULL, 为 NULL 时表示使用规则序列号)
**           iSeqNum           指定网络接口的规则序列号 (从 0 开始)
** 输　出  : ERROR_NONE or PX_ERROR
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
LW_API
INT  API_INetNpfRuleDelEx (CPCHAR  pcIfname,
                           BOOL    bIn,
                           PVOID   pvRule,
                           INT     iSeqNum)
{
    INT                    i;
    __PNPF_NETIF_CB        pnpfni;
    __PNPF_RULE_MAC        pnpfrm;

    if (!pcIfname) {
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    if (!pvRule && (iSeqNum < 0)) {                                     /*  这两个参数不能同时无效      */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    __NPF_LOCK_W();                                                     /*  锁定 NPF 表                 */
    pnpfni = __npfNetifFind(pcIfname);
    if (pnpfni == LW_NULL) {
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */
        _ErrorHandle(EINVAL);
        return  (PX_ERROR);
    }

    if (pvRule) {                                                       /*  通过句柄删除                */
        pnpfrm = _LIST_ENTRY(pvRule, __NPF_RULE_MAC, NPFRM_lineManage);
        if (bIn) {
            _List_Line_Del(&pnpfrm->NPFRM_lineManage, 
                           &pnpfni->NPFNI_npfrnIn[pnpfrm->NPFRM_iRule]);
        } else {
            _List_Line_Del(&pnpfrm->NPFRM_lineManage, 
                           &pnpfni->NPFNI_npfrnOut[pnpfrm->NPFRM_iRule]);
        }
        _G_ulNpfCounter--;
        
    } else {                                                            /*  通过序号删除                */
        PLW_LIST_LINE   plineTemp;

        for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
            plineTemp = bIn ? pnpfni->NPFNI_npfrnIn[i] : pnpfni->NPFNI_npfrnOut[i];
            while (plineTemp) {
                if (iSeqNum == 0) {
                    goto    __rule_find;
                }
                iSeqNum--;
                plineTemp = _list_line_get_next(plineTemp);
            }
        }

__rule_find:
        if (iSeqNum || (plineTemp == LW_NULL)) {
            __NPF_UNLOCK();                                             /*  解锁 NPF 表                 */
            _ErrorHandle(EINVAL);                                       /*  iSeqNum 参数错误            */
            return  (PX_ERROR);
        }
        if (bIn) {
            _List_Line_Del(plineTemp, &pnpfni->NPFNI_npfrnIn[i]);
        } else {
            _List_Line_Del(plineTemp, &pnpfni->NPFNI_npfrnOut[i]);
        }
        _G_ulNpfCounter--;
        pvRule = plineTemp;
    }

    /*
     *  如果 pnpfni 中没有任何规则, 同时也没有任何网卡连接. pnpfni 可以被删除.
     */
    if (!pnpfni->NPFNI_bAttached) {                                     /*  没有连接的网卡              */
        INT     i;

        for (i = 0; i < __NPF_NETIF_RULE_MAX; i++) {
            if (pnpfni->NPFNI_npfrnIn[i] ||
                pnpfni->NPFNI_npfrnOut[i]) {
                break;
            }
        }

        if (i >= __NPF_NETIF_RULE_MAX) {                                /*  此网络接口控制结果没有规则  */
            __npfNetifDelete(pnpfni);
        }
    }
    __NPF_UNLOCK();                                                     /*  解锁 NPF 表                 */

    __SHEAP_FREE(pvRule);                                               /*  释放内存                    */

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_INetNpfInit
** 功能描述: net packet filter 初始化
** 输　入  : NONE
** 输　出  : ERROR_NONE or PX_ERROR
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
LW_API
INT  API_INetNpfInit (VOID)
{
    static BOOL bIsInit = LW_FALSE;

    if (bIsInit) {
        return  (ERROR_NONE);
    }

    _G_ulNpfLock = API_SemaphoreRWCreate("sem_npflcok", LW_OPTION_DELETE_SAFE |
                                         LW_OPTION_WAIT_FIFO | LW_OPTION_OBJECT_GLOBAL, LW_NULL);
    if (_G_ulNpfLock == LW_OBJECT_HANDLE_INVALID) {
        return  (PX_ERROR);
    }

#if LW_CFG_SHELL_EN > 0
    /*
     *  加入 SHELL 命令.
     */
    API_TShellKeywordAdd("npfs", __tshellNetNpfShow);
    API_TShellHelpAdd("npfs",    "show net packet filter rule(s).\n");

    API_TShellKeywordAdd("npfruleadd", __tshellNetNpfRuleAdd);
    API_TShellFormatAdd("npfruleadd",  " [netifname] [rule] [input | output] [allow | deny] [args...]");
    API_TShellHelpAdd("npfruleadd",    "add a rule into net packet filter.\n"
                                       "eg. npfruleadd en1 mac input deny 11:22:33:44:55:66\n"
                                       "    npfruleadd en1 ip input allow 192.168.0.5 192.168.0.10\n"
                                       "    npfruleadd lo0 udp input deny 0.0.0.0 255.255.255.255 433 500\n"
                                       "    npfruleadd wl2 tcp input deny 192.168.0.1 192.168.0.200 169 169\n");

    API_TShellKeywordAdd("npfruledel", __tshellNetNpfRuleDel);
    API_TShellFormatAdd("npfruledel",  " [netifname] [input | output] [rule sequence num]");
    API_TShellHelpAdd("npfruledel",    "delete a rule from net packet filter.\n");
#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */

#if LW_CFG_PROCFS_EN > 0
    __procFsNpfInit();
#endif                                                                  /*  LW_CFG_PROCFS_EN > 0        */

    bIsInit = LW_TRUE;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_INetNpfDropGet
** 功能描述: net packet filter 丢弃的数据包个数 (包括规则性过滤和缓存不足造成的丢弃)
** 输　入  : NONE
** 输　出  : 丢弃的数据包个数
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
LW_API
ULONG  API_INetNpfDropGet (VOID)
{
    return  (__NPF_PACKET_DROP_GET());
}
/*********************************************************************************************************
** 函数名称: API_INetNpfAllowGet
** 功能描述: net packet filter 允许通过的数据包个数
** 输　入  : NONE
** 输　出  : 允许通过的数据包个数
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
LW_API
ULONG  API_INetNpfAllowGet (VOID)
{
    return  (__NPF_PACKET_ALLOW_GET());
}
/*********************************************************************************************************
** 函数名称: API_INetNpfShow
** 功能描述: net packet filter 显示当前所有的过滤条目
** 输　入  : iFd           打印目标文件描述符
** 输　出  : ERROR_NONE or PX_ERROR
** 全局变量:
** 调用模块:
                                           API 函数
*********************************************************************************************************/
LW_API
INT  API_INetNpfShow (INT  iFd)
{
    INT     iFilterFd;
    CHAR    cBuffer[512];
    ssize_t sstNum;
    
    iFilterFd = open("/proc/net/netfilter", O_RDONLY);
    if (iFilterFd < 0) {
        fprintf(stderr, "can not open /proc/net/netfilter : %s\n", lib_strerror(errno));
        return  (PX_ERROR);
    }
    
    do {
        sstNum = read(iFilterFd, cBuffer, sizeof(cBuffer));
        if (sstNum > 0) {
            write(iFd, cBuffer, (size_t)sstNum);
        }
    } while (sstNum > 0);
    
    close(iFilterFd);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __tshellNetNpfShow
** 功能描述: 系统命令 "npfs"
** 输　入  : iArgC         参数个数
**           ppcArgV       参数表
** 输　出  : 0
** 全局变量:
** 调用模块:
*********************************************************************************************************/
#if LW_CFG_SHELL_EN > 0

static INT  __tshellNetNpfShow (INT  iArgC, PCHAR  *ppcArgV)
{
    return  (API_INetNpfShow(STD_OUT));
}
/*********************************************************************************************************
** 函数名称: __tshellNetNpfRuleAdd
** 功能描述: 系统命令 "npfruleadd"
** 输　入  : iArgC         参数个数
**           ppcArgV       参数表
** 输　出  : 0
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  __tshellNetNpfRuleAdd (INT  iArgC, PCHAR  *ppcArgV)
{
#define __NPF_TSHELL_RADD_ARG_NETIF     1
#define __NPF_TSHELL_RADD_ARG_RULE      2
#define __NPF_TSHELL_RADD_ARG_IN        3
#define __NPF_TSHELL_RADD_ARG_ALLOW     4
#define __NPF_TSHELL_RADD_ARG_MAC       5
#define __NPF_TSHELL_RADD_ARG_IPS       5
#define __NPF_TSHELL_RADD_ARG_IPE       6
#define __NPF_TSHELL_RADD_ARG_PORTS     7
#define __NPF_TSHELL_RADD_ARG_PORTE     8
    PVOID    pvRule;
    BOOL     bIn;
    BOOL     bAllow;

    if (iArgC < 6) {
        fprintf(stderr, "arguments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    if (!lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_IN], "input")) {
        bIn = LW_TRUE;
    } else {
        bIn = LW_FALSE;
    }
    
    if (!lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_ALLOW], "allow")) {
        bAllow = LW_TRUE;
    } else {
        bAllow = LW_FALSE;
    }

    if (lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_RULE], "mac") == 0) {
        INT         i;
        UINT8       ucMac[NETIF_MAX_HWADDR_LEN];
        INT         iMac[NETIF_MAX_HWADDR_LEN];

        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_MAC], "%x:%x:%x:%x:%x:%x",
                   &iMac[0], &iMac[1], &iMac[2], &iMac[3], &iMac[4], &iMac[5]) != 6) {
            fprintf(stderr, "mac argument error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        for (i = 0; i < ETHARP_HWADDR_LEN; i++) {
            ucMac[i] = (UINT8)iMac[i];
        }
        pvRule = API_INetNpfRuleAddEx(ppcArgV[__NPF_TSHELL_RADD_ARG_NETIF],
                                      LWIP_NPF_RULE_MAC, bIn, bAllow,
                                      ucMac, LW_NULL, LW_NULL, 0, 0);
        if (pvRule == LW_NULL) {
            fprintf(stderr, "can not add mac rule, error: %s\n", lib_strerror(errno));
            return  (PX_ERROR);
        }

    } else if (lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_RULE], "ip") == 0) {
        if (iArgC != 7) {
            fprintf(stderr, "arguments error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        pvRule = API_INetNpfRuleAddEx(ppcArgV[__NPF_TSHELL_RADD_ARG_NETIF],
                                      LWIP_NPF_RULE_IP, bIn, bAllow,
                                      LW_NULL,
                                      ppcArgV[__NPF_TSHELL_RADD_ARG_IPS],
                                      ppcArgV[__NPF_TSHELL_RADD_ARG_IPE], 0, 0);
        if (pvRule == LW_NULL) {
            fprintf(stderr, "can not add ip rule, error: %s\n", lib_strerror(errno));
            return  (PX_ERROR);
        }

    } else if (lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_RULE], "udp") == 0) {
        INT     iPortS = -1;
        INT     iPortE = -1;

        if (iArgC != 9) {
            fprintf(stderr, "arguments error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_PORTS], "%i", &iPortS) != 1) {
            fprintf(stderr, "port argument error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_PORTE], "%i", &iPortE) != 1) {
            fprintf(stderr, "port argument error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        
        pvRule = API_INetNpfRuleAddEx(ppcArgV[__NPF_TSHELL_RADD_ARG_NETIF],
                                      LWIP_NPF_RULE_UDP, bIn, bAllow,
                                      LW_NULL,
                                      ppcArgV[__NPF_TSHELL_RADD_ARG_IPS],
                                      ppcArgV[__NPF_TSHELL_RADD_ARG_IPE],
                                      htons((u16_t)iPortS),
                                      htons((u16_t)iPortE));
        if (pvRule == LW_NULL) {
            fprintf(stderr, "can not add udp rule, error: %s\n", lib_strerror(errno));
            return  (PX_ERROR);
        }

    } else if (lib_strcmp(ppcArgV[__NPF_TSHELL_RADD_ARG_RULE], "tcp") == 0) {
        INT     iPortS = -1;
        INT     iPortE = -1;

        if (iArgC != 9) {
            fprintf(stderr, "arguments error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_PORTS], "%i", &iPortS) != 1) {
            fprintf(stderr, "port argument error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        if (sscanf(ppcArgV[__NPF_TSHELL_RADD_ARG_PORTE], "%i", &iPortE) != 1) {
            fprintf(stderr, "port argument error!\n");
            return  (-ERROR_TSHELL_EPARAM);
        }
        
        pvRule = API_INetNpfRuleAddEx(ppcArgV[__NPF_TSHELL_RADD_ARG_NETIF],
                                      LWIP_NPF_RULE_TCP, bIn, bAllow,
                                      LW_NULL,
                                      ppcArgV[__NPF_TSHELL_RADD_ARG_IPS],
                                      ppcArgV[__NPF_TSHELL_RADD_ARG_IPE],
                                      htons((u16_t)iPortS),
                                      htons((u16_t)iPortE));
        if (pvRule == LW_NULL) {
            fprintf(stderr, "can not add tcp rule, error: %s\n", lib_strerror(errno));
            return  (PX_ERROR);
        }

    } else {
        fprintf(stderr, "rule type argument error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __tshellNetNpfRuleDel
** 功能描述: 系统命令 "npfruledel"
** 输　入  : iArgC         参数个数
**           ppcArgV       参数表
** 输　出  : 0
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  __tshellNetNpfRuleDel (INT  iArgC, PCHAR  *ppcArgV)
{
    BOOL    bIn;
    INT     iError;
    INT     iSeqNum = -1;

    if (iArgC != 4) {
        fprintf(stderr, "arguments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }
    
    if (!lib_strcmp(ppcArgV[2], "input")) {
        bIn = LW_TRUE;
    } else {
        bIn = LW_FALSE;
    }

    if (sscanf(ppcArgV[3], "%i", &iSeqNum) != 1) {
        fprintf(stderr, "arguments error!\n");
        return  (-ERROR_TSHELL_EPARAM);
    }

    iError = API_INetNpfRuleDelEx(ppcArgV[1], bIn, LW_NULL, iSeqNum);
    if (iError) {
        if (errno == EINVAL) {
            fprintf(stderr, "arguments error!\n");
        } else {
            fprintf(stderr, "can not delete rule, error: %s\n", lib_strerror(errno));
        }
    }

    return  (iError);
}

#endif                                                                  /*  LW_CFG_SHELL_EN > 0         */
/*********************************************************************************************************
  /proc/net/netfilter
*********************************************************************************************************/
#if LW_CFG_PROCFS_EN > 0
/*********************************************************************************************************
  网络 proc 文件函数声明
*********************************************************************************************************/
static ssize_t  __procFsNetFilterRead(PLW_PROCFS_NODE  p_pfsn, 
                                      PCHAR            pcBuffer, 
                                      size_t           stMaxBytes,
                                      off_t            oft);
/*********************************************************************************************************
  网络 proc 文件操作函数组
*********************************************************************************************************/
static LW_PROCFS_NODE_OP    _G_pfsnoNetFilterFuncs = {
    __procFsNetFilterRead, LW_NULL
};
/*********************************************************************************************************
  网络 proc 文件节点
*********************************************************************************************************/
static LW_PROCFS_NODE       _G_pfsnNetFilter[] = 
{
    LW_PROCFS_INIT_NODE("netfilter", 
                        (S_IFREG | S_IRUSR | S_IRGRP | S_IROTH), 
                        &_G_pfsnoNetFilterFuncs, 
                        "F",
                        0),
};
/*********************************************************************************************************
** 函数名称: __procFsNetFilterPrint
** 功能描述: 打印网络 netfilter 文件
** 输　入  : bIn           TRUE: INPUT FALSE: OUTPUT
**           pcBuffer      缓冲
**           stTotalSize   缓冲区大小
**           pstOft        当前偏移量
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static VOID  __procFsNetFilterPrint (BOOL  bIn, PCHAR  pcBuffer, size_t  stTotalSize, size_t *pstOft)
{
    INT              i, iSeqNum;
    CHAR             cIpBuffer1[INET_ADDRSTRLEN];
    CHAR             cIpBuffer2[INET_ADDRSTRLEN];

    __PNPF_NETIF_CB  pnpfni;
    PLW_LIST_LINE    plineTempNpfni;
    PLW_LIST_LINE    plineTemp;
    
    for (i = 0; i < __NPF_NETIF_HASH_SIZE; i++) {
        for (plineTempNpfni  = _G_plineNpfHash[i];
             plineTempNpfni != LW_NULL;
             plineTempNpfni  = _list_line_get_next(plineTempNpfni)) {

            pnpfni  = _LIST_ENTRY(plineTempNpfni, __NPF_NETIF_CB, NPFNI_lineHash);
            iSeqNum = 0;

            plineTemp = bIn ? 
                        pnpfni->NPFNI_npfrnIn[LWIP_NPF_RULE_MAC] :
                        pnpfni->NPFNI_npfrnOut[LWIP_NPF_RULE_MAC];
            while (plineTemp) {
                __PNPF_RULE_MAC  pnpfrm;

                pnpfrm = _LIST_ENTRY(plineTemp, __NPF_RULE_MAC, NPFRM_lineManage);

                *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                         "%-5s %-6s %6d %-4s %-5s "
                         "%02x:%02x:%02x:%02x:%02x:%02x %-15s %-15s %-6s %-6s\n",
                         pnpfni->NPFNI_cName, (pnpfni->NPFNI_bAttached) ? "YES" : "NO",
                         iSeqNum, "MAC", (pnpfrm->NPFRM_bAllow) ? "YES" : "NO",
                         pnpfrm->NPFRM_ucMac[0],
                         pnpfrm->NPFRM_ucMac[1],
                         pnpfrm->NPFRM_ucMac[2],
                         pnpfrm->NPFRM_ucMac[3],
                         pnpfrm->NPFRM_ucMac[4],
                         pnpfrm->NPFRM_ucMac[5],
                         "N/A", "N/A", "N/A", "N/A");
                iSeqNum++;
                plineTemp = _list_line_get_next(plineTemp);
            }

            plineTemp = bIn ? 
                        pnpfni->NPFNI_npfrnIn[LWIP_NPF_RULE_IP] :
                        pnpfni->NPFNI_npfrnOut[LWIP_NPF_RULE_IP];
            while (plineTemp) {
                __PNPF_RULE_IP  pnpfri;
                ip4_addr_t      ipaddrS, ipaddrE;

                pnpfri = _LIST_ENTRY(plineTemp, __NPF_RULE_IP, NPFRI_lineManage);
                
                ipaddrS.addr = PP_HTONL(pnpfri->NPFRI_ipaddrHboS.addr);
                ipaddrE.addr = PP_HTONL(pnpfri->NPFRI_ipaddrHboE.addr);
                
                *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                         "%-5s %-6s %6d %-4s %-5s %-17s %-15s %-15s %-6s %-6s\n",
                         pnpfni->NPFNI_cName, (pnpfni->NPFNI_bAttached) ? "YES" : "NO",
                         iSeqNum, "IP", (pnpfri->NPFRI_bAllow) ? "YES" : "NO", "N/A",
                         ip4addr_ntoa_r(&ipaddrS, cIpBuffer1, INET_ADDRSTRLEN),
                         ip4addr_ntoa_r(&ipaddrE, cIpBuffer2, INET_ADDRSTRLEN),
                         "N/A", "N/A");
                iSeqNum++;
                plineTemp = _list_line_get_next(plineTemp);
            }

            plineTemp = bIn ? 
                        pnpfni->NPFNI_npfrnIn[LWIP_NPF_RULE_UDP] :
                        pnpfni->NPFNI_npfrnOut[LWIP_NPF_RULE_UDP];
            while (plineTemp) {
                __PNPF_RULE_UDP  pnpfru;
                ip4_addr_t       ipaddrS, ipaddrE;

                pnpfru = _LIST_ENTRY(plineTemp, __NPF_RULE_UDP, NPFRU_lineManage);
                
                ipaddrS.addr = PP_HTONL(pnpfru->NPFRU_ipaddrHboS.addr);
                ipaddrE.addr = PP_HTONL(pnpfru->NPFRU_ipaddrHboE.addr);

                *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                         "%-5s %-6s %6d %-4s %-5s %-17s %-15s %-15s %-6d %-6d\n",
                         pnpfni->NPFNI_cName, (pnpfni->NPFNI_bAttached) ? "YES" : "NO",
                         iSeqNum, "UDP", (pnpfru->NPFRU_bAllow) ? "YES" : "NO", "N/A",
                         ip4addr_ntoa_r(&ipaddrS, cIpBuffer1, INET_ADDRSTRLEN),
                         ip4addr_ntoa_r(&ipaddrE, cIpBuffer2, INET_ADDRSTRLEN),
                         pnpfru->NPFRU_usPortHboS,
                         pnpfru->NPFRU_usPortHboE);
                iSeqNum++;
                plineTemp = _list_line_get_next(plineTemp);
            }

            plineTemp = bIn ? 
                        pnpfni->NPFNI_npfrnIn[LWIP_NPF_RULE_TCP] :
                        pnpfni->NPFNI_npfrnOut[LWIP_NPF_RULE_TCP];
            while (plineTemp) {
                __PNPF_RULE_TCP  pnpfrt;
                ip4_addr_t       ipaddrS, ipaddrE;

                pnpfrt = _LIST_ENTRY(plineTemp, __NPF_RULE_TCP, NPFRT_lineManage);
                
                ipaddrS.addr = PP_HTONL(pnpfrt->NPFRT_ipaddrHboS.addr);
                ipaddrE.addr = PP_HTONL(pnpfrt->NPFRT_ipaddrHboE.addr);

                *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, 
                         "%-5s %-6s %6d %-4s %-5s %-17s %-15s %-15s %-6d %-6d\n",
                         pnpfni->NPFNI_cName, (pnpfni->NPFNI_bAttached) ? "YES" : "NO",
                         iSeqNum, "TCP", (pnpfrt->NPFRT_bAllow) ? "YES" : "NO", "N/A",
                         ip4addr_ntoa_r(&ipaddrS, cIpBuffer1, INET_ADDRSTRLEN),
                         ip4addr_ntoa_r(&ipaddrE, cIpBuffer2, INET_ADDRSTRLEN),
                         pnpfrt->NPFRT_usPortHboS,
                         pnpfrt->NPFRT_usPortHboE);
                iSeqNum++;
                plineTemp = _list_line_get_next(plineTemp);
            }
        }
    }
    
    *pstOft = bnprintf(pcBuffer, stTotalSize, *pstOft, "\n");
}
/*********************************************************************************************************
** 函数名称: __procFsNetFilterRead
** 功能描述: procfs 读一个读取网络 netfilter 文件
** 输　入  : p_pfsn        节点控制块
**           pcBuffer      缓冲区
**           stMaxBytes    缓冲区大小
**           oft           文件指针
** 输　出  : 实际读取的数目
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
static ssize_t  __procFsNetFilterRead (PLW_PROCFS_NODE  p_pfsn, 
                                       PCHAR            pcBuffer, 
                                       size_t           stMaxBytes,
                                       off_t            oft)
{
    const CHAR      cFilterInfoHdr[] = 
    "NETIF ATTACH SEQNUM RULE ALLOW MAC               IPs             IPe             PORTs  PORTe\n";
    
          PCHAR     pcFileBuffer;
          size_t    stRealSize;                                         /*  实际的文件内容大小          */
          size_t    stCopeBytes;
    
    /*
     *  由于预置内存大小为 0 , 所以打开后第一次读取需要手动开辟内存.
     */
    pcFileBuffer = (PCHAR)API_ProcFsNodeBuffer(p_pfsn);
    if (pcFileBuffer == LW_NULL) {                                      /*  还没有分配内存              */
        size_t  stNeedBufferSize = 0;
        
        __NPF_LOCK_R();                                                 /*  锁定 NPF 表                 */
        stNeedBufferSize = (size_t)(_G_ulNpfCounter * 128);
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */
        
        stNeedBufferSize += (sizeof(cFilterInfoHdr) * 2) + 64;
        
        if (API_ProcFsAllocNodeBuffer(p_pfsn, stNeedBufferSize)) {
            _ErrorHandle(ENOMEM);
            return  (0);
        }
        pcFileBuffer = (PCHAR)API_ProcFsNodeBuffer(p_pfsn);             /*  重新获得文件缓冲区地址      */
        
        stRealSize = bnprintf(pcFileBuffer, stNeedBufferSize, 0, "input >>\n\n");
        stRealSize = bnprintf(pcFileBuffer, stNeedBufferSize, stRealSize, cFilterInfoHdr);
                                                                        /*  打印头信息                  */
        __NPF_LOCK_R();                                                 /*  锁定 NPF 表                 */
        __procFsNetFilterPrint(LW_TRUE, pcFileBuffer, stNeedBufferSize, &stRealSize);
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */
        
        stRealSize = bnprintf(pcFileBuffer, stNeedBufferSize, stRealSize, "output >>\n\n");
        stRealSize = bnprintf(pcFileBuffer, stNeedBufferSize, stRealSize, cFilterInfoHdr);
                                                                        /*  打印头信息                  */
        __NPF_LOCK_R();                                                 /*  锁定 NPF 表                 */
        __procFsNetFilterPrint(LW_FALSE, pcFileBuffer, stNeedBufferSize, &stRealSize);
        __NPF_UNLOCK();                                                 /*  解锁 NPF 表                 */
        
        stRealSize = bnprintf(pcFileBuffer, stNeedBufferSize, stRealSize, 
                              "drop:%d  allow:%d\n", 
                              __NPF_PACKET_DROP_GET(), __NPF_PACKET_ALLOW_GET());
        
        API_ProcFsNodeSetRealFileSize(p_pfsn, stRealSize);
    } else {
        stRealSize = API_ProcFsNodeGetRealFileSize(p_pfsn);
    }
    if (oft >= stRealSize) {
        _ErrorHandle(ENOSPC);
        return  (0);
    }
    
    stCopeBytes  = __MIN(stMaxBytes, (size_t)(stRealSize - oft));       /*  计算实际拷贝的字节数        */
    lib_memcpy(pcBuffer, (CPVOID)(pcFileBuffer + oft), (UINT)stCopeBytes);
    
    return  ((ssize_t)stCopeBytes);
}
/*********************************************************************************************************
** 函数名称: __procFsNpfInit
** 功能描述: procfs 初始化网络 netfilter 文件
** 输　入  : NONE
** 输　出  : NONE
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  __procFsNpfInit (VOID)
{
    API_ProcFsMakeNode(&_G_pfsnNetFilter[0],  "/net");
}

#endif                                                                  /*  LW_CFG_PROCFS_EN > 0        */
#endif                                                                  /*  LW_CFG_NET_EN > 0           */
                                                                        /*  LW_CFG_NET_NPF_EN > 0       */
/*********************************************************************************************************
  END
*********************************************************************************************************/
