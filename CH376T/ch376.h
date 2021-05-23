#include"CH376INC.H"

#include "UserConfig.h"

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO操作，只针对单一IO
//确保 n 值小于16
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n) 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n) 



#define	CH376_SPI_SDI			PBout(15)     /* PB15复用      SDI   */
#define	CH376_SPI_SDO			PBout(14)     /* PB14复用      SDO   */
#define	CH376_SPI_SCK			PBout(13)	    /* PB14复用      SCK   */
#define  CH376_SPI_SCS			PBout(12)     /* PB12推挽输出  CS       */
//#define  CH376_RST            	PCout(5)     /* PC5推挽输出  RST   */
#define	CH376_INT_WIRE      	PCin(6)      /* PC6上拉输入  INT   */
/* 附加的USB操作状态定义 */
#define		ERR_USB_UNKNOWN		0xFA	                                                /* 未知错误,不应该发生的情况,需检查硬件或者程序错误 */


void CH376_PORT_INIT( void );  		                                                /* CH376通讯接口初始化 */
void xEndCH376Cmd( void );															/* 结束SPI命令 */
void xWriteCH376Cmd( uint8_t mCmd );	                                                /* 向CH376写命令 */
void xWriteCH376Data( uint8_t mData );	                                                /* 向CH376写数据 */
uint8_t xReadCH376Data( void );			                                                /* 从CH376读数据 */
uint8_t Query376Interrupt( void );		                                                /* 查询CH376中断(INT#引脚为低电平) */
uint8_t mInitCH376Host( void );			                                                /* 初始化CH376 */
void CH376SPI_Init(void);						  	    													  
//void SPI2_Init(void);			 //初始化SPI口  
uint8_t SPI2_ReadWriteByte(uint8_t TxData);//SPI总线读写一个字节

void SPI2_Init(void);			 //初始化SPI口
void SPIx_SetSpeed(uint8_t SpeedSet); //设置SPI速度   
uint8_t SPI2_ReadWriteByte(uint8_t TxData);//SPI总线读写一个字节

