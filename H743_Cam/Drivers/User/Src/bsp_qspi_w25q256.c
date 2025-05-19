

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_qspi.h"
#include "bsp_qspi_W25Q256.h"


/* 
    W25Q256JV有256块，每块有16个扇区，每个扇区Sector有16页，每页有128字节，共计16MB
*/


/* QSPI引脚和时钟相关配置宏定义 */

#define QSPI_CS_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOG_CLK_ENABLE()//注意修改启动的时钟源
#define QSPI_CLK_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOF_CLK_ENABLE()//注意修改启动的时钟源
#define QSPI_BK1_D0_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()//注意修改启动的时钟源
#define QSPI_BK1_D1_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()//注意修改启动的时钟源
#define QSPI_BK1_D2_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()//注意修改启动的时钟源
#define QSPI_BK1_D3_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOF_CLK_ENABLE()//注意修改启动的时钟源

#define QSPI_FORCE_RESET()              __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()            __HAL_RCC_QSPI_RELEASE_RESET()

#define QSPI_CS_PIN                     GPIO_PIN_6 //QSPI_CS
#define QSPI_CS_GPIO_PORT               GPIOG
#define QSPI_CS_GPIO_AF                 GPIO_AF10_QUADSPI//有坑，自行比较CubeMX生成的正确指向

#define QSPI_CLK_PIN                    GPIO_PIN_10 //QSPI_CLK
#define QSPI_CLK_GPIO_PORT              GPIOF
#define QSPI_CLK_GPIO_AF                GPIO_AF9_QUADSPI//有坑，自行比较CubeMX生成的正确指向

#define QSPI_BK1_D0_PIN                 GPIO_PIN_8 //QSPI_BK1_D0
#define QSPI_BK1_D0_GPIO_PORT           GPIOF
#define QSPI_BK1_D0_GPIO_AF             GPIO_AF10_QUADSPI//有坑，自行比较CubeMX生成的正确指向

#define QSPI_BK1_D1_PIN                 GPIO_PIN_9 //QSPI_BK1_D1
#define QSPI_BK1_D1_GPIO_PORT           GPIOF
#define QSPI_BK1_D1_GPIO_AF             GPIO_AF10_QUADSPI//有坑，自行比较CubeMX生成的正确指向

#define QSPI_BK1_D2_PIN                 GPIO_PIN_7 //QSPI_BK1_D2
#define QSPI_BK1_D2_GPIO_PORT           GPIOF
#define QSPI_BK1_D2_GPIO_AF             GPIO_AF9_QUADSPI//有坑，自行比较CubeMX生成的正确指向

#define QSPI_BK1_D3_PIN                 GPIO_PIN_6 //QSPI_BK1_D3
#define QSPI_BK1_D3_GPIO_PORT           GPIOF
#define QSPI_BK1_D3_GPIO_AF             GPIO_AF9_QUADSPI//有坑，自行比较CubeMX生成的正确指向


/* 供本文件调用的全局变量 */
QSPI_HandleTypeDef hqspi;

