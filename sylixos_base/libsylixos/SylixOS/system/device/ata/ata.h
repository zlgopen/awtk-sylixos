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
** 文   件   名: ata.h
**
** 创   建   人: Gong.YuJian (弓羽箭)
**
** 文件创建日期: 2018 年 09 月 03 日
**
** 描        述: ATA 驱动.
*********************************************************************************************************/

#ifndef __ATA_H
#define __ATA_H

/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_ATA_EN > 0)

#include "ataCfg.h"
/*********************************************************************************************************
  模式选项
*********************************************************************************************************/
#define ATA_PRI_NATIVE_EN                   0x1
#define ATA_PRI_NATIVE_CAP                  0x2
#define ATA_SEC_NATIVE_EN                   0x4
#define ATA_SEC_NATIVE_CAP                  0x8
/*********************************************************************************************************
  驱动配置选项 (ATA_GEO_CURRENT | ATA_DMA_AUTO | ATA_BITS_32)
*********************************************************************************************************/
#define ATA_MODE_MASK                       0x00FF                      /* transfer mode mask           */
#define ATA_GEO_MASK                        0x0300                      /* geometry mask                */
#define ATA_PIO_MASK                        0x3000                      /* RW PIO mask                  */
#define ATA_BITS_MASK                       0xc000                      /* RW bits size mask            */

#define ATA_PIO_DEF_W                       0x00                        /* PIO default trans mode       */
#define ATA_PIO_DEF_WO                      0x01                        /* PIO default trans no IORDY   */
#define ATA_PIO_W_0                         0x08                        /* PIO flow control mode 0      */
#define ATA_PIO_W_1                         0x09                        /* PIO flow control mode 1      */
#define ATA_PIO_W_2                         0x0a                        /* PIO flow control mode 2      */
#define ATA_PIO_W_3                         0x0b                        /* PIO flow control mode 3      */
#define ATA_PIO_W_4                         0x0c                        /* PIO flow control mode 4      */

#define ATA_PIO_DEF_0                       ATA_PIO_DEF_W               /* PIO default mode             */
#define ATA_PIO_DEF_1                       ATA_PIO_DEF_WO              /* PIO default mode, no IORDY   */
#define ATA_PIO_0                           ATA_PIO_W_0                 /* PIO mode 0                   */
#define ATA_PIO_1                           ATA_PIO_W_1                 /* PIO mode 1                   */
#define ATA_PIO_2                           ATA_PIO_W_2                 /* PIO mode 2                   */
#define ATA_PIO_3                           ATA_PIO_W_3                 /* PIO mode 3                   */
#define ATA_PIO_4                           ATA_PIO_W_4                 /* PIO mode 4                   */

#define ATA_PIO_AUTO                        0x000D                      /* PIO max supported mode       */
#define ATA_DMA_AUTO                        0x0046                      /* DMA max supported mode       */

#define ATA_DMA_SINGLE_0                    0x10                        /* singleword DMA mode 0        */
#define ATA_DMA_SINGLE_1                    0x11                        /* singleword DMA mode 1        */
#define ATA_DMA_SINGLE_2                    0x12                        /* singleword DMA mode 2        */

#define ATA_DMA_MULTI_0                     0x20                        /* multiword DMA mode 0         */
#define ATA_DMA_MULTI_1                     0x21                        /* multiword DMA mode 1         */
#define ATA_DMA_MULTI_2                     0x22                        /* multiword DMA mode 2         */

#define ATA_DMA_ULTRA_0                     0x40                        /* Ultra DMA mode 0             */
#define ATA_DMA_ULTRA_1                     0x41                        /* Ultra DMA mode 1             */
#define ATA_DMA_ULTRA_2                     0x42                        /* Ultra DMA mode 2             */
#define ATA_DMA_ULTRA_3                     0x43                        /* Ultra DMA mode 3             */
#define ATA_DMA_ULTRA_4                     0x44                        /* Ultra DMA mode 4             */
#define ATA_DMA_ULTRA_5                     0x45                        /* Ultra DMA mode 5             */
#define ATA_DMA_ULTRA_6                     0x46                        /* Ultra DMA mode 6             */

#define ATA_GEO_FORCE                       0x0100                      /* set geometry in the table    */
#define ATA_GEO_PHYSICAL                    0x0200                      /* set physical geometry        */
#define ATA_GEO_CURRENT                     0x0300                      /* set current geometry         */

#define ATA_PIO_SINGLE                      0x1000                      /* RW PIO single sector         */
#define ATA_PIO_MULTI                       0x2000                      /* RW PIO multi sector          */
/*********************************************************************************************************
  PIO 模式代码以及当前模式的偏移量
*********************************************************************************************************/
#define ATA_SET_PIO_MODE_0                  0x0
#define ATA_SET_PIO_MODE_1                  0x1
#define ATA_SET_PIO_MODE_2                  0x2
#define ATA_SET_PIO_MODE_3                  0x3
#define ATA_SET_PIO_MODE_4                  0x4
/*********************************************************************************************************
  DMA 模式代码以及当前模式的偏移量
*********************************************************************************************************/
#define ATA_SET_SDMA_MODE_0                 0x0
#define ATA_SET_SDMA_MODE_1                 0x1
#define ATA_SET_SDMA_MODE_2                 0x2

