/**
******************************************************************************
* @file    clock_weather_app.c
* @author  MCD Application Team
* @brief   This file provides the clock and weather APIs
******************************************************************************
* @attention
*
* Copyright (c) 2019 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "clock_weather_app.h"

#include "stm32h7xx_hal.h"
  
#include "stm32h7b3i_eval.h"
#include "stm32h7b3i_eval_sdram.h"
#include "stm32h7b3i_eval_ospi.h"

#include "fuzz.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern int32_t es_wifi_driver(net_if_handle_t * pnetif);

net_if_handle_t netif;
net_event_handler_t net_handler;
net_if_driver_init_func es_wifi_driver_ptr;

/* Private function prototypes -----------------------------------------------*/
int RNG_Init(void);
LTDC_HandleTypeDef hltdc;
DMA2D_HandleTypeDef hdma2d;
RNG_HandleTypeDef RngHandle;


/* Private functions ---------------------------------------------------------*/
// int32_t es_wifi_driver(net_if_handle_t *pnetif)
// {
//   return es_wifi_if_init(pnetif);
// }


void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;
  
  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);
  
  /* The voltage scaling allows optimizing the power consumption when the device is
  clocked below the maximum system frequency, to update the voltage scaling value
  regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
  
  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 280;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if (ret != HAL_OK)
  {
    while (1)
    {
    }
  }
  
  /* RK070ER9427 LCD clock configuration */
  /* LCD clock configuration */
  /* PLL3_VCO Input = HSE_VALUE/PLL3M = 1 Mhz */
  /* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 288 Mhz */
  /* PLLLCDCLK = PLL3_VCO Output/PLL3R = 288/9 = 32 Mhz */
  /* PLLUSBCLK = PLL3_VCO Output/PLL3Q = 288/6 = 48 Mhz */
  /* LTDC clock frequency = 24 Mhz */
  /* USB clock frequency = 48 Mhz */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLL3.PLL3M = 24;
  PeriphClkInitStruct.PLL3.PLL3N = 288;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 6;
  PeriphClkInitStruct.PLL3.PLL3R = 9;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    while (1)
    {
    }
  }
  
  /* PLL3 for USB Clock */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    while (1)
    {
    }
  }
  
  /* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
    RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);
  
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
  if (ret != HAL_OK)
  {
    while (1)
    {
    }
  }
  
  /*
  Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
  (GPIO, SPI, FMC, QSPI ...)  when  operating at  high frequencies(please refer to product datasheet)
  The I/O Compensation Cell activation  procedure requires :
  - The activation of the CSI clock
  - The activation of the SYSCFG clock
  - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
  */
  __HAL_RCC_CSI_ENABLE() ;
  
  __HAL_RCC_SYSCFG_CLK_ENABLE() ;
  
  HAL_EnableCompensationCell();
}

/**
  * @brief  Configure the MPU attributes as Write Through for Internal D1SRAM.
  * @note   The Base Address is 0x24000000 since this memory interface is the AXI.
  *         The Configured Region Size is 512KB because same as D1SRAM size.
  * @param  None
  * @retval None
  */
  void MPU_Config(void)
  {
    MPU_Region_InitTypeDef MPU_InitStruct;
    
    /* Disable the MPU */
    HAL_MPU_Disable();

    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x00;
    MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x87;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Configure the MPU attributes for Quad-SPI area to strongly ordered
    This setting is essentially needed to avoid MCU blockings!
    See also STM Application Note AN4861 */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = OCTOSPI1_BASE;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_256MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    
    /* Configure the MPU attributes for the QSPI 64MB to Cacheable WT */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = OCTOSPI1_BASE;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_64MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    
    /* Setup SDRAM in SO */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = SDRAM_DEVICE_ADDR;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_256MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    
    /* Setup SDRAM in Write-through (Buffers) */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = SDRAM_DEVICE_ADDR;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_8MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER4;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    
    /* Setup AXI SRAM, SRAM1 and SRAM2 in Write-through */
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = D1_AXISRAM_BASE;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_1MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER5;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  }

/**
  * @brief  Initializes module wifi network interface
  * @param  none
  * @retval none
  */
