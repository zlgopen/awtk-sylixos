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
** 文   件   名: ataLib.c
**
** 创   建   人: Gong.YuJian (弓羽箭)
**
** 文件创建日期: 2018 年 09 月 03 日
**
** 描        述: ATA 驱动库.
*********************************************************************************************************/
#define  __SYLIXOS_STDIO
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
/*********************************************************************************************************
  裁剪宏
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_ATA_EN > 0)
#include "linux/compat.h"
#include "ata.h"
#include "ataLib.h"
/*********************************************************************************************************
** 函数名称: ataIdString
** 功能描述: 将 ID 值转换为字符串信息
** 输　入  : pusId      ID 参数
**           pcBuff     字符串缓存
**           uiLen      缓存大小
** 输　出  : 字符串信息
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static PCHAR  ataIdString (const UINT16 *pusId, PCHAR  pcBuff, UINT32  uiLen)
{
    REGISTER INT    i;
    UINT8           ucChar   = 0;                                       /* 字节数据                     */
    INT             iOffset  = 0;                                       /* 字偏移                       */
    PCHAR           pcString = pcBuff;                                  /* 字符串缓冲区                 */

    /*
     *  将字数据转换为字符串数据
     */
    iOffset  = 0;
    pcString = pcBuff;
    
    for (i = 0; i < (uiLen - 1); i += 2) {
        ucChar = (UINT8)(pusId[iOffset] >> 8);
        *pcString = ucChar;
        pcString += 1;

        ucChar = (UINT8)(pusId[iOffset] & 0xff);
        *pcString = ucChar;
        pcString += 1;
        iOffset  += 1;
    }

    /*
     *  删除无效空格信息
     */
    pcString = pcBuff + lib_strnlen(pcBuff, uiLen - 1);
    while ((pcString > pcBuff) && (pcString[-1] == ' ')) {
        pcString--;
    }
    *pcString = '\0';

    iOffset  = 0;
    pcString = pcBuff;
    
    for (i = 0; i < uiLen; i++) {
        if (pcString[iOffset] == ' ') {
            pcString += 1;
        } else {
            break;
        }
    }

    return  (pcString);
}
/*********************************************************************************************************
** 函数名称: ataDriveSerialInfoGet
** 功能描述: 序列号信息
** 输　入  : hDrive     驱动器句柄
**           pcBuf      缓冲区
**           stLen      缓冲区大小
** 输　出  : 字符串信息
** 全局变量:
** 调用模块:
*********************************************************************************************************/
PCHAR  ataDriveSerialInfoGet (ATA_DRIVE_HANDLE  hDrive, PCHAR  pcBuf, size_t  stLen)
{
    ATA_PARAM_HANDLE        hParam;                                     /* 参数句柄                     */
    PCHAR                   pcSerial;                                   /* 设备串号                     */
    CHAR                    cSerial[21] = { 0 };                        /* 序列号缓冲区                 */

    if ((!hDrive) || (!pcBuf)) {
        return  (LW_NULL);
    }

    hParam = &hDrive->ATADRIVE_tParam;
    lib_bzero(&cSerial[0], 21);
    pcSerial = ataIdString((const UINT16 *)&hParam->ATAPARAM_ucSerial[0], &cSerial[0], 21);
    lib_strncpy(pcBuf, pcSerial, __MIN(stLen, lib_strlen(pcSerial) + 1));

    return  (pcSerial);
}
/*********************************************************************************************************
** 函数名称: ataDriveFwRevInfoGet
** 功能描述: 固件版本号信息
** 输　入  : hDrive     驱动器句柄
**           pcBuf      缓冲区
**           stLen      缓冲区大小
** 输　出  : 字符串信息
** 全局变量:
** 调用模块:
*********************************************************************************************************/
PCHAR  ataDriveFwRevInfoGet (ATA_DRIVE_HANDLE  hDrive, PCHAR  pcBuf, size_t  stLen)
{
    ATA_PARAM_HANDLE        hParam;                                     /* 参数句柄                     */
    PCHAR                   pcFirmware;                                 /* 固件版本                     */
    CHAR                    cFirmware[9] = { 0 };                       /* 固件版本缓冲区               */

    if ((!hDrive) || (!pcBuf)) {
        return  (LW_NULL);
    }

    hParam = &hDrive->ATADRIVE_tParam;
    lib_bzero(&cFirmware[0], 9);
    pcFirmware = ataIdString((const UINT16 *)&hParam->ATAPARAM_ucFwRev[0], &cFirmware[0], 9);
    lib_strncpy(pcBuf, pcFirmware, __MIN(stLen, lib_strlen(pcFirmware) + 1));

    return  (pcFirmware);
}
/*********************************************************************************************************
** 函数名称: ataDriveModelInfoGet
** 功能描述: 设备详细信息
** 输　入  : hDrive     驱动器句柄
**           pcBuf      缓冲区
**           stLen      缓冲区大小
** 输　出  : 字符串信息
** 全局变量:
** 调用模块:
*********************************************************************************************************/
PCHAR  ataDriveModelInfoGet (ATA_DRIVE_HANDLE  hDrive, PCHAR  pcBuf, size_t  stLen)
{
    ATA_PARAM_HANDLE        hParam;                                     /* 参数句柄                     */
    PCHAR                   pcProduct;                                  /* 产品信息                     */
    CHAR                    cProduct[41] = { 0 };                       /* 产品信息缓冲区               */

    if ((!hDrive) || (!pcBuf)) {
        return  (LW_NULL);
    }

    hParam = &hDrive->ATADRIVE_tParam;
    lib_bzero(&cProduct[0], 41);
    pcProduct = ataIdString((const UINT16 *)&hParam->ATAPARAM_ucModel[0], &cProduct[0], 41);
    lib_strncpy(pcBuf, pcProduct, __MIN(stLen, lib_strlen(pcProduct) + 1));

    return  (pcProduct);
}
/*********************************************************************************************************
** 函数名称: ataSwapBufLe16
** 功能描述: 将小端数据转换换位本地字节序
** 输　入  : pusBuf     控制器句柄
**           stWords    缓冲区字长度
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ataSwapBufLe16 (UINT16 *pusBuf, size_t  stWords)
{
    REGISTER UINT   i;

    for (i = 0; i < stWords; i++) {
        pusBuf[i] = le16_to_cpu(pusBuf[i]);
    }
}
/*********************************************************************************************************
** 函数名称: ataDriveSelect
** 功能描述: 驱动器选择
** 输　入  : hCtrl          控制器控制句柄
**           uiCtrl         控制器索引 (ATA_CTRL_MAX)
**           uiDrive        驱动器索引 (ATA_DRIVE_MAX)
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT ataDriveSelect (ATA_CTRL_HANDLE  hCtrl, UINT  uiCtrl, UINT  uiDrive)
{
    volatile INT        i;
    UINT8               ucData;

    ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
    ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

    ATA_WAIT_STATUS(hCtrl);

    i = 0;
    while (i++ < 10000000) {
        ATA_IO_BYTES_READ(hCtrl, ATA_STATUS_ADDR(hCtrl), &ucData, 1);
        if ((ucData & (ATA_STAT_BUSY | ATA_STAT_DRQ)) == 0) {
            return (ERROR_NONE);
        }
    }

    return  (PX_ERROR);
}
/*********************************************************************************************************
** 函数名称: ataUdmaCableCheck
** 功能描述: UDMA 能力检查
** 输　入  : hCtrl              控制器控制句柄
**           uiCtrl             控制器索引 (ATA_CTRL_MAX)
**           uiDrive            驱动器索引 (ATA_DRIVE_MAX)
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static VOID  ataUdmaCableCheck (ATA_CTRL_HANDLE  hCtrl, UINT  uiCtrl, UINT  uiDrive)
{
    hCtrl->ATACTRL_iUdmaCableOk = LW_TRUE;
}
/*********************************************************************************************************
** 函数名称: API_AtaDriveCommandSend
** 功能描述: 发送驱动器命令
** 输　入  : hCtrl              控制器控制句柄
**           uiCtrl             控制器索引 (ATA_CTRL_MAX)
**           uiDrive            驱动器索引 (ATA_DRIVE_MAX)
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static void ataBestTransferModeFind (ATA_CTRL_HANDLE  hCtrl, UINT  uiCtrl, UINT  uiDrive)
{
    ATA_DRIVE_HANDLE        hDrive   = &hCtrl->ATACTRL_tDrive[uiDrive];
    ATA_PARAM_HANDLE        hParam   = &hDrive->ATADRIVE_tParam;
    UINT16                  usRwMode = hDrive->ATADRIVE_usRwMode;

    switch (usRwMode) {

    case ATA_DMA_ULTRA_5:
    case ATA_DMA_ULTRA_4:
    case ATA_DMA_ULTRA_3:
        ataUdmaCableCheck(hCtrl, uiCtrl, uiDrive);
        if (hCtrl->ATACTRL_iUdmaCableOk == LW_FALSE) {
            usRwMode = ATA_DMA_ULTRA_2;
        }
        break;

    case ATA_DMA_MULTI_2:
        if (hParam->ATAPARAM_usCycletimeDma > 120) {
            usRwMode = ATA_DMA_MULTI_1;
        }
        break;

    case ATA_DMA_MULTI_1:
        if (hParam->ATAPARAM_usCycletimeDma > 180) {
            usRwMode = ATA_DMA_SINGLE_2;
        }
        break;

    case ATA_DMA_SINGLE_2:
        if (hParam->ATAPARAM_usCycletimeDma < 240) {
            break;
        }

    case ATA_DMA_MULTI_0:
    case ATA_DMA_SINGLE_1:
    case ATA_DMA_SINGLE_0:
        usRwMode = (UINT16)(ATA_PIO_W_0 + hDrive->ATADRIVE_usPioMode);
        break;

    default:
        usRwMode = hDrive->ATADRIVE_usRwMode;
    }


    hDrive->ATADRIVE_usRwMode = usRwMode;
    switch (hDrive->ATADRIVE_usRwMode) {

    case ATA_PIO_W_4:
        if (hParam->ATAPARAM_usCycletimePioIordy > 120) {
            usRwMode = ATA_PIO_W_3;
        }

    case ATA_PIO_W_3:
        if (hParam->ATAPARAM_usCycletimePioIordy > 180) {
            usRwMode = ATA_PIO_W_2;
        }
        if (hParam->ATAPARAM_usCycletimePioIordy > 240) {
            usRwMode = ATA_PIO_W_0;
        }

    case ATA_PIO_W_2:
        break;

    case ATA_PIO_W_1:
    case ATA_PIO_W_0:
        usRwMode = ATA_PIO_W_0;
        break;

    default:
        usRwMode = hDrive->ATADRIVE_usRwMode;
    }

    hDrive->ATADRIVE_usRwMode = usRwMode;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d best mode find 0x%x.\r\n", uiCtrl, uiDrive, usRwMode);
}
/*********************************************************************************************************
** 函数名称: ataDevIdentify
** 功能描述: 获取驱动器参数
** 输　入  : hCtrlDrv       控制器驱动控制句柄
**           uiCtrl         控制器索引 (ATA_CTRL_MAX)
**           uiDrive        驱动器索引 (ATA_DRIVE_MAX)
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT ataDevIdentify (ATA_CTRL_HANDLE  hCtrl, UINT  uiCtrl, UINT  uiDrive)
{
    INT                     iRet;
    ATA_DRIVE_HANDLE        hDrive      = &hCtrl->ATACTRL_tDrive[uiDrive];
    UINT32                  uiSignature = hDrive->ATADRIVE_uiSignature;
    UINT8                   ucData;

    if ((uiSignature != ATA_SIGNATURE) && (uiSignature != ATAPI_SIGNATURE)) {
        hDrive->ATADRIVE_iType       = ATA_TYPE_NONE;
        hDrive->ATADRIVE_iState      = ATA_TYPE_NONE;
        hDrive->ATADRIVE_uiSignature = (UINT32)-1;

        ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata dev identify device UNKNOWN 0x%08x.\r\n",
                uiCtrl, uiDrive, uiSignature);
        return  (PX_ERROR);
    }

    iRet = ataDriveSelect(hCtrl, uiCtrl, uiDrive);
    if (iRet != ERROR_NONE) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d driver %d ata dev selected device failed.\r\n", uiCtrl, uiDrive);
        return  (PX_ERROR);
    }

    ucData = (UINT8)0xaa;
    ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
    ucData = (UINT8)0x55;
    ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);

    ATA_IO_BYTES_READ(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
    if (ucData == 0xaa) {
        if (uiSignature == ATAPI_SIGNATURE) {
            hDrive->ATADRIVE_iType = ATA_TYPE_ATAPI;
            ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ATAPI device find.\r\n", uiCtrl, uiDrive);
            return  (ERROR_NONE);

        } else if (uiSignature == ATA_SIGNATURE) {
            hDrive->ATADRIVE_iType = ATA_TYPE_ATA;
            ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ATA device find.\r\n", uiCtrl, uiDrive);
            return(ERROR_NONE);

        } else {
            hDrive->ATADRIVE_iType       = ATA_TYPE_NONE;
            hDrive->ATADRIVE_iState      = ATA_TYPE_NONE;
            hDrive->ATADRIVE_uiSignature = 0;

            ATA_LOG(ATA_LOG_ERR, "ctrl %d driver %d UNKNOWN device find.\r\n", uiCtrl, uiDrive);
            return  (PX_ERROR);
        }

    } else {
        hDrive->ATADRIVE_iType       = ATA_TYPE_NONE;
        hDrive->ATADRIVE_iState      = ATA_TYPE_NONE;
        hDrive->ATADRIVE_uiSignature = (UINT32)-1;

        ATA_LOG(ATA_LOG_ERR, "ctrl %d driver %d UNKNOWN device find.\r\n", uiCtrl, uiDrive);
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** 函数名称: ataDriveParamRead
** 功能描述: 获取驱动器参数
** 输　入  : hCtrlDrv       控制器驱动控制句柄
**           uiCtrl         控制器索引 (ATA_CTRL_MAX)
**           uiDrive        驱动器索引 (ATA_DRIVE_MAX)
**           pvBuff         参数缓冲区
**           iCmd           指定命令
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT ataDriveParamRead (ATA_CTRL_HANDLE  hCtrl,
                              UINT             uiCtrl,
                              UINT             uiDrive,
                              PVOID            pvBuff,
                              INT              iCmd)
{
    INT         iRet;
    INT         iRetry = LW_TRUE;
    UINT8       ucData;
    INT         iRetryCount = 0;

    while (iRetry) {
        iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, 0);
        if (iRet != ERROR_NONE) {
            continue;
        }

        ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

        ucData = (UINT8)iCmd;
        ATA_IO_BYTES_WRITE(hCtrl, ATA_COMMAND_ADDR(hCtrl), &ucData, 1);

        ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d param read wait for interrupt.\r\n", uiCtrl, uiDrive);

        if (hCtrl->ATACTRL_hCtrlDrv->ATADRV_iIntDisable) {
            iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ, ATA_STAT_DRQ);
        } else {
            iRet = API_SemaphoreBPend(hCtrl->ATACTRL_hSemSync, LW_TICK_HZ / 5);
        }
        if ((hCtrl->ATACTRL_iIntStatus & ATA_STAT_ERR) ||
            (iRet != ERROR_NONE)) {
            ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucData, 1);
            ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d astatus 0x%x int status 0x%x.\r\n",
                    uiCtrl, uiDrive, ucData, hCtrl->ATACTRL_iIntStatus);

            ATA_IO_BYTES_READ(hCtrl, ATA_ERROR_ADDR(hCtrl), &ucData, 1);
            ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d error 0x%x int status 0x%x.\r\n",
                    uiCtrl, uiDrive, ucData, hCtrl->ATACTRL_iIntStatus);

            if (++iRetryCount > ATA_RETRY_NUM) {
                return  (PX_ERROR);

            } else {
                ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d try to read params again.\r\n", uiCtrl, uiDrive);
            }

        } else {
            iRetry = LW_FALSE;
        }
    }

    iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_DRQ | ATA_STAT_BUSY, ATA_STAT_DRQ);
    if (iRet != ERROR_NONE) {
        return  (PX_ERROR);
    }

    ATA_IO_WORDS_READ(hCtrl, ATA_DATA_ADDR(hCtrl), pvBuff, 256);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataCtrlReset
** 功能描述: 控制器复位
** 输　入  : hCtrl      控制器控制句柄
**           uiCtrl     控制器索引 (ATA_CTRL_MAX)
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT ataCtrlReset (ATA_CTRL_HANDLE  hCtrl, UINT  uiCtrl)
{
    ATA_DRIVE_HANDLE        hDrive;
    UINT32                  uiSignature;
    UINT                    uiDrive;
    UINT8                   ucData;
    volatile INT            i;

    ucData = (UINT8)(ATA_CTL_SRST | ATA_CTL_NIEN);
    ATA_IO_BYTES_WRITE(hCtrl, ATA_D_CONTROL_ADDR(hCtrl), &ucData, 1);

    for (i = 0; i < 100; i++) {
        ATA_DELAY_400NSEC(hCtrl);
    }

    ucData = (UINT8)0;
    ATA_IO_BYTES_WRITE(hCtrl, ATA_D_CONTROL_ADDR(hCtrl), &ucData, 1);

    for (i = 0; i < 10000; i++) {
        ATA_DELAY_400NSEC(hCtrl);
    }

    i = 0;
    do {
        if (++i == 30) {
            ATA_LOG(ATA_LOG_PRT, "ctrl %d reset timeout.\r\n", uiCtrl);
            return  (PX_ERROR);
        }

        ucData = 0;
        ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucData, 1);
    } while (ucData & ATA_STAT_BUSY);

    ATA_LOG(ATA_LOG_PRT, "ctrl %d reset retry %d.\r\n", uiCtrl, i);

    for (uiDrive = 0; uiDrive < hCtrl->ATACTRL_uiDriveNum; uiDrive++) {
        hDrive = &hCtrl->ATACTRL_tDrive[uiDrive];

        ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
        ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

        hDrive->ATADRIVE_uiSignature = 0;
        ATA_IO_BYTES_READ(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_uiSignature |= ucData << 24;
        ATA_IO_BYTES_READ(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_uiSignature |= ucData << 16;
        ATA_IO_BYTES_READ(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_uiSignature |= ucData << 8;
        ATA_IO_BYTES_READ(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_uiSignature |= ucData << 0;

        uiSignature = hDrive->ATADRIVE_uiSignature;
        if (uiSignature == ATA_SIGNATURE) {
            ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ATA device 0x%08x.\r\n", uiCtrl, uiDrive,uiSignature);

        } else if (uiSignature == ATAPI_SIGNATURE) {
            ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ATAPI device 0x%08x.\r\n",uiCtrl,uiDrive,uiSignature);

        } else {
            ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ERROR device 0x%08x.\r\n",uiCtrl,uiDrive,uiSignature);
        }
    }

    ATA_LOG(ATA_LOG_PRT, "ctrl %d reset ok.\r\n", uiCtrl);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataDevInit
** 功能描述: 设备初始化
** 输　入  : hCtrlDrv       控制器驱动控制句柄
**           uiCtrl         控制器索引 (ATA_CTRL_MAX)
**           uiDrive        驱动器索引 (ATA_DRIVE_MAX)
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
static INT ataDevInit (ATA_CTRL_HANDLE  hCtrl, UINT  uiCtrl, UINT  uiDrive)
{
    volatile INT        i;
    INT                 iRet;
    UINT8               ucData;
    UINT32              uiDevType;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata dev int.\r\n", uiCtrl, uiDrive);

    ucData = ATA_CTL_SRST;
    ATA_IO_BYTES_WRITE(hCtrl, ATA_D_CONTROL_ADDR(hCtrl), &ucData, 1);

    for (i = 0; i < 20; i++) {
        ATA_DELAY_400NSEC(hCtrl);
    }

    ucData = 0;
    ATA_IO_BYTES_WRITE(hCtrl, ATA_D_CONTROL_ADDR(hCtrl), &ucData, 1);

    for (i = 0; i < 5000; i++) {                                        /* 2 ms                         */
        ATA_DELAY_400NSEC(hCtrl);
    }

    iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_BUSY, 0);
    if (iRet != ERROR_NONE) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d driver %d ata dev busy.\r\n", uiCtrl, uiDrive);
        return  (PX_ERROR);
    }

    if (hCtrl->ATACTRL_hSemSync == LW_OBJECT_HANDLE_INVALID) {
        hCtrl->ATACTRL_hSemSync  = API_SemaphoreBCreate("ata_sync",
                                                        LW_FALSE,
                                                        (LW_OPTION_WAIT_FIFO | LW_OPTION_OBJECT_GLOBAL),
                                                        LW_NULL);
    }

    uiDevType = 0;
    ATA_IO_BYTES_READ(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
    uiDevType |= ucData << 24;
    ATA_IO_BYTES_READ(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);
    uiDevType |= ucData << 16;
    ATA_IO_BYTES_READ(hCtrl, ATA_CYLLO_ADDR(hCtrl), &ucData, 1);
    uiDevType |= ucData << 8;
    ATA_IO_BYTES_READ(hCtrl, ATA_CYLHI_ADDR(hCtrl), &ucData, 1);
    uiDevType |= ucData << 0;
    if (uiDevType == ATA_SIGNATURE) {
        ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata dev int device ATA.\r\n", uiCtrl, uiDrive);

    } else if (uiDevType == ATAPI_SIGNATURE) {
        ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata dev int device ATAPI.\r\n", uiCtrl, uiDrive);

    } else {
        ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata dev int device UNKNOWN.\r\n", uiCtrl, uiDrive);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataCtrlDelay
** 功能描述: 发送驱动器命令
** 输　入  : hCtrl      控制器控制句柄
** 输　出  : NONE
** 全局变量:
** 调用模块:
*********************************************************************************************************/
VOID  ataCtrlDelay (ATA_CTRL_HANDLE  hCtrl)
{
    UINT8       ucReg;

    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucReg, 1);
    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucReg, 1);
    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucReg, 1);
    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucReg, 1);
    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucReg, 1);
    ATA_IO_BYTES_READ(hCtrl, ATA_A_STATUS_ADDR(hCtrl), &ucReg, 1);
}
/*********************************************************************************************************
** 函数名称: ataDriveCommandSend
** 功能描述: 发送驱动器命令
** 输　入  : hCtrl              控制器控制句柄
**           uiCtrl             控制器索引 (ATA_CTRL_MAX)
**           uiDrive            驱动器索引 (ATA_DRIVE_MAX)
**           iCmd               命令
**           uiFeature          功能寄存器
**           usSectorCnt        扇区数量
**           usSectorNum        扇区编号
**           ucCylLo            柱面数量低
**           ucCylHi            柱面数量高
**           ucSdh              SDH 寄存器
**           iFlags             标志参数
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT  ataDriveCommandSend (ATA_CTRL_HANDLE  hCtrl,
                          UINT             uiCtrl,
                          UINT             uiDrive,
                          INT              iCmd,
                          UINT32           uiFeature,
                          UINT16           usSectorCnt,
                          UINT16           usSectorNum,
                          UINT8            ucCylLo,
                          UINT8            ucCylHi,
                          UINT8            ucSdh)
{
    INT                         iRet;
    ATA_DRIVE_HANDLE            hDrive = &hCtrl->ATACTRL_tDrive[uiDrive];
    ATA_DRIVE_INFO_HANDLE       hInfo  = hDrive->ATADRIVE_hInfo;
    BOOL                        bRetry = LW_TRUE;
    INT                         iRetryCount = 0;
    UINT8                       ucData;

    ATA_LOG(ATA_LOG_PRT,
            "ctrl %d drive %d cmd 0x%x feature 0x%x sector cnt 0x%x sector num 0x%x"
            " cylinder low  0x%x cylinder high 0x%x sdh 0x%x.\r\n",
            uiCtrl, uiDrive, iCmd, uiFeature, usSectorCnt, usSectorNum, ucCylLo, ucCylHi, ucSdh);

    while (bRetry) {
        ataDriveSelect(hCtrl, uiCtrl, uiDrive);

        ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d cmd send start.\r\n", uiCtrl, uiDrive);

        switch (iCmd) {

        case ATA_CMD_INITP:
            if (hDrive->ATADRIVE_iType == ATA_TYPE_ATAPI) {
                ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d atapi ATA_CMD_INITP error.\r\n", uiCtrl, uiDrive);
                return  (ERROR_NONE);
            }

            ucData = (UINT8)hInfo->ATADINFO_uiSectors;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)((uiDrive << ATA_DRIVE_BIT) | ((hInfo->ATADINFO_uiHeads - 1) & 0x0f));
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

            ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d cmd CMD_INITP CNT 0x%x ATA_SDH_D_SELECT 0x%x.\r\n",
                    uiCtrl, uiDrive, hInfo->ATADINFO_uiSectors,
                    (uiDrive << ATA_DRIVE_BIT) | ((hInfo->ATADINFO_uiHeads - 1) & 0x0f));
            break;

        case ATA_CMD_RECALIB:
            ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

            ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d cmd ATA_CMD_RECALIB ATA_SDH_D_SELECT 0x%x.\r\n",
                    uiCtrl, uiDrive, uiDrive << ATA_DRIVE_BIT);
            break;

        case ATAPI_CMD_SRST:
            if (hDrive->ATADRIVE_iType == ATA_TYPE_ATA) {
                ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d ata ATAPI_CMD_SRST error.\r\n", uiCtrl, uiDrive);
                return  (ERROR_NONE);
            }

            ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_DIAGNOSE:
            break;

        case ATA_CMD_SEEK:
            if (hDrive->ATADRIVE_iType == ATA_TYPE_ATA) {

                ucData = (UINT8)uiFeature;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);

                ucData = (UINT8)(uiFeature >> 8);
                ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);

                ucData = (UINT8)(hDrive->ATADRIVE_ucOkLba |
                                 (uiDrive << ATA_DRIVE_BIT) |
                                 (usSectorCnt & 0xff));
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

            } else {
                ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d atapi ATA_CMD_SEEK error.\r\n", uiCtrl, uiDrive);
                return  (PX_ERROR);
            }
            break;

        case ATA_CMD_SET_FEATURE:
            ucData = (UINT8)usSectorCnt;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)uiFeature;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_FEATURE_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

            bRetry = LW_FALSE;
            break;

        case ATA_CMD_SET_MULTI:
            if (hDrive->ATADRIVE_iType == ATA_TYPE_ATA) {
                ucData = (UINT8)uiFeature;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);

                ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

            } else {
                ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d atapi CMD_SET_MULTI error.\r\n", uiCtrl, uiDrive);
                return  (PX_ERROR);
            }
            break;

        case ATA_CMD_IDLE:
        case ATA_CMD_STANDBY:
            ucData = (UINT8)uiFeature;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

            bRetry = LW_FALSE;
            break;

        case ATA_CMD_STANDBY_IMMEDIATE:
        case ATA_CMD_IDLE_IMMEDIATE:
        case ATA_CMD_SLEEP:
        case ATA_CMD_CHECK_POWER_MODE:
        case ATA_CMD_FLUSH_CACHE:
            ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_SECURITY_ERASE_PREPARE:
        case ATA_CMD_SECURITY_FREEZE_LOCK:
            ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_SECURITY_ERASE_UNIT:
        case ATA_CMD_SECURITY_DISABLE_PASSWORD:
        case ATA_CMD_SECURITY_SET_PASSWORD:
        case ATA_CMD_SECURITY_UNLOCK:
            break;

        case ATA_CMD_SMART:
            switch (uiFeature) {

            case ATA_SMART_ATTRIB_AUTO:
                ucData = (UINT8)usSectorCnt;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
                break;

            case ATA_SMART_READ_LOG_SECTOR:
            case ATA_SMART_WRITE_LOG_SECTOR:
                ucData = (UINT8)usSectorCnt;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);

                ucData = (UINT8)usSectorNum;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);
                break;

            case ATA_SMART_OFFLINE_IMMED:
                ucData = (UINT8)usSectorCnt;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);
                break;

            case ATA_SMART_RETURN_STATUS:
            case ATA_SMART_ENABLE_OPER:
            case ATA_SMART_SAVE_ATTRIB:
            case ATA_SMART_READ_DATA:
            case ATA_SMART_DISABLE_OPER:
                break;

            default:
                ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d smart sub cmd 0x%x error.\r\n",
                        uiCtrl, uiDrive, uiFeature);
                return  (PX_ERROR);
            }

            ucData = (UINT8)0x4f;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)0xc2;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)uiFeature;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_FEATURE_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_GET_MEDIA_STATUS:
            ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_MEDIA_EJECT:
        case ATA_CMD_MEDIA_LOCK:
        case ATA_CMD_MEDIA_UNLOCK:
            if (hDrive->ATADRIVE_iType == ATA_TYPE_ATA) {
                ucData = (UINT8)(uiDrive << ATA_DRIVE_BIT);
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);

            } else {
                ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d atapi media commands error.\r\n", uiCtrl, uiDrive);
                return  (PX_ERROR);
            }
            break;

        case ATA_CMD_CFA_ERASE_SECTORS:
        case ATA_CMD_CFA_WRITE_MULTIPLE_WITHOUT_ERASE:
        case ATA_CMD_CFA_WRITE_SECTORS_WITHOUT_ERASE:
            ucData = (UINT8)usSectorCnt;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_CFA_TRANSLATE_SECTOR:
            ucData = (UINT8)usSectorNum;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)ucCylLo;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)ucCylHi;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);

            ucData = (UINT8)(hDrive->ATADRIVE_ucOkLba | (uiDrive << ATA_DRIVE_BIT) | (ucSdh & 0xff));
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_CFA_REQUEST_EXTENDED_ERROR_CODE:
            break;

        case ATA_CMD_READ_NATIVE_MAX_ADDRESS:
            ucData = (UINT8)(hDrive->ATADRIVE_ucOkLba | (uiDrive << ATA_DRIVE_BIT) | (ucSdh & 0xff));
            ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_SET_MAX:
            switch (uiFeature) {

            case ATA_SUB_SET_MAX_ADDRESS:
                ucData = (UINT8)usSectorCnt;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);

                ucData = (UINT8)usSectorNum;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);

                ucData = (UINT8)ucCylLo;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);

                ucData = (UINT8)ucCylHi;
                ATA_IO_BYTES_WRITE(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);

                ucData = (UINT8)(hDrive->ATADRIVE_ucOkLba | (uiDrive << ATA_DRIVE_BIT) | (ucSdh & 0xff));
                ATA_IO_BYTES_WRITE(hCtrl, ATA_SDH_D_SELECT_ADDR(hCtrl), &ucData, 1);
                break;

            case ATA_SUB_SET_MAX_SET_PASS:
            case ATA_SUB_SET_MAX_LOCK:
            case ATA_SUB_SET_MAX_UNLOCK:
            case ATA_SUB_SET_MAX_FREEZE_LOCK:
                break;

            default:
                ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d set max sub cmd 0x%x not support.\r\n",
                        uiCtrl, uiDrive, uiFeature);
                return  (PX_ERROR);
            }

            ucData = (UINT8)uiFeature;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_FEATURE_ADDR(hCtrl), &ucData, 1);
            break;

        case ATA_CMD_READ_VERIFY_SECTORS:
            break;

        case ATA_CMD_NOP:
            ucData = (UINT8)uiFeature;
            ATA_IO_BYTES_WRITE(hCtrl, ATA_FEATURE_ADDR(hCtrl), &ucData, 1);
            break;

        default:
            ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d ata cmd 0x%x not support.\r\n", uiCtrl, uiDrive, iCmd);
            return  (PX_ERROR);
        }

        ucData = (UINT8)iCmd;
        ATA_IO_BYTES_WRITE(hCtrl, ATA_COMMAND_ADDR(hCtrl), &ucData, 1);

        if (hCtrl->ATACTRL_hCtrlDrv->ATADRV_iIntDisable) {
            iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_READY | ATA_STAT_BUSY, ATA_STAT_READY);
        } else {
            iRet = API_SemaphoreBPend(hCtrl->ATACTRL_hSemSync, hCtrl->ATACTRL_ulSemSyncTimeout);
        }
        if ((hCtrl->ATACTRL_iIntStatus & ATA_STAT_ERR) || (iRet != ERROR_NONE)) {
            if (hCtrl->ATACTRL_iIntStatus & ATA_STAT_ERR) {
                ATA_IO_BYTES_READ(hCtrl, ATA_ERROR_ADDR(hCtrl), &ucData, 1);
                if (ucData & ATA_ERR_ABRT) {
                    ATA_LOG(ATA_LOG_PRT,
                            "ctrl %d drive %d cmd abort int status 0x%x ret %d(0x%x) error data 0x%x"
                            " cmd 0x%0x feature 0x%0x sector cnt 0x%x sector num 0x%x"
                            " cyl low 0x%x cyl high 0x%x sdh 0x%x.\r\n",
                            uiCtrl, uiDrive, hCtrl->ATACTRL_iIntStatus, iRet, iRet, ucData,
                            iCmd, uiFeature, usSectorCnt, usSectorNum, ucCylLo, ucCylHi, ucSdh);

                    return  (PX_ERROR);
                }

            } else {
                ATA_LOG(ATA_LOG_PRT,
                        "ctrl %d drive %d ata cmd 0x%x retrying int status 0x%x"
                        " sem ret 0x%x error data 0x%x.\r\n",
                        uiCtrl, uiDrive, iCmd, hCtrl->ATACTRL_iIntStatus, iRet, ucData);
            }

            if (++iRetryCount > ATA_RETRY_NUM) {
                ATA_LOG(ATA_LOG_ERR, "ctrl %d drive %d ata cmd 0x%x retry cnt %d [0 - %d].\r\n",
                        uiCtrl, uiDrive, iCmd, iRetryCount, ATA_RETRY_NUM);

                return  (PX_ERROR);
            }

        } else {
            bRetry = LW_FALSE;
        }
    }

    switch (iCmd) {

    case ATA_CMD_SEEK:
        iRet = API_AtaCtrlStatusCheck(hCtrl, ATA_STAT_SEEKCMPLT, ATA_STAT_SEEKCMPLT);
        if (iRet != ERROR_NONE) {
            return  (PX_ERROR);
        }
        break;

    case ATA_CMD_DIAGNOSE:
        iRet = API_AtaCtrlStatusCheck(hCtrl, (ATA_STAT_BUSY|ATA_STAT_READY|ATA_STAT_DRQ), ATA_STAT_READY);
        if (iRet != ERROR_NONE) {
            return  (PX_ERROR);
        }

        hDrive->ATADRIVE_uiSignature = 0;
        ATA_IO_BYTES_READ(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_uiSignature |= ucData << 24;
        ATA_IO_BYTES_READ(hCtrl, ATA_SECTOR_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_uiSignature |= ucData << 16;
        ATA_IO_BYTES_READ(hCtrl, ATA_CYLLO_BCNTLO_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_uiSignature |= ucData << 8;
        ATA_IO_BYTES_READ(hCtrl, ATA_CYLHI_BCNTHI_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_uiSignature |= ucData << 0;

        ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d ata cmd 0x%x ATA_CMD_DIAGNOSE signature 0x%x.\r\n",
                uiCtrl, uiDrive, iCmd, hDrive->ATADRIVE_uiSignature);
        break;

    case ATA_CMD_CHECK_POWER_MODE:
        ATA_IO_BYTES_READ(hCtrl, ATA_SECCNT_ADDR(hCtrl), &ucData, 1);
        hDrive->ATADRIVE_ucCheckPower = ucData;

        ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d ata cmd 0x%x ATA_CMD_CHECK_POWER_MODE mode 0x%x.\r\n",
                uiCtrl, uiDrive, iCmd, hDrive->ATADRIVE_ucCheckPower);
        break;

    case ATA_CMD_NOP:
        ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d cmd 0x%x ATA_CMD_NOP aborted.\r\n", uiCtrl, uiDrive, iCmd);
        break;

    case ATA_CMD_READ_VERIFY_SECTORS:
    case ATA_CMD_FLUSH_CACHE:
        break;

    default:
        break;
    }

    ATA_LOG(ATA_LOG_PRT, "ctrl %d drive %d ata cmd 0x%x ok.\r\n", uiCtrl, uiDrive, iCmd);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataDriveInit
** 功能描述: 驱动器初始化
** 输　入  : hCtrlDrv       控制器驱动控制句柄
**           uiCtrl         控制器索引 (ATA_CTRL_MAX)
**           uiDrive        驱动器索引 (ATA_DRIVE_MAX)
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT ataDriveInit (ATA_CTRL_HANDLE  hCtrl, UINT  uiCtrl, UINT  uiDrive)
{
    INT                         iRet         = PX_ERROR;
    ATA_DRIVE_HANDLE            hDrive       = &hCtrl->ATACTRL_tDrive[uiDrive];
    ATA_PARAM_HANDLE            hParam       = &hDrive->ATADRIVE_tParam;
    ATA_DRIVE_INFO_HANDLE       hInfo        = hDrive->ATADRIVE_hInfo;
    UINT32                      uiConfigType = hCtrl->ATACTRL_uiConfigType;
    FUNCPTR                     pfuncDmaCtrlMode;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drive int.\r\n", uiCtrl, uiDrive);

    iRet = API_SemaphoreMPend(hCtrl->ATACTRL_hSemDev, hCtrl->ATACTRL_ulSemSyncTimeout * 2);
    if (iRet != ERROR_NONE) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d driver %d ata drive int get dev lock failed.\r\n", uiCtrl, uiDrive);
        return(PX_ERROR);
    }

    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drive int identify.\r\n", uiCtrl, uiDrive);

    iRet = ataDevIdentify(hCtrl, uiCtrl, uiDrive);
    if (iRet != ERROR_NONE) {
        hDrive->ATADRIVE_iState = ATA_DEV_NONE;
        goto  __error_handle;
    }

    hDrive->ATADRIVE_iDmaUse = LW_FALSE;

    if (hDrive->ATADRIVE_iType == ATA_TYPE_ATA) {
        hDrive->ATADRIVE_pfuncDevReset = LW_NULL;

        iRet = ataDriveParamRead(hCtrl, uiCtrl, uiDrive, (PVOID)hParam, ATA_CMD_IDENT_DEV);
        if (iRet == ERROR_NONE) {
            if ((uiConfigType & ATA_GEO_MASK) != ATA_GEO_FORCE) {
#if LW_CFG_CPU_ENDIAN == 1
                if (hCtrl->ATACTRL_hCtrlDrv->ATADRV_iBeSwap) {
                    ataSwapBufLe16((UINT16 *)hParam, (size_t)(256));
                }
#endif
            }

            if ((uiConfigType & ATA_GEO_MASK) == ATA_GEO_FORCE) {
                ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drive int config type ATA_GEO_FORCE.\r\n",
                        uiCtrl, uiDrive);

                ataDriveCommandSend(hCtrl, uiCtrl, uiDrive, ATA_CMD_INITP, 0, 0, 0, 0, 0, 0);
                ataDriveParamRead(hCtrl, uiCtrl, uiDrive, (PVOID)hParam, ATA_CMD_IDENT_DEV);

#if LW_CFG_CPU_ENDIAN == 1
                if (hCtrl->ATACTRL_hCtrlDrv->ATADRV_iBeSwap) {
                    ataSwapBufLe16((UINT16 *)hParam, (size_t)(256));
                }
#endif

            } else if ((uiConfigType & ATA_GEO_MASK) == ATA_GEO_PHYSICAL) {
                hInfo->ATADINFO_uiCylinders = hParam->ATAPARAM_usCylinders;
                hInfo->ATADINFO_uiHeads     = hParam->ATAPARAM_usHeads;
                hInfo->ATADINFO_uiSectors   = hParam->ATAPARAM_usSectors;

                ATA_LOG(ATA_LOG_PRT,
                        "ctrl %d driver %d ata drive ATA_GEO_PHYSICAL"
                        "cylinders %d heads %d sectors %d.\r\n",
                        uiCtrl, uiDrive,
                        hInfo->ATADINFO_uiCylinders, hInfo->ATADINFO_uiHeads, hInfo->ATADINFO_uiSectors);

            } else if ((uiConfigType & ATA_GEO_MASK) == ATA_GEO_CURRENT) {
                ATA_LOG(ATA_LOG_PRT,
                        "ctrl %d driver %d ata drive ATA_GEO_CURRENT"
                        " logical_sector_size[0] %d logical_sector_size[1] %d phys_logical_size %d.\r\n",
                        uiCtrl, uiDrive,
                        hParam->ATAPARAM_usLogicSectorSize[0], hParam->ATAPARAM_usLogicSectorSize[1],
                        hParam->ATAPARAM_usPhysicalLogicalSector);

                if ((hParam->ATAPARAM_usCurrentCylinders) &&
                    (hParam->ATAPARAM_usCurrentHeads) &&
                    (hParam->ATAPARAM_usCurrentSectors)) {
                    hInfo->ATADINFO_uiCylinders = hParam->ATAPARAM_usCurrentCylinders;
                    hInfo->ATADINFO_uiHeads = hParam->ATAPARAM_usCurrentHeads;
                    hInfo->ATADINFO_uiSectors = hParam->ATAPARAM_usCurrentSectors;

                    ATA_LOG(ATA_LOG_PRT,
                            "ctrl %d driver %d ata drive ATA_GEO_CURRENT1"
                            " cylinders %d heads %d sectors %d.\r\n",
                            uiCtrl, uiDrive,
                            hInfo->ATADINFO_uiCylinders,
                            hInfo->ATADINFO_uiHeads,
                            hInfo->ATADINFO_uiSectors);

                } else {
                    hInfo->ATADINFO_uiCylinders = hParam->ATAPARAM_usCylinders;
                    hInfo->ATADINFO_uiHeads = hParam->ATAPARAM_usHeads;
                    hInfo->ATADINFO_uiSectors = hParam->ATAPARAM_usSectors;

                    ATA_LOG(ATA_LOG_PRT,
                            "ctrl %d driver %d ata drive ATA_GEO_CURRENT2"
                            " cylinders %d heads %d sectors %d.\r\n",
                            uiCtrl, uiDrive,
                            hInfo->ATADINFO_uiCylinders,
                            hInfo->ATADINFO_uiHeads,
                            hInfo->ATADINFO_uiSectors);
                }
            }

            if (hParam->ATAPARAM_usCapabilities & ATA_IOLBA_MASK) {
                hDrive->ATADRIVE_ullCapacity = ((UINT64)hParam->ATAPARAM_usSectors0) |
                                               ((UINT64)hParam->ATAPARAM_usSectors1 << 16);

                ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drive reports LBA (60-61) 0x%llx.\r\n",
                        uiCtrl, uiDrive, hDrive->ATADRIVE_ullCapacity);
            }

            if (hDrive->ATADRIVE_ullCapacity == (UINT64)ATA_LBA28_MAX) {
                hDrive->ATADRIVE_ullCapacity = ((UINT64)hParam->ATAPARAM_usLba48Size[0])       |
                                               ((UINT64)hParam->ATAPARAM_usLba48Size[1] << 16) |
                                               ((UINT64)hParam->ATAPARAM_usLba48Size[2] << 32) |
                                               ((UINT64)hParam->ATAPARAM_usLba48Size[3] << 48);
                hDrive->ATADRIVE_iUseLba48 = LW_TRUE;
            }

            if (hDrive->ATADRIVE_ullCapacity > ATA_LBA48_MAX) {
                goto  __error_handle;
            }

            ataDriveCommandSend(hCtrl, uiCtrl, uiDrive, ATA_CMD_INITP, 0, 0, 0, 0, 0, 0);

            if ((hParam->ATAPARAM_usMajorRevNum == 0) ||
                ((~hParam->ATAPARAM_usMajorRevNum) == 0) ||
                ((hParam->ATAPARAM_usMajorRevNum & 0xf) != 0)) {
                ataDriveCommandSend(hCtrl, uiCtrl, uiDrive, ATA_CMD_RECALIB, 0, 0, 0, 0, 0, 0);
            }

        } else {
            hDrive->ATADRIVE_iState = ATA_DEV_PREAD_ERR;
            goto  __error_handle;
        }

    } else if (hDrive->ATADRIVE_iType == ATA_TYPE_ATAPI) {
        goto  __error_handle;
    }

    hDrive->ATADRIVE_iOkRemovable      = (hParam->ATAPARAM_usConfig & ATA_CONFIG_REMOVABLE) ?
                                         LW_TRUE : LW_FALSE;
    hDrive->ATADRIVE_iOkInterleavedDMA = (hParam->ATAPARAM_usCapabilities & ATA_INTER_DMA_MASK) ?
                                         LW_TRUE :LW_FALSE;
    hDrive->ATADRIVE_iOkCommandQue     = (hParam->ATAPARAM_usCapabilities & ATA_CMD_QUE_MASK  ) ?
                                         LW_TRUE : LW_FALSE;
    hDrive->ATADRIVE_iOkOverlap        = (hParam->ATAPARAM_usCapabilities & ATA_OVERLAP_MASK  ) ?
                                         LW_TRUE : LW_FALSE;
    hDrive->ATADRIVE_usMultiSecs       = (UINT16)(hParam->ATAPARAM_usMultiSecs & ATA_MULTISEC_MASK);
    hDrive->ATADRIVE_iOkMulti          = (UINT16)((hDrive->ATADRIVE_usMultiSecs) ? LW_TRUE : LW_FALSE);
    hDrive->ATADRIVE_iOkIoRdy          = (UINT16)((hParam->ATAPARAM_usCapabilities & ATA_IORDY_MASK)  ?
                                         LW_TRUE : LW_FALSE);
    hDrive->ATADRIVE_ucOkLba           = (UINT8)((hParam->ATAPARAM_usCapabilities & ATA_IOLBA_MASK)  ?
                                         ATA_USE_LBA : 0);
    hDrive->ATADRIVE_iOkDma            = (hParam->ATAPARAM_usCapabilities & ATA_DMA_CAP_MASK) ?
                                         LW_TRUE : LW_FALSE;

    ATA_LOG(ATA_LOG_PRT,
            "ctrl %d driver %d ata drive int \r\n"
            "   hDrive->ATADRIVE_usMultiSecs    0x%x \r\n"
            "   hDrive->ATADRIVE_iOkMulti       0x%x\r\n"
            "   hDrive->ATADRIVE_iOkIoRdy       0x%x\r\n"
            "   hDrive->ATADRIVE_ucOkLba        0x%x\r\n"
            "   hDrive->ATADRIVE_iOkDma         0x%x\r\n"
            "   hDrive->ATADRIVE_iOkRemovable   0x%x.\r\n"
            "   hDrive->ATADRIVE_iUseLba48      0x%x.\r\n",
            uiCtrl, uiDrive,
            hDrive->ATADRIVE_usMultiSecs, hDrive->ATADRIVE_iOkMulti,
            hDrive->ATADRIVE_iOkIoRdy, hDrive->ATADRIVE_ucOkLba,
            hDrive->ATADRIVE_iOkDma, hDrive->ATADRIVE_iOkRemovable, hDrive->ATADRIVE_iUseLba48);

    hDrive->ATADRIVE_usPioMode       = 0xff;
    hDrive->ATADRIVE_usSingleDmaMode = 0xff;
    hDrive->ATADRIVE_usMultiDmaMode  = 0xff;
    hDrive->ATADRIVE_usUltraDmaMode  = 0xff;
    hDrive->ATADRIVE_usPioMode = (UINT16)((hParam->ATAPARAM_usPioMode >> 8) & ATA_PIO_MASK_012);

    if (hDrive->ATADRIVE_usPioMode > ATA_SET_PIO_MODE_2) {
        hDrive->ATADRIVE_usPioMode = ATA_SET_PIO_MODE_0;
    }

    if ((hDrive->ATADRIVE_iOkIoRdy) && (hParam->ATAPARAM_usValid & ATA_WORD64_70_VALID)) {
        if (hParam->ATAPARAM_usAdvancedPio & ATA_BIT_MASK0) {
            hDrive->ATADRIVE_usPioMode = ATA_SET_PIO_MODE_3;
        }

        if (hParam->ATAPARAM_usAdvancedPio & ATA_BIT_MASK1) {
            hDrive->ATADRIVE_usPioMode = ATA_SET_PIO_MODE_4;
        }
    }

    if ((hDrive->ATADRIVE_iOkDma) && (hParam->ATAPARAM_usValid & ATA_WORD64_70_VALID)) {
        hDrive->ATADRIVE_usSingleDmaMode = (UINT16)((hParam->ATAPARAM_usDmaMode >> 8) & 0x03);
        if (hDrive->ATADRIVE_usSingleDmaMode >= ATA_SET_SDMA_MODE_2) {
            hDrive->ATADRIVE_usSingleDmaMode = ATA_SET_SDMA_MODE_0;
        }
        hDrive->ATADRIVE_usMultiDmaMode  = ATA_SET_MDMA_MODE_0;

        if (hParam->ATAPARAM_usSingleDma & ATA_BIT_MASK2) {
            hDrive->ATADRIVE_usSingleDmaMode = ATA_SET_SDMA_MODE_2;

        } else if (hParam->ATAPARAM_usSingleDma & ATA_BIT_MASK1) {
            hDrive->ATADRIVE_usSingleDmaMode = ATA_SET_SDMA_MODE_1;

        } else if (hParam->ATAPARAM_usSingleDma & ATA_BIT_MASK0) {
            hDrive->ATADRIVE_usSingleDmaMode = ATA_SET_SDMA_MODE_0;
        }

        if (hParam->ATAPARAM_usMultiDma & ATA_BIT_MASK2) {
            hDrive->ATADRIVE_usMultiDmaMode = ATA_SET_MDMA_MODE_2;

        } else if (hParam->ATAPARAM_usMultiDma & ATA_BIT_MASK1) {
            hDrive->ATADRIVE_usMultiDmaMode = ATA_SET_MDMA_MODE_1;

        } else if (hParam->ATAPARAM_usMultiDma & ATA_BIT_MASK0) {
            hDrive->ATADRIVE_usMultiDmaMode = ATA_SET_MDMA_MODE_0;
        }
    }

    if ((hDrive->ATADRIVE_iOkDma) && (hParam->ATAPARAM_usValid & ATA_WORD88_VALID)) {
        if (hParam->ATAPARAM_usUltraDma & ATA_BIT_MASK5) {
            hDrive->ATADRIVE_usUltraDmaMode = ATA_SET_UDMA_MODE_5;

        } else if (hParam->ATAPARAM_usUltraDma & ATA_BIT_MASK4) {
            hDrive->ATADRIVE_usUltraDmaMode = ATA_SET_UDMA_MODE_4;

        } else if (hParam->ATAPARAM_usUltraDma & ATA_BIT_MASK3) {
            hDrive->ATADRIVE_usUltraDmaMode = ATA_SET_UDMA_MODE_3;

        } else if (hParam->ATAPARAM_usUltraDma & ATA_BIT_MASK2) {
            hDrive->ATADRIVE_usUltraDmaMode = ATA_SET_UDMA_MODE_2;

        } else if (hParam->ATAPARAM_usUltraDma & ATA_BIT_MASK1) {
            hDrive->ATADRIVE_usUltraDmaMode = ATA_SET_UDMA_MODE_1;

        } else if (hParam->ATAPARAM_usUltraDma & ATA_BIT_MASK0) {
            hDrive->ATADRIVE_usUltraDmaMode = ATA_SET_UDMA_MODE_0;
        }
    }

    hDrive->ATADRIVE_usRwBits = (UINT16) (uiConfigType & ATA_BITS_MASK);
    hDrive->ATADRIVE_usRwPio  = (UINT16)(uiConfigType & ATA_PIO_MASK);
    hDrive->ATADRIVE_usRwMode = ATA_PIO_DEF_W;

    ATA_LOG(ATA_LOG_PRT,
            "ctrl %d driver %d ata drive int \r\n"
            "   hDrive->ATADRIVE_usPioMode          0x%x\r\n"
            "   hDrive->ATADRIVE_usSingleDmaMode    0x%x\r\n"
            "   hDrive->ATADRIVE_usMultiDmaMode     0x%x\r\n"
            "   hDrive->ATADRIVE_usUltraDmaMode     0x%x\r\n"
            "   hDrive->ATADRIVE_usRwBits           0x%x\r\n"
            "   hDrive->ATADRIVE_usRwPio            0x%x.\r\n",
            uiCtrl, uiDrive,
            hDrive->ATADRIVE_usPioMode, hDrive->ATADRIVE_usSingleDmaMode,
            hDrive->ATADRIVE_usMultiDmaMode, hDrive->ATADRIVE_usUltraDmaMode,
            hDrive->ATADRIVE_usRwBits, hDrive->ATADRIVE_usRwPio);

    ATA_LOG(ATA_LOG_PRT,
            "ctrl %d driver %d ata drive int \r\n"
            "Serial No         : %20s\r\n"
            "Model Number      : %40s\r\n"
            "Firmware revision : %8s \r\n",
            uiCtrl, uiDrive,
            hParam->ATAPARAM_ucSerial, hParam->ATAPARAM_ucModel, hParam->ATAPARAM_ucFwRev);

    switch (uiConfigType & ATA_MODE_MASK) {

    case ATA_PIO_AUTO:
        hDrive->ATADRIVE_usRwMode = (UINT16)(ATA_PIO_W_0 + hDrive->ATADRIVE_usPioMode);
        break;

    case ATA_DMA_AUTO:
        if (hDrive->ATADRIVE_usUltraDmaMode != 0xff) {
            hDrive->ATADRIVE_usRwMode = (UINT16)(ATA_DMA_ULTRA_0 + hDrive->ATADRIVE_usUltraDmaMode);

        } else if (hDrive->ATADRIVE_usMultiDmaMode != 0xff) {
            if (hDrive->ATADRIVE_usMultiDmaMode) {
                hDrive->ATADRIVE_usRwMode=(UINT16) (ATA_DMA_MULTI_0 +hDrive->ATADRIVE_usMultiDmaMode);

            } else {
                hDrive->ATADRIVE_usRwMode = (UINT16)(ATA_PIO_W_0 + hDrive->ATADRIVE_usPioMode);
            }
        } else if (hDrive->ATADRIVE_usSingleDmaMode != 0xff) {
            hDrive->ATADRIVE_usRwMode = (UINT16)(((hDrive->ATADRIVE_usPioMode<=ATA_SET_PIO_MODE_1) &&
                                                  (hDrive->ATADRIVE_usSingleDmaMode == 2)) ?
                                                 ATA_DMA_SINGLE_2 :
                                                 ATA_PIO_W_0 + hDrive->ATADRIVE_usPioMode);
        } else {
            hDrive->ATADRIVE_usRwMode = (UINT16)(ATA_PIO_W_0 + hDrive->ATADRIVE_usPioMode);
        }
        break;

    default:
        hDrive->ATADRIVE_usRwMode = (UINT16)(uiConfigType & ATA_MODE_MASK);
    }

    ataBestTransferModeFind(hCtrl, uiCtrl, uiDrive);

    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drive best rw mode 0x%x.\r\n",
            uiCtrl, uiDrive, hDrive->ATADRIVE_usRwMode);

    hDrive->ATADRIVE_iDmaUse = LW_FALSE;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drive rw mode before negotiation 0x%x.\r\n",
            uiCtrl, uiDrive, hDrive->ATADRIVE_usRwMode);
    if ((hCtrl->ATACTRL_hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlModeGet)) {
        pfuncDmaCtrlMode = hCtrl->ATACTRL_hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlModeGet;
        hDrive->ATADRIVE_usRwMode = (UINT16)((*pfuncDmaCtrlMode)(hDrive->ATADRIVE_usRwMode));
    }
    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drive rw mode after negotiation 0x%x.\r\n",
            uiCtrl, uiDrive, hDrive->ATADRIVE_usRwMode);

    hDrive->ATADRIVE_iDmaUse = (hDrive->ATADRIVE_iOkDma &&
                                (hDrive->ATADRIVE_usRwMode >= ATA_DMA_SINGLE_0)) ? LW_TRUE : LW_FALSE;

    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d using dma %d ok dma %d rw mode %d.\r\n",
            uiCtrl, uiDrive,
            hDrive->ATADRIVE_iDmaUse, hDrive->ATADRIVE_iOkDma, hDrive->ATADRIVE_usRwMode);

    if (!hDrive->ATADRIVE_iDmaUse) {
        hDrive->ATADRIVE_usRwMode =(UINT16)( ATA_PIO_W_0 + hDrive->ATADRIVE_usPioMode);
    }

    if ((hCtrl->ATACTRL_hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlModeSet)) {
        pfuncDmaCtrlMode = hCtrl->ATACTRL_hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlModeSet;
        (*pfuncDmaCtrlMode)(hCtrl->ATACTRL_hCtrlDrv, uiCtrl, uiDrive, hDrive->ATADRIVE_usRwMode);
    }

    ataBestTransferModeFind(hCtrl, uiCtrl, uiDrive);

    ataDriveCommandSend(hCtrl, uiCtrl, uiDrive,
                        ATA_CMD_SET_FEATURE, ATA_SUB_SET_RWMODE, hDrive->ATADRIVE_usRwMode, 0, 0, 0, 0);
    ataDriveCommandSend(hCtrl, uiCtrl, uiDrive, ATA_CMD_SET_FEATURE, ATA_SUB_DISABLE_REVE, 0, 0, 0, 0, 0);

    if ((hParam->ATAPARAM_usRemovcyl == ATA_SPEC_CONFIG_VALUE_0) ||
        (hParam->ATAPARAM_usRemovcyl == ATA_SPEC_CONFIG_VALUE_1)) {
        ataDriveCommandSend(hCtrl, uiCtrl, uiDrive,
                            ATA_CMD_SET_FEATURE, ATA_SUB_POW_UP_STDBY_SPIN, 0, 0, 0, 0, 0);
    }

    hDrive->ATADRIVE_iSupportSmart = LW_FALSE;
    if ((hParam->ATAPARAM_usFeaturesEnabled0) & 0x0001) {
        ataDriveCommandSend(hCtrl, uiCtrl, uiDrive, ATA_CMD_SMART, ATA_SMART_DISABLE_OPER, 0, 0, 0, 0, 0);
    }

    if (hParam->ATAPARAM_usFeaturesSupported1 & 0x0010) {
        if ((hParam->ATAPARAM_usFeaturesEnabled1 & 0x0010)) {
            ataDriveCommandSend(hCtrl, uiCtrl, uiDrive,
                                ATA_CMD_SET_FEATURE, ATA_SUB_DISABLE_NOTIFY, 0, 0, 0, 0, 0);
        }
    }

    if (hParam->ATAPARAM_usFeaturesSupported1 & 0x0008) {
        if ((hParam->ATAPARAM_usFeaturesEnabled1 & 0x0008)) {
            ataDriveCommandSend(hCtrl, uiCtrl, uiDrive,
                                ATA_CMD_SET_FEATURE, ATA_SUB_DIS_ADV_POW_MNGMT, 0, 0, 0, 0, 0);
        }
    }

    if ((hDrive->ATADRIVE_usRwPio == ATA_PIO_MULTI) &&
        (hDrive->ATADRIVE_iType == ATA_TYPE_ATA)) {
        if (hDrive->ATADRIVE_iOkMulti) {
            ataDriveCommandSend(hCtrl, uiCtrl, uiDrive,
                                ATA_CMD_SET_MULTI, hDrive->ATADRIVE_usMultiSecs, 0, 0, 0, 0, 0);

        } else {
            hDrive->ATADRIVE_usRwPio = ATA_PIO_SINGLE;
        }
    }

    hDrive->ATADRIVE_iState = ATA_DEV_OK;

__error_handle:
    API_SemaphoreMPost(hCtrl->ATACTRL_hSemDev);

    if (hDrive->ATADRIVE_iState != ATA_DEV_OK) {
        return  (PX_ERROR);
    }

    ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drive int ok state 0x%x.\r\n",
            uiCtrl, uiDrive, hDrive->ATADRIVE_iState);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: ataDrvInit
** 功能描述: 驱动初始化
** 输　入  : hCtrlDrv               控制器驱动控制句柄
**           uiCtrl                 控制器索引 (ATA_CTRL_MAX)
**           uiDrives               驱动器数量 (ATA_DRIVE_MAX)
**           uiConfigType           配置信息, 如 DMA 模式等
**           ulSemSyncTimeout       同步超时时间
** 输　出  : ERROR or OK
** 全局变量:
** 调用模块:
*********************************************************************************************************/
INT ataDrvInit (ATA_DRV_HANDLE  hCtrlDrv, UINT  uiCtrl, ATA_CTRL_CFG_HANDLE  hCtrlCfg)
{
    REGISTER INT                i;
    INT                         iRet = PX_ERROR;
    ATA_CTRL_HANDLE             hCtrl;
    ATA_DRIVE_HANDLE            hDrive;
    ATA_DRIVE_INFO_HANDLE       hDriveInfo;
    UINT                        uiDrives;
    UINT32                      uiConfigType;
    ULONG                       ulWdgTimeout;
    ULONG                       ulSyncTimeout;
    ULONG                       ulSyncTimeoutLoop;

    if ((!hCtrlDrv) || (uiCtrl >= hCtrlDrv->ATADRV_uiCtrlNum) || (!hCtrlCfg)) {
        ATA_LOG(ATA_LOG_ERR,
                "drv handle %p ctrl %d [0 - %d].\r\n", hCtrlDrv, uiCtrl, hCtrlDrv->ATADRV_uiCtrlNum);
        return  (PX_ERROR);
    }

    uiDrives = hCtrlCfg->ATACTRLCFG_ucDrives;
    if (uiDrives > ATA_DRIVE_MAX) {
        ATA_LOG(ATA_LOG_ERR, "drv handle %p drivers %d [0 - %d].\r\n", hCtrlDrv, uiDrives, ATA_DRIVE_MAX);
        return  (PX_ERROR);
    }

    uiConfigType      = hCtrlCfg->ATACTRLCFG_uiConfigType;
    ulWdgTimeout      = hCtrlCfg->ATACTRLCFG_ulWdgTimeout;
    ulSyncTimeout     = hCtrlCfg->ATACTRLCFG_ulSyncTimeout;
    ulSyncTimeoutLoop = hCtrlCfg->ATACTRLCFG_ulSyncTimeoutLoop;
    ATA_LOG(ATA_LOG_PRT,
            "ctrl %d drivers %d ata drv int ctrl config 0x%08x sync timeout %d wdg timeout %d.\r\n",
            uiCtrl, uiDrives, uiConfigType, ulSyncTimeout, ulWdgTimeout);

    hCtrl = &hCtrlDrv->ATADRV_tCtrl[uiCtrl];
    if (hCtrl->ATACTRL_iDrvInstalled) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d ata host controller already initialized.\r\n", uiCtrl);
        return  (PX_ERROR);
    }

    /*
     *  通过控制器配置更新参数
     */
    lib_memcpy((PVOID)&hCtrl->ATACTRL_tAtaReg, (PVOID)&hCtrlCfg->ATACTRLCFG_tAtaReg, sizeof(ATA_REG_CB));
    hCtrl->ATACTRL_ulCtlrIntVector  = hCtrlCfg->ATACTRLCFG_ulCtlrIntVector;
    hCtrl->ATACTRL_pfuncCtlrIntServ = hCtrlCfg->ATACTRLCFG_pfuncCtlrIntServ;
    hCtrl->ATACTRL_pfuncCtrlIntPre  = hCtrlCfg->ATACTRLCFG_pfuncCtrlIntPre;
    hCtrl->ATACTRL_pfuncCtrlIntPost = hCtrlCfg->ATACTRLCFG_pfuncCtrlIntPost;
    hCtrl->ATACTRL_uiCtrl           = uiCtrl;
    hCtrl->ATACTRL_uiConfigType     = uiConfigType;
    hCtrl->ATACTRL_uiDriveNum       = uiDrives;
    hCtrl->ATACTRL_hCtrlDrv         = hCtrlDrv;

    if (!hCtrlCfg->ATACTRLCFG_pfuncDelay) {
        hCtrl->ATACTRL_pfuncDelay = (FUNCPTR)ataCtrlDelay;

    } else {
        hCtrl->ATACTRL_pfuncDelay = hCtrlCfg->ATACTRLCFG_pfuncDelay;
    }

    ATA_LOG(ATA_LOG_PRT, "ctrl %d ata drv int cmd base 0x%llx ctrl base 0x%llx busm base 0x%llx.\r\n",
            uiCtrl, hCtrl->ATA_CTRL_CMD_BASE, hCtrl->ATA_CTRL_CTRL_BASE, hCtrl->ATA_CTRL_BUSM_BASE);

    iRet = ataCtrlReset(hCtrl, uiCtrl);
    if (iRet != ERROR_NONE) {
        ATA_LOG(ATA_LOG_PRT, "ctrl %d ata drv int controller reset failed.\r\n", uiCtrl);
        return(PX_ERROR);
    }
    ATA_LOG(ATA_LOG_PRT, "ctrl %d ata drv int controller reset ok.\r\n", uiCtrl);

    hCtrl->ATACTRL_hSemSync = API_SemaphoreBCreate("ata_sync",
                                                   LW_FALSE,
                                                   (LW_OPTION_WAIT_FIFO | LW_OPTION_OBJECT_GLOBAL),
                                                   LW_NULL);
    if (hCtrl->ATACTRL_hSemSync == LW_OBJECT_HANDLE_INVALID) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d ata drv int sync semaphore create failed.\r\n", uiCtrl);
        return  (PX_ERROR);
    }

    if (ulWdgTimeout) {
        hCtrl->ATACTRL_ulWdgTimeout = ulWdgTimeout;

    } else {
        hCtrl->ATACTRL_ulWdgTimeout = ATA_WDG_TIMEOUT_DEF;
    }
    if (ulSyncTimeout) {
        hCtrl->ATACTRL_ulSemSyncTimeout = ulSyncTimeout;

    } else {
        hCtrl->ATACTRL_ulSemSyncTimeout = ATA_SEM_TIMEOUT_DEF;
    }
    if (ulSyncTimeoutLoop) {
        hCtrl->ATACTRL_ulSyncTimeoutLoop = ulSyncTimeoutLoop;

    } else {
        hCtrl->ATACTRL_ulSyncTimeoutLoop = ATA_TIMEOUT_LOOP_NUM;
    }

    hCtrl->ATACTRL_hSemDev = API_SemaphoreMCreate("ata_dlock",
                                                  LW_PRIO_DEF_CEILING,
                                                  (LW_OPTION_WAIT_PRIORITY | LW_OPTION_DELETE_SAFE |
                                                   LW_OPTION_INHERIT_PRIORITY | LW_OPTION_OBJECT_GLOBAL),
                                                  LW_NULL);
    if (hCtrl->ATACTRL_hSemDev == LW_OBJECT_HANDLE_INVALID) {
        ATA_LOG(ATA_LOG_ERR, "ctrl %d ata drv int dev lock semaphore create failed.\r\n", uiCtrl);
        return  (PX_ERROR);
    }

    ATA_SPIN_ISR_INIT(hCtrl);

    /*
     *  DMA 复位及初始化
     */
    if ((hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlReset)) {
        (*hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlReset)(hCtrlDrv, uiCtrl);
    }

    if ((hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlInit)) {
        iRet = (*hCtrlDrv->ATADRV_tDma.ATADRVDMA_pfuncDmaCtrlInit)(hCtrlDrv, uiCtrl);
        if (iRet != ERROR_NONE) {
            hCtrlDrv->ATADRV_tDma.ATADRVDMA_iDmaCtrlSupport = LW_FALSE;
        }
    }

    /*
     *  中断链接与使能
     */
    if (!hCtrlDrv->ATADRV_iIntDisable) {
        if ((hCtrlDrv->ATADRV_tInt.ATADRVINT_pfuncCtrlIntConnect)) {
            iRet = (*hCtrlDrv->ATADRV_tInt.ATADRVINT_pfuncCtrlIntConnect)(hCtrlDrv, uiCtrl, "ata_isr");
            if (iRet != ERROR_NONE) {
                ATA_LOG(ATA_LOG_ERR, "ctrl %d ata drv interrupt connect failed.\r\n", uiCtrl);
                return  (PX_ERROR);
            }
        }

        if ((hCtrlDrv->ATADRV_tInt.ATADRVINT_pfuncCtrlIntEnable)) {
            iRet = (*hCtrlDrv->ATADRV_tInt.ATADRVINT_pfuncCtrlIntEnable)(hCtrlDrv, uiCtrl);
            if (iRet != ERROR_NONE) {
                ATA_LOG(ATA_LOG_ERR, "ctrl %d ata drv interrupt enable failed.\r\n", uiCtrl);
                return  (PX_ERROR);
            }
        }
    }

    hCtrl->ATACTRL_iDrvInstalled = LW_TRUE;
    for (i = 0; i < hCtrl->ATACTRL_uiDriveNum; i++) {
        hDrive                 = &hCtrl->ATACTRL_tDrive[i];
        hDrive->ATADRIVE_hCtrl = hCtrl;

        hDriveInfo = &hCtrlDrv->ATADRV_tDriveInfo[uiCtrl * hCtrl->ATACTRL_uiDriveNum + i];

        hDrive->ATADRIVE_hInfo         = hDriveInfo;
        hDrive->ATADRIVE_iState        = ATA_DEV_INIT;
        hDrive->ATADRIVE_iType         = ATA_TYPE_INIT;
        hDrive->ATADRIVE_iDiagCode     = 0;
        hDrive->ATADRIVE_pfuncDevReset = (FUNCPTR)ataDevInit;

        ATA_LOG(ATA_LOG_PRT,
                "ctrl %d driver %d ctrl num %d ata drv int drv info %p driver info %p bytes %d.\r\n",
                uiCtrl, i, hCtrlDrv->ATADRV_uiCtrlNum,
                hDriveInfo, hDrive->ATADRIVE_hInfo, hDriveInfo->ATADINFO_uiBytes);

        ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d signature 0x%08x.\r\n",
                uiCtrl, i, hDrive->ATADRIVE_uiSignature);

        iRet = ataDriveInit(hCtrl, uiCtrl, i);
        if (iRet != ERROR_NONE) {
            ATA_LOG(ATA_LOG_PRT, "ctrl %d driver %d ata drv int drive failed.\r\n", uiCtrl, i);
        }
    }

    return  (ERROR_NONE);
}

#endif                                                                  /* (LW_CFG_DEVICE_EN > 0) &&    */
                                                                        /* (LW_CFG_ATA_EN > 0)          */
/*********************************************************************************************************
  END
*********************************************************************************************************/
