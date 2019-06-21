/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : dgus.h
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��21��
��������   : �ṩ��Ƭ������DGUS�����ӿ� �ļ�������صĽӿڶ���
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/
#include "t5los8051.h"
#include "system/sys.h"
#include "app_interface.h"
#include "system/uart.h" 
#ifndef _DGUS_H_
#define _DGUS_H_

/* �ļ�Flash��ַ���� */
#define ADDR_T5L51_BIN		(UINT32)(0x10000);
#define ADDR_DWIN_OS		(UINT32)(0x18000);
#define LIB(x)				(UINT32)(0x30000 + (x * 4) << 10)
#define WORD_LIB(x)			(UINT32)(0x001 << 20 + (0x100 * x) << 10)
#define BMP(x)				(UINT32)(0x040 << 20 + (0x080 * x) << 10)

#define FLAG_EN				(0x5A)
#define FLAG_NO_EN			(0x00)
#define SPACE_1				(0x00)
#define SPACE_2				(0x01)
#define SPACE_3				(0x02)
#define SPACE_4				(0x03)

#define ADDR_UP_EN			(0x438)
#define ADDR_UP_TIME		(0x439)
#define ADDR_UP_SPANCE1		(0x43A)
#define ADDR_UP_SPANCE2		(0x43E)
#define ADDR_UP_SPANCE3		(0x442)
#define ADDR_UP_SPANCE4		(0x446)
#define ADDR_UP_SET			(0x44A)

#ifndef BUF_SIZE
#define BUF_SIZE			(0x1000)
#endif
#ifndef CONTROL_SIZE
#define CONTROL_SIZE		(0x200)
#endif

#define UPDATE_FAILED		(0xFF)

void ReadDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len);	/* ��DGUS Addr��ַ Len�����ֽڳ��� ����Buf */
void WriteDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len);	/* дDGUS Addr��ַ Len�����ֽڳ��� ����Buf */
UINT8 SystemUpdate(UINT8 FileType);
#endif