void WIFI_Init(void)
{
  es_wifi_driver_ptr = &es_wifi_driver;
  NET_Init();  
}

/**
  * @brief  Network interface initialization
  * @param  none
  * @retval 0 in case of success, an error code otherwise
  */
int NET_Init(void)
{
  if (net_if_init(&netif, es_wifi_driver_ptr, &net_handler) != NET_OK)
  {
    return -1;
  }
  
  net_if_wait_state(&netif,NET_STATE_INITIALIZED,STATE_TRANSITION_TIMEOUT);
  if ( net_if_start (&netif) != NET_OK )
  {
    return -1;
  }
  
  net_if_wait_state(&netif,NET_STATE_READY,STATE_TRANSITION_TIMEOUT);
  
  return 0;
}

/**
  * @brief  Get available access points
  * @param  APs: pointer Access points structure
  * @retval ES Wifi status
  */
int32_t WIFI_Get_Access_Points(net_wifi_scan_results_t *APs)
{
  int32_t ret;
  ret = net_wifi_scan(&netif,NET_WIFI_SCAN_PASSIVE,NULL);
  if (ret == NET_OK)
  {
    ret = net_wifi_get_scan_results(&netif,APs,MAX_LISTED_AP);
    if (ret > 0) ret= NET_OK;
  }
  return ret;
}

/**

  * @brief  Connect network interface
  * @param  ssid: wifi network name
  * @param  password:  access point password
  * @param  encryption : security mode
  * @retval 0 in case of success, an error code otherwise
  */
int8_t WIFI_Connect(char *ssid, uint8_t *password, int32_t encryption )
{  
  net_wifi_credentials_t  Credentials = 
  {
    (char const*) ssid,
    (char const*)password,
    encryption
  };
  
  if (net_wifi_set_credentials(&netif, &Credentials) != NET_OK)
  {
    return -1;
  }
  
  if (netif.state == NET_STATE_CONNECTING )
  {
    netif.state = NET_STATE_CONNECTED ; 
  }
    
  if (netif.state == NET_STATE_CONNECTED ) 
  {
    if(net_if_disconnect(&netif)!= NET_OK)
    {
      return  -1;
    }
  }
  
  if(net_if_connect (&netif) != NET_OK)
  {
    return -1;
  }
  
  
  return NET_OK;
}

int RNG_Init(void)
{
  /* Configure the RNG peripheral */
  RngHandle.Instance = RNG;
  
  /* DeInitialize the RNG peripheral */
  if (HAL_RNG_DeInit(&RngHandle) != HAL_OK)
  {
    /* DeInitialization Error */
    return 1;
  }    
  
  /* Initialize the RNG peripheral */
  if (HAL_RNG_Init(&RngHandle) != HAL_OK)
  {
    /* Initialization Error */
    return 1;
  }
  return 0;
}

/**
* @brief RNG MSP Initialization
*        This function configures the hardware resources used in this example:
*           - Peripheral's clock enable
* @param hrng: RNG handle pointer
* @retval None
*/
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  
  /*Select PLL output as RNG clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
  PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_PLL;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct))
  {
    /* Initialization Error */
    while(1);
  }
  
  /* RNG Peripheral clock enable */
  __HAL_RCC_RNG_CLK_ENABLE();
  
}

/**
* @brief RNG MSP De-Initialization
*        This function freeze the hardware resources used in this example:
*          - Disable the Peripheral's clock
* @param hrng: RNG handle pointer
* @retval None
*/
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
  /* Enable RNG reset state */
  __HAL_RCC_RNG_FORCE_RESET();
  
  /* Release RNG from reset state */
  __HAL_RCC_RNG_RELEASE_RESET();
}

void hw_init()
{
  SCB_EnableICache();
  SCB_EnableDCache();
  
  MPU_Config();
  
  HAL_Init();
  
  SystemClock_Config();
  
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);
  
  __HAL_RCC_CRC_CLK_ENABLE();
      
  /* For error reporting */
  BSP_LED_Init(LED_RED);
  BSP_LED_Off(LED_RED);
      
  /* Disable FMC Bank1 to avoid speculative/cache accesses */
  FMC_Bank1_R->BTCR[0] &= ~FMC_BCRx_MBKEN;
  
  /* Initialize WIFI*/
  WIFI_Init();
  
  /* Initialize RNG */
  RNG_Init();  
  
}

