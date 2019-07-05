/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : app_interface.c
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��21��
��������   : CH376Ӧ�ó���ӿ�
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/
#include "app_interface.h"
#include "string.h"
#include "stdio.h"
#include "driver/usb/para_port.h"
#include "driver/usb/ch376.h"
#include "driver/uart/uart.h"
#include "driver/dgus/dgus.h"

/********************************���ں�������*********************************/

void Delay(void);

/*****************************************************************************
 �� �� ��  : Delay	�����ʱԼ 0.6ms
 ��������  : ��ʱ����
 �������  : ��	 
 �������  : ��
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
void Delay(void)
{
	UINT8 i, j;
	for(i = 0; i < 100; i++)
		for(j = 0; j < 100; j++);
}

/*****************************************************************************
 �� �� ��  : USBInit
 ��������  : ���CH376ͨѶ������USB����ģʽ�����̳�ʼ��
 �������  : ��	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 USBInit(void)
{
	UINT8 i = 0, Status = 0;
	xWriteCH376Cmd(CMD11_CHECK_EXIST);  	/* ���Ե�Ƭ����CH376֮���ͨѶ�ӿ� */
	xWriteCH376Data(0x65);
	/* ͨѶ�ӿڲ�����,����ԭ����:�ӿ������쳣,�����豸Ӱ��(Ƭѡ��Ψһ),���ڲ�����,һֱ�ڸ�λ,���񲻹��� */ 
	if (xReadCH376Data() != 0x9A)  return ERR_USB_UNKNOWN;

	xWriteCH376Cmd(CMD11_SET_USB_MODE);		/* �豸USB����ģʽ */
	xWriteCH376Data(USB_HOST_ON_NO_SOF);
	for (i=100; i!=0; i--) 
		if (xReadCH376Data() == CMD_RET_SUCCESS) break;
	if (0 == i) return ERR_USB_UNKNOWN;

	for (i = 0; i < 100; i ++)				/* ������״̬ */								   
	{
		Delay();
		Status = CH376DiskMount();
		if (USB_INT_SUCCESS == Status) break;
		if (ERR_DISK_DISCON == Status) break;
		if (CH376GetDiskStatus() >= DEF_DISK_MOUNTED && i >= 5) break;
	}
	return USB_INT_SUCCESS;
}

UINT8 CheckIC(void)
{
	UINT8 i = 0;
	xWriteCH376Cmd(CMD11_CHECK_EXIST);  	/* ���Ե�Ƭ����CH376֮���ͨѶ�ӿ� */
	xWriteCH376Data(0x65);
	/* ͨѶ�ӿڲ�����,����ԭ����:�ӿ������쳣,�����豸Ӱ��(Ƭѡ��Ψһ),���ڲ�����,һֱ�ڸ�λ,���񲻹��� */ 
	if (xReadCH376Data() != 0x9A)  return 0;
	else 
	{
		xWriteCH376Cmd(CMD11_SET_USB_MODE);		/* �豸USB����ģʽ */
		xWriteCH376Data(USB_HOST_ON_NO_SOF);
		for (i=100; i!=0; i--) 
			if (xReadCH376Data() == CMD_RET_SUCCESS) break;
		return 0x5A;
	}
}

UINT8 CheckConnect(void)
{
	if (CH376DiskConnect() != USB_INT_SUCCESS) return 0;
	else return 0x5A;
}

