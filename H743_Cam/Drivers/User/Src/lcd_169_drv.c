/***
1.69��LCD ���ú���
***/

#include "lcd_169_drv.h"

SPI_HandleTypeDef hspi1;	      // SPI_HandleTypeDef �ṹ�����

#define  LCD_SPI hspi1           // SPI�ֲ��꣬�����޸ĺ���ֲ

static pFONT *LCD_AsciiFonts;		// Ӣ�����壬ASCII�ַ���
static pFONT *LCD_CHFonts;		   // �������壨ͬʱҲ����Ӣ�����壩

// Ϊ������ַ���ʾ�ٶȣ������Դ��д�룬��Ҫ�������ַ����ص�һ����д�룬������һ�����ص�д�� 
// ������Ҫ����һ���ַ���ʾ������
// �û�����ʵ�����ȥ�޸Ĵ˴��������Ĵ�С��
// �����û���Ҫ��ʾ32*32����ĺ���ʱ����Ҫ�Ĵ�СΪ 32*32*2 = 2048 �ֽڣ�ÿ�����ص�ռ2�ֽڣ�
uint16_t  LCD_Buff[1024];        // LCD��������16λ��ÿ�����ص�ռ2�ֽڣ�

struct	
{
	uint32_t Color;  				   //	������ɫ
	uint32_t BkgColor;			   //	������ɫ
	uint8_t  Direction;				 //	��ʾ����
  uint16_t Width;            // ��Ļ���ؿ��
  uint16_t Height;           // ��Ļ���ظ߶�	
  uint8_t  X_Offset;         // X����ƫ��
  uint8_t  Y_Offset;         // Y����ƫ��
}TFT_Ptr;  //Һ�������ṹ

// �ú����޸���HAL��SPI�⺯����ԭ������д�볤�����ƣ������޸�ΪSPI�������ݲ������ݳ��ȵ�д��
HAL_StatusTypeDef LCD_SPI_Transmit(SPI_HandleTypeDef *hspi, uint16_t pData, uint32_t Size);
HAL_StatusTypeDef LCD_SPI_TransmitBuffer (SPI_HandleTypeDef *hspi, uint16_t *pData, uint32_t Size);

extern void Error_Handler(void);

/****************************************************************************************************************************************
*	�� �� ��:	HAL_SPI_MspInit
*	��ڲ���:	hspi - SPI_HandleTypeDef����ı���������ʾ����� SPI ���
*	��������:	��ʼ�� SPI ����
****************************************************************************************************************************************/

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
   GPIO_InitTypeDef GPIO_InitStruct = {0};
   if(hspi->Instance==SPI1)
   {
		
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
    PeriphClkInitStruct.PLL2.PLL2M = 16;
    PeriphClkInitStruct.PLL2.PLL2N = 100;
    PeriphClkInitStruct.PLL2.PLL2P = 1;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 2;                 //24MHZ/16*100/10=15MHZ
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
		__HAL_RCC_SPI1_CLK_ENABLE();			// ʹ��SPI1ʱ��
		__HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();

/**SPI1 GPIO Configuration
    PD7  ------> SPI1_MOSI
    PB3  ------> SPI1_SCK
    PB4  ------> SPI1_MISO
    */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
		
		/*PA15 SPI1 Һ����Ƭѡ */
		GPIO_InitStruct.Pin =GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
	
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
		
		/*PC4 Һ��SPI C/D  */
		/*PC13 Һ��SPI RST  */
		GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_4;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	}
  
}

/****************************************************************************************************************************************
*	�� �� ��:	MX_SPI1_Init
*	��������:	��ʼ��SPI����
*	˵    ��:ʹ�����Ƭѡ	 
****************************************************************************************************************************************/


void MX_SPI1_Init(void)
{
	LCD_SPI.Instance 									= SPI1;							   				//	ʹ��SPI1
	LCD_SPI.Init.Mode 								= SPI_MODE_MASTER;            //	����ģʽ
	LCD_SPI.Init.Direction 						= SPI_DIRECTION_1LINE;        //	����
	LCD_SPI.Init.DataSize 						= SPI_DATASIZE_8BIT;        	//	8λ���ݿ��
	LCD_SPI.Init.CLKPolarity 					= SPI_POLARITY_LOW;         	//	CLK����ʱ���ֵ͵�ƽ
	LCD_SPI.Init.CLKPhase 						= SPI_PHASE_1EDGE;          	//	������CLK��һ��������Ч
	LCD_SPI.Init.NSS 									= SPI_NSS_SOFT;        			  //	ʹ�����Ƭѡ   
	
  // SPI����ʱ��Ϊ100M������2��Ƶ�õ�50M ��SCKʱ��
	LCD_SPI.Init.BaudRatePrescaler 		= SPI_BAUDRATEPRESCALER_2;
	
	LCD_SPI.Init.FirstBit	 						= SPI_FIRSTBIT_MSB;					 	//	��λ����
	LCD_SPI.Init.TIMode 							= SPI_TIMODE_DISABLE;        	//	��Ϊ��ȷ�� SPI �ӿ��Ա�׼ģʽ���У��Ӷ���� TI �豸���м��ݵ����ݴ���
	LCD_SPI.Init.CRCCalculation				= SPI_CRCCALCULATION_DISABLE;	//	��ֹCRC
	LCD_SPI.Init.CRCPolynomial 				= 0x0;                       	// CRCУ����				
	LCD_SPI.Init.NSSPMode 						= SPI_NSS_PULSE_ENABLE;      	//	ʹ��Ƭѡ����ģʽ
	LCD_SPI.Init.NSSPolarity 					= SPI_NSS_POLARITY_LOW;      	//	Ƭѡ�͵�ƽ��Ч
	LCD_SPI.Init.FifoThreshold 				= SPI_FIFO_THRESHOLD_02DATA;  //	FIFO��ֵ
	LCD_SPI.Init.TxCRCInitializationPattern 	= SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;   // ���Ͷ�CRC��ʼ��ģʽ�������ò���
	LCD_SPI.Init.RxCRCInitializationPattern 	= SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;   // ���ն�CRC��ʼ��ģʽ�������ò���
	LCD_SPI.Init.MasterSSIdleness 				= SPI_MASTER_SS_IDLENESS_00CYCLE;            // �����ӳ�����Ϊ0
	LCD_SPI.Init.MasterInterDataIdleness 		= SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;     // ����ģʽ�£���������֮֡����ӳ�����
	LCD_SPI.Init.MasterReceiverAutoSusp 		= SPI_MASTER_RX_AUTOSUSP_DISABLE;            // ��ֹ�Զ����չ���
	LCD_SPI.Init.MasterKeepIOState 				= SPI_MASTER_KEEP_IO_STATE_DISABLE; 	 		//	����ģʽ�£���ֹSPI���ֵ�ǰ����״̬
	LCD_SPI.Init.IOSwap 								= SPI_IO_SWAP_DISABLE;				            // ������MOSI��MISO
	
   HAL_SPI_Init(&LCD_SPI);
}
/****************************************************************************************************************************************
*	�� �� ��: TFT_SEND_CMD
*
*	��ڲ���: ��Ҫд��Ŀ���ָ��
*
*	��������: ��������Ļ������д��ָ��
*
****************************************************************************************************************************************/

void  TFT_SEND_CMD(uint8_t lcd_command)
{
	 TFT_DC_C;     // ����ָ��ѡ�� �͵�ƽ:����ָ��
   LCD_CSL;  		 //����CS���ţ���ʼ����
   HAL_SPI_Transmit(&LCD_SPI, &lcd_command, 1, 1000); // ����SPI����
	 LCD_CSH;  		//����CS����,�������
}

/****************************************************************************************************************************************
*	�� �� ��: TFT_SEND_DATA
*
*	��ڲ���: lcd_data - ��Ҫд������ݣ�8λ
*
*	��������: д��8λ����
*	
****************************************************************************************************************************************/

void  TFT_SEND_DATA(uint8_t lcd_data)
{
   TFT_DC_D;     // ����ָ��ѡ�� ��������ߵ�ƽ�������δ��� ����
	 LCD_CSL;  //����NSS���ţ���ʼ����
   HAL_SPI_Transmit(&LCD_SPI, &lcd_data, 1, 1000) ; // ����SPI����
	 LCD_CSH;  //����NSS���ţ���ʼ����
}

/****************************************************************************************************************************************
*	�� �� ��: TFT_SEND_DATA_16b
*
*	��ڲ���: lcd_data - ��Ҫд������ݣ�16λ
*
*	��������: д��16λ����
*	
****************************************************************************************************************************************/

