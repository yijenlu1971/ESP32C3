#ifndef __FONTS_H
#define __FONTS_H

#include <stdint.h>


// ������ؽṹ����
typedef struct _pFont
{    
	const uint8_t 		*pTable;  		//	��ģ�����ַ
	uint16_t 					Width; 		 		//	�����ַ�����ģ���
	uint16_t 					Height; 			//	�����ַ�����ģ����
	uint16_t 					Sizes;	 			//	�����ַ�����ģ���ݸ���
	uint16_t					Table_Rows;		// �ò���ֻ�к�����ģ�õ�����ʾ��ά������д�С
} pFONT;


/*------------------------------------ �������� ---------------------------------------------*/

extern	pFONT	CN_Font12 ;		//	12*12��������
extern	pFONT	CN_Font16 ;    //	16*16��������
extern	pFONT	CN_Font20 ;    //	20*20��������
extern	pFONT	CN_Font24 ;    //	24*24��������
extern	pFONT	CN_Font32 ;    //	32*32��������


/*------------------------------------ ASCII���� ---------------------------------------------*/

extern pFONT ASCII_Font32;		// 32*16��������
extern pFONT ASCII_Font24;		// 24*12��������
extern pFONT ASCII_Font20; 		// 20*10��������
extern pFONT ASCII_Font16; 		// 16*08��������
extern pFONT ASCII_Font12; 		// 12*06��������

#endif 
 
