#ifndef __spi_lcd
#define __spi_lcd

#include "stm32h7xx_hal.h"
#include "usart.h"

#include "lcd_fonts.h"	


#define LCD_CSH   GPIOA->ODR |= GPIO_PIN_15;			// SPI CS
#define LCD_CSL   GPIOA->ODR &= ~GPIO_PIN_15;			// SPI CS

#ifdef STM32H750xx
#define LCD_RST_H
#define LCD_RST_L
#else
#define LCD_RST_H   GPIOC->ODR |= GPIO_PIN_13;
#define LCD_RST_L   GPIOC->ODR &= ~GPIO_PIN_13;
#endif

/*-------宏定义 -------------------------------------------*/

#define LCD_Width     240		// LCD的像素长度
#define LCD_Height    280		// LCD的像素宽度

// 显示方向参数
// 使用示例：LCD_DisplayDirection(Direction_H) 设置屏幕横屏显示
#define	Direction_H				0							//LCD横屏显示
#define	Direction_H_Flip	1		    			//LCD横屏显示,上下翻转
#define	Direction_V				2							//LCD竖屏显示 
#define	Direction_V_Flip	3				    	//LCD竖屏显示,上下翻转 



/*---------------------------------------- 常用颜色 ------------------------------------------------------

颜色值为16位色颜色数值
 
 */                                                  						
#define 	LCD_WHITE       0xFFFFFF	 //  白色
#define 	LCD_BLACK       0x000000    // 黑色
                        
#define 	LCD_BLUE        0x0000FF	 //	 蓝色
#define 	LCD_GREEN       0x00FF00    // 绿色
#define 	LCD_RED         0xFF0000    // 红色
#define 	LCD_CYAN        0x00FFFF    // 蓝绿色
#define 	LCD_MAGENTA     0xFF00FF    // 紫红色
#define 	LCD_YELLOW      0xFFFF00    // 黄色
#define 	LCD_GREY        0x2C2C2C    // 灰色
												
#define 	LIGHT_BLUE      0x8080FF    //	亮蓝色
#define 	LIGHT_GREEN     0x80FF80    //	亮绿色
#define 	LIGHT_RED       0xFF8080    //	亮红色
#define 	LIGHT_CYAN      0x80FFFF    //	亮蓝绿色
#define 	LIGHT_MAGENTA   0xFF80FF    //	亮紫红色
#define 	LIGHT_YELLOW    0xFFFF80    //	亮黄色
#define 	LIGHT_GREY      0xA3A3A3    //	亮灰色
												
#define 	DARK_BLUE       0x000080    //	暗蓝色
#define 	DARK_GREEN      0x008000    //	暗绿色
#define 	DARK_RED        0x800000    //	暗红色
#define 	DARK_CYAN       0x008080    //	暗蓝绿色
#define 	DARK_MAGENTA    0x800080    //	暗紫红色
#define 	DARK_YELLOW     0x808000    //	暗黄色
#define 	DARK_GREY       0x404040    //	暗灰色


/*-------------- 函数声明 ----------------------------------------------*/

void  SPI_LCD_Init(void);      // 液晶屏以及SPI初始化   
void  LCD_Clear(void);			   // 清屏函数
void  LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);	// 局部清屏函数

void  LCD_SetAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);	// 设置坐标		
void  LCD_SetColor(uint32_t Color); 	   		 	   //	设置画笔颜色
void  LCD_SetBackColor(uint32_t Color);    				//	设置背景颜色
void  LCD_SetDirection(uint8_t direction);	      //	设置显示方向

//---显示ASCII字符
void  LCD_SetAsciiFont(pFONT *fonts);					   					//	设置ASCII字体
void 	LCD_DisplayChar(uint16_t x, uint16_t y,uint8_t c);	//	显示单个ASCII字符
void 	LCD_DisplayString( uint16_t x, uint16_t y, char *p);//	显示ASCII字符串

//---	显示中文字符，包括ASCII码
void 	LCD_SetTextFont(pFONT *fonts);										// 设置文本字体，包括中文和ASCII字体
void 	LCD_DisplayChinese(uint16_t x, uint16_t y, char *pText);		// 显示单个汉字
void 	LCD_DisplayText(uint16_t x, uint16_t y, char *pText) ;		// 显示字符串，包括中文和ASCII字符


void  LCD_DisplayNumber( uint16_t x, uint16_t y, int32_t number,uint8_t len) ;					// 显示整数
void  LCD_DisplayDecimals( uint16_t x, uint16_t y, double number,uint8_t len,uint8_t decs);	// 显示小数

//---	2D图形函数
void  LCD_DrawPoint(uint16_t x,uint16_t y,uint32_t color);   	//画点

void  LCD_DrawLine_V(uint16_t x, uint16_t y, uint16_t height);          // 画垂直线
void  LCD_DrawLine_H(uint16_t x, uint16_t y, uint16_t width);           // 画水平线
void  LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);	// 两点之间画线

void  LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);			//画矩形
void  LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r);									//画圆
void  LCD_DrawEllipse(int x, int y, int r1, int r2);											//画椭圆

//---	区域填充函数
void  LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height);			//填充矩形
void  LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r);									//填充圆

//---	绘制单色图片
void 	LCD_DrawImage(uint16_t x,uint16_t y,uint16_t width,uint16_t height,const uint8_t *pImage)  ;

//---	批量复制函数，直接将数据复制到屏幕的显存
void	LCD_CopyBuffer(uint16_t x, uint16_t y,uint16_t width,uint16_t height,uint16_t *DataBuff);

/*--------------------------------------------- LCD其它引脚 -----------------------------------------------*/


#ifdef STM32H750xx
#define		LCD_Bkglight_OFF	HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_RESET);	// 低电平，关闭背光
#define 	LCD_Bkglight_ON		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_6, GPIO_PIN_SET);		  // 高电平，开启背光

#define		TFT_DC_C		   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_12, GPIO_PIN_RESET);	   // 低电平，指令传输 
#define 	TFT_DC_D		   HAL_GPIO_WritePin(GPIOG, GPIO_PIN_12, GPIO_PIN_SET);			 // 高电平，数据传输

#else
#define		LCD_Bkglight_OFF		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);	// 低电平，关闭背光
#define 	LCD_Bkglight_ON		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);		  // 高电平，开启背光
 
#define		TFT_DC_C		   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);	   // 低电平，指令传输 
#define 	TFT_DC_D		   HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);			 // 高电平，数据传输
#endif

#endif 




