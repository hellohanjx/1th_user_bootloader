/**
  * @file    fsmc_nor.c 
  * @author  WB R&D Team - openmcu666
  * @version V1.0
  * @date    2016.05.04
  * @brief   Nor Flash Driver
  */
#include "fsmc_nor.h"

u16 TxBuffer_NOR[BUFFER_SIZE_NOR];
u16 RxBuffer_NOR[BUFFER_SIZE_NOR];
/**
  * @brief  初始化FSMC及其IO
  * @param  None
  * @retval None
  */
void FSMC_NOR_Init(void)
{
  FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
  FSMC_NORSRAMTimingInitTypeDef  p;
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | 
                         RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);
	
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
  /* NOR Data lines configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
                                GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 |
                                GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* NOR Address lines configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
                                GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12 | GPIO_Pin_13 |
                                GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_Init(GPIOF, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 |
                                GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;                            
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* NOE and NWE configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* NE2 configuration */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  /*-- FSMC Configuration ----------------------------------------------------*/
  p.FSMC_AddressSetupTime = 0x02;
  p.FSMC_AddressHoldTime = 0x01;
  p.FSMC_DataSetupTime = 0x03;
  p.FSMC_BusTurnAroundDuration = 0x00;
  p.FSMC_CLKDivision = 0x00;
  p.FSMC_DataLatency = 0x00;
  p.FSMC_AccessMode = FSMC_AccessMode_B;

  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM2;
  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
  FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;

  FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;

  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM2, ENABLE);//使能FSMC Bank1_NOR Bank
}

/**
  * @brief  读取NorFlash的ID
  * @param  NOR_ID
  * @retval None
  */
void FSMC_NOR_ReadID(NOR_IDTypeDef* NOR_ID)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x0090);

  NOR_ID->Manufacturer_Code = *(vu16 *) ADDR_SHIFT(0x0000);
  NOR_ID->Device_Code1 = *(vu16 *) ADDR_SHIFT(0x0001);
  NOR_ID->Device_Code2 = *(vu16 *) ADDR_SHIFT(0x000E);
  NOR_ID->Device_Code3 = *(vu16 *) ADDR_SHIFT(0x000F);
}

/**
  * @brief  擦除块
  * @param  BlockAddr:块地址
  * @retval NOR_SUCCESS, NOR_ERROR, NOR_TIMEOUT
  */
NOR_Status FSMC_NOR_EraseBlock(u32 BlockAddr)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x0080);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE((Bank1_NOR2_ADDR + BlockAddr), 0x30);

  return (FSMC_NOR_GetStatus(BlockErase_Timeout));
}

/**
  * @brief  整片擦除
  * @param  None
  * @retval NOR_SUCCESS, NOR_ERROR, NOR_TIMEOUT
  */
NOR_Status FSMC_NOR_EraseChip(void)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x0080);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x0010);

  return (FSMC_NOR_GetStatus(ChipErase_Timeout));
}

/**
  * @brief  写入16位数据
  * @param  WriteAddr:写入的地址;Data:写入的数据
  * @retval NOR_SUCCESS, NOR_ERROR, NOR_TIMEOUT
  */
NOR_Status FSMC_NOR_WriteHalfWord(u32 WriteAddr, u16 Data)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00A0);
  NOR_WRITE((Bank1_NOR2_ADDR + WriteAddr), Data);

  return (FSMC_NOR_GetStatus(Program_Timeout));
}

/**
  * @brief  写入指定长度的数据
  * @param  pBuffer:指向写入的地址;WriteAddr :写入的地址;NumHalfwordToWrite:要写入的数据长度
  * @retval NOR_SUCCESS, NOR_ERROR, NOR_TIMEOUT
  */
NOR_Status FSMC_NOR_WriteBuffer(u16* pBuffer, u32 WriteAddr, u32 NumHalfwordToWrite)
{
  NOR_Status status = NOR_ONGOING; 

  do
  {
    status = FSMC_NOR_WriteHalfWord(WriteAddr, *pBuffer++);
    WriteAddr = WriteAddr + 2;
    NumHalfwordToWrite--;
  }
  while((status == NOR_SUCCESS) && (NumHalfwordToWrite != 0));
  
  return (status); 
}

