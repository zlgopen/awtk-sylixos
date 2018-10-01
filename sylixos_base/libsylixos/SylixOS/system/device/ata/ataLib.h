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
** 文   件   名: ataLib.h
**
** 创   建   人: Gong.YuJian (弓羽箭)
**
** 文件创建日期: 2018 年 09 月 03 日
**
** 描        述: ATA 驱动库.
*********************************************************************************************************/

#ifndef __ATALIB_H
#define __ATALIB_H

/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_ATA_EN > 0)
#include "ataCfg.h"
#include "ata.h"
/*********************************************************************************************************
  调试模式
*********************************************************************************************************/
#define ATA_LOG_RUN                         __LOGMESSAGE_LEVEL
#define ATA_LOG_PRT                         __LOGMESSAGE_LEVEL
#define ATA_LOG_ERR                         __ERRORMESSAGE_LEVEL
#define ATA_LOG_BUG                         __BUGMESSAGE_LEVEL
#define ATA_LOG_ALL                         __PRINTMESSAGE_LEVEL

#if (ATA_DEBUG_EN > 0)
#define ATA_LOG                             _DebugFormat
#define ATA_INT_LOG                         _DebugFormat
#define ATA_CMD_LOG                         _DebugFormat
#define ATA_ATAPI_LOG                       _DebugFormat
#define ATA_ATAPI_INT_LOG                   _DebugFormat
#define ATA_ATAPI_CMD_LOG                   _DebugFormat

#else
#define ATA_LOG(level, fmt, ...)
#define ATA_INT_LOG(level, fmt, ...)
#define ATA_CMD_LOG(level, fmt, ...)
#define ATA_ATAPI_LOG(level, fmt, ...)
#define ATA_ATAPI_INT_LOG(level, fmt, ...)
#define ATA_ATAPI_CMD_LOG(level, fmt, ...)
#endif
/*********************************************************************************************************
  标记信息
*********************************************************************************************************/
#define ATA_FLAG(x, y)                      ((x & y) ? '+' : '-')
/*********************************************************************************************************
  版本字符串格式
*********************************************************************************************************/
#define ATA_DRV_VER_STR_LEN                 20
#define ATA_DRV_VER_FORMAT(ver)             "%d.%d.%d-rc%d", (ver >> 24) & 0xff, (ver >> 16) & 0xff,    \
                                            (ver >> 8) & 0xff, ver & 0xff
/*********************************************************************************************************
  自旋锁操作
*********************************************************************************************************/
#define ATA_SPIN_ISR_INIT(ctrl)             LW_SPIN_INIT(&ctrl->ATACTRL_slSpinLock)
#define ATA_SPIN_ISR_LOCK(ctrl, level)      LW_SPIN_LOCK_QUICK(&ctrl->ATACTRL_slSpinLock, &level)
#define ATA_SPIN_ISR_UNLOCK(ctrl, level)    LW_SPIN_UNLOCK_QUICK(&ctrl->ATACTRL_slSpinLock, level)
/*********************************************************************************************************
  LBA 选项
*********************************************************************************************************/
#define ATA_LBA28_MAX                       0x0fffffff
#define ATA_LBA48_MAX                       (UINT64)(0xffffffff | ((UINT64)0xffff << 32))
#define ATA_48BIT_MASK                      0x0000000000ff
/*********************************************************************************************************
  配置选项
*********************************************************************************************************/
#define ATA_CONFIG_PROT_TYPE                0xc000
#define ATA_CONFIG_PROT_TYPE_ATAPI          0x8000
#define ATA_CONFIG_REMOVABLE                0x0080
#define ATA_CONFIG_PKT_TYPE                 0x0060
#define ATA_CONFIG_PKT_TYPE_MICRO           0x0000
#define ATA_CONFIG_PKT_TYPE_INTER           0x0020
#define ATA_CONFIG_PKT_TYPE_ACCEL           0x0040
#define ATA_CONFIG_PKT_SIZE                 0x0003
#define ATA_CONFIG_PKT_SIZE_12              0x0000

#define ATA_SPEC_CONFIG_VALUE_0             0x37c8
#define ATA_SPEC_CONFIG_VALUE_1             0x738c
#define ATA_SPEC_CONFIG_VALUE_2             0x8c73
#define ATA_SPEC_CONFIG_VALUE_3             0xc837

#define ATA_SIGNATURE                       0x01010000
#define ATAPI_SIGNATURE                     0x010114EB

#define ATAPI_MAX_CMD_LENGTH                12

