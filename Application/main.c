/*
bootloader
�޸ļ�¼��
2017-7-1��
1.�޸��˵���ʱ�����bug����Ϊ��rtc�ж��е���ʱ��ֵ
2.���Ӷ� RA8875 7��������Ļ֧��
3.ע����������֧�ֺڵװ��֣�����ʾ����
*/

#include "stm32f10x_conf.h"
#include "lcd_header.h"
#include "bsp_key.h"
#include "uart485.h"

enum{init, init_complete, inser_u, ready, search_file, updata, updating_main, updating_driver, updata_complete};
#define BUFF_SIZE			1024	//��������С
#define UPADTA_SIZE		BUFF_SIZE
volatile uint8_t rd_buf[BUFF_SIZE+4];	//����������,����4�������
volatile uint16_t realCount;			//��U�̶������ļ���ʵ�ֽ���

volatile uint32_t JumpAddress;
pFunction Jump_To_Application;
	
extern void __set_MSP(uint32_t topOfMainStack);
extern void LOG_Init(void);
extern void LOG(char* ptr);
extern void LoadBinProgress(uint16_t progress,uint16_t color);
extern volatile uint8_t countDown;

/*
��ת���û�����
*/
void run_to_app(void)
{
	handshake_with_driver(APP ,NULL, NULL);//֪ͨ���������app

	#if SCREEN != TM12864Z
		lcd_show_character(&pt_word[18]);//"��������"
	#else
		DisStr("��������        " ,3, 0);
	#endif
	
	Delay_ms(3000);//�ȴ�2s����ת
	JumpAddress = *(__IO uint32_t*) (FLASH_APP_ADDR+4);
	Jump_To_Application = (pFunction) JumpAddress;         
	__set_MSP(*(__IO uint32_t*) FLASH_APP_ADDR);  //�ж�������ת��
	
	Jump_To_Application();                //��ת���û�app
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
	
	//NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);//�����ж�������ƫ��
	//SystemInit();//��ʼ��STM32 ʱ��
	//NVIC_Configuration();   
	
	RCC_Configuration();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);//�����ж����ȼ�,����ռ�ж�

	key_Configuration();//��ʼ��
	lcd_init();
	RTC_Init();
	LED_Init();
	Delay_Init();	//systick��ʼ�� 
	CH376SPI_Init();
	uart_485_config();
	Delay_Init();
	
	#if SCREEN != TM12864Z
	lcd_show_character(&pt_word[0]);//"��������"
	#else
	DisStr("�������U ��    ", 0, 0);
	#endif

	countDown = 3;
	lcd_show_time(&pt_word[1]);//"��ʾ��ǰʱ��"
	
	while(1)
	{
		LED_Toggle_On(1);
		Delay_ms(200);	
		LED_Toggle_Off(1);
		
		switch(ch376Status)
		{
			case init://��ʼ̬
				#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[2]);//"���ڼ��U��"
				#else
					DisStr("���ڼ��U ��    ", 1, 0);
				#endif
				
				s = mInitCH376Host();//��ʼ��ch376
				
				if (s!=0x14)//ch376δ׼����
				{
					if(ch376Status ++ >10)
					{
						#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[3]);//"CH376 ERROR,error Code is "
						//��ʾ������
						lcd_show_num(s, pt_word[3].valx, pt_word[3].valy, pt_word[3].color, pt_word[3].background, *(pt_word[3].font), 0, 10);
						#else
						DisStr("CH376 ��      ",1, 0);
						#endif
					}
					else
						Delay_ms(100);	//�ȴ�100ms�������
				}
				else
				{
					ch376Status = init_complete;
				}
			break;
				
			case init_complete://��ʼ���ɹ�
				/* ���U���Ƿ�����,�ȴ�U�̲���,����SD��,
						�����ɵ�Ƭ��ֱ�Ӳ�ѯSD�����Ĳ��״̬���� */
				if( CH376DiskConnect( ) != USB_INT_SUCCESS )
				{  
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[4]);//"δ����U�� "
					#else
						DisStr("δ����U ��    ",1, 0);
					#endif
					
					Delay_ms(100);
				}
				else
				{
					ch376Status = inser_u;//����U��
					
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[5]);//"��⵽U��"
					#else
						DisStr("��⵽U ��      ", 1, 0);
					#endif
					
					LED_Toggle_On(LED_USB);	//U�̵Ƶ���
					Delay_ms( 200 );  // ���ڼ�⵽USB�豸��,���ȴ�100~50mS,��Ҫ�����ЩMP3̫��,���ڼ�⵽USB�豸
				}
				
				//����ʱ
				if( !(countDown > 0 && countDown <= 3) )//
					countDown = 0;
				#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[17]);//"�������"
					lcd_show_num(countDown, pt_word[17].valx, pt_word[17].valy, pt_word[17].color, pt_word[17].background, *(pt_word[17].font), 1, 2);
				#else
					DisStr("�������", 3, 1);
					DisStrNum_02(countDown, 3, 0);
				#endif
				
				if(countDown == 0)
				{
					run_to_app();
				}

			break;
			
			case inser_u://����U��
				LED_Toggle_On(LED_USB);//����U��ָʾ��
				for ( i = 0; i < 100; i++ ) 
				{   
					delay_ms( 50 );
					s = CH376DiskMount( );  	//��ʼ�����̲����Դ����Ƿ����.   
					if ( s == USB_INT_SUCCESS ) //׼����
					{
						ch376Status = ready;
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[6]);//"U������"
						#else
							DisStr("U ������        ", 1, 0);
						#endif

						//��ʾU����Ϣ
						i = CH376ReadBlock((unsigned char*) buf );  //�����Ҫ,���Զ�ȡ���ݿ�CH376_CMD_DATA.DiskMountInq,���س���
						if( i == sizeof( INQUIRY_DATA ) )  //U�̵ĳ��̺Ͳ�Ʒ��Ϣ
						{  
							buf[ i ] = 0;
							
							#if SCREEN != TM12864Z
								sprintf((char*)str, "%s", ((P_INQUIRY_DATA)buf) -> VendorIdStr );//������Ϣ
								//������ʾ������Ϣ
								lcd_show_string((char*)str, pt_word[9].x, pt_word[9].y, pt_word[9].color, pt_word[9].background, *(pt_word[9].font));
							#endif
							
							rs = handshake_with_driver(WAIT ,NULL, NULL);//֪ͨ������ȴ�
						}
						break;              
					}					
					else 
					if(s == ERR_DISK_DISCON )//��⵽�Ͽ�,���¼�Ⲣ��ʱ
					{
						break;  
					}
					if(CH376GetDiskStatus( ) >= DEF_DISK_MOUNTED && i >= 5 ) //�е�U�����Ƿ���δ׼����,�������Ժ���,ֻҪ�佨������MOUNTED�ҳ���5*50mS
					{
						ch376Status = ready;//U��׼������
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[6]);//"U������"
						#else
							DisStr("U ������        ", 1, 0);
						#endif
						
						//��ʾU����Ϣ
						i = CH376ReadBlock((unsigned char*) buf );  //�����Ҫ,���Զ�ȡ���ݿ�CH376_CMD_DATA.DiskMountInq,���س���
						if( i == sizeof( INQUIRY_DATA ) )  //U�̵ĳ��̺Ͳ�Ʒ��Ϣ
						{  
							buf[ i ] = 0;
							
							#if SCREEN != TM12864Z
								sprintf((char*)str, "%s", ((P_INQUIRY_DATA)buf) -> VendorIdStr );//������Ϣ
								//������ʾ������Ϣ
								lcd_show_string((char*)str, pt_word[9].x, pt_word[9].y, pt_word[9].color, pt_word[9].background, *(pt_word[9].font));
							#endif
							
							rs = handshake_with_driver(WAIT ,NULL, NULL);
						}
						
						break; 
					}
				}
				
				if(s == ERR_DISK_DISCON ) //��⵽�Ͽ�,���¼�Ⲣ��ʱ */
				{
					LED_Toggle_Off(LED_USB);//
					ch376Status = init_complete;
					
					#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[7]);//"U�̶Ͽ�"
					#else
					DisStr("U �̶Ͽ�        ", 1, 0);
					#endif
					
					Delay_ms(1000); //�� 1s
				}
				
				if(CH376GetDiskStatus( ) < DEF_DISK_MOUNTED ) //δ֪USB�豸,����USB���̡���ӡ����
				{
					ch376Status = init_complete;
					
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[8]);//"δ֪�豸,���Ƴ� "
					#else
						DisStr("δ֪�豸,���Ƴ� ", 1, 0);
					#endif
				}
			break;
			
			case ready://U��׼������
			{
				#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[25]);//"��ѡ������Ŀ��"
					lcd_show_character(&pt_word[23]);//"1:�����������"
				#else
					DisStr("��ѡ������Ŀ��", 0, 0);
					DisStr("1:�����������", 1, 0);
				#endif
				
				if(rs == TRUE)
				{
					handshake_with_driver(WAIT ,NULL, NULL);
					
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[24]);//"2:�������������"
					#else
						DisStr("2:�������������", 2, 0);
					#endif
				}
				
				#if SCREEN != TM12864Z
					lcd_show_character(&pt_word[26]);//"������һ��"
				#else
					DisStr("                ", 3, 0);
				#endif

				//�ȴ�����ѡ�������ļ�
				keyValue = key_run() ;
				if(keyValue == '1')
				{
					//ѡ�����������ļ�
					strcpy( (char *)TarName, "\\BAIXING.BIN" ); //Ŀ���ļ���
					ch376Status = search_file;
				}
				else
				if(keyValue == '2' && rs == 1)
				{
					//ѡ�������������ļ�
					strcpy( (char *)TarName, "\\DRIVER.BIN" ); //Ŀ���ļ���
					ch376Status = search_file;
				}
				else
				if(keyValue == '*')
				{
					run_to_app();
				}
			}
			break;

			case search_file://Ѱ�������ļ�
				s =	CH376FileOpenPath( (unsigned char *)TarName );
				
				if(s != USB_INT_SUCCESS)
				{
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[10]);//"δ���������ļ�"
						//��ʾ�������
						lcd_show_num(s, pt_word[10].valx, pt_word[10].valy, pt_word[10].color, pt_word[10].background, *(pt_word[10].font), 0, 10);
						lcd_show_character(&pt_word[27]);//ˢ������
					#else
						DisStr("δ�ҵ������ļ�  ", 1, 0);
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
						lcd_show_character(&pt_word[11]);//"���ҵ������ļ�" 
					#else
						DisStr("���ҵ������ļ�  ", 1, 0);
					#endif
					ch376Status = updata;
				}
			break;

			case updata://��ʼ����
				s =	CH376FileOpenPath( (unsigned char *)TarName );//���ļ�
				if(s != USB_INT_SUCCESS)
				{
					if(s == ERR_DISK_DISCON)
					{
						LED_Toggle_Off(LED_USB);
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[12]);//"U���Ƴ����¼�� " 
						#else
							DisStr("U ���Ƴ����¼��", 1, 0);
						#endif
					}
					else
					{
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[13]);//"�����쳣���¼��" 
							//��ʾ�������
							lcd_show_num(s, pt_word[13].valx, pt_word[13].valy, pt_word[13].color, pt_word[13].background, *(pt_word[13].font), 0, 10);
						#else
							DisStr("�����쳣���¼��", 1, 0);
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
						DisStr("�����������Ժ�", 1, 0);
					#endif
					
					LED_Toggle_On(LED_DOWN);//��������ָʾ��

				}
			break;
				
			case updating_main://��������
			{
				uint32_t i,count = 0;
				puint32_t ptr;
				ptr = (puint32_t)rd_buf;
				
				FLASH_Unlock();//����flash
				
				fileSize = CH376GetFileSize();//�ļ���С
					
				do
				{
					FLASH_Status fStatus;
					if((count % (PAGE_SIZE)) == 0)//����Ҫд�Ŀ�
					{
						FLASH_ErasePage(count + FLASH_APP_ADDR);
					}
					
					s = CH376ByteRead((uint8_t*)rd_buf, BUFF_SIZE ,(puint16_t)&realCount); //���ֽ�Ϊ��λ�ӵ�ǰλ�ö�������
					if (s != DEF_DISK_OPEN_FILE)
					{
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[15]);//"����ʧ��������" 
							//��ʾ����ʧ�ܴ�����
							lcd_show_num(s, pt_word[15].valx, pt_word[15].valy, pt_word[15].color, pt_word[15].background, *(pt_word[15].font), 0, 10);
						#else
							DisStr("����ʧ��������  ", 1, 0);
						#endif
						ch376Status = init;
					}

					fStatus = FLASH_COMPLETE;
					
					//�����������ݶ�д��flash ,ÿ��д1 ���� 4Byte
					for(i = 0; (i < (realCount / 4)) && (fStatus == FLASH_COMPLETE); i++)
					{
						uint8_t j = 0;
						//���������д3��
						do{
							fStatus = FLASH_ProgramWord(count + FLASH_APP_ADDR, ptr[i]);//д��Ƭ�ڵ�FLASH(����/4Byteд)
							if(fStatus == FLASH_COMPLETE)
							{
								count += 4;
							}
						}while (fStatus != FLASH_COMPLETE && j++<3);
					}
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[16]);//"Byteд��Flash" 
						//��ʾд����ֽ���
						lcd_show_num(count, pt_word[16].valx, pt_word[16].valy, pt_word[16].color, pt_word[16].background, *(pt_word[16].font), 0, 10);
					#else
						DisStr("        Byteд��",  2, 0);
						DisStrNum(count, 2, 0);
					#endif
				}while(realCount == BUFF_SIZE);
				
				ch376Status = updata_complete;//�����������
			}
			break;
			
			case updating_driver://����������
			{
				uint8_t rs = FALSE;
				uint32_t amont = 0, updataNum = 0;//���������;
				
				fileSize = CH376GetFileSize();//�ļ���С
				do
				{
					s = CH376ByteRead((uint8_t*)&rd_buf[4], UPADTA_SIZE ,(puint16_t)&realCount); //���ֽ�Ϊ��λ�ӵ�ǰλ�ö�������
					if (s != DEF_DISK_OPEN_FILE)
					{
						#if SCREEN != TM12864Z
							lcd_show_character(&pt_word[15]);//"����ʧ��������" 
							//��ʾ����ʧ�ܴ�����
							lcd_show_num(s, pt_word[15].valx, pt_word[15].valy, pt_word[15].color, pt_word[15].background, *(pt_word[15].font), 0, 10);
						#else
							DisStr("����ʧ��������  ", 1, 0);
						#endif
						return 0;//����ֱ���˳�ȥ
					}
					else//��U�̶����ݳɹ�
					{
						updataNum++;
						rd_buf[0] = updataNum >> 24;
						rd_buf[1] = updataNum >> 16;
						rd_buf[2] = updataNum >> 8;
						rd_buf[3] = updataNum ;
						do{
							//�����������ݷ��͵������壬ÿ�η���һҳ����
							rs = handshake_with_driver(UPDATA ,(uint8_t*)&rd_buf, (realCount+4));
							if(rs != TRUE)
							{
								#if SCREEN != TM12864Z
									lcd_show_character(&pt_word[15]);//"����ʧ��������" 
									//��ʾ����ʧ�ܴ�����
									lcd_show_num(s, pt_word[15].valx, pt_word[15].valy, pt_word[15].color, pt_word[15].background, *(pt_word[15].font), 0, 10);
								#else
									DisStr("����ʧ��        ", 1, 0);
								#endif
							}
						}while(rs != TRUE);
					}
					amont += realCount;
							
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[16]);//"Byteд��Flash" 
						//��ʾд����ֽ���
						lcd_show_num(amont, pt_word[16].valx, pt_word[16].valy, pt_word[16].color, pt_word[16].background, *(pt_word[16].font), 0, 10);
					#else
						DisStr("        Byteд��",  2, 0);
						DisStrNum(amont, 2, 0);
					#endif
				}while(realCount == UPADTA_SIZE);
				
				rs = handshake_with_driver(APP ,NULL, NULL);
				ch376Status = updata_complete;//�����������
			}
			break;
			
			case updata_complete:
				if(keyValue == '1')//�������������ֱ�ӽ����û�����
				{
					run_to_app();
				}
				else
				if(keyValue == '2')
				{
					//��ʾ�����������ɹ�
					#if SCREEN != TM12864Z
						lcd_show_character(&pt_word[29]);//"�����������ɹ�"
					#else
						DisStr("�����������ɹ�", 3, 0);
					#endif

					delay_ms(2000);
//					lcd_show_character(&pt_word[27]);//"�����������ɹ�"

//					lcd_show_character(&pt_word[25]);//"��ѡ������Ŀ��"
//					lcd_show_character(&pt_word[23]);//"1:�����������"
//					lcd_show_character(&pt_word[26]);//"������һ��"
//					
//					if(key_run() == '1')
//					{
//						//ѡ�����������ļ�
//						strcpy( (char *)TarName, "\\BAIXING.BIN" ); //Ŀ���ļ���
//						ch376Status = search_file;
//					}
					ch376Status = ready;
				}
					
			break;
		}
	}
}		
