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
** 文   件   名: ms_boot_fatfs.c
**
** 创   建   人: Jiao.jinxing
**
** 文件创建日期: 2020 年 04 月 07 日
**
** 描        述: FAT 文件系统
*********************************************************************************************************/
#include "main.h"
#include <string.h>
#include "ms_boot_fatfs.h"

#if MS_CFG_BOOT_EXT_FS_TYPE == MS_BOOT_FATFS

#define __MNT_PREFIX_LEN    (sizeof("/sd0") - 1)

/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc ( /* Returns pointer to the allocated memory block */
    UINT msize      /* Number of bytes to allocate */
)
{
    return ms_kmalloc(msize);    /* Allocate a new memory block with POSIX API */
}

/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
    void* mblock    /* Pointer to the memory block to free */
)
{
    ms_kfree(mblock);    /* Discard the memory block with POSIX API */
}

int __ms_fatfs_result_to_errno(FRESULT fresult)
{
    int err;

    switch (fresult) {
    case FR_OK:
        err = 0;
        break;

    case FR_DISK_ERR:
        err = EIO;
        break;

    case FR_INT_ERR:
        err = EFAULT;
        break;

    case FR_NOT_READY:
        err = EIO;
        break;

    case FR_NO_FILE:
        err = ENOENT;
        break;

    case FR_NO_PATH:
        err = ENOTDIR;
        break;

    case FR_INVALID_NAME:
        err = EINVAL;
        break;

    case FR_DENIED:
        err = EACCES;
        break;

    case FR_EXIST:
        err = EEXIST;
        break;

    case FR_INVALID_OBJECT:
        err = EBADF;
        break;

    case FR_WRITE_PROTECTED:
        err = EACCES;
        break;

    case FR_INVALID_DRIVE:
        err = ENODEV;
        break;

    case FR_NOT_ENABLED:
        err = ENODEV;
        break;

    case FR_NO_FILESYSTEM:
        err = EINVAL;
        break;

    case FR_MKFS_ABORTED:
        err = EIO;
        break;

    case FR_TIMEOUT:
        err = ETIMEDOUT;
        break;

    case FR_LOCKED:
        err = EBUSY;
        break;

    case FR_NOT_ENOUGH_CORE:
        err = ENOMEM;
        break;

    case FR_TOO_MANY_OPEN_FILES:
        err = ENFILE;
        break;

    case FR_INVALID_PARAMETER:
        err = EINVAL;
        break;

    default:
        err = EFAULT;
        break;
    }

    return err;
}

static ms_err_t __ms_fatfs_open(const char *source, ms_ptr_t *handle)
{
    FIL *fatfs_file;
    FRESULT fresult;
    ms_err_t err;

    fatfs_file = ms_kzalloc(sizeof(FIL));
    if (fatfs_file != MS_NULL) {
        fresult = f_open(fatfs_file, source + __MNT_PREFIX_LEN, FA_READ);
        if (fresult != FR_OK) {
            ms_kfree(fatfs_file);
            err = -__ms_fatfs_result_to_errno(fresult);
        } else {
            *handle = fatfs_file;
            err = MS_ERR_NONE;
        }
    } else {
        err = MS_ERR_KERN_HEAP_NO_MEM;
    }

    return err;
}

static ms_err_t __ms_fatfs_close(ms_ptr_t handle)
{
    FRESULT fresult;
    ms_err_t err;

    fresult = f_close(handle);
    if (fresult == FR_OK) {
        ms_kfree(handle);
        err = MS_ERR_NONE;
    } else {
        err = -__ms_fatfs_result_to_errno(fresult);
    }

    return err;
}

static ms_err_t __ms_fatfs_read(ms_ptr_t handle, ms_ptr_t buf, ms_uint32_t len, ms_uint32_t *read_len)
{
    FRESULT fresult;
    ms_err_t err;

    fresult = f_read(handle, buf, len, (UINT *)read_len);
    if (fresult == FR_OK) {
        err = MS_ERR_NONE;
    } else {
        err = -__ms_fatfs_result_to_errno(fresult);
    }

    return err;
}

static ms_err_t __ms_fatfs_seek(ms_ptr_t handle, ms_uint32_t offset)
{
    FRESULT fresult;
    ms_err_t err;

    fresult = f_lseek(handle, offset);
    if (fresult == FR_OK) {
        err = MS_ERR_NONE;
    } else {
        err = -__ms_fatfs_result_to_errno(fresult);
    }

    return err;
}

static ms_err_t __ms_fatfs_stat(const char *source, ms_boot_extfs_stat_t *stat)
{
    FRESULT fresult;
    FILINFO finfo;
    ms_err_t err;

    fresult = f_stat(source + __MNT_PREFIX_LEN, &finfo);
    if (fresult == FR_OK) {
        stat->size = finfo.fsize;
        err = MS_ERR_NONE;
    } else {
        err = -__ms_fatfs_result_to_errno(fresult);
    }

    return err;
}

ms_boot_extfs_if_t ms_fatfs_if = {
        .open  = __ms_fatfs_open,
        .close = __ms_fatfs_close,
        .read  = __ms_fatfs_read,
        .seek  = __ms_fatfs_seek,
        .stat  = __ms_fatfs_stat,
};

#if MS_CFG_BOOT_SHELL_EN > 0

/**
 * @brief ls command.
 *
 * @param[in] argc              Arguments count
 * @param[in] argv              Arguments array
 * @param[in] io                Pointer to shell io driver
 *
 * @return N/A
 */
static void __ms_shell_fatfs_ls(int argc, char *argv[], const ms_shell_io_t *io)
{
    FATFS_DIR dir;
    FRESULT fresult;
    const char *path;

    if (argc > 1) {
        path = argv[1U];
    } else {
        path = "/";
    }

    fresult = f_opendir(&dir, path);
    if (fresult == F_OK) {
        FILINFO finfo;

        while (MS_TRUE) {
            fresult = f_readdir(&dir, &finfo);
            if (fresult == FR_OK) {
                if (finfo.fname[0] != '\0') {
                    io->_printf("%s\n", finfo.fname);
                } else {
                    break;
                }
            } else {
                break;
            }
        }
        f_closedir(&dir);
    }
}

MS_SHELL_CMD(ls, __ms_shell_fatfs_ls, "List fatfs files and directories, ls path", __ms_shell_cmd_fatfsls);

#endif

#endif
/*********************************************************************************************************
  END
*********************************************************************************************************/
