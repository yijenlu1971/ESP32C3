/*---------------------
		V2 LED����ɫ���ĳ�ʼ��
*/
#include "main.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOE_CLK_ENABLE();		  											// ��ʼ��V2 LED  GPIOʱ��	
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);		//����V2 LED

	GPIO_InitStruct.Pin 		= GPIO_PIN_3;					 // V2 LED ����
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;	 // �������ģʽ
	GPIO_InitStruct.Pull 	= GPIO_PULLUP;					 // �ڲ�����
	GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW; // ����ģʽ
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