UINT8 CheckDiskInit(void)
{
	if (CH376DiskMount() != USB_INT_SUCCESS) return 0;
	else return 0x5A;
}
/*****************************************************************************
 �� �� ��  : CreateFileOrDir
 ��������  : �������ļ�����Ŀ¼,֧�ֶ༶·��
 �������  : PUINT8 pPathName	�ļ�����·����
 			 UINT8 TypePath		·������	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 CreateFileOrDir(PUINT8 pPathName, UINT8 TypePath)
{
	AlphabetTransfrom(pPathName);
	if (TypePath != PATH_FILE && TypePath != PATH_DIR) return DWIN_ERROR;
	return (CH376CreateFileOrDir(pPathName, TypePath));	
}
UINT8 CH376CreateFileOrDir(PUINT8 pPathName, UINT8 TypePath)
{
	UINT8 xdata NameBuf[PATH_NUMBER][12];
	UINT8 i = 0;
	UINT8 j = 0;
	UINT8 Status = 0;
	memset(NameBuf, 0, sizeof(NameBuf));
	/* (1) ���ļ�Ŀ¼��ֳɵ����ļ�����Ŀ¼ */	
	for (i = 0; i < PATH_NUMBER; i++)		
	{
		for (j = 0; j < 12; j++)
		{
			NameBuf[i][j] =  *pPathName++;
			if (*pPathName == '/' || *pPathName == 0) 
			{
				pPathName++;
				break;	
			}
		}
		if (*pPathName == 0) break;
	}
	/* (2) ��ÿһ��Ŀ¼���ļ� ���������½� ������֮��ͻ���ļ���Ŀ¼��ɾ�� */
	for (j = 0; j < i+1; j++)				
	{
		Status = CH376FileOpen(NameBuf[j]);
		if (j == i && TypePath == PATH_FILE)	
		{
			switch (Status)	/* ���һ��·��Ϊ�ļ� */
			{
				case ERR_MISS_FILE:				/* �������ļ���Ŀ¼������ */
				{
					while (USB_INT_SUCCESS != CH376FileCreate(NameBuf[j]));
					break;	
				}
				case ERR_OPEN_DIR:				/* ������Ŀ¼ */
				{
					if (USB_INT_SUCCESS != CH376DeleteFile(NameBuf[j])) return CH376Error();
					j = 0;
					break;
				}
				default:						/* �ļ����� */
					break;
			}
			continue;	
		}
		switch (Status)	/* �м�·��Ĭ��ΪĿ¼ */
		{
			case ERR_MISS_FILE:					/* �������ļ���Ŀ¼������ */
			{
				while (USB_INT_SUCCESS != CH376DirCreate(NameBuf[j]));
				break;	
			}
			case USB_INT_SUCCESS: 				/* �������ļ� */
			{
				while (USB_INT_SUCCESS == CH376DeleteFile(NameBuf[j]));
				j = 0;
				break;
			}
			default:							/* Ŀ¼���� */
				break;
		}
	}
	return DWIN_OK;
}
/*****************************************************************************
 �� �� ��  : RmFileOrDir
 ��������  : ɾ���ļ�����Ŀ¼,֧�ֶ༶·��
 �������  : PUINT8 pPathName �ļ�����·����	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 RmFileOrDir(PUINT8 pPathName)
{
	AlphabetTransfrom(pPathName);
	if (USB_INT_SUCCESS == CH376FileDeletePath(pPathName)) return DWIN_OK;
	else return DWIN_ERROR;
}
/*****************************************************************************
 �� �� ��  : ReadFile
 ��������  : ��ȡ�ļ���Ϣ	   (��ȡ��С���ļ��ܴ�С��ʱ��ֻ���ȡ�ļ�����ƫ�ƺ��ܴ�С)
 �������  : PUINT8 pPathName  �ļ�����·����
             PUINT8 pBuf       ���������ݳ���
			 UINT16 DataLen    ��ȡ���ݵĳ���
			 UINT32 SectorOffset ��ȡ����ʼ����λ��	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 ReadFile(PUINT8 pPathName, PUINT8 pData, UINT16 DataLen, UINT32 SectorOffset)	/* ��ȡ�ļ���Ϣ */	
{	/* �ַ��洢������pBuf 4096�ֽ� = 8������ */
	UINT8 Status = 0;
	UINT32 SectorCount = 0, Count = 0;
	UINT32 FileSize = 0;
	if (NULL == pPathName)	return DWIN_NULL_POINT;
	AlphabetTransfrom(pPathName);
	memset(pData, 0, BUF_SIZE);	
	/* (1) ����ļ��������,��ȡ�ļ����ȺͿɶ�ȡ������ */
	Status = CH376FileOpenPath(pPathName);	
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	FileSize = CH376GetFileSize();
	if (FileSize == 0)	return CH376Error();	/* ���ļ� */
	/* (2) ��⽫��ȡ���ֽ���λ���Ƿ�����ļ����ֽ��� ���������ֱ�ӷ��� */
	if ((SectorOffset != 0) &&(((FileSize >> 9)) < (SectorOffset - 1))) return CH376Error();
	/* (3) ��⽫��ȡ���ֽ����Ƿ�����ļ�ƫ�ƺ���ֽ��� ������ֱ��ȡʣ����ļ��ֽ�����Ϊ��ȡ�� */
	if ((FileSize + 512) < ((SectorOffset << 9) + DataLen)) FileSize = FileSize - (SectorOffset << 9);
	else  FileSize = (UINT32)DataLen;
	CH376SecLocate(SectorOffset);				/* ��������ƫ�� */
	while (FileSize)
	{
	/* (4) ���ö�ȡ������ */
		/* �Ƿ����β����ͷ���� ��������ȡһ������ */
		if (FileSize % DEF_SECTOR_SIZE) SectorCount = (FileSize >> 9) + 1;	
		else SectorCount = (FileSize >> 9);
		/* һ��ֻ�ܶ�ȡ8������ ������ȡ8 */
		if (SectorCount > (BUF_SIZE / DEF_SECTOR_SIZE))
		{
			Count = BUF_SIZE / DEF_SECTOR_SIZE;
			FileSize -= BUF_SIZE;
		} 
		else 
		{
			Count = SectorCount;
			FileSize = 0;
		}
	/* (5) �ļ����ݶ�ȡ */	
		Status = CH376SectorRead(pData, (UINT8)Count, NULL);	
		if (Status != USB_INT_SUCCESS) return CH376Error();
	}
	CH376CloseFile(0);
	return DWIN_OK;
	
	
}
/*****************************************************************************
 �� �� ��  : WriteFile
 ��������  : ���ļ������������½�
 �������  : PUINT8 pPathName  �ļ�����·����
             PUINT8 pBuf       ���������ݳ���
			 UINT8 Flag        д���־λ: ���ļ�ͷ��д��/���ļ�β��д��	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 WriteFile(PUINT8 pPathName, PUINT8 pData, UINT16 DataLen, UINT32 SectorOffset)	/* д���ļ������������½� */
{
	UINT8 xdata Buf[BUF_SIZE];					/* �ַ��洢������ 4096�ֽ� = 8������ */
	UINT8 xdata EndBuf[DEF_SECTOR_SIZE];
	UINT8 Status = 0, SectorCount = 0, Flag = 0;
	PUINT8 pMid = NULL; 
	UINT16 BufFreeLen = 0, BufSourceLen = 0, EndBufSize = 0;
	UINT32 FileSize = 0;

	memset(Buf, 0, BUF_SIZE);
	memset(EndBuf, 0, DEF_SECTOR_SIZE);
	if ((NULL == pPathName) || (NULL == pData)) return DWIN_ERROR;
	AlphabetTransfrom(pPathName);
	if (SectorOffset == 0) Flag = WRITE_FROM_HEAD;
	else Flag = WRITE_FROM_END;
	
	/* (1) ����ļ�������� ���������½��ļ� */
	Status = CH376FileOpenPath(pPathName);
	if (Status != USB_INT_SUCCESS) 
	{
		CH376CreateFileOrDir(pPathName, PATH_FILE);
		Status = CH376FileOpenPath(pPathName);
		if (Status != USB_INT_SUCCESS) return CH376Error();
		Flag = 	WRITE_FROM_HEAD;
	}
	/* (2) ���ݱ�־����ѡ��д��ʽ ���ļ���ʼ/���ļ���β */
	switch (Flag)
	{
		case WRITE_FROM_HEAD:
		{
			CH376SecLocate(0);
			break;
		}
		case WRITE_FROM_END:	/* ������β��������Ҫ������ƴ��д�� */
		{
			FileSize = CH376GetFileSize();
			EndBufSize = FileSize % DEF_SECTOR_SIZE;
			if (EndBufSize)		/* �Ƿ����β����ͷ���� �����ܷ�����512�ж� */ 
			{
				Status = CH376SectorRead(EndBuf, 1, NULL);
			}
			CH376SecLocate(0xFFFFFFFF);
			break;
		}
		default:
			return CH376Error();
	}
	FileSize += DataLen;
	if (EndBufSize != 0)		/* ����ͷ���� ����pData��ϳ����ݰ� дһ�β�����4K������ */ 
	{
		strncpy(Buf, EndBuf, EndBufSize);			
		BufFreeLen = BUF_SIZE - EndBufSize;
		pMid = Buf;
		pMid += EndBufSize;
		if (DataLen > BufFreeLen)
		{
			strncpy(pMid, pData, BufFreeLen);
			SectorCount = BUF_SIZE / DEF_SECTOR_SIZE;
			DataLen -= BufFreeLen;
			pData += BufFreeLen;	
		}
		else
		{
			strncpy(pMid, pData, DataLen);
			SectorCount = (EndBufSize + DataLen) / DEF_SECTOR_SIZE;			/* ������ �������������������������+1 */
			if ((EndBufSize + DataLen) % DEF_SECTOR_SIZE) SectorCount++;	/* �����������ȡ�� ������㲹�� ������+1 */
			DataLen = 0;
		}
		Status = CH376SectorWrite(Buf, SectorCount, NULL);
		memset(Buf, 0, BUF_SIZE);	/* д�꽫51���������� */
	}
	/* (3) ѭ��д���� */
	while(DataLen)
	{
		if (DataLen > BUF_SIZE)
		{
			strncpy(Buf, pData, BUF_SIZE);
			SectorCount = BUF_SIZE / DEF_SECTOR_SIZE;
			DataLen -= BUF_SIZE;
			pData += BUF_SIZE;	
		}
		else
		{
			strncpy(Buf, pData, DataLen);
			SectorCount = DataLen / DEF_SECTOR_SIZE;		/* ������ �������������������������+1 */
			if (DataLen % DEF_SECTOR_SIZE) SectorCount++;	/* �����������ȡ�� ������㲹�� ������+1 */
			DataLen = 0;
		}
		Status = CH376SectorWrite(Buf, SectorCount, NULL);
		memset(Buf, 0, BUF_SIZE);				/* ��������� */
	}
	CH376WriteVar32(VAR_FILE_SIZE, FileSize);	/* ����ȷ�ĵ�ǰ�ļ�����д��CH376�ڴ� */
	Status = CH376SectorWrite(pData, 0, NULL);	/* д0����,ʵ����ˢ���ļ����� �ѻ�������������д��USB */
	CH376CloseFile(0);
	return DWIN_OK;
}