/*
*********************************************************************************************************
*    函 数 名: bsp_InitQSPI_W25Q256
*    功能说明: QSPI Flash硬件初始化，配置基本参数
*    形    参: 无
*    返 回 值: 0 表示成功， 1 表示失败
*********************************************************************************************************
*/
uint8_t bsp_InitQSPI_W25Q256(void)
{
    uint32_t i;
    char *p;
    
    /* 将句柄手动清零，防止作为全局变量的时候没有清零 */
    p = (char *)&hqspi;
    for (i = 0; i < sizeof(QSPI_HandleTypeDef); i++)
    {
        *p++ = 0;
    }
  
    /* 复位QSPI */
    hqspi.Instance = QUADSPI;   
    
    if (HAL_QSPI_DeInit(&hqspi) != HAL_OK)
    {
        return 1;
    }

    /* 设置时钟速度，QSPI clock = 240MHz / (ClockPrescaler+1) = 120MHz  W25Q256JV这款芯片最高支持133MHZ */
    hqspi.Init.ClockPrescaler     = 2-1;//2分频
    /* 设置FIFO阀值，范围1 - 32 */
    hqspi.Init.FifoThreshold      = 32;
    /* 
        QUADSPI在FLASH驱动信号后过半个CLK周期才对FLASH驱动的数据采样。
        在外部信号延迟时，这有利于推迟数据采样。
    */
    hqspi.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    /* Flash大小是2^(FlashSize + 1) = 2^24 = 16MB  这里FlashSize不用减1，调试过程中发现-1会内存映射失败*/
    hqspi.Init.FlashSize          = QSPI_FLASH_SIZE; //QSPI_FLASH_SIZE - 1; 2020-03-04, 需要扩大一倍，否则内存映射方位最后1个地址时，会异常
    /* 命令之间的CS片选至少保持1个时钟周期的高电平，保险起见，给它5个周期 */
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;
    /*
       MODE0: 表示片选信号空闲期间，CLK时钟信号是低电平
       MODE3: 表示片选信号空闲期间，CLK时钟信号是高电平
    */
    hqspi.Init.ClockMode         = QSPI_CLOCK_MODE_0;
    /* QSPI有两个BANK，这里使用的BANK1 */
    hqspi.Init.FlashID           = QSPI_FLASH_ID_1;
    /* V7开发板仅使用了BANK1，这里是禁止双BANK */
    hqspi.Init.DualFlash         = QSPI_DUALFLASH_DISABLE;
    /* 初始化配置QSPI */
    if (HAL_QSPI_Init(&hqspi) != HAL_OK)return 1;//失败则返回1
    return 0;//成功返回0
}

/*
*********************************************************************************************************
*    函 数 名: HAL_QSPI_MspInit
*    功能说明: QSPI底层初始化，被HAL_QSPI_Init调用的底层函数
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)//下面的引脚初始化，建议直接使用CubeMX生成的代码！！！
{
    GPIO_InitTypeDef GPIO_InitStruct;
/* 使能QPSI时钟  */
    __HAL_RCC_QSPI_CLK_ENABLE();
/* 复位时钟接口 */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();    
/* 使能GPIO时钟 */
	
    QSPI_CS_GPIO_CLK_ENABLE();
    QSPI_CLK_GPIO_CLK_ENABLE();
    QSPI_BK1_D0_GPIO_CLK_ENABLE();
    QSPI_BK1_D1_GPIO_CLK_ENABLE();
    QSPI_BK1_D2_GPIO_CLK_ENABLE();
    QSPI_BK1_D3_GPIO_CLK_ENABLE();
	
	
/* QSPI CS GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//这句代码不能少
    GPIO_InitStruct.Alternate = QSPI_CS_GPIO_AF;//有坑
    HAL_GPIO_Init(QSPI_CS_GPIO_PORT, &GPIO_InitStruct);
		
/* QSPI CLK GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_CLK_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//这句代码不能少
    GPIO_InitStruct.Alternate = QSPI_CLK_GPIO_AF;//有坑
    HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &GPIO_InitStruct);
		
/* QSPI D0 GPIO pin 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_BK1_D0_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//这句代码不能少
    GPIO_InitStruct.Alternate = QSPI_BK1_D0_GPIO_AF;//有坑
    HAL_GPIO_Init(QSPI_BK1_D0_GPIO_PORT, &GPIO_InitStruct);
		
/* QSPI D1 GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_BK1_D1_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//这句代码不能少
    GPIO_InitStruct.Alternate = QSPI_BK1_D1_GPIO_AF;//有坑
    HAL_GPIO_Init(QSPI_BK1_D1_GPIO_PORT, &GPIO_InitStruct);
		
/* QSPI D2 GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_BK1_D2_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//这句代码不能少
    GPIO_InitStruct.Alternate = QSPI_BK1_D2_GPIO_AF;//有坑
    HAL_GPIO_Init(QSPI_BK1_D2_GPIO_PORT, &GPIO_InitStruct);
		
/* QSPI D3 GPIO 引脚配置 */
    GPIO_InitStruct.Pin = QSPI_BK1_D3_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//这句代码不能少
    GPIO_InitStruct.Alternate = QSPI_BK1_D3_GPIO_AF;//有坑
    HAL_GPIO_Init(QSPI_BK1_D3_GPIO_PORT, &GPIO_InitStruct);		
}

