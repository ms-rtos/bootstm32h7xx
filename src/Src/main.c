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
** 文   件   名: main.c
**
** 创   建   人: Jiao.jinxing
**
** 文件创建日期: 2020 年 04 月 07 日
**
** 描        述: MS-BOOT 主函数
*********************************************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ms_boot_fatfs.h"
#include "ms_boot_littlefs.h"
#include "ms_littlefs_drv.h"
#include "ms_flash_drv.h"
#include "segger/SEGGER_RTT.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static ms_boot_extfs_if_t *extfs_if;
static ms_uint64_t heap_memory[MS_CFG_BOOT_HEAP_SIZE / sizeof(ms_uint64_t)];

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Initialize function.
 *
 * @return N/A
 */
void _init(void)
{
}

/**
 * @brief Kernel info print.
 *
 * @param[in] buf               Pointer to content which need to be print
 * @param[in] len               The length of buffer
 *
 * @return N/A
 */
void ms_bsp_printk(const char *buf, ms_size_t len)
{
#if MS_CFG_BOOT_TRACE_EN > 0
    ms_trace_write(MS_NULL, buf, len);
#endif
}

/**
 * @brief Boot MS-RTOS.
 *
 * @param[in] addr              Pointer to MS-RTOS image base address
 *
 * @return N/A
 */
static void ms_boot_os(ms_addr_t *addr)
{
    register ms_func_t func = (ms_func_t)addr[1U];

#if MS_CFG_BOOT_OS_RUN_IN != MS_BOOT_FLASH
    __disable_irq();
    SCB_CleanDCache();
    SCB_InvalidateICache();
    __enable_irq();
#endif

    ms_printk(MS_PK_NOTICE, "Start MS-RTOS...\n");

    /*
     * Disable system tick
     */
    SysTick->CTRL = 0U;

    __asm__ __volatile__ (
            "MOV  R13, %[sp]\n"
            :
            : [sp] "r" (addr[0])
            : "cc");

    func();
}

#if MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_SDRAM
/*
 * Debug register
 */
#define __ARMV7M_DHCSR                      0xE000EDF0U
#define __ARMV7M_DCRSR                      0xE000EDF4U
#define __ARMV7M_DCRDR                      0xE000EDF8U
#define __ARMV7M_DEMCR                      0xE000EDFCU

/*
 * DWT
 */
#define __ARMV7M_DWT_CR                     0xE0001000U
#define __ARMV7M_DWT_CYCCNT                 0xE0001004U
#define __ARMV7M_DWT_LAR                    0xE0001FB0U
#define __ARMV7M_DWT_LSR                    0xE0001FB4U

/*
 * Debug Fault Status Register (DFSR)
 */
/* Bits 31:5 - Reserved */
#define __ARMV7M_DFSR_RESETALL              0x1FU
#define __ARMV7M_DFSR_EXTERNAL              (1U << 4U)
#define __ARMV7M_DFSR_VCATCH                (1U << 3U)
#define __ARMV7M_DFSR_DWTTRAP               (1U << 2U)
#define __ARMV7M_DFSR_BKPT                  (1U << 1U)
#define __ARMV7M_DFSR_HALTED                (1U << 0U)

/*
 * Debug Halting Control and Status Register (DHCSR)
 */
/* This key must be written to bits 31:16 for write to take effect */
#define __ARMV7M_DHCSR_DBGKEY               0xA05F0000U
/* Bits 31:26 - Reserved */
#define __ARMV7M_DHCSR_S_RESET_ST           (1U << 25U)
#define __ARMV7M_DHCSR_S_RETIRE_ST          (1U << 24U)
/* Bits 23:20 - Reserved */
#define __ARMV7M_DHCSR_S_LOCKUP             (1U << 19U)
#define __ARMV7M_DHCSR_S_SLEEP              (1U << 18U)
#define __ARMV7M_DHCSR_S_HALT               (1U << 17U)
#define __ARMV7M_DHCSR_S_REGRDY             (1U << 16U)
/* Bits 15:6 - Reserved */
#define __ARMV7M_DHCSR_C_SNAPSTALL          (1U << 5U)    /* v7m only */
/* Bit 4 - Reserved */
#define __ARMV7M_DHCSR_C_MASKINTS           (1U << 3U)
#define __ARMV7M_DHCSR_C_STEP               (1U << 2U)
#define __ARMV7M_DHCSR_C_HALT               (1U << 1U)
#define __ARMV7M_DHCSR_C_DEBUGEN            (1U << 0U)

