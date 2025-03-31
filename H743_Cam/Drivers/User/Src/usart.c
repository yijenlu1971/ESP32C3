/*
 USART1 初始化
***/


#include "main.h"

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	if(huart->Instance==USART1)
	{
		__HAL_RCC_USART1_CLK_ENABLE();		// 开启 USART1 时钟
		__HAL_RCC_GPIOA_CLK_ENABLE();			// 开启GPIOA时钟

		GPIO_InitStruct.Pin 			= GPIO_PIN_9|GPIO_PIN_10;			// TX/RX引脚
		GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;							// 复用推挽输出
		GPIO_InitStruct.Pull 		= GPIO_PULLUP;									// 上拉
		GPIO_InitStruct.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;	// 速度等级 
		GPIO_InitStruct.Alternate 	= GPIO_AF7_USART1;					// 复用为USART1
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	}
}

/*************************************************************************************************
*	函 数 名:	USART1_Init
*	入口参数:	无
*	返 回 值:	无
*	函数功能:	初始化串口配置
*	说    明:	无		 
*************************************************************************************************/

void USART1_Init(void)
{
  
	huart1.Instance = USART1;          						//选择USART1
  huart1.Init.BaudRate = 115200;     						//速率115200bps
  huart1.Init.WordLength = UART_WORDLENGTH_8B;  //数据位:8位
  huart1.Init.StopBits = UART_STOPBITS_1;       //停止位:1位	
  huart1.Init.Parity = UART_PARITY_NONE;        //校验: 无
  huart1.Init.Mode = UART_MODE_TX_RX;           //发送和接收模式
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;  //硬件流控: 无
  huart1.Init.OverSampling = UART_OVERSAMPLING_16; //过采样模式为 16 倍采样
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE; //采样模式为 禁用一位采样  默认是3位采样,接收精度更高
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1; //时钟预分频器设置为 1，即不进行分频
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT; //高级功能初始化设置为 不进行任何高级功能初始化。
  if (HAL_UART_Init(&huart1) != HAL_OK)  //检查 USART1 的初始化是否成功
  {
    Error_Handler();                     //错误处理
  }
}


//--------------用于printf 格式化输出函数的实现 fputc的重定义--------------------------------------------
int fputc(int ch, FILE* stream)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	 
    return ch;
}
