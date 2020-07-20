/*
*********************************************************************************************************
*
*  ģ������ : W25Q256 QSPI����ģ��
*  �ļ����� : bsp_qspi_w25q256.h
*
*  Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#ifndef BSP_QSPI_W25QXX_H
#define BSP_QSPI_W25QXX_H

#include "ms_rtos.h"
#include "stm32h7xx_hal.h"

/* ���������ֲ����� */
#define FLASH_ADDR_MODE                        (QSPI_ADDRESS_24_BITS)
#define FLASH_JEDEC_ID                         (0xEF4017)

/* W25QXX ������Ϣ */
#undef  QSPI_FLASH_SIZE
#define QSPI_FLASH_SIZE                        23                        /* Flash��С��2^23 = 8MB */
#define QSPI_SECTOR_SIZE                       (4 * 1024)                /* ������С��4KB */
#define QSPI_PAGE_SIZE                         256                       /* ҳ��С��256�ֽ� */
#define QSPI_END_ADDR                          (1 << QSPI_FLASH_SIZE)    /* ĩβ��ַ */
#define QSPI_FLASH_SIZES                       8*1024*1024               /* Flash��С��2^23 = 8MB */

/* W25QXX ������� */
#define CMD_Write_Enable                       0x06           /* дʹ��ָ�� *///
#define CMD_Volatile_SR_Write_Enable           0x50           /* дʹ��ָ�volatile�� *///
#define CMD_Write_Disable                      0x04           /* дʧ��ָ�� *///
#define CMD_Release_Power_down__ID             0xAB           /* �ϵ��ͷ�/�豸��ʶָ�� *///
#define CMD_Manufacturer__Device_ID            0x90           /* ��ȡ������/�豸IDָ�� *///
#define CMD_JEDEC_ID                           0x9F           /* ��JEDEC IDָ�� *///
#define CMD_Read_Unique_ID                     0x4B           /* ��ΨһIDָ�� *///
#define CMD_Read_Data                          0x03           /* ������ָ�� *///
#define CMD_Fast_Read                          0x0B           /* ���ٶ�����ָ�� *///

#define CMD_Page_Program                       0x02           /* ҳ���ָ�� *///
#define CMD_Sector_Erase_4KB                   0x20           /* 4KB��������ָ�� *///
#define CMD_Block_Erase_32KB                   0x52           /* 32KB�����ָ�� *///
#define CMD_Block_Erase_64KB                   0xD8           /* 64KB�����ָ�� *///
#define CMD_Chip_Erase                         0xC7           /* ȫƬ����ָ�� *///
#define CMD_Chip_Erase_1                       0x60           /* ȫƬ����ָ��1 *///

#define CMD_Read_Status_Register_1             0x05           /* ��״̬�Ĵ���1ָ�� *///
#define CMD_Write_Status_Register_1            0x01           /* д״̬�Ĵ���1ָ�� *///
#define CMD_Read_Status_Register_2             0x35           /* ��״̬�Ĵ���2ָ�� *///
#define CMD_Write_Status_Register_2            0x31           /* д״̬�Ĵ���2ָ�� *///
#define CMD_Read_Status_Register_3             0x15           /* ��״̬�Ĵ���3ָ�� *///
#define CMD_Write_Status_Register_3            0x11           /* д״̬�Ĵ���3ָ�� *///

#define CMD_Read_SFDP_Register                 0x5A           /* ��SFDP�Ĵ���ָ�� *///
#define CMD_Erase_Security_Register            0x44           /* ������ȫ�Ĵ���ָ�� *///
#define CMD_Program_Security_Register          0x42           /* ��̰�ȫ�Ĵ���ָ�� *///
#define CMD_Read_Security_Register             0x48           /* ����ȫ�Ĵ���ָ�� *///
#define CMD_Global_Block_Lock                  0x7E           /* ȫ�ֿ�����ָ�� *///û����
#define CMD_Global_Block_Unlock                0x98           /* ȫ�ֿ����ָ�� *///û����
#define CMD_Read_Block_Lock                    0x3D           /* ��ȫ�ֿ�����״ָ̬�� *///û����
#define CMD_Individual_Block_Lock              0x36           /* ����������ָ�� *///û����
#define CMD_Individual_Block_Unlock            0x39           /* ���������ָ�� *///û����

#define CMD_Erase__Program_Suspend             0x75           /* ����/��̹���ָ�� *///
#define CMD_Erase__Program_Resume              0x7A           /* ����/��ָ̻�ָ�� *///
#define CMD_Power_down                         0xB9           /* ����ָ�� *///
#define CMD_Enable_Reset                       0x66           /* ʹ�ܸ�λָ�� *///
#define CMD_Reset_Device                       0x99           /* ��λ�豸ָ�� *///

#define CMD_Fast_Read_Dual_Output              0x3B           /* ���ٶ�ȡ˫���ָ�� *///
#define CMD_Mftr__Device_ID_Dual_IO            0x92           /* ��ȡ������/�豸ID˫���ָ�� *///
#define CMD_Fast_Read_Dual_IO                  0xBB           /* ���ٶ�ȡ˫�������ָ�� *///
#define CMD_Quad_Input_Page_Program            0x32           /* ҳ���������ָ�� *///
#define CMD_Fast_Read_Quad_Output              0x6B           /* ���ٶ�ȡ�����ָ�� *///
#define CMD_Mftr__Device_ID_Quad_IO            0x94           /* ��ȡ������/�豸ID���������ָ�� *///
#define CMD_Fast_Read_Quad_IO                  0xEB           /* ���ٶ�ȡ���������ָ�� *///
#define CMD_Set_Burst_with_Wrap                0x77           /* ���û���ָ�� *//////////////////����

#define CMD_ENTER_4_BYTE_ADDR_MODE             0xB7           /* ����4��ַģʽָ�� */
#define CMD_EXIT_4_BYTE_ADDR_MODE              0xE9           /* �˳�4��ַģʽָ�� */

#define W25X_EnterQPIMode                      0x38           /* SPI */
#define W25X_ExitQPIMode                       0xFF           /* QPI */
#define W25X_SetReadParameters                 0xC0           /* QPI */
#define QE_MASK                                0x02
#define BUSY_MASK                              0x01

typedef enum
{
  W25QXX_SPI_MODE = 0,                 /*!< 1-1-1 commands, Power on H/W default setting */
  W25QXX_SPI_2IO_MODE,                 /*!< 1-1-2, 1-2-2 read commands                   */
  W25QXX_SPI_4IO_MODE,                 /*!< 1-1-4, 1-4-4 read commands                   */
  W25QXX_QPI_MODE                      /*!< 4-4-4 commands                               */
} W25QXX_Interface_t;

void     bsp_InitQSPI_W25Q32(void);
void     QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi);
void     QSPI_WriteDisable(QSPI_HandleTypeDef *hqspi);
uint8_t  QSPI_Read_Status_Register(const uint8_t _uiReadAddr);
void     QSPI_Write_Status_Register(uint8_t _uiReadAddr, const uint8_t *_pBuf);
void     QSPI_Read_Data(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
void     QSPI_Fast_Read(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
void     QSPI_Fast_Read_Quad_IO(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);
uint32_t QSPI_Read_JEDEC_ID(void);
void     QSPI_Enable_Reset(QSPI_HandleTypeDef *hqspi);
void     QSPI_Reset_Device(QSPI_HandleTypeDef *hqspi);
uint8_t  QSPI_EnableMemoryMappedMode(QSPI_HandleTypeDef *hqspi);
void     W25QXX_ExitQPIMode(void);
void     W25QXX_EnterQPIMode(void);
void     W25QXX_Enter4BytesAddressMode(QSPI_HandleTypeDef *hqspi, W25QXX_Interface_t Mode);
void     W25QXX_Exit4BytesAddressMode(QSPI_HandleTypeDef *Ctx, W25QXX_Interface_t Mode);

extern   QSPI_HandleTypeDef   QSPIHandle;

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
