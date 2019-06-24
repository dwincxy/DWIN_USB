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
#include "stdio.h"
/********************************���ں�������*********************************/

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