#define ATA_SET_MDMA_MODE_0                 0x0
#define ATA_SET_MDMA_MODE_1                 0x1
#define ATA_SET_MDMA_MODE_2                 0x2

#define ATA_SET_UDMA_MODE_0                 0x0
#define ATA_SET_UDMA_MODE_1                 0x1
#define ATA_SET_UDMA_MODE_2                 0x2
#define ATA_SET_UDMA_MODE_3                 0x3
#define ATA_SET_UDMA_MODE_4                 0x4
#define ATA_SET_UDMA_MODE_5                 0x5
/*********************************************************************************************************
  位图掩码
*********************************************************************************************************/
#define ATA_BIT_MASK0                       0x0001
#define ATA_BIT_MASK1                       0x0002
#define ATA_BIT_MASK2                       0x0004
#define ATA_BIT_MASK3                       0x0008
#define ATA_BIT_MASK4                       0x0010
#define ATA_BIT_MASK5                       0x0020
#define ATA_BIT_MASK6                       0x0040
#define ATA_BIT_MASK7                       0x0080
#define ATA_BIT_MASK8                       0x0100
#define ATA_BIT_MASK9                       0x0200
#define ATA_BIT_MASK10                      0x0400
#define ATA_BIT_MASK11                      0x0800
#define ATA_BIT_MASK12                      0x1000
#define ATA_BIT_MASK13                      0x2000
#define ATA_BIT_MASK14                      0x4000
#define ATA_BIT_MASK15                      0x8000
/*********************************************************************************************************
  数据位宽
*********************************************************************************************************/
#define ATA_BITS_16                         0x4000                      /* RW bits size, 16 bits        */
#define ATA_BITS_32                         0x8000                      /* RW bits size, 32 bits        */
/*********************************************************************************************************
  状态寄存器
*********************************************************************************************************/
#define ATA_STAT_BUSY                       0x80                        /* controller busy              */
#define ATA_STAT_READY                      0x40                        /* selected drive ready         */
#define ATA_STAT_DMAR                       0x20                        /* DMA Ready                    */
#define ATA_STAT_SERV                       0x10                        /* Service                      */
#define ATA_STAT_DRQ                        0x08                        /* Data Request                 */
#define ATA_STAT_ERR                        0x01                        /* Error Detect                 */
#define ATA_STAT_CHK                        0x01                        /* check                        */
/*********************************************************************************************************
  ATA 参数控制块
*********************************************************************************************************/
typedef struct ata_param_cb {
    UINT16          ATAPARAM_usConfig;                  /* [000] general configuration                  */
    UINT16          ATAPARAM_usCylinders;               /* [001] number of cylinders                    */
    UINT16          ATAPARAM_usRemovcyl;                /* [002] number of removable cylinders          */
    UINT16          ATAPARAM_usHeads;                   /* [003] number of heads                        */
    UINT16          ATAPARAM_usBytesTrack;              /* [004] number of unformatted bytes/track      */
    UINT16          ATAPARAM_usBytesSec;                /* [005] number of unformatted bytes/sector     */
    UINT16          ATAPARAM_usSectors;                 /* [006] number of sectors/track                */
    UINT16          ATAPARAM_usBytesGap;                /* [007] minimum bytes in intersector gap       */
    UINT16          ATAPARAM_usBytesSync;               /* [008] minimum bytes in sync field            */
    UINT16          ATAPARAM_usVendstat;                /* [009] number of words of vendor status       */
    UINT8           ATAPARAM_ucSerial[20];              /* [010] controller serial number               */
    UINT16          ATAPARAM_usType;                    /* [020] controller type                        */
    UINT16          ATAPARAM_usSize;                    /* [021] sector buffer size, in sectors         */
    UINT16          ATAPARAM_usBytesEcc;                /* [022] ecc bytes appended                     */
    UINT8           ATAPARAM_ucFwRev[8];                /* [023] firmware revision                      */
    UINT8           ATAPARAM_ucModel[40];               /* [027] model name                             */
    UINT16          ATAPARAM_usMultiSecs;               /* [047] RW multiple support bits 7-0 max secs  */
    UINT16          ATAPARAM_usDwordIo;                 /* [048] Dword IO                               */
    UINT16          ATAPARAM_usCapabilities;            /* [049] capabilities                           */
    UINT16          ATAPARAM_usCapabilities1;           /* [050] capabilities 1                         */
    UINT16          ATAPARAM_usPioMode;                 /* [051] PIO data transfer cycle timing mode    */
    UINT16          ATAPARAM_usDmaMode;                 /* [052] single word DMA transfer cycle timing  */
    UINT16          ATAPARAM_usValid;                   /* [053] field validity                         */
    UINT16          ATAPARAM_usCurrentCylinders;        /* [054] number of current logical cylinders    */
    UINT16          ATAPARAM_usCurrentHeads;            /* [055] number of current logical heads        */
    UINT16          ATAPARAM_usCurrentSectors;          /* [056] number of current logical sectors/track*/
    UINT16          ATAPARAM_usCapacity0;               /* [057] current capacity in sectors            */
    UINT16          ATAPARAM_usCapacity1;               /* [058] current capacity in sectors            */
    UINT16          ATAPARAM_usMultiSet;                /* [059] multiple sector setting                */
    UINT16          ATAPARAM_usSectors0;                /* [060] total number of user addressable sector*/
    UINT16          ATAPARAM_usSectors1;                /* [061] total number of user addressable sector*/
    UINT16          ATAPARAM_usSingleDma;               /* [062] single word DMA transfer               */
    UINT16          ATAPARAM_usMultiDma;                /* [063] multi word DMA transfer                */
    UINT16          ATAPARAM_usAdvancedPio;             /* [064] flow control PIO modes supported       */
    UINT16          ATAPARAM_usCycletimeDma;            /* [065] minimum multi DMA transfer cycle time  */
    UINT16          ATAPARAM_usCycletimeMulti;          /* [066] recommended multiword DMA cycle time   */
    UINT16          ATAPARAM_usCycletimePioNoIordy;     /* [067] min PIO transfer cycle time wo flow ctl*/
    UINT16          ATAPARAM_usCycletimePioIordy;       /* [068] min PIO transfer cycle time w IORDY    */
    UINT16          ATAPARAM_usAdditionalSupported;     /* [069] Additional Supported                   */
    UINT16          ATAPARAM_usReserved70;              /* [070] reserved                               */
    UINT16          ATAPARAM_usPktCmdRelTime;           /* [071] Typical Time for Release after Packet  */
    UINT16          ATAPARAM_usServCmdRelTime;          /* [072] Typical Time for Release after SERVICE */
    UINT16          ATAPARAM_usReservedPacketID[2];     /* [073] reserved for packet id                 */
    UINT16          ATAPARAM_usQueueDepth;              /* [075] maximum queue depth - 1                */
    UINT16          ATAPARAM_usSataCapabilities;        /* [076] SATA capabilities                      */
    UINT16          ATAPARAM_usSataAddCapabilities;     /* [077] SATA Additional Capabilities           */
    UINT16          ATAPARAM_usSataFeaturesSupported;   /* [078] SATA features Supported                */
    UINT16          ATAPARAM_usSataFeaturesEnabled;     /* [079] SATA features Enabled                  */
    UINT16          ATAPARAM_usMajorRevNum;             /* [080] Major ata version number               */
    UINT16          ATAPARAM_usMinorVersNum;            /* [081] minor version number                   */
    UINT16          ATAPARAM_usFeaturesSupported0;      /* [082] Features Supported word 0              */
    UINT16          ATAPARAM_usFeaturesSupported1;      /* [083] Features Supported word 1              */
    UINT16          ATAPARAM_usFeaturesSupported2;      /* [084] Features Supported word 2              */
    UINT16          ATAPARAM_usFeaturesEnabled0;        /* [085] Features Enabled word 0                */
    UINT16          ATAPARAM_usFeaturesEnabled1;        /* [086] Features Enabled word 1                */
    UINT16          ATAPARAM_usFeaturesEnabled2;        /* [087] Features Enabled word 2                */
    UINT16          ATAPARAM_usUltraDma;                /* [088] ultra DMA transfer                     */
    UINT16          ATAPARAM_usTimeErase;               /* [089] time to perform security erase         */
    UINT16          ATAPARAM_usTimeEnhancedErase;       /* [090] time to perform enhanced security erase*/
    UINT16          ATAPARAM_usCurrentAPM;              /* [091] current power management value         */
    UINT16          ATAPARAM_usPasswordRevisionCode;    /* [092] master password revision code          */
    UINT16          ATAPARAM_usResetResult;             /* [093] result of last reset                   */
    UINT16          ATAPARAM_usAcousticManagement;      /* [094] recommended acoustic management        */
    UINT16          ATAPARAM_usReserved95[5];           /* [095] reserved                               */
    UINT16          ATAPARAM_usLba48Size[4];            /* [100] 48-bit LBA size (stored little endian) */
    UINT16          ATAPARAM_usStreamingTimePio;        /* [104] Streaming Transfer Time PIO            */
    UINT16          ATAPARAM_usTrimBlockNumMax;         /* [105] Max number blocks DATA SET MANAGEMENT  */
    UINT16          ATAPARAM_usPhysicalLogicalSector;   /* [106] Physical / logical sector size         */
    UINT16          ATAPARAM_usReserved107[10];         /* [107] reserved                               */
    UINT16          ATAPARAM_usLogicSectorSize[2];      /* [117] Logical Sector Size                    */
    UINT16          ATAPARAM_usReserved119[8];          /* [119] reserved                               */
    UINT16          ATAPARAM_usRemovableFeatureSupport; /* [127] removable media status notification    */
    UINT16          ATAPARAM_usSecurityStatus;          /* [128] security status                        */
    UINT16          ATAPARAM_usVendor[31];              /* [129] vendor specific                        */
    UINT16          ATAPARAM_usCfaPowerMode;            /* [160]                                        */
    UINT16          ATAPARAM_usReserved161[8];          /* [161] reserved assignment by Compact Flash   */
    UINT16          ATAPARAM_usDataSetManagement;       /* [169] DATA SET MANAGEMENT command support    */
    UINT16          ATAPARAM_usReserved170[6];          /* [170] reserved                               */
    UINT16          ATAPARAM_usCurrentMediaSN[30];      /* [176] Current Media Serial Number            */
    UINT16          ATAPARAM_usReserved206[49];         /* reserved                                     */
    UINT16          ATAPARAM_usChecksum;                /* integrity word                               */
} ATA_PARAM_CB;