/*
 * Debug Core Register Selector Register (DCRSR)
 */
#define __ARMV7M_DCRSR_REGWnR               0x00010000U
#define __ARMV7M_DCRSR_REGSEL_MASK          0x0000001FU
#define __ARMV7M_DCRSR_REGSEL_XPSR          0x00000010U
#define __ARMV7M_DCRSR_REGSEL_MSP           0x00000011U
#define __ARMV7M_DCRSR_REGSEL_PSP           0x00000012U

/*
 * Debug Exception and Monitor Control Register (DEMCR)
 */
/* Bits 31:25 - Reserved */
#define __ARMV7M_DEMCR_TRCENA               (1U << 24U)
/* Bits 23:20 - Reserved */
#define __ARMV7M_DEMCR_MON_REQ              (1U << 19U)   /* v7m only */
#define __ARMV7M_DEMCR_MON_STEP             (1U << 18U)   /* v7m only */
#define __ARMV7M_DEMCR_VC_MON_PEND          (1U << 17U)   /* v7m only */
#define __ARMV7M_DEMCR_VC_MON_EN            (1U << 16U)   /* v7m only */
/* Bits 15:11 - Reserved */
#define __ARMV7M_DEMCR_VC_HARDERR           (1U << 10U)
#define __ARMV7M_DEMCR_VC_INTERR            (1U << 9U)    /* v7m only */
#define __ARMV7M_DEMCR_VC_BUSERR            (1U << 8U)    /* v7m only */
#define __ARMV7M_DEMCR_VC_STATERR           (1U << 7U)    /* v7m only */
#define __ARMV7M_DEMCR_VC_CHKERR            (1U << 6U)    /* v7m only */
#define __ARMV7M_DEMCR_VC_NOCPERR           (1U << 5U)    /* v7m only */
#define __ARMV7M_DEMCR_VC_MMERR             (1U << 4U)    /* v7m only */
/* Bits 3:1 - Reserved */
#define __ARMV7M_DEMCR_VC_CORERESET         (1U << 0U)

static ms_bool_t ms_armv7m_debug_mode(void)
{
    return (ms_read32(__ARMV7M_DHCSR) & __ARMV7M_DHCSR_C_DEBUGEN) ? MS_TRUE : MS_FALSE;
}

#endif

#if MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_FLASH

/**
 * @brief Update system firmware.
 *
 * @return N/A
 */
