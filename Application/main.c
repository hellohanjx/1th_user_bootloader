/*
bootloader
修改记录：
2017-7-1：
1.修改了倒计时读秒的bug，改为在rtc中断中倒计时减值
2.增加对 RA8875 7寸旺宝屏幕支持
3.注意旺宝屏不支持黑底白字，会显示乱码
*/

#include "stm32f10x_conf.h"
#include "lcd_header.h"
#include "bsp_key.h"
#include "uart485.h"

enum{init, init_complete, inser_u, ready, search_file, updata, updating_main, updating_driver, updata_complete};
#define BUFF_SIZE			1024	//缓冲区大小
#define UPADTA_SIZE		BUFF_SIZE
volatile uint8_t rd_buf[BUFF_SIZE+4];	//升级缓冲区,包含4个包序号
volatile uint16_t realCount;			//从U盘读出的文件真实字节数

volatile uint32_t JumpAddress;
pFunction Jump_To_Application;
	
extern void __set_MSP(uint32_t topOfMainStack);
extern void LOG_Init(void);
extern void LOG(char* ptr);
extern void LoadBinProgress(uint16_t progress,uint16_t color);
extern volatile uint8_t countDown;

/*
跳转到用户程序
*/
void run_to_app(void)
{
	handshake_with_driver(APP ,NULL, NULL);//通知驱动板进入app

	#if SCREEN != TM12864Z
		lcd_show_character(&pt_word[18]);//"正在启动"
	#else
		DisStr("正在启动        " ,3, 0);
	#endif
	
	Delay_ms(3000);//等待2s再跳转
	JumpAddress = *(__IO uint32_t*) (FLASH_APP_ADDR+4);
	Jump_To_Application = (pFunction) JumpAddress;         
	__set_MSP(*(__IO uint32_t*) FLASH_APP_ADDR);  //中断向量表转移
	
	Jump_To_Application();                //跳转到用户app
}
/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int main(void)
{
	volatile char str[64];
	volatile uint32_t fileSize;
	volatile uint16_t s,i;
	volatile uint8_t buf[64], TarName[64], ch376Status = init, keyValue = 0xff, rs;
	
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);//设置中断向量表偏移
	//SystemInit();//初始化STM32 时钟
	//NVIC_Configuration();   
	
	RCC_Configuration();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//设置中断优先级,无抢占中断

	key_Configuration();//初始化
	lcd_init();
	RTC_Init();
	LED_Init();
	Delay_Init();	//systick初始化 
	CH376SPI_Init();
	uart_485_config();
	Delay_Init();
	
	#if SCREEN != TM12864Z
	lcd_show_character(&pt_word[0]);//"正在启动"
	#else
	DisStr("升级请插U 盘    ", 0, 0);
	#endif

	countDown = 3;
	lcd_show_time(&pt_word[1]);//"显示当前时间"
	
	while(1)
	{
		LED_Toggle_On(1);
		Delay_ms(200);	
		LED_Toggle_Off(1);
		
		switch(ch376Status)
		{
			case init://初始态
				#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[2]);//"正在检测U盘"
				#else
					DisStr("正在检测U 盘    ", 1, 0);
				#endif
				
				s = mInitCH376Host();//初始化ch376
				
				if (s!=0x14)//ch376未准备好
				{
					if(ch376Status ++ >10)
					{
						#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[3]);//"CH376 ERROR,error Code is "
						//显示错误码
						lcd_show_num(s, pt_word[3].valx, pt_word[3].valy, pt_word[3].color, pt_word[3].background, *(pt_word[3].font), 0, 10);
						#else
						DisStr("CH376 损坏      ",1, 0);
						#endif
					}
					else
						Delay_ms(100);	//等待100ms继续检测
				}
				else
				{
					ch376Status = init_complete;
				}
			break;
				
			case init_complete://初始化成功
				/* 检查U盘是否连接,等待U盘插入,对于SD卡,
						可以由单片机直接查询SD卡座的插拔状态引脚 */
				if( CH376DiskConnect( ) != USB_INT_SUCCESS )
				{  
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[4]);//"未插入U盘 "
					#else
						DisStr("未插入U 盘    ",1, 0);
					#endif
					
					Delay_ms(100);
				}
				else
				{
					ch376Status = inser_u;//插入U盘
					
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[5]);//"检测到U盘"
					#else
						DisStr("检测到U 盘      ", 1, 0);
					#endif
					
					LED_Toggle_On(LED_USB);	//U盘灯点亮
					Delay_ms( 200 );  // 对于检测到USB设备的,最多等待100~50mS,主要针对有些MP3太慢,对于检测到USB设备
				}
				
				//倒计时
				if( !(countDown > 0 && countDown <= 3) )//
					countDown = 0;
				#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[17]);//"秒后启动"
					lcd_show_num(countDown, pt_word[17].valx, pt_word[17].valy, pt_word[17].color, pt_word[17].background, *(pt_word[17].font), 1, 2);
				#else
					DisStr("秒后启动", 3, 1);
					DisStrNum_02(countDown, 3, 0);
				#endif
				
				if(countDown == 0)
				{
					run_to_app();
				}

			break;
			
			case inser_u://插入U盘
				LED_Toggle_On(LED_USB);//启动U盘指示灯
				for ( i = 0; i < 100; i++ ) 
				{   
					delay_ms( 50 );
					s = CH376DiskMount( );  	//初始化磁盘并测试磁盘是否就绪.   
					if ( s == USB_INT_SUCCESS ) //准备好
					{
						ch376Status = ready;
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[6]);//"U盘正常"
						#else
							DisStr("U 盘正常        ", 1, 0);
						#endif

						//显示U盘信息
						i = CH376ReadBlock((unsigned char*) buf );  //如果需要,可以读取数据块CH376_CMD_DATA.DiskMountInq,返回长度
						if( i == sizeof( INQUIRY_DATA ) )  //U盘的厂商和产品信息
						{  
							buf[ i ] = 0;
							
							#if SCREEN != TM12864Z
								sprintf((char*)str, "%s", ((P_INQUIRY_DATA)buf) -> VendorIdStr );//厂商信息
								//这里显示厂商信息
								lcd_show_string((char*)str, pt_word[9].x, pt_word[9].y, pt_word[9].color, pt_word[9].background, *(pt_word[9].font));
							#endif
							
							rs = handshake_with_driver(WAIT ,NULL, NULL);//通知驱动板等待
						}
						break;              
					}					
					else 
					if(s == ERR_DISK_DISCON )//检测到断开,重新检测并计时
					{
						break;  
					}
					if(CH376GetDiskStatus( ) >= DEF_DISK_MOUNTED && i >= 5 ) //有的U盘总是返回未准备好,不过可以忽略,只要其建立连接MOUNTED且尝试5*50mS
					{
						ch376Status = ready;//U盘准备就绪
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[6]);//"U盘正常"
						#else
							DisStr("U 盘正常        ", 1, 0);
						#endif
						
						//显示U盘信息
						i = CH376ReadBlock((unsigned char*) buf );  //如果需要,可以读取数据块CH376_CMD_DATA.DiskMountInq,返回长度
						if( i == sizeof( INQUIRY_DATA ) )  //U盘的厂商和产品信息
						{  
							buf[ i ] = 0;
							
							#if SCREEN != TM12864Z
								sprintf((char*)str, "%s", ((P_INQUIRY_DATA)buf) -> VendorIdStr );//厂商信息
								//这里显示厂商信息
								lcd_show_string((char*)str, pt_word[9].x, pt_word[9].y, pt_word[9].color, pt_word[9].background, *(pt_word[9].font));
							#endif
							
							rs = handshake_with_driver(WAIT ,NULL, NULL);
						}
						
						break; 
					}
				}
				
				if(s == ERR_DISK_DISCON ) //检测到断开,重新检测并计时 */
				{
					LED_Toggle_Off(LED_USB);//
					ch376Status = init_complete;
					
					#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[7]);//"U盘断开"
					#else
					DisStr("U 盘断开        ", 1, 0);
					#endif
					
					Delay_ms(1000); //等 1s
				}
				
				if(CH376GetDiskStatus( ) < DEF_DISK_MOUNTED ) //未知USB设备,例如USB键盘、打印机等
				{
					ch376Status = init_complete;
					
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[8]);//"未知设备,请移除 "
					#else
						DisStr("未知设备,请移除 ", 1, 0);
					#endif
				}
			break;
			
			case ready://U盘准备就绪
			{
				#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[25]);//"请选择升级目标"
					lcd_show_character(&pt_word[23]);//"1:升级主板程序"
				#else
					DisStr("请选择升级目标", 0, 0);
					DisStr("1:升级主板程序", 1, 0);
				#endif
				
				if(rs == TRUE)
				{
					handshake_with_driver(WAIT ,NULL, NULL);
					
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[24]);//"2:升级驱动板程序"
					#else
						DisStr("2:升级驱动板程序", 2, 0);
					#endif
				}
				
				#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[26]);//"清空最后一行"
				#else
					DisStr("                ", 3, 0);
				#endif

				//等待键盘选择升级文件
				keyValue = key_run() ;
				if(keyValue == '1')
				{
					//选择主板升级文件
					strcpy( (char *)TarName, "\\BAIXING.BIN" ); //目标文件名
					ch376Status = search_file;
				}
				else
				if(keyValue == '2' && rs == 1)
				{
					//选择驱动板升级文件
					strcpy( (char *)TarName, "\\DRIVER.BIN" ); //目标文件名
					ch376Status = search_file;
				}
				else
				if(keyValue == '*')
				{
					run_to_app();
				}
			}
			break;

			case search_file://寻找升级文件
				s =	CH376FileOpenPath( (unsigned char *)TarName );
				
				if(s != USB_INT_SUCCESS)
				{
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[10]);//"未找升级到文件"
						//显示错误代码
						lcd_show_num(s, pt_word[10].valx, pt_word[10].valy, pt_word[10].color, pt_word[10].background, *(pt_word[10].font), 0, 10);
						lcd_show_character(&pt_word[27]);//刷第三行
					#else
						DisStr("未找到升级文件  ", 1, 0);
					#endif
					
					handshake_with_driver(WAIT ,NULL, NULL);
					
					keyValue = key_run() ;
					if(keyValue == '*')
					{
						ch376Status = ready;
					}
				}
				else
				{
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[11]);//"已找到升级文件" 
					#else
						DisStr("已找到升级文件  ", 1, 0);
					#endif
					ch376Status = updata;
				}
			break;

			case updata://开始升级
				s =	CH376FileOpenPath( (unsigned char *)TarName );//打开文件
				if(s != USB_INT_SUCCESS)
				{
					if(s == ERR_DISK_DISCON)
					{
						LED_Toggle_Off(LED_USB);
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[12]);//"U盘移除重新检测 " 
						#else
							DisStr("U 盘移除重新检测", 1, 0);
						#endif
					}
					else
					{
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[13]);//"发生异常重新检测" 
							//显示错误代码
							lcd_show_num(s, pt_word[13].valx, pt_word[13].valy, pt_word[13].color, pt_word[13].background, *(pt_word[13].font), 0, 10);
						#else
							DisStr("发生异常重新检测", 1, 0);
						#endif
					}
					ch376Status = init; 
				}
				else
				{
					if(keyValue == '1')
					ch376Status = updating_main;
					else
					if(keyValue == '2')
					ch376Status = updating_driver;
					
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[14]);//
					#else
						DisStr("正在升级请稍候", 1, 0);
					#endif
					
					LED_Toggle_On(LED_DOWN);//启动升级指示灯

				}
			break;
				
			case updating_main://主板升级
			{
				uint32_t i,count = 0;
				puint32_t ptr;
				ptr = (puint32_t)rd_buf;
				
				FLASH_Unlock();//解锁flash
				
				fileSize = CH376GetFileSize();//文件大小
					
				do
				{
					FLASH_Status fStatus;
					if((count % (PAGE_SIZE)) == 0)//擦除要写的块
					{
						FLASH_ErasePage(count + FLASH_APP_ADDR);
					}
					
					s = CH376ByteRead((uint8_t*)rd_buf, BUFF_SIZE ,(puint16_t)&realCount); //以字节为单位从当前位置读出数据
					if (s != DEF_DISK_OPEN_FILE)
					{
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[15]);//"升级失败请重试" 
							//显示升级失败错误码
							lcd_show_num(s, pt_word[15].valx, pt_word[15].valy, pt_word[15].color, pt_word[15].background, *(pt_word[15].font), 0, 10);
						#else
							DisStr("升级失败请重试  ", 1, 0);
						#endif
						ch376Status = init;
					}

					fStatus = FLASH_COMPLETE;
					
					//将读到的数据都写入flash ,每次写1 个字 4Byte
					for(i = 0; (i < (realCount / 4)) && (fStatus == FLASH_COMPLETE); i++)
					{
						uint8_t j = 0;
						//最多允许重写3次
						do{
							fStatus = FLASH_ProgramWord(count + FLASH_APP_ADDR, ptr[i]);//写到片内的FLASH(按字/4Byte写)
							if(fStatus == FLASH_COMPLETE)
							{
								count += 4;
							}
						}while (fStatus != FLASH_COMPLETE && j++<3);
					}
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[16]);//"Byte写入Flash" 
						//显示写入的字节数
						lcd_show_num(count, pt_word[16].valx, pt_word[16].valy, pt_word[16].color, pt_word[16].background, *(pt_word[16].font), 0, 10);
					#else
						DisStr("        Byte写入",  2, 0);
						DisStrNum(count, 2, 0);
					#endif
				}while(realCount == BUFF_SIZE);
				
				ch376Status = updata_complete;//升级任务完成
			}
			break;
			
			case updating_driver://驱动板升级
			{
				uint8_t rs = FALSE;
				uint32_t amont = 0, updataNum = 0;//升级包序号;
				
				fileSize = CH376GetFileSize();//文件大小
				do
				{
					s = CH376ByteRead((uint8_t*)&rd_buf[4], UPADTA_SIZE ,(puint16_t)&realCount); //以字节为单位从当前位置读出数据
					if (s != DEF_DISK_OPEN_FILE)
					{
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[15]);//"升级失败请重试" 
							//显示升级失败错误码
							lcd_show_num(s, pt_word[15].valx, pt_word[15].valy, pt_word[15].color, pt_word[15].background, *(pt_word[15].font), 0, 10);
						#else
							DisStr("升级失败请重试  ", 1, 0);
						#endif
						return 0;//这里直接退出去
					}
					else//从U盘读数据成功
					{
						updataNum++;
						rd_buf[0] = updataNum >> 24;
						rd_buf[1] = updataNum >> 16;
						rd_buf[2] = updataNum >> 8;
						rd_buf[3] = updataNum ;
						do{
							//将读到的数据发送到驱动板，每次发送一页数据
							rs = handshake_with_driver(UPDATA ,(uint8_t*)&rd_buf, (realCount+4));
							if(rs != TRUE)
							{
								#if SCREEN != TM12864Z
									lcd_show_character(&pt_word[15]);//"升级失败请重试" 
									//显示升级失败错误码
									lcd_show_num(s, pt_word[15].valx, pt_word[15].valy, pt_word[15].color, pt_word[15].background, *(pt_word[15].font), 0, 10);
								#else
									DisStr("升级失败        ", 1, 0);
								#endif
							}
						}while(rs != TRUE);
					}
					amont += realCount;
							
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[16]);//"Byte写入Flash" 
						//显示写入的字节数
						lcd_show_num(amont, pt_word[16].valx, pt_word[16].valy, pt_word[16].color, pt_word[16].background, *(pt_word[16].font), 0, 10);
					#else
						DisStr("        Byte写入",  2, 0);
						DisStrNum(amont, 2, 0);
					#endif
				}while(realCount == UPADTA_SIZE);
				
				rs = handshake_with_driver(APP ,NULL, NULL);
				ch376Status = updata_complete;//升级任务完成
			}
			break;
			
			case updata_complete:
				if(keyValue == '1')//主程序升级完成直接进入用户程序
				{
					run_to_app();
				}
				else
				if(keyValue == '2')
				{
					//提示驱动板升级成功
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[29]);//"驱动板升级成功"
					#else
						DisStr("驱动板升级成功", 3, 0);
					#endif

					delay_ms(2000);
//					lcd_show_character(&pt_word[27]);//"驱动板升级成功"

//					lcd_show_character(&pt_word[25]);//"请选择升级目标"
//					lcd_show_character(&pt_word[23]);//"1:升级主板程序"
//					lcd_show_character(&pt_word[26]);//"清空最后一行"
//					
//					if(key_run() == '1')
//					{
//						//选择主板升级文件
//						strcpy( (char *)TarName, "\\BAIXING.BIN" ); //目标文件名
//						ch376Status = search_file;
//					}
					ch376Status = ready;
				}
					
			break;
		}
	}
}		