void  TFT_SEND_DATA_16b(uint16_t lcd_data)
{
   uint8_t lcd_data_buff[2];   // ���ݷ�����
   TFT_DC_D;     					 // ����ָ��ѡ�� ��������ߵ�ƽ�������δ��� ����
	 LCD_CSL;  									 //����CS���ţ���ʼ����
   lcd_data_buff[0] = lcd_data>>8;  // �����ݲ��
   lcd_data_buff[1] = lcd_data;
		
	 HAL_SPI_Transmit(&LCD_SPI, lcd_data_buff, 2, 1000) ;   // ����SPI����
	 LCD_CSH;  //����CS����,�������
}

/****************************************************************************************************************************************
*	�� �� ��: LCD_WriteBuff
*
*	��ڲ���: DataBuff - ��������DataSize - ���ݳ���
*
*	��������: ����д�����ݵ���Ļ
*	
****************************************************************************************************************************************/

void  LCD_WriteBuff(uint16_t *DataBuff, uint16_t DataSize)
{
	TFT_DC_D;     // ����ָ��ѡ�� �ߵ�ƽ:��������	
	LCD_CSL;  	//����CS���ţ���ʼ����
  //��Ϊ16λ���ݿ�ȣ����Ч��
  LCD_SPI.Init.DataSize 	= SPI_DATASIZE_16BIT;   //	16λ���ݿ��
  HAL_SPI_Init(&LCD_SPI);	
	HAL_SPI_Transmit(&LCD_SPI, (uint8_t *)DataBuff, DataSize, 1000) ; // ����SPI����
	LCD_CSH;  //����CS����,�������
  //�Ļ�8λ���ݿ�ȣ���Ϊָ��Ͳ������ݶ��ǰ���8λ�����
	LCD_SPI.Init.DataSize 	= SPI_DATASIZE_8BIT;    //	8λ���ݿ��
  HAL_SPI_Init(&LCD_SPI);	
}


/****************************************************************************************************************************************
*	�� �� ��: SPI_LCD_Init
*
*	��������: ��ʼ��SPI�Լ���Ļ�������ĸ��ֲ���
*	
****************************************************************************************************************************************/

void SPI_LCD_Init(void)
{ 
	MX_SPI1_Init();               // ��ʼ��SPI�Ϳ�������
  LCD_RST_L;                    //RST �õ�
	HAL_Delay(10);
	LCD_RST_H;                    //RST �ø�  ��ɶ�����ͷ�ĸ�λ
  HAL_Delay(10);               	// ��Ļ����ɸ�λʱ�������ϵ縴λ������Ҫ�ȴ�����5ms���ܷ���ָ��

 	TFT_SEND_CMD(0x36);       // ��������LCD����ʾ���������������������ɫ��ʽ��RGB��BGR��
	                              // ���ó� ���ϵ��¡������ң�RGB���ظ�ʽ
	TFT_SEND_DATA(0x00);     //����
	TFT_SEND_CMD(0x3A);				// �ӿ����ظ�ʽ ָ���������ʹ�� 12λ��16λ����18λɫ
	TFT_SEND_DATA(0x05);     // �˴����ó� 16λ ���ظ�ʽ

  // ���µ�ѹ����ָ�ʹ�ó���Ĭ��ֵ
 	TFT_SEND_CMD(0xB2);			
	TFT_SEND_DATA(0x0C);
	TFT_SEND_DATA(0x0C); 
	TFT_SEND_DATA(0x00); 
	TFT_SEND_DATA(0x33); 
	TFT_SEND_DATA(0x33); 			

	TFT_SEND_CMD(0xB7);		   // դ����ѹ����ָ��	
	TFT_SEND_DATA(0x35);    // VGH = 13.26V��VGL = -10.43V

	TFT_SEND_CMD(0xBB);			 // ������ѹ����ָ��
	TFT_SEND_DATA(0x19);    // VCOM = 1.35V

	TFT_SEND_CMD(0xC0);
	TFT_SEND_DATA(0x2C);

	TFT_SEND_CMD(0xC2);       // VDV �� VRH ��Դ����
	TFT_SEND_DATA(0x01);     // VDV �� VRH ���û���������

	TFT_SEND_CMD(0xC3);				// VRH��ѹ ����ָ��  
	TFT_SEND_DATA(0x12);     // VRH��ѹ = 4.6+( vcom+vcom offset+vdv)
				
	TFT_SEND_CMD(0xC4);		   // VDV��ѹ ����ָ��	
	TFT_SEND_DATA(0x20);     // VDV��ѹ = 0v

	TFT_SEND_CMD(0xC6); 		// ����ģʽ��֡�ʿ���ָ��
	TFT_SEND_DATA(0x0F);   	// ������Ļ��������ˢ��֡��Ϊ60֡    

	TFT_SEND_CMD(0xD0);			// ��Դ����ָ��
	TFT_SEND_DATA(0xA4);     // ��Ч���ݣ��̶�д��0xA4
	TFT_SEND_DATA(0xA1);     // AVDD = 6.8V ��AVDD = -4.8V ��VDS = 2.3V

	TFT_SEND_CMD(0xE0);       // ������ѹ٤��ֵ�趨
	TFT_SEND_DATA(0xD0);
	TFT_SEND_DATA(0x04);
	TFT_SEND_DATA(0x0D);
	TFT_SEND_DATA(0x11);
	TFT_SEND_DATA(0x13);
	TFT_SEND_DATA(0x2B);
	TFT_SEND_DATA(0x3F);
	TFT_SEND_DATA(0x54);
	TFT_SEND_DATA(0x4C);
	TFT_SEND_DATA(0x18);
	TFT_SEND_DATA(0x0D);
	TFT_SEND_DATA(0x0B);
	TFT_SEND_DATA(0x1F);
	TFT_SEND_DATA(0x23);

	TFT_SEND_CMD(0xE1);      // ������ѹ٤��ֵ�趨
	TFT_SEND_DATA(0xD0);
	TFT_SEND_DATA(0x04);
	TFT_SEND_DATA(0x0C);
	TFT_SEND_DATA(0x11);
	TFT_SEND_DATA(0x13);
	TFT_SEND_DATA(0x2C);
	TFT_SEND_DATA(0x3F);
	TFT_SEND_DATA(0x44);
	TFT_SEND_DATA(0x51);
	TFT_SEND_DATA(0x2F);
	TFT_SEND_DATA(0x1F);
	TFT_SEND_DATA(0x1F);
	TFT_SEND_DATA(0x20);
	TFT_SEND_DATA(0x23);

	TFT_SEND_CMD(0x21);       // �򿪷��ԣ���Ϊ����ǳ����ͣ�������Ҫ������

 // �˳�����ָ�LCD�������ڸ��ϵ硢��λʱ�����Զ���������ģʽ ����˲�����Ļ֮ǰ����Ҫ�˳�����  
	TFT_SEND_CMD(0x11);       // �˳����� ָ��
   HAL_Delay(120);          // ��Ҫ�ȴ�120ms���õ�Դ��ѹ��ʱ�ӵ�·�ȶ�����

 // ����ʾָ�LCD�������ڸ��ϵ硢��λʱ�����Զ��ر���ʾ 
	TFT_SEND_CMD(0x29);       // ����ʾ   	

// ���½���һЩ������Ĭ������
   LCD_SetDirection(Direction_V);  	      //	������ʾ����
	LCD_SetBackColor(LCD_BLACK);           // ���ñ���ɫ
 	LCD_SetColor(LCD_WHITE);               // ���û���ɫ  
	LCD_Clear();                           // ����

   LCD_SetAsciiFont(&ASCII_Font24);       // ����Ĭ������
 

// ȫ���������֮�󣬴򿪱���	
   LCD_Bkglight_ON;  // ��������ߵ�ƽ��������
}

/****************************************************************************************************************************************
*	�� �� ��:	 LCD_SetAddress
*
*	��ڲ���:	 x1 - ��ʼˮƽ����   y1 - ��ʼ��ֱ����  
*              x2 - �յ�ˮƽ����   y2 - �յ㴹ֱ����	   
*	
*	��������:   ������Ҫ��ʾ����������		 			 
*****************************************************************************************************************************************/

