/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

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

#define ADDR_CamBufA	0xC0000000
#define ADDR_CamBufB	0xC0020000
#define ADDR_CamBufC	0xC0040000
#define ADDR_Tensor		0xC0080000

#define LED_OFF 	  	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)		// ����͵�ƽ������LED1	
#define LED_ON 	  		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET)			// ����ߵ�ƽ���ر�LED1	
#define LED_Toggle	  HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_3);										  // ��תIO��״̬


void LED_Init(void);
void Error_Handler(void);

extern uint8_t bDmaBufA;
extern uint16_t	Camera_BufferA[], Camera_BufferB[], Camera_BufferC[];
//extern uint16_t	Camera_BufferA[], *Camera_BufferB, *Camera_BufferC;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
