#*********************************************************************************************************
#
#                                 北京翼辉信息技术有限公司
#
#                                   微型安全实时操作系统
#
#                                       MS-RTOS(TM)
#
#                               Copyright All Rights Reserved
#
#--------------文件信息--------------------------------------------------------------------------------
#
# 文   件   名: bootstm32h7xx.mk
#
# 创   建   人: IoT Studio
#
# 文件创建日期: 2020 年 05 月 22 日
#
# 描        述: 本文件由 IoT Studio 生成，用于配置 Makefile 功能，请勿手动修改
#*********************************************************************************************************

#*********************************************************************************************************
# Clear setting
#*********************************************************************************************************
include $(CLEAR_VARS_MK)

#*********************************************************************************************************
# Target
#*********************************************************************************************************
LOCAL_TARGET_NAME := bootstm32h7xx.elf

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS :=  \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cec.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_comp.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dcmi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dts.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gfxmmu.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hrtim.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_irda.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_lptim.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdios.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nand.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nor.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ospi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_otfdec.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pssi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ramecc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spdifrx.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sram.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_swpmi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart_ex.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_wwdg.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_adc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_bdma.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_comp.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_crc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_crs.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dac.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_delayblock.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma2d.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_exti.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_hrtim.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_i2c.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_lptim.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_lpuart.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_mdma.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_opamp.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_pwr.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rcc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rng.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rtc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_spi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_swpmi.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_tim.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usart.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c \
src/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_utils.c \
src/Src/fatfs/src/ff.c \
src/Src/fatfs/src/ff_gen_drv.c \
src/Src/fatfs/src/diskio.c \
src/Src/fatfs/src/option/unicode.c \
src/Src/littlefs/lfs.c \
src/Src/littlefs/lfs_util.c \
src/Src/ms_littlefs_drv.c \
src/Src/ms_flash_drv.c \
src/Src/ms_boot_fatfs.c \
src/Src/ms_boot_littlefs.c \
src/Src/main.c \
src/Src/sd_diskio.c \
src/Src/ms_mini_libc.c \
src/Src/stm32h7xx_hal_msp.c \
src/Src/stm32h7xx_it.c \
src/Src/iot_pi_pro_sd.c \
src/Src/qspi_w25qxx/bsp_qspi_w25qxx.c \
src/Drivers/CMSIS/Device/ST/STM32H7xx/Source/Templates/system_stm32h7xx.c \
src/Src/startup_stm32h750xx.S

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your header files search path")
#*********************************************************************************************************
LOCAL_INC_PATH := \
-I"./src/Drivers/CMSIS/Device/ST/STM32H7xx/Include" \
-I"./src/Drivers/CMSIS/Include" \
-I"./src/Drivers/STM32H7xx_HAL_Driver/Inc" \
-I"./src/Drivers/BSP/Components" \
-I"./src/Drivers/BSP/Components/Common" \
-I"./src/Drivers" \
-I"./src/Src" \
-I"./src/Src/fatfs/src" \
-I"./src/Inc" \
-I"./src" \
-I"./."

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL := \
-D__MS_KERNEL_SPACE \
-DUSE_HAL_DRIVER \
-DSTM32H750xx \
-DARM_MATH_CM7 \
-D__FPU_PRESENT=1 \
-DUSE_USB_HS=1 \
-DDATA_IN_D2_SRAM=1

#*********************************************************************************************************
# Compiler flags
#*********************************************************************************************************
LOCAL_CFLAGS   := 
LOCAL_CXXFLAGS := 

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB      := 
LOCAL_DEPEND_LIB_PATH := 

#*********************************************************************************************************
# Link script file
#*********************************************************************************************************
LOCAL_LD_SCRIPT := STM32H750_FLASH.ld

#*********************************************************************************************************
# C++ config
#*********************************************************************************************************
LOCAL_USE_CXX        := no
LOCAL_USE_CXX_EXCEPT := no

#*********************************************************************************************************
# Code coverage config
#*********************************************************************************************************
LOCAL_USE_GCOV := no

#*********************************************************************************************************
# User link command
#*********************************************************************************************************
LOCAL_PRE_LINK_CMD   := 
LOCAL_POST_LINK_CMD  := 
LOCAL_PRE_STRIP_CMD  := 
LOCAL_POST_STRIP_CMD := 

include $(BOOTLOADER_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