static ms_err_t __ms_update_os(const char *path)
{
    ms_err_t err;
    ms_ptr_t handle;

    err = extfs_if->open(path, &handle);
    if (err == MS_ERR_NONE) {
        ms_boot_extfs_stat_t fstat;

        err = extfs_if->stat(path, &fstat);
        if (err == MS_ERR_NONE) {
            if ((fstat.size > 0) && (fstat.size <= MS_CFG_BOOT_OS_SIZE)) {
                ms_ptr_t *buffer = ms_kmalloc(MS_CFG_BOOT_READ_BUF_SIZE);

                if (buffer != MS_NULL) {
                    err = ms_flash_unlock();
                    if (err == MS_ERR_NONE) {
                        ms_bool_t program_ok = MS_FALSE;

                        while (!program_ok) {
                            ms_printk(MS_PK_NOTICE, "Erase MS-RTOS partition...");
                            err = ms_flash_erase_os();
                            if (err == MS_ERR_NONE) {
                                ms_uint32_t remain_len = fstat.size;
                                ms_uint32_t read_len;
                                ms_uint32_t len;
                                ms_addr_t   address = MS_CFG_BOOT_OS_BASE;

                                ms_printk(MS_PK_NOTICE, "Success!\n");

                                ms_printk(MS_PK_NOTICE, "Program MS-RTOS image...");

                                while (remain_len > 0) {
                                    read_len = MS_MIN(remain_len, MS_CFG_BOOT_READ_BUF_SIZE);

                                    err = extfs_if->read(handle, buffer, read_len, &len);
                                    if ((err != MS_ERR_NONE) || (len == 0)) {
                                        ms_printk(MS_PK_ERR, "\nFailed to read MS-RTOS image file %s!\n", path);
                                        break;
                                    } else {
                                        err = ms_flash_program(address, buffer, len);
                                        if (err != MS_ERR_NONE) {
                                            ms_printk(MS_PK_ERR, "\nFailed to program MS-RTOS image!\n");
                                            break;
                                        }

                                        address    += len;
                                        remain_len -= len;

                                        ms_printk(MS_PK_NOTICE, ".");
                                    }
                                }

                                if (remain_len == 0) {
                                    program_ok = MS_TRUE;
                                    ms_printk(MS_PK_NOTICE, "Success!\n");
                                } else {
                                    ms_printk(MS_PK_ERR, "Failed!\n");
                                }
                            } else {
                                ms_printk(MS_PK_ERR, "Failed!\n");
                            }
                        }

                        (void)ms_flash_lock();

                    } else {
                        ms_printk(MS_PK_ERR, "Failed to unlock flash!\n");
                    }

                    ms_kfree(buffer);

                } else {
                    ms_printk(MS_PK_ERR, "Failed to allocate buffer, size=%d!\n", MS_CFG_BOOT_READ_BUF_SIZE);
                    err = MS_ERR_KERN_HEAP_NO_MEM;
                }
            } else {
                ms_printk(MS_PK_ERR, "MS-RTOS image file %s length error, length=%d!\n", path, fstat.size);
                err = MS_ERR;
            }
        } else {
            ms_printk(MS_PK_ERR, "Failed to get MS-RTOS image file %s status!\n", path);
        }

        extfs_if->close(handle);

    } else {
        ms_printk(MS_PK_ERR, "Failed to open MS-RTOS image file %s!\n", path);
    }

    return err;
}

/**
 * @brief Update system firmware.
 *
 * @return N/A
 */
