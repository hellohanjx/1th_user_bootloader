#ifndef __IAP_H
#define __IAP_H

#include <stm32f10x.h>

#define FLASH_APP_ADDR		(0x08010000)  	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH),����0X08000000~0X0800FFFF�Ŀռ�ΪIAPʹ��
typedef  void (*pFunction)(void);


#define PAGE_SIZE  2048		//������ÿҳ2K,С����ÿҳ1K



#endif /* __IAP_H */

