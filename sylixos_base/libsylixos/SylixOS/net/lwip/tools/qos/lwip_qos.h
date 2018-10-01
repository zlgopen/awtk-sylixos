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
** 文   件   名: lwip_qos.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2018 年 08 月 06 日
**
** 描        述: QoS 用户可配置工具.
*********************************************************************************************************/

#ifndef __LWIP_QOS_H
#define __LWIP_QOS_H

/*********************************************************************************************************
  裁剪控制
*********************************************************************************************************/
#if (LW_CFG_NET_EN > 0) && (LW_CFG_NET_QOS_EN > 0)

#define LWIP_QOS_PRIO_LOWEST    0
#define LWIP_QOS_PRIO_HIGHEST   7

#define LWIP_QOS_RULE_IP        0
#define LWIP_QOS_RULE_UDP       1
#define LWIP_QOS_RULE_TCP       2

#define LWIP_QOS_CMP_METHOD_SRC     1
#define LWIP_QOS_CMP_METHOD_DEST    2
#define LWIP_QOS_CMP_METHOD_BOTH    (LWIP_QOS_CMP_METHOD_SRC | LWIP_QOS_CMP_METHOD_DEST)

LW_API INT      API_INetQosInit(VOID);
LW_API PVOID    API_INetQosRuleAdd(CPCHAR  pcIfname,
                                   INT     iRule,
                                   INT     iCmpMethod,
                                   CPCHAR  pcAddrStart,
                                   CPCHAR  pcAddrEnd,
                                   UINT16  usPortStart,
                                   UINT16  usPortEnd,
                                   UINT8   ucPrio,
                                   BOOL    bDontDrop);
LW_API INT      API_INetQosRuleDel(CPCHAR  pcIfname,
                                   PVOID   pvRule,
                                   INT     iSeqNum);
LW_API INT      API_INetQosShow(INT  iFd);

#define inetQosInit         API_INetQosInit
#define inetQosRuleAdd      API_INetQosRuleAdd
#define inetQosRuleDel      API_INetQosRuleDel
#define inetQosShow         API_INetQosShow

#endif                                                                  /*  LW_CFG_NET_EN > 0           */
                                                                        /*  LW_CFG_NET_QOS_EN > 0       */
#endif                                                                  /*  __LWIP_QOS_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
