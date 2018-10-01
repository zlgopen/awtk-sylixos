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
** 文   件   名: fsCommon.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2009 年 07 月 05 日
**
** 描        述: 文件系统接口公共服务部分(这个部分加入的晚了, 导致很多文件系统公共行为没有很好的封装
                                          在以后的版本中, 将会慢慢改进).
** BUG:
2009.10.28  Linkcounter 使用 atomic 操作.
2009.12.01  加入了文件系统注册函数. (不包含 yaffs 文件系统)
2011.03.06  修正 gcc 4.5.1 相关 warning.
2012.03.08  修正 __LW_FILE_ERROR_NAME_STR 字符串.
2012.04.01  加入权限判断.
2013.01.31  将权限判断加入 IO 系统.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if LW_CFG_MAX_VOLUMES > 0
#include "limits.h"
/*********************************************************************************************************
  文件名不能包含的字符 (特殊系统可由用户定义 __LW_FILE_ERROR_NAME_STR, 但是不推荐)
*********************************************************************************************************/
#ifndef __LW_FILE_ERROR_NAME_STR
#define __LW_FILE_ERROR_NAME_STR        "\\*?<>:\"|\t\r\n"              /*  不能包含在文件内的字符      */
#endif                                                                  /*  __LW_FILE_ERROR_NAME_STR    */
/*********************************************************************************************************
  文件系统名对应的文件系统装载函数 (不针对 yaffs 系统)
*********************************************************************************************************/
typedef struct {
    LW_LIST_LINE                 FSN_lineManage;                        /*  管理链表                    */
    FUNCPTR                      FSN_pfuncCreate;                       /*  文件系统创建函数            */
    FUNCPTR                      FSN_pfuncCheck;                        /*  文件系统检查函数            */
    FUNCPTR                      FSN_pfuncProb;                         /*  文件系统类型探测            */
    CHAR                         FSN_pcFsName[1];                       /*  文件系统名称                */
} __LW_FILE_SYSTEM_NODE;
typedef __LW_FILE_SYSTEM_NODE   *__PLW_FILE_SYSTEM_NODE;

