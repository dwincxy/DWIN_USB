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
/********************************���ں�������*********************************/
UINT8 SenderBuf(PUINT8 pBuf, UINT8 FileFlag, UINT32 BufSize);
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
 �� �� ��  : CH376USBInit
 ��������  : ���CH376ͨѶ������USB����ģʽ�����̳�ʼ��
 �������  : ��	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 CH376USBInit(void)
{
	UINT8 i = 0, Status = 0;
	xWriteCH376Cmd(CMD11_CHECK_EXIST);  	/* ���Ե�Ƭ����CH376֮���ͨѶ�ӿ� */
	xWriteCH376Data(0x65);
	/* ͨѶ�ӿڲ�����,����ԭ����:�ӿ������쳣,�����豸Ӱ��(Ƭѡ��Ψһ),���ڲ�����,һֱ�ڸ�λ,���񲻹��� */ 
	if (xReadCH376Data() != 0x9A) return ERR_USB_UNKNOWN;

	xWriteCH376Cmd(CMD11_SET_USB_MODE);		/* �豸USB����ģʽ */
	xWriteCH376Data(USB_HOST_ON_NO_SOF);
	for (i=100; i!=0; i--) 
		if (xReadCH376Data() == CMD_RET_SUCCESS) break;
	if (0 == i) return ERR_USB_UNKNOWN;

	while ((Status = CH376DiskConnect()) != USB_INT_SUCCESS);

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
/*****************************************************************************
 �� �� ��  : CH376TouchNewFile
 ��������  : �������ļ�,֧�ֶ༶·��
 �������  : PUINT8 pPathName �ļ�����·����	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 CH376TouchNewFile(PUINT8 pPathName)
{
	AlphabetTransfrom(pPathName);
	return (CH376FileCreatePath(pPathName));	
}
/*****************************************************************************
 �� �� ��  : CH376RmFile
 ��������  : ɾ���ļ�����Ŀ¼,֧�ֶ༶·��
 �������  : PUINT8 pPathName �ļ�����·����	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 CH376RmFile(PUINT8 pPathName)
{
	AlphabetTransfrom(pPathName);
	return (CH376FileDeletePath(pPathName));
}
/*****************************************************************************
 �� �� ��  : CH376TouchDir
 ��������  : ��������Ŀ¼
 �������  : PUINT8 pPathName �ļ�����·����	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 CH376TouchDir(PUINT8 pPathName)
{
	AlphabetTransfrom(pPathName);
	return (CH376DirCreate(pPathName));	
}
/*****************************************************************************
 �� �� ��  : CH376ReadFile
 ��������  : ��ȡ�ļ���Ϣ
 �������  : PUINT8 pPathName  �ļ�����·����
             PUINT8 pBuf       ���������ݳ���
			 PUINT32 pFileSize �����ص��ļ�����	 
 �������  : USB_INT_SUCCESS �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 CH376ReadFile(PUINT8 pPathName, PUINT8 pBuf, PUINT32 pFileSize)	/* ��ȡ�ļ���Ϣ */	
{	/* �ַ��洢������pBuf 4096�ֽ� = 8������ */
	UINT8 Status = 0;
	UINT32 SectorCount = 0, Count = 0;

	if (NULL == pPathName)	return DWIN_NULL_POINT;
	AlphabetTransfrom(pPathName);
	memset(pBuf, 0, BUF_SIZE);	
	/* (1) ����ļ��������,��ȡ�ļ����ȺͿɶ�ȡ������ */
	Status = CH376FileOpenPath(pPathName);	
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	CH376SecLocate(0);
	*pFileSize = CH376GetFileSize();
	
	if (*pFileSize == 0)	return DWIN_ERROR;		/* ���ļ� */
	if (*pFileSize % DEF_SECTOR_SIZE)				/* �Ƿ����β����ͷ���� �����ܷ�����512�ж� */ 
	{
		SectorCount = (*pFileSize >> 9) + 1;		/* T5L��16λ ������32λ�˳��� ��������λ����ʵ�� */		
	}
	else
	{
		SectorCount = (*pFileSize >> 9);
	}
	/* (2) �ļ����ݶ�ȡ�ͷ��� */
	while(1)
	{
		if (SectorCount > BUF_SIZE / DEF_SECTOR_SIZE) 
		{
			Count = BUF_SIZE / DEF_SECTOR_SIZE;
			SectorCount -= BUF_SIZE / DEF_SECTOR_SIZE;
		}
		else
		{
			Count = SectorCount;
		}
		Status = CH376SectorRead(pBuf, (UINT8)Count, NULL);
		if (Status != USB_INT_SUCCESS) break;
		/*	
		if (*pFileSize > BUF_SIZE)
		{
			 Status = SenderBuf(pBuf, FILE_T5L51_BIN, BUF_SIZE);
			 *pFileSize -= BUF_SIZE;
		}
		else Status = SenderBuf(pBuf, FILE_T5L51_BIN, *pFileSize); */
		if (Count == SectorCount) break;		/* ��ȡ��� */ 
		
	}
	CH376CloseFile(0);
	return Status;
}
/*****************************************************************************
 �� �� ��  : CH376WriteFile
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
UINT8 CH376WriteFile(PUINT8 pPathName, PUINT8 pBuf, UINT8 Flag)			/* д���ļ������������½� */
{
	UINT8 xdata Buf[BUF_SIZE];					/* �ַ��洢������ 4096�ֽ� = 8������ */
	UINT8 xdata EndBuf[DEF_SECTOR_SIZE];
	UINT8 Status, SectorCount;
	UINT16 BufFreeLen, BufSourceLen;
	UINT32 FileSize;

	if ((NULL == pPathName) || (NULL == pBuf)) return DWIN_ERROR;
	AlphabetTransfrom(pPathName);
	memset(Buf, 0, BUF_SIZE);
	memset(EndBuf, 0, DEF_SECTOR_SIZE);
	/* (1) ����ļ�������� ���������½��ļ� */
	Status = CH376FileOpenPath(pPathName);
	if (Status != USB_INT_SUCCESS) 
	{
		CH376FileCreatePath(pPathName); 
		Status = CH376FileOpenPath(pPathName);
		if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
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
		case WRITE_FROM_END:					/* ������β��������Ҫ������ƴ��д�� */
		{
			FileSize = CH376GetFileSize();
			if (FileSize % DEF_SECTOR_SIZE)		/* �Ƿ����β����ͷ���� �����ܷ�����512�ж� */ 
			{
				Status = CH376SectorRead(EndBuf, 1, NULL);
			}
			CH376SecLocate(0xFFFFFFFF);
			break;
		}
		default:
			return DWIN_ERROR;
	}
	if (strlen(EndBuf) != 0) 
	{
		strncpy(Buf, EndBuf, strlen(EndBuf));	/* ��ͷ������д�뵽������ */
	}
	FileSize = FileSize + strlen(pBuf);			/* ����д����ļ���С */
	/* (2) ѭ��д���� */
	while(1)
	{
		BufFreeLen = BUF_SIZE - strlen(Buf);
		BufSourceLen = strlen(pBuf);
		strncat(Buf, pBuf, (BufSourceLen > BufFreeLen ? BufFreeLen : BufSourceLen));
		if (BUF_SIZE == strlen(Buf)) SectorCount = BUF_SIZE / DEF_SECTOR_SIZE;
		else									/* ���һ��д����, Buf������û�д�������� */
		{
			SectorCount = strlen(Buf) / DEF_SECTOR_SIZE + (strlen(Buf) % DEF_SECTOR_SIZE ? 1 : 0);
			Buf[strlen(Buf)] = 0; 
		}
		Status = CH376SectorWrite(Buf, SectorCount, NULL);
		if (SectorCount != BUF_SIZE / DEF_SECTOR_SIZE) break;
		memset(Buf, 0, BUF_SIZE);				/* ��������� */
		pBuf += BufFreeLen;
	}
	CH376WriteVar32(VAR_FILE_SIZE, FileSize);	/* ����ȷ�ĵ�ǰ�ļ�����д��CH376�ڴ� */
	Status = CH376SectorWrite(pBuf, 0, NULL);	/* д0����,ʵ����ˢ���ļ����� �ѻ�������������д��USB */
	return Status;
}

UINT8 SenderBuf(PUINT8 pBuf, UINT8 FileFlag, UINT32 BufSize)
{
	if (FileFlag == FILE_T5L51_BIN)
	SendString(pBuf, BufSize);
	return DWIN_OK;
}

/*****************************************************************************
 �� �� ��  : FindDWINFile
 ��������  : ����DWIN�����ļ�
 �������  : PUINT8 MatchString  ������ ƥ���ļ��ľ���·��	 
 �������  : DWIN_OK �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 FindDWINFile(PUINT8 MatchString)
{
	FAT_NAME MatchLish[MATCH_LIST_SIZE];
	UINT8 Status = 0, i = 0;
	UINT16 NameLen = 0;
	Status = CH376MatchFile(MatchString, DWIN_DIR, MatchLish);	
	for (i = 0; i < 10; i++)
	{
		NameLen = strlen(MatchLish[i].NAME);
		if ((MatchLish[i].NAME[NameLen - 3]  == 'B') &&
			(MatchLish[i].NAME[NameLen - 2] == 'I') &&
			(MatchLish[i].NAME[NameLen - 1] == 'N') &&
			(MatchLish[i].Attr != ATTR_VOLUME_ID) &&
			(MatchLish[i].Attr != ATTR_DIRECTORY))
		{
			memset(MatchString, 0, strlen(MatchString));
			strcpy(MatchString, DWIN_DIR);
			strcat(MatchString, "/");
			strcat(MatchString, MatchLish[i].NAME);		/* ���ҵ��ľ���·��д��MatchString */
			return DWIN_OK;
		}
	}
	return DWIN_ERROR;
}









