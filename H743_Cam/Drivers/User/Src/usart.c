/*
 USART1 ��ʼ��
***/


#include "main.h"

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	if(huart->Instance==USART1)
	{
		__HAL_RCC_USART1_CLK_ENABLE();		// ���� USART1 ʱ��
		__HAL_RCC_GPIOA_CLK_ENABLE();			// ����GPIOAʱ��

		GPIO_InitStruct.Pin 			= GPIO_PIN_9|GPIO_PIN_10;			// TX/RX����
		GPIO_InitStruct.Mode 		= GPIO_MODE_AF_PP;							// �����������
		GPIO_InitStruct.Pull 		= GPIO_PULLUP;									// ����
		GPIO_InitStruct.Speed 		= GPIO_SPEED_FREQ_VERY_HIGH;	// �ٶȵȼ� 
		GPIO_InitStruct.Alternate 	= GPIO_AF7_USART1;					// ����ΪUSART1
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	}
}

/*************************************************************************************************
*	�� �� ��:	USART1_Init
*	��ڲ���:	��
*	�� �� ֵ:	��
*	��������:	��ʼ����������
*	˵    ��:	��		 
*************************************************************************************************/

void USART1_Init(void)
{
  
	huart1.Instance = USART1;          						//ѡ��USART1
  huart1.Init.BaudRate = 115200;     						//����115200bps
  huart1.Init.WordLength = UART_WORDLENGTH_8B;  //����λ:8λ
  huart1.Init.StopBits = UART_STOPBITS_1;       //ֹͣλ:1λ	
  huart1.Init.Parity = UART_PARITY_NONE;        //У��: ��
  huart1.Init.Mode = UART_MODE_TX_RX;           //���ͺͽ���ģʽ
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;  //Ӳ������: ��
  huart1.Init.OverSampling = UART_OVERSAMPLING_16; //������ģʽΪ 16 ������
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE; //����ģʽΪ ����һλ����  Ĭ����3λ����,���վ��ȸ���
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1; //ʱ��Ԥ��Ƶ������Ϊ 1���������з�Ƶ
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT; //�߼����ܳ�ʼ������Ϊ �������κθ߼����ܳ�ʼ����
  if (HAL_UART_Init(&huart1) != HAL_OK)  //��� USART1 �ĳ�ʼ���Ƿ�ɹ�
  {
    Error_Handler();                     //������
  }
}


//--------------����printf ��ʽ�����������ʵ�� fputc���ض���--------------------------------------------
int fputc(int ch, FILE* stream)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	 
    return ch;
}
