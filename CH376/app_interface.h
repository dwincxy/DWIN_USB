/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : app_interface.h
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��21��
��������   : CH376Ӧ�ó���ӿ�
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/
#include "para_port.h"
#include "ch376.h"
#include "file_sys.h"
#include "string.h"
#include "system/uart.h"
#include "ch376/dgus.h"
#ifndef _APP_INTERFACE_H_
#define _APP_INTERFACE_H_

/********************************�궨��***************************************/
#define	USB_DEVICE_OFF					(0x00)	/* δ����USB�豸��ʽ */
#define	USB_DEVICE_ON_OUTSIDE_FIRMWARE	(0x01)	/* ������USB�豸��ʽ,�ⲿ�̼�ģʽ(�������ӷ�ʽ��֧��) */
#define	USB_DEVICE_ON_INSIDE_FIRMWARE	(0x02)	/* ������USB�豸��ʽ ,���ù̼�ģʽ */
#define SD_HOST							(0x03)	/* SD������ģʽ */
#define USB_HOST_OFF					(0x04)	/* δ����USB������ʽ */
#define USB_HOST_ON_NO_SOF				(0x05)	/* ������USB������ʽ,���Զ�����SOF�� */
#define USB_HOST_ON_SOF					(0x06)	/* ������USB������ʽ,�Զ�����SOF�� */
#define USB_HOST_ON_RESET_USB			(0x07)	/* ������USB������ʽ,��λUSB���� */
#define BUF_SIZE						(0x1000)/* ������BUF����С Խ��������дԽ�� �ɵ�Ƭ��RAM��С���� */
#define SUFFIX_SIZE						(0x03)	/* �ļ���׺����С */
#define	WRITE_FROM_HEAD					(0x00)	/* ����д��ʽ��ѡ����ļ�ͷ��д */
#define WRITE_FROM_END					(0x01)	/* ����д��ʽ��ѡ����ļ�β��д */ 
#define FILE_T5L51_BIN					(0x01)	/* �����ļ�ѡ�� T5L51.BIN */
#define FILE_DWINOS_BIN					(0x02)	/* �����ļ�ѡ�� DWINOS.BIN */
#define FILE_XXX_LIB					(0x03)	/* �����ļ�ѡ�� XXX.LIB */
#define FILE_XXX_BIN					(0x04)	/* �����ļ�ѡ�� XXX.BIN */
#define FILE_XXX_ICL					(0x05)	/* �����ļ�ѡ�� XXX.ICL */
#define FILE_ALL						(0xFF)	/* ����ϵͳ���� */
#define DWIN_DIR						("/DWIN_SET")	/* DWIN��������ļ����ļ��� */
#define MATCH_LIST_SIZE					(40)	/* ƥ���ļ��б���ļ���Ŀ */

/* �ļ�Flash��ַ���� */
#define ADDR_T5L51_BIN		(UINT32)(0x10000);							/* T5L51*.BIN �ļ���ַ */
#define ADDR_DWIN_OS		(UINT32)(0x18000);							/* DWINOS* �ļ���ַ */
#define LIB(x)				(UINT32)(0x30000 + (x * 4) << 10)			/* *.LIB ���ļ���ַ */
#define FONT(x)				(UINT32)(0x001 << 20 + (0x100 * x) << 10)	/* XX*.BIN �ֿ��ļ���ַ */
#define ICL(x)				(UINT32)(0x040 << 20 + (0x080 * x) << 10)	/* XX*.BMP ͼƬ�ļ���ַ */
/* ����ʹ�ܱ�� */
#define FLAG_EN				(0x5A)
#define FLAG_NO_EN			(0x00)
/* �����ռ� */
#define SPACE_1				(0x00)
#define SPACE_2				(0x01)
#define SPACE_3				(0x02)
#define SPACE_4				(0x03)
/* ����������ص�ַ */
#define ADDR_UP_EN			(0x438)
#define ADDR_UP_TIME		(0x439)
#define ADDR_UP_SPANCE1		(0x43A)
#define ADDR_UP_SPANCE2		(0x43E)
#define ADDR_UP_SPANCE3		(0x442)
#define ADDR_UP_SPANCE4		(0x446)
#define ADDR_UP_CONFIG		(0x44A)
/* ��������С���� ����+���� = 521B + 4096B */
#ifndef BUF_SIZE
#define BUF_SIZE			(0x1000)
#endif
#ifndef CONTROL_SIZE
#define CONTROL_SIZE		(0x200)
#endif
/********************************���⺯������*********************************/

/* ���CH376ͨѶ������USB����ģʽ�����̳�ʼ�� */
UINT8 CH376USBInit(void);
/* �������ļ� */
UINT8 CH376TouchNewFile(PUINT8 pPathName);
/* ��������Ŀ¼ */
UINT8 CH376TouchDir(PUINT8 pPathName);
/* ɾ���ļ�����Ŀ¼ */
UINT8 CH376RmFile(PUINT8 pPathName);
/* ��ȡ�ļ���Ϣ */
UINT8 CH376ReadFile(PUINT8 pPathName, PUINT8 pBuf, PUINT32 pFileSize, UINT32 SectorOffset);
/* д���ļ������������½� */
UINT8 CH376WriteFile(PUINT8 pPathName, PUINT8 pBuf, UINT8 Flag);
/* ����DWIN�����ļ� */
UINT8 FindDWINFile(PUINT8 pMatchString, PUINT8 pFileSuffix);
/* ϵͳ���� */
UINT8 SystemUpdate(UINT8 FileType, UINT16 FileNumber);

#endif