typedef ATA_PARAM_CB       *ATA_PARAM_HANDLE;
/*********************************************************************************************************
  ATA 参数控制块
*********************************************************************************************************/
typedef enum {
    IN_DATA  = O_RDONLY,                                                /* from drive to memory         */
    OUT_DATA = O_WRONLY,                                                /* to drive from memory         */
    NON_DATA                                                            /* non data command             */
} ATA_DATA_DIR;
/*********************************************************************************************************
  ATA 驱动器信息控制块
*********************************************************************************************************/
typedef struct ata_drive_info_cb {
    UINT32                  ATADINFO_uiCylinders;                       /* 柱面数量                     */
    UINT32                  ATADINFO_uiHeads;                           /* 磁头数量                     */
    UINT32                  ATADINFO_uiSectors;                         /* 扇区数量                     */
    UINT32                  ATADINFO_uiBytes;                           /* 每个扇区的字节大小           */
    UINT32                  ATADINFO_uiPrecomp;                         /* 补充柱面数量                 */

    /*
     *  磁盘缓冲参数配置
     */
    size_t                  ATADINFO_stCacheMemSize;                    /* 磁盘缓冲大小                 */
    INT                     ATADINFO_iBurstRead;                        /* 读操作的扇区猝发数           */
    INT                     ATADINFO_iBurstWrite;                       /* 写操作的扇区猝发数           */
    INT                     ATADINFO_iPipeline;                         /* 用于并行加速的管线数量       */
    INT                     ATADINFO_iMsgCount;                         /* 管线消息队列缓冲数量         */

    PVOID                   ATADINFO_pvReserved[4];                     /* 保留区                       */
} ATA_DRIVE_INFO_CB;