void LCD_SetAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)		
{
	TFT_SEND_CMD(0x2a);			//	�е�ַ���ã���X����
	TFT_SEND_DATA_16b(x1+TFT_Ptr.X_Offset);
	TFT_SEND_DATA_16b(x2+TFT_Ptr.X_Offset);

	TFT_SEND_CMD(0x2b);			//	�е�ַ���ã���Y����
	TFT_SEND_DATA_16b(y1+TFT_Ptr.Y_Offset);
	TFT_SEND_DATA_16b(y2+TFT_Ptr.Y_Offset);

	TFT_SEND_CMD(0x2c);			//	��ʼд���Դ棬��Ҫ��ʾ����ɫ����
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_SetColor
*
*	��ڲ���:	Color - Ҫ��ʾ����ɫ��ʾ����0x0000FF ��ʾ��ɫ
*
*	��������:	�˺����������û��ʵ���ɫ��������ʾ�ַ������㻭�ߡ���ͼ����ɫ
*
*	˵    ��:	1. Ϊ�˷����û�ʹ���Զ�����ɫ����ڲ��� Color ʹ��24λ RGB888����ɫ��ʽ���û����������ɫ��ʽ��ת��
*					2. 24λ����ɫ�У��Ӹ�λ����λ�ֱ��Ӧ R��G��B  3����ɫͨ��
*
*****************************************************************************************************************************************/

void LCD_SetColor(uint32_t Color)
{
	uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0; //������ɫͨ����ֵ

	Red_Value   = (uint16_t)((Color&0x00F80000)>>8);   // ת���� 16λ ��RGB565��ɫ
	Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
	Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);

	TFT_Ptr.Color = (uint16_t)(Red_Value | Green_Value | Blue_Value);  // ����ɫд��ȫ��LCD����		
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_SetBackColor
*
*	��ڲ���:	Color - Ҫ��ʾ����ɫ��ʾ����0x0000FF ��ʾ��ɫ
*
*	��������:	���ñ���ɫ,�˺������������Լ���ʾ�ַ��ı���ɫ
*
*	˵    ��:	1. Ϊ�˷����û�ʹ���Զ�����ɫ����ڲ��� Color ʹ��24λ RGB888����ɫ��ʽ���û����������ɫ��ʽ��ת��
*					2. 24λ����ɫ�У��Ӹ�λ����λ�ֱ��Ӧ R��G��B  3����ɫͨ��
*
*****************************************************************************************************************************************/

void LCD_SetBackColor(uint32_t Color)
{
	uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0; //������ɫͨ����ֵ

	Red_Value   = (uint16_t)((Color&0x00F80000)>>8);   // ת���� 16λ ��RGB565��ɫ
	Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
	Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);

	TFT_Ptr.BkgColor = (uint16_t)(Red_Value | Green_Value | Blue_Value);	// ����ɫд��ȫ��LCD����			   	
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_SetDirection
*
*	��ڲ���:	direction - Ҫ��ʾ�ķ���
*
*	��������:	����Ҫ��ʾ�ķ���
*
*	˵    ��:   1. ��������� Direction_H ��Direction_V ��Direction_H_Flip ��Direction_V_Flip        
*              2. ʹ��ʾ�� LCD_DisplayDirection(Direction_H) ����������Ļ������ʾ
*
*****************************************************************************************************************************************/

void LCD_SetDirection(uint8_t direction)
{
	TFT_Ptr.Direction = direction;    // д��ȫ��LCD����

   if( direction == Direction_H )   // ������ʾ
   {
      TFT_SEND_CMD(0x36);    		// �Դ���ʿ��� ָ��������÷����Դ�ķ�ʽ
      TFT_SEND_DATA(0x70);        // ������ʾ
      TFT_Ptr.X_Offset   = 20;             // ���ÿ���������ƫ����
      TFT_Ptr.Y_Offset   = 0;   
      TFT_Ptr.Width      = LCD_Height;		// ���¸�ֵ������
      TFT_Ptr.Height     = LCD_Width;		
   }
   else if( direction == Direction_V )
   {
      TFT_SEND_CMD(0x36);    		// �Դ���ʿ��� ָ��������÷����Դ�ķ�ʽ
      TFT_SEND_DATA(0x00);        // ��ֱ��ʾ
      TFT_Ptr.X_Offset   = 0;              // ���ÿ���������ƫ����
      TFT_Ptr.Y_Offset   = 20;     
      TFT_Ptr.Width      = LCD_Width;		// ���¸�ֵ������
      TFT_Ptr.Height     = LCD_Height;						
   }
   else if( direction == Direction_H_Flip )
   {
      TFT_SEND_CMD(0x36);   			 // �Դ���ʿ��� ָ��������÷����Դ�ķ�ʽ
      TFT_SEND_DATA(0xA0);         // ������ʾ�������·�ת��RGB���ظ�ʽ
      TFT_Ptr.X_Offset   = 20;              // ���ÿ���������ƫ����
      TFT_Ptr.Y_Offset   = 0;      
      TFT_Ptr.Width      = LCD_Height;		 // ���¸�ֵ������
      TFT_Ptr.Height     = LCD_Width;				
   }
   else if( direction == Direction_V_Flip )
   {
      TFT_SEND_CMD(0x36);    		// �Դ���ʿ��� ָ��������÷����Դ�ķ�ʽ
      TFT_SEND_DATA(0xC0);        // ��ֱ��ʾ �������·�ת��RGB���ظ�ʽ
      TFT_Ptr.X_Offset   = 0;              // ���ÿ���������ƫ����
      TFT_Ptr.Y_Offset   = 20;     
      TFT_Ptr.Width      = LCD_Width;		// ���¸�ֵ������
      TFT_Ptr.Height     = LCD_Height;				
   }   
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_SetAsciiFont
*
*	��ڲ���:	*fonts - Ҫ���õ�ASCII����
*
*	��������:	����ASCII���壬��ѡ��ʹ�� 3216/2412/2010/1608/1206 ���ִ�С������
*
*	˵    ��:	1. ʹ��ʾ�� LCD_SetAsciiFont(&ASCII_Font24) �������� 2412�� ASCII����
*					2. �����ģ����� lcd_fonts.c 			
*
*****************************************************************************************************************************************/

void LCD_SetAsciiFont(pFONT *Asciifonts)
{
  LCD_AsciiFonts = Asciifonts;
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_Clear
*
*	��������:	������������LCD���Ϊ LCD.BkgColor ����ɫ
*
*	˵    ��:	���� LCD_SetBackColor() ����Ҫ����ı���ɫ���ٵ��øú�����������
*
*****************************************************************************************************************************************/

void LCD_Clear(void)
{
   LCD_SetAddress(0,0,TFT_Ptr.Width-1,TFT_Ptr.Height-1);	// ��������
	
	TFT_DC_D;     // ����ָ��ѡ�� ��������ߵ�ƽ�������δ��� ����	
  LCD_CSL;  //����cs���ţ���ʼ���� 
	// �޸�Ϊ16λ���ݿ�ȣ�д�����ݸ���Ч�ʣ�����Ҫ���	
   LCD_SPI.Init.DataSize 	= SPI_DATASIZE_16BIT;   //	16λ���ݿ��
   HAL_SPI_Init(&LCD_SPI);		
	
   LCD_SPI_Transmit(&LCD_SPI, TFT_Ptr.BkgColor, TFT_Ptr.Width * TFT_Ptr.Height) ;   // ��������
   LCD_CSH;  //����cs���ţ��������
	// �Ļ�8λ���ݿ�ȣ���Ϊָ��Ͳ������ݶ��ǰ���8λ�����
	 LCD_SPI.Init.DataSize 	= SPI_DATASIZE_8BIT;    //	8λ���ݿ��
   HAL_SPI_Init(&LCD_SPI);
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_ClearRect
*
*	��ڲ���:	x - ��ʼˮƽ����
*					y - ��ʼ��ֱ����
*					width  - Ҫ�������ĺ��򳤶�
*					height - Ҫ��������������
*
*	��������:	�ֲ�������������ָ��λ�ö�Ӧ���������Ϊ LCD.BkgColor ����ɫ
*
*	˵    ��:	1. ���� LCD_SetBackColor() ����Ҫ����ı���ɫ���ٵ��øú�����������
*				   2. ʹ��ʾ�� LCD_ClearRect( 10, 10, 100, 50) ���������(10,10)��ʼ�ĳ�100��50������
*
*****************************************************************************************************************************************/

void LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
   LCD_SetAddress( x, y, x+width-1, y+height-1);	// ��������	
	
	TFT_DC_D;     // ����ָ��ѡ�� ��������ߵ�ƽ�������δ��� ����	
  LCD_CSL;  //����NSS���ţ���ʼ����
// �޸�Ϊ16λ���ݿ�ȣ�д�����ݸ���Ч�ʣ�����Ҫ���	
   LCD_SPI.Init.DataSize 	= SPI_DATASIZE_16BIT;   //	16λ���ݿ��
   HAL_SPI_Init(&LCD_SPI);		
	
   LCD_SPI_Transmit(&LCD_SPI, TFT_Ptr.BkgColor, width*height) ;  // ��������
  LCD_CSH;  //����NSS���ţ���ʼ����
// �Ļ�8λ���ݿ�ȣ���Ϊָ��Ͳ������ݶ��ǰ���8λ�����
	LCD_SPI.Init.DataSize 	= SPI_DATASIZE_8BIT;    //	8λ���ݿ��
   HAL_SPI_Init(&LCD_SPI);

}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_DrawPoint
*
*	��ڲ���:	x - ��ʼˮƽ����
*					y - ��ʼ��ֱ����
*					color  - Ҫ���Ƶ���ɫ��ʹ�� 24λ RGB888 ����ɫ��ʽ���û����������ɫ��ʽ��ת��
*
*	��������:	��ָ���������ָ����ɫ�ĵ�
*
*	˵    ��:	ʹ��ʾ�� LCD_DrawPoint( 10, 10, 0x0000FF) ��������(10,10)������ɫ�ĵ�
*
*****************************************************************************************************************************************/

void LCD_DrawPoint(uint16_t x,uint16_t y,uint32_t color)
{
	LCD_SetAddress(x,y,x,y);	//	�������� 

	TFT_SEND_DATA_16b(color)	;
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_DisplayChar
*
*	��ڲ���:	x - ��ʼˮƽ����
*					y - ��ʼ��ֱ����
*					c  - ASCII�ַ�
*
*	��������:	��ָ��������ʾָ�����ַ�
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetAsciiFont(&ASCII_Font24) ����Ϊ 2412��ASCII����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0x000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayChar( 10, 10, 'a') ��������(10,10)��ʾ�ַ� 'a'
*
*****************************************************************************************************************************************/

void LCD_DisplayChar(uint16_t x, uint16_t y,uint8_t c)
{
	uint16_t  index = 0, counter = 0 ,i = 0, w = 0;		// ��������
   uint8_t   disChar;		//�洢�ַ��ĵ�ַ

	c = c - 32; 	// ����ASCII�ַ���ƫ��

	for(index = 0; index < LCD_AsciiFonts->Sizes; index++)	
	{
		disChar = LCD_AsciiFonts->pTable[c*LCD_AsciiFonts->Sizes + index]; //��ȡ�ַ���ģֵ
		for(counter = 0; counter < 8; counter++)
		{ 
			if(disChar & 0x01)	
			{		
            LCD_Buff[i] =  TFT_Ptr.Color;			// ��ǰģֵ��Ϊ0ʱ��ʹ�û���ɫ���
			}
			else		
			{		
            LCD_Buff[i] = TFT_Ptr.BkgColor;		//����ʹ�ñ���ɫ���Ƶ�
			}
			disChar >>= 1;
			i++;
         w++;
 			if( w == LCD_AsciiFonts->Width ) // ���д������ݴﵽ���ַ���ȣ����˳���ǰѭ��
			{								   // ������һ�ַ���д��Ļ���
				w = 0;
				break;
			}        
		}	
	}		
   LCD_SetAddress( x, y, x+LCD_AsciiFonts->Width-1, y+LCD_AsciiFonts->Height-1);	   // ��������	
   LCD_WriteBuff(LCD_Buff,LCD_AsciiFonts->Width*LCD_AsciiFonts->Height);          // д���Դ�
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_DisplayString
*
*	��ڲ���:	x - ��ʼˮƽ����
*					y - ��ʼ��ֱ����
*					p - ASCII�ַ������׵�ַ
*
*	��������:	��ָ��������ʾָ�����ַ���
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetAsciiFont(&ASCII_Font24) ����Ϊ 2412��ASCII����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0x0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0x000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayString( 10, 10, "fendou") ������ʼ����Ϊ(10,10)�ĵط���ʾ�ַ���"fendou"
*
*****************************************************************************************************************************************/

void LCD_DisplayString( uint16_t x, uint16_t y, char *p) 
{  
	while ((x < TFT_Ptr.Width) && (*p != 0))	//�ж���ʾ�����Ƿ񳬳���ʾ�������ַ��Ƿ�Ϊ���ַ�
	{
		 LCD_DisplayChar( x,y,*p);
		 x += LCD_AsciiFonts->Width; //��ʾ��һ���ַ�
		 p++;	//ȡ��һ���ַ���ַ
	}
}

/****************************************************************************************************************************************
*	�� �� ��:	LCD_SetTextFont
*
*	��ڲ���:	*fonts - Ҫ���õ��ı�����
*
*	��������:	�����ı����壬�������ĺ�ASCII�ַ���
*
*	˵    ��:	1. ��ѡ��ʹ�� 3232/2424/2020/1616/1212 ���ִ�С���������壬
*						���Ҷ�Ӧ������ASCII����Ϊ 3216/2412/2010/1608/1206
*					2. �����ģ����� lcd_fonts.c 
*					3. �����ֿ�ʹ�õ���С�ֿ⣬���õ��˶�Ӧ�ĺ�����ȥȡģ
*					4. ʹ��ʾ�� LCD_SetTextFont(&CH_Font24) �������� 2424�����������Լ�2412��ASCII�ַ�����
*
*****************************************************************************************************************************************/

void LCD_SetTextFont(pFONT *fonts)
{
	LCD_CHFonts = fonts;		// ������������
	switch(fonts->Width )
	{
		case 12:	LCD_AsciiFonts = &ASCII_Font12;	break;	// ����ASCII�ַ�������Ϊ 1206
		case 16:	LCD_AsciiFonts = &ASCII_Font16;	break;	// ����ASCII�ַ�������Ϊ 1608
		case 20:	LCD_AsciiFonts = &ASCII_Font20;	break;	// ����ASCII�ַ�������Ϊ 2010	
		case 24:	LCD_AsciiFonts = &ASCII_Font24;	break;	// ����ASCII�ַ�������Ϊ 2412
		case 32:	LCD_AsciiFonts = &ASCII_Font32;	break;	// ����ASCII�ַ�������Ϊ 3216		
		default: break;
	}
}
/******************************************************************************************************************************************
*	�� �� ��:	LCD_DisplayChinese
*
*	��ڲ���:	x - ��ʼˮƽ����
*					y - ��ʼ��ֱ����
*					pText - �����ַ�
*
*	��������:	��ָ��������ʾָ���ĵ��������ַ�
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetTextFont(&CH_Font24) ����Ϊ 2424�����������Լ�2412��ASCII�ַ�����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0xff000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayChinese( 10, 10, "��") ��������(10,10)��ʾ�����ַ�"��"
*
*****************************************************************************************************************************************/

void LCD_DisplayChinese(uint16_t x, uint16_t y, char *pText) 
{
	uint16_t  i=0,index = 0, counter = 0;	// ��������
	uint16_t  addr;	// ��ģ��ַ
   uint8_t   disChar;	//��ģ��ֵ
	uint16_t  Xaddress = 0; //ˮƽ����

	while(1)
	{		
		// �Ա������еĺ��ֱ��룬���Զ�λ�ú�����ģ�ĵ�ַ		
		if ( *(LCD_CHFonts->pTable + (i+1)*LCD_CHFonts->Sizes + 0)==*pText && *(LCD_CHFonts->pTable + (i+1)*LCD_CHFonts->Sizes + 1)==*(pText+1) )	
		{   
			addr=i;	// ��ģ��ַƫ��
			break;
		}				
		i+=2;	// ÿ�������ַ�����ռ���ֽ�

		if(i >= LCD_CHFonts->Table_Rows)	break;	// ��ģ�б�������Ӧ�ĺ���	
	}	
	i=0;
	for(index = 0; index <LCD_CHFonts->Sizes; index++)
	{	
		disChar = *(LCD_CHFonts->pTable + (addr)*LCD_CHFonts->Sizes + index);	// ��ȡ��Ӧ����ģ��ַ
		
		for(counter = 0; counter < 8; counter++)
		{ 
			if(disChar & 0x01)	
			{		
            LCD_Buff[i] =  TFT_Ptr.Color;			// ��ǰģֵ��Ϊ0ʱ��ʹ�û���ɫ���
			}
			else		
			{		
            LCD_Buff[i] = TFT_Ptr.BkgColor;		// ����ʹ�ñ���ɫ���Ƶ�
			}
         i++;
			disChar >>= 1;
			Xaddress++;  //ˮƽ�����Լ�
			
			if( Xaddress == LCD_CHFonts->Width ) 	//	���ˮƽ����ﵽ���ַ���ȣ����˳���ǰѭ��
			{														//	������һ�еĻ���
				Xaddress = 0;
				break;
			}
		}	
	}	
   LCD_SetAddress( x, y, x+LCD_CHFonts->Width-1, y+LCD_CHFonts->Height-1);	   // ��������	
   LCD_WriteBuff(LCD_Buff,LCD_CHFonts->Width*LCD_CHFonts->Height);            // д���Դ�
}

/*****************************************************************************************************************************************
*	�� �� ��:	LCD_DisplayText
*
*	��ڲ���:	x - ��ʼˮƽ����
*					y - ��ʼ��ֱ����
*					pText - �ַ�����������ʾ���Ļ���ASCII�ַ�
*
*	��������:	��ָ��������ʾָ�����ַ���
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetTextFont(&CH_Font24) ����Ϊ 2424�����������Լ�2412��ASCII�ַ�����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0xff000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayChinese( 10, 10, "�ܶ�STM32") ��������(10,10)��ʾ�ַ���"�ܶ�STM32"
*
*******************************************************/

void LCD_DisplayText(uint16_t x, uint16_t y, char *pText) 
{  
 	
	while(*pText != 0)	// �ж��Ƿ�Ϊ���ַ�
	{
		if(*pText<=0x7F)	// �ж��Ƿ�ΪASCII��
		{
			LCD_DisplayChar(x,y,*pText);	// ��ʾASCII
			x+=LCD_AsciiFonts->Width;				// ˮƽ���������һ���ַ���
			pText++;								// �ַ�����ַ+1
		}
		else					// ���ַ�Ϊ����
		{			
			LCD_DisplayChinese(x,y,pText);	// ��ʾ����
			x+=LCD_CHFonts->Width;				// ˮƽ���������һ���ַ���
			pText+=2;								// �ַ�����ַ+2�����ֵı���Ҫ2�ֽ�
		}
	}	
}

/*****************************************************************************************************************************************
*	�� �� ��:	LCD_DisplayNumber
*
*	��ڲ���:	x - ��ʼˮƽ����
*					y - ��ʼ��ֱ����
*					number - Ҫ��ʾ������,��Χ�� -2147483648~2147483647 ֮��
*					len - ���ֵ�λ�������λ������len��������ʵ�ʳ�������������Ҫ��ʾ��������Ԥ��һ��λ�ķ�����ʾ�ռ�
*
*	��������:	��ָ��������ʾָ������������
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetAsciiFont(&ASCII_Font24) ����Ϊ��ASCII�ַ�����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0x0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0x000000) ����Ϊ��ɫ�ı���ɫ
*			
*						
*****************************************************************************************************************************************/

void  LCD_DisplayNumber( uint16_t x, uint16_t y, int32_t number, uint8_t len) 
{  
	char   Number_Buffer[15];				// ���ڴ洢ת������ַ���

	
	sprintf( Number_Buffer , "%0.*d",len, number );	// �� number ת�����ַ�����������ʾ		
	
	
	LCD_DisplayString( x, y,(char *)Number_Buffer) ;  // ��ת���õ����ַ�����ʾ����
	
}

/***************************************************************************************************************************************
*	�� �� ��:	LCD_DisplayDecimals
*
*	��ڲ���:	x - ��ʼˮƽ����
*					y - ��ʼ��ֱ����
*					decimals - Ҫ��ʾ������, double��ȡֵ1.7 x 10^��-308��~ 1.7 x 10^��+308����������ȷ��׼ȷ����Чλ��Ϊ15~16λ
*
*       			len - ������������λ��������С����͸��ţ�����ʵ�ʵ���λ��������ָ������λ��������ʵ�ʵ��ܳ���λ�����
*							ʾ��1��С�� -123.123 ��ָ�� len <=8 �Ļ�����ʵ���ճ���� -123.123
*							ʾ��2��С�� -123.123 ��ָ�� len =10 �Ļ�����ʵ�����   -123.123(����ǰ����������ո�λ) 
*							ʾ��3��С�� -123.123 ��ָ�� len =10 �Ļ��������ú��� LCD_ShowNumMode() ����Ϊ���0ģʽʱ��ʵ����� -00123.123 
*
*					decs - Ҫ������С��λ������С����ʵ��λ��������ָ����С��λ����ָ���Ŀ�������������
*							 ʾ����1.12345 ��ָ�� decs Ϊ4λ�Ļ�����������Ϊ1.1235
*
*	��������:	��ָ��������ʾָ���ı���������С��
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetAsciiFont(&ASCII_Font24) ����Ϊ��ASCII�ַ�����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0x0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0x000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayDecimals( 10, 10, a, 5, 3) ��������(10,10)��ʾ�ֱ���a,�ܳ���Ϊ5λ�����б���3λС��
*						
*****************************************************************************************************************************************/

void  LCD_DisplayDecimals( uint16_t x, uint16_t y, double decimals, uint8_t len, uint8_t decs) 
{  
	char  Number_Buffer[20];				// ���ڴ洢ת������ַ���
	
	
		sprintf( Number_Buffer , "%0*.*lf",len,decs, decimals );	// �� number ת�����ַ�����������ʾ		

	
	LCD_DisplayString( x, y,(char *)Number_Buffer) ;	// ��ת���õ����ַ�����ʾ����
}


/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawLine
*
*	��ڲ���: x1 - ��� ˮƽ����
*			 	 y1 - ��� ��ֱ����
*
*				 x2 - �յ� ˮƽ����
*            y2 - �յ� ��ֱ����
*
*	��������: ������֮�仭��
*
*	˵    ��: �ú�����ֲ��ST�ٷ������������
*						 
*****************************************************************************************************************************************/

#define ABS(X)  ((X) > 0 ? (X) : -(X))    

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;

	deltax = ABS(x2 - x1);        /* The difference between the x's */
	deltay = ABS(y2 - y1);        /* The difference between the y's */
	x = x1;                       /* Start x off at the first pixel */
	y = y1;                       /* Start y off at the first pixel */

	if (x2 >= x1)                 /* The x-values are increasing */
	{
	 xinc1 = 1;
	 xinc2 = 1;
	}
	else                          /* The x-values are decreasing */
	{
	 xinc1 = -1;
	 xinc2 = -1;
	}

	if (y2 >= y1)                 /* The y-values are increasing */
	{
	 yinc1 = 1;
	 yinc2 = 1;
	}
	else                          /* The y-values are decreasing */
	{
	 yinc1 = -1;
	 yinc2 = -1;
	}

	if (deltax >= deltay)         /* There is at least one x-value for every y-value */
	{
	 xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
	 yinc2 = 0;                  /* Don't change the y for every iteration */
	 den = deltax;
	 num = deltax / 2;
	 numadd = deltay;
	 numpixels = deltax;         /* There are more x-values than y-values */
	}
	else                          /* There is at least one y-value for every x-value */
	{
	 xinc2 = 0;                  /* Don't change the x for every iteration */
	 yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
	 den = deltay;
	 num = deltay / 2;
	 numadd = deltax;
	 numpixels = deltay;         /* There are more y-values than x-values */
	}
	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
	 LCD_DrawPoint(x,y,TFT_Ptr.Color);             /* Draw the current pixel */
	 num += numadd;              /* Increase the numerator by the top of the fraction */
	 if (num >= den)             /* Check if numerator >= denominator */
	 {
		num -= den;               /* Calculate the new numerator value */
		x += xinc1;               /* Change the x as appropriate */
		y += yinc1;               /* Change the y as appropriate */
	 }
	 x += xinc2;                 /* Change the x as appropriate */
	 y += yinc2;                 /* Change the y as appropriate */
	}  
}

/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawLine_V
*
*	��ڲ���: x - ˮƽ����
*			 	 y - ��ֱ����
*				 height - ��ֱ���
*
*	��������: ��ָ��λ�û���ָ������� ��ֱ ��
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����		
*            3. ���ֻ�ǻ���ֱ���ߣ�����ʹ�ô˺������ٶȱ� LCD_DrawLine ��ܶ�
*  ���ܲ��ԣ�
*****************************************************************************************************************************************/

void LCD_DrawLine_V(uint16_t x, uint16_t y, uint16_t height)
{
   uint16_t i ; // ��������

	for (i = 0; i < height; i++)
	{
       LCD_Buff[i] =  TFT_Ptr.Color;  // д�뻺����
   }   
   LCD_SetAddress( x, y, x, y+height-1);	     // ��������	

   LCD_WriteBuff(LCD_Buff,height);          // д���Դ�
}

/****************************************************************************
*	�� �� ��: LCD_DrawLine_H
*
*	��ڲ���: x - ˮƽ����
*			 	 y - ��ֱ����
*				 width  - ˮƽ���
*
*	��������: ��ָ��λ�û���ָ������� ˮƽ ��
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����		
*            3. ���ֻ�ǻ� ˮƽ ���ߣ�����ʹ�ô˺������ٶȱ� LCD_DrawLine ��ܶ�
*  ���ܲ��ԣ�
******************************************/

void LCD_DrawLine_H(uint16_t x, uint16_t y, uint16_t width)
{
   uint16_t i ; // ��������

	for (i = 0; i < width; i++)
	{
       LCD_Buff[i] =  TFT_Ptr.Color;  // д�뻺����
   }   
   LCD_SetAddress( x, y, x+width-1, y);	     // ��������	

   LCD_WriteBuff(LCD_Buff,width);          // д���Դ�
}
/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawRect
*
*	��ڲ���: x - ˮƽ����
*			 	 y - ��ֱ����
*			 	 width  - ˮƽ���
*				 height - ��ֱ���
*
*	��������: ��ָ��λ�û���ָ������ľ�������
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*						 
*****************************************************************************************************************************************/

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
   // ����ˮƽ��
   LCD_DrawLine_H( x,  y,  width);           
   LCD_DrawLine_H( x,  y+height-1,  width);

   // ���ƴ�ֱ��
   LCD_DrawLine_V( x,  y,  height);
   LCD_DrawLine_V( x+width-1,  y,  height);
}


/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawCircle
*
*	��ڲ���: x - Բ�� ˮƽ����
*			 	 y - Բ�� ��ֱ����
*			 	 r  - �뾶
*
*	��������: ������ (x,y) ���ư뾶Ϊ r ��Բ������
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*
*****************************************************************************************************************************************/

void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r)
{
	int Xadd = -r, Yadd = 0, err = 2-2*r, e2;
	do {   

		LCD_DrawPoint(x-Xadd,y+Yadd,TFT_Ptr.Color);
		LCD_DrawPoint(x+Xadd,y+Yadd,TFT_Ptr.Color);
		LCD_DrawPoint(x+Xadd,y-Yadd,TFT_Ptr.Color);
		LCD_DrawPoint(x-Xadd,y-Yadd,TFT_Ptr.Color);
		
		e2 = err;
		if (e2 <= Yadd) {
			err += ++Yadd*2+1;
			if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
		}
		if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);   
}


/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawEllipse
*
*	��ڲ���: x - Բ�� ˮƽ����
*			 	 y - Բ�� ��ֱ����
*			 	 r1  - ˮƽ����ĳ���
*				 r2  - ��ֱ����ĳ���
*
*	��������: ������ (x,y) ����ˮƽ����Ϊ r1 ��ֱ����Ϊ r2 ����Բ����
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*
*****************************************************************************************************************************************/

void LCD_DrawEllipse(int x, int y, int r1, int r2)
{
  int Xadd = -r1, Yadd = 0, err = 2-2*r1, e2;
  float K = 0, rad1 = 0, rad2 = 0;
   
  rad1 = r1;
  rad2 = r2;
  
  if (r1 > r2)
  { 
    do {
      K = (float)(rad1/rad2);
		 
		LCD_DrawPoint(x-Xadd,y+(uint16_t)(Yadd/K),TFT_Ptr.Color);
		LCD_DrawPoint(x+Xadd,y+(uint16_t)(Yadd/K),TFT_Ptr.Color);
		LCD_DrawPoint(x+Xadd,y-(uint16_t)(Yadd/K),TFT_Ptr.Color);
		LCD_DrawPoint(x-Xadd,y-(uint16_t)(Yadd/K),TFT_Ptr.Color);     
		 
      e2 = err;
      if (e2 <= Yadd) {
        err += ++Yadd*2+1;
        if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
      }
      if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);
  }
  else
  {
    Yadd = -r2; 
    Xadd = 0;
    do { 
      K = (float)(rad2/rad1);

		LCD_DrawPoint(x-(uint16_t)(Xadd/K),y+Yadd,TFT_Ptr.Color);
		LCD_DrawPoint(x+(uint16_t)(Xadd/K),y+Yadd,TFT_Ptr.Color);
		LCD_DrawPoint(x+(uint16_t)(Xadd/K),y-Yadd,TFT_Ptr.Color);
		LCD_DrawPoint(x-(uint16_t)(Xadd/K),y-Yadd,TFT_Ptr.Color);  
		 
      e2 = err;
      if (e2 <= Xadd) {
        err += ++Xadd*3+1;
        if (-Yadd == Xadd && e2 <= Yadd) e2 = 0;
      }
      if (e2 > Yadd) err += ++Yadd*3+1;     
    }
    while (Yadd <= 0);
  }
}

/***************************************************************************************************************************************
*	�� �� ��: LCD_FillCircle
*
*	��ڲ���: x - Բ�� ˮƽ����
*			 	 y - Բ�� ��ֱ����
*			 	 r  - �뾶
*
*	��������: ������ (x,y) ���뾶Ϊ r ��Բ������
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*
*****************************************************************************************************************************************/

void LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r)
{
  int32_t  D;    /* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (r << 1);
  
  CurX = 0;
  CurY = r;
  
  while (CurX <= CurY)
  {
    if(CurY > 0) 
    { 
      LCD_DrawLine_V(x - CurX, y - CurY,2*CurY);
      LCD_DrawLine_V(x + CurX, y - CurY,2*CurY);
    }
    
    if(CurX > 0) 
    {
		// LCD_DrawLine(x - CurY, y - CurX,x - CurY,y - CurX + 2*CurX);
		// LCD_DrawLine(x + CurY, y - CurX,x + CurY,y - CurX + 2*CurX); 	

      LCD_DrawLine_V(x - CurY, y - CurX,2*CurX);
      LCD_DrawLine_V(x + CurY, y - CurX,2*CurX);
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
  LCD_DrawCircle(x, y, r);  
}

/***************************************************************************************************************************************
*	�� �� ��: LCD_FillRect
*
*	��ڲ���: x - ˮƽ����
*			 	 y - ��ֱ����
*			 	 width  - ˮƽ���
*				 height -��ֱ���
*
*	��������: ������ (x,y) ���ָ�������ʵ�ľ���
*
*	˵    ��: Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*						 
*****************************************************************************************************************************************/

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
   LCD_SetAddress( x, y, x+width-1, y+height-1);	// ��������	
	
	TFT_DC_D;     // ����ָ��ѡ�� ��������ߵ�ƽ�������δ��� ����	
LCD_CSL;  //����NSS���ţ���ʼ����
// �޸�Ϊ16λ���ݿ�ȣ�д�����ݸ���Ч�ʣ�����Ҫ���	
   LCD_SPI.Init.DataSize 	= SPI_DATASIZE_16BIT;   //	16λ���ݿ��
   HAL_SPI_Init(&LCD_SPI);		
	
   LCD_SPI_Transmit(&LCD_SPI, TFT_Ptr.Color, width*height) ;
LCD_CSH;  //����NSS���ţ���ʼ����
// �Ļ�8λ���ݿ�ȣ���Ϊָ��Ͳ������ݶ��ǰ���8λ�����
	LCD_SPI.Init.DataSize 	= SPI_DATASIZE_8BIT;    //	8λ���ݿ��
   HAL_SPI_Init(&LCD_SPI);
}


