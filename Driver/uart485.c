/*
uart5
485 使用
可惜的是stm32f103 串口5没有DMA，所以只能用中断实现


注意：对于驱动板来说，完成一个动作需要两条指令，这两条指令是否中间可以打断，还需要考虑
*/

#include "uart485.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"

/*485方向切换*/
#define OPEN_485_RECV	GPIO_ResetBits(GPIOG,GPIO_Pin_11);
#define OPEN_485_SEND	GPIO_SetBits(GPIOG,GPIO_Pin_11);
volatile uint8_t *txBuf;	//
volatile uint32_t txLen;
volatile uint32_t txCount;

#define ACK 1
static volatile uint8_t packageNum = 0;//包序号
static volatile uint8_t recv_status = 0;//接收状态
/*
接收与发送缓冲区
*/
static volatile uint8_t rx[50];
static volatile uint16_t rx_len;


/*
485串口配置
*/
void uart_485_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//这里需要打开一下GPIO口时钟，或者在统一的文件中打开，后面就不用打开了
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA
						 | RCC_APB2Periph_GPIOB 
						 | RCC_APB2Periph_GPIOC 
						 | RCC_APB2Periph_GPIOD 
						 | RCC_APB2Periph_GPIOE 
						 | RCC_APB2Periph_GPIOF 
						 | RCC_APB2Periph_GPIOG 
						 | RCC_APB2Periph_AFIO , ENABLE);
	//注意这个复用时钟其实只在 I2C1 用了下
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5 ,ENABLE);//代开uart5外设时钟
	
	//485 tx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//485_RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
	//485_DIR
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);	
	
	
	USART_DeInit (UART5);
	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);
	
	USART_ClearFlag(UART5, USART_FLAG_TC);//清发送完成标志
	USART_ClearFlag(UART5, USART_IT_IDLE);//清发送完成标志
	USART_ClearFlag(UART5, USART_IT_RXNE);//清发送完成标志
	
	USART_ITConfig(UART5,  USART_FLAG_TC, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_IDLE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_RXNE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_FE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_NE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_PE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_CTS, DISABLE); 	
	
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn; 		   //USART5接收中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   //次占优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(UART5, ENABLE);
	
	USART_GetFlagStatus(UART5, USART_FLAG_TC);
}
/*
485总线发送命令。
调用前一定要锁定总线。
*/
static uint8_t send_to_485Bus(uint8_t *tx, uint16_t tx_len)
{
	uint8_t err;
	USART_ITConfig(UART5,USART_IT_RXNE, DISABLE);
	err = UART5->SR;
	err = UART5->DR;//清洗寄存器
	
	USART_ClearFlag(UART5, USART_FLAG_TC);//清发送完成标志
	USART_ClearFlag(UART5, USART_IT_IDLE);//清发送完成标志
	USART_ClearFlag(UART5, USART_IT_RXNE);//清发送完成标志
	USART_GetFlagStatus(UART5, USART_FLAG_TC);

	rx_len = 0;
	OPEN_485_SEND; //切换为485发送
	
	txBuf = tx;
	txLen = tx_len;
	txCount = 0;
//	for(i = 0; i < tx_len; i++)
//	{
////		UART5->SR;
////		USART_SendData(UART5, tx[i]);
////		while( USART_GetFlagStatus(UART5, USART_FLAG_TC) != SET);
//		while(!USART_GetFlagStatus(UART5, USART_FLAG_TXE));
//		USART_SendData(USART5, tx[i]);	
//	}
	USART_ITConfig(UART5,USART_IT_RXNE, DISABLE);   		//关接收中断
	USART_ITConfig(UART5,USART_IT_IDLE, DISABLE);   		//关空闲中断
	USART_ITConfig(UART5,USART_IT_TC, ENABLE);   		//开启发送中断
	UART5->DR = txBuf[txCount++];	//开始发数据
//	OPEN_485_RECV;	//485方向变为接收
//	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//打开485接收中断
//	USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);//开空闲中断
	return err; 
}