typedef ATA_DRIVE_INFO_CB      *ATA_DRIVE_INFO_HANDLE;
typedef ATA_DRIVE_INFO_CB       ATAPI_DRIVE_INFO_CB;
typedef ATAPI_DRIVE_INFO_CB    *ATAPI_DRIVE_INFO_HANDLE;
/*********************************************************************************************************
  ATA 驱动器状态
*********************************************************************************************************/
#define ATA_DEV_OK                          (0x00)                      /* 驱动器正常                   */
#define ATA_DEV_NONE                        (0x01)                      /* 驱动器不在线或响应异常       */
#define ATA_DEV_DIAG_ERR                    (0x02)                      /* 驱动器诊断失败               */
#define ATA_DEV_PREAD_ERR                   (0x03)                      /* 驱动器读取参数失败           */
#define ATA_DEV_MED_CH                      (0x04)                      /* 驱动器状态改变               */
#define ATA_DEV_INIT                        (-1)                        /* 驱动器未初始化               */
/*********************************************************************************************************
  ATA 设备类型
*********************************************************************************************************/
#define ATA_TYPE_NONE                       (0x00)                      /* 驱动器不在线或响应异常       */
#define ATA_TYPE_ATA                        (0x01)                      /* ATA 设备                     */
#define ATA_TYPE_ATAPI                      (0x02)                      /* ATAPI 设备                   */
#define ATA_TYPE_INIT                       (-1)                        /* 设备未初始化                 */
/*********************************************************************************************************
  ATA 驱动器控制块
*********************************************************************************************************/
typedef struct ata_drive_cb {
    ATA_DRIVE_INFO_HANDLE   ATADRIVE_hInfo;                             /* 驱动器信息                   */
    INT                     ATADRIVE_iState;                            /* 驱动器状态                   */

    UINT32                  ATADRIVE_uiSignature;
    INT                     ATADRIVE_iType;                             /* 设备类型 ATA/ATAPI/NONE      */
    INT                     ATADRIVE_iDiagCode;                         /* 诊断码                       */

    FUNCPTR                 ATADRIVE_pfuncDevReset;                     /* 驱动器复位                   */
    ATA_PARAM_CB            ATADRIVE_tParam;                            /* 驱动器参数                   */

    INT                     ATADRIVE_iDriveType;                        /* 驱动器类型 HDD/CD-ROM/CD-R   */

    INT                     ATADRIVE_iDmaUse;                           /* 驱动器是否使用 DMA           */
    INT                     ATADRIVE_iOkMulti;                          /* 是否支持 MULTI               */
    INT                     ATADRIVE_iOkIoRdy;                          /* 是否支持 IORDY               */
    INT                     ATADRIVE_iOkDma;                            /* 是否支持 DMA                 */
    INT                     ATADRIVE_iOkInterleavedDMA;
    INT                     ATADRIVE_iOkCommandQue;
    INT                     ATADRIVE_iOkOverlap;
    INT                     ATADRIVE_iOkRemovable;                      /* 是否支持移除                 */
    INT                     ATADRIVE_iSupportSmart;                     /* 是否支持 SMART               */
    INT                     ATADRIVE_iUseLba48;                         /* 是否使用 48 位逻辑地址       */
    UINT64                  ATADRIVE_ullCapacity;                       /* 驱动器最大能力               */
    UINT16                  ATADRIVE_usMultiSecs;                       /* 多扇区支持能力               */
    UINT16                  ATADRIVE_usPioMode;                         /* PIO 模式支持能力             */
    UINT16                  ATADRIVE_usSingleDmaMode;                   /* 单 DMA 支持                  */
    UINT16                  ATADRIVE_usMultiDmaMode;                    /* 多 DMA 支持                  */
    UINT16                  ATADRIVE_usUltraDmaMode;                    /* 增强 DMA 支持                */
    UINT16                  ATADRIVE_usRwMode;                          /* 读写模式                     */
    UINT16                  ATADRIVE_usRwBits;                          /* 16 或 32 位读写              */
    UINT16                  ATADRIVE_usRwPio;                           /* PIO 读写                     */
    UINT8                   ATADRIVE_ucCheckPower;                      /* 电源状态                     */
    UINT8                   ATADRIVE_ucOkLba;                           /* 是否支持 LBA                 */

    UINT8                   ATADRIVE_ucCmdLength;                       /* 12 或 16 字节命令长度        */

    INT                     ATADRIVE_iOkPEJ;
    INT                     ATADRIVE_iOkLock;
    UINT16                  ATADRIVE_usMediaStatusNotifyVer;
    UINT16                  ATADRIVE_usNativeMaxAdd[4];
    UINT16                  ATADRIVE_usCfaErrCode;

    struct ata_dev_cb      *ATADRIVE_hDev;                              /* 设备句柄                     */
    struct ata_ctrl_cb     *ATADRIVE_hCtrl;                             /* 控制器句柄                   */

    PVOID                   ATADRIVE_pvReserved[4];
} ATA_DRIVE_CB;

