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
** 文   件   名: iso9660FsLib.c
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2018 年 07 月 18 日
**
** 描        述: ISO9660 文件系统内部函数.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
#include "../SylixOS/system/include/s_system.h"
/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if LW_CFG_MAX_VOLUMES > 0 && LW_CFG_ISO9660FS_EN > 0
#include "iso9660FsLib.h"
/*********************************************************************************************************
  ISO 9660 data structures fall into three main categories: the Volume Descriptors, the Directory
  Structures, and the Path Tables. These structures are interrelated as shown in figure 1. The Volume
  Descriptor tells where the directory structure and the Path Table are located, the directories tell us
  where the actual files are located, and the Path table gives us short cuts to each directory .
*********************************************************************************************************/
/*********************************************************************************************************
  ISO9660 standard config
*********************************************************************************************************/
#define ISO9660_LS_SIZE             2048                                /*  logic sector size           */
#define ISO9660_MAX_FILENAME_LENGTH 256
/*********************************************************************************************************
  ISO9660 The Volume Descriptors constants
*********************************************************************************************************/
#define ISO9660_PVD_BASE_LS         16                                  /*  Volume Descriptors sector   */
#define ISO9660_STD_ID              "CD001"                             /*  Standard Identifier         */
#define ISO9660_STD_ID_SIZE         5
#define ISO9660_VD_SIZE             2048
#define ISO9660_VD_VERSION          ((UINT8)1)
/*********************************************************************************************************
  ISO9660 The Volume Descriptors
  There are currently four types of Volume Descriptors defined in ISO 9660. Only one of these, the
  Primary Volume Descriptor, is commonly used. The other types are the Boot Record, the
  Supplementary Volume Descriptor, and the Volume Partition Descriptor. The Boot Record can be
  used for systems that must perform some type of initialization before the user can access the volume,
  although ISO 9660 does not specify what information must be in the Boot Record or how it is to be
  used. The Supplementary Volume Descriptor can be used to identify an alternate character set for
  use by systems that do not support the ISO 646 character set. The Volume Partition Descriptor can
  be used to logically divide the volume into smaller volume partitions, although ISO 9660 does not
  specify how to do this, only that it can be done The Volume Descriptors are recorded starting at
  Logical Sector 16
*********************************************************************************************************/
#define ISO9660_VD_BOOT             ((UINT8)0)
#define ISO9660_VD_PRIMARY          ((UINT8)1)
#define ISO9660_VD_SUPPLEM          ((UINT8)2)
#define ISO9660_VD_PARTN            ((UINT8)3)
#define ISO9660_VD_SETTERM          ((UINT8)255)
/*********************************************************************************************************
  The Primary Volume Descriptor
  +----------------------------------+
  |    Standard Identifier (CD001)   |
  +----------------------------------+
  |        Volume Identifier         |
  +----------------------------------+
  |      Volume Set Identifier       |
  +----------------------------------+
  |        System Identifier         |
  +----------------------------------+
  |           Volume Size            |
  +----------------------------------+
  |   Number of Volumes in this Set  |
  +----------------------------------+
  | Number of this Volume in the Set |
  +----------------------------------+
  |         Logical Block Size       |
  +----------------------------------+
  |      Size of the Path Table      |
  +----------------------------------+
  |    Location of the Path Table    |
  +----------------------------------+
  |      Root Directory Record       |
  +----------------------------------+
  |         Other Identifiers        |
  +----------------------------------+
  |            Time Stamps           |
  +----------------------------------+
*********************************************************************************************************/

#endif                                                                  /*  LW_CFG_MAX_VOLUMES > 0      */
                                                                        /*  LW_CFG_ISO9660FS_EN > 0     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
