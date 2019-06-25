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
#include "stdio.h"
/********************************���ں�������*********************************/

UINT8 SenderBuf(PUINT8 pBuf, UINT8 FileFlag, UINT32 BufSize);
void Delay(void);
void UpdateSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize);

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
	return (CH376FileOrDirCreate(pPathName));
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
UINT8 CH376ReadFile(PUINT8 pPathName, PUINT8 pBuf, PUINT32 pFileSize, UINT32 SectorOffset)	/* ��ȡ�ļ���Ϣ */	
{	/* �ַ��洢������pBuf 4096�ֽ� = 8������ */
	UINT8 Status = 0;
	UINT32 SectorCount = 0, Count = 0;

	if (NULL == pPathName)	return DWIN_NULL_POINT;
	AlphabetTransfrom(pPathName);
	memset(pBuf, 0, BUF_SIZE);	
	/* (1) ����ļ��������,��ȡ�ļ����ȺͿɶ�ȡ������ */
	Status = CH376FileOpenPath(pPathName);	
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	CH376SecLocate(SectorOffset);
	*pFileSize = CH376GetFileSize() - (SectorOffset << 9);
	
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
	if (SectorCount > BUF_SIZE / DEF_SECTOR_SIZE) 
	{
		Count = BUF_SIZE / DEF_SECTOR_SIZE;
	}
	else
	{
		Count = SectorCount;
	}
	Status = CH376SectorRead(pBuf, (UINT8)Count, NULL);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
			
	if (*pFileSize > BUF_SIZE)
	{
		Status = SenderBuf(pBuf, FILE_T5L51_BIN, BUF_SIZE);
		*pFileSize -= BUF_SIZE;
	}
	else Status = SenderBuf(pBuf, FILE_T5L51_BIN, *pFileSize);
	
	CH376CloseFile(0);
	return DWIN_OK;
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
             PUINT8 pFileSuffix  ������ ƥ���ļ��ĺ�׺	 
 �������  : DWIN_OK �ɹ�
 			 ���� ����
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 FindDWINFile(PUINT8 pMatchString, PUINT8 pFileSuffix)
{
	FAT_NAME xdata MatchLish[MATCH_LIST_SIZE];
	UINT8 Status = 0, i = 0;
	UINT16 NameLen = 0;
	if (strlen(pFileSuffix) != 3) return DWIN_ERROR;				/* ����׺���Ƿ�Ϸ� */
	Status = CH376MatchFile(pMatchString, DWIN_DIR, MatchLish);	
	for (i = 0; i < 10; i++)
	{
		NameLen = strlen(MatchLish[i].NAME);
		if ((MatchLish[i].NAME[NameLen - 3] == *pFileSuffix++) &&	/* ��׺ƥ���ҷ��ļ�Ŀ¼ */
			(MatchLish[i].NAME[NameLen - 2] == *pFileSuffix++) &&
			(MatchLish[i].NAME[NameLen - 1] == *pFileSuffix)   &&
			(MatchLish[i].Attr != ATTR_VOLUME_ID) &&
			(MatchLish[i].Attr != ATTR_DIRECTORY))
		{
			memset(pMatchString, 0, strlen(pMatchString));
			strcpy(pMatchString, DWIN_DIR);
			strcat(pMatchString, "/");
			strcat(pMatchString, MatchLish[i].NAME);				/* ���ҵ��ľ���·��д��MatchString */
			return DWIN_OK;
		}
	}
	return DWIN_ERROR;
}
/*****************************************************************************
 �� �� ��  : SystemUpdate
 ��������  : ϵͳ����
 �������  : UINT8 FileType    �����ļ�����
             UINT16 FileNumber �����ļ��ı��	 
 �������  : DWIN_OK �ɹ�
             ����    ʧ��
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 SystemUpdate(UINT8 FileType, UINT16 FileNumber)
{
	UINT8 xdata Buf[BUF_SIZE + CONTROL_SIZE];
	UINT8 xdata FileName[22];	/* /DWIN_SET/ + ���� 8 + '.' + 3 */
	UINT8 xdata Suffix[SUFFIX_SIZE];
	UINT8 Status = 0;
	UINT8 UpSpace = 0;
	PUINT8 pBufFile = Buf;
	UINT32 FileAddr = 0;
	UINT32 AddrBuff = 0;
	UINT32 FileSize = 0;
	UINT32 SectorOffset = 0;	
	memset(Buf, 0, sizeof(Buf));
	memset(FileName, 0, sizeof(FileName));
	memset(Suffix, 0, SUFFIX_SIZE);
	/* (1) �����ļ����� �趨��ز��� */
	switch (FileType)
	{
		case FILE_T5L51_BIN:
		{
			strcpy(FileName, "T5L51*");
			strcpy(Suffix, "BIN");
			FileAddr = ADDR_T5L51_BIN;
			UpSpace = SPACE_1;
			break;
		}

		case FILE_DWINOS_BIN:
		{
			strcpy(FileName, "DWINOS*");
			strcpy(Suffix, "BIN");
			FileAddr = ADDR_DWIN_OS;
			UpSpace = SPACE_1;
			break;
		}

		case FILE_XXX_LIB:
		{
			sprintf(FileName, "%d*", FileNumber);
			strcpy(Suffix, "LIB");
			FileAddr = LIB(FileNumber);
			UpSpace = SPACE_1;
			break;
		}
		case FILE_XXX_BIN:
		{
			sprintf(FileName, "%d*", FileNumber);
			strcpy(Suffix, "BIN");
			FileAddr = FONT(FileNumber);
			UpSpace = SPACE_2;
			break;
		}
		case FILE_XXX_ICL:
		{
			sprintf(FileName, "%d*", FileNumber);
			strcpy(Suffix, "ICL");
			FileAddr = ICL(FileNumber);
			UpSpace = SPACE_2;
			break;
		}
		default:
			return DWIN_ERROR;
	}
	/* (2) �����ļ� �������ȡ�ļ���Ϣ����4K������ */
	Status = FindDWINFile(FileName, Suffix);/* ����Ŀ���ļ��� */
	if (Status != DWIN_OK) return Status;
	pBufFile += CONTROL_SIZE;				/* �л������ݱ������� */
	Status = CH376ReadFile(FileName, pBufFile, &FileSize, 0);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	if (FileSize > BUF_SIZE) FileSize = BUF_SIZE;
	/* (3) ����ǰ512�ֽڿ�������Ϣ */
	ReadDGUS(ADDR_UP_CONFIG, Buf, 4);			
	AddrBuff = (Buf[3] << 8) & 0xFF00;		/* ��ȡ����DGUS��ַ ��8bit��0x00 */
	ReadDGUS(AddrBuff, Buf, 1);
	if (Buf[0] == FLAG_NO_EN) 
	{										/* ���ø��²��� */
		UpdateSet(Buf, FLAG_NO_EN, UpSpace, FileAddr, (UINT16)FileSize);
	}
	else return DWIN_ERROR;										
	WriteDGUS(AddrBuff, Buf, (UINT16)FileSize + CONTROL_SIZE);	/* �״�д�벻�������� ��ֹ�������� */
	AddrBuff += CONTROL_SIZE / 2;			/* �ƶ���ַָ�뵽д������ֵ�β�� */
	while (FileSize == BUF_SIZE)				/* �ļ���δ��ȡ��� */
	{
		AddrBuff += BUF_SIZE / 2;			/* ��ַƫ��:�ƶ���ַָ�뵽д�����ݵ�β�� */
		SectorOffset += 8;					/* ����ƫ��:һ��ƫ��8������(512B) 4096B */
		Status = CH376ReadFile(FileName, pBufFile, &FileSize, SectorOffset);
		if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
		if (FileSize > BUF_SIZE) FileSize = BUF_SIZE;
		WriteDGUS(AddrBuff, pBufFile, (UINT16)FileSize);
	}	
	ReadDGUS(AddrBuff, Buf, 1);
	/* (4) ��λд��������־ �������� */
	if (Buf[0] == FLAG_NO_EN)
	{
		Buf[0] = FLAG_EN;					/* �������� */
		WriteDGUS(AddrBuff, Buf, 4);
	}
	//SendString(Buf, BUF_SIZE + CONTROL_SIZE);
	return DWIN_OK;
}
/*****************************************************************************
 �� �� ��  : UpdateSet
 ��������  : ���¿���������
 �������  : PUINT8 pBuf     ������BUF������
             UINT8 Flag_EN   ����ʹ�ܱ�־λ
			 UINT8 UpSpace   �����ռ�
			 UINT32 UpAddr   �ļ�������ַ	
			 UINT16 FileSize �����ļ���С 
 �������  : DWIN_OK �ɹ�
             ����    ʧ��
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
void UpdateSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize)
{
	 *pBuf++ = Flag_EN;					/* ������־ */
	 *pBuf++ = UpSpace;					/* �����ռ�ѡ�� */
	 *pBuf++ = (UINT8)(UpAddr >> 24);	/* Զ������Ŀ���ַ */
	 *pBuf++ = (UINT8)(UpAddr >> 16);
	 *pBuf++ = (UINT8)(UpAddr >> 8);
	 *pBuf++ = (UINT8)(UpAddr);
	 *pBuf++ = (UINT8)(FileSize >> 8);	/* �����ֽڳ��� 0x0001 - 0x0FFF */
	 *pBuf++ = (UINT8)(FileSize); 		
	 *pBuf++ = 0x00;					/* Ĭ�ϲ�����CRCУ�� */
	 *pBuf++ = 0x00;
}