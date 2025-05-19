/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
//#include "w25q64.h"
#include "FlashOS.h"
#include "bsp_qspi_w25q256.h"
#include "sdram.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

//QSPI_HandleTypeDef hqspi;


/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_QUADSPI_Init(void);
void MX_FMC_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define VECT_TAB_OFFSET      0x00000000UL
#define APPLICATION_ADDRESS  0x90000000UL

typedef  void (*pFunction)(void);
pFunction JumpToApplication;
//Flash_T w25q64 = Flash_T();
SDRAM_HandleTypeDef hsdram1;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* USER CODE BEGIN 1 */
	int rc;

	/* USER CODE END 1 */

	/* MPU Configuration--------------------------------------------------------*/
	//MPU_Config();
	SCB_EnableICache();		// 使能ICache  通过启用指令缓存来加速程序执行和减少内存访问延迟
	SCB_EnableDCache();		// 使能DCache  通过启用数据缓存来加速数据访问和减少内存带宽消耗

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();

	MX_FMC_Init();
	SDRAM_Initialization_Sequence(&hsdram1);	// 配置SDRAM相关时序和控制方式
//	rc = SDRAM_Test();
	memset((uint8_t *)SDRAM_BANK_ADDR, 0, SDRAM_Size);

	//MX_QUADSPI_Init();
	/* USER CODE BEGIN 2 */
	//w25q64.init();
	rc = bsp_InitQSPI_W25Q256();
	HAL_MPU_Disable();
	// SCB_DisableDCache();
	//w25q64.memory_map();
	rc = QSPI_DeviceReset();
	rc = QSPI_MemoryMapped();
	//rc = ProgramPage(QSPI_FLASH_MEM_ADDR, 256, buf);

	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	for(uint8_t i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}
	__set_CONTROL(0);
	__disable_irq();
	__set_PRIMASK(1);
	
	HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);
//	printf("Jump to App\r\n");

	JumpToApplication = (pFunction) (*(__IO uint32_t*)(APPLICATION_ADDRESS + 4));
	__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
	JumpToApplication();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */    
		HAL_Delay(500);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	}
  	/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
	Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
							  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
							  |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
	Error_Handler();
  }
}
#if 0
/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 2;
  hqspi.Init.FifoThreshold = 32;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = POSITION_VAL(0X800000)-1;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
	Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */
//   HAL_QSPI_MspInit(&hqspi);
  /* USER CODE END QUADSPI_Init 2 */

}
#endif
/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void MX_FMC_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime = 4;
  SdramTiming.RowCycleDelay = 7;
  SdramTiming.WriteRecoveryTime = 3;
  SdramTiming.RPDelay = 2;
  SdramTiming.RCDDelay = 2;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

extern "C"
{
void ExSRAM_Init(void)
{
	__IO uint32_t tmpmrd = 0;
	FMC_SDRAM_CommandTypeDef *Command;

	MX_FMC_Init();

	/* Configure a clock configuration enable command */
	Command->CommandMode 					= FMC_SDRAM_CMD_CLK_ENABLE;	// 开启SDRAM时钟 
	Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK; 	// 选择要控制的区域
	Command->AutoRefreshNumber 		= 1;
	Command->ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&hsdram1, Command, SDRAM_TIMEOUT);	// 发送控制指令
	for(volatile int delay = 0; delay < 2000; delay++){}

	/* Configure a PALL (precharge all) command */ 
	Command->CommandMode 					= FMC_SDRAM_CMD_PALL;		// 预充电命令
	Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;	// 选择要控制的区域
	Command->AutoRefreshNumber 		= 1;
	Command->ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&hsdram1, Command, SDRAM_TIMEOUT);  // 发送控制指令

	/* Configure a Auto-Refresh command */ 
	Command->CommandMode 					= FMC_SDRAM_CMD_AUTOREFRESH_MODE;	// 使用自动刷新
	Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;          // 选择要控制的区域
	Command->AutoRefreshNumber		= 8;                                // 自动刷新次数
	Command->ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&hsdram1, Command, SDRAM_TIMEOUT);	// 发送控制指令

	/* Program the external memory mode register */
	tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1   |
							SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
							SDRAM_MODEREG_CAS_LATENCY_3           |
							SDRAM_MODEREG_OPERATING_MODE_STANDARD |
							SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	Command->CommandMode					= FMC_SDRAM_CMD_LOAD_MODE;	// 加载模式寄存器命令
	Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;	// 选择要控制的区域
	Command->AutoRefreshNumber 		= 1;
	Command->ModeRegisterDefinition 	= tmpmrd;

	HAL_SDRAM_SendCommand(&hsdram1, Command, SDRAM_TIMEOUT);	// 发送控制指令
	
	HAL_SDRAM_ProgramRefreshRate(&hsdram1, 1543);  // 配置刷新率
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
}
void SVC_Handler(void)
{
	__set_CONTROL( __get_CONTROL( ) & ~CONTROL_nPRIV_Msk ) ;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
