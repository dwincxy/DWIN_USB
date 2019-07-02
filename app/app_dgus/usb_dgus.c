/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : usb_dgus.c
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��27��
��������   : USB�ӿ���صĵ�DGUSӦ�ó���ʵ��
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/

#include "usb_dgus.h"
#include "app/app_usb/app_interface.h"
#include "driver/dgus/dgus.h"
#include "driver/uart/uart.h"
#include "string.h"


/********************************�궨��***************************************/
/* USB DGUS�Ĵ�����ַ */
#define DGUS_ADDR_GET_OR_SET_PATH		(0x5C0)
#define DGUS_ADDR_CREATE_OR_DEL_PATH	(0x5C4)
#define DGUS_ADDR_READ_OR_WRITE_FILE	(0x5C8)
#define DGUS_ADDR_SEARCH_FILE			(0x5D0)
#define DGUS_ADDR_SYSTEM_UP				(0x5D4)
/* USB ������������ */
#define ACK_GET_OR_SET_PATH				(0x05)
#define ACK_CREATE_OR_DEL_PATH			(0x04)
#define ACK_READ_OR_WRITE_FILE			(0x03)
#define ACK_SEARCH_FILE					(0x02)
#define ACK_SYSTEM_UP					(0x01)	
/* ��־λ���� */
#define FLAG_START						(0x5A)
#define FLAG_END						(0x00)
#define FLAG_READ						(0x5A)
#define FLAG_WRITE						(0xA5)
#define FLAG_CREATE						(0x5A)
#define FLAG_DELETE						(0xA5)
#define TYPE_FILE						(0x55)
#define TYPE_DIR 						(0xAA)

#define MATCH_LIST_NUM					(0x28)
#define MATCH_LIST_LEN					(0x280)
#define MATCH_STRING_LEN				(0x10)
#define PATH_LENGTH						(0x80)
#ifndef BUF_SIZE
#define BUF_SIZE						(0x1000)
#endif

		
/*****************************************************************************
 �� �� ��  : USBModule
 ��������  : Ψһ����ӿ� �Զ�ɨ��DGUS�����־��ִ����Ӧ����
 �������  : ��	 
 �������  : ��
 �޸���ʷ  :
 ��    ��  : 2019��6��28��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
void USBModule(void)
{
	UINT8 ACK = 0;
	/* (1) ɨ��DGUS������־λ ȷ����ӦӦ���־ ��ɨ��ļĴ��� ִ�����ȼ��� */
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_GET_OR_SET_PATH, FLAG_READ) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_GET_OR_SET_PATH, FLAG_WRITE))
		ACK = ACK_GET_OR_SET_PATH;
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_CREATE_OR_DEL_PATH, FLAG_CREATE) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_CREATE_OR_DEL_PATH, FLAG_DELETE))
		ACK = ACK_CREATE_OR_DEL_PATH;
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_READ_OR_WRITE_FILE, FLAG_READ) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_READ_OR_WRITE_FILE, FLAG_WRITE))
		ACK = ACK_READ_OR_WRITE_FILE;
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SEARCH_FILE, FLAG_START))
		ACK = ACK_SEARCH_FILE;
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SYSTEM_UP, FLAG_START))
		ACK = ACK_SYSTEM_UP;

	/* (2) Ӧ����Ӧ */
	switch (ACK)
	{
		case ACK_SYSTEM_UP:
		{
			//AckSystemUp();
			break;
		}
		case ACK_SEARCH_FILE:
		{
			AckSearchFile();
			break;
		}
		case ACK_READ_OR_WRITE_FILE:
		{
			AckReadOrWriteFile();
			break;
		}
		case ACK_CREATE_OR_DEL_PATH:
		{
			AckCreateOrDelPath();
			break;
		}
		case ACK_GET_OR_SET_PATH:
		{
			AckGetOrSetPath();
			break;
		}
		default:
			break;
	}
}
/*****************************************************************************
 �� �� ��  : CompareDgusRegValue
 ��������  : �Ƚ�DGUS�Ĵ������ֽڵ������Ƿ��Value���
 �������  : UINT32 AddrDgus	DGUS�Ĵ�����ַ
 			 UINT8 Value		���Ƚϵ��ֽ� 	 
 �������  : DWIN_OK    ��ֵ���
 			 DWIN_ERROR ��ֵ����
 �޸���ʷ  :
 ��    ��  : 2019��6��28��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 CompareDgusRegValue(UINT32 AddrDgus, UINT8 Value)
{
	UINT8 DgusValue = 0;
	ReadDGUS(AddrDgus, &DgusValue, 1);
	if (DgusValue == Value) return DWIN_OK;
	else return DWIN_ERROR;
}
/*****************************************************************************
 �� �� ��  : ReadDgusClientString
 ��������  : ��ȡDGUS�ͻ��˵������ַ� ����ȥ��DGUS������־���ַ������ַ����� 
 �������  : UINT32 AddrDgus	DGUS�Ĵ�����ַ
 			 PUINT8 pData		DGUS���ݵĽ��ջ�����
			 PUINT16 pDataLen	��ȡǰ����ȡ���� ��ȡ��DGUS���ݵ�ʵ�ʳ��� 	 
 �������  : ��
 �޸���ʷ  :
 ��    ��  : 2019��6��28��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
void ReadDgusClientString(UINT32 AddrDgus, PUINT8 pData, PUINT16 pDataLen)
{
	UINT8 i = 0, Data1 = 0, Data2 = 0;
	UINT16 Len = 0;
	Len = *pDataLen;
	ReadDGUS(AddrDgus, pData, *pDataLen);
	for (i = 0; i < *pDataLen; i++)	/* �˴�ֻ����������� ����������� DGUS�洢���ݲ����Զ����� */
	{
		if (pData[i] == 0xFF && pData[i + 1] == 0xFF)
		{
			*pDataLen = i;
		}
	}
	if (*pDataLen == Len) *pDataLen = strlen(pData);
}
/*****************************************************************************
 �� �� ��  : WriteDgusClientString
 ��������  : ��DGUS�ͻ���д������ д�������ݻ��������־ д���ʵ�ʳ��Ȼ�+2
 �������  : UINT32 AddrDgus	DGUS�Ĵ�����ַ
 			 PUINT8 pData		DGUS���ݵ�д�뻺����
			 UINT16 DataLen		д�볤�� 	 
 �������  : ��
 �޸���ʷ  :
 ��    ��  : 2019��6��28��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
void WriteDgusClientString(UINT32 AddrDgus, PUINT8 pData, UINT16 DataLen)
{
	/* д��0x00 0x00 ������־������DGUS�ͻ�����ʾ */
	pData[DataLen++] = 0x00;
	pData[DataLen++] = 0x00;
	WriteDGUS(AddrDgus, pData, DataLen);
	
}