static ms_err_t ms_update_os(void)
{
    ms_err_t err;
    ms_ptr_t handle;

    err = extfs_if->open(MS_CFG_BOOT_UPDATE_REQ_FILE, &handle);
    if (err == MS_ERR_NONE) {
        char path[MS_IO_PATH_BUF_SIZE];
        ms_uint32_t len;

        len = 0;
        err = extfs_if->read(handle, path, sizeof(path), &len);

        (void)extfs_if->close(handle);

        if (err == MS_ERR_NONE) {
            if (len > 0) {
                err = extfs_if->open(path, &handle);
                if (err == MS_ERR_NONE) {
                    ms_boot_extfs_stat_t fstat;

                    err = extfs_if->stat(path, &fstat);
                    if (err == MS_ERR_NONE) {
                        if ((fstat.size > 0) && (fstat.size % sizeof(ms_flashfs_action_t) == 0)) {
                            ms_flashfs_action_t *actions = (ms_flashfs_action_t *)ms_kmalloc(fstat.size);

                            if (actions != MS_NULL) {
                                err = extfs_if->read(handle, (ms_ptr_t)actions, fstat.size, &len);
                                if ((err == MS_ERR_NONE) && (len == fstat.size)) {
                                    ms_flashfs_action_t *action = actions;
                                    ms_uint32_t n_action = fstat.size / sizeof(ms_flashfs_action_t);
                                    ms_uint32_t i;

                                    for (i = 0; i < n_action; i++) {
                                        if (action->id == MS_FLASHFS_ACTION_UPDATE_OS) {
                                            err = __ms_update_os(action->source);

                                            if (err == MS_ERR_NONE) {
                                                ms_printk(MS_PK_NOTICE, "Update success!\n");
                                            } else {
                                                ms_printk(MS_PK_ERR, "Update failure!\n");
                                            }
                                            break;
                                        }
                                        action++;
                                    }
                                } else {
                                    ms_printk(MS_PK_ERR, "Failed to read update actions file %s!\n", path);
                                    err = MS_ERR;
                                }

                                ms_kfree(actions);
                            } else {
                                ms_printk(MS_PK_ERR, "Failed to allocate buffer, size=%d!\n", fstat.size);
                                err = MS_ERR_KERN_HEAP_NO_MEM;
                            }
                        } else {
                            ms_printk(MS_PK_ERR, "Update actions file %s length error, length=%d!\n", path, fstat.size);
                            err = MS_ERR;
                        }
                    } else {
                        ms_printk(MS_PK_ERR, "Failed to get update actions file %s status!\n", path);
                    }
                    extfs_if->close(handle);

                } else {
                    ms_printk(MS_PK_ERR, "Failed to open update actions file %s!\n", path);
                }
            } else {
                ms_printk(MS_PK_ERR, "Update request file %s is empty!\n", MS_CFG_BOOT_UPDATE_REQ_FILE);
                err = MS_ERR;
            }
        } else {
            ms_printk(MS_PK_ERR, "Failed to read update request file %s!\n", MS_CFG_BOOT_UPDATE_REQ_FILE);
        }
    } else {
        ms_printk(MS_PK_NOTICE, "Update request file %s no exist!\n", MS_CFG_BOOT_UPDATE_REQ_FILE);
    }

    return err;
}

#elif MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_SDRAM

/**
 * @brief Load MS-RTOS image.
 *
 * @return N/A
 */
static ms_err_t ms_load_os(void)
{
    ms_ptr_t handle;
    ms_err_t err;

    err = extfs_if->open(MS_CFG_BOOT_OS_IMAGE_FILE, &handle);
    if (err == MS_ERR_NONE) {
        ms_boot_extfs_stat_t fstat;

        err = extfs_if->stat(MS_CFG_BOOT_OS_IMAGE_FILE, &fstat);
        if (err == MS_ERR_NONE) {
            ms_uint32_t len;

            ms_printk(MS_PK_NOTICE, "Loading MS-RTOS image file %s...", MS_CFG_BOOT_OS_IMAGE_FILE);

            err = extfs_if->read(handle, (ms_ptr_t)MS_CFG_BOOT_OS_BASE, fstat.size, &len);
            if ((err == MS_ERR_NONE) && (len == fstat.size)) {
                ms_printk(MS_PK_NOTICE, "Success!\n");
            } else {
                ms_printk(MS_PK_ERR, "Failed!\n");
            }
        } else {
            ms_printk(MS_PK_ERR, "Failed to get MS-RTOS image file %s status!\n", MS_CFG_BOOT_OS_IMAGE_FILE);
        }

        extfs_if->close(handle);
    } else {
        ms_printk(MS_PK_NOTICE, "MS-RTOS image file %s no exist!\n", MS_CFG_BOOT_OS_IMAGE_FILE);
    }

    return err;
}

/**
 * @brief Check MS-RTOS image valid?
 *
 * @param[in] addr Pointer to MS-RTOS image base address
 *
 * @return MS_TRUE if valid, MS_FALSE if invalid
 */