/*
*********************************************************************************************************
*    函 数 名: HAL_QSPI_MspDeInit
*    功能说明: QSPI底层复位，被HAL_QSPI_Init调用的底层函数
*    形    参: hqspi QSPI_HandleTypeDef类型句柄
*    返 回 值: 无
*********************************************************************************************************
*/
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi)
{
    /* 复位QSPI引脚 */
    HAL_GPIO_DeInit(QSPI_CS_GPIO_PORT, QSPI_CS_PIN);
    HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D0_GPIO_PORT, QSPI_BK1_D0_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D1_GPIO_PORT, QSPI_BK1_D1_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D2_GPIO_PORT, QSPI_BK1_D2_PIN);
    HAL_GPIO_DeInit(QSPI_BK1_D3_GPIO_PORT, QSPI_BK1_D3_PIN);

    /* 复位QSPI */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();

    /* 关闭QSPI时钟 */
    __HAL_RCC_QSPI_CLK_DISABLE();
}



/*-----------------------------以上是硬件初始化！！----------------------------------以上是硬件初始化！！-------------------------*/
/*-----------------------------以上是硬件初始化！！----------------------------------以上是硬件初始化！！-------------------------*/


//在STM32H750VBT6_QSPI_W25Q256_TEST文件与STM32H750VBT6_QSPI_W25Q256_USE文件，上述代码都是直接HAL库生成
//但是在下载算法中不可以这样！！










/*----------------------以下是软件备用函数(6个函数)！！-----------------------以下是软件备用函数(6个函数)！！--------------------*/
/*----------------------以下是软件备用函数(6个函数)！！-----------------------以下是软件备用函数(6个函数)！！--------------------*/




/*
*********************************************************************************************************
*    函 数 名: QSPI_Erase_Bluck_64K
*    功能说明: 擦除指定的扇区，扇区大小64KB
*    形    参: _uiSectorAddr : 扇区地址，以64KB为单位的地址
*    返 回 值: 无
*********************************************************************************************************
*/
uint8_t QSPI_Erase_Bluck_64K(uint32_t address)
{
	
    QSPI_CommandTypeDef          sCommand = {0};
    /* 写使能 */
    if(QSPI_WriteEnable()!=HAL_OK)return 1;//无法写使能，返回1
    /* 基本配置 */
    sCommand.InstructionMode     = QSPI_INSTRUCTION_1_LINE;         /* 1线方式发送指令 */
    sCommand.AddressSize         = QSPI_ADDRESS_32_BITS;            /* 32位地址 */
    sCommand.AlternateByteMode   = QSPI_ALTERNATE_BYTES_NONE;       /* 无交替字节 */
    sCommand.DdrMode             = QSPI_DDR_MODE_DISABLE;           /* W25Q256JV不支持DDR */
    sCommand.DdrHoldHalfCycle    = QSPI_DDR_HHC_ANALOG_DELAY;       /* DDR模式，数据输出延迟 */
    sCommand.SIOOMode            = QSPI_SIOO_INST_EVERY_CMD;        /* 每次传输都发指令 */
    /* 擦除配置 */
    sCommand.Instruction         = CMD_ERASE_BLOCK_64K;             //对应手册上的指令为: Block Erase (64KB) 
    sCommand.DummyCycles         = 0;                               /* 无需空周期 */
    sCommand.AddressMode         = QSPI_ADDRESS_1_LINE;
    sCommand.DataMode            = QSPI_DATA_NONE;                  /* 无需发送数据 */
    sCommand.NbData 					   = 0; 	                            /* 地址发送是1线方式 */
    sCommand.Address             = address;                         /* 扇区首地址，保证是4KB整数倍 */

	 
    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY)!= HAL_OK)return 1;
    /* 等待编程结束 */

	 return QSPI_AutoPollingMemReady();//返回等待结果！！！
	 
}

