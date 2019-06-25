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
//UINT8 xdata FileDataBuf[0x1000];
UINT16 TIMES = 0;
FAT_NAME MatchLish[DIR_FILE_MAX] = {0};
int main()
{
	
	FAT_DIR_INFO xdata Dir;
	UINT8 j = 0;
	UINT32 i = 0;
	UINT8 COUNT = 0;
	UINT32 Addr = 0;
	UINT32 Size = 0;
	UINT8 xdata Buf[BUF_SIZE];
	UINT32 Buf_Size = 0;
	UINT8 xdata filename[64];	
	INIT_CPU(); 	
	CH376_PORT_INIT();
	UART5_Init();
	for (i = 0; i < DIR_FILE_MAX; i++)
		memset(MatchLish[i].NAME, 0, sizeof(MatchLish[i].NAME));
	memset(filename, 0, sizeof(filename));
	memset(Buf, 0, sizeof(Buf));
	memset(&Dir, 0, sizeof(FAT_DIR_INFO));
	//memset(FileDataBuf, 0, sizeof(FileDataBuf));	
	//strcpy(filename, "T5L51*");		 ///FILEEE.TXT
	strcpy(filename, "/DWIN_SET/T5L51.BIN");
	//ReadDGUS(0x1000, filename, 2);
	//UART5_Sendbyte(filename[1]);
	//UART5_Sendbyte(filename[1]);
	//res = CH376USBInit();
	//SystemUpdate(FILE_T5L51_BIN);
	Dir.DIR_Attr = 0x21;
	CH376USBInit();
	//CH376ReadFile(filename, Buf, &Buf_Size, 0);
	//UART5_Sendbyte(SetFileMessage(filename, &Dir));
	GetFileMessage(filename, &Dir);
	//UART5_Sendbyte((UINT8)(Dir.DIR_FileSize >> 24));
	//UART5_Sendbyte((UINT8)(Dir.DIR_FileSize >> 16));
	UART5_Sendbyte((UINT8)(Dir.DIR_CrtDate >> 8));
	UART5_Sendbyte((UINT8)(Dir.DIR_CrtDate));
	/*CH376MatchFile("13*", "/DWIN_SET", MatchLish);
	
	for (i = 0; i < DIR_FILE_MAX; i++)
	{
		UART5_SendString(MatchLish[i].NAME);
		UART5_Sendbyte('\n');
	}*/

	//FindDWINFile(filename, "BIN");
	//CH376ReadFile(filename, Buf, &Buf_Size, 2);
	//SendString(Buf, Buf_Size);
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