static ms_bool_t ms_os_valid(ms_addr_t *addr)
{
#if 0 /* vector table */
            .long   __ms_boot_stack_start__                         /* Top of Stack                             */
            .long   Reset_Handler                                   /* Reset Handler                            */
            .long   NMI_Handler                                     /* NMI Handler                              */
            .long   HardFault_Handler                               /* Hard Fault Handler                       */
            .long   MemManage_Handler                               /* MPU Fault Handler                        */
            .long   BusFault_Handler                                /* Bus Fault Handler                        */
            .long   UsageFault_Handler                              /* Usage Fault Handler                      */
            .long   0                                               /* Reserved                                 */
            .long   0                                               /* Reserved                                 */
            .long   0                                               /* Reserved                                 */
            .long   0                                               /* Reserved                                 */
            .long   SVC_Handler                                     /* SVCall Handler                           */
            .long   DebugMon_Handler                                /* Reserved                                 */
            .long   0                                               /* Reserved                                 */
            .long   PendSV_Handler                                  /* PendSV Handler                           */
            .long   SysTick_Handler                                 /* SysTick Handler                          */
#endif
    if ((addr[1U]   > (ms_addr_t)addr) &&
        (addr[2U]   > (ms_addr_t)addr) &&
        (addr[3U]   > (ms_addr_t)addr) &&
        (addr[4U]   > (ms_addr_t)addr) &&
        (addr[5U]   > (ms_addr_t)addr) &&
        (addr[6U]   > (ms_addr_t)addr) &&
        (addr[7U]  == 0U) &&
        (addr[8U]  == 0U) &&
        (addr[9U]  == 0U) &&
        (addr[10U] == 0U) &&
        (addr[11U]  > (ms_addr_t)addr) &&
        (addr[12U]  > (ms_addr_t)addr) &&
        (addr[13U] == 0U) &&
        (addr[14U]  > (ms_addr_t)addr) &&
        (addr[15U]  > (ms_addr_t)addr)) {
        return MS_TRUE;
    } else {
        return MS_FALSE;
    }
}

/**
 * @brief Configure MPU.
 *
 * @return N/A
 */
