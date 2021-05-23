/*********************************************
以下是SPI模块的初始化代码，配置成主机模式，访问ch376t						  
SPI口初始化
这里针是对SPI2的初始化
*********************************************/

#include "stm32f10x_conf.h"
 

void SPI2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );//GPIOB 时钟使能
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2,  ENABLE );//SPI2 时钟使能
 
	//SPI2 MOSI MISO SCK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  	//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);						//初始化引脚
 	GPIO_SetBits(GPIOB,GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);//上拉

	//SPI2 NSS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 					//CH376 CS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB,GPIO_Pin_12);			//使能spi设备
	
	//SPI2 配置
	/* spi 四种工作模式
	Mode 0 CPOL=0, CPHA=0
	Mode 1 CPOL=0, CPHA=1
	Mode 2 CPOL=1, CPHA=0
	Mode 3 CPOL=1, CPHA=1
	CPOL 时钟极性 0:空闲高电平；1：空闲低电平
	CPHA 时钟相位(SPI在SCLK第几个边沿开始采样) 0：第1个边沿；1：第2个边沿
	*/
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;				//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	//工作在模式3
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;					//选择了串行时钟的稳态:空闲时为高电平(CPOL)
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;				//数据捕获于第1个时钟沿(CPHA)
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;						//NSS信号由软件控制
	//ch376最高支持24M，apb2 最高36M ，所以2分频可以
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//预分频为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;						//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  								//根据SPI_InitStruct中指定的参数初始化外设SPI2寄存器
 
	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
	
	SPI2_ReadWriteByte(0xff);//启动传输
}   
  

//void SPI_I2S_SendData(SPI_TypeDef* SPIx, uint16_t Data)//发送数据
//uint16_t SPI_I2S_ReceiveData(SPI_TypeDef* SPIx)//接收数据
//SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE);//查看spi传输状态




//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
uint8_t SPI2_ReadWriteByte(uint8_t TxData)//@这里个人觉得函数类型应当是 uint16_t
{		
	uint8_t retry=0;	
	uint16_t	recv;
	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //等待发送区空
	{
		retry++;
		if(retry>200)
			return 0;
	}		
	
	SPI_I2S_SendData(SPI2, TxData); //通过外设SPI2发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)//等待接收完一个byte
	{
		retry++;
		if(retry>200)
			return 0;
	}	  						    
	recv = SPI_I2S_ReceiveData(SPI2); //返回通过SPI2最近接收的数据		
	return recv;
}































