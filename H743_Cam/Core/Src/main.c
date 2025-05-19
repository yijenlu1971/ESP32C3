/****************************************************************************
* Copyright (C), 2024 奋斗嵌入式工作室 www.fdiot.top
*
* 本例程在 奋斗版STM32H743VIT开发板V1.0上调试通过           
* QQ: 9191274, 旺旺：sun68, Email: sun68@163.com 
* 淘宝店铺：ourstm.taobao.com  
*
* 文件名: main.c
* 内容简述:	
*       
*	演示同奋斗OV2640摄像头采集的图像在奋斗1.69寸液晶（240*280）上显示出来。
    
	基于MDK版本：        5.3
	基于官方HAL库
*
* 文件历史:
* 版本号  日期       作者    说明
* v0.1    2024-11-28 sun68  创建该文件
*/
#define GLOBALS
#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOSConfig.h"
#include "lcd_169_drv.h"
#include "dcmi_ov2640.h"
#include "ei_logging.h"
#include "sdram.h"

#define ROUT_STACK_SIZE			( (uint16_t) 200 )
#define EI_STACK_SIZE				( (uint16_t) 300 )

char	tskRoutName[] = "ROUT";
char	tskEiName[] = "EIDT";
osThreadId_t	tskId_Routine, tskId_Ei;
uint8_t bDmaBufA = TRUE, bEiFlag = FALSE;


uint16_t	Camera_BufferA[Display_Width*Display_Height];
uint16_t	Camera_BufferB[Display_Width*Display_Height];
__attribute__((section(".ramd2"))) uint16_t	Camera_BufferC[Display_Width*Display_Height*3/2];
//uint16_t	*Camera_BufferA = (uint16_t *)0xC0000000;
//uint16_t	*Camera_BufferB = (uint16_t *)ADDR_CamBufB;
//uint16_t	*Camera_BufferC = (uint16_t *)ADDR_CamBufC;
//__attribute__((section(".exsram"))) volatile uint8_t myTest[128];

SDRAM_HandleTypeDef hsdram1;
UART_HandleTypeDef huart1;

/********************************************** 函数声明 *******************************************/
void SystemClock_Config(void);		// 时钟初始化
void MPU_Config(void);					// MPU配置
void MX_FMC_Init(void);
extern void ei_edge_impulse();

//-----------液晶背光控制脚配置------------------------------
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  
#ifdef STM32H750xx
  __HAL_RCC_GPIOH_CLK_ENABLE();
  /*Configure GPIO pin : PH6 背景光控制*/
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);	
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);   //打开液晶背光
#else
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  /*Configure GPIO pin : PA7 背景光控制*/
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);   //打开液晶背光

	/*Configure GPIO pin : PE12 补光LED控制*/
	GPIO_InitStruct.Pin = GPIO_PIN_12;                   
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET);   //补光LED
#endif
}

void Routine( void *arg )
{
 	SPI_LCD_Init();      	// 液晶屏以及SPI初始化 
 	LCD_DisplayString(10, 130, "Waiting Please...");
	DCMI_OV2640_Init();   			 	// DCMI以及OV2640初始化
	OV2640_DMA_Transmit_Continuous((uint32_t)Camera_BufferA, OV2640_BufferSize);  // 启动DMA连续传输
//	OV2640_DMA_Transmit_Snapshot(bDmaBufA ? Camera_BufferA : Camera_BufferB, OV2640_BufferSize);

	while (1)
	{
		if ( DCMI_FrameState == 1 )	// 采集到了一帧图像
		{		
  		DCMI_FrameState = 0;		// 清标志位
			bEiFlag = TRUE;

			LCD_CopyBuffer(0, 0, Display_Width, Display_Height, 
					(uint16_t *)Camera_BufferB);	// 将图像数据复制到屏幕
//					bDmaBufA ? (uint16_t *)Camera_BufferB : (uint16_t *)Camera_BufferA);	// 将图像数据复制到屏幕
			LCD_DisplayString( 34, 252, "fps:");
			LCD_DisplayNumber( 80, 252, OV2640_FPS, 2) ;	// 显示帧率	
			
			LED_Toggle;	// PE3
		}
		
		osDelay(5);
	}
}