/*
*********************************************************************************************************
*    函 数 名: QSPI_EraseChip
*    功能说明: 擦除整片
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
uint8_t QSPI_EraseChip(void)
{
    QSPI_CommandTypeDef           sCommand = {0};

    /* 写使能 */
    if(QSPI_WriteEnable()!=HAL_OK)return 1;

    /* 基本配置 */
    sCommand.InstructionMode      = QSPI_INSTRUCTION_1_LINE;       /* 1线方式发送指令 */
    sCommand.AddressSize          = QSPI_ADDRESS_32_BITS;          /* 32位地址 */
    sCommand.AlternateByteMode    = QSPI_ALTERNATE_BYTES_NONE;     /* 无交替字节 */
    sCommand.DdrMode              = QSPI_DDR_MODE_DISABLE;         /* W25Q256JV不支持DDR */
    sCommand.DdrHoldHalfCycle     = QSPI_DDR_HHC_ANALOG_DELAY;     /* DDR模式，数据输出延迟 */
    sCommand.SIOOMode             = QSPI_SIOO_INST_EVERY_CMD;      /* 每次传输都发指令 */
    /* 擦除配置 */
    sCommand.Instruction          = CMD_ERASE_CHIP;                /* 32bit地址方式的扇区擦除命令，扇区大小64KB,对应手册上的指令为: Chip Erase*/
    sCommand.AddressMode          = QSPI_ADDRESS_1_LINE;           /* 地址发送是1线方式 */
    sCommand.Address              = 0;                             /* */
    sCommand.DataMode             = QSPI_DATA_NONE;                /* 无需发送数据 */
    sCommand.DummyCycles          = 0;                             /* 无需空周期 */
    if (HAL_QSPI_Command(&hqspi, &sCommand, 0)!= HAL_OK)return 1;
                                                                   /* 等待编程结束 */
    return QSPI_AutoPollingMemReady();                             //返回等待结果
}




/*
*********************************************************************************************************
*    函 数 名: QSPI_WriteBuffer
*    功能说明: 页编程，页大小128字节，任意页都可以写入
*    形    参: _pBuf : 数据源缓冲区；
*              _uiWrAddr ：目标区域首地址，即页首地址，比如0， 128, 512等。
*              _usSize ：数据个数，不能超过页面大小，范围1 - 128。
*    返 回 值: 0:成功， 1：失败
*********************************************************************************************************
*/
uint8_t QSPI_WriteBuffer(uint8_t *_pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize)
{
    QSPI_CommandTypeDef           sCommand = {0};
    /* 写使能 */
    if(QSPI_WriteEnable()!=HAL_OK)return 1;
    /* 基本配置 */
    sCommand.InstructionMode      = QSPI_INSTRUCTION_1_LINE;       	   /* 1线方式发送指令 */
    sCommand.AddressSize          = QSPI_ADDRESS_32_BITS;          	   /* 32位地址 */
    sCommand.AlternateByteMode    = QSPI_ALTERNATE_BYTES_NONE;     	   /* 无交替字节 */
    sCommand.DdrMode              = QSPI_DDR_MODE_DISABLE;         	   /* W25Q256JV不支持DDR */
    sCommand.DdrHoldHalfCycle     = QSPI_DDR_HHC_ANALOG_DELAY;     	   /* DDR模式，数据输出延迟 */
    sCommand.SIOOMode             = QSPI_SIOO_INST_ONLY_FIRST_CMD; 	   /* 仅发送一次命令 */
    /* 写序列配置 */
    sCommand.Instruction          = CMD_WRITE;                         /* 32bit地址的4线快速写入命令,对应手册上的指令是: Quad Input Page Program */
    sCommand.DummyCycles          = 0;                                 /* 不需要空周期 */
    sCommand.AddressMode          = QSPI_ADDRESS_1_LINE;               /* 4线地址方式 */
    sCommand.DataMode             = QSPI_DATA_4_LINES;                 /* 4线数据方式 */
    sCommand.NbData               = _usWriteSize;                      /* 写数据大小 */
    sCommand.Address              = _uiWriteAddr;                      /* 写入地址 */
    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK) return 1;
                                                                       /* 启动传输 */
    if (HAL_QSPI_Transmit(&hqspi, _pBuf, HAL_MAX_DELAY)!= HAL_OK)     return 1;
                                                                       /* 等待Flash页编程完毕 */
    return QSPI_AutoPollingMemReady();                                 //等待操作完成！！！
}


