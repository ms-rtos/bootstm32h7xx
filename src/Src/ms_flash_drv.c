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
** 文   件   名: ms_flash_drv.c
**
** 创   建   人: Jiao.jinxing
**
** 文件创建日期: 2020 年 04 月 07 日
**
** 描        述: FLASH 驱动接口
*********************************************************************************************************/
#include "main.h"
#include <string.h>
#include "ms_flash_drv.h"

ms_err_t ms_flash_unlock(void)
{
    return (HAL_FLASH_Unlock() == HAL_OK) ? MS_ERR_NONE : MS_ERR;
}

ms_err_t ms_flash_lock(void)
{
    return (HAL_FLASH_Lock() == HAL_OK) ? MS_ERR_NONE : MS_ERR;
}

ms_err_t ms_flash_erase_os(void)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t sector_err;
    ms_err_t err;

    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = MS_CFG_BOOT_OS_SECTOR_BASE;
    EraseInitStruct.NbSectors     = MS_CFG_BOOT_OS_SECTOR_COUNT;

    err = (HAL_FLASHEx_Erase(&EraseInitStruct, &sector_err) == HAL_OK) ? MS_ERR_NONE : MS_ERR;

    return err;
}

ms_err_t ms_flash_program(ms_addr_t addr, ms_const_ptr_t buf, ms_uint32_t len)
{
    ms_ptr_t addr_bak = (ms_ptr_t)addr;
    const ms_uint32_t *wbuf = buf;
    ms_size_t wlen = len >> 2U;
    ms_err_t err = MS_ERR;
    
    len = MS_ROUND_UP(len, 4);

    while (wlen > 0) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr, *wbuf++) != HAL_OK) {
            break;
        }

        wlen--;
        addr += 4U;
    }

    if (wlen == 0) {
        if (memcmp(addr_bak, buf, len) == 0) {
            err = MS_ERR_NONE;
        }
    }

    return err;
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
