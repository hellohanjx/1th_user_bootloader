#include "stm32f10x_conf.h"

void CH376SPI_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC , ENABLE );	
	   
	//CH376复位引脚
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;//CH376 RST
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	//GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//GPIO_Init(GPIOC, &GPIO_InitStructure);

	//ch376 中断引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;//CH376 INT
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	SPI2_Init(); //初始化SPI2
}


/*******************************************************************************
* 函  数  名      : CH376_PORT_INIT
* 描      述      : 由于使用软件模拟SPI读写时序,所以进行初始化.
*                   如果是硬件SPI接口,那么可使用mode3(CPOL=1&CPHA=1)或
*                   mode0(CPOL=0&CPHA=0),CH376在时钟上升沿采样输入,下降沿输出,数
*                   据位是高位在前.
*******************************************************************************/
void	CH376_PORT_INIT( void )
{
	CH376_SPI_SCS = 1;  	/* 禁止SPI片选 */
	CH376_INT_WIRE = 1;   /* 默认为高电平,SPI模式3,也可以用SPI模式0,但模拟程序可能需稍做修改 */
/* 对于双向I/O引脚模拟SPI接口,那么必须在此设置SPI_SCS,SPI_SCK,SPI_SDI为输出方向,
*  SPI_SDO为输入方向 */
}

/*******************************************************************************
* 函  数  名      : xEndCH376Cmd   结束命令.
*******************************************************************************/
void	xEndCH376Cmd( void )
{ 
	CH376_SPI_SCS = 1; 	/* SPI片选无效,结束CH376命令,仅用于SPI接口方式 */
}

/*******************************************************************************
SPI输出8个位数据.    * 发送: uint8_t d:要发送的数据.
*******************************************************************************/
void	Spi376OutByte( uint8_t d )
{  
   SPI2_ReadWriteByte(d);	 
}

/*******************************************************************************
* 描      述      : SPI接收8个位数据.  uint8_t d:接收到的数据.
*******************************************************************************/
uint8_t	Spi376InByte( void )
{
/* 如果是硬件SPI接口,应该是先查询SPI状态寄存器以等待SPI字节传输完成,然后从SPI数据寄存器读出数据 */
//	while((SPI1->SR&1<<0)==0);    //RXEN=0 接收缓冲区为空						    
//	return SPI1->DR;  //返回收到的数据	
 	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET); //检查指定的SPI标志位设置与否:接受缓存非空标志位
	return SPI_I2S_ReceiveData(SPI1);
}



/*******************************************************************************
* 描      述      : 向CH376写  命令.
* 输      入      : uint8_t mCmd:要发送的命令.
*******************************************************************************/
void	xWriteCH376Cmd( uint8_t mCmd )
{
	CH376_SPI_SCS = 1;    /* 防止之前未通过xEndCH376Cmd禁止SPI片选 */
	Delay_us(10 );
	Delay_us(10 );
/* 对于双向I/O引脚模拟SPI接口,那么必须确保已经设置SPI_SCS,SPI_SCK,SPI_SDI为输出
*  方向,SPI_SDO为输入方向 */
	CH376_SPI_SCS = 0;      /* SPI片选有效 */
	Spi376OutByte( mCmd );  /* 发出命令码 */
	Delay_us(10 ); 
	Delay_us(10 ); 
	Delay_us(10 );   /* 延时1.5uS确保读写周期大于1.5uS,或者用上面一行的状态查询代替 */
	Delay_us(10 ); 
}

/*******************************************************************************
* 描      述      : 向CH376写   数据.
* 输      入      : uint8_t mData:
*                   要发送的数据.
*******************************************************************************/
void	xWriteCH376Data( uint8_t mData )
{
	Spi376OutByte( mData );
	Delay_us(10 );  /* 确保读写周期大于0.6uS */
}

/*******************************************************************************
* 函  数  名      : xReadCH376Data
* 描      述      : 从CH376读数据.
*******************************************************************************/
uint8_t	xReadCH376Data( void )
{
	uint8_t i;
	Delay_us(10);
	i = SPI2_ReadWriteByte( 0xFF );
	return( i );
}

/*******************************************************************************
* 描      述      : 查询CH376中断(INT#低电平).
* 返      回      : FALSE:无中断.       TRUE:有中断.
*******************************************************************************/
uint8_t	Query376Interrupt( void )
{
	uint8_t i;
	//如果 CH376_INT_WIRE 拉低，则返回 TRUE
	i = (CH376_INT_WIRE ? FALSE : TRUE ); 	/* 如果连接了CH376的中断引脚则直接查询中断引脚 */
	return( i ); 
}

/*******************************************************************************
* 描      述      : 初始化CH376.
* 返      回      : FALSE:无中断.  TRUE:有中断.
*******************************************************************************/
uint8_t	mInitCH376Host( void )
{
	uint8_t	res;	
	Delay_us(200);
	Delay_us(200);
	Delay_us(200);

	CH376_PORT_INIT( );           				//接口硬件初始化
	//========================
	//第1步 检查ch376连接与复位，及晶振是否起振
	//========================
	xWriteCH376Cmd( CMD11_CHECK_EXIST );  //测试单片机与CH376之间的通讯接口
	xWriteCH376Data( 0x55 );
	res = xReadCH376Data( );
//	printf("res =%02x \n",(unsigned short)res);
	xEndCH376Cmd( );
	if ( res != 0xAA ) 
		return( ERR_USB_UNKNOWN );  /* 通讯接口不正常,可能原因有:接口连接异常,其它设备影响(片选不唯一),串口波特率,一直在复位,晶振不工作 */
	
	
	//========================
	//第2步 设置模式 6为U盘，3为SD卡
	//========================
	xWriteCH376Cmd( CMD11_SET_USB_MODE ); /* 设备USB工作模式 */
	xWriteCH376Data( 0x06 ); //06H=已启用的主机方式并且自动产生SOF包  ???
	Delay_ms( 20 );
	res = xReadCH376Data( );
//	printf("res =%02x \n",(unsigned short)res);
	xEndCH376Cmd( );//停止CH376工作

	if ( res == CMD_RET_SUCCESS )  //RES=51  命令操作成功
	{
	    return( USB_INT_SUCCESS ); //USB事务或者传输操作成功 
	}
	else 
	{
	    return( ERR_USB_UNKNOWN );/* 设置模式错误 */
	}
}
/************************************ End *************************************/