/**
  * @brief  写入指定长度的数据到Nor(只能用于S29GL128P NOR memory)
  * @param  pBuffer:指向写入的地址;WriteAddr :写入的地址;NumHalfwordToWrite:要写入的数据长度
  * @retval NOR_SUCCESS, NOR_ERROR, NOR_TIMEOUT
  */
NOR_Status FSMC_NOR_ProgramBuffer(u16* pBuffer, u32 WriteAddr, u32 NumHalfwordToWrite)
{       
  u32 lastloadedaddress = 0x00;
  u32 currentaddress = 0x00;
  u32 endaddress = 0x00;

  currentaddress = WriteAddr;
  endaddress = WriteAddr + NumHalfwordToWrite - 1;
  lastloadedaddress = WriteAddr;

  NOR_WRITE(ADDR_SHIFT(0x005555), 0x00AA);                           //发送解锁命令序列

  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);  

  NOR_WRITE(ADDR_SHIFT(WriteAddr), 0x0025);                           //写缓冲加载命令
  NOR_WRITE(ADDR_SHIFT(WriteAddr), (NumHalfwordToWrite - 1));

  while(currentaddress <= endaddress)
  {
    lastloadedaddress = currentaddress;
 
    NOR_WRITE(ADDR_SHIFT(currentaddress), *pBuffer++);
    currentaddress += 1; 
  }

  NOR_WRITE(ADDR_SHIFT(lastloadedaddress), 0x29);
  
  return(FSMC_NOR_GetStatus(Program_Timeout));
}

/**
  * @brief  读取半字数据
  * @param  ReadAddr:读取的地址
  * @retval 读取的数据
  */
u16 FSMC_NOR_ReadHalfWord(u32 ReadAddr)
{
  NOR_WRITE(ADDR_SHIFT(0x005555), 0x00AA); 
  NOR_WRITE(ADDR_SHIFT(0x002AAA), 0x0055);  
  NOR_WRITE((Bank1_NOR2_ADDR + ReadAddr), 0x00F0 );

  return (*(vu16 *)((Bank1_NOR2_ADDR + ReadAddr)));
}

/**
  * @brief  写入指定长度的数据
  * @param  pBuffer:指向读取的缓冲区;ReadAddr :读取的地址;NumHalfwordToWrite:要读取的数据长度
  * @retval None
  */
void FSMC_NOR_ReadBuffer(u16* pBuffer, u32 ReadAddr, u32 NumHalfwordToRead)
{
  NOR_WRITE(ADDR_SHIFT(0x05555), 0x00AA);
  NOR_WRITE(ADDR_SHIFT(0x02AAA), 0x0055);
  NOR_WRITE((Bank1_NOR2_ADDR + ReadAddr), 0x00F0);

  for(; NumHalfwordToRead != 0x00; NumHalfwordToRead--) 
  {
   
    *pBuffer++ = *(vu16 *)((Bank1_NOR2_ADDR + ReadAddr));
    ReadAddr = ReadAddr + 2; 
  }  
}

/**
  * @brief  返回读取模式
  * @param  None
  * @retval NOR_SUCCESS
  */
NOR_Status FSMC_NOR_ReturnToReadMode(void)
{
  NOR_WRITE(Bank1_NOR2_ADDR, 0x00F0);

  return (NOR_SUCCESS);
}

/**
  * @brief  复位Nor Flash
  * @param  None
  * @retval NOR_SUCCESS
  */
NOR_Status FSMC_NOR_Reset(void)
{
  NOR_WRITE(ADDR_SHIFT(0x005555), 0x00AA); 
  NOR_WRITE(ADDR_SHIFT(0x002AAA), 0x0055); 
  NOR_WRITE(Bank1_NOR2_ADDR, 0x00F0); 

  return (NOR_SUCCESS);
}

/**
  * @brief  读取Nor状态
  * @param  Timeout: 超时
  * @retval NOR_SUCCESS; NOR_ERROR; NOR_TIMEOUT
  */
