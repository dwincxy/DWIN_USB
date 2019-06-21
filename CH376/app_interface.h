/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : app_interface.h
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��4��
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
#define	WRITE_FROM_HEAD					(0x00)	/* ����д��ʽ��ѡ����ļ�ͷ��д */
#define WRITE_FROM_END					(0x01)	/* ����д��ʽ��ѡ����ļ�β��д */ 
#define FILE_T5L51_BIN					(0x01)	/* �����ļ�ѡ�� T5L51.BIN */
#define FILE_DWINOS_BIN					(0x02)	/* �����ļ�ѡ�� DWINOS.BIN */
#define FILE_XXX_LIB					(0x04)	/* �����ļ�ѡ�� XXX.BIN */
#define FILE_ALL						(FILE_T5L51_BIN | FILE_DWINOS_BIN |FILE_XXX_LIB)	/* ����ϵͳ���� */
#define DWIN_DIR						("/DWIN_SET")	/* DWIN��������ļ����ļ��� */
#define MATCH_LIST_SIZE					(10)	/* ƥ���ļ��б���ļ���Ŀ */
/********************************���⺯������*********************************/

UINT8 CH376HostInit(void);

UINT8 Query376Interrupt(void);

UINT8 CH376TouchNewFile(PUINT8 PathName);

UINT8 CH376TouchDir(PUINT8 PathName);

UINT8 CH376ReadFile(PUINT8 pPathName, PUINT8 pBuf, PUINT32 pFileSize);

UINT8 CH376WriteFile(PUINT8 pPathName, PUINT8 pBuf, UINT8 Flag);

UINT8 FindDWINFile(PUINT8 MatchString);

#endif