static LW_SPINLOCK_DEFINE       (_G_slFsNode) = LW_SPIN_INITIALIZER;
static LW_LIST_LINE_HEADER       _G_plineFsNodeHeader = LW_NULL;        /*  文件系统入口表              */
/*********************************************************************************************************
** 函数名称: __fsRegister
** 功能描述: 注册一个文件系统
** 输　入  : pcName           文件系统名
**           pfuncCreate      文件系统创建函数
**           pfuncCheck       文件系统检查函数
** 输　出  : ERROR
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  __fsRegister (CPCHAR  pcName, FUNCPTR  pfuncCreate, FUNCPTR  pfuncCheck, FUNCPTR  pfuncProb)
{
    INTREG                  iregInterLevel;
    __PLW_FILE_SYSTEM_NODE  pfsnNew;

    if (!pcName || !pfuncCreate) {
        return  (PX_ERROR);
    }
    
    pfsnNew = (__PLW_FILE_SYSTEM_NODE)__SHEAP_ALLOC(lib_strlen(pcName) + 
                                                    sizeof(__LW_FILE_SYSTEM_NODE));
    if (pfsnNew == LW_NULL) {
        _DebugHandle(__ERRORMESSAGE_LEVEL, "system low memory.\r\n");
        _ErrorHandle(ERROR_SYSTEM_LOW_MEMORY);
        return  (PX_ERROR);
    }
    pfsnNew->FSN_pfuncCreate = pfuncCreate;
    pfsnNew->FSN_pfuncCheck  = pfuncCheck;
    pfsnNew->FSN_pfuncProb   = pfuncProb;
    lib_strcpy(pfsnNew->FSN_pcFsName, pcName);
    
    LW_SPIN_LOCK_QUICK(&_G_slFsNode, &iregInterLevel);
    _List_Line_Add_Ahead(&pfsnNew->FSN_lineManage, &_G_plineFsNodeHeader);
    LW_SPIN_UNLOCK_QUICK(&_G_slFsNode, iregInterLevel);
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __fsCreateFuncGet
** 功能描述: 获取文件系统创建函数
** 输　入  : pcName           文件系统名
**           pblkd            对应磁盘
**           ucPartType       对应分区类型
** 输　出  : 文件系统创建函数
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
FUNCPTR  __fsCreateFuncGet (CPCHAR   pcName, PLW_BLK_DEV  pblkd, UINT8  ucPartType)
{
    INTREG                  iregInterLevel;
    __PLW_FILE_SYSTEM_NODE  pfsnFind;
    PLW_LIST_LINE           plineTemp;

    if (!pcName) {
        return  (LW_NULL);
    }
    
    LW_SPIN_LOCK_QUICK(&_G_slFsNode, &iregInterLevel);
    plineTemp = _G_plineFsNodeHeader;
    LW_SPIN_UNLOCK_QUICK(&_G_slFsNode, iregInterLevel);
    
    while (plineTemp) {
        pfsnFind = (__PLW_FILE_SYSTEM_NODE)plineTemp;
        if (lib_strcmp(pfsnFind->FSN_pcFsName, pcName) == 0) {
            break;
        }
        plineTemp = _list_line_get_next(plineTemp);
    }
    
    if (plineTemp) {
        if (pfsnFind->FSN_pfuncCheck && pblkd) {
            if (pfsnFind->FSN_pfuncCheck(pblkd, ucPartType) < ERROR_NONE) {
                return  (LW_NULL);
            }
        }
        return  (pfsnFind->FSN_pfuncCreate);
    
    } else {
        return  (LW_NULL);
    }
}
/*********************************************************************************************************
** 函数名称: __fsPartitionProb
** 功能描述: 分区类型探测
** 输　入  : pblkd            对应磁盘
** 输　出  : 分区类型
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
UINT8  __fsPartitionProb (PLW_BLK_DEV  pblkd)
{
#ifndef LW_DISK_PART_TYPE_EMPTY
#define LW_DISK_PART_TYPE_EMPTY  0x00
#endif

    INTREG                  iregInterLevel;
    __PLW_FILE_SYSTEM_NODE  pfsnProb;
    PLW_LIST_LINE           plineTemp;
    UINT8                   ucPartType = LW_DISK_PART_TYPE_EMPTY;
    
    LW_SPIN_LOCK_QUICK(&_G_slFsNode, &iregInterLevel);
    plineTemp = _G_plineFsNodeHeader;
    LW_SPIN_UNLOCK_QUICK(&_G_slFsNode, iregInterLevel);
    
    while (plineTemp) {
        pfsnProb = (__PLW_FILE_SYSTEM_NODE)plineTemp;
        if (pfsnProb->FSN_pfuncProb) {
            pfsnProb->FSN_pfuncProb(pblkd, &ucPartType);
            if (ucPartType != LW_DISK_PART_TYPE_EMPTY) {
                break;
            }
        }
        plineTemp = _list_line_get_next(plineTemp);
    }
    
    return  (LW_DISK_PART_TYPE_EMPTY);
}
/*********************************************************************************************************
** 函数名称: __fsCheckFileName
** 功能描述: 检查文件名操作
** 输　入  : pcName           文件名
** 输　出  : ERROR
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
INT  __fsCheckFileName (CPCHAR  pcName)
{
    REGISTER PCHAR  pcTemp;
    
    /*
     *  不能建立 . 或 .. 文件
     */
    pcTemp = lib_rindex(pcName, PX_DIVIDER);
    if (pcTemp) {
        pcTemp++;
        if (*pcTemp == PX_EOS) {                                        /*  文件名长度为 0              */
            return  (PX_ERROR);
        }
        if ((lib_strcmp(pcTemp, ".")  == 0) ||
            (lib_strcmp(pcTemp, "..") == 0)) {                          /*  . , .. 检查                 */
            return  (PX_ERROR);
        }
    } else {
        if (pcName[0] == PX_EOS) {                                      /*  文件名长度为 0              */
            return  (PX_ERROR);
        }
    }
    
    /*
     *  不能包含非法字符
     */
    pcTemp = (PCHAR)pcName;
    for (; *pcTemp != PX_EOS; pcTemp++) {
        if (lib_strchr(__LW_FILE_ERROR_NAME_STR, *pcTemp)) {            /*  检查合法性                  */
            return  (PX_ERROR);
        }
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: __fsDiskLinkCounterAdd
** 功能描述: 将物理磁盘链接数量加1
** 输　入  : pblkd           块设备控制块
** 输　出  : ERROR
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  __fsDiskLinkCounterAdd (PLW_BLK_DEV  pblkd)
{
    INTREG       iregInterLevel;
    PLW_BLK_DEV  pblkdPhy;

    __LW_ATOMIC_LOCK(iregInterLevel);
    if (pblkd->BLKD_pvLink) {
        pblkdPhy = (PLW_BLK_DEV)pblkd->BLKD_pvLink;                     /*  获得物理设备连接            */
        pblkdPhy->BLKD_uiLinkCounter++;
        if (pblkdPhy->BLKD_uiLinkCounter == 1) {
            pblkdPhy->BLKD_uiPowerCounter = 0;
            pblkdPhy->BLKD_uiInitCounter  = 0;
        }
    } else {
        pblkd->BLKD_uiLinkCounter++;
    }
    __LW_ATOMIC_UNLOCK(iregInterLevel);
}
/*********************************************************************************************************
** 函数名称: __fsDiskLinkCounterDec
** 功能描述: 将物理磁盘链接数量减1
** 输　入  : pblkd           块设备控制块
** 输　出  : ERROR
** 全局变量: 
** 调用模块: 
*********************************************************************************************************/
VOID  __fsDiskLinkCounterDec (PLW_BLK_DEV  pblkd)
{
    INTREG       iregInterLevel;
    PLW_BLK_DEV  pblkdPhy;

    __LW_ATOMIC_LOCK(iregInterLevel);
    if (pblkd->BLKD_pvLink) {
        pblkdPhy = (PLW_BLK_DEV)pblkd->BLKD_pvLink;                     /*  获得物理设备连接            */
        pblkdPhy->BLKD_uiLinkCounter--;
    } else {
        pblkd->BLKD_uiLinkCounter--;
    }
    __LW_ATOMIC_UNLOCK(iregInterLevel);
}
/*********************************************************************************************************
** 函数名称: __fsDiskLinkCounterGet
** 功能描述: 获取物理磁盘链接数量
** 输　入  : pblkd           块设备控制块
** 输　出  : 物理磁盘连接数量
** 全局变量:
** 调用模块:
*********************************************************************************************************/
UINT  __fsDiskLinkCounterGet (PLW_BLK_DEV  pblkd)
{
    INTREG       iregInterLevel;
    PLW_BLK_DEV  pblkdPhy;
    UINT         uiRet;

    __LW_ATOMIC_LOCK(iregInterLevel);
    if (pblkd->BLKD_pvLink) {
        pblkdPhy = (PLW_BLK_DEV)pblkd->BLKD_pvLink;                     /*  获得物理设备连接            */
        uiRet    = pblkdPhy->BLKD_uiLinkCounter;
    } else {
        uiRet    = pblkd->BLKD_uiLinkCounter;
    }
    __LW_ATOMIC_UNLOCK(iregInterLevel);

    return  (uiRet);
}

#endif                                                                  /*  (LW_CFG_MAX_VOLUMES > 0)    */
/*********************************************************************************************************
  END
*********************************************************************************************************/