/*
485中断
*/
void UART5_IRQHandler(void)
{
	uint8_t sr;
	sr = sr;
	
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)//接收到数据
	{
		sr = UART5->SR;
		USART_ClearITPendingBit(UART5, USART_IT_RXNE);
		if(rx_len == 0)//过滤第一个字节
		{
			uint8_t be = UART5->DR;
			if(be == 0xbb){
				rx[rx_len++] = be;
			}
		}else
		{
			rx[rx_len++] = UART5->DR;
		}
	}
	
	else
	if(USART_GetITStatus(UART5, USART_IT_TC) != RESET)//发送完成，每次一字节
	{
		USART_ClearITPendingBit(UART5, USART_IT_TC);//清除中断待处理位
		if(txCount < txLen)
			UART5->DR = txBuf[txCount++];
		else
		{
			USART_ITConfig(UART5, USART_IT_TC,DISABLE);//关485发送中断
			OPEN_485_RECV;	//485方向变为接收
			USART_ITConfig(UART5,USART_IT_TC, DISABLE);   		//关发送中断
			USART_ITConfig(UART5,USART_IT_RXNE, ENABLE);   		//开接收中断
			USART_ITConfig(UART5,USART_IT_IDLE, ENABLE);   		//开空闲中断

		}
	}
	
	else
	{
		if(USART_GetITStatus(UART5, USART_IT_IDLE) != RESET )
		{
			uint8_t sr = USART_ReceiveData(UART5);
			USART_ClearITPendingBit(UART5, USART_IT_IDLE);
			USART_ITConfig(UART5, USART_IT_IDLE, DISABLE);//关空闲中断
			recv_status = 1;//收到数据
		}
		if(USART_GetITStatus(UART5, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//其他错误
		{
			uint8_t sr = USART_ReceiveData(UART5);
			USART_ClearITPendingBit(UART5, USART_IT_PE | USART_IT_FE | USART_IT_NE);
		}
	}
}



/*
主板与驱动板通讯格式
包格式：
包头：0xaa	<1Byte>
包序号：0~255循环递增 <1Byte>
包长：总长度 <2Byte>
主板地址：1 <1Byte>
驱动板地址：1 <1Byte>
命令类型：1， <1Byte>
数据：（可为空） <不定长>
校验：包头~数据 的和校验 <1Byte>
*/

/*
与驱动板握手
参数：cmd-> 1,开始升级；2,不用升级直接进入用户程序；3,升级数据包;4,等待主机命令
*/
static uint8_t str[1100];
uint8_t handshake_with_driver(uint8_t cmd, uint8_t *dat, uint16_t datLen)
{
	uint8_t chk = 0, rs = FALSE;
	uint16_t i = 0, j = 0;
	
	str[i++] = 0xaa;
	str[i++] = packageNum + 1;
	str[i++] = ' ' ;
	str[i++] = ' ' ;
	str[i++] = 1;
	str[i++] = 0;
	str[i++] = cmd;
	if(cmd == UPDATA)//升级数据包
	{
		for(j = 0; j < datLen; j++)
			str[i++] = dat[j];
	}
	str[2] = (i + 1) >> 8;
	str[3] = (i + 1) & 0xff;
	
	for(j = 0; j < i; j++)//计算校验和
	{
		chk += str[j];
	}
	str[i++] = chk;
	
	//串口发送数据
	send_to_485Bus(str, i); 
	j = 0;
	while(recv_status != 1 && j++ < 300)//最长等待4s
	{
		delay_ms(10);
	}
	if(recv_status == 1)//解析收到的数据
	{
		recv_status = 0;
		if(rx[0] == 0xbb)//包头对
		{
			if(str[1] == rx[1])//流水号对
			{
				if(rx_len == (rx[2] << 8 | rx[3]))//包长对
				{
					if(rx[4] == str[4])//主板地址对
					{
						if(rx[5] == str[5])//驱动板地址对
						{
							rs = TRUE;
						}
						else
						{
							rs = 2;
							//驱动板地址不对
						}
					}
					else
					{
						//主板地址不对
						rs = 3;
					}
				}
				else
				{
					//包长不对
					rs = 4;
				}
			}
			else
			{
				//流水号不对
				rs = 5;
			}
		}
		else
		{
			//包头不对
			rs = 6;
		}
	}
	packageNum = (packageNum + 1)%255;

	return rs;
}
