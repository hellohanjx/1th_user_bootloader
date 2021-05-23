#include "RA8875.h"
#include<stdio.h>
static char log_buf[8][60];
static uint8_t curhead=0;

void LOG_Init(){
	uint8_t i=0;
	for(i=0;i<8;i++){
		log_buf[i][0]=0;
	}
	curhead=0;
	RA8875_SetFrontColor(BLUE);
}
void LOG(char* pstr){
	uint16_t i;
	for(i=0; pstr[i]!=0;i++)
	{
			log_buf[curhead][i]=pstr[i];
	}
	for(;i<59;i++)
	{
		log_buf[curhead][i]=' ';
	}
	log_buf[curhead][i]=0;
	for(i=0;i<8;i++){
		uint8_t line=(curhead+8-i)%8;
		RA8875_DispAscii(10, 20*(8-i),&log_buf[line][0]);
	}
	curhead=(curhead+1)%8;
}

void LoadBinProgress(uint16_t progress,uint16_t color){
	char str[50];
	sprintf(str,"App bin has been loaded %u%%",progress);
	RA8875_DispAscii(10, 20*9,str);
}