void EI_Proc( void *arg )
{
	while(1)
	{
		if( bEiFlag )
		{
			bEiFlag = FALSE;
			ei_edge_impulse();
		}

		osDelay(5);
	}
}

/***************************************************************************************************
*	函 数 名: main
*
*	说    明: 主程序
*
****************************************************************************************************/

int main(void)
{
	volatile uint32_t	clk;
	osThreadAttr_t	osAttr;

	for(uint8_t i = 0; i < 8; i++)
	{
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}
	SCB->VTOR = QSPI_BASE;
	__enable_irq();
	__set_PRIMASK(0);

//	MPU_Config();				  // MPU配置   用于配置内存保护单元 0x24000000开始的内存
	SCB_EnableICache();		// 使能ICache  通过启用指令缓存来加速程序执行和减少内存访问延迟
	SCB_EnableDCache();		// 使能DCache  通过启用数据缓存来加速数据访问和减少内存带宽消耗
	HAL_Init();					  // 初始化HAL库
	SystemClock_Config();	// 配置系统时钟，主频480MHz

	/*clk = HAL_RCC_GetSysClockFreq();
	clk = HAL_RCC_GetHCLKFreq();
	clk = HAL_RCC_GetPCLK1Freq();
	clk = HAL_RCC_GetPCLK2Freq();*/

	LED_Init();					  // 初始化LED引脚 (PE3)
	USART1_Init();				// USART1初始化	(PA9, PA10)
	MX_GPIO_Init();       // 液晶背光和摄像头补光LED控制 (PA7, PE12)
#ifdef STM32H750xx
	MX_FMC_Init();
	SDRAM_Initialization_Sequence(&hsdram1);	// 配置SDRAM相关时序和控制方式
//	uint8_t rc = SDRAM_Test();
//	myTest[0] = 0x01;
#endif

	if (osKernelInitialize() != osOK)
	{
		EI_LOGE("osKernelInitialize error!!!\r\n");
		return -1;
	}

	EI_LOGI("osThreadNew\r\n");

	memset(&osAttr, 0, sizeof(osAttr));
	osAttr.name = tskRoutName;
	osAttr.stack_size = ROUT_STACK_SIZE;
	osAttr.priority = osPriorityNormal;
	osAttr.attr_bits = osThreadDetached;
	tskId_Routine = osThreadNew( Routine, NULL, &osAttr );

	osAttr.name = tskEiName;
	osAttr.stack_size = EI_STACK_SIZE;
	tskId_Ei = osThreadNew( EI_Proc, NULL, &osAttr );

	EI_LOGI("osKernelStart\r\n");
	osKernelStart();
	
  /* Infinite loop */
	while (1)
	{
	}
}

/****************************************************************************************************/
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 480000000 (CPU Clock)
  *            HCLK(Hz)                       = 240000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  120MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  120MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  120MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  120MHz)
  *            HSE Frequency(Hz)              = 24 MHz
  *            PLL_M                          = 4
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 2
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
#ifdef STM32H750xx
  RCC_OscInitStruct.PLL.PLLM = 5;		// 25MHz/5 * 192 /2
  RCC_OscInitStruct.PLL.PLLN = 192;
#else
  RCC_OscInitStruct.PLL.PLLM = 4;		// 24MHz/4 * 160 /2
  RCC_OscInitStruct.PLL.PLLN = 160;
#endif
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);    
  
}

//	配置MPU (Memory Protection Unit)
//
void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	HAL_MPU_Disable();		// 先禁止MPU

	MPU_InitStruct.Enable 				= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress 		= 0x24000000;
	MPU_InitStruct.Size 					= MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission 	= MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable 		= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable 		= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable 		= MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number 				= MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField 		= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable 	= 0x00;
	MPU_InitStruct.DisableExec 		= MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);	

	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);	// 使能MPU
}

#ifdef STM32H750xx
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

#endif
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
