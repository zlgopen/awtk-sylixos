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
** 文   件   名: ata.c
**
** 创   建   人: Gong.YuJian (弓羽箭)
**
** 文件创建日期: 2018 年 09 月 03 日
**
** 描        述: ATA 驱动.
*********************************************************************************************************/
#define  __SYLIXOS_PCI_DRV
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_ATA_EN > 0)
#include "linux/compat.h"
#include "pci_ids.h"
#include "ataLib.h"
#include "ata.h"
/*********************************************************************************************************
  函数声明
*********************************************************************************************************/
static INT  ataBlkDevFill(ATA_DEV_HANDLE  hDev);
/*********************************************************************************************************
** 函数名称: ataBlkPioReadWrite
** 功能描述: 块设备 PIO 读写操作
** 输　入  : hDev           设备句柄
**           uiCtrl         控制器索引 (ATA_CTRL_MAX)
**           uiDrive        驱动器索引 (ATA_DRIVE_MAX)
**           ulCylinder     柱面
**           ulHead         磁头
**           ulSector       扇区
**           pvBuffer       缓冲区地址
**           ulBlkCount     数量
**           iDirection     读写标志
**           ulBlkStart     起始
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkPioReadWrite (ATA_DEV_HANDLE  hDev,
                                UINT            uiCtrl,
                                UINT            uiDrive,
                                ULONG           ulCylinder,
                                ULONG           ulHead,
                                ULONG           ulSector,
                                PVOID           pvBuffer,
                                UINT64          ullBlkCount,
                                INT             iDirection,
                                UINT64          ullBlkStart)
{
    INT                         iRet;
    ATA_DRIVE_HANDLE            hDrive = hDev->ATADEV_hDrive;
    ATA_CTRL_HANDLE             hCtrl  = hDrive->ATADRIVE_hCtrl;
    ATA_DRIVE_INFO_HANDLE       hInfo  = hDrive->ATADRIVE_hInfo;
    UINT8                       ucReg;
    UINT8                       ucData;
    INT                         iCmd;
    INT                         iRetryCount  = 0;
    UINT64                      ullBlock     = 1;
    UINT64                      ullSectorCnt = ullBlkCount;
    ULONG                       ulWords;
    UINT16                     *pusBuff;

    hDrive = hDev->ATADEV_hDrive;
    hCtrl  = hDrive->ATADRIVE_hCtrl;
    hInfo  = hDrive->ATADRIVE_hInfo;

    ATA_LOG(ATA_LOG_PRT,
            "ctrl %d drive %d blk pio read write cylinder 0x%llx head 0x%llx sector 0x%llx buff %p"
            " sector start 0x%llx sector cnt 0x%llx dir=%d.\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, ulCylinder, ulHead, ulSector, pvBuffer,
            ullBlkStart, ullBlkCount, iDirection);

    while (++iRetryCount <= ATA_RETRY_NUM) {
        iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 0);
        if (iRet!= ERROR_NONE) {
            return  (PX_ERROR);
        }

        pusBuff = (UINT16 *)pvBuffer;

        ucData = (UINT8)(ullBlkCount >> 8);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
        ucData = (UINT8)ullBlkCount;
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
        if (hDrive->ATADRIVE_iUseLba48 == LW_TRUE) {
            ucData = (UINT8)((ullBlkStart >> 24) & ATA_48BIT_MASK);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);
            ucData = (UINT8)(ullBlkStart & ATA_48BIT_MASK);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)((ullBlkStart >> 32) & ATA_48BIT_MASK);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);
            ucData = (UINT8)((ullBlkStart >> 8) & ATA_48BIT_MASK);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)((ullBlkStart >> 40) & ATA_48BIT_MASK);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);
            ucData = (UINT8)((ullBlkStart >> 16) & ATA_48BIT_MASK);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)((uiDrive << ATA_DRIVE_BIT) | hDrive->ATADRIVE_ucOkLba);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

        } else {
            ucData = (UINT8)ulSector;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)ulCylinder;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);
            ucData = (UINT8)(ulCylinder >> 8);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)((uiDrive << ATA_DRIVE_BIT) | hDrive->ATADRIVE_ucOkLba | (ulHead & 0x0f));
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
        }

        ATA_LOG(ATA_LOG_PRT,
                "ctrl %d drive %d blk pio read write"
                " Sector Count 0x%llx Sector 0x%llx Cyl High 0x%x Cyl Low 0x%x Head 0x%x dir=%d.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive,
                ullBlkCount, ulSector, (UINT8)(ulCylinder >> 8), (UINT8)ulCylinder,
                (UINT8)((uiDrive << ATA_DRIVE_BIT) | hDrive->ATADRIVE_ucOkLba | (ulHead & 0x0f)),
                iDirection);

        if (hDrive->ATADRIVE_usRwPio == ATA_PIO_MULTI) {
            ullBlock = hDrive->ATADRIVE_usMultiSecs;
        }

        iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 0);
        if (iRet!= ERROR_NONE) {
            return  (PX_ERROR);
        }

        ulWords = (hInfo->ATADINFO_uiBytes * ullBlock) >> 1;
        if (iDirection == O_WRONLY) {
            if (hDrive->ATADRIVE_usRwPio == ATA_PIO_MULTI) {
                if (hDrive->ATADRIVE_iUseLba48 == LW_TRUE) {
                    iCmd = ATA_CMD_WRITE_MULTI_EXT;
                } else {
                    iCmd = ATA_CMD_WRITE_MULTI;
                }

            } else {
                if (hDrive->ATADRIVE_iUseLba48 == LW_TRUE) {
                    iCmd = ATA_CMD_WRITE_EXT;
                } else {
                    iCmd = ATA_CMD_WRITE;
                }
            }

            ucData = (UINT8)iCmd;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_COMMAND_ADDR(hCtrl), &ucData, 1);

            while (ullSectorCnt > 0) {
                if ((hDrive->ATADRIVE_usRwPio == ATA_PIO_MULTI) &&
                    (ullSectorCnt < ullBlock)) {
                    ullBlock = ullSectorCnt;
                    ulWords = (hInfo->ATADINFO_uiBytes * ullBlock) >> 1;
                }

                iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, ATA_STAT_DRQ);
                if (iRet != ERROR_NONE) {
                    return  (PX_ERROR);
                }

                if (hDrive->ATADRIVE_usRwBits == ATA_BITS_16) {
                    ATA_IO_WORDS_WRITE(hCtrl, ATA_DATA_ADDR(hCtrl), pusBuff, ulWords);
                } else {
                    ATA_IO_DWORDS_WRITE(hCtrl, ATA_DATA_ADDR(hCtrl), pusBuff, ulWords >> 1);
                }

                if (hCtrl->ATACTRL_hCtrlDrv->ATADRV_iIntDisable) {
                    iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_BUSY, 0);
                } else {
                    iRet = API_SemaphoreBPend(hCtrl->ATACTRL_hSemSync, hCtrl->ATACTRL_ulSemSyncTimeout);
                }
                if ((hCtrl->ATACTRL_iIntStatus & ATA_STAT_ERR) ||
                    (iRet != ERROR_NONE)) {
                    goto  __error_handle;
                }

                pusBuff      += ulWords;
                ullSectorCnt -= ullBlock;
            }

        } else {
            if (hDrive->ATADRIVE_usRwPio == ATA_PIO_MULTI) {
                if (hDrive->ATADRIVE_iUseLba48 == LW_TRUE) {
                    iCmd = ATA_CMD_READ_MULTI_EXT;
                } else {
                    iCmd = ATA_CMD_READ_MULTI;
                }

            } else {
                if (hDrive->ATADRIVE_iUseLba48 == LW_TRUE) {
                    iCmd = ATA_CMD_READ_EXT;
                } else {
                    iCmd = ATA_CMD_READ;
                }
            }

            ucData = (UINT8)iCmd;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_COMMAND_ADDR(hCtrl), &ucData, 1);

            while (ullSectorCnt > 0) {
                if ((hDrive->ATADRIVE_usRwPio == ATA_PIO_MULTI) &&
                    (ullSectorCnt < ullBlock)) {
                    ullBlock = ullSectorCnt;
                    ulWords = (hInfo->ATADINFO_uiBytes * ullBlock) >> 1;
                }

                if (hCtrl->ATACTRL_hCtrlDrv->ATADRV_iIntDisable) {
                    iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, ATA_STAT_DRQ);
                } else {
                    iRet = API_SemaphoreBPend(hCtrl->ATACTRL_hSemSync, hCtrl->ATACTRL_ulSemSyncTimeout);
                }
                if ((hCtrl->ATACTRL_iIntStatus & ATA_STAT_ERR) ||
                    (iRet != ERROR_NONE)) {
                    goto  __error_handle;
                }

                if (hDrive->ATADRIVE_usRwBits == ATA_BITS_16) {
                    ATA_IO_WORDS_READ(hCtrl, ATA_DATA_ADDR(hCtrl), pusBuff, ulWords);
                } else {
                    ATA_IO_DWORDS_READ(hCtrl, ATA_DATA_ADDR(hCtrl), pusBuff, ulWords >> 1);
                }

                pusBuff      += ulWords;
                ullSectorCnt -= ullBlock;
            }
        }

        ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk pio read write ok cmd 0x%x dir %d(%s).\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, iCmd,
                iDirection, (iDirection == O_RDONLY) ? "read" : "write");

        return  (ERROR_NONE);

__error_handle:
        ATA_IO_BYTES_READ(hCtrl, ATA_ERROR_ADDR(hCtrl), (PVOID)&ucReg, 1);
        ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), (PVOID)&ucData, 1);
        ATA_LOG(ATA_LOG_ERR,
                "ctrl %d drive %d blk pio read write error cmd 0x%x astatus 0x%x error 0x%x ret %d(0x%x)"
                " cylinder 0x%llx head 0x%llx sector 0x%llx buffer %p blk count 0x%llx blk start 0x%llx"
                " current buffer %p current sector count 0x%llx current blk 0x%llx current words 0x%llx"
                " int status 0x%x int cnt %d  int error status 0x%x int error cnt %d dir %d.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, iCmd, ucData, ucReg, iRet, iRet,
                ulCylinder, ulHead, ulSector, pvBuffer, ullBlkCount, ullBlkStart,
                (PVOID)pusBuff, ullSectorCnt, ullBlock, ulWords,
                hCtrl->ATACTRL_iIntStatus, hCtrl->ATACTRL_ulIntCount,
                hCtrl->ATACTRL_iIntErrStatus, hCtrl->ATACTRL_ulIntErrCount, iDirection);
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: ataBlkDmaReadWrite
** 功能描述: 块设备读写操作
** 输　入  : hDev           设备句柄
**           uiCtrl         控制器索引 (ATA_CTRL_MAX)
**           uiDrive        驱动器索引 (ATA_DRIVE_MAX)
**           ulCylinder     柱面
**           ulHead         磁头
**           ulSector       扇区
**           pvBuffer       缓冲区地址
**           ullBlkCount    数量
**           iDirection     读写标志
**           ullBlkStart    起始
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkDmaReadWrite (ATA_DEV_HANDLE  hDev,
                                UINT            uiCtrl,
                                UINT            uiDrive,
                                ULONG           ulCylinder,
                                ULONG           ulHead,
                                ULONG           ulSector,
                                PVOID           pvBuffer,
                                UINT64          ullBlkCount,
                                INT             iDirection,
                                UINT64          ullBlkStart)
{
    INT                         iRet;
    ATA_DRV_HANDLE              hCtrlDrv;
    ATA_DRIVE_HANDLE            hDrive = hDev->ATADEV_hDrive;
    ATA_CTRL_HANDLE             hCtrl  = hDrive->ATADRIVE_hCtrl;
    ATA_DRIVE_INFO_HANDLE       hInfo  = hDrive->ATADRIVE_hInfo;
    UINT8                       ucReg;
    UINT8                       ucData;
    INT                         iCmd;

    hDrive   = hDev->ATADEV_hDrive;
    hCtrl    = hDrive->ATADRIVE_hCtrl;
    hInfo    = hDrive->ATADRIVE_hInfo;
    hCtrlDrv = hCtrl->ATACTRL_hCtrlDrv;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk dma read write cylinder %d head %d sector %d buff %p"
            " sector start %d sector cnt %d dir=%d(%s).\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, ulCylinder, ulHead, ulSector, pvBuffer,
            ullBlkStart, ullBlkCount, iDirection, (iDirection == O_RDONLY) ? "read" : "write");

    iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 0);
    if (iRet != ERROR_NONE) {
        return  (PX_ERROR);
    }

    if (hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlCfg) {
        iRet = (*hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlCfg)(hCtrlDrv,
                                                                  uiCtrl,
                                                                  uiDrive,
                                                                  pvBuffer,
                                                                  (size_t)(hInfo->ATADINFO_uiBytes *
                                                                           ullBlkCount),
                                                                  iDirection);
        if (iRet != ERROR_NONE) {
            return  (PX_ERROR);
        }
    }

    ucData = (UINT8)(ullBlkCount >> 8);
    ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
    ucData = (UINT8)ullBlkCount;
    ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
    if (hDrive->ATADRIVE_iUseLba48 == LW_TRUE) {
        ucData = (UINT8)((ullBlkStart >> 24) & ATA_48BIT_MASK);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);
        ucData = (UINT8)(ullBlkStart & ATA_48BIT_MASK);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);

        ucData = (UINT8)((ullBlkStart >> 32) & ATA_48BIT_MASK);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);
        ucData = (UINT8)((ullBlkStart >> 8) & ATA_48BIT_MASK);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);

        ucData = (UINT8)((ullBlkStart >> 40) & ATA_48BIT_MASK);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);
        ucData = (UINT8)((ullBlkStart >> 16) & ATA_48BIT_MASK);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);

        ucData = (UINT8)((uiDrive << ATA_DRIVE_BIT) | hDrive->ATADRIVE_ucOkLba);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

    } else {
        ucData = (UINT8)ulSector;
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);

        ucData = (UINT8)ulCylinder;
        ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);
        ucData = (UINT8)(ulCylinder >> 8);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);

        ucData = (UINT8)((uiDrive << ATA_DRIVE_BIT) | hDrive->ATADRIVE_ucOkLba | (ulHead & 0x0f));
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
    }

    if (iDirection == O_WRONLY) {
        if (hDrive->ATADRIVE_iUseLba48 == LW_TRUE) {
            iCmd = ATA_CMD_WRITE_DMA_EXT;
        } else {
            iCmd = ATA_CMD_WRITE_DMA;
        }
        API_CacheDmaFlush(pvBuffer, (size_t)(hInfo->ATADINFO_uiBytes * ullBlkCount));

    } else {
        if (hDrive->ATADRIVE_iUseLba48 == LW_TRUE) {
            iCmd = ATA_CMD_READ_DMA_EXT;
        } else {
            iCmd = ATA_CMD_READ_DMA;
        }
    }

    ucData = (UINT8)iCmd;
    ATA_IO_BYTES_WRITE(hCtrl, ATA_COMMAND_ADDR(hCtrl), &ucData, 1);
    ATA_DELAY_400NSEC(hCtrl);

    if (hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlStart) {
        (*hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlStart)(hCtrlDrv, uiCtrl);
    }
    if (hCtrl->ATACTRL_hCtrlDrv->ATADRV_iIntDisable) {
        iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 0);
    } else {
        iRet = API_SemaphoreBPend(hCtrl->ATACTRL_hSemSync, hCtrl->ATACTRL_ulSemSyncTimeout);
    }
    if (hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlStop) {
        (*hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlStop)(hCtrlDrv, uiCtrl);
    }
    if ((hCtrl->ATACTRL_iIntStatus & ATA_STAT_ERR) ||
        (iRet != ERROR_NONE)) {
        goto  __error_handle;
    }

    if (iDirection == O_RDONLY) {
        API_CacheDmaInvalidate(pvBuffer, (size_t)(hInfo->ATADINFO_uiBytes * ullBlkCount));
    }

    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucData, 1);
    if (ucData & (ATA_STAT_BUSY | ATA_STAT_DRQ)) {
        goto  __error_handle;
    }

    return  (ERROR_NONE);

__error_handle:
    ATA_IO_BYTES_READ(hCtrl, ATA_ERROR_ADDR(hCtrl), (PVOID)&ucReg, 1);
    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), (PVOID)&ucData, 1);
    ATA_LOG(ATA_LOG_ERR,
            "ctrl %d drive %d blk pio read write error cmd 0x%x astatus 0x%x error 0x%x ret %d(0x%x)"
            " cylinder 0x%llx head 0x%llx sector 0x%llx buffer %p blk count 0x%llx blk start 0x%llx"
            " int status 0x%x int cnt %d  int error status 0x%x int error cnt %d dir %d(%s).\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, iCmd, ucData, ucReg, iRet, iRet,
            ulCylinder, ulHead, ulSector, pvBuffer, ullBlkCount, ullBlkStart,
            hCtrl->ATACTRL_iIntStatus, hCtrl->ATACTRL_ulIntCount,
            hCtrl->ATACTRL_iIntErrStatus, hCtrl->ATACTRL_ulIntErrCount,
            iDirection, (iDirection == O_RDONLY) ? "read" : "write");

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: ataBlkReadWrite
** 功能描述: 块设备读写操作
** 输　入  : hDev           设备句柄
**           pvBuffer       缓冲区地址
**           ulBlkStart     起始
**           ulBlkCount     数量
**           iDirection     读写标志
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkReadWrite (ATA_DEV_HANDLE  hDev,
                             PVOID           pvBuffer,
                             ULONG           ulBlkStart,
                             ULONG           ulBlkCount,
                             INT             iDirection)
{
    INT                         iRet      = PX_ERROR;
    INT                         iStatus   = PX_ERROR;
    ATA_DRIVE_HANDLE            hDrive    = hDev->ATADEV_hDrive;
    ATA_CTRL_HANDLE             hCtrl     = hDrive->ATADRIVE_hCtrl;
    PLW_BLK_DEV                 hBlk      = &hDev->ATADEV_tBlkDev;
    ATA_DRIVE_INFO_HANDLE       hInfo     = hDrive->ATADRIVE_hInfo;
    UINT8                      *pucBuffer = (UINT8 *)pvBuffer;

    UINT32                      uiRetryRw0  = 0;
    UINT32                      uiRetryRw1  = 0;
    UINT32                      uiRetrySeek = 0;
    ULONG                       ulCylinder;
    ULONG                       ulHead;
    ULONG                       ulSector;
    ULONG                       ulSectorCnt;
    ULONG                       i;

    if ((!hCtrl->ATACTRL_iDrvInstalled) || (hDrive->ATADRIVE_iState != ATA_DEV_OK) || (!pucBuffer)) {
        ATA_LOG(ATA_LOG_ERR,
                "ctrl %d drive %d blk read write invalid argument installed %d state %d buff %p.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive,
                hCtrl->ATACTRL_iDrvInstalled,
                hDrive->ATADRIVE_iState,
                pvBuffer);
        return  (PX_ERROR);
    }

    ulSectorCnt = hBlk->BLKD_ulNSector;
    if ((ulBlkStart + ulBlkCount) > ulSectorCnt) {
        ATA_LOG(ATA_LOG_ERR,
                "ctrl %d drive %d blk read write invalid start 0x%llx num 0x%llx sector cnt 0x%llx.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, ulBlkStart, ulBlkCount, ulSectorCnt);
        return  (PX_ERROR);
    }

    API_SemaphoreMPend(hCtrl->ATACTRL_hSemDev, LW_OPTION_WAIT_INFINITE);

    ulBlkStart += (ULONG)hDev->ATADEV_ullBlkOffset;
    for (i = 0; i < ulBlkCount; i += ulSectorCnt) {
        if (hDrive->ATADRIVE_ucOkLba) {
            ulHead     = (ulBlkStart & ATA_LBA_HEAD_MASK) >> 24;
            ulCylinder = (ulBlkStart & ATA_LBA_CYL_MASK) >> 8;
            ulSector   = (ulBlkStart & ATA_LBA_SECTOR_MASK);

        } else {
            ulCylinder = ulBlkStart / (hInfo->ATADINFO_uiSectors * hInfo->ATADINFO_uiHeads);
            ulSector   = ulBlkStart % (hInfo->ATADINFO_uiSectors * hInfo->ATADINFO_uiHeads);
            ulHead     = ulSector / hInfo->ATADINFO_uiSectors;
            ulSector   = ulSector % hInfo->ATADINFO_uiSectors + 1;
        }

        ulSectorCnt = __MIN(ulBlkCount - i, ATA_MAX_RW_SECTORS);

        ATA_LOG(ATA_LOG_PRT,
                "ctrl %d drive %d blk read write cyl 0x%llx head 0x%llx sec 0x%llx"
                " buffer %p blk start 0x%llx blk cnt 0x%llx dir %d sector cnt 0x%llx.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, ulCylinder, ulHead, ulSector,
                pucBuffer, ulBlkStart, ulBlkCount, iDirection, ulSectorCnt);

        uiRetryRw1 = 0;
        uiRetryRw0 = 0;

        if ((hDrive->ATADRIVE_iDmaUse) &&
            (hCtrl->ATACTRL_hCtrlDrv->ATADRV_tDma.ATADRVDMA_iDmaCtrlSupport)) {
            iRet = ataBlkDmaReadWrite(hDev,
                                      hDev->ATADEV_uiCtrl,
                                      hDev->ATADEV_uiDrive,
                                      ulCylinder,
                                      ulHead,
                                      ulSector,
                                      (PVOID)pucBuffer,
                                      ulSectorCnt,
                                      iDirection,
                                      ulBlkStart);
            if (iRet != ERROR_NONE) {
                hDrive->ATADRIVE_iDmaUse = LW_FALSE;
                goto  __error_dma;
            }

        } else {
__error_dma:
            do {
                iRet = ataBlkPioReadWrite(hDev,
                                          hDev->ATADEV_uiCtrl,
                                          hDev->ATADEV_uiDrive,
                                          ulCylinder,
                                          ulHead,
                                          ulSector,
                                          (PVOID)pucBuffer,
                                          ulSectorCnt,
                                          iDirection,
                                          ulBlkStart);
                if (iRet == ERROR_NONE) {
                    break;
                }

                if (++uiRetryRw0 > ATA_RETRY_NUM) {
                    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk read write retrying.\r\n",
                            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);

                    if (++uiRetryRw1 > ATA_RETRY_NUM) {
                        goto __done_handle;
                    }

                    uiRetrySeek = 0;
                    do {
                        iStatus = ataDriveCommandSend(hCtrl, hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive,
                                                      ATA_CMD_SEEK, ulCylinder, ulHead, 0, 0, 0, 0);
                        if (iStatus == ERROR_NONE) {
                            break;
                        }
                    } while (iStatus != ERROR_NONE);

                    if (++uiRetrySeek > ATA_RETRY_NUM) {
                        goto  __done_handle;
                    }

                    uiRetryRw0 = 0;
                }

            } while (iRet != ERROR_NONE);
        }

        ulBlkStart += ulSectorCnt;
        pucBuffer  += hBlk->BLKD_ulBytesPerSector * ulSectorCnt;
    }

    iRet = ERROR_NONE;

__done_handle:
    if (iRet == PX_ERROR) {
        _ErrorHandle (EIO);
    }

    API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);

    return  (iRet);
}
/*********************************************************************************************************
** 函数名称: ataBlkRd
** 功能描述: 块设备读操作
** 输　入  : hDev           设备句柄
**           pvBuffer       缓冲区地址
**           ulBlkStart     起始
**           ulBlkCount     数量
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkRd (ATA_DEV_HANDLE  hDev, PVOID  pvBuffer, ULONG  ulBlkStart, ULONG  ulBlkCount)
{
    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk read handle %p.\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, hDev);

    return  (ataBlkReadWrite(hDev, pvBuffer, ulBlkStart, ulBlkCount, O_RDONLY));
}
/*********************************************************************************************************
** 函数名称: ataBlkWrt
** 功能描述: 块设备写操作
** 输　入  : hDev           设备句柄
**           pvBuffer       缓冲区地址
**           ulBlkStart     起始
**           ulBlkCount     数量
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkWrt (ATA_DEV_HANDLE  hDev, PVOID  pvBuffer, ULONG  ulBlkStart, ULONG  ulBlkCount)
{
    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk write handle %p.\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive, hDev);

    return  (ataBlkReadWrite(hDev, pvBuffer, ulBlkStart, ulBlkCount, O_WRONLY));
}
/*********************************************************************************************************
** 函数名称: ataBlkIoctl
** 功能描述: 块设备 ioctl
** 输　入  : hDev       设备控制句柄
**           iCmd       命令
**           lArg       参数
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkIoctl (ATA_DEV_HANDLE  hDev, INT  iCmd, LONG  lArg)
{
    ATA_DRIVE_HANDLE        hDrive = hDev->ATADEV_hDrive;
    ATA_CTRL_HANDLE         hCtrl  = hDrive->ATADRIVE_hCtrl;
    PLW_BLK_INFO            hBlkInfo;                                   /*  设备信息                    */

    if ((!hCtrl->ATACTRL_iDrvInstalled) ||
        (hDrive->ATADRIVE_iState != ATA_DEV_OK)) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d blk ioctl status error.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);
        return(PX_ERROR);
    }

    API_SemaphoreMPend(hCtrl->ATACTRL_hSemDev, LW_OPTION_WAIT_INFINITE);

    switch (iCmd) {

    /*
     *  必须要支持的命令
     */
    case FIOSYNC:
    case FIODATASYNC:
    case FIOSYNCMETA:
    case FIOFLUSH:                                                      /*  将缓存写入磁盘              */
    case FIOUNMOUNT:                                                    /*  卸载卷                      */
    case FIODISKINIT:                                                   /*  初始化磁盘                  */
    case FIODISKCHANGE:                                                 /*  磁盘媒质发生变化            */
        break;

    case FIOTRIM:
        break;
    /*
     *  低级格式化
     */
    case FIODISKFORMAT:                                                 /*  格式化卷                    */
        API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);
        return  (PX_ERROR);                                             /*  不支持低级格式化            */

    /*
     *  FatFs 扩展命令
     */
    case LW_BLKD_CTRL_POWER:
    case LW_BLKD_CTRL_LOCK:
    case LW_BLKD_CTRL_EJECT:
        break;

    case LW_BLKD_GET_SECSIZE:
        *((ULONG *)lArg) = hDev->ATADEV_tBlkDev.BLKD_ulBytesPerSector;
        break;

    case LW_BLKD_GET_BLKSIZE:
        *((ULONG *)lArg) = hDev->ATADEV_tBlkDev.BLKD_ulBytesPerBlock;
        break;

    case LW_BLKD_GET_SECNUM:
        *((ULONG *)lArg) = hDev->ATADEV_tBlkDev.BLKD_ulNSector;
        break;

    case LW_BLKD_CTRL_INFO:
        hBlkInfo = (PLW_BLK_INFO)lArg;
        if (!hBlkInfo) {
            _ErrorHandle(EINVAL);
            API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);
            return  (PX_ERROR);
        }
        lib_bzero(hBlkInfo, sizeof(LW_BLK_INFO));
        hBlkInfo->BLKI_uiType = LW_BLKD_CTRL_INFO_TYPE_ATA;
        ataDriveSerialInfoGet(hDrive, hBlkInfo->BLKI_cSerial,   LW_BLKD_CTRL_INFO_STR_SZ);
        ataDriveFwRevInfoGet(hDrive,  hBlkInfo->BLKI_cFirmware, LW_BLKD_CTRL_INFO_STR_SZ);
        ataDriveModelInfoGet(hDrive,  hBlkInfo->BLKI_cProduct,  LW_BLKD_CTRL_INFO_STR_SZ);
        break;

    case FIOWTIMEOUT:
    case FIORTIMEOUT:
        break;

    default:
        _ErrorHandle(ENOSYS);
        API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);
        return  (PX_ERROR);
    }

    API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataBlkStatus
** 功能描述: 块设备复位
** 输　入  : hDev       设备控制句柄
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkReset (ATA_DEV_HANDLE  hDev)
{
    INT                     iRet   = PX_ERROR;
    ATA_DRIVE_HANDLE        hDrive = hDev->ATADEV_hDrive;
    ATA_CTRL_HANDLE         hCtrl  = hDrive->ATADRIVE_hCtrl;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk dev reset.\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);

    if (!hCtrl->ATACTRL_iDrvInstalled) {
        return  (ERROR_NONE);
    }

    if ((hDrive->ATADRIVE_iType == ATA_TYPE_ATA) && (!hDrive->ATADRIVE_pfuncDevReset)) {
        return  (ERROR_NONE);
    }

    if (hDrive->ATADRIVE_iState != ATA_DEV_MED_CH) {
        API_SemaphoreMPend(hCtrl->ATACTRL_hSemDev, LW_OPTION_WAIT_INFINITE);

        if ((hDrive->ATADRIVE_iState != ATA_DEV_INIT) &&
            (hDrive->ATADRIVE_pfuncDevReset)) {
            iRet = (*hDrive->ATADRIVE_pfuncDevReset)(hCtrl, hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);
            if (iRet != ERROR_NONE) {
                API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);
                return  (PX_ERROR);
            }
        }

        iRet = ataDriveInit(hCtrl, hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);
        if (iRet != ERROR_NONE) {
            API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);
            return  (PX_ERROR);
        }

        ataBlkDevFill(hDev);

        API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);
    }

    hDrive->ATADRIVE_iState = ATA_DEV_OK;

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataBlkStatus
** 功能描述: 块设备状态检查
** 输　入  : hDev       设备控制句柄
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkStatus (ATA_DEV_HANDLE  hDev)
{
    ATA_DRIVE_HANDLE        hDrive = hDev->ATADEV_hDrive;
    ATA_CTRL_HANDLE         hCtrl  = hDrive->ATADRIVE_hCtrl;

    if ((!hCtrl->ATACTRL_iDrvInstalled) || (hDrive->ATADRIVE_iState != ATA_DEV_OK)) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d blk dev status error.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);
        return  (PX_ERROR);
    }

    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk dev status ok.\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataBlkDevFill
** 功能描述: 构造块设备
** 输　入  : hDev       设备控制句柄
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataBlkDevFill (ATA_DEV_HANDLE  hDev)
{
    ATA_DRIVE_HANDLE            hDrive       = hDev->ATADEV_hDrive;
    ATA_PARAM_HANDLE            hParam       = &hDrive->ATADRIVE_tParam;
    ATA_DRIVE_INFO_HANDLE       hInfo        = hDrive->ATADRIVE_hInfo;
    PLW_BLK_DEV                 hBlk         = &hDev->ATADEV_tBlkDev;
    UINT64                      ullBlkMax    = 0;
    UINT32                      uiCylinders  = hInfo->ATADINFO_uiCylinders;
    UINT32                      uiHeads      = hInfo->ATADINFO_uiHeads;
    UINT32                      uiSectors    = hInfo->ATADINFO_uiSectors;
    UINT32                      uiBytes      = hInfo->ATADINFO_uiBytes;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk dev fill start.\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);

    if ((hDrive->ATADRIVE_iState != ATA_DEV_OK) &&
        (hDrive->ATADRIVE_iState != ATA_DEV_MED_CH)) {
        ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk dev fill state error.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);
        return  (PX_ERROR);
    }

    if (hDrive->ATADRIVE_iType == ATA_TYPE_ATA) {
        ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk dev fill ATA.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);

        if ((hDrive->ATADRIVE_ucOkLba) &&
            (hDrive->ATADRIVE_ullCapacity) &&
            (hDrive->ATADRIVE_ullCapacity > (UINT64)(uiCylinders * uiHeads * uiSectors))) {
            ullBlkMax = hDrive->ATADRIVE_ullCapacity - hDev->ATADEV_ullBlkOffset;

            ATA_LOG(ATA_LOG_PRT,
                    "ctrl %d drive %d blk dev use lba cyl=%d, heads=%d, secs=%d info=%p bytes=%d.\r\n",
                    hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive,
                    uiCylinders, uiHeads, uiSectors, hInfo, uiBytes);

        } else {
            ullBlkMax = (UINT64)(uiCylinders * uiHeads * uiSectors) - hDev->ATADEV_ullBlkOffset;

            ATA_LOG(ATA_LOG_PRT,
                    "ctrl %d drive %d blk dev use chs cyl=%d, heads=%d, secs=%d info=%p bytes=%d.\r\n",
                    hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive,
                    uiCylinders, uiHeads, uiSectors, hInfo, uiBytes);
        }

        if ((hDev->ATADEV_ullBlkCnt == 0) ||
            (hDev->ATADEV_ullBlkCnt > ullBlkMax)) {
            hDev->ATADEV_ullBlkCnt = ullBlkMax;
        }

        hBlk->BLKD_ulNSector         = hDev->ATADEV_ullBlkCnt;
        hBlk->BLKD_ulBytesPerSector  = uiBytes;
        hBlk->BLKD_ulBytesPerBlock   = uiBytes;
        hBlk->BLKD_bRemovable        = LW_TRUE;
        hBlk->BLKD_iRetry            = 1;
        hBlk->BLKD_iFlag             = O_RDWR;
        hBlk->BLKD_bDiskChange       = LW_FALSE;
        hBlk->BLKD_pfuncBlkRd        = ataBlkRd;
        hBlk->BLKD_pfuncBlkWrt       = ataBlkWrt;
        hBlk->BLKD_pfuncBlkIoctl     = ataBlkIoctl;
        hBlk->BLKD_pfuncBlkReset     = ataBlkReset;
        hBlk->BLKD_pfuncBlkStatusChk = ataBlkStatus;

    } else if (hDrive->ATADRIVE_iType == ATA_TYPE_ATAPI) {
        ATA_LOG(ATA_LOG_PRT,  "ctrl %d drive %d blk dev fill ATAPI.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);

        if (hParam->ATAPARAM_usConfig & ATA_CONFIG_REMOVABLE) {
            hBlk->BLKD_bRemovable = LW_TRUE;
        } else {
            hBlk->BLKD_bRemovable = LW_FALSE;
        }

        return  (PX_ERROR);

    } else {
        ATA_LOG(ATA_LOG_ERR,  "ctrl %d drive %d blk dev fill error type.\r\n",
                hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive);
        return  (PX_ERROR);
    }

    ATA_LOG(ATA_LOG_PRT,
            "ctrl %d drive %d blk dev fill ok sector cnt %d bytes per sector %d bytes per block %d.\r\n",
            hDev->ATADEV_uiCtrl, hDev->ATADEV_uiDrive,
            hBlk->BLKD_ulNSector, hBlk->BLKD_ulBytesPerSector, hBlk->BLKD_ulBytesPerBlock);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataBlkDevCreate
** 功能描述: 创建块设备
** 输　入  : hCtrlDrv           控制器驱动控制句柄
**           uiCtrl             控制器索引 (ATA_CTRL_MAX)
**           uiDrive            驱动器索引 (ATA_DRIVE_MAX)
**           stBlkCnt           扇区数量
**           stBlkOffset        扇区偏移
** 输　出  : 块设备
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static PLW_BLK_DEV  ataBlkDevCreate (ATA_DRV_HANDLE  hCtrlDrv,
                                     UINT            uiCtrl,
                                     UINT            uiDrive,
                                     size_t          stBlkCnt,
                                     size_t          stBlkOffset)
{
    ATA_DEV_HANDLE          hDev;
    ATA_CTRL_HANDLE         hCtrl;
    ATA_DRIVE_HANDLE        hDrive;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk dev create start.\r\n", uiCtrl, uiDrive);

    if ((!hCtrlDrv) || (uiCtrl >= hCtrlDrv->ATADRV_uiCtrlNum)) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d drv or ctrl invalid argument.\r\n", uiCtrl, uiDrive);
        return  (LW_NULL);
    }

    hCtrl = &hCtrlDrv->ATADRV_tCtrl[uiCtrl];
    if ((!hCtrl) || (!hCtrl->ATACTRL_iDrvInstalled)) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d drv not installed.\r\n", uiCtrl, uiDrive);
        return  (LW_NULL);
    }

    if (uiDrive >= hCtrl->ATACTRL_uiDriveNum) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d drive invalid argument.\r\n", uiCtrl, uiDrive);
        return  (LW_NULL);
    }
    hDrive = &hCtrl->ATACTRL_tDrive[uiDrive];
    if ((!hDrive) ||
        (!hDrive->ATADRIVE_hInfo) ||
        (hDrive->ATADRIVE_iType != ATA_TYPE_ATA) ||
        (hDrive->ATADRIVE_iState != ATA_DEV_OK)) {
        return  (LW_NULL);
    }

    hDev = (ATA_DEV_HANDLE)__SHEAP_ZALLOC(sizeof(ATA_DEV_CB));
    if (!hDev) {
        _ErrorHandle(ENOMEM);
        ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d ata dev alloc failed.\r\n", uiCtrl, uiDrive);
        return  (LW_NULL);
    }
    hDrive->ATADRIVE_hDev     = hDev;
    
    hDev->ATADEV_hDrive       = hDrive;
    hDev->ATADEV_uiCtrl       = uiCtrl;
    hDev->ATADEV_uiDrive      = uiDrive;
    hDev->ATADEV_ullBlkOffset = stBlkOffset;
    hDev->ATADEV_ullBlkCnt    = stBlkCnt;

    ataBlkDevFill(hDev);

    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d blk dev create ok.\r\n", uiCtrl, uiDrive);

    return  ((PLW_BLK_DEV)hDev);
}
/*********************************************************************************************************
** 函数名称: ataDriveConfig
** 功能描述: 配置驱动器
** 输　入  : hCtrlDrv       控制器驱动控制句柄
**           uiCtrl         控制器索引 (ATA_CTRL_MAX)
**           uiDrive        驱动器索引 (ATA_DRIVE_MAX)
**           pvBuff         参数缓冲区
**           iCmd           指定命令
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT  ataDriveConfig (ATA_DRV_HANDLE  hCtrlDrv, UINT  uiCtrl, UINT  uiDrive, CPCHAR cpcName)
{
    ATA_CTRL_HANDLE             hCtrl;
    ATA_DRIVE_HANDLE            hDrive;
    ATA_DRIVE_INFO_HANDLE       hInfo;
    PLW_BLK_DEV                 hBlkDev;
#if (ATA_DISK_MOUNTEX_EN > 0)
    LW_DISKCACHE_ATTR           dcattrl;
#endif

    if ((!hCtrlDrv) || (uiCtrl >= hCtrlDrv->ATADRV_uiCtrlNum)) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d drv or ctrl invalid argument.\r\n", uiCtrl, uiDrive);
        return  (PX_ERROR);
    }

    hCtrl = &hCtrlDrv->ATADRV_tCtrl[uiCtrl];
    if ((!hCtrl) || (!hCtrl->ATACTRL_iDrvInstalled)) {
        ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d drv not installed.\r\n", uiCtrl, uiDrive);
        return  (PX_ERROR);
    }

    if (uiDrive >= hCtrl->ATACTRL_uiDriveNum) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d drive invalid argument.\r\n", uiCtrl, uiDrive);
        return  (PX_ERROR);
    }
    hDrive = &hCtrl->ATACTRL_tDrive[uiDrive];
    if ((!hDrive) ||
        (!hDrive->ATADRIVE_hInfo) ||
        (hDrive->ATADRIVE_iState != ATA_DEV_OK)) {
        return  (PX_ERROR);
    }

    hInfo = hDrive->ATADRIVE_hInfo;

    hBlkDev = ataBlkDevCreate(hCtrlDrv, uiCtrl, uiDrive, 0, 0);
    if (!hBlkDev) {
        ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d drive config blk dev create error.\r\n", uiCtrl, uiDrive);
        return  (PX_ERROR);
    }

#if (ATA_DISK_MOUNTEX_EN > 0)
    dcattrl.DCATTR_pvCacheMem       = LW_NULL;
    dcattrl.DCATTR_bCacheCoherence  = LW_TRUE;

    if (hInfo->ATADINFO_stCacheMemSize) {
        dcattrl.DCATTR_stMemSize = hInfo->ATADINFO_stCacheMemSize;

    } else {
        dcattrl.DCATTR_stMemSize = ATA_CACHE_SIZE;
    }

    if (hInfo->ATADINFO_iBurstRead) {
        dcattrl.DCATTR_iMaxRBurstSector = hInfo->ATADINFO_iBurstRead;

    } else {
        dcattrl.DCATTR_iMaxRBurstSector = ATA_CACHE_BURST_RD;
    }

    if (hInfo->ATADINFO_iBurstWrite) {
        dcattrl.DCATTR_iMaxWBurstSector = hInfo->ATADINFO_iBurstWrite;

    } else {
        dcattrl.DCATTR_iMaxWBurstSector = ATA_CACHE_BURST_WR;
    }

    if (hInfo->ATADINFO_iPipeline) {
        dcattrl.DCATTR_iPipeline = hInfo->ATADINFO_iPipeline;

    } else {
        dcattrl.DCATTR_iPipeline = ATA_CACHE_PL;
    }

    if (hInfo->ATADINFO_iMsgCount) {
        dcattrl.DCATTR_iMsgCount = hInfo->ATADINFO_iMsgCount;

    } else {
        dcattrl.DCATTR_iMsgCount = ATA_CACHE_MSG_CNT;
    }

    if (hInfo->ATADINFO_iPipeline) {
        dcattrl.DCATTR_bParallel = LW_TRUE;

    } else {
        dcattrl.DCATTR_bParallel = LW_FALSE;
    }

    API_OemDiskMount2(ATA_ATA_MEDIA_NAME, hBlkDev, &dcattrl);
    
#else
    API_OemDiskMount(ATA_ATA_MEDIA_NAME,
                     hBlkDev,
                     LW_NULL,
                     (hInfo->ATADINFO_stCacheMemSize) ? hInfo->ATADINFO_stCacheMemSize : ATA_CACHE_SIZE,
                     (hInfo->ATADINFO_iBurstWrite) ? hInfo->ATADINFO_iBurstWrite : ATA_CACHE_BURST_WR);
#endif

    return (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_AtaCtrlStatusCheck
** 功能描述: 驱动器选择
** 输　入  : hCtrl          控制器控制句柄
**           ucMask         掩码
**           ucStatus       状态
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
LW_API
INT  API_AtaCtrlStatusCheck (ATA_CTRL_HANDLE  hCtrl, UINT8  ucMask, UINT8  ucStatus)
{
    REGISTER INT        i;
    UINT8               ucData;
    struct timespec     tvOld;
    struct timespec     tvNow;

    lib_clock_gettime(CLOCK_MONOTONIC, &tvOld);
    for (i = 0; i < hCtrl->ATACTRL_ulSyncTimeoutLoop; i++) {
        ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucData, 1);
        if ((ucData & ucMask) == ucStatus) {
            break;
        }

        lib_clock_gettime(CLOCK_MONOTONIC, &tvNow);
        if ((tvNow.tv_sec - tvOld.tv_sec) >= hCtrl->ATACTRL_ulWdgTimeout) {
            ATA_LOG(ATA_LOG_ERR, "ata ctrl status check timeout %ds data 0x%x mask 0x%x status 0x%x.\r\n",
                    hCtrl->ATACTRL_ulWdgTimeout, ucData, ucMask, ucStatus);
            return  (PX_ERROR);
        }

        ATA_DELAY_400NSEC(hCtrl);
    }

    if (i >= hCtrl->ATACTRL_ulSyncTimeoutLoop) {
        ATA_LOG(ATA_LOG_ERR, "ata ctrl status check loop max %d data 0x%x mask 0x%x status 0x%x.\r\n",
                hCtrl->ATACTRL_ulSyncTimeoutLoop, ucData, ucMask, ucStatus);
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: API_AtaCtrlIrqIsr
** 功能描述: 中断服务程序服务
** 输　入  : pvArg          中断参数
**           ulVector       中断向量
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
LW_API
irqreturn_t  API_AtaCtrlIrqIsr (PVOID  pvArg, ULONG  ulVector)
{
    INT                 iRet = PX_ERROR;
    ATA_CTRL_HANDLE     hCtrl;
    UINT8               ucReg;
    INTREG              iregInterLevel = 0;

    hCtrl = (ATA_CTRL_HANDLE)pvArg;

    iRet = ERROR_NONE;
    if (hCtrl->ATACTRL_pfuncCtrlIntPre) {
        iRet = (*hCtrl->ATACTRL_pfuncCtrlIntPre)(hCtrl->ATACTRL_hCtrlDrv, hCtrl->ATACTRL_uiCtrl);
    }
    if (iRet != ERROR_NONE) {
        return  (LW_IRQ_NONE);
    }

    hCtrl->ATACTRL_ulIntCount++;

    ATA_SPIN_ISR_LOCK(hCtrl, iregInterLevel);
    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), (PVOID)&ucReg, 1);
    if (!(ucReg & ATA_STAT_BUSY)) {
        ATA_IO_BYTES_READ(hCtrl, ATA_STATUS_ADDR(hCtrl), (PVOID)&ucReg, 1);
        hCtrl->ATACTRL_iIntStatus = ucReg;
    
    } else {
        hCtrl->ATACTRL_iIntErrStatus = ucReg;
        hCtrl->ATACTRL_ulIntErrCount++;
    }
    ATA_SPIN_ISR_UNLOCK(hCtrl, iregInterLevel);

    iRet = ERROR_NONE;
    if (hCtrl->ATACTRL_pfuncCtrlIntPost) {
        iRet = (*hCtrl->ATACTRL_pfuncCtrlIntPost)(hCtrl->ATACTRL_hCtrlDrv, hCtrl->ATACTRL_uiCtrl);
    }

    if (iRet == ERROR_NONE) {
        API_SemaphoreBPost(hCtrl->ATACTRL_hSemSync);
        return  (LW_IRQ_HANDLED);
    }

    return  (LW_IRQ_NONE);
}
/*********************************************************************************************************
** 函数名称: API_AtaDrvInit
** 功能描述: 驱动初始化
** 输　入  : hAtaDrv        驱动控制句柄
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
LW_API
INT API_AtaDrvInit (ATA_DRV_HANDLE  hAtaDrv)
{
    REGISTER INT            i;
    REGISTER INT            j;
    INT                     iRet = PX_ERROR;
    ATA_CTRL_CFG_HANDLE     hCtrlCfg;
    ATA_DRV_HANDLE          hDrv;
    CHAR                    cDevName[ATA_DEV_NAME_MAX];

    ATA_LOG(ATA_LOG_PRT,
            "drv: %s index %d ctrl init.\r\n", hAtaDrv->ATADRV_cDrvName, hAtaDrv->ATADRV_uiUnitNumber);

    hDrv = (ATA_DRV_HANDLE)__SHEAP_ZALLOC(sizeof(ATA_DRV_CB));
    if (!hDrv) {                                                        /* 分配控制块失败               */
        ATA_LOG(ATA_LOG_ERR, "drv: %s drv sheap zalloc return null.\r\n", hAtaDrv->ATADRV_cDrvName);
        _ErrorHandle(ENOMEM);
        return  (PX_ERROR);
    }

    lib_memcpy((PVOID)hDrv, (CPVOID)hAtaDrv, sizeof(ATA_DRV_CB));       /* 更新驱动参数                 */

    for (i = 0; i < hDrv->ATADRV_uiCtrlNum; i++) {                      /* 初始化指定控制器             */
        hCtrlCfg = &hDrv->ATADRV_tCtrlCfg[i];
        iRet = ataDrvInit(hDrv, i, hCtrlCfg);
        if (iRet != ERROR_NONE) {
            ATA_LOG(ATA_LOG_PRT, "ctrl %d drv init error.\r\n", i);
        }
    }

    for (i = 0; i < hDrv->ATADRV_uiCtrlNum; i++) {                      /* 初始化指定控制器上的驱动器   */
        hCtrlCfg = &hDrv->ATADRV_tCtrlCfg[i];
        for (j = 0; j < hCtrlCfg->ATACTRLCFG_ucDrives; j++) {
            snprintf(cDevName, ATA_DEV_NAME_MAX, "/atai%dc%dd%d", hDrv->ATADRV_uiUnitNumber, i, j);
            ataDriveConfig(hDrv, i, j, cDevName);
        }
    }

    return  (ERROR_NONE);
}

#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_ATA_EN > 0)         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