UINT8 MatchFile(PUINT8 pDir,PUINT8 pMatchString, PUINT8 pBuf)
{
	UINT8 Status = 0, i = 0;
	if(pBuf == NULL) return DWIN_ERROR;
	AlphabetTransfrom(pDir);
	AlphabetTransfrom(pMatchString);
	Status = CH376MatchFile(pMatchString, pDir, (P_FAT_NAME)pBuf);
	return (Status == USB_INT_SUCCESS ? DWIN_OK : DWIN_ERROR);
}

void SysUpGetFileMesg(UINT8 FileType, UINT8 FileNumber, PUINT8 pUpSpace, PUINT32 pFileAddr, PUINT8 pString)
{
	switch (FileType)
	{
		case FILE_T5L51_BIN:
		{
			strcpy(pString, "T5L51");
			pString += 6;
			strcpy(pString, ".BIN");
			*pFileAddr = ADDR_T5L51_BIN;
			*pUpSpace = SPACE_1;
			break;
		}

		case FILE_DWINOS_BIN:
		{
			strcpy(pString, "DWINOS");
			pString += 6;
			strcpy(pString, ".BIN");
			*pFileAddr = ADDR_DWIN_OS;
			*pUpSpace = SPACE_1;
			break;
		}

		case FILE_XXX_LIB:
		{
			sprintf(pString, "%d", (UINT16)FileNumber);
			pString += 6;
			strcpy(pString, ".LIB");
			*pFileAddr = LIB((UINT32)FileNumber);
			*pUpSpace = SPACE_1;
			break;
		}
		case FILE_XXX_BIN:
		{
			sprintf(pString, "%d", (UINT16)FileNumber);
			pString += 6;
			strcpy(pString, ".BIN");
			pString += 6;
			strcpy(pString, ".DZK");
			pString += 6;
			strcpy(pString, ".HZK");
			*pFileAddr = FONT((UINT32)FileNumber);
			*pUpSpace = SPACE_2;
			break;
		}
		case FILE_XXX_ICL:
		{
			sprintf(pString, "%d", (UINT16)FileNumber);
			pString += 6;
			strcpy(pString, ".ICL");
			*pFileAddr = ICL((UINT32)FileNumber);
			*pUpSpace = SPACE_2;
			break;
		}
		default:
			break;
	}
}

