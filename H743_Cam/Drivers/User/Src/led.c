/*---------------------
		V2 LED（蓝色）的初始化
*/
#include "main.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

#ifdef STM32H750xx
	__HAL_RCC_GPIOC_CLK_ENABLE();		  											// 初始化V2 LED  GPIO时钟	
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);		//点亮V2 LED

	GPIO_InitStruct.Pin 		= GPIO_PIN_13;				 // V2 LED 引脚
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;	 // 推挽输出模式
	GPIO_InitStruct.Pull 	= GPIO_PULLUP;					 // 内部上拉
	GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW; // 低速模式
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);	
#else
	__HAL_RCC_GPIOE_CLK_ENABLE();		  											// 初始化V2 LED  GPIO时钟	
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);		//点亮V2 LED

	GPIO_InitStruct.Pin 		= GPIO_PIN_3;					 // V2 LED 引脚
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;	 // 推挽输出模式
	GPIO_InitStruct.Pull 	= GPIO_PULLUP;					 // 内部上拉
	GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW; // 低速模式
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
#endif
}