__attribute__((annotate("no_instrument")))
static void feed_testcase(void)
{
  unsigned char temp[] = {0x0d, 0x0a, 0x49, 0x53, 0x4d, 0x34, 0x33, 0x33, 0x36, 0x32, 0x2d, 0x4d, 0x33, 0x47, 0x2d, 0x4c, 0x34, 0x34, 0x2d, 0x53, 0x50, 0x49, 0x2c, 0x43, 0x33, 0x2e, 0x35, 0x2e, 0x32, 0x2e, 0x35, 0x2e, 0x53, 0x54, 0x4d, 0x2c, 0x76, 0x33, 0x2e, 0x35, 0x2e, 0x32, 0x2c, 0x76, 0x31, 0x2e, 0x34, 0x2e, 0x30, 0x2e, 0x72, 0x63, 0x31, 0x2c, 0x76, 0x38, 0x2e, 0x32, 0x2e, 0x31, 0x2c, 0x31, 0x32, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2c, 0x49, 0x6e, 0x76, 0x65, 0x6e, 0x74, 0x65, 0x6b, 0x20, 0x65, 0x53, 0x2d, 0x57, 0x69, 0x46, 0x69, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x15, 0x0d, 0x0a, 0x43, 0x34, 0x3a, 0x37, 0x46, 0x3a, 0x35, 0x31, 0x3a, 0x39, 0x34, 0x3a, 0x44, 0x34, 0x3a, 0x45, 0x32, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x15, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x5b, 0x4a, 0x4f, 0x49, 0x4e, 0x20, 0x20, 0x20, 0x5d, 0x20, 0x4c, 0x49, 0x5f, 0x48, 0x33, 0x43, 0x2c, 0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38, 0x2e, 0x31, 0x2e, 0x33, 0x2c, 0x30, 0x2c, 0x30, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x4c, 0x49, 0x5f, 0x48, 0x33, 0x43, 0x2c, 0x31, 0x71, 0x61, 0x7a, 0x32, 0x77, 0x73, 0x78, 0x2c, 0x33, 0x2c, 0x31, 0x2c, 0x30, 0x2c, 0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38, 0x2e, 0x31, 0x2e, 0x33, 0x2c, 0x32, 0x35, 0x35, 0x2e, 0x32, 0x35, 0x35, 0x2e, 0x32, 0x35, 0x35, 0x2e, 0x30, 0x2c, 0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38, 0x2e, 0x31, 0x2e, 0x31, 0x2c, 0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38, 0x2e, 0x31, 0x2e, 0x31, 0x2c, 0x38, 0x2e, 0x38, 0x2e, 0x38, 0x2e, 0x38, 0x2c, 0x33, 0x2c, 0x30, 0x2c, 0x30, 0x2c, 0x55, 0x53, 0x2c, 0x31, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x31, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x15, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x0d, 0x0a, 0x5b, 0x54, 0x43, 0x50, 0x20, 0x20, 0x52, 0x43, 0x5d, 0x20, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6e, 0x67, 0x20, 0x74, 0x6f, 0x20, 0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38, 0x2e, 0x31, 0x2e, 0x31, 0x31, 0x30, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20, 0x15, 0x0d, 0x0a, 0x0d, 0x0a, 0x4f, 0x4b, 0x0d, 0x0a, 0x3e, 0x20};
  TestCaseLen = sizeof(temp);
  memcpy(DeviceTestCaseBuffer, temp, TestCaseLen);
}

int main(void)
{
  char *SSID = "SpectrumSetup-A0";
  char *PASSWORD = "dustylight459";
  char *SECURITY = "WPA2-Mixed";
  
  hw_init();

  feed_testcase();

  FuzzStart();

  if (WIFI_Connect(SSID, PASSWORD, net_wifi_string_to_security(SECURITY)) != NET_OK) {
	  __BKPT(0x10);
  }

  FuzzFinish();

  for (;;);
}