UINT8 SysUpGetDWINFile(PUINT8 pMatchList)
{
	UINT8 Status = 0;
	UINT8 i = 0;
	Status = CH376MatchFile("*", DWIN_DIR, (P_FAT_NAME)pMatchList);
	if (Status == ERR_MISS_FILE) return DWIN_OK; 
	else return DWIN_ERROR;
}

UINT8 SysUpFileMatch(PUINT8 pSource, PUINT8 pDest, PUINT8 pResult, PUINT32 pFileSize)
{
	UINT8 i = 0;
	PUINT8 pNow = pDest;
	PUINT8 pStatus = NULL;
	for (; *pSource != 0; pSource += sizeof(FAT_NAME))
	{
		i = 0;
		for (pNow = pDest; *pNow != 0; pNow += 6)
		{
			pStatus = strstr(pSource, pNow);
			if (pStatus != NULL) i++;
			if (i == 0 && pStatus == NULL) break;
			if (i == 2)
			{
				sprintf(pResult, "%s/%s", DWIN_DIR, pSource);
				pSource += sizeof(((FAT_NAME *)0) -> NAME);
				*pFileSize = *pFileSize | *pSource++;
				*pFileSize = *pFileSize | ((UINT16)(*pSource++) << 8);
				*pFileSize = *pFileSize | ((UINT32)(*pSource++) << 16);
				*pFileSize = *pFileSize | ((UINT32)(*pSource) << 24);	
				return DWIN_OK;
			}
		}
	}
	return DWIN_ERROR;
}