typedef ATA_DRIVE_CB       *ATA_DRIVE_HANDLE;
/*********************************************************************************************************
  ATA IO 地址控制块
*********************************************************************************************************/
typedef struct ata_io_resource_cb {
    addr_t      ATAIO_addrBase;                                         /* 资源基地址                   */
    PVOID       ATAIO_pvHandle;                                         /* 资源扩展参数                 */
} ATA_IO_RESOURCE_CB;

typedef ATA_IO_RESOURCE_CB     *ATA_IO_RESOURCE_HANDLE;
/*********************************************************************************************************
  ATA 寄存器控制块
*********************************************************************************************************/
typedef struct ata_reg_cb {
    ATA_IO_RESOURCE_CB      ATAREG_tCommand;                            /* 命令寄存器资源               */
    ATA_IO_RESOURCE_CB      ATAREG_tControl;                            /* 控制寄存器资源               */
    ATA_IO_RESOURCE_CB      ATAREG_tBusMaster;                          /* DMA 寄存器资源               */

    PVOID                   ATAREG_pvReserved[4];                       /* 保留区                       */
} ATA_REG_CB;

typedef ATA_REG_CB     *ATA_REG_HANDLE;
/*********************************************************************************************************
  ATA 控制器控制块
*********************************************************************************************************/
typedef struct ata_ctrl_cb {
    UINT                    ATACTRL_uiCtrl;                             /* 控制器索引                   */
    UINT32                  ATACTRL_uiConfigType;                       /* 来源于驱动的配置             */
    INT                     ATACTRL_iDrvInstalled;                      /* 是否已经安装驱动             */

    UINT                    ATACTRL_uiDriveNum;                         /* 驱动器数量                   */
    ATA_DRIVE_CB            ATACTRL_tDrive[ATA_DRIVE_MAX];              /* 驱动器句柄 (ATA_MAX_DRIVES)  */

    FUNCPTR                 ATACTRL_pfuncDelay;                         /* 短延时 驱动可指定此处理      */
    ATA_REG_CB              ATACTRL_tAtaReg;                            /* ATA 寄存器 驱动需初始化基地址*/
#define ATA_CTRL_CMD_BASE           ATACTRL_tAtaReg.ATAREG_tCommand.ATAIO_addrBase
#define ATA_CTRL_CTRL_BASE          ATACTRL_tAtaReg.ATAREG_tControl.ATAIO_addrBase
#define ATA_CTRL_BUSM_BASE          ATACTRL_tAtaReg.ATAREG_tBusMaster.ATAIO_addrBase
#define ATA_CTRL_CMD_HANDLE         ATACTRL_tAtaReg.ATAREG_tCommand.ATAIO_pvHandle
#define ATA_CTRL_CTRL_HANDLE        ATACTRL_tAtaReg.ATAREG_tControl.ATAIO_pvHandle
#define ATA_CTRL_BUSM_HANDLE        ATACTRL_tAtaReg.ATAREG_tBusMaster.ATAIO_pvHandle

    ULONG                   ATACTRL_ulCtlrIntVector;                    /* 中断向量                     */
    PINT_SVR_ROUTINE        ATACTRL_pfuncCtlrIntServ;                   /* 控制器中断服务               */
    FUNCPTR                 ATACTRL_pfuncCtrlIntPre;                    /* 控制器中断前期工作           */
    FUNCPTR                 ATACTRL_pfuncCtrlIntPost;                   /* 控制器中断后期工作           */

    INT                     ATACTRL_iIntStatus;                         /* 中断状态                     */
    ULONG                   ATACTRL_ulIntCount;                         /* 中断计数                     */
    INT                     ATACTRL_iIntErrStatus;                      /* 中断错误状态                 */
    ULONG                   ATACTRL_ulIntErrCount;                      /* 中断错误计数                 */

    LW_HANDLE               ATACTRL_hSemDev;                            /* 设备锁                       */
    LW_HANDLE               ATACTRL_hSemSync;                           /* 同步信号量                   */
    ULONG                   ATACTRL_ulWdgTimeout;                       /* 定时器超时时间               */
    ULONG                   ATACTRL_ulSemSyncTimeout;                   /* 同步异常超时时间 轮询以秒单位*/
    ULONG                   ATACTRL_ulSyncTimeoutLoop;                  /* 同步信号量超时重试次数       */

    INT                     ATACTRL_iPwrDown;                           /* 下电模式                     */
    INT                     ATACTRL_iCtrlType;                          /* 控制器类型                   */
    INT                     ATACTRL_iChanged;                           /* 是否支持热插拔               */
    INT                     ATACTRL_iUdmaCableOk;                       /* UDMA 能力                    */

    struct ata_drv_cb      *ATACTRL_hCtrlDrv;                           /* 控制器驱动句柄               */
    spinlock_t              ATACTRL_slSpinLock;                         /* 自旋锁                       */

    PVOID                   ATACTRL_pvReserved[8];                      /* 保留区                       */
} ATA_CTRL_CB;

