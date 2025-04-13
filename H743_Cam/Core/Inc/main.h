#include "stm32h7xx_hal.h"
#ifdef GLOBALS 
#define EXT
#else
#define EXT extern 
#endif

#include "stdlib.h"
#include "stdio.h"
#include <string.h>

#define TRUE		1
#define FALSE		0

EXT UART_HandleTypeDef huart1;  // UART_HandleTypeDef �ṹ�����

//#define Camera_BufferA	0x24000000    // ����ͷͼ�񻺳���
//#define Camera_BufferB	0x24020000
//#define Camera_BufferC	0x24040000

#define LED_OFF 	  	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)		// ����͵�ƽ������LED1	
#define LED_ON 	  		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET)			// ����ߵ�ƽ���ر�LED1	
#define LED_Toggle	  HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_3);										  // ��תIO��״̬


void LED_Init(void);
void Error_Handler(void);

extern uint8_t bDmaBufA;
extern uint16_t	Camera_BufferA[], Camera_BufferB[], Camera_BufferC[];