#define ATA_SDH_IBM                         0xa0
#define ATA_SDH_LBA                         0xe0

#define ATA_CTL_4BIT                        0x8
#define ATA_CTL_SRST                        0x4
#define ATA_CTL_NIEN                        0x2

#define ATA_ERR_ABRT                        0x04
#define ATA_USE_LBA                         0x40

#define ATA_DRIVE_BIT                       4                           /* 1 << ATA_DRIVE_BIT           */

#define ATA_STAT_WRTFLT                     0x20
#define ATA_STAT_SEEKCMPLT                  0x10
#define ATA_STAT_ECCCOR                     0x04

#define ATA_WORD54_58_VALID                 0x01
#define ATA_WORD64_70_VALID                 0x02
#define ATA_WORD88_VALID                    0x04
/*********************************************************************************************************
  命令
*********************************************************************************************************/
#define ATA_CMD_DIAGNOSE                    0x90
#define ATAPI_CMD_SRST                      0x08
#define ATA_CMD_RECALIB                     0x10
#define ATA_CMD_FORMAT                      0x50
#define ATA_CMD_EXECUTE_DEVICE_DIAGNOSTIC   0x90
#define ATA_CMD_IDENT_DEV                   0xec
#define ATA_CMD_INITP                       0x91
#define ATA_CMD_READ_DMA                    0xC8
#define ATA_CMD_READ_DMA_EXT                0x25
#define ATA_CMD_READ_MULTI                  0xC4
#define ATA_CMD_READ_MULTI_EXT              0x29
#define ATA_CMD_READ_VERIFY_SECTOR_S        0x40
#define ATA_CMD_READ                        0x20
#define ATA_CMD_READ_EXT                    0x24
#define ATA_CMD_SEEK                        0x70
#define ATA_CMD_SET_FEATURE                 0xEF
#define ATA_CMD_SET_MULTI                   0xC6
#define ATA_CMD_WRITE_DMA                   0xCA
#define ATA_CMD_WRITE_DMA_EXT               0x35
#define ATA_CMD_WRITE_MULTI                 0xC5
#define ATA_CMD_WRITE_MULTI_EXT             0x39
#define ATA_CMD_WRITE                       0x30
#define ATA_CMD_WRITE_EXT                   0x34
#define ATA_CMD_DOWNLOAD_MICROCODE          0x92
#define ATA_CMD_READ_BUFFER                 0xE4
#define ATA_CMD_WRITE_BUFFER                0xE8
#define ATA_CMD_CHECK_POWER_MODE            0xE5
#define ATA_CMD_IDLE_IMMEDIATE              0xE1
#define ATA_CMD_SLEEP                       0xE6
#define ATA_CMD_STANDBY_IMMEDIATE           0xE0
#define ATA_CMD_READ_VERIFY_SECTORS         0x40
#define ATA_PI_CMD_SRST                     0x08
#define ATA_PI_CMD_PKTCMD                   0xA0
#define ATA_PI_CMD_IDENTD                   0xA1
#define ATA_PI_CMD_SERVICE                  0xA2
#define ATA_CMD_NOP                         0x00

#define ATA_CMD_FLUSH_CACHE                 0xE7
#define ATA_CMD_GET_MEDIA_STATUS            0xDA
#define ATA_CMD_IDLE                        0xE3
#define ATA_CMD_MEDIA_EJECT                 0xED
#define ATA_CMD_MEDIA_LOCK                  0xDE
#define ATA_CMD_MEDIA_UNLOCK                0xDF
#define ATA_CMD_READ_DMA_QUEUED             0xC7
#define ATA_CMD_READ_NATIVE_MAX_ADDRESS     0xF8
#define ATA_CMD_SECURITY_DISABLE_PASSWORD   0xF6
#define ATA_CMD_SECURITY_ERASE_PREPARE      0xF3
#define ATA_CMD_SECURITY_ERASE_UNIT         0xF4
#define ATA_CMD_SECURITY_FREEZE_LOCK        0xF5
#define ATA_CMD_SECURITY_SET_PASSWORD       0xF1
#define ATA_CMD_SECURITY_UNLOCK             0xF2

#define ATA_CMD_SET_MAX                     0xF9
#define ATA_CMD_SMART                       0xB0
#define ATA_CMD_STANDBY                     0xE2
#define ATA_CMD_WRITE_DMA_QUEUED            0xCC