static void MPU_Config (void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    /* Configure the MPU attributes for SDRAM */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = MS_CFG_BOOT_OS_BASE;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

#elif MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_QSPI_FLASH

#endif

#if MS_CFG_BOOT_SHELL_EN > 0
/**
 * @brief boot command.
 *
 * @param[in] argc              Arguments count
 * @param[in] argv              Arguments array
 * @param[in] io                Pointer to shell io driver
 *
 * @return N/A
 */
static void __ms_shell_boot(int argc, char *argv[], const ms_shell_io_t *io)
{
    ms_addr_t *addr;

    if (argc > 1) {
        addr = (ms_addr_t *)ms_strtoul(argv[1U], MS_NULL, 16U);

    } else {
        addr = (ms_addr_t *)MS_CFG_BOOT_OS_BASE;
    }

    ms_boot_os(addr);
}

MS_SHELL_CMD(boot,   __ms_shell_boot,   "Boot MS-RTOS, boot [address]", __ms_shell_cmd_boot);

#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    ms_err_t err;

    /* Enable the CPU Cache */
    CPU_CACHE_Enable();
  
    /* STM32H7xx HAL library initialization:
     - Systick timer is configured by default as source of time base, but user 
       can eventually implement his proper time base source (a general purpose 
       timer for example or other time source), keeping in mind that Time base 
       duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
       handled in milliseconds basis.
     - Set NVIC Group Priority to 4
     - Low Level Initialization
     */
    HAL_Init();

    /* Configure the system clock to 400 MHz */
    SystemClock_Config();

    /* Initial SDRAM */
#if MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_SDRAM
    BSP_SDRAM_Init();
    MPU_Config();
#endif

    /*
     * Initial kernel heap
     */
    err = ms_kheap_unit_init(heap_memory, sizeof(heap_memory));
    if (err != MS_ERR_NONE) {
        ms_printk(MS_PK_NOTICE, "ms_kheap_unit_init err=%d\n", err);
    }

#if MS_CFG_BOOT_EXT_FS_TYPE == MS_BOOT_FATFS
    static FATFS fatfs;     /* File system object for SD card logical drive */
    static char sd_path[4]; /* SD card logical drive path */
    FRESULT res;

    /*
     * Install SD driver
     */
    FATFS_LinkDriver(&SD_Driver, sd_path);

    /*
     * Mount fatfs
     */
    res = f_mount(&fatfs, (TCHAR const *)sd_path, 0);
    if (res != FR_OK) {
        ms_printk(MS_PK_NOTICE, "f_mount res=%d\n", res);
    }

    extfs_if = &ms_fatfs_if;

#elif MS_CFG_BOOT_EXT_FS_TYPE == MS_BOOT_LITTLEFS
    static lfs_t littlefs;

    /*
     * Mount littlefs
     */
    err = ms_littlefs_init(&littlefs);
    if (err != MS_ERR_NONE) {
        ms_printk(MS_PK_NOTICE, "ms_littlefs_init err=%d\n", err);
    }

    extfs_if = &ms_littlefs_if;
#endif

#if MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_FLASH
    /*
     * Try update MS-RTOS image
     */
    ms_update_os();

#elif MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_SDRAM
    if (ms_armv7m_debug_mode()) {
        ms_printk(MS_PK_NOTICE, "Debug mode! wait host download...\n");

        /*
         * Debug mode, maybe debug BSP, delay one second for host download MS-RTOS image
         */
        bzero((ms_addr_t *)MS_CFG_BOOT_OS_BASE, 64U);

        HAL_Delay(4000);

        if (!ms_os_valid((ms_addr_t *)MS_CFG_BOOT_OS_BASE)) {
            /*
             * Havn't a valid MS-RTOS image, maybe debug application, load MS-RTOS image
             */
            ms_load_os();
        }

        /*
         * Boot MS-RTOS
         */
        if (ms_os_valid((ms_addr_t *)MS_CFG_BOOT_OS_BASE)) {
            ms_boot_os((ms_addr_t *)MS_CFG_BOOT_OS_BASE);
        }

    } else {
        /*
         * Try load MS-RTOS image
         */
        ms_load_os();
    }

#elif MS_CFG_BOOT_OS_RUN_IN == MS_BOOT_QSPI_FLASH
    bsp_InitQSPI_W25Q32();

    QSPI_EnableMemoryMappedMode(&QSPIHandle);
#endif

    /*
     * Enter shell
     */
#if MS_CFG_BOOT_SHELL_EN > 0
    ms_shell_io_t bsp_shell_io = {
            ms_trace_getc,
            ms_trace_putc,
            ms_trace_write,
            ms_trace_printf,
    };

    while (MS_TRUE) {
        ms_shell_enter(&bsp_shell_io);
    }
#else
    while (MS_TRUE) {
        ms_boot_os((ms_addr_t *)MS_CFG_BOOT_OS_BASE);
    }
#endif
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000 (CPU Clock)
  *            HCLK(Hz)                       = 200000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
   HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY); /* PWR set to LDO for the STM32H750B-DISCO board */

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ms_printk(MS_PK_ERR, "Failed at %s, %d\n", __FUNCTION__, __LINE__); }
  }

  /* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                 RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if(ret != HAL_OK)
  {
    while(1) { ms_printk(MS_PK_ERR, "Failed at %s, %d\n", __FUNCTION__, __LINE__); }
  }

  /* Initializes the LTDC, USART3 and I2C3 periph clocks */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_I2C3;
  PeriphClkInitStruct.PLL3.PLL3M = 5;
  PeriphClkInitStruct.PLL3.PLL3N = 160;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 88;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_2;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    while(1) { ms_printk(MS_PK_ERR, "Failed at %s, %d\n", __FUNCTION__, __LINE__); }
  }
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*********************************************************************************************************
  END
*********************************************************************************************************/
