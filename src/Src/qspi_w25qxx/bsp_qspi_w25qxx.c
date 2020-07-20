/*
*********************************************************************************************************
*
*  ģ������ : W25Q256 QSPI����ģ��
*  �ļ����� : bsp_qspi_w25q256.c
*  ��    �� : V1.0
*  ˵    �� : ʹ��CPU��QSPI������������FLASH���ṩ�����Ķ�д����������4�߷�ʽ��MDMA����
*
*  �޸ļ�¼ :
*    �汾��  ����         ����       ˵��
*    V1.0    2020-01-15   Eric2013    ��ʽ����
*
*  Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

//#include "bsp.h"
#include "bsp_qspi_w25qxx.h"

/* 
  STM32-V7���������

  PB6/QUADSPI_BK1_NCS   AF10
  PB2/QUADSPI_CLK       AF9
  PD11/QUADSPI_BK1_IO0  AF9
  PC10/QUADSPI_BK1_IO1  AF9
  PE2/QUADSPI_BK1_IO2   AF9
  PA1/QUADSPI_BK1_IO3   AF9

  W25Q256JV��512�飬ÿ����16��������ÿ������Sector��16ҳ��ÿҳ��256�ֽڣ�����32MB
*/

/* QSPI���ź�ʱ��������ú궨�� */
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

/* �����ļ����õ�ȫ�ֱ��� */
QSPI_HandleTypeDef  QSPIHandle;
uint32_t            Flash_ID;   /* FLASH ID */

static void Error_Handler(char *file, uint32_t line);

/*
*********************************************************************************************************
*  �� �� ��: bsp_InitQSPI_W25Q32
*  ����˵��: QSPI FlashӲ����ʼ�������û�������
*  ��    ��: ��
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitQSPI_W25Q32(void)
{
  uint8_t uCount;
  
  /* ��λQSPI */
  QSPIHandle.Instance = QUADSPI;
  if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  /* ����ʱ���ٶȣ�QSPI clock = 200MHz / (ClockPrescaler+1) = 100MHz */
  QSPIHandle.Init.ClockPrescaler  = 1;
  
  /* ����FIFO��ֵ����Χ1 - 32 */
  QSPIHandle.Init.FifoThreshold   = 32;
  
  /*
    QUADSPI��FLASH�����źź�����CLK���ڲŶ�FLASH���������ݲ�����
    ���ⲿ�ź��ӳ�ʱ�����������Ƴ����ݲ�����
  */
  QSPIHandle.Init.SampleShifting  = QSPI_SAMPLE_SHIFTING_HALFCYCLE; 
  
  /*Flash��С��2^(FlashSize + 1) = 2^25 = 32MB */
  QSPIHandle.Init.FlashSize       = QSPI_FLASH_SIZE - 1; 
  
  /* ����֮���CSƬѡ���ٱ���1��ʱ�����ڵĸߵ�ƽ */
  QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;
  
  /*
     MODE0: ��ʾƬѡ�źſ����ڼ䣬CLKʱ���ź��ǵ͵�ƽ
     MODE3: ��ʾƬѡ�źſ����ڼ䣬CLKʱ���ź��Ǹߵ�ƽ
     W25Q32JV������ģʽ��֧��
  */
  QSPIHandle.Init.ClockMode = QSPI_CLOCK_MODE_0;
  
  /* QSPI������BANK������ʹ�õ�BANK1 */
  QSPIHandle.Init.FlashID   = QSPI_FLASH_ID_1;
  
  /* V7�������ʹ����BANK1�������ǽ�ֹ˫BANK */
  QSPIHandle.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  
  /* ��ʼ������QSPI */
  if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  /* �˳�QPIģʽ */
  W25QXX_ExitQPIMode();
  /* QSPI memory reset */
  QSPI_Enable_Reset(&QSPIHandle);
  QSPI_Reset_Device(&QSPIHandle);
  /* ��ȡflash id */
  Flash_ID = QSPI_Read_JEDEC_ID();
  if(Flash_ID != FLASH_JEDEC_ID)
  {
    while(1);
  }
  
  /* W25Q32JV״̬�Ĵ������� */
  uCount = 0x00;
  QSPI_Write_Status_Register(1,&uCount);
  uCount = 0x02;
  QSPI_Write_Status_Register(2,&uCount);
  uCount = 0x20;
  QSPI_Write_Status_Register(3,&uCount);
  
  /* ����QPIģʽ */
  W25QXX_EnterQPIMode();

  if (FLASH_ADDR_MODE == QSPI_ADDRESS_32_BITS) {
    /* ����4��ַģʽ */
    W25QXX_Enter4BytesAddressMode(&QSPIHandle, W25QXX_QPI_MODE);
  }
}

