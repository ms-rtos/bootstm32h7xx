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
** 文   件   名: ms_littlefs_drv.c
**
** 创   建   人: Jiao.jinxing
**
** 文件创建日期: 2020 年 04 月 07 日
**
** 描        述: littlefs 驱动接口
*********************************************************************************************************/
#include "main.h"
#include <string.h>
#include "ms_littlefs_drv.h"
#include "ms_boot_littlefs.h"

#if MS_CFG_BOOT_EXT_FS_TYPE == MS_BOOT_LITTLEFS

#include "stm32h743i_eval_qspi.h"

/**
 * @brief stm32f7 spi nor flash driver.
 */

lfs_t *ms_lfs_partition = MS_NULL;

/*
 * Read a region in a block. Negative error codes are propogated to the user.
 */
static int __lfs_block_read(const struct lfs_config *c, lfs_block_t block,
                            lfs_off_t off, void *buffer, lfs_size_t size)
{
    int ret;

    if (BSP_QSPI_Read((uint8_t *)buffer, (block * c->block_size + off), size) == QSPI_OK) {
        ret = LFS_ERR_OK;
    } else {
        ret = LFS_ERR_CORRUPT;
    }

    return ret;
}

/*
 * Program a region in a block. The block must have previously
 * been erased. Negative error codes are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 */
static int __lfs_block_prog(const struct lfs_config *c, lfs_block_t block,
                            lfs_off_t off, const void *buffer, lfs_size_t size)
{
    int ret;

    if (BSP_QSPI_Write((uint8_t *)buffer, (block * c->block_size + off), size) == QSPI_OK) {
        ret = LFS_ERR_OK;
    } else {
        ret = LFS_ERR_CORRUPT;
    }

    return ret;
}

/*
 * Erase a block. A block must be erased before being programmed.
 * The state of an erased block is undefined. Negative error codes
 * are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 *
 */
static int __lfs_block_erase(const struct lfs_config *c, lfs_block_t block)
{
    int ret;

    if (BSP_QSPI_Erase_Block(block * c->block_size) == QSPI_OK) {
        ret = LFS_ERR_OK;
    } else {
        ret = LFS_ERR_CORRUPT;
    }

    return ret;
}

/*
 * Sync the state of the underlying block device. Negative error codes
 * are propogated to the user.
 */
static int __lfs_block_sync(const struct lfs_config *c)
{
    return 0;
}

/*
 * configuration of the filesystem is provided by this struct
 */
static struct lfs_config __lfs_config = {
    /*
     * block device operations
     */
    .read  = __lfs_block_read,
    .prog  = __lfs_block_prog,
    .erase = __lfs_block_erase,
    .sync  = __lfs_block_sync,
};

/*
 * Create spi nor flash device file and mount
 */
ms_err_t ms_littlefs_init(lfs_t *lfs)
{
    ms_err_t err;

    if (BSP_QSPI_Init() == QSPI_OK) {
        QSPI_Info info;

        if (BSP_QSPI_GetInfo(&info) == QSPI_OK) {
            int ret;

            __lfs_config.read_size      = 1U;
            __lfs_config.prog_size      = info.ProgPageSize;
            __lfs_config.block_size     = info.EraseSectorSize;
            __lfs_config.block_count    = info.EraseSectorsNumber;
            __lfs_config.cache_size     = info.ProgPageSize;
            __lfs_config.block_cycles   = 500U;
            __lfs_config.lookahead_size = 8U * ((__lfs_config.block_count + 63U) / 64U);

            ret = lfs_mount(lfs, &__lfs_config);
            if (ret == LFS_ERR_OK) {
                ms_lfs_partition = lfs;
                err = MS_ERR_NONE;
            } else {
                err = -__ms_littlefs_err_to_errno(ret);
            }
        } else {
            err = MS_ERR;
        }
    } else {
        err = MS_ERR;
    }

    return err;
}

#endif
/*********************************************************************************************************
  END
*********************************************************************************************************/