typedef ATA_CTRL_CB    *ATA_CTRL_HANDLE;
/*********************************************************************************************************
  ATA 控制器配置控制块
  ATACTRLCFG_uiConfigType
    (ATA_GEO_CURRENT | ATA_PIO_MULTI | ATA_BITS_16)
    (ATA_GEO_CURRENT | ATA_DMA_AUTO  | ATA_BITS_32)
*********************************************************************************************************/
typedef struct ata_ctrl_cfg_cb {
    UINT8                   ATACTRLCFG_ucDrives;                        /* 驱动器最大数量               */
    UINT32                  ATACTRLCFG_uiConfigType;                    /* 工作模式配置                 */
    ULONG                   ATACTRLCFG_ulWdgTimeout;                    /* 定时器超时时间               */
    ULONG                   ATACTRLCFG_ulSyncTimeout;                   /* 同步异常超时时间 轮询以秒单位*/
    ULONG                   ATACTRLCFG_ulSyncTimeoutLoop;               /* 同步信号量超时重试次数       */

    /*
     *  同步给指定控制器的控制块
     */
    FUNCPTR                 ATACTRLCFG_pfuncDelay;                      /* 短延时 驱动可指定此处理      */
    ATA_REG_CB              ATACTRLCFG_tAtaReg;                         /* ATA 寄存器 驱动需初始化基地址*/
#define ATA_CTRLCFG_CMD_BASE        ATACTRLCFG_tAtaReg.ATAREG_tCommand.ATAIO_addrBase
#define ATA_CTRLCFG_CTRL_BASE       ATACTRLCFG_tAtaReg.ATAREG_tControl.ATAIO_addrBase
#define ATA_CTRLCFG_BUSM_BASE       ATACTRLCFG_tAtaReg.ATAREG_tBusMaster.ATAIO_addrBase
#define ATA_CTRLCFG_CMD_HANDLE      ATACTRLCFG_tAtaReg.ATAREG_tCommand.ATAIO_pvHandle
#define ATA_CTRLCFG_CTRL_HANDLE     ATACTRLCFG_tAtaReg.ATAREG_tControl.ATAIO_pvHandle
#define ATA_CTRLCFG_BUSM_HANDLE     ATACTRLCFG_tAtaReg.ATAREG_tBusMaster.ATAIO_pvHandle

    ULONG                   ATACTRLCFG_ulCtlrIntVector;                 /* 中断向量                     */
    PINT_SVR_ROUTINE        ATACTRLCFG_pfuncCtlrIntServ;                /* 控制器中断服务               */
    FUNCPTR                 ATACTRLCFG_pfuncCtrlIntPre;                 /* 控制器中断前期工作           */
    FUNCPTR                 ATACTRLCFG_pfuncCtrlIntPost;                /* 控制器中断后期工作           */

    PVOID                   ATACTRLCFG_pvReserved[8];                   /* 保留区                       */
} ATA_CTRL_CFG_CB;