void SysUpWaitOsFinishRead(UINT32 AddrDgus)
{
	UINT8 Flag = 0;
	do
	{
		ReadDGUS(AddrDgus, &Flag, 1);
	}
	while(Flag == FLAG_EN);
}

void SysUpPcakSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize)
{
	*pBuf++ = Flag_EN;					/* ������־ */
	*pBuf++ = UpSpace;					/* �����ռ�ѡ�� */
	*pBuf++ = (UINT8)(UpAddr >> 24);	/* Զ������Ŀ���ַ */
	*pBuf++ = (UINT8)(UpAddr >> 16);
	*pBuf++ = (UINT8)(UpAddr >> 8);
	*pBuf++ = (UINT8)(UpAddr);
	*pBuf++ = (UINT8)(FileSize >> 8);	/* �����ֽڳ��� 0x0001 - 0x0FFF */
	*pBuf++ = (UINT8)(FileSize); 		
	*pBuf++ = 0x00;						/* Ĭ�ϲ�����CRCУ�� */
	*pBuf = 0x00;
}

void SendUpPackToDGUS(UINT32 AddrDgusHead, UINT32 AddrDgusMesg, PUINT8 BufHead, PUINT8 BufMesg, UINT16 MesgSize)
{
	SysUpWaitOsFinishRead(AddrDgusHead);
	WriteDGUS(AddrDgusHead, BufHead, 10);
	WriteDGUS(AddrDgusMesg, BufMesg, MesgSize);
	if (*BufHead != FLAG_EN)
	{
		*BufHead = FLAG_EN;
		SysUpWaitOsFinishRead(AddrDgusHead);
		WriteDGUS(AddrDgusHead, BufHead, 10);
	}
}

void SysUpFileSend(PUINT8 pPath, UINT8 UpSpace, UINT32 AddrDgusPck,UINT32 AddrFileSave, UINT32 FileSize)
{
	UINT8 xdata BufHead[CONTROL_SIZE];
	UINT8 xdata BufMesg[BUF_SIZE];
	UINT8 Count = 0;
	UINT16 PackSize = 0, FirstPackSize = 0;
	UINT32 SectorOffset = 0, FirstAddrFileSave = 0;
	UINT32 AddrDgusPackHead = 0, AddrDgusPackMesg = 0;
	memset(BufHead, 0, sizeof(BufHead));
	memset(BufMesg, 0, sizeof(BufMesg));
	AddrDgusPackHead = AddrDgusPck;
	AddrDgusPackMesg = AddrDgusPck + (CONTROL_SIZE / 2);
	if(FileSize > BUF_SIZE) FirstPackSize = BUF_SIZE;
	else 
	{
		FirstPackSize = (UINT16)FileSize;
		FileSize = 0;
	}
	FirstAddrFileSave = AddrFileSave;
	/* Send Pack */
	while(FileSize)
	{
		if (FileSize > BUF_SIZE) 
		{
			PackSize = BUF_SIZE;
			FileSize -= BUF_SIZE;
		}
		else
		{
			PackSize = (UINT16)FileSize;
			FileSize = 0;
		}
		SysUpPcakSet(BufHead, FLAG_NO_EN, UpSpace, AddrFileSave, PackSize);
		ReadFile(pPath, BufMesg, PackSize, SectorOffset);
		if (Count == 0) 
		{
			memset(BufMesg, 0, 16); /* FirstPack Head Clean, Because of .ICL's scaning */
			Count = 1;
		}
		SendUpPackToDGUS(AddrDgusPackHead, AddrDgusPackMesg, BufHead, BufMesg, PackSize);	
		memset(BufMesg, 0, sizeof(BufMesg));
		AddrFileSave += BUF_SIZE;		
		SectorOffset += BUF_SIZE / DEF_SECTOR_SIZE;
	}
	SysUpPcakSet(BufHead, FLAG_NO_EN, UpSpace, FirstAddrFileSave, FirstPackSize);
	ReadFile(pPath, BufMesg, FirstPackSize, 0);
	SendUpPackToDGUS(AddrDgusPackHead, AddrDgusPackMesg, BufHead, BufMesg, FirstPackSize);
}

