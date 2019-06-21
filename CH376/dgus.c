/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : dgus.c
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��21��
��������   : �ṩ��Ƭ������DGUS�����ӿ� �ļ�������صĽӿڶ���
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/
#include "dgus.h"

void UpdateSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize);
/*****************************************************************************
 �� �� ��  : ReadDGUS
 ��������  : ��DGUS�Ĵ���
 �������  : UINT32 Addr  DGUS�Ĵ�����ַ
             PUINT8 pBuf  ���ջ�����
			 UINT16 Len   ��ȡ�����ֽڳ���	 
 �������  : ��
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
void ReadDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len)
{
	UINT8 Offset = 0;
	UINT8 EndLen = 0;
	Offset = Addr & 0x01;
	Addr = Addr >> 1;
	ADR_H = (UINT8)(Addr >> 16);
	ADR_M = (UINT8)(Addr >> 8);
	ADR_L = (UINT8)(Addr);
	ADR_INC = 1;
	RAMMODE = 0xAF;
	while (!APP_ACK);
	if (Offset == 1)			/* ��ַƫ������ */
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA1;
		if (Len == 1) return;
        *pBuf++ = DATA0;
		Len -= 2;
	}
	EndLen = Len & 0x03;		/* β������ 0-3 */
	Len = Len >> 2;
	while (Len--)
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA3;
		*pBuf++ = DATA2;
		*pBuf++ = DATA1;
		*pBuf++ = DATA0;
	}
	if (EndLen)					/* β�������� �������ݳ���ѡ���ȡ���� */
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA3;
		if (EndLen & 2) *pBuf++ = DATA2;
		if (EndLen & 3) *pBuf++ = DATA1;
	}
	RAMMODE = 0x00;	 
}
/*****************************************************************************
 �� �� ��  : WriteDGUS
 ��������  : дDGUS�Ĵ���
 �������  : UINT32 Addr  DGUS�Ĵ�����ַ
             PUINT8 pBuf  ���ͻ�����
			 UINT16 Len   ���������ֽڳ���	 
 �������  : ��
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
void WriteDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len)
{
	UINT8 Offset;
	UINT8 EndLen;
	Offset = Addr & 1;
	Addr >>= 1;
	ADR_L = (UINT8)(Addr);
	ADR_M = (UINT8)(Addr >> 8);
	ADR_H = (UINT8)(Addr >> 16);
	ADR_INC = 1;
	RAMMODE = 0x8F;
	while (!APP_ACK);
	if (Offset == 1)			/* ��ַƫ������ */
	{
		ADR_INC = 0;
        RAMMODE = 0xAF;
        APP_EN = 1;
        while (APP_EN);			/* ��дִ�н���	*/

        ADR_INC = 0x01; 
        RAMMODE = 0x8F;     
        DATA1 = *pBuf++;
		if (Len == 1) return;
        DATA0 = *pBuf++;
        APP_EN = 1;
        while (APP_EN);			/* ��дִ�н���	*/
        Len -= 2;
	}
	EndLen = Len & 0x03;		/* β������ 0-3 */
	Len >>=2;
	while (Len--)
	{
		DATA3 = *pBuf++;
		DATA2 = *pBuf++;
		DATA1 = *pBuf++;
		DATA0 = *pBuf++;
		APP_EN = 1;
		while (!APP_EN);
	}
	if (EndLen)					/* β�������� �������ݳ���ѡ��д����� */
	{
		DATA3 = *pBuf++;
		if (EndLen & 2) DATA2 = *pBuf++;
		if (EndLen & 3) DATA1 = *pBuf++;
		APP_EN = 1;
		while (!APP_EN);
	}
	RAMMODE = 0x00;
}
/*****************************************************************************
 �� �� ��  : SystemUpdate
 ��������  : ϵͳ����
 �������  : UINT8 FileType  �����ļ�����	 
 �������  : DWIN_OK �ɹ�
             ����    ʧ��
 �޸���ʷ  :
 ��    ��  : 2019��6��21��
 ��    ��  : chenxianyue
 �޸�����  : ����
*****************************************************************************/
UINT8 SystemUpdate(UINT8 FileType)
{
	UINT8 xdata Buf[BUF_SIZE + CONTROL_SIZE];
	UINT8 xdata FileName[20];	/* ���� 8 + '.' + 3 */
	UINT8 Status = 0;
	UINT8 UpSpace = 0;
	PUINT8 pBufFile = Buf;
	UINT32 FileAddr = 0;
	UINT32 AddrBuff = 0;
	UINT32 FileSize = 0;	
	memset(Buf, 0, sizeof(Buf));
	memset(FileName, 0, sizeof(FileName));
	/* (1) �����ļ����� �趨��ز��� */
	switch (FileType)
	{
		case FILE_T5L51_BIN:
		{
			strcpy(FileName, "T5L51*");
			FileAddr = ADDR_T5L51_BIN;
			UpSpace = SPACE_1;
			break;
		}

		case FILE_DWINOS_BIN:
		{
			strcpy(FileName, "DWINOS*");
			FileAddr = ADDR_DWIN_OS;
			UpSpace = SPACE_1;
			break;
		}

		case FILE_XXX_LIB:
		{
			UpSpace = SPACE_1;
			break;
		}
		case FILE_ALL:
		{
			//SystemUpdate(FILE_T5L51_BIN);
			//SystemUpdate(FILE_DWINOS_BIN);
			//SystemUpdate(FILE_XXX_LIB);
			break;
		}
		default:
			return UPDATE_FAILED;
	}
	/* (2) �����ļ� �������ȡ�ļ���Ϣ����4K������ */
	Status = FindDWINFile(FileName);	/* ����Ŀ���ļ��� */
	if (Status != DWIN_OK) return Status;
	pBufFile += CONTROL_SIZE;			/* �л������ݱ������� */
	Status = CH376ReadFile(FileName, pBufFile, &FileSize);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	/* (3) ����ǰ512�ֽڿ�������Ϣ */
	ReadDGUS(ADDR_UP_SET, Buf, 4);			
	AddrBuff = (Buf[3] << 8) & 0xFF00;	/* ��ȡ����DGUS��ַ */
	ReadDGUS(AddrBuff, Buf, 1);
	if (Buf[0] == FLAG_NO_EN) 
	{									/* ���ø��²��� */
		UpdateSet(Buf, FLAG_NO_EN, SPACE_1, FileAddr, (UINT16)FileSize);
	}
	else return DWIN_ERROR;
										/* �״�д�벻�������� ��ֹ�������� */
	WriteDGUS(AddrBuff, Buf, (UINT16)FileSize + CONTROL_SIZE);	
	ReadDGUS(AddrBuff, Buf, 1);
	/* (4) ��λд��������־ �������� */
	if (Buf[0] == FLAG_NO_EN)
	{
		Buf[0] = FLAG_EN;				/* �������� */
		WriteDGUS(AddrBuff, Buf, 4);
	}
	SendString(Buf, BUF_SIZE + CONTROL_SIZE);
	return DWIN_OK;
}

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