/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawImage
*
*	��ڲ���: x - ��ʼˮƽ����
*				 y - ��ʼ��ֱ����
*			 	 width  - ͼƬ��ˮƽ���
*				 height - ͼƬ�Ĵ�ֱ���
*				*pImage - ͼƬ���ݴ洢�����׵�ַ
*
*	��������: ��ָ�����괦��ʾͼƬ
*
*	˵    ��: 1.Ҫ��ʾ��ͼƬ��Ҫ���Ƚ���ȡģ����ϤͼƬ�ĳ��ȺͿ��
*            2.ʹ�� LCD_SetColor() �������û���ɫ��LCD_SetBackColor() ���ñ���ɫ
*						 
*****************************************************************************************************************************************/

void 	LCD_DrawImage(uint16_t x,uint16_t y,uint16_t width,uint16_t height,const uint8_t *pImage) 
{  
   uint8_t   disChar;	         // ��ģ��ֵ
	uint16_t  Xaddress = x;       // ˮƽ����
 	uint16_t  Yaddress = y;       // ��ֱ����  
	uint16_t  i=0,j=0,m=0;        // ��������
	uint16_t  BuffCount = 0;      // ����������
   uint16_t  Buff_Height = 0;    // ������������

// ��Ϊ��������С���ޣ���Ҫ�ֶ��д��
   Buff_Height = (sizeof(LCD_Buff)/2) / height;    // ���㻺�����ܹ�д��ͼƬ�Ķ�����

	for(i = 0; i <height; i++)             // ѭ������д��
	{
		for(j = 0; j <(float)width/8; j++)  
		{
			disChar = *pImage;

			for(m = 0; m < 8; m++)
			{ 
				if(disChar & 0x01)	
				{		
               LCD_Buff[BuffCount] =  TFT_Ptr.Color;			// ��ǰģֵ��Ϊ0ʱ��ʹ�û���ɫ���
				}
				else		
				{		
				   LCD_Buff[BuffCount] = TFT_Ptr.BkgColor;		//����ʹ�ñ���ɫ���Ƶ�
				}
				disChar >>= 1;     // ģֵ��λ
				Xaddress++;        // ˮƽ�����Լ�
				BuffCount++;       // ����������       
				if( (Xaddress - x)==width ) // ���ˮƽ����ﵽ���ַ���ȣ����˳���ǰѭ��,������һ�еĻ���		
				{											 
					Xaddress = x;				                 
					break;
				}
			}	
			pImage++;			
		}
      if( BuffCount == Buff_Height*width  )  // �ﵽ�������������ɵ��������ʱ
      {
         BuffCount = 0; // ������������0

         LCD_SetAddress( x, Yaddress , x+width-1, Yaddress+Buff_Height-1);	// ��������	
         LCD_WriteBuff(LCD_Buff,width*Buff_Height);          // д���Դ�     

         Yaddress = Yaddress+Buff_Height;    // ������ƫ�ƣ���ʼд����һ��������
      }     
      if( (i+1)== height ) // �������һ��ʱ
      {
         LCD_SetAddress( x, Yaddress , x+width-1,i+y);	   // ��������	
         LCD_WriteBuff(LCD_Buff,width*(i+1+y-Yaddress));    // д���Դ�     
      }
	}	
}


