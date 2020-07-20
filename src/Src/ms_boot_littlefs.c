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
** 文   件   名: ms_boot_littlefs.c
**
** 创   建   人: Jiao.jinxing
**
** 文件创建日期: 2020 年 04 月 07 日
**
** 描        述: littlefs 文件系统
*********************************************************************************************************/
#include "main.h"
#include <string.h>
#include "littlefs/lfs.h"
#include "ms_boot_littlefs.h"

#if MS_CFG_BOOT_EXT_FS_TYPE == MS_BOOT_LITTLEFS

#define __MNT_PREFIX_LEN    (sizeof("/nor") - 1)

MS_WEAK lfs_t *ms_lfs_partition = MS_NULL;

int __ms_littlefs_err_to_errno(int err)
{
    switch (err) {
    case LFS_ERR_OK:
        err = 0;
        break;

    case LFS_ERR_IO:
        err = EIO;
        break;

    case LFS_ERR_CORRUPT:
        err = EFAULT;
        break;

    case LFS_ERR_NOENT:
        err = ENOENT;
        break;

    case LFS_ERR_EXIST:
        err = EEXIST;
        break;

    case LFS_ERR_NOTDIR:
        err = ENOTDIR;
        break;

    case LFS_ERR_ISDIR:
        err = EISDIR;
        break;

    case LFS_ERR_NOTEMPTY:
        err = ENOTEMPTY;
        break;

    case LFS_ERR_BADF:
        err = EBADF;
        break;

    case LFS_ERR_FBIG:
        err = EFBIG;
        break;

    case LFS_ERR_INVAL:
        err = EINVAL;
        break;

    case LFS_ERR_NOSPC:
        err = ENOSPC;
        break;

    case LFS_ERR_NOMEM:
        err = ENOMEM;
        break;

    case LFS_ERR_NOATTR:
        err = EINVAL;
        break;

    case LFS_ERR_NAMETOOLONG:
        err = ENAMETOOLONG;
        break;

    default:
        err = EFAULT;
        break;
    }

    return err;
}

static ms_err_t __ms_littlefs_open(const char *source, ms_ptr_t *handle)
{
    lfs_file_t *lfs_file;
    int ret;
    ms_err_t err;

    lfs_file = ms_kzalloc(sizeof(lfs_file_t));
    if (lfs_file != MS_NULL) {
        ret = lfs_file_open(ms_lfs_partition, lfs_file, source + __MNT_PREFIX_LEN, LFS_O_RDONLY);
        if (ret != LFS_ERR_OK) {
            ms_kfree(lfs_file);
            err = -__ms_littlefs_err_to_errno(ret);
        } else {
            *handle = lfs_file;
            err = MS_ERR_NONE;
        }
    } else {
        err = MS_ERR_KERN_HEAP_NO_MEM;
    }

    return err;
}

static ms_err_t __ms_littlefs_close(ms_ptr_t handle)
{
    int ret;
    ms_err_t err;

    ret = lfs_file_close(ms_lfs_partition, handle);
    if (ret == LFS_ERR_OK) {
        ms_kfree(handle);
        err = MS_ERR_NONE;
    } else {
        err = -__ms_littlefs_err_to_errno(ret);
    }

    return err;
}

static ms_err_t __ms_littlefs_read(ms_ptr_t handle, ms_ptr_t buf, ms_uint32_t len, ms_uint32_t *read_len)
{
    ms_ssize_t ret;
    ms_err_t err;

    ret = lfs_file_read(ms_lfs_partition, handle, buf, len);
    if (ret >= 0) {
        *read_len = ret;
        err = MS_ERR_NONE;
    } else {
        err = -__ms_littlefs_err_to_errno(ret);
    }

    return err;
}

static ms_err_t __ms_littlefs_seek(ms_ptr_t handle, ms_uint32_t offset)
{
    lfs_soff_t ret;
    ms_err_t err;

    ret = lfs_file_seek(ms_lfs_partition, handle, offset, LFS_SEEK_SET);
    if (ret >= 0) {
        err = MS_ERR_NONE;
    } else {
        err = -__ms_littlefs_err_to_errno(ret);
    }

    return err;
}

static ms_err_t __ms_littlefs_stat(const char *source, ms_boot_extfs_stat_t *stat)
{
    struct lfs_info linfo;
    int ret;
    ms_err_t err;

    ret = lfs_stat(ms_lfs_partition, source + __MNT_PREFIX_LEN, &linfo);
    if (ret == LFS_ERR_OK) {
        stat->size = linfo.size;
        err = MS_ERR_NONE;
    } else {
        err = -__ms_littlefs_err_to_errno(ret);
    }

    return err;
}

ms_boot_extfs_if_t ms_littlefs_if = {
        .open  = __ms_littlefs_open,
        .close = __ms_littlefs_close,
        .read  = __ms_littlefs_read,
        .seek  = __ms_littlefs_seek,
        .stat  = __ms_littlefs_stat,
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
static void __ms_shell_littlefs_ls(int argc, char *argv[], const ms_shell_io_t *io)
{
    lfs_dir_t dir;
    const char *path;
    int ret;

    if (argc > 1) {
        path = argv[1U];
    } else {
        path = "/";
    }

    if (ms_lfs_partition == MS_NULL) {
        ret = LFS_ERR_NOENT;
    } else {
        ret = lfs_dir_open(ms_lfs_partition, &dir, path);
    }
    if (ret == LFS_ERR_OK) {
        struct lfs_info linfo;

        while (MS_TRUE) {
            ret = lfs_dir_read(ms_lfs_partition, &dir, &linfo);
            if (ret > 0) {
                io->_printf("%s\n", linfo.name);
            } else {
                break;
            }
        }

        lfs_dir_close(ms_lfs_partition, &dir);
    }
}

MS_SHELL_CMD(ls, __ms_shell_littlefs_ls, "List littlefs files and directories, ls [path]", __ms_shell_cmd_littlefsls);

#endif

#endif
/*********************************************************************************************************
  END
*********************************************************************************************************/
