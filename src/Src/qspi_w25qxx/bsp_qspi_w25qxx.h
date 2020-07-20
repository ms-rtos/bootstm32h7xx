/*
*********************************************************************************************************
*
*  模块名称 : W25Q256 QSPI驱动模块
*  文件名称 : bsp_qspi_w25q256.h
*
*  Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#ifndef BSP_QSPI_W25QXX_H
#define BSP_QSPI_W25QXX_H

#include "ms_rtos.h"
#include "stm32h7xx_hal.h"

/* 根据数据手册配置 */
#define FLASH_ADDR_MODE                        (QSPI_ADDRESS_24_BITS)
#define FLASH_JEDEC_ID                         (0xEF4017)

/* W25QXX 基本信息 */
#undef  QSPI_FLASH_SIZE
#define QSPI_FLASH_SIZE                        23                        /* Flash大小，2^23 = 8MB */
#define QSPI_SECTOR_SIZE                       (4 * 1024)                /* 扇区大小，4KB */
#define QSPI_PAGE_SIZE                         256                       /* 页大小，256字节 */
#define QSPI_END_ADDR                          (1 << QSPI_FLASH_SIZE)    /* 末尾地址 */
#define QSPI_FLASH_SIZES                       8*1024*1024               /* Flash大小，2^23 = 8MB */

/* W25QXX 相关命令 */
#define CMD_Write_Enable                       0x06           /* 写使能指令 *///
#define CMD_Volatile_SR_Write_Enable           0x50           /* 写使能指令（volatile） *///
#define CMD_Write_Disable                      0x04           /* 写失能指令 *///
#define CMD_Release_Power_down__ID             0xAB           /* 断电释放/设备标识指令 *///
#define CMD_Manufacturer__Device_ID            0x90           /* 读取制造商/设备ID指令 *///
#define CMD_JEDEC_ID                           0x9F           /* 读JEDEC ID指令 *///
#define CMD_Read_Unique_ID                     0x4B           /* 读唯一ID指令 *///
#define CMD_Read_Data                          0x03           /* 读数据指令 *///
#define CMD_Fast_Read                          0x0B           /* 快速读数据指令 *///

#define CMD_Page_Program                       0x02           /* 页编程指令 *///
#define CMD_Sector_Erase_4KB                   0x20           /* 4KB扇区擦除指令 *///
#define CMD_Block_Erase_32KB                   0x52           /* 32KB块擦除指令 *///
#define CMD_Block_Erase_64KB                   0xD8           /* 64KB块擦除指令 *///
#define CMD_Chip_Erase                         0xC7           /* 全片擦除指令 *///
#define CMD_Chip_Erase_1                       0x60           /* 全片擦除指令1 *///

#define CMD_Read_Status_Register_1             0x05           /* 读状态寄存器1指令 *///
#define CMD_Write_Status_Register_1            0x01           /* 写状态寄存器1指令 *///
#define CMD_Read_Status_Register_2             0x35           /* 读状态寄存器2指令 *///
#define CMD_Write_Status_Register_2            0x31           /* 写状态寄存器2指令 *///
#define CMD_Read_Status_Register_3             0x15           /* 读状态寄存器3指令 *///
#define CMD_Write_Status_Register_3            0x11           /* 写状态寄存器3指令 *///

#define CMD_Read_SFDP_Register                 0x5A           /* 读SFDP寄存器指令 *///
#define CMD_Erase_Security_Register            0x44           /* 擦除安全寄存器指令 *///
#define CMD_Program_Security_Register          0x42           /* 编程安全寄存器指令 *///
#define CMD_Read_Security_Register             0x48           /* 读安全寄存器指令 *///
#define CMD_Global_Block_Lock                  0x7E           /* 全局块锁定指令 *///没测试
#define CMD_Global_Block_Unlock                0x98           /* 全局块解锁指令 *///没测试
#define CMD_Read_Block_Lock                    0x3D           /* 读全局块锁定状态指令 *///没测试
#define CMD_Individual_Block_Lock              0x36           /* 单独块锁定指令 *///没测试
#define CMD_Individual_Block_Unlock            0x39           /* 单独块解锁指令 *///没测试

#define CMD_Erase__Program_Suspend             0x75           /* 擦除/编程挂起指令 *///
#define CMD_Erase__Program_Resume              0x7A           /* 擦除/编程恢复指令 *///
#define CMD_Power_down                         0xB9           /* 掉电指令 *///
#define CMD_Enable_Reset                       0x66           /* 使能复位指令 *///
#define CMD_Reset_Device                       0x99           /* 复位设备指令 *///

#define CMD_Fast_Read_Dual_Output              0x3B           /* 快速读取双输出指令 *///
#define CMD_Mftr__Device_ID_Dual_IO            0x92           /* 读取制造商/设备ID双输出指令 *///
#define CMD_Fast_Read_Dual_IO                  0xBB           /* 快速读取双输入输出指令 *///
#define CMD_Quad_Input_Page_Program            0x32           /* 页编程四输入指令 *///
#define CMD_Fast_Read_Quad_Output              0x6B           /* 快速读取四输出指令 *///
#define CMD_Mftr__Device_ID_Quad_IO            0x94           /* 读取制造商/设备ID四输入输出指令 *///
#define CMD_Fast_Read_Quad_IO                  0xEB           /* 快速读取四输入输出指令 *///
#define CMD_Set_Burst_with_Wrap                0x77           /* 设置环绕指令 *//////////////////不做

#define CMD_ENTER_4_BYTE_ADDR_MODE             0xB7           /* 进入4地址模式指令 */
#define CMD_EXIT_4_BYTE_ADDR_MODE              0xE9           /* 退出4地址模式指令 */

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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