#define ATA_SMART_READ_DATA                 0xd0
#define ATA_SMART_ATTRIB_AUTO               0xd2
#define ATA_SMART_SAVE_ATTRIB               0xd3
#define ATA_SMART_OFFLINE_IMMED             0xd4
#define ATA_SMART_READ_LOG_SECTOR           0xd5
#define ATA_SMART_WRITE_LOG_SECTOR          0xd6
#define ATA_SMART_ENABLE_OPER               0xd8
#define ATA_SMART_DISABLE_OPER              0xd9
#define ATA_SMART_RETURN_STATUS             0xda


#define ATA_MAX_RW_SECTORS                  0x100                       /* max sectors per transfer     */
#define ATAPI_CDROM_BYTE_PER_BLK            2048                        /* user data in CDROM Model     */
#define ATAPI_BLOCKS                        100                         /* number of blocks             */

#define ATA_MULTISEC_MASK                   0x00ff

#define ATA_INTER_DMA_MASK                  0x8000
#define ATA_CMD_QUE_MASK                    0x4000
#define ATA_OVERLAP_MASK                    0x2000
#define ATA_IORDY_MASK                      0x0800
#define ATA_IOLBA_MASK                      0x0200
#define ATA_DMA_CAP_MASK                    0x0100

#define ATA_HWRR_CBLID                      0x2000

#define ATA_PIO_MASK_012                    0x03                        /* PIO Mode 0,1,2               */
#define ATA_PIO_MASK_34                     0x02                        /* PIO Mode 3,4                 */

#define ATA_LBA_HEAD_MASK                   0x0f000000
#define ATA_LBA_CYL_MASK                    0x00ffff00
#define ATA_LBA_SECTOR_MASK                 0x000000ff

#define ATA_CMD_CFA_ERASE_SECTORS                  0xC0
#define ATA_CMD_CFA_REQUEST_EXTENDED_ERROR_CODE    0x03
#define ATA_CMD_CFA_TRANSLATE_SECTOR               0x87
#define ATA_CMD_CFA_WRITE_MULTIPLE_WITHOUT_ERASE   0xCD
#define ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE    0x38

#define ATA_SUB_SET_MAX_ADDRESS             0x00
#define ATA_SUB_SET_MAX_SET_PASS            0x01
#define ATA_SUB_SET_MAX_LOCK                0x02
#define ATA_SUB_SET_MAX_UNLOCK              0x03
#define ATA_SUB_SET_MAX_FREEZE_LOCK         0x04

#define SET_MAX_VOLATILE                    0x00
#define SET_MAX_NON_VOLATILE                0x01

#define ATA_SUB_ENABLE_WCACHE               0x02                        /* enable write cache           */
#define ATA_SUB_SET_RWMODE                  0x03                        /* set transfer mode            */
#define ATA_SUB_ENB_ADV_POW_MNGMNT          0x05                        /* enable advanced power        */
#define ATA_SUB_ENB_POW_UP_STDBY            0x06                        /* Enable Power-Up In Standby   */
#define ATA_SUB_POW_UP_STDBY_SPIN           0x07                        /* device spin-up               */
#define ATA_SUB_BOOTMETHOD                  0x09                        /* Reserved for Address offset  */
#define ATA_SUB_ENA_CFA_POW_MOD1            0x0A                        /* Enable CFA power mode 1      */
#define ATA_SUB_DISABLE_NOTIFY              0x31                        /* Disable Media Status         */

#define ATA_SUB_DISABLE_LOOK                0x55
#define ATA_SUB_ENA_INTR_RELEASE            0x5D
#define ATA_SUB_ENA_SERV_INTR               0x5E
#define ATA_SUB_DISABLE_REVE                0x66

#define ATA_SUB_DISABLE_WCACHE              0x82
#define ATA_SUB_DIS_ADV_POW_MNGMT           0x85
#define ATA_SUB_DISB_POW_UP_STDBY           0x86
#define ATA_SUB_BOOTMETHOD_REPORT           0x89

#define ATA_SUB_DIS_CFA_POW_MOD1            0x8A
#define ATA_SUB_ENABLE_NOTIFY               0x95
#define ATA_SUB_ENABLE_LOOK                 0xaa

