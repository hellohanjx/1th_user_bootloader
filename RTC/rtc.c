/**
  * @file    RTC.c 
  * @author  WB R&D Team - openmcu666
  * @version V1.0
  * @date    2016.05.04
  * @brief   Real-Time Clock Driver
  */
	
#include "stm32f10x_conf.h"

_calendar_obj calendar;

uint32_t THH = 0, TMM = 0, TSS = 0;

//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//year:���
//����ֵ:������ǲ�������.1,��.0,����
uint8_t Is_Leap_Year(uint16_t year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   

//����ʱ�� 
//�������ʱ��ת��Ϊ���� 
//��1970��1��1��Ϊ��׼ 
//1970~2099��Ϊ�Ϸ���� 
//����ֵ:0,�ɹ�;����:�������. 
//�·����ݱ�                         
uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�      
//ƽ����·����ڱ� 
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31}; 
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec) 
{ 
	uint16_t t; 
	uint32_t seccount=0; 
	if(syear<1970||syear>2099)
		return 1;       
	for(t=1970;t<syear;t++)             //��������ݵ�������� 
	{      
		if(Is_Leap_Year(t))
			seccount+=31622400;			//����������� 
		else 
			seccount+=31536000;         //ƽ��������� 
	} 
	smon-=1; 
	for(t=0;t<smon;t++)                	//��ǰ���·ݵ���������� 
	{    
		seccount+=(uint32_t)mon_table[t]*86400;    	//�·���������� 
		if(Is_Leap_Year(syear)&&t==1)
			seccount+=86400;					//���� 2 �·�����һ���������
	} 
	seccount+=(uint32_t)(sday-1)*86400;                //��ǰ�����ڵ����������   
	seccount+=(uint32_t)hour*3600;                     //Сʱ������ 
	seccount+=(uint32_t)min*60;                        //���������� 
	seccount+=sec;                                //�������Ӽ���ȥ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);      //ʹ��PWR ��BKP ����ʱ��    
	PWR_BackupAccessCmd(ENABLE);      				//ʹ�� RTC�ͺ󱸼Ĵ�������   
	RTC_SetCounter(seccount);                       //����RTC��������ֵ
	RTC_WaitForLastTask();                        	//�ȴ����һ�ζ�RTC�Ĵ�����д�������    
	return 0;        
} 


//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//year,month,day������������ 
//����ֵ�����ں�																						 
uint16_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// ���Ϊ21����,�������100  
	if (yearH>19)yearL+=100;
	// ����������ֻ��1900��֮���  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}			  


//�õ���ǰ��ʱ�䣬���������calendar�ṹ������
//����ֵ:0,�ɹ�;����:�������.
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0; 
	uint32_t temp=0;
	uint16_t temp1=0;
	
	timecount = RTC_GetCounter();//��ʵ���ǵõ����������ܹ������룬Ȼ��ʼ����

 	temp=timecount/86400;   //�õ�����(��������Ӧ��)
	if(daycnt!=temp)//����һ����
	{	  
		daycnt=temp;
		temp1 = 1970;	//��1970�꿪ʼ
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//������
			{
				if(temp>=366)temp-=366;//�����������
				else break;  
			}
			else 
				temp-=365;	  //ƽ�� 
			temp1++;  
		}   
		calendar.w_year=temp1;//�õ����
		temp1=0;
		while(temp>=28)//������һ����
		{
			if(Is_Leap_Year(calendar.w_year)&&temp1==1)//�����ǲ�������/2�·�
			{
				if(temp>=29)temp-=29;//�����������
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//ƽ��
				else break;
			}
			temp1++;  
		}
		calendar.w_month=temp1+1;	//�õ��·�
		calendar.w_date=temp+1;  	//�õ����� 
	}
	temp=timecount%86400;     		//�õ�������   	   
	calendar.hour=temp/3600;     	//Сʱ
	calendar.min=(temp%3600)/60; 	//����	
	calendar.sec=(temp%3600)%60; 	//����
	calendar.week = RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//��ȡ����   
	return 0;
}	 