/*
*********************************************************************************************************
*  �� �� ��: HAL_QSPI_MspInit
*  ����˵��: QSPI�ײ��ʼ������HAL_QSPI_Init���õĵײ㺯��
*  ��    ��: ��
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* ʹ��QPSIʱ��  */
  QSPI_CLK_ENABLE();
  
  /* ��λʱ�ӽӿ� */
  QSPI_FORCE_RESET();
  QSPI_RELEASE_RESET();
  
  /* ʹ��GPIOʱ�� */
  QSPI_CLK_GPIO_CLK_ENABLE();
  QSPI_BK1_CS_GPIO_CLK_ENABLE();
  QSPI_BK1_D0_GPIO_CLK_ENABLE();
  QSPI_BK1_D1_GPIO_CLK_ENABLE();
  QSPI_BK1_D2_GPIO_CLK_ENABLE();
  QSPI_BK1_D3_GPIO_CLK_ENABLE(); 

  /* QSPI CS GPIO �������� */
  GPIO_InitStruct.Pin       = QSPI_BK1_CS_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = QSPI_BK1_CS_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_CS_GPIO_PORT, &GPIO_InitStruct);
  
  /* QSPI CLK GPIO �������� */
  GPIO_InitStruct.Pin       = QSPI_CLK_PIN;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = QSPI_CLK_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &GPIO_InitStruct);

  /* QSPI D0 GPIO pin �������� */
  GPIO_InitStruct.Pin       = QSPI_BK1_D0_PIN;
  GPIO_InitStruct.Alternate = QSPI_BK1_D0_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &GPIO_InitStruct);

  /* QSPI D1 GPIO �������� */
  GPIO_InitStruct.Pin       = QSPI_BK1_D1_PIN;
  GPIO_InitStruct.Alternate = QSPI_BK1_D1_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &GPIO_InitStruct);

  /* QSPI D2 GPIO �������� */
  GPIO_InitStruct.Pin       = QSPI_BK1_D2_PIN;
  GPIO_InitStruct.Alternate = QSPI_BK1_D2_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &GPIO_InitStruct);

  /* QSPI D3 GPIO �������� */
  GPIO_InitStruct.Pin       = QSPI_BK1_D3_PIN;
  GPIO_InitStruct.Alternate = QSPI_BK1_D3_PIN_AF_TYPE;
  HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &GPIO_InitStruct);
}

/*
*********************************************************************************************************
*  �� �� ��: HAL_QSPI_MspDeInit
*  ����˵��: QSPI�ײ㸴λ����HAL_QSPI_Init���õĵײ㺯��
*  ��    ��: hqspi QSPI_HandleTypeDef���;��
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
  /* ��λQSPI���� */
  HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_CS_GPIO_PORT, QSPI_BK1_CS_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
  HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);
  
  /* ��λQSPI */
  QSPI_FORCE_RESET();
  QSPI_RELEASE_RESET();
  
  /* �ر�QSPIʱ�� */
  QSPI_CLK_DISABLE();
}

/*
*********************************************************************************************************
*  �� �� ��: QSPI_WriteEnable
*  ����˵��: дʹ��
*  ��    ��: hqspi  QSPI_HandleTypeDef�����
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* ���ݷ���ģʽ    :�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q256JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_Write_Enable;  /* дʹ��ָ�� */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000000;
  
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
//  /* �ȴ�дʹ����� */
//  StatusMatch = 0;
//  QSPI_AutoPollingMemReady(&QSPIHandle);  
//  while(StatusMatch == 0);
//  StatusMatch = 0;
}