/***************************************************************************************************************************************
*	�� �� ��: LCD_CopyBuffer
*
*	��ڲ���: x - ��ʼˮƽ����
*				 y - ��ʼ��ֱ����
*			 	 width  - Ŀ�������ˮƽ���
*				 height - Ŀ������Ĵ�ֱ���
*				*pImage - ���ݴ洢�����׵�ַ
*
*	��������: ��ָ�����괦��ֱ�ӽ����ݸ��Ƶ���Ļ���Դ�
*
*	˵    ��: �������ƺ�������������ֲ LVGL ���߽�����ͷ�ɼ���ͼ����ʾ����
*						 
*****************************************************************************************************************************************/

void	LCD_CopyBuffer(uint16_t x, uint16_t y,uint16_t width,uint16_t height,uint16_t *DataBuff)
{
	
	LCD_SetAddress(x,y,x+width-1,y+height-1);
  // �޸�Ϊ16λ���ݿ�ȣ�д�����ݸ���Ч�ʣ�����Ҫ���	
	LCD_SPI.Init.DataSize 	= SPI_DATASIZE_16BIT;   //	16λ���ݿ��
  HAL_SPI_Init(&LCD_SPI);	
	TFT_DC_D;     // ����ָ��ѡ�� ��������ߵ�ƽ�������δ��� ����	
  LCD_CSL;  //����NSS���ţ���ʼ���� 
	
	LCD_SPI_TransmitBuffer(&LCD_SPI, DataBuff,width * height) ;
	LCD_CSH;  //����NSS���ţ���ʼ����
//	HAL_SPI_Transmit(&hspi5, (uint8_t *)DataBuff, (x2-x1+1) * (y2-y1+1), 1000) ;
	
// �Ļ�8λ���ݿ�ȣ���Ϊָ��Ͳ������ݶ��ǰ���8λ�����
	LCD_SPI.Init.DataSize 	= SPI_DATASIZE_8BIT;    //	8λ���ݿ��
   HAL_SPI_Init(&LCD_SPI);		
	
}

