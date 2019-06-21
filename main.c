/******************************************************************************
*
*                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾
*
*******************************************************************************
* �� �� ��   : main.c
* �� �� ��   : V1.0
* ��    ��   : chenxianyue
* ��������   : 2019��6��4��
* ��������   : ������������Ͳ�����ʼ������ѭ������Ҫ���ܺ�����ڡ�
* �޸���ʷ   :
* ��    ��   : 
* ��    ��   :
* �޸�����   : 
******************************************************************************/
#include "t5los8051.h"
#include "system/sys.h"
#include "ch376/app_interface.h"
#include "system/uart.h"
#include "ch376/ch376.h"
#include "ch376/para_port.h"
#include "ch376/file_sys.h"
#include "ch376/dgus.h"
#include "string.h"
#include "stdio.h"

void Delay(void);
UINT8 res = 0x00;
UINT32 DiskFree;
UINT32 NewSize;
UINT32 PRINT_EN = 0;
UINT8 xdata FileDataBuf[0x1000];
UINT16 TIMES = 0;
FAT_NAME MatchLish[DIR_FILE_MAX] = {0};
int main()
{
	
	UINT8 j = 0;
	UINT32 i = 0;
	UINT8 COUNT = 0;
	UINT32 Addr = 0;
	UINT32 Size = 0;
	UINT8 xdata filename[64];	
	INIT_CPU(); 	
	CH376_PORT_INIT();
	UART5_Init();
	memset(filename, 0, sizeof(filename));
	memset(FileDataBuf, 0, sizeof(FileDataBuf));	
	strcpy(filename, "T5L51*");		 ///FILEEE.TXT
	//strcpy(filename, "/FILEEE.TXT");
	/*
	res = CH376HostInit();
	res = FindDWINFile(filename);
	UART5_SendString(filename);
	//EA = 1;
	//res = CH376ReadFile(filename, FileDataBuf);
	//UART5_SendString("end\n");
	//UART5_Sendbyte(res);
	//UART5_SendString(filename);
	/*
	if (0)
	{
		res = CH376WriteFile(filename, FileDataBuf, WRITE_FROM_END);
		UART5_Sendbyte(res);	
	}
	else  res = CH376ReadFile(filename);
	CH376CloseFile(1);
	PRINT_EN = 1;*/
	//memset(filename, 0, sizeof(filename));	
	//WriteDGUS(0x0f, filename, 4);
	//ReadDGUS(0x0f, filename, 2);
	//for (i = 0; i < 2; i++) UART5_Sendbyte(filename[i]);
	//Addr = 0x43A;
	//Size = 8;
	//ReadDGUS(Addr, FileDataBuf, Size);
	//UART5_Sendbyte(FileDataBuf[1]);
	//for(i = 0; i < Size; i++)
	//	UART5_Sendbyte(FileDataBuf[i]);
	res = CH376HostInit();
	//UART5_Sendbyte(res);
	SystemUpdate(FILE_T5L51_BIN);
	while(1);	
	//res = CH376TouchDir(filename);			/* ����Ŀ¼ */
	//res = CH376TouchNewFile(filename);		/* �����ļ� */
	//res = CH376FileDeletePath(filename);		/* ɾ���ļ� */
	return 0;
}

void T0_ISR_PC(void)	interrupt 1
{
	EA = 0;
	TIMES++;
	TH0 = T1MS >> 8;
    TL0 = T1MS;
	if (TIMES == 100)
	{
		PRINT_EN++;
		TIMES = 0;
	}		
	EA = 1;
}