UINT8 SystemUpdate(UINT8 FileType, UINT8 FileNumber)
{
	UINT8 xdata String[24]; //4 * 6
	UINT8 xdata FilePath[22];
	UINT8 xdata FileList[sizeof(FAT_NAME) * DIR_FILE_MAX];
	UINT8 UpSpace = 0;
	UINT32 AddrFile = 0, FileSize = 0, AddrDgusPack = 0;
	memset(String, 0, sizeof(String));
	memset(FilePath, 0, sizeof(FilePath));
	memset(FileList, 0, sizeof(FileList));
	
	ReadDGUS(ADDR_UP_CONFIG, FilePath, 4);
	AddrDgusPack = ((UINT16)FilePath[3] << 8) | 0x00;
	SysUpGetFileMesg(FileType, FileNumber, &UpSpace, &AddrFile, String);
	SysUpGetDWINFile(FileList);
	SysUpFileMatch(FileList, String, FilePath, &FileSize);
	UART5_SendString(FilePath);
	SysUpFileSend(FilePath, UpSpace, AddrDgusPack, AddrFile, FileSize);
	return DWIN_OK;
}

UINT8 GetFileMessage(PUINT8 pFilePath, PUINT8 pBuf)
{
	UINT8 Status = 0;
	P_DIR_TYPE pDir = NULL;
	UINT8 xdata FatDir[sizeof(FAT_DIR_INFO)];
	P_FAT_DIR_INFO pFatDir = (P_FAT_DIR_INFO)FatDir;

	memset(FatDir, 0, sizeof(FAT_DIR_INFO));
	AlphabetTransfrom(pFilePath);
	Status = CH376GetFileMessage(pFilePath, (P_FAT_DIR_INFO)FatDir);
	pDir = (P_DIR_TYPE)pBuf;
	
	pDir -> DIR_Attr     = pFatDir -> DIR_Attr;
	pDir -> DIR_CrtTime  = pFatDir -> DIR_CrtTime;
	pDir -> DIR_CrtDate  = pFatDir -> DIR_CrtDate;
	pDir -> DIR_WrtTime  = pFatDir -> DIR_WrtTime;
	pDir -> DIR_WrtDate  = pFatDir -> DIR_WrtDate;
	pDir -> DIR_FileSize = pFatDir -> DIR_FileSize;
	return Status;
}

UINT8 SetFileMessage(PUINT8 pFilePath, PUINT8 pBuf)
{
	UINT8 Status = 0;
	P_DIR_TYPE pDir = NULL;
	UINT8 xdata FatDir[sizeof(FAT_DIR_INFO)];
	P_FAT_DIR_INFO pFatDir = (P_FAT_DIR_INFO)FatDir;

	memset(FatDir, 0, sizeof(FAT_DIR_INFO));
	AlphabetTransfrom(pFilePath);
	pDir = (P_DIR_TYPE)pBuf;
	pFatDir -> DIR_Attr     = pDir -> DIR_Attr;
	pFatDir -> DIR_CrtTime  = pDir -> DIR_CrtTime;
	pFatDir -> DIR_CrtDate  = pDir -> DIR_CrtDate;
	pFatDir -> DIR_WrtTime  = pDir -> DIR_WrtTime;
	pFatDir -> DIR_WrtDate  = pDir -> DIR_WrtDate;
	pFatDir -> DIR_FileSize = pDir -> DIR_FileSize;
	Status = CH376SetFileMessage(pFilePath, (P_FAT_DIR_INFO)FatDir);
	return Status;

}