/******************************
*
* ���¼��������޸���HAL�Ŀ⺯����Ŀ����Ϊ��SPI�������ݲ������ݳ��ȵ�д�룬��������������ٶ�
*
********************************/


/**
  * @brief Handle SPI Communication Timeout.
  * @param hspi: pointer to a SPI_HandleTypeDef structure that contains
  *              the configuration information for SPI module.
  * @param Flag: SPI flag to check
  * @param Status: flag state to check
  * @param Timeout: Timeout duration
  * @param Tickstart: Tick start value
  * @retval HAL status
  */
HAL_StatusTypeDef LCD_SPI_WaitOnFlagUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Flag, FlagStatus Status,
                                                    uint32_t Tickstart, uint32_t Timeout)
{
   /* Wait until flag is set */
   while ((__HAL_SPI_GET_FLAG(hspi, Flag) ? SET : RESET) == Status)
   {
      /* Check for the Timeout */
      if ((((HAL_GetTick() - Tickstart) >=  Timeout) && (Timeout != HAL_MAX_DELAY)) || (Timeout == 0U))
      {
         return HAL_TIMEOUT;
      }
   }
   return HAL_OK;
}


/**
 * @brief  Close Transfer and clear flags.
 * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
 *               the configuration information for SPI module.
 * @retval HAL_ERROR: if any error detected
 *         HAL_OK: if nothing detected
 */
 void LCD_SPI_CloseTransfer(SPI_HandleTypeDef *hspi)
{
  uint32_t itflag = hspi->Instance->SR;

  __HAL_SPI_CLEAR_EOTFLAG(hspi);
  __HAL_SPI_CLEAR_TXTFFLAG(hspi);

  /* Disable SPI peripheral */
  __HAL_SPI_DISABLE(hspi);

  /* Disable ITs */
  __HAL_SPI_DISABLE_IT(hspi, (SPI_IT_EOT | SPI_IT_TXP | SPI_IT_RXP | SPI_IT_DXP | SPI_IT_UDR | SPI_IT_OVR | SPI_IT_FRE | SPI_IT_MODF));

  /* Disable Tx DMA Request */
  CLEAR_BIT(hspi->Instance->CFG1, SPI_CFG1_TXDMAEN | SPI_CFG1_RXDMAEN);

  /* Report UnderRun error for non RX Only communication */
  if (hspi->State != HAL_SPI_STATE_BUSY_RX)
  {
    if ((itflag & SPI_FLAG_UDR) != 0UL)
    {
      SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_UDR);
      __HAL_SPI_CLEAR_UDRFLAG(hspi);
    }
  }

  /* Report OverRun error for non TX Only communication */
  if (hspi->State != HAL_SPI_STATE_BUSY_TX)
  {
    if ((itflag & SPI_FLAG_OVR) != 0UL)
    {
      SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_OVR);
      __HAL_SPI_CLEAR_OVRFLAG(hspi);
    }
  }

  /* SPI Mode Fault error interrupt occurred -------------------------------*/
  if ((itflag & SPI_FLAG_MODF) != 0UL)
  {
    SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_MODF);
    __HAL_SPI_CLEAR_MODFFLAG(hspi);
  }

  /* SPI Frame error interrupt occurred ------------------------------------*/
  if ((itflag & SPI_FLAG_FRE) != 0UL)
  {
    SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FRE);
    __HAL_SPI_CLEAR_FREFLAG(hspi);
  }

  hspi->TxXferCount = (uint16_t)0UL;
  hspi->RxXferCount = (uint16_t)0UL;
}