/*
RTC����
*/
void RTC_Configuration(void)
{	
	uint32_t	i = 0;
	
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE );	//ʹ�ܵ�Դ�뱸����ʱ��
	PWR_BackupAccessCmd(ENABLE);          //�������BKP������
	BKP_DeInit();	                      //��λBKP�Ĵ���
	RCC_LSEConfig(RCC_LSE_ON);	          //����LSE������ RTC ʱ��Դ
	
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && i < 5000)	   //�ȴ�LSE����
	{
		i++;
		delay_ms(1);//������ʱ
	}
	if(i >= 5000)
	{
		//��������
		lcd_show_character(&pt_word[20]);
		delay_ms(5000);//��ʾ5s��
	}
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);//ѡ�� LSE Ϊ RTC ʱ��Դ�� 32.768 kHZ����
	RCC_RTCCLKCmd(ENABLE);	            //RTC ʱ��ʹ��
	RTC_WaitForSynchro();	            //�ȴ� RTC �� APB ͬ��
	
	RTC_WaitForLastTask();	  			//�ȴ�����RTC�Ĵ�����д�������
	RTC_EnterConfigMode();    			//��������RTC
	RTC_SetPrescaler(32767);			//����RTCʱ�ӷ�Ƶֵ32767�������Ƶ��= (32.768 KHz)/(32767+1)=1Hz(1s)
	RTC_WaitForLastTask();	  			//�ȴ�����RTC�Ĵ�����д�������
	RTC_ITConfig(RTC_IT_SEC, ENABLE);	//RTC ���ж�ʹ��
//	RTC_Set(2017,1,1,12,0,0);			//����ʱ��   
	RTC_WaitForLastTask();	  			//�ȴ�����RTC�Ĵ�����д�������
	RTC_ExitConfigMode();				//�˳�����ģʽ    
}

//==========================
//RCT �жϷ������� 
//==========================
void RTC_NVIC_Config(void)
{       
	NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}


/*
���ܣ�RTC��ʼ��

ע�⣺BootLoader ��Ҫ��rtc���в���
*/
void RTC_Init(void)
{
	//ע�⣬bkp��������ʱ��Ҳ�ǿɶ���
	//RTC�Ĵ�������Ҳ�ᱣ������
    if(BKP_ReadBackupRegister(BKP_DR1) == 0)//�����־λΪ0��˵���ǵ�һ�����û��ߵ��û����
	{
		RTC_Configuration();
		
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)       	//���POWER�Ƿ�����λ��־
		{
			lcd_show_character(&pt_word[21]);//"��Դ��λ"
		}
		else 
		if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)	 //���PIN��λ��־����
		{
			lcd_show_character(&pt_word[22]);//"���¸�λ��"
		}
	}
	else
	{
		RTC_WaitForSynchro();	                    //�ȴ�RTC�Ĵ���ͬ�����ȴ����һ�ζ�RTC�Ĵ�����д������� ��
		RTC_ITConfig(RTC_IT_SEC, ENABLE);	        //RTC ���ж�ʹ��
		RTC_WaitForLastTask();	                    //�ȴ�����RTC�Ĵ�����д�������
	}
	
	RTC_NVIC_Config();	//RTC�ж�����
	RTC_Get();    		//����ʱ��
	return ;      
}


//======================================
//rtc ʵʱʱ���ж�
//======================================
volatile uint8_t countDown; //����ʱ����

void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)//���ж�
	{
		RTC_ClearITPendingBit(RTC_IT_SEC);    		//����жϱ�־λ
		RTC_Get();									//����ʱ��
		countDown--;								//����ʱ
	}
//	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)         //�����ж� 
//	{ 
//		RTC_ClearITPendingBit(RTC_IT_ALR);      		//�������ж�         
//	}
	
	RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW); 	//�����ж��������жϱ�־λ
	RTC_WaitForLastTask();				        	//�ȴ� RTC д�������
}

