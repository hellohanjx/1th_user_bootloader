#ifndef __IAP_H
#define __IAP_H

#include <stm32f10x.h>

#define FLASH_APP_ADDR		(0x08010000)  	//第一个应用程序起始地址(存放在FLASH),保留0X08000000~0X0800FFFF的空间为IAP使用
typedef  void (*pFunction)(void);


#define PAGE_SIZE  2048		//大容量每页2K,小容量每页1K



#endif /* __IAP_H */