/**
  * @brief  רΪ��Ļ�������޸ģ�����Ҫ��������ɫ��������
  * @param  hspi   : spi�ľ��
  * @param  pData  : Ҫд�������
  * @param  Size   : ���ݴ�С
  * @retval HAL status
  */

HAL_StatusTypeDef LCD_SPI_Transmit(SPI_HandleTypeDef *hspi,uint16_t pData, uint32_t Size)
{
   uint32_t    tickstart;  
   uint32_t    Timeout = 1000;      // ��ʱ�ж�
   uint32_t    LCD_pData_32bit;     // ��32λ����ʱ������
   uint32_t    LCD_TxDataCount;     // �������
   HAL_StatusTypeDef errorcode = HAL_OK;

	/* Check Direction parameter */
	assert_param(IS_SPI_DIRECTION_2LINES_OR_1LINE_2LINES_TXONLY(hspi->Init.Direction));

	/* Process Locked */
	__HAL_LOCK(hspi);

	/* Init tickstart for timeout management*/
	tickstart = HAL_GetTick();

	if (hspi->State != HAL_SPI_STATE_READY)
	{
		errorcode = HAL_BUSY;
		__HAL_UNLOCK(hspi);
		return errorcode;
	}

	if ( Size == 0UL)
	{
		errorcode = HAL_ERROR;
		__HAL_UNLOCK(hspi);
		return errorcode;
	}

	/* Set the transaction information */
	hspi->State       = HAL_SPI_STATE_BUSY_TX;
	hspi->ErrorCode   = HAL_SPI_ERROR_NONE;

	LCD_TxDataCount   = Size;                // ��������ݳ���
	LCD_pData_32bit   = (pData<<16)|pData ;  // ��32λ����ʱ���ϲ�2�����ص����ɫ  

	/*Init field not used in handle to zero */
	hspi->pRxBuffPtr  = NULL;
	hspi->RxXferSize  = (uint16_t) 0UL;
	hspi->RxXferCount = (uint16_t) 0UL;
	hspi->TxISR       = NULL;
	hspi->RxISR       = NULL;

	/* Configure communication direction : 1Line */
	if (hspi->Init.Direction == SPI_DIRECTION_1LINE)
	{
		SPI_1LINE_TX(hspi);
	}

// ��ʹ��Ӳ�� TSIZE ���ƣ��˴�����Ϊ0���������ƴ�������ݳ���
	MODIFY_REG(hspi->Instance->CR2, SPI_CR2_TSIZE, 0);

	/* Enable SPI peripheral */
	__HAL_SPI_ENABLE(hspi);

	if (hspi->Init.Mode == SPI_MODE_MASTER)
	{
		 /* Master transfer start */
		 SET_BIT(hspi->Instance->CR1, SPI_CR1_CSTART);
	}

	/* Transmit data in 16 Bit mode */
	while (LCD_TxDataCount > 0UL)
	{
		/* Wait until TXP flag is set to send data */
		if (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_TXP))
		{
			if ((hspi->TxXferCount > 1UL) && (hspi->Init.FifoThreshold > SPI_FIFO_THRESHOLD_01DATA))
			{
				*((__IO uint32_t *)&hspi->Instance->TXDR) = (uint32_t )LCD_pData_32bit;
				LCD_TxDataCount -= (uint16_t)2UL;
			}
			else
			{
				*((__IO uint16_t *)&hspi->Instance->TXDR) =  (uint16_t )pData;
				LCD_TxDataCount--;
			}
		}
		else
		{
			/* Timeout management */
			if ((((HAL_GetTick() - tickstart) >=  Timeout) && (Timeout != HAL_MAX_DELAY)) || (Timeout == 0U))
			{
				/* Call standard close procedure with error check */
				LCD_SPI_CloseTransfer(hspi);

				/* Process Unlocked */
				__HAL_UNLOCK(hspi);

				SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_TIMEOUT);
				hspi->State = HAL_SPI_STATE_READY;
				return HAL_ERROR;
			}
		}
	}

	if (LCD_SPI_WaitOnFlagUntilTimeout(hspi, SPI_SR_TXC, RESET, tickstart, Timeout) != HAL_OK)
	{
		SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
	}

	SET_BIT((hspi)->Instance->CR1 , SPI_CR1_CSUSP); // �������SPI����
	/* �ȴ�SPI���� */
	if (LCD_SPI_WaitOnFlagUntilTimeout(hspi, SPI_FLAG_SUSP, RESET, tickstart, Timeout) != HAL_OK)
	{
		SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
	}
	LCD_SPI_CloseTransfer(hspi);   /* Call standard close procedure with error check */

	SET_BIT((hspi)->Instance->IFCR , SPI_IFCR_SUSPC);  // ��������־λ


	/* Process Unlocked */
	__HAL_UNLOCK(hspi);

	hspi->State = HAL_SPI_STATE_READY;

	if (hspi->ErrorCode != HAL_SPI_ERROR_NONE)
	{
		return HAL_ERROR;
	}
	return errorcode;
}

