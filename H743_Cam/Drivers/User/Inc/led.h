
/*----------------------------------------- LED���ƺ� ----------------------------------*/
						
#define LED1_ON 	  	HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET)		// ����͵�ƽ������LED1	
#define LED1_OFF 	  	HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET)			// ����ߵ�ƽ���ر�LED1	
#define LED1_Toggle	HAL_GPIO_TogglePin(LED1_PORT,LED1_PIN);										  // ��תIO��״̬
			
/*---------------------------------------- �������� ------------------------------------*/

void LED_Init(void);




