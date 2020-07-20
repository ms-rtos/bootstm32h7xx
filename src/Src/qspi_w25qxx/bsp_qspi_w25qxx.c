/*
*********************************************************************************************************
*
*  模块名称 : W25Q256 QSPI驱动模块
*  文件名称 : bsp_qspi_w25q256.c
*  版    本 : V1.0
*  说    明 : 使用CPU的QSPI总线驱动串行FLASH，提供基本的读写函数，采用4线方式，MDMA传输
*
*  修改记录 :
*    版本号  日期         作者       说明
*    V1.0    2020-01-15   Eric2013    正式发布
*
*  Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

//#include "bsp.h"
#include "bsp_qspi_w25qxx.h"

/* 
  STM32-V7开发板接线

  PB6/QUADSPI_BK1_NCS   AF10
  PB2/QUADSPI_CLK       AF9
  PD11/QUADSPI_BK1_IO0  AF9
  PC10/QUADSPI_BK1_IO1  AF9
  PE2/QUADSPI_BK1_IO2   AF9
  PA1/QUADSPI_BK1_IO3   AF9

  W25Q256JV有512块，每块有16个扇区，每个扇区Sector有16页，每页有256字节，共计32MB
*/

/* QSPI引脚和时钟相关配置宏定义 */
#define QSPI_CLK_ENABLE()              __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()             __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_BK1_CS_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_BK1_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()
#define QSPI_BK1_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
#define QSPI_BK1_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()
#define QSPI_BK1_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()

#define QSPI_MDMA_CLK_ENABLE()         __HAL_RCC_MDMA_CLK_ENABLE()
#define QSPI_FORCE_RESET()             __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()           __HAL_RCC_QSPI_RELEASE_RESET()

#define QSPI_CLK_PIN                   GPIO_PIN_2
#define QSPI_CLK_GPIO_PORT             GPIOB
#define QSPI_CLK_PIN_AF_TYPE           GPIO_AF9_QUADSPI

#define QSPI_BK1_CS_PIN                GPIO_PIN_6
#define QSPI_BK1_CS_GPIO_PORT          GPIOB
#define QSPI_BK1_CS_PIN_AF_TYPE        GPIO_AF10_QUADSPI

#define QSPI_BK1_D0_PIN                GPIO_PIN_11
#define QSPI_BK1_D0_GPIO_PORT          GPIOD
#define QSPI_BK1_D0_PIN_AF_TYPE        GPIO_AF9_QUADSPI

#define QSPI_BK1_D1_PIN                GPIO_PIN_10
#define QSPI_BK1_D1_GPIO_PORT          GPIOC
#define QSPI_BK1_D1_PIN_AF_TYPE        GPIO_AF9_QUADSPI

#define QSPI_BK1_D2_PIN                GPIO_PIN_2
#define QSPI_BK1_D2_GPIO_PORT          GPIOE
#define QSPI_BK1_D2_PIN_AF_TYPE        GPIO_AF9_QUADSPI

#define QSPI_BK1_D3_PIN                GPIO_PIN_1
#define QSPI_BK1_D3_GPIO_PORT          GPIOA
#define QSPI_BK1_D3_PIN_AF_TYPE        GPIO_AF9_QUADSPI

/* 供本文件调用的全局变量 */
QSPI_HandleTypeDef  QSPIHandle;
uint32_t            Flash_ID;   /* FLASH ID */

static void Error_Handler(char *file, uint32_t line);