/*
*********************************************************************************************************
*    函 数 名: QSPI_WriteEnable
*    功能说明: 写使能
*    形    参: hqspi  QSPI_HandleTypeDef句柄。
*    返 回 值: 无
*********************************************************************************************************
*/
uint8_t QSPI_WriteEnable(void)
{
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode 		= QSPI_INSTRUCTION_1_LINE;            /*1线方式发送指令 */
    sCommand.AddressSize 			  = QSPI_ADDRESS_32_BITS;               /*32位地址 */
    sCommand.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;     	    /*无交替字节 */
    sCommand.DdrMode 				    = QSPI_DDR_MODE_DISABLE;              /*W25Q256JV不支持DDR */
    sCommand.DdrHoldHalfCycle 	= QSPI_DDR_HHC_ANALOG_DELAY;      	  /*DDR模式，数据输出延迟 */
    sCommand.SIOOMode 				  = QSPI_SIOO_INST_EVERY_CMD;    		    /*每次发送命令 */
    sCommand.Instruction 			  = CMD_WRITE_ENABLE;   				        /*写使能,对应手册上的指令为: Write Enable*/
    sCommand.DummyCycles 			  = 0;                                  /*不需要空周期*/
    sCommand.AddressMode 			  = QSPI_ADDRESS_NONE;              	  /*无地址信息*/
    sCommand.DataMode 				  = QSPI_DATA_NONE;                     /*无数据信息*/
    sCommand.NbData 				  	= 0;                             	    /*写数据大小 */
    sCommand.Address 				    = 0;  

    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_MAX_DELAY) != HAL_OK) return 1;
		
    return 0;

}

/*
*********************************************************************************************************
*    函 数 名: QSPI_AutoPollingMemReady
*    功能说明: 等待QSPI Flash就绪，主要用于Flash擦除和页编程时使用
*    形    参: hqspi  QSPI_HandleTypeDef句柄
*    返 回 值: 
*********************************************************************************************************
*/
uint8_t QSPI_AutoPollingMemReady(void)
{
	
    QSPI_CommandTypeDef           sCommand = {0};
    QSPI_AutoPollingTypeDef       sConfig  = {0};

    /* 基本配置 */
    sCommand.InstructionMode      = QSPI_INSTRUCTION_1_LINE;            /* 1线方式发送指令 */
    sCommand.AddressSize          = QSPI_ADDRESS_32_BITS;               /* 32位地址 */
    sCommand.AlternateByteMode    = QSPI_ALTERNATE_BYTES_NONE;          /* 无交替字节 */
    sCommand.DdrMode              = QSPI_DDR_MODE_DISABLE;              /* W25Q256JV不支持DDR */
    sCommand.DdrHoldHalfCycle     = QSPI_DDR_HHC_ANALOG_DELAY;          /* DDR模式，数据输出延迟 */
    sCommand.SIOOMode             = QSPI_SIOO_INST_EVERY_CMD;           /* 每次传输都发指令 */
    /* 读取状态*/
    sCommand.Instruction          = CMD_GET_REG1;                       /* 读取状态命令,对应手册上的指令为: Read Status Register-1 */
    sCommand.AddressMode          = QSPI_ADDRESS_NONE;                  /* 无需地址 */
    sCommand.DataMode             = QSPI_DATA_1_LINE;                   /* 1线数据 */
    sCommand.DummyCycles          = 0;                                  /* 无需空周期 */

    /* 屏蔽位设置的bit0，匹配位等待bit0为0，即不断查询状态寄存器bit0，等待其为0 */
    
    sConfig.Match           		  = 0x00;
    sConfig.Mask            		  = 0x01;
	  sConfig.Interval        		  = 0x10;
    sConfig.StatusBytesSize 		  = 1;
	  sConfig.MatchMode       		  = QSPI_MATCH_MODE_AND;
    sConfig.AutomaticStop   		  = QSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_QSPI_AutoPolling(&hqspi, &sCommand, &sConfig, HAL_MAX_DELAY) != HAL_OK) return 1;
    return 0;
}