void AckCreateOrDelPath(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 Mod = 0, FileType = 0;
	UINT16 PathLength = 0;
	UINT32 AddrDgusPath = 0;
	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	/* (1) ��ȡ������ */
	ReadDGUS(DGUS_ADDR_CREATE_OR_DEL_PATH, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	FileType = Cmd[2];
	AddrDgusPath = (Cmd[3] << 8) | Cmd[4];
	PathLength = PATH_LENGTH;
	/* (2) ��ȡ·���� */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	Path[PathLength + 1] = 0;
	/* (3) ���ݿ�����ִ�д���/ɾ�� �ļ�/Ŀ¼�Ĳ��� */
	switch (Mod)
	{
		case FLAG_CREATE:
		{
			Cmd[1] = CreateFileOrDir(Path, FileType);
			break;
		}
		case FLAG_DELETE:
		{
			Cmd[1] = RmFileOrDir(Path);
			break;
		}
		default:
			break;
	}
	/* (4) д���� */
	Cmd[0] = 0;			/* �����־λ */
	WriteDGUS(DGUS_ADDR_CREATE_OR_DEL_PATH, Cmd, sizeof(Cmd));
}

void AckSearchFile(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 xdata AimString[MATCH_STRING_LEN];
	UINT8 xdata MatchLish[MATCH_LIST_LEN];
	PUINT8 pMatch = NULL;
	UINT8 Mod = 0, Status = 0, MatchNumber = 0, i = 0;
	UINT16 PathLength = 0, AimStringLen = 0;
	UINT32 AddrDgusPath = 0, AddrDgusAimString = 0, AddrDgusMatchResult = 0;

	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	memset(AimString, 0, MATCH_STRING_LEN);
	memset(MatchLish, 0, MATCH_LIST_LEN);
	PathLength = PATH_LENGTH;
	AimStringLen = MATCH_STRING_LEN;
	/* (1) ��ȡ������ */
	ReadDGUS(DGUS_ADDR_SEARCH_FILE, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	AddrDgusPath = (Cmd[2] << 8) | Cmd[3];
	AddrDgusAimString = (Cmd[4] << 8) | Cmd[5];
	AddrDgusMatchResult = (Cmd[6] << 8) | Cmd[7];
	WriteDGUS(AddrDgusMatchResult, MatchLish, MATCH_LIST_LEN);
	/* (2) ��ȡ·���� */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	Path[PathLength + 1] = 0;
	/* (3) ��ȡƥ���ַ� */
	ReadDgusClientString(AddrDgusAimString, AimString, &AimStringLen);
	AimString[AimStringLen] = 0;
	/* (4) ��ʼƥ�� */
	Status = MatchFile(Path, AimString, MatchLish);
	/* (5) ��ȡƥ������ */
	pMatch = MatchLish;
	for (i = 0; i < MATCH_LIST_NUM; i++)
	{
		if (*pMatch == 0) break;
		pMatch += MATCH_LIST_LEN / MATCH_LIST_NUM;
		MatchNumber++;
	}
	/* (6) ����ƥ���� */
	Cmd[0] = 0;			/* �����־λ */
	Cmd[1] = MatchNumber;
	WriteDGUS(DGUS_ADDR_SEARCH_FILE, Cmd, sizeof(Cmd));
	/* ����д�뻺�����Ѿ������������ʼ�� ������д������־0x00 0x00������DGUS�ͻ�����ʾ */
	WriteDGUS(AddrDgusMatchResult, MatchLish, (MatchNumber * (MATCH_LIST_LEN / MATCH_LIST_NUM)));
}

void AckReadOrWriteFile(void)
{
	UINT8 xdata Cmd[16];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 xdata Buf[BUF_SIZE];
	UINT8 Mod = 0, Status = 0;
	UINT16 PathLength = 0, ByteSize = 0;
	UINT32 AddrDgusPath = 0, AddrDgusFileMsg = 0, SectorOffset = 0;
	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	memset(Buf, 0, BUF_SIZE);
	PathLength = PATH_LENGTH;
	/* (1) ��ȡ������ */
	ReadDGUS(DGUS_ADDR_READ_OR_WRITE_FILE, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	AddrDgusPath = (Cmd[2] << 8) | Cmd[3];
	AddrDgusFileMsg = (Cmd[4] << 8) | Cmd[5];
	SectorOffset = (Cmd[6] << 16) | (Cmd[7] << 8) | Cmd[8];
	ByteSize = (Cmd[9] << 8) | Cmd[10];
	/* (2) ��ȡ�ļ�·�� */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	Path[PathLength + 1] = 0;
	/* (3) ���ݱ�־λ�����ļ���ȡ�����ļ�д�� */
	switch (Mod)
	{
		case FLAG_READ:
		{
			UART5_SendString(Path);
			Status = ReadFile(Path, Buf, ByteSize, SectorOffset);
			UART5_SendString(Buf);
			WriteDgusClientString(AddrDgusFileMsg, Buf, ByteSize);
			if (Status == DWIN_OK) Cmd[1] = 0x00;
			else Cmd[1] = 0xFF;
			break;
		}
		case FLAG_WRITE:
		{
			ReadDgusClientString(AddrDgusFileMsg, Buf, &ByteSize);
			Status = WriteFile(Path, Buf, ByteSize, SectorOffset);
			if (Status == DWIN_OK) Cmd[1] = 0x00;
			else Cmd[1] = 0xFF;
			break;
		}
		default:
		{
			Cmd[1] = 0xFF;
			break;
		}
	}
	/* (6) ����ִ�н�� */
	Cmd[0] = 0x00;		/* �����־λ */
	WriteDGUS(DGUS_ADDR_READ_OR_WRITE_FILE, Cmd, sizeof(Cmd));
}

void AckGetOrSetPath(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 xdata DirName[sizeof(DIR_TYPE)];
	UINT8 Mod = 0, Status = 0;
	UINT16 PathLength = 0;
	UINT32 AddrDgusPath = 0, AddrDgusDirAttr = 0;
	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	memset(DirName, 0, sizeof(DirName));
	PathLength = PATH_LENGTH;
	/* (1) ��ȡ������ */
	ReadDGUS(DGUS_ADDR_GET_OR_SET_PATH, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	AddrDgusPath = (Cmd[2] << 8) | Cmd[3];
	AddrDgusDirAttr = (Cmd[4] << 8) | Cmd[5];
	/* (2) ��ȡ�ļ�·�� */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	Path[PathLength + 1] = 0;
	/* (3) ���ݱ�־λ�����ļ����Զ�ȡ��������д�� */
	switch(Mod)
	{
		case FLAG_READ:
		{
			Status = GetFileMessage(Path, DirName);
			WriteDGUS(AddrDgusDirAttr, DirName, sizeof(DirName));
			break;
		}
		case FLAG_WRITE:
		{
			ReadDGUS(AddrDgusDirAttr, DirName, sizeof(DirName));
			Status = SetFileMessage(Path, DirName);
			break;
		}
		default:
		{
			Status = DWIN_ERROR;
			break;
		}
	}
	/* (6) ����ִ�н�� */
	Cmd[0] = 0x00;		/* �����־λ */
	Cmd[1] = Status;
	WriteDGUS(DGUS_ADDR_GET_OR_SET_PATH, Cmd, sizeof(Cmd));
}