/*
*********************************************************************************************************
*  函 数 名: bsp_InitQSPI_W25Q32
*  功能说明: QSPI Flash硬件初始化，配置基本参数
*  形    参: 无
*  返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitQSPI_W25Q32(void)
{
  uint8_t uCount;
  
  /* 复位QSPI */
  QSPIHandle.Instance = QUADSPI;
  if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  /* 设置时钟速度，QSPI clock = 200MHz / (ClockPrescaler+1) = 100MHz */
  QSPIHandle.Init.ClockPrescaler  = 1;
  
  /* 设置FIFO阀值，范围1 - 32 */
  QSPIHandle.Init.FifoThreshold   = 32;
  
  /*
    QUADSPI在FLASH驱动信号后过半个CLK周期才对FLASH驱动的数据采样。
    在外部信号延迟时，这有利于推迟数据采样。
  */
  QSPIHandle.Init.SampleShifting  = QSPI_SAMPLE_SHIFTING_HALFCYCLE; 
  
  /*Flash大小是2^(FlashSize + 1) = 2^25 = 32MB */
  QSPIHandle.Init.FlashSize       = QSPI_FLASH_SIZE - 1; 
  
  /* 命令之间的CS片选至少保持1个时钟周期的高电平 */
  QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;
  
  /*
     MODE0: 表示片选信号空闲期间，CLK时钟信号是低电平
     MODE3: 表示片选信号空闲期间，CLK时钟信号是高电平
     W25Q32JV对两种模式均支持
  */
  QSPIHandle.Init.ClockMode = QSPI_CLOCK_MODE_0;
  
  /* QSPI有两个BANK，这里使用的BANK1 */
  QSPIHandle.Init.FlashID   = QSPI_FLASH_ID_1;
  
  /* V7开发板仅使用了BANK1，这里是禁止双BANK */
  QSPIHandle.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  
  /* 初始化配置QSPI */
  if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  /* 退出QPI模式 */
  W25QXX_ExitQPIMode();
  /* QSPI memory reset */
  QSPI_Enable_Reset(&QSPIHandle);
  QSPI_Reset_Device(&QSPIHandle);
  /* 读取flash id */
  Flash_ID = QSPI_Read_JEDEC_ID();
  if(Flash_ID != FLASH_JEDEC_ID)
  {
    while(1);
  }
  
  /* W25Q32JV状态寄存器配置 */
  uCount = 0x00;
  QSPI_Write_Status_Register(1,&uCount);
  uCount = 0x02;
  QSPI_Write_Status_Register(2,&uCount);
  uCount = 0x20;
  QSPI_Write_Status_Register(3,&uCount);
  
  /* 进入QPI模式 */
  W25QXX_EnterQPIMode();

  if (FLASH_ADDR_MODE == QSPI_ADDRESS_32_BITS) {
    /* 进入4地址模式 */
    W25QXX_Enter4BytesAddressMode(&QSPIHandle, W25QXX_QPI_MODE);
  }
}

/*
*********************************************************************************************************
*  函 数 名: HAL_QSPI_MspInit
*  功能说明: QSPI底层初始化，被HAL_QSPI_Init调用的底层函数
*  形    参: 无
*  返 回 值: 无
*********************************************************************************************************
*/
void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* 使能QPSI时钟  */
  QSPI_CLK_ENABLE();
  
  /* 复位时钟接口 */
  QSPI_FORCE_RESET();
  QSPI_RELEASE_RESET();
  
  /* 使能GPIO时钟 */
  QSPI_CLK_GPIO_CLK_ENABLE();
  QSPI_BK1_CS_GPIO_CLK_ENABLE();
  QSPI_BK1_D0_GPIO_CLK_ENABLE();
  QSPI_BK1_D1_GPIO_CLK_ENABLE();
  QSPI_BK1_D2_GPIO_CLK_ENABLE();
  QSPI_BK1_D3_GPIO_CLK_ENABLE(); 

  /* QSPI CS GPIO 引脚配置 */
  GPIO_InitStruct.Pin       = QSPI_BK1_CS_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = QSPI_BK1_CS_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_CS_GPIO_PORT, &GPIO_InitStruct);
  
  /* QSPI CLK GPIO 引脚配置 */
  GPIO_InitStruct.Pin       = QSPI_CLK_PIN;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = QSPI_CLK_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &GPIO_InitStruct);

  /* QSPI D0 GPIO pin 引脚配置 */
  GPIO_InitStruct.Pin       = QSPI_BK1_D0_PIN;
  GPIO_InitStruct.Alternate = QSPI_BK1_D0_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &GPIO_InitStruct);

  /* QSPI D1 GPIO 引脚配置 */
  GPIO_InitStruct.Pin       = QSPI_BK1_D1_PIN;
  GPIO_InitStruct.Alternate = QSPI_BK1_D1_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &GPIO_InitStruct);

  /* QSPI D2 GPIO 引脚配置 */
  GPIO_InitStruct.Pin       = QSPI_BK1_D2_PIN;
  GPIO_InitStruct.Alternate = QSPI_BK1_D2_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &GPIO_InitStruct);

  /* QSPI D3 GPIO 引脚配置 */
  GPIO_InitStruct.Pin       = QSPI_BK1_D3_PIN;
  GPIO_InitStruct.Alternate = QSPI_BK1_D3_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &GPIO_InitStruct);
}

/*
*********************************************************************************************************
*  函 数 名: HAL_QSPI_MspDeInit
*  功能说明: QSPI底层复位，被HAL_QSPI_Init调用的底层函数
*  形    参: hqspi QSPI_HandleTypeDef类型句柄
*  返 回 值: 无
*********************************************************************************************************
*/
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
  /* 复位QSPI引脚 */
  HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_CS_GPIO_PORT, QSPI_BK1_CS_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);
  
  /* 复位QSPI */
  QSPI_FORCE_RESET();
  QSPI_RELEASE_RESET();
  
  /* 关闭QSPI时钟 */
  QSPI_CLK_DISABLE();
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_WriteEnable
*  功能说明: 写使能
*  形    参: hqspi  QSPI_HandleTypeDef句柄。
*  返 回 值: 无
*********************************************************************************************************
*/
void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* 数据发送模式    :无需数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q256JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_Write_Enable;  /* 写使能指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000000;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
//  /* 等待写使能完成 */
//  StatusMatch = 0;
//  QSPI_AutoPollingMemReady(&QSPIHandle);  
//  while(StatusMatch == 0);
//  StatusMatch = 0;
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_WriteDisable
*  功能说明: 写失能
*  形    参: hqspi  QSPI_HandleTypeDef句柄。
*  返 回 值: 无
*********************************************************************************************************
*/
void QSPI_WriteDisable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* 数据发送模式    :无需数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q256JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_Write_Disable;  /* 写失能指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000000;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_Read_Status_Register
*  功能说明: 读状态寄存器。
*  形    参: _uiReadAddr : 状态寄存器序号(1/2/3)。
*  返 回 值: 状态值
*********************************************************************************************************
*/
uint8_t QSPI_Read_Status_Register(const uint8_t _uiReadAddr)
{
  QSPI_CommandTypeDef sCommand = {0};
  uint8_t buf[1];
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* 数据发送模式    :1线方式发送数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q256JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = _uiReadAddr==1?CMD_Read_Status_Register_1:\
                               _uiReadAddr==2?CMD_Read_Status_Register_2:\
                               _uiReadAddr==3?CMD_Read_Status_Register_3:CMD_Read_Status_Register_1;  /* 读状态寄存器指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000001;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  if (HAL_QSPI_Receive(&QSPIHandle, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  return buf[0];
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_Write_Status_Register
*  功能说明: 写状态寄存器。
*  形    参: _uiReadAddr : 状态寄存器序号(1/2/3)。
*            _pBuf       : 写入值
*  返 回 值: 无
*********************************************************************************************************
*/
void QSPI_Write_Status_Register(uint8_t _uiReadAddr, const uint8_t *_pBuf)
{
  QSPI_CommandTypeDef sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* 数据发送模式    :1线方式发送数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q256JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = _uiReadAddr==1?CMD_Write_Status_Register_1:\
                               _uiReadAddr==2?CMD_Write_Status_Register_2:\
                               _uiReadAddr==3?CMD_Write_Status_Register_3:CMD_Write_Status_Register_1;  /* 写状态寄存器指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000001;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  if (HAL_QSPI_Transmit(&QSPIHandle, (uint8_t *)_pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_Read_Data
*  功能说明: 连续读取若干字节，字节个数不能超出芯片容量。
*  形    参: _pBuf : 数据源缓冲区。
*      _uiReadAddr : 起始地址。
*      _usSize     : 数据个数, 可以大于PAGE_SIZE, 但是不能超出芯片总容量。
*  返 回 值: 无
*  注意:此指令最大支持的时钟为50MHz
*********************************************************************************************************
*/
void QSPI_Read_Data(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
  QSPI_CommandTypeDef sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;        /* 地址发送模式    :1线方式 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* 数据发送模式    :1线方式 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q256JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_Read_Data;  /* 读数据指令 */
  sCommand.Address           = _uiReadAddr;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = _uiSize;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  if (HAL_QSPI_Receive(&QSPIHandle, _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_Fast_Read
*  功能说明: 快速读取若干字节，字节个数不能超出芯片容量。
*  形    参: _pBuf : 数据源缓冲区。
*      _uiReadAddr : 起始地址。
*      _usSize     : 数据个数, 可以大于PAGE_SIZE, 但是不能超出芯片总容量。
*  返 回 值: 无
*********************************************************************************************************
*/
void QSPI_Fast_Read(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
  QSPI_CommandTypeDef sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;        /* 地址发送模式    :1线方式 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 8;                          /* 空周期位数      :8  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* 数据发送模式    :1线方式 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q256JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_Fast_Read;  /* 快速读数据指令 */
  sCommand.Address           = _uiReadAddr;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = _uiSize;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  if (HAL_QSPI_Receive(&QSPIHandle, _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_Fast_Read_Quad_IO
*  功能说明: 快速读取四输入输出读取若干字节，字节个数不能超出芯片容量。
*  形    参: _pBuf : 数据源缓冲区。
*      _uiReadAddr : 起始地址。
*      _usSize     : 数据个数, 可以大于PAGE_SIZE, 但是不能超出芯片总容量。
*  返 回 值: 无
*********************************************************************************************************
*/
void QSPI_Fast_Read_Quad_IO(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
  QSPI_CommandTypeDef sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_2_LINES;       /* 地址发送模式    :2线方式 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;/* 交替字节发送模式:2线方式 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_2_LINES;          /* 数据发送模式    :2线方式 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q256JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_Fast_Read_Dual_IO;  /* 快速读取双输入输出指令 */
  sCommand.Address           = _uiReadAddr;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x000000FF;/* 必须是0xFx */
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_8_BITS;
  sCommand.NbData            = _uiSize;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  if (HAL_QSPI_Receive(&QSPIHandle, _pBuf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_Read_JEDEC_ID
*  功能说明: 读取器件JEDEC ID
*  形    参: 无
*  返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit）W25Q32JV--->0x00EF4016
*********************************************************************************************************
*/
uint32_t QSPI_Read_JEDEC_ID(void)
{
  QSPI_CommandTypeDef sCommand = {0};
  uint32_t uiID;
  uint8_t  buf[3];
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无需交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* 数据发送模式    :1线方式 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q32JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_JEDEC_ID;  /* 读取器件JEDEC ID指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000003;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  if (HAL_QSPI_Receive(&QSPIHandle, buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  uiID = (buf[0] << 16) | (buf[1] << 8 ) | buf[2];
  
  return uiID;
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_Enable_Reset
*  功能说明: 使能复位
*  形    参: hqspi  QSPI_HandleTypeDef句柄。
*  返 回 值: 无
*********************************************************************************************************
*/
void QSPI_Enable_Reset(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* 数据发送模式    :无需数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q32JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_Enable_Reset;  /* 使能复位指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000000;
  
  if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_Reset_Device
*  功能说明: 复位设备
*  形    参: hqspi  QSPI_HandleTypeDef句柄。
*  返 回 值: 无
*********************************************************************************************************
*/
void QSPI_Reset_Device(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* 数据发送模式    :无需数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q32JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_Reset_Device;  /* 复位设备指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000000;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*  函 数 名: QSPI_EnableMemoryMappedMode
*  功能说明: 使能内存映射模式
*  形    参: hqspi  QSPI_HandleTypeDef句柄。
*  返 回 值: 0:成功  1:失败
*********************************************************************************************************
*/
uint8_t QSPI_EnableMemoryMappedMode(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef      sCommand = {0};
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;   /* 指令发送模式    :4线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;       /* 地址发送模式    :4线方式 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无需交替字节 */
  sCommand.DummyCycles       = 6;                          /* 空周期位数      :6  */
  sCommand.DataMode          = QSPI_DATA_4_LINES;          /* 数据发送模式    :4线方式 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q32JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = CMD_Fast_Read_Quad_IO;  /* 快速读取四输入输出指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = FLASH_ADDR_MODE;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_8_BITS;
  sCommand.NbData            = 0x00000000;
  
  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod     = 0;
  
  if (HAL_QSPI_MemoryMapped(hqspi, &sCommand, &s_mem_mapped_cfg) != HAL_OK)
  {
    return 1;
  }
  
  return 0;
}

/*
*********************************************************************************************************
*  函 数 名: W25QXX_ExitQPIMode
*  功能说明: 退出QPI模式
*  形    参: 无
*  返 回 值: 无
*********************************************************************************************************
*/
void W25QXX_ExitQPIMode(void)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;   /* 指令发送模式    :4线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* 数据发送模式    :无需数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q32JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = W25X_ExitQPIMode;  /* 退出QPI模式指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000000;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/*
*********************************************************************************************************
*  函 数 名: W25QXX_EnterQPIMode
*  功能说明: 进入QPI模式
*  形    参: 无
*  返 回 值: 无
*********************************************************************************************************
*/
void W25QXX_EnterQPIMode(void)
{
  uint8_t dat;
  QSPI_CommandTypeDef     sCommand = {0};
  
  dat = QSPI_Read_Status_Register(2); //先读出状态寄存器2的原始值
  if ((dat & QE_MASK) == 0x00)        //QE位未使能
  {
    QSPI_WriteEnable(&QSPIHandle);    //写使能
    dat |= QE_MASK;                   //使能QE位
    QSPI_Write_Status_Register(2, &dat);//写状态寄存器2
  }
  
  /*
  sCommand.Instruction           指令数据          @0x0 - 0xFF
  sCommand.Address               地址数据          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        交替字节数据      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           地址位数          @8/16/24/32
  sCommand.AlternateBytesSize    交替字节位数      @8/16/24/32
  sCommand.DummyCycles           空周期位数        @0 - 31
  sCommand.InstructionMode       指令发送模式      @无/1/2/4线
  sCommand.AddressMode           地址发送模式      @无/1/2/4线
  sCommand.AlternateByteMode     交替字节发送模式  @无/1/2/4线
  sCommand.DataMode              数据发送模式      @无/1/2/4线
  sCommand.NbData                数据发送长度      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDR模式           @使能/失能
  sCommand.DdrHoldHalfCycle      延迟输出数据      @使能/失能
  sCommand.SIOOMode              指令仅发送一次    @使能/失能
  */
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* 指令发送模式    :1线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* 数据发送模式    :无需数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q32JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = W25X_EnterQPIMode;  /* 进入QPI模式指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000000;
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  /* 基本配置 */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;   /* 指令发送模式    :4线方式发送指令 */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* 地址发送模式    :无需地址 */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* 交替字节发送模式:无交替字节 */
  sCommand.DummyCycles       = 0;                          /* 空周期位数      :0  */
  sCommand.DataMode          = QSPI_DATA_4_LINES;          /* 数据发送模式    :4线方式发送数据 */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDR模式         :W25Q32JV不支持DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* 延迟输出数据    :DDR模式，数据输出延迟 */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* 指令仅发送一次  :每次传输都发指令 */
  
  /* 数据配置 */
  sCommand.Instruction       = W25X_SetReadParameters;  /* 设置QPI模式参数指令 */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000001;
  dat                        = 0x02 << 4; //设置P5&P4=10,6个dummy clocks,104MHz
  QSPI_WriteEnable(&QSPIHandle);//这个可有可无
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  if (HAL_QSPI_Transmit(&QSPIHandle, (uint8_t *)&dat, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
  {
    Error_Handler(__FILE__, __LINE__);
  }
}

/**
  * @brief  This function set the QSPI memory in 4-byte address mode
  *          SPI/QPI; 1-0-1/4-0-4
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
void W25QXX_Enter4BytesAddressMode(QSPI_HandleTypeDef *hqspi, W25QXX_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the command */
  s_command.InstructionMode   = (Mode == W25QXX_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = CMD_ENTER_4_BYTE_ADDR_MODE;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /*write enable */
  QSPI_WriteEnable(hqspi);

  /* Send the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

}

/**
  * @brief  Flash exit 4 Byte address mode. Effect 3/4 address byte commands only.
  *         SPI/QPI; 1-0-0/4-0-0
  * @param  Ctx Component object pointer
  * @param  Mode Interface mode
  * @retval QSPI memory status
  */
void W25QXX_Exit4BytesAddressMode(QSPI_HandleTypeDef *Ctx, W25QXX_Interface_t Mode)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the command */
  s_command.InstructionMode   = (Mode == W25QXX_QPI_MODE) ? QSPI_INSTRUCTION_4_LINES : QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = CMD_EXIT_4_BYTE_ADDR_MODE;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(Ctx, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }

}

void Error_Handler(char *file, uint32_t line)
{
    while (1) { ; };
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