/**
  * @brief  רΪ����д�������޸ģ�ʹ֮���޳��ȵĴ�������
  * @param  hspi   : spi�ľ��
  * @param  pData  : Ҫд�������
  * @param  Size   : ���ݴ�С
  * @retval HAL status
  */
HAL_StatusTypeDef LCD_SPI_TransmitBuffer (SPI_HandleTypeDef *hspi, uint16_t *pData, uint32_t Size)
{
   uint32_t    tickstart;  
   uint32_t    Timeout = 1000;      // ��ʱ�ж�
   uint32_t    LCD_TxDataCount;     // �������
   HAL_StatusTypeDef errorcode = HAL_OK;

	/* Check Direction parameter */
	assert_param(IS_SPI_DIRECTION_2LINES_OR_1LINE_2LINES_TXONLY(hspi->Init.Direction));

	/* Process Locked */
	__HAL_LOCK(hspi);

	/* Init tickstart for timeout management*/
	tickstart = HAL_GetTick();

	if (hspi->State != HAL_SPI_STATE_READY)
	{
		errorcode = HAL_BUSY;
		__HAL_UNLOCK(hspi);
		return errorcode;
	}

	if ( Size == 0UL)
	{
		errorcode = HAL_ERROR;
		__HAL_UNLOCK(hspi);
		return errorcode;
	}

	/* Set the transaction information */
	hspi->State       = HAL_SPI_STATE_BUSY_TX;
	hspi->ErrorCode   = HAL_SPI_ERROR_NONE;

	LCD_TxDataCount   = Size;                // ��������ݳ���

	/*Init field not used in handle to zero */
	hspi->pRxBuffPtr  = NULL;
	hspi->RxXferSize  = (uint16_t) 0UL;
	hspi->RxXferCount = (uint16_t) 0UL;
	hspi->TxISR       = NULL;
	hspi->RxISR       = NULL;

	/* Configure communication direction : 1Line */
	if (hspi->Init.Direction == SPI_DIRECTION_1LINE)
	{
		SPI_1LINE_TX(hspi);
	}

// ��ʹ��Ӳ�� TSIZE ���ƣ��˴�����Ϊ0���������ƴ�������ݳ���
	MODIFY_REG(hspi->Instance->CR2, SPI_CR2_TSIZE, 0);

	/* Enable SPI peripheral */
	__HAL_SPI_ENABLE(hspi);

	if (hspi->Init.Mode == SPI_MODE_MASTER)
	{
		 /* Master transfer start */
		 SET_BIT(hspi->Instance->CR1, SPI_CR1_CSTART);
	}

	/* Transmit data in 16 Bit mode */
	while (LCD_TxDataCount > 0UL)
	{
		/* Wait until TXP flag is set to send data */
		if (__HAL_SPI_GET_FLAG(hspi, SPI_FLAG_TXP))
		{
			if ((LCD_TxDataCount > 1UL) && (hspi->Init.FifoThreshold > SPI_FIFO_THRESHOLD_01DATA))
			{
				*((__IO uint32_t *)&hspi->Instance->TXDR) = *((uint32_t *)pData);
				pData += 2;
				LCD_TxDataCount -= 2;
			}
			else
			{
				*((__IO uint16_t *)&hspi->Instance->TXDR) = *((uint16_t *)pData);
				pData += 1;
				LCD_TxDataCount--;
			}
		}
		else
		{
			/* Timeout management */
			if ((((HAL_GetTick() - tickstart) >=  Timeout) && (Timeout != HAL_MAX_DELAY)) || (Timeout == 0U))
			{
				/* Call standard close procedure with error check */
				LCD_SPI_CloseTransfer(hspi);

				/* Process Unlocked */
				__HAL_UNLOCK(hspi);

				SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_TIMEOUT);
				hspi->State = HAL_SPI_STATE_READY;
				return HAL_ERROR;
			}
		}
	}

	if (LCD_SPI_WaitOnFlagUntilTimeout(hspi, SPI_SR_TXC, RESET, tickstart, Timeout) != HAL_OK)
	{
		SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
	}

	SET_BIT((hspi)->Instance->CR1 , SPI_CR1_CSUSP); // �������SPI����
	/* �ȴ�SPI���� */
	if (LCD_SPI_WaitOnFlagUntilTimeout(hspi, SPI_FLAG_SUSP, RESET, tickstart, Timeout) != HAL_OK)
	{
		SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
	}
	LCD_SPI_CloseTransfer(hspi);   /* Call standard close procedure with error check */

	SET_BIT((hspi)->Instance->IFCR , SPI_IFCR_SUSPC);  // ��������־λ


	/* Process Unlocked */
	__HAL_UNLOCK(hspi);

	hspi->State = HAL_SPI_STATE_READY;

	if (hspi->ErrorCode != HAL_SPI_ERROR_NONE)
	{
		return HAL_ERROR;
	}
	return errorcode;
}