typedef ATA_CTRL_CFG_CB    *ATA_CTRL_CFG_HANDLE;
/*********************************************************************************************************
  ATA 驱动 IO 操作控制块
*********************************************************************************************************/
typedef struct ata_drv_io_cb {
    FUNCPTR                 ATADRVIO_pfuncIoBytesRead;                  /* 多字节读                     */
    FUNCPTR                 ATADRVIO_pfuncIoBytesWrite;                 /* 多字节写                     */
    FUNCPTR                 ATADRVIO_pfuncIoWordsRead;                  /* 多字读                       */
    FUNCPTR                 ATADRVIO_pfuncIoWordsWrite;                 /* 多字写                       */
    FUNCPTR                 ATADRVIO_pfuncIoDwordsRead;                 /* 多双字读                     */
    FUNCPTR                 ATADRVIO_pfuncIoDwordsWrite;                /* 多双字写                     */

    PVOID                   ATADRVIO_pvReserved[4];                     /* 保留区                       */
} ATA_DRV_IO_CB;

typedef ATA_DRV_IO_CB      *ATA_DRV_IO_HANDLE;
/*********************************************************************************************************
  ATA 驱动 DMA 操作控制块
*********************************************************************************************************/
typedef struct ata_drv_dma_cb {
    PVOID                   ATADRVDMA_pvDmaCtrl;                        /* DMA 控制器控制句柄           */
    FUNCPTR                 ATADRVDMA_pfuncDmaCtrlInit;                 /* 初始化 DMA 控制器            */
    FUNCPTR                 ATADRVDMA_pfuncDmaCtrlCfg;                  /* 配置 DMA 控制器              */
    FUNCPTR                 ATADRVDMA_pfuncDmaCtrlStart;                /* 启动 DMA 传输                */
    FUNCPTR                 ATADRVDMA_pfuncDmaCtrlStop;                 /* 停止 DMA 传输                */
    FUNCPTR                 ATADRVDMA_pfuncDmaCtrlCheck;                /* 检查 DMA 状态                */
    FUNCPTR                 ATADRVDMA_pfuncDmaCtrlModeSet;              /* 设置 DMA 模式                */
    FUNCPTR                 ATADRVDMA_pfuncDmaCtrlModeGet;              /* 获取 DMA 模式                */
    FUNCPTR                 ATADRVDMA_pfuncDmaCtrlReset;                /* 复位 DMA 控制器              */
    INT                     ATADRVDMA_iDmaCtrlSupport;                  /* 支持 DMA 传输                */

    PVOID                   ATADRVDMA_pvReserved[4];                    /* 保留区                       */
} ATA_DRV_DMA_CB;

typedef ATA_DRV_DMA_CB      *ATA_DRV_DMA_HANDLE;
/*********************************************************************************************************
  ATA 驱动中断操作控制块
*********************************************************************************************************/
typedef struct ata_drv_int_cb {
    FUNCPTR                 ATADRVINT_pfuncCtrlIntConnect;              /* 链接中断                     */
    FUNCPTR                 ATADRVINT_pfuncCtrlIntEnable;               /* 使能中断                     */

    PVOID                   ATADRVINT_pvReserved[4];                    /* 保留区                       */
} ATA_DRV_INT_CB;

