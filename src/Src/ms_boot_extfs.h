/*********************************************************************************************************
**
**                                ���������Ϣ�������޹�˾
**
**                                  ΢�Ͱ�ȫʵʱ����ϵͳ
**
**                                      MS-RTOS(TM)
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: ms_boot_extfs.h
**
** ��   ��   ��: Jiao.jinxing
**
** �ļ���������: 2020 �� 04 �� 07 ��
**
** ��        ��: �ⲿ�ļ�ϵͳ
*********************************************************************************************************/

#ifndef MS_BOOT_EXTFS_H
#define MS_BOOT_EXTFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ms_boot_extfs_stat_t
 */
typedef struct {
    ms_uint32_t  size;
} ms_boot_extfs_stat_t;

/*
 * ms_flashfs_extfs_if_t
 */
typedef struct {
    ms_err_t (*open)(const char *source, ms_ptr_t *handle);
    ms_err_t (*close)(ms_ptr_t handle);
    ms_err_t (*read)(ms_ptr_t handle, ms_ptr_t buf, ms_uint32_t len, ms_uint32_t *read_len);
    ms_err_t (*seek)(ms_ptr_t handle, ms_uint32_t offset);
    ms_err_t (*stat)(const char *source, ms_boot_extfs_stat_t *stat);
} const ms_boot_extfs_if_t;

#ifdef __cplusplus
}
#endif

#endif                                                                  /*  MS_BOOT_EXTFS_H             */
/*********************************************************************************************************
  END
*********************************************************************************************************/