/*
*********************************************************************************************************
*    函 数 名: QSPI_MemoryMapped
*    功能说明: QSPI内存映射，地址 0x90000000
*    形    参: 无
*    返 回 值: 无
*********************************************************************************************************
*/
uint8_t QSPI_MemoryMapped(void)
{
    QSPI_CommandTypeDef 			  	  	 s_command        = {0};
    QSPI_MemoryMappedTypeDef 		  		 s_mem_mapped_cfg = {0};
    

    /* 基本配置 */
    s_command.InstructionMode 	  		 = QSPI_INSTRUCTION_1_LINE;     /* 1线方式发送指令 */ 
    s_command.AddressSize 			  		 = QSPI_ADDRESS_32_BITS;        /* 32位地址 */
    s_command.AlternateByteMode    	   = QSPI_ALTERNATE_BYTES_NONE;   /* 无交替字节 */
    s_command.DdrMode 				  		   = QSPI_DDR_MODE_DISABLE;       /* W25Q256JV不支持DDR */
    s_command.DdrHoldHalfCycle 	  		 = QSPI_DDR_HHC_ANALOG_DELAY;   /* DDR模式，数据输出延迟 */
    s_command.SIOOMode 				  	  	 = QSPI_SIOO_INST_EVERY_CMD;    /* 每次传输都发指令 */
    s_command.Instruction 			  		 = CMD_READ;                    /* 对应手册上的指令是: Fast Read Quad I/O  */ 
    s_command.AddressMode 			  		 = QSPI_ADDRESS_4_LINES;        /* 4个地址线 */
    s_command.DataMode 				  	  	 = QSPI_DATA_4_LINES;           /* 4个数据线 */
    s_command.DummyCycles 			  		 = 6;                           /* 空周期 */
    /* 关闭溢出计数 */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod 		 = 0;

    
    if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK) return 1;

    return 0;
}

uint8_t QSPI_DeviceReset(void)	
{
	QSPI_CommandTypeDef s_command;	// QSPI传输配置

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;   	// 1线指令模式
	s_command.AddressMode 		 = QSPI_ADDRESS_NONE;   			// 无地址模式
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 	// 无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     	// 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 	// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 	// 每次传输数据都发送指令
	s_command.DataMode 			 = QSPI_DATA_NONE;       			// 无数据模式	
	s_command.DummyCycles 		 = 0;                     			// 空周期个数
	s_command.Instruction 		 = CMD_EnableReset;       // 执行复位使能命令

	// 发送复位使能命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return 1;			// 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (QSPI_AutoPollingMemReady() != HAL_OK)
	{
		return 1;	// 轮询等待无响应
	}

	s_command.Instruction  = CMD_ResetDevice;     // 复位器件命令    

	//发送复位器件命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return 1;		  // 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (QSPI_AutoPollingMemReady() != HAL_OK)
	{
		return 1;	// 轮询等待无响应
	}	


	s_command.Instruction  = CMD_Enter_4Byte_Mode;     // 4-byte Address Mode  

	//发送复位器件命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return 1;		  // 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (QSPI_AutoPollingMemReady() != HAL_OK)
	{
		return 1;	// 轮询等待无响应
	}	

	return 0;	// 复位成功
}

