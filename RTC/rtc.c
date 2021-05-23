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

//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//year:年份
//返回值:该年份是不是闰年.1,是.0,不是
uint8_t Is_Leap_Year(uint16_t year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   

//设置时钟 
//把输入的时钟转换为秒钟 
//以1970年1月1日为基准 
//1970~2099年为合法年份 
//返回值:0,成功;其他:错误代码. 
//月份数据表                         
uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表      
//平年的月份日期表 
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31}; 
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec) 
{ 
	uint16_t t; 
	uint32_t seccount=0; 
	if(syear<1970||syear>2099)
		return 1;       
	for(t=1970;t<syear;t++)             //把所有年份的秒钟相加 
	{      
		if(Is_Leap_Year(t))
			seccount+=31622400;			//闰年的秒钟数 
		else 
			seccount+=31536000;         //平年的秒钟数 
	} 
	smon-=1; 
	for(t=0;t<smon;t++)                	//把前面月份的秒钟数相加 
	{    
		seccount+=(uint32_t)mon_table[t]*86400;    	//月份秒钟数相加 
		if(Is_Leap_Year(syear)&&t==1)
			seccount+=86400;					//闰年 2 月份增加一天的秒钟数
	} 
	seccount+=(uint32_t)(sday-1)*86400;                //把前面日期的秒钟数相加   
	seccount+=(uint32_t)hour*3600;                     //小时秒钟数 
	seccount+=(uint32_t)min*60;                        //分钟秒钟数 
	seccount+=sec;                                //最后的秒钟加上去 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);      //使能PWR 和BKP 外设时钟    
	PWR_BackupAccessCmd(ENABLE);      				//使能 RTC和后备寄存器访问   
	RTC_SetCounter(seccount);                       //设置RTC计数器的值
	RTC_WaitForLastTask();                        	//等待最近一次对RTC寄存器的写操作完成    
	return 0;        
} 


//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//year,month,day：公历年月日 
//返回值：星期号																						 
uint16_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}			  


//得到当前的时间，结果保存在calendar结构体里面
//返回值:0,成功;其他:错误代码.
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0; 
	uint32_t temp=0;
	uint16_t temp1=0;
	
	timecount = RTC_GetCounter();//其实就是得到计数器中总共多少秒，然后开始计算

 	temp=timecount/86400;   //得到天数(秒钟数对应的)
	if(daycnt!=temp)//超过一天了
	{	  
		daycnt=temp;
		temp1 = 1970;	//从1970年开始
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//是闰年
			{
				if(temp>=366)temp-=366;//闰年的秒钟数
				else break;  
			}
			else 
				temp-=365;	  //平年 
			temp1++;  
		}   
		calendar.w_year=temp1;//得到年份
		temp1=0;
		while(temp>=28)//超过了一个月
		{
			if(Is_Leap_Year(calendar.w_year)&&temp1==1)//当年是不是闰年/2月份
			{
				if(temp>=29)temp-=29;//闰年的秒钟数
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
				else break;
			}
			temp1++;  
		}
		calendar.w_month=temp1+1;	//得到月份
		calendar.w_date=temp+1;  	//得到日期 
	}
	temp=timecount%86400;     		//得到秒钟数   	   
	calendar.hour=temp/3600;     	//小时
	calendar.min=(temp%3600)/60; 	//分钟	
	calendar.sec=(temp%3600)%60; 	//秒钟
	calendar.week = RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//获取星期   
	return 0;
}	 




/*
RTC配置
*/
void RTC_Configuration(void)
{	
	uint32_t	i = 0;
	
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE );	//使能电源与备份区时钟
	PWR_BackupAccessCmd(ENABLE);          //允许访问BKP备份域
	BKP_DeInit();	                      //复位BKP寄存器
	RCC_LSEConfig(RCC_LSE_ON);	          //开启LSE，设置 RTC 时钟源
	
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && i < 5000)	   //等待LSE起振
	{
		i++;
		delay_ms(1);//粗略延时
	}
	if(i >= 5000)
	{
		//晶振不起振
		lcd_show_character(&pt_word[20]);
		delay_ms(5000);//显示5s钟
	}
	
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);//选择 LSE 为 RTC 时钟源， 32.768 kHZ晶振
	RCC_RTCCLKCmd(ENABLE);	            //RTC 时钟使能
	RTC_WaitForSynchro();	            //等待 RTC 与 APB 同步
	
	RTC_WaitForLastTask();	  			//等待最后对RTC寄存器的写操作完成
	RTC_EnterConfigMode();    			//允许配置RTC
	RTC_SetPrescaler(32767);			//设置RTC时钟分频值32767，则计数频率= (32.768 KHz)/(32767+1)=1Hz(1s)
	RTC_WaitForLastTask();	  			//等待最后对RTC寄存器的写操作完成
	RTC_ITConfig(RTC_IT_SEC, ENABLE);	//RTC 秒中断使能
//	RTC_Set(2017,1,1,12,0,0);			//设置时间   
	RTC_WaitForLastTask();	  			//等待最后对RTC寄存器的写操作完成
	RTC_ExitConfigMode();				//退出配置模式    
}

//==========================
//RCT 中断分组设置 
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
功能：RTC初始化

注意：BootLoader 不要对rtc进行操作
*/
void RTC_Init(void)
{
	//注意，bkp区不开启时钟也是可读的
	//RTC寄存器掉电也会保存设置
    if(BKP_ReadBackupRegister(BKP_DR1) == 0)//如果标志位为0，说明是第一次配置或者电池没电了
	{
		RTC_Configuration();
		
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)       	//检查POWER是否发生复位标志
		{
			lcd_show_character(&pt_word[21]);//"电源复位"
		}
		else 
		if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)	 //检查PIN复位标志设置
		{
			lcd_show_character(&pt_word[22]);//"按下复位键"
		}
	}
	else
	{
		RTC_WaitForSynchro();	                    //等待RTC寄存器同步（等待最近一次对RTC寄存器的写操作完成 ）
		RTC_ITConfig(RTC_IT_SEC, ENABLE);	        //RTC 秒中断使能
		RTC_WaitForLastTask();	                    //等待最后对RTC寄存器的写操作完成
	}
	
	RTC_NVIC_Config();	//RTC中断设置
	RTC_Get();    		//更新时间
	return ;      
}


//======================================
//rtc 实时时钟中断
//======================================
volatile uint8_t countDown; //倒计时读秒

void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET)//秒中断
	{
		RTC_ClearITPendingBit(RTC_IT_SEC);    		//清除中断标志位
		RTC_Get();									//更新时间
		countDown--;								//倒计时
	}
//	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)         //闹钟中断 
//	{ 
//		RTC_ClearITPendingBit(RTC_IT_ALR);      		//清闹钟中断         
//	}
	
	RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW); 	//清秒中断与闹钟中断标志位
	RTC_WaitForLastTask();				        	//等待 RTC 写操作完成
}

