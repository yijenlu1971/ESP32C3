/****************************************************************************
* Copyright (C), 2024 �ܶ�Ƕ��ʽ������ www.fdiot.top
*
* �������� �ܶ���STM32H743VIT������V1.0�ϵ���ͨ��           
* QQ: 9191274, ������sun68, Email: sun68@163.com 
* �Ա����̣�ourstm.taobao.com  
*
* �ļ���: main.c
* ���ݼ���:	
*       
*	��ʾͬ�ܶ�OV2640����ͷ�ɼ���ͼ���ڷܶ�1.69��Һ����240*280������ʾ������
    
	����MDK�汾��        5.3
	���ڹٷ�HAL��
*
* �ļ���ʷ:
* �汾��  ����       ����    ˵��
* v0.1    2024-11-28 sun68  �������ļ�
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

/********************************************** �������� *******************************************/
void SystemClock_Config(void);		// ʱ�ӳ�ʼ��
void MPU_Config(void);					// MPU����
void MX_FMC_Init(void);
extern void ei_edge_impulse();

//-----------Һ��������ƽ�����------------------------------
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* GPIO Ports Clock Enable */
  
#ifdef STM32H750xx
  __HAL_RCC_GPIOH_CLK_ENABLE();
  /*Configure GPIO pin : PH6 ���������*/
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);	
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);   //��Һ������
#else
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  /*Configure GPIO pin : PA7 ���������*/
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);   //��Һ������

	/*Configure GPIO pin : PE12 ����LED����*/
	GPIO_InitStruct.Pin = GPIO_PIN_12;                   
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET);   //����LED
#endif
}

void Routine( void *arg )
{
 	SPI_LCD_Init();      	// Һ�����Լ�SPI��ʼ�� 
 	LCD_DisplayString(10, 130, "Waiting Please...");
	DCMI_OV2640_Init();   			 	// DCMI�Լ�OV2640��ʼ��
	OV2640_DMA_Transmit_Continuous((uint32_t)Camera_BufferA, OV2640_BufferSize);  // ����DMA��������
//	OV2640_DMA_Transmit_Snapshot(bDmaBufA ? Camera_BufferA : Camera_BufferB, OV2640_BufferSize);

	while (1)
	{
		if ( DCMI_FrameState == 1 )	// �ɼ�����һ֡ͼ��
		{		
  		DCMI_FrameState = 0;		// ���־λ
			bEiFlag = TRUE;

			LCD_CopyBuffer(0, 0, Display_Width, Display_Height, 
					(uint16_t *)Camera_BufferB);	// ��ͼ�����ݸ��Ƶ���Ļ
//					bDmaBufA ? (uint16_t *)Camera_BufferB : (uint16_t *)Camera_BufferA);	// ��ͼ�����ݸ��Ƶ���Ļ
			LCD_DisplayString( 34, 252, "fps:");
			LCD_DisplayNumber( 80, 252, OV2640_FPS, 2) ;	// ��ʾ֡��	
			
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
*	�� �� ��: main
*
*	˵    ��: ������
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

//	MPU_Config();				  // MPU����   ���������ڴ汣����Ԫ 0x24000000��ʼ���ڴ�
	SCB_EnableICache();		// ʹ��ICache  ͨ������ָ��������ٳ���ִ�кͼ����ڴ�����ӳ�
	SCB_EnableDCache();		// ʹ��DCache  ͨ���������ݻ������������ݷ��ʺͼ����ڴ��������
	HAL_Init();					  // ��ʼ��HAL��
	SystemClock_Config();	// ����ϵͳʱ�ӣ���Ƶ480MHz

	/*clk = HAL_RCC_GetSysClockFreq();
	clk = HAL_RCC_GetHCLKFreq();
	clk = HAL_RCC_GetPCLK1Freq();
	clk = HAL_RCC_GetPCLK2Freq();*/

	LED_Init();					  // ��ʼ��LED���� (PE3)
	USART1_Init();				// USART1��ʼ��	(PA9, PA10)
	MX_GPIO_Init();       // Һ�����������ͷ����LED���� (PA7, PE12)
#ifdef STM32H750xx
	MX_FMC_Init();
	SDRAM_Initialization_Sequence(&hsdram1);	// ����SDRAM���ʱ��Ϳ��Ʒ�ʽ
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

//	����MPU (Memory Protection Unit)
//
void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	HAL_MPU_Disable();		// �Ƚ�ֹMPU

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

	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);	// ʹ��MPU
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
	Command->CommandMode 					= FMC_SDRAM_CMD_CLK_ENABLE;	// ����SDRAMʱ�� 
	Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK; 	// ѡ��Ҫ���Ƶ�����
	Command->AutoRefreshNumber 		= 1;
	Command->ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&hsdram1, Command, SDRAM_TIMEOUT);	// ���Ϳ���ָ��
	for(volatile int delay = 0; delay < 2000; delay++){}

	/* Configure a PALL (precharge all) command */ 
	Command->CommandMode 					= FMC_SDRAM_CMD_PALL;		// Ԥ�������
	Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;	// ѡ��Ҫ���Ƶ�����
	Command->AutoRefreshNumber 		= 1;
	Command->ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&hsdram1, Command, SDRAM_TIMEOUT);  // ���Ϳ���ָ��

	/* Configure a Auto-Refresh command */ 
	Command->CommandMode 					= FMC_SDRAM_CMD_AUTOREFRESH_MODE;	// ʹ���Զ�ˢ��
	Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;          // ѡ��Ҫ���Ƶ�����
	Command->AutoRefreshNumber		= 8;                                // �Զ�ˢ�´���
	Command->ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&hsdram1, Command, SDRAM_TIMEOUT);	// ���Ϳ���ָ��

	/* Program the external memory mode register */
	tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1   |
							SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
							SDRAM_MODEREG_CAS_LATENCY_3           |
							SDRAM_MODEREG_OPERATING_MODE_STANDARD |
							SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	Command->CommandMode					= FMC_SDRAM_CMD_LOAD_MODE;	// ����ģʽ�Ĵ�������
	Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;	// ѡ��Ҫ���Ƶ�����
	Command->AutoRefreshNumber 		= 1;
	Command->ModeRegisterDefinition 	= tmpmrd;

	HAL_SDRAM_SendCommand(&hsdram1, Command, SDRAM_TIMEOUT);	// ���Ϳ���ָ��
	
	HAL_SDRAM_ProgramRefreshRate(&hsdram1, 1543);  // ����ˢ����
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