typedef ATA_DRV_INT_CB     *ATA_DRV_INT_HANDLE;
/*********************************************************************************************************
  ATA 驱动扩展操作控制块
*********************************************************************************************************/
typedef struct ata_drv_ext_cb {
    FUNCPTR                 ATADRVEXT_pfuncOptCtrl;
    FUNCPTR                 ATADRVEXT_pfuncVendorDriveInfoShow;
    FUNCPTR                 ATADRVEXT_pfuncVendorDriveRegNameGet;
    FUNCPTR                 ATADRVEXT_pfuncVendorDriveInit;
    FUNCPTR                 ATADRVEXT_pfuncVendorCtrlInfoShow;
    PVOIDFUNCPTR            ATADRVEXT_pfuncVendorCtrlRegNameGet;
    PVOIDFUNCPTR            ATADRVEXT_pfuncVendorCtrlTypeNameGet;
    FUNCPTR                 ATADRVEXT_pfuncVendorCtrlIntEnable;
    FUNCPTR                 ATADRVEXT_pfuncVendorCtrlIntConnect;
    FUNCPTR                 ATADRVEXT_pfuncVendorCtrlInit;
    FUNCPTR                 ATADRVEXT_pfuncVendorCtrlReadyWork;
    FUNCPTR                 ATADRVEXT_pfuncVendorPlatformInit;
    FUNCPTR                 ATADRVEXT_pfuncVendorDrvReadyWork;

    FUNCPTR                 ATADRVEXT_pfuncCtrlProbe;
    FUNCPTR                 ATADRVEXT_pfuncCtrlRemove;

    PVOID                   ATADRVEXT_pvReserved[4];                    /* 保留区                       */
} ATA_DRV_EXT_CB;

typedef ATA_DRV_EXT_CB     *ATA_DRV_EXT_HANDLE;
/*********************************************************************************************************
  驱动控制块
*********************************************************************************************************/
typedef struct ata_drv_cb {
    LW_LIST_LINE            ATADRV_lineDrvNode;                         /* 驱动管理节点                 */

    CHAR                    ATADRV_cDrvName[ATA_DRV_NAME_MAX];          /* 驱动名称                     */
    UINT32                  ATADRV_uiDrvVer;                            /* 驱动版本                     */

    UINT32                  ATADRV_uiUnitNumber;                        /* 本类设备索引                 */
    PVOID                   ATADRV_pvArg;                               /* 驱动参数                     */

    UINT                    ATADRV_uiCtrlNum;                           /* 控制器数量                   */
    ATA_CTRL_CB             ATADRV_tCtrl[ATA_CTRL_MAX];                 /* 控制器句柄 (ATA_CTRL_MAX)    */

    ATA_CTRL_CFG_CB         ATADRV_tCtrlCfg[ATA_CTRL_MAX];              /* 控制器参数 (ATA_CTRL_MAX)    */
                                                                        /* 驱动器 (CTRL_MAX * DRIVE_MAX)*/
    ATA_DRIVE_INFO_CB       ATADRV_tDriveInfo[ATA_CTRL_MAX * ATA_DRIVE_MAX];
    ATA_DRV_IO_CB           ATADRV_tIo;                                 /* IO 操作不能为空              */
    ATA_DRV_DMA_CB          ATADRV_tDma;                                /* DMA 控制器操作               */
    ATA_DRV_INT_CB          ATADRV_tInt;                                /* 中断操作                     */
    ATA_DRV_EXT_CB          ATADRV_tExt;                                /* 扩展操作                     */

    INT                     ATADRV_iIntDisable;                         /* 是否禁能中断                 */
    INT                     ATADRV_iBeSwap;                             /* 是否需要调整大小端字节序     */

#define ATA_DRV_FLAG_MASK      0xffff                                   /* 掩码                         */
#define ATA_DRV_FLAG_ACTIVE    0x01                                     /* 是否激活                     */
    INT                     ATADRV_iDrvFlag;                            /* 驱动标志                     */
    UINT32                  ATADRV_uiDrvCtrlNum;                        /* 关联控制器数                 */
    LW_LIST_LINE_HEADER     ATADRV_plineDrvCtrlHeader;                  /* 控制器管理链表头             */

    PVOID                   ATADRV_pvReserved[8];
} ATA_DRV_CB;

typedef ATA_DRV_CB    *ATA_DRV_HANDLE;
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
LW_API INT              API_AtaCtrlStatusCheck(ATA_CTRL_HANDLE  hCtrl, UINT8  ucMask, UINT8  ucStatus);
LW_API irqreturn_t      API_AtaCtrlIrqIsr(PVOID  pvArg, ULONG  ulVector);
LW_API INT              API_AtaDrvInit(ATA_DRV_HANDLE  hAtaDrv);


#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_ATA_EN > 0)         */
#endif                                                                  /*  __ATA_H                     */
/*********************************************************************************************************
  END
*********************************************************************************************************/
