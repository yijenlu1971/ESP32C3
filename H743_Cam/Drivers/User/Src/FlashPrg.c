#include "FlashOS.h"
#include "bsp_qspi_w25q256.h"

/*
*********************************************************************************************************
*	函 数 名: EraseChip
*	功能说明: 整个芯片擦除
*	形    参: 无
*	返 回 值: 0 表示成功， 1表示失败
*********************************************************************************************************
*/
int EraseChip (void) 
{	
    /* W25Q256初始化 */
    if (bsp_InitQSPI_W25Q256()!= 0)return 1;//失败则返回1
    if (QSPI_EraseChip()!= 0)      return 1;//失败则返回1
    /* 内存映射 */    
    if (QSPI_MemoryMapped() != 0)  return 1;//失败则返回1
	
                                   return 0;//成功返回0      
}

/*
*********************************************************************************************************
*	函 数 名: EraseSector
*	功能说明: 扇区擦除
*	形    参: adr 擦除地址
*	返 回 值: 无
*********************************************************************************************************
*/
int EraseSector (unsigned long adr) 
{	


    /* 地址要在操作的芯片范围内 */
    if (adr < QSPI_FLASH_MEM_ADDR || adr >= QSPI_FLASH_MEM_ADDR + QSPI_FLASH_SIZES)
			                                  return 1;
	
    adr -= QSPI_FLASH_MEM_ADDR;
    /* W25Q256初始化 */
    if (bsp_InitQSPI_W25Q256() != 0)    return 1;//失败则返回1
    /* 扇区擦除 */
    if (QSPI_Erase_Bluck_64K(adr) != 0) return 1;//失败则返回1
    /* 内存映射 */    
    if (QSPI_MemoryMapped() != 0)       return 1;//失败则返回1
    
                                        return 0;//成功返回0   
}

/*
*********************************************************************************************************
*	函 数 名: ProgramPage
*	功能说明: 页编程
*	形    参: adr 页起始地址
*             sz  页大小
*             buf 要写入的数据地址
*	返 回 值: 无
*********************************************************************************************************
*/
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) 
{
    int size;
    /* 地址要在操作的芯片范围内 */    
	if (adr < QSPI_FLASH_MEM_ADDR || adr >= QSPI_FLASH_MEM_ADDR + QSPI_FLASH_SIZES)
		                              return 1;//地址超了，非天上去了！返回1
   
    /* W25Q128初始化 */
	 if (bsp_InitQSPI_W25Q256() != 0)return 1;//QSPI硬件不过，没得搞返回1
    adr -= QSPI_FLASH_MEM_ADDR;
    size =  sz;
    /* 页编程 */
    while(size > 0)
    {
        if (QSPI_WriteBuffer(buf, adr, 256) == 1)
        {
            QSPI_MemoryMapped(); 
            return 1;//失败则返回1   
        }
        size-= 256;
        adr += 256;
        buf += 256;
    }
    
    /* 内存映射 */    
    if (QSPI_MemoryMapped() != 0)return 1;//失败则返回1
	 
                                 return 0;//成功返回0                      
}






