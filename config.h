/*********************************************************************************************************
**
**                                北京翼辉信息技术有限公司
**
**                                  微型安全实时操作系统
**
**                                      MS-RTOS(TM)
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: config.h
**
** 创   建   人: IoT Studio
**
** 文件创建日期: 2020 年 02 月 14 日
**
** 描        述: 本文件由 IoT Studio 生成，用于配置 BSP 相关地址信息
*********************************************************************************************************/

#ifndef __BSP_CONFIG_H
#define __BSP_CONFIG_H

#define MS_BOOT_LITTLEFS            0U
#define MS_BOOT_FATFS               1U

#define MS_BOOT_FLASH               0U
#define MS_BOOT_SDRAM               1U
#define MS_BOOT_QSPI_FLASH          2U

#define MS_CFG_BOOT_OS_RUN_IN       MS_BOOT_QSPI_FLASH

#define MS_CFG_BOOT_MINI_LIBC_EN    1U

#define MS_CFG_BOOT_EXT_FS_TYPE     MS_BOOT_FATFS
#define MS_CFG_BOOT_HEAP_SIZE       (64U * 1024U)
#define MS_CFG_BOOT_SHELL_EN        0U
#define MS_CFG_BOOT_TRACE_EN        0U

#if MS_CFG_BOOT_OS_RUN_IN   == MS_BOOT_SDRAM
#define MS_CFG_BOOT_OS_BASE         0xc0000000U
#elif MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_FLASH
#define MS_CFG_BOOT_OS_BASE         0x08020000U
#elif MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_QSPI_FLASH
#define MS_CFG_BOOT_OS_BASE         0x90000000U
#endif
#define MS_CFG_BOOT_OS_SIZE         (512U * 1024U)

/*
 * MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_FLASH ONLY
 */
#define MS_CFG_BOOT_OS_SECTOR_BASE  FLASH_SECTOR_4
#define MS_CFG_BOOT_OS_SECTOR_COUNT 2U
#define MS_CFG_BOOT_READ_BUF_SIZE   (10U * 1024U)
#define MS_CFG_BOOT_UPDATE_REQ_FILE "/sd0/update/update_req"

/*
 * MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_SDRAM ONLY
 */
#define MS_CFG_BOOT_OS_IMAGE_FILE   "/sd0/update/firmware/MS-RTOS_IOT_PI_PRO.bin"

#endif                                                                  /*  __BSP_CONFIG_H              */
/*********************************************************************************************************
  END
*********************************************************************************************************/