/*
*********************************************************************************************************
*  �� �� ��: QSPI_WriteDisable
*  ����˵��: дʧ��
*  ��    ��: hqspi  QSPI_HandleTypeDef�����
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_WriteDisable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* ���ݷ���ģʽ    :�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q256JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_Write_Disable;  /* дʧ��ָ�� */
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
*  �� �� ��: QSPI_Read_Status_Register
*  ����˵��: ��״̬�Ĵ�����
*  ��    ��: _uiReadAddr : ״̬�Ĵ������(1/2/3)��
*  �� �� ֵ: ״ֵ̬
*********************************************************************************************************
*/
uint8_t QSPI_Read_Status_Register(const uint8_t _uiReadAddr)
{
  QSPI_CommandTypeDef sCommand = {0};
  uint8_t buf[1];
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* ���ݷ���ģʽ    :1�߷�ʽ�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q256JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = _uiReadAddr==1?CMD_Read_Status_Register_1:\
                               _uiReadAddr==2?CMD_Read_Status_Register_2:\
                               _uiReadAddr==3?CMD_Read_Status_Register_3:CMD_Read_Status_Register_1;  /* ��״̬�Ĵ���ָ�� */
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
*  �� �� ��: QSPI_Write_Status_Register
*  ����˵��: д״̬�Ĵ�����
*  ��    ��: _uiReadAddr : ״̬�Ĵ������(1/2/3)��
*            _pBuf       : д��ֵ
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_Write_Status_Register(uint8_t _uiReadAddr, const uint8_t *_pBuf)
{
  QSPI_CommandTypeDef sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* ���ݷ���ģʽ    :1�߷�ʽ�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q256JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = _uiReadAddr==1?CMD_Write_Status_Register_1:\
                               _uiReadAddr==2?CMD_Write_Status_Register_2:\
                               _uiReadAddr==3?CMD_Write_Status_Register_3:CMD_Write_Status_Register_1;  /* д״̬�Ĵ���ָ�� */
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
*  �� �� ��: QSPI_Read_Data
*  ����˵��: ������ȡ�����ֽڣ��ֽڸ������ܳ���оƬ������
*  ��    ��: _pBuf : ����Դ��������
*      _uiReadAddr : ��ʼ��ַ��
*      _usSize     : ���ݸ���, ���Դ���PAGE_SIZE, ���ǲ��ܳ���оƬ��������
*  �� �� ֵ: ��
*  ע��:��ָ�����֧�ֵ�ʱ��Ϊ50MHz
*********************************************************************************************************
*/
void QSPI_Read_Data(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
  QSPI_CommandTypeDef sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;        /* ��ַ����ģʽ    :1�߷�ʽ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* ���ݷ���ģʽ    :1�߷�ʽ */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q256JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_Read_Data;  /* ������ָ�� */
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
*  �� �� ��: QSPI_Fast_Read
*  ����˵��: ���ٶ�ȡ�����ֽڣ��ֽڸ������ܳ���оƬ������
*  ��    ��: _pBuf : ����Դ��������
*      _uiReadAddr : ��ʼ��ַ��
*      _usSize     : ���ݸ���, ���Դ���PAGE_SIZE, ���ǲ��ܳ���оƬ��������
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_Fast_Read(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
  QSPI_CommandTypeDef sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;        /* ��ַ����ģʽ    :1�߷�ʽ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 8;                          /* ������λ��      :8  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* ���ݷ���ģʽ    :1�߷�ʽ */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q256JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_Fast_Read;  /* ���ٶ�����ָ�� */
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
*  �� �� ��: QSPI_Fast_Read_Quad_IO
*  ����˵��: ���ٶ�ȡ�����������ȡ�����ֽڣ��ֽڸ������ܳ���оƬ������
*  ��    ��: _pBuf : ����Դ��������
*      _uiReadAddr : ��ʼ��ַ��
*      _usSize     : ���ݸ���, ���Դ���PAGE_SIZE, ���ǲ��ܳ���оƬ��������
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_Fast_Read_Quad_IO(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
  QSPI_CommandTypeDef sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_2_LINES;       /* ��ַ����ģʽ    :2�߷�ʽ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_2_LINES;/* �����ֽڷ���ģʽ:2�߷�ʽ */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_2_LINES;          /* ���ݷ���ģʽ    :2�߷�ʽ */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q256JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_Fast_Read_Dual_IO;  /* ���ٶ�ȡ˫�������ָ�� */
  sCommand.Address           = _uiReadAddr;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x000000FF;/* ������0xFx */
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
*  �� �� ��: QSPI_Read_JEDEC_ID
*  ����˵��: ��ȡ����JEDEC ID
*  ��    ��: ��
*  �� �� ֵ: 32bit������ID (���8bit��0����ЧIDλ��Ϊ24bit��W25Q32JV--->0x00EF4016
*********************************************************************************************************
*/
uint32_t QSPI_Read_JEDEC_ID(void)
{
  QSPI_CommandTypeDef sCommand = {0};
  uint32_t uiID;
  uint8_t  buf[3];
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:���轻���ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_1_LINE;           /* ���ݷ���ģʽ    :1�߷�ʽ */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q32JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_JEDEC_ID;  /* ��ȡ����JEDEC IDָ�� */
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
*  �� �� ��: QSPI_Enable_Reset
*  ����˵��: ʹ�ܸ�λ
*  ��    ��: hqspi  QSPI_HandleTypeDef�����
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_Enable_Reset(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* ���ݷ���ģʽ    :�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q32JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_Enable_Reset;  /* ʹ�ܸ�λָ�� */
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
*  �� �� ��: QSPI_Reset_Device
*  ����˵��: ��λ�豸
*  ��    ��: hqspi  QSPI_HandleTypeDef�����
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void QSPI_Reset_Device(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* ���ݷ���ģʽ    :�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q32JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_Reset_Device;  /* ��λ�豸ָ�� */
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
*  �� �� ��: QSPI_EnableMemoryMappedMode
*  ����˵��: ʹ���ڴ�ӳ��ģʽ
*  ��    ��: hqspi  QSPI_HandleTypeDef�����
*  �� �� ֵ: 0:�ɹ�  1:ʧ��
*********************************************************************************************************
*/
uint8_t QSPI_EnableMemoryMappedMode(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef      sCommand = {0};
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;   /* ָ���ģʽ    :4�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_4_LINES;       /* ��ַ����ģʽ    :4�߷�ʽ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:���轻���ֽ� */
  sCommand.DummyCycles       = 6;                          /* ������λ��      :6  */
  sCommand.DataMode          = QSPI_DATA_4_LINES;          /* ���ݷ���ģʽ    :4�߷�ʽ */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q32JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = CMD_Fast_Read_Quad_IO;  /* ���ٶ�ȡ���������ָ�� */
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
*  �� �� ��: W25QXX_ExitQPIMode
*  ����˵��: �˳�QPIģʽ
*  ��    ��: ��
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void W25QXX_ExitQPIMode(void)
{
  QSPI_CommandTypeDef     sCommand = {0};
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;   /* ָ���ģʽ    :4�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* ���ݷ���ģʽ    :�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q32JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = W25X_ExitQPIMode;  /* �˳�QPIģʽָ�� */
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
*  �� �� ��: W25QXX_EnterQPIMode
*  ����˵��: ����QPIģʽ
*  ��    ��: ��
*  �� �� ֵ: ��
*********************************************************************************************************
*/
void W25QXX_EnterQPIMode(void)
{
  uint8_t dat;
  QSPI_CommandTypeDef     sCommand = {0};
  
  dat = QSPI_Read_Status_Register(2); //�ȶ���״̬�Ĵ���2��ԭʼֵ
  if ((dat & QE_MASK) == 0x00)        //QEλδʹ��
  {
    QSPI_WriteEnable(&QSPIHandle);    //дʹ��
    dat |= QE_MASK;                   //ʹ��QEλ
    QSPI_Write_Status_Register(2, &dat);//д״̬�Ĵ���2
  }
  
  /*
  sCommand.Instruction           ָ������          @0x0 - 0xFF
  sCommand.Address               ��ַ����          @0x0 - 0xFFFFFFFF
  sCommand.AlternateBytes        �����ֽ�����      @0x0 - 0xFFFFFFFF
  sCommand.AddressSize           ��ַλ��          @8/16/24/32
  sCommand.AlternateBytesSize    �����ֽ�λ��      @8/16/24/32
  sCommand.DummyCycles           ������λ��        @0 - 31
  sCommand.InstructionMode       ָ���ģʽ      @��/1/2/4��
  sCommand.AddressMode           ��ַ����ģʽ      @��/1/2/4��
  sCommand.AlternateByteMode     �����ֽڷ���ģʽ  @��/1/2/4��
  sCommand.DataMode              ���ݷ���ģʽ      @��/1/2/4��
  sCommand.NbData                ���ݷ��ͳ���      @0x0 - 0xFFFFFFFF
  sCommand.DdrMode               DDRģʽ           @ʹ��/ʧ��
  sCommand.DdrHoldHalfCycle      �ӳ��������      @ʹ��/ʧ��
  sCommand.SIOOMode              ָ�������һ��    @ʹ��/ʧ��
  */
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    /* ָ���ģʽ    :1�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_NONE;             /* ���ݷ���ģʽ    :�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q32JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = W25X_EnterQPIMode;  /* ����QPIģʽָ�� */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000000;
  if (HAL_QSPI_Command(&QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler(__FILE__, __LINE__);
  }
  
  /* �������� */
  sCommand.InstructionMode   = QSPI_INSTRUCTION_4_LINES;   /* ָ���ģʽ    :4�߷�ʽ����ָ�� */
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;          /* ��ַ����ģʽ    :�����ַ */
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  /* �����ֽڷ���ģʽ:�޽����ֽ� */
  sCommand.DummyCycles       = 0;                          /* ������λ��      :0  */
  sCommand.DataMode          = QSPI_DATA_4_LINES;          /* ���ݷ���ģʽ    :4�߷�ʽ�������� */
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;      /* DDRģʽ         :W25Q32JV��֧��DDR */
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  /* �ӳ��������    :DDRģʽ����������ӳ� */
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;   /* ָ�������һ��  :ÿ�δ��䶼��ָ�� */
  
  /* �������� */
  sCommand.Instruction       = W25X_SetReadParameters;  /* ����QPIģʽ����ָ�� */
  sCommand.Address           = 0x00000000;
  sCommand.AddressSize       = QSPI_ADDRESS_32_BITS;
  sCommand.AlternateBytes    = 0x00000000;
  sCommand.AlternateBytesSize= QSPI_ALTERNATE_BYTES_32_BITS;
  sCommand.NbData            = 0x00000001;
  dat                        = 0x02 << 4; //����P5&P4=10,6��dummy clocks,104MHz
  QSPI_WriteEnable(&QSPIHandle);//������п���
  
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

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
