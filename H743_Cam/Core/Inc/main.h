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

EXT UART_HandleTypeDef huart1;  // UART_HandleTypeDef 结构体变量

//#define Camera_BufferA	0x24000000    // 摄像头图像缓冲区
//#define Camera_BufferB	0x24020000
//#define Camera_BufferC	0x24040000

#define LED_OFF 	  	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)		// 输出低电平，点亮LED1	
#define LED_ON 	  		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET)			// 输出高电平，关闭LED1	
#define LED_Toggle	  HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_3);										  // 翻转IO口状态


void LED_Init(void);
void Error_Handler(void);

extern uint8_t bDmaBufA;
extern uint16_t	Camera_BufferA[], Camera_BufferB[], Camera_BufferC[];