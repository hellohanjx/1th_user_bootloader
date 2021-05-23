
#ifndef __FSMC_NOR_H
#define __FSMC_NOR_H

#include "stm32f10x.h"
#include "Gpio.h"

typedef struct
{
  u16 Manufacturer_Code;
  u16 Device_Code1;
  u16 Device_Code2;
  u16 Device_Code3;
}NOR_IDTypeDef;

typedef enum
{
  NOR_SUCCESS = 0,
  NOR_ONGOING,
  NOR_ERROR,
  NOR_TIMEOUT
}NOR_Status;

#define Bank1_NOR2_ADDR           ((u32)0x64000000)
#define BUFFER_SIZE_NOR            0x400

#define BlockErase_Timeout        ((u32)0x00A00000)
#define ChipErase_Timeout         ((u32)0x30000000) 
#define Program_Timeout           ((u32)0x00001400)

#define ADDR_SHIFT(A)             (Bank1_NOR2_ADDR + (2 * (A)))
#define NOR_WRITE(Address, Data)  (*(vu16 *)(Address) = (Data))


void FSMC_NOR_Init(void);
void FSMC_NOR_ReadID(NOR_IDTypeDef* NOR_ID);
NOR_Status FSMC_NOR_EraseBlock(u32 BlockAddr);
NOR_Status FSMC_NOR_EraseChip(void);
NOR_Status FSMC_NOR_WriteHalfWord(u32 WriteAddr, u16 Data);
NOR_Status FSMC_NOR_WriteBuffer(u16* pBuffer, u32 WriteAddr, u32 NumHalfwordToWrite);
NOR_Status FSMC_NOR_ProgramBuffer(u16* pBuffer, u32 WriteAddr, u32 NumHalfwordToWrite);
u16 FSMC_NOR_ReadHalfWord(u32 ReadAddr);
void FSMC_NOR_ReadBuffer(u16* pBuffer, u32 ReadAddr, u32 NumHalfwordToRead);
NOR_Status FSMC_NOR_ReturnToReadMode(void);
NOR_Status FSMC_NOR_Reset(void);
NOR_Status FSMC_NOR_GetStatus(u32 Timeout);
void NOR_Test(void);
#endif 