NOR_Status FSMC_NOR_GetStatus(u32 Timeout)
{ 
  u16 val1 = 0x00, val2 = 0x00;
  NOR_Status status = NOR_ONGOING; 
  u32 timeout = Timeout;

  while((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) != RESET) && (timeout > 0)) //忙等待
  {
    timeout--;
  }

  timeout = Timeout;
  
  while((GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == RESET) && (timeout > 0))   
  {
    timeout--;
  }
  
  while((Timeout != 0x00) && (status != NOR_SUCCESS))                        //读取Nor状态
  {
    Timeout--;

    val1 = *(vu16 *)(Bank1_NOR2_ADDR);                                       //Read DQ6 and DQ5 
    val2 = *(vu16 *)(Bank1_NOR2_ADDR);

    if((val1 & 0x0040) == (val2 & 0x0040)) 
    {
      return NOR_SUCCESS;
    }

    if((val1 & 0x0020) != 0x0020)
    {
      status = NOR_ONGOING;
    }

    val1 = *(vu16 *)(Bank1_NOR2_ADDR);
    val2 = *(vu16 *)(Bank1_NOR2_ADDR);
    
    if((val1 & 0x0040) == (val2 & 0x0040)) 
    {
      return NOR_SUCCESS;
    }
    else if((val1 & 0x0020) == 0x0020)
    {
      return NOR_ERROR;
    }
  }

  if(Timeout == 0x00)
  {
    status = NOR_TIMEOUT;
  } 

  return (status);
}
/*
void NOR_Test(void)
{
	u32 Index;
	vu32 WRStatus=0;
	vu32 time1=0,time2=0;
	float speed=0.00;
	printf("\r\n***********************NOR Flash读写测试**********************\r\n");
  
	FSMC_NOR_ReturnToReadMode();

  FSMC_NOR_EraseBlock(WR_ADDR_NOR);                                        //NOR 块擦除

  Fill_Buffer(TxBuffer_NOR, BUFFER_SIZE_NOR, 0x3210);                      //填充写入NOR的数据
  
	TIM_Init();
	time1=get_runtime();
	FSMC_NOR_WriteBuffer(TxBuffer_NOR, WR_ADDR_NOR, BUFFER_SIZE_NOR);
  time2=get_runtime();
  
	speed=(time2-time1)/100000.00;//转为秒
	//printf("\r\n写1KB耗时%dus,写速度:%dKB/s\r\n",(time2-time1)*10,100000/(time2-time1)); //10us中断1次
  printf("\r\n写1KB耗时%dus,写速度:%.2fKB/s\r\n",(time2-time1)*10,1/speed); //10us中断1次
	time1=0;
	time2=0;
	time1=get_runtime();
	FSMC_NOR_ReadBuffer(RxBuffer_NOR, WR_ADDR_NOR, BUFFER_SIZE_NOR);  
  time2=get_runtime();
  printf("\r\n读1KB耗时%dus,读速度:%dKB/s\r\n",(time2-time1)*10,100000/(time2-time1)); //10us中断1次
 	Set_TIM2_IRQn(DISABLE);
  
	for(Index = 0x00; (Index < BUFFER_SIZE_NOR) && (WRStatus == 0); Index++)//检查数据正确性
  {
    if (RxBuffer_NOR[Index] != TxBuffer_NOR[Index])
    {
	    GPIO_ResetBits(LED_GPIO_PORT, LED3_GPIO_PIN);   //如果读出来的和写进去的不相同则LD3亮

      WRStatus = Index + 1;
    }
  }	

  if(WRStatus == 0)
  {	
		printf("\r\nNor Flash读写测试通过\r\n");
    GPIO_ResetBits(LED_GPIO_PORT, LED1_GPIO_PIN);	   //LD1亮的话说明写进去的和读出来的全部完全一样
  }
  else
  { 
		printf("\r\nNor Flash读写测试失败,请复位后再试一次\r\n");
    GPIO_ResetBits(LED_GPIO_PORT, LED2_GPIO_PIN);     //如果读出来的和写进去的不相同则LD2亮 
  }
	
}*/
