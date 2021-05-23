/*
uart5
485 ʹ��
��ϧ����stm32f103 ����5û��DMA������ֻ�����ж�ʵ��


ע�⣺������������˵�����һ��������Ҫ����ָ�������ָ���Ƿ��м���Դ�ϣ�����Ҫ����
*/

#include "uart485.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"

/*485�����л�*/
#define OPEN_485_RECV	GPIO_ResetBits(GPIOG,GPIO_Pin_11);
#define OPEN_485_SEND	GPIO_SetBits(GPIOG,GPIO_Pin_11);
volatile uint8_t *txBuf;	//
volatile uint32_t txLen;
volatile uint32_t txCount;

#define ACK 1
static volatile uint8_t packageNum = 0;//�����
static volatile uint8_t recv_status = 0;//����״̬
/*
�����뷢�ͻ�����
*/
static volatile uint8_t rx[50];
static volatile uint16_t rx_len;


/*
485��������
*/
void uart_485_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//������Ҫ��һ��GPIO��ʱ�ӣ�������ͳһ���ļ��д򿪣�����Ͳ��ô���
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA
						 | RCC_APB2Periph_GPIOB 
						 | RCC_APB2Periph_GPIOC 
						 | RCC_APB2Periph_GPIOD 
						 | RCC_APB2Periph_GPIOE 
						 | RCC_APB2Periph_GPIOF 
						 | RCC_APB2Periph_GPIOG 
						 | RCC_APB2Periph_AFIO , ENABLE);
	//ע���������ʱ����ʵֻ�� I2C1 ������
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5 ,ENABLE);//����uart5����ʱ��
	
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
	
	USART_ClearFlag(UART5, USART_FLAG_TC);//�巢����ɱ�־
	USART_ClearFlag(UART5, USART_IT_IDLE);//�巢����ɱ�־
	USART_ClearFlag(UART5, USART_IT_RXNE);//�巢����ɱ�־
	
	USART_ITConfig(UART5,  USART_FLAG_TC, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_IDLE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_RXNE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_FE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_NE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_PE, DISABLE); 
	USART_ITConfig(UART5,  USART_IT_CTS, DISABLE); 	
	
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn; 		   //USART5�����ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		   //��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(UART5, ENABLE);
	
	USART_GetFlagStatus(UART5, USART_FLAG_TC);
}
/*
485���߷������
����ǰһ��Ҫ�������ߡ�
*/
static uint8_t send_to_485Bus(uint8_t *tx, uint16_t tx_len)
{
	uint8_t err;
	USART_ITConfig(UART5,USART_IT_RXNE, DISABLE);
	err = UART5->SR;
	err = UART5->DR;//��ϴ�Ĵ���
	
	USART_ClearFlag(UART5, USART_FLAG_TC);//�巢����ɱ�־
	USART_ClearFlag(UART5, USART_IT_IDLE);//�巢����ɱ�־
	USART_ClearFlag(UART5, USART_IT_RXNE);//�巢����ɱ�־
	USART_GetFlagStatus(UART5, USART_FLAG_TC);

	rx_len = 0;
	OPEN_485_SEND; //�л�Ϊ485����
	
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
	USART_ITConfig(UART5,USART_IT_RXNE, DISABLE);   		//�ؽ����ж�
	USART_ITConfig(UART5,USART_IT_IDLE, DISABLE);   		//�ؿ����ж�
	USART_ITConfig(UART5,USART_IT_TC, ENABLE);   		//���������ж�
	UART5->DR = txBuf[txCount++];	//��ʼ������
//	OPEN_485_RECV;	//485�����Ϊ����
//	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//��485�����ж�
//	USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);//�������ж�
	return err; 
}



/*
485�ж�
*/
void UART5_IRQHandler(void)
{
	uint8_t sr;
	sr = sr;
	
	if(USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)//���յ�����
	{
		sr = UART5->SR;
		USART_ClearITPendingBit(UART5, USART_IT_RXNE);
		if(rx_len == 0)//���˵�һ���ֽ�
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
	if(USART_GetITStatus(UART5, USART_IT_TC) != RESET)//������ɣ�ÿ��һ�ֽ�
	{
		USART_ClearITPendingBit(UART5, USART_IT_TC);//����жϴ�����λ
		if(txCount < txLen)
			UART5->DR = txBuf[txCount++];
		else
		{
			USART_ITConfig(UART5, USART_IT_TC,DISABLE);//��485�����ж�
			OPEN_485_RECV;	//485�����Ϊ����
			USART_ITConfig(UART5,USART_IT_TC, DISABLE);   		//�ط����ж�
			USART_ITConfig(UART5,USART_IT_RXNE, ENABLE);   		//�������ж�
			USART_ITConfig(UART5,USART_IT_IDLE, ENABLE);   		//�������ж�

		}
	}
	
	else
	{
		if(USART_GetITStatus(UART5, USART_IT_IDLE) != RESET )
		{
			uint8_t sr = USART_ReceiveData(UART5);
			USART_ClearITPendingBit(UART5, USART_IT_IDLE);
			USART_ITConfig(UART5, USART_IT_IDLE, DISABLE);//�ؿ����ж�
			recv_status = 1;//�յ�����
		}
		if(USART_GetITStatus(UART5, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//��������
		{
			uint8_t sr = USART_ReceiveData(UART5);
			USART_ClearITPendingBit(UART5, USART_IT_PE | USART_IT_FE | USART_IT_NE);
		}
	}
}



/*
������������ͨѶ��ʽ
����ʽ��
��ͷ��0xaa	<1Byte>
����ţ�0~255ѭ������ <1Byte>
�������ܳ��� <2Byte>
�����ַ��1 <1Byte>
�������ַ��1 <1Byte>
�������ͣ�1�� <1Byte>
���ݣ�����Ϊ�գ� <������>
У�飺��ͷ~���� �ĺ�У�� <1Byte>
*/

/*
������������
������cmd-> 1,��ʼ������2,��������ֱ�ӽ����û�����3,�������ݰ�;4,�ȴ���������
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
	if(cmd == UPDATA)//�������ݰ�
	{
		for(j = 0; j < datLen; j++)
			str[i++] = dat[j];
	}
	str[2] = (i + 1) >> 8;
	str[3] = (i + 1) & 0xff;
	
	for(j = 0; j < i; j++)//����У���
	{
		chk += str[j];
	}
	str[i++] = chk;
	
	//���ڷ�������
	send_to_485Bus(str, i); 
	j = 0;
	while(recv_status != 1 && j++ < 300)//��ȴ�4s
	{
		delay_ms(10);
	}
	if(recv_status == 1)//�����յ�������
	{
		recv_status = 0;
		if(rx[0] == 0xbb)//��ͷ��
		{
			if(str[1] == rx[1])//��ˮ�Ŷ�
			{
				if(rx_len == (rx[2] << 8 | rx[3]))//������
				{
					if(rx[4] == str[4])//�����ַ��
					{
						if(rx[5] == str[5])//�������ַ��
						{
							rs = TRUE;
						}
						else
						{
							rs = 2;
							//�������ַ����
						}
					}
					else
					{
						//�����ַ����
						rs = 3;
					}
				}
				else
				{
					//��������
					rs = 4;
				}
			}
			else
			{
				//��ˮ�Ų���
				rs = 5;
			}
		}
		else
		{
			//��ͷ����
			rs = 6;
		}
	}
	packageNum = (packageNum + 1)%255;

	return rs;
}