#define ATA_SUB_ENABLE_REVE                 0xcc
#define ATA_SUB_DIS_INTR_RELEASE            0xDD
#define ATA_SUB_DIS_SERV_INTR               0xDE
/*********************************************************************************************************
  ATA 寄存器
*********************************************************************************************************/
#define ATA_DATA_REG                        0                           /* (RW) data register (16 bits) */
#define ATA_ERROR_REG                       1                           /* (R)  error register          */
#define ATA_FEATURE_REG                     1                           /* (W)  feature/precompensation */
#define ATA_SECCNT_REG                      2                           /* (RW) sector count for ATA    */
#define ATA_SECTOR_REG                      3                           /* (RW) first sector number     */
#define ATA_CYL_LO_REG                      4                           /* (RW) cylinder low byte       */
#define ATA_CYL_HI_REG                      5                           /* (RW) cylinder high byte      */
#define ATA_SDH_REG                         6                           /* (RW) sector size/drive/head  */
#define ATA_COMMAND_REG                     7                           /* (W)  command register        */
#define ATA_STATUS_REG                      7                           /* (R)  immediate status        */
#define ATA_A_STATUS_REG                    0                           /* (R)  alternate status        */
#define ATA_D_CONTROL_REG                   0                           /* (W)  disk controller control */
#define ATA_D_ADDRESS_REG                   1                           /* (R)  disk controller address */
/*********************************************************************************************************
  寄存器地址
*********************************************************************************************************/
#define ATA_DATA_ADDR(ctrl)                 (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_DATA_REG)
#define ATA_ERROR_ADDR(ctrl)                (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_ERROR_REG)
#define ATA_FEATURE_ADDR(ctrl)              (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_FEATURE_REG)
#define ATA_SECCNT_ADDR(ctrl)               (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_SECCNT_REG)
#define ATA_SECTOR_ADDR(ctrl)               (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_SECTOR_REG)
#define ATA_CYLLO_BCNTLO_ADDR(ctrl)         (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_CYL_LO_REG)
#define ATA_CYLHI_BCNTHI_ADDR(ctrl)         (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_CYL_HI_REG)
#define ATA_CYLLO_ADDR(ctrl)                (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_CYL_LO_REG)
#define ATA_CYLHI_ADDR(ctrl)                (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_CYL_HI_REG)
#define ATA_SDH_D_SELECT_ADDR(ctrl)         (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_SDH_REG)
#define ATA_STATUS_ADDR(ctrl)               (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_STATUS_REG)
#define ATA_COMMAND_ADDR(ctrl)              (PVOID)(ctrl->ATA_CTRL_CMD_BASE + ATA_COMMAND_REG)

#define ATA_A_STATUS_ADDR(ctrl)             (PVOID)(ctrl->ATA_CTRL_CTRL_BASE + ATA_A_STATUS_REG)
#define ATA_D_CONTROL_ADDR(ctrl)            (PVOID)(ctrl->ATA_CTRL_CTRL_BASE + ATA_D_CONTROL_REG)
#define ATA_D_ADDRESS_ADDR(ctrl)            (PVOID)(ctrl->ATA_CTRL_CTRL_BASE + ATA_D_ADDRESS_REG)
/*********************************************************************************************************
  控制器 IO 操作
*********************************************************************************************************/
#define ATA_IO_BYTES_READ(ctrl, addr, buff, len)                                                        \
            ctrl->ATACTRL_hCtrlDrv->ATADRV_tIo.ATADRVIO_pfuncIoBytesRead(ctrl->ATACTRL_hCtrlDrv,        \
                                                                         ctrl->ATACTRL_uiCtrl,          \
                                                                         (addr_t)addr,                  \
                                                                         (PVOID)buff,                   \
                                                                         (size_t)len)
#define ATA_IO_BYTES_WRITE(ctrl, addr, buff, len)                                                       \
            ctrl->ATACTRL_hCtrlDrv->ATADRV_tIo.ATADRVIO_pfuncIoBytesWrite(ctrl->ATACTRL_hCtrlDrv,       \
                                                                          ctrl->ATACTRL_uiCtrl,         \
                                                                          (addr_t)addr,                 \
                                                                          (PVOID)buff,                  \
                                                                          (size_t)len)
#define ATA_IO_WORDS_READ(ctrl, addr, buff, len)                                                        \
            ctrl->ATACTRL_hCtrlDrv->ATADRV_tIo.ATADRVIO_pfuncIoWordsRead(ctrl->ATACTRL_hCtrlDrv,        \
                                                                         ctrl->ATACTRL_uiCtrl,          \
                                                                         (addr_t)addr,                  \
                                                                         (PVOID)buff,                   \
                                                                         (size_t)len)
#define ATA_IO_WORDS_WRITE(ctrl, addr, buff, len)                                                       \
            ctrl->ATACTRL_hCtrlDrv->ATADRV_tIo.ATADRVIO_pfuncIoWordsWrite(ctrl->ATACTRL_hCtrlDrv,       \
                                                                          ctrl->ATACTRL_uiCtrl,         \
                                                                          (addr_t)addr,                 \
                                                                          (PVOID)buff,                  \
                                                                          (size_t)len)
#define ATA_IO_DWORDS_READ(ctrl, addr, buff, len)                                                       \
            ctrl->ATACTRL_hCtrlDrv->ATADRV_tIo.ATADRVIO_pfuncIoDwordsRead(ctrl->ATACTRL_hCtrlDrv,       \
                                                                          ctrl->ATACTRL_uiCtrl,         \
                                                                          (addr_t)addr,                 \
                                                                          (PVOID)buff,                  \
                                                                          (size_t)len)
#define ATA_IO_DWORDS_WRITE(ctrl, addr, buff, len)                                                      \
            ctrl->ATACTRL_hCtrlDrv->ATADRV_tIo.ATADRVIO_pfuncIoDwordsWrite(ctrl->ATACTRL_hCtrlDrv,      \
                                                                           ctrl->ATACTRL_uiCtrl,        \
                                                                           (addr_t)addr,                \
                                                                           (PVOID)buff,                 \
                                                                           (size_t)len)
/*********************************************************************************************************
  等待状态 (>= 400ns)
*********************************************************************************************************/
#define ATA_WAIT_STATUS(ctrl)               (*ctrl->ATACTRL_pfuncDelay)(ctrl)
#define ATA_DELAY_400NSEC(ctrl)             (*ctrl->ATACTRL_pfuncDelay)(ctrl)
/*********************************************************************************************************
  ATA 设备控制块
*********************************************************************************************************/
typedef struct ata_dev_cb {
    BLK_DEV                 ATADEV_tBlkDev;                             /* 块设备                       */

    UINT                    ATADEV_uiCtrl;                              /* ctrl no.  0 - 1              */
    UINT                    ATADEV_uiDrive;                             /* drive no. 0 - 1              */
    UINT64                  ATADEV_ullBlkOffset;                        /* sector offset                */
    UINT64                  ATADEV_ullBlkCnt;                           /* number of sectors            */

    PCHAR                   ATADEV_pcBuff;                              /* Current position ser buffer  */
    PCHAR                   ATADEV_pcBuffEnd;                           /* End of user buffer           */
    ATA_DATA_DIR            ATADEV_iDirection;                          /* Transfer direction           */
    INT                     ATADEV_iTransCount;                         /* Number of transfer cycles    */
    INT                     ATADEV_iErrNum;                             /* Error description message    */

    UINT8                   ATADEV_ucIntReason;                         /* Interrupt Reason Register    */
    INT                     ATADEV_iStatus;                             /* Status Register              */
    UINT16                  ATADEV_usTransSize;                         /* Byte Count Register          */

    struct ata_drive_cb    *ATADEV_hDrive;

    PVOID                   ATADEV_pvReserved[16];
} ATA_DEV_CB;

typedef ATA_DEV_CB     *ATA_DEV_HANDLE;
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
PCHAR       ataDriveSerialInfoGet(ATA_DRIVE_HANDLE  hDrive, PCHAR  pcBuf, size_t  stLen);
PCHAR       ataDriveFwRevInfoGet(ATA_DRIVE_HANDLE   hDrive, PCHAR  pcBuf, size_t  stLen);
PCHAR       ataDriveModelInfoGet(ATA_DRIVE_HANDLE   hDrive, PCHAR  pcBuf, size_t  stLen);

VOID        ataSwapBufLe16(UINT16  *pusBuf, size_t  stWords);

VOID        ataCtrlDelay(ATA_CTRL_HANDLE  hCtrl);
INT         ataDriveCommandSend(ATA_CTRL_HANDLE  hCtrl,
                                UINT             uiCtrl,
                                UINT             uiDrive,
                                INT              iCmd,
                                UINT32           uiFeature,
                                UINT16           usSectorCnt,
                                UINT16           usSectorNum,
                                UINT8            ucCylLo,
                                UINT8            ucCylHi,
                                UINT8            ucSdh);
INT         ataDriveInit(ATA_CTRL_HANDLE  hCtrl, UINT  uiCtrl, UINT  uiDrive);
INT         ataDrvInit(ATA_DRV_HANDLE  hCtrlDrv, UINT  uiCtrl, ATA_CTRL_CFG_HANDLE  hCtrlCfg);

#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_ATA_EN > 0)         */
#endif                                                                  /*  __ATALIB_H                  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
