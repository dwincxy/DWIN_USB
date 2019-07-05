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
#ifndef _APP_INTERFACE_H_
#define _APP_INTERFACE_H_
#include "driver/system/sys.h"
#include "app/app_usb/file_sys.h"
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
#define FILE_XXX_LIB					(0x03)	/* �����ļ�ѡ�� XXX.LIB */
#define FILE_XXX_BIN					(0x04)	/* �����ļ�ѡ�� XXX.BIN */
#define FILE_XXX_ICL					(0x05)	/* �����ļ�ѡ�� XXX.ICL */
#define FILE_ALL						(0x5A)	/* ����ϵͳ���� */
#define DWIN_DIR						("/DWIN_SET")	/* DWIN��������ļ����ļ��� */
#define MATCH_LIST_SIZE					(40)	/* ƥ���ļ��б���ļ���Ŀ */
/* �ļ�Flash��ַ���� */
#define ADDR_T5L51_BIN		(UINT32)(0x10000);							/* T5L51*.BIN �ļ���ַ */
#define ADDR_DWIN_OS		(UINT32)(0x18000);							/* DWINOS* �ļ���ַ */
#define LIB(x)				(UINT32)((0x30 + x) << 12)			/* *.LIB ���ļ���ַ */
#define FONT(x)				(UINT32)(x << 18)	/* XX*.BIN �ֿ��ļ���ַ */
#define ICL(x)				(UINT32)(x << 18)	/* XX*.ICL ͼƬ�ļ���ַ */
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

#define PATH_FILE			(0x55)
#define PATH_DIR			(0xAA)

typedef struct _DIR_TYPE
{
	UINT8  DIR_Attr;
	UINT16 DIR_CrtTime;
	UINT16 DIR_CrtDate;
	UINT16 DIR_WrtTime;
	UINT16 DIR_WrtDate;
	UINT32 DIR_FileSize;
}DIR_TYPE, *P_DIR_TYPE;	
/********************************���⺯������*********************************/

UINT8 USBInit(void);												/* ���CH376ͨѶ������USB����ģʽ�����̳�ʼ�� */
UINT8 CheckIC(void);
UINT8 CheckConnect(void);
UINT8 CheckDiskInit(void);
UINT8 CH376CreateFileOrDir(PUINT8 pPathName, UINT8 TypePath);
UINT8 CreateFileOrDir(PUINT8 pPathName, UINT8 TypePath);			/* �������ļ�����Ŀ¼ */
UINT8 RmFileOrDir(PUINT8 pPathName);								/* ɾ���ļ�����Ŀ¼ */
UINT8 ReadFile(PUINT8 pPathName, PUINT8 pData, UINT16 DataLen, UINT32 SectorOffset);	/* ��ȡ�ļ���Ϣ */
UINT8 WriteFile(PUINT8 pPathName, PUINT8 pData, UINT16 DataLen, UINT32 SectorOffset);	/* д���ļ������������½� */
UINT8 MatchFile(PUINT8 pDir,PUINT8 pMatchString, PUINT8 pBuf);
UINT8 SystemUpdate(UINT8 FileType, UINT8 FileNumber);				/* ϵͳ���� */
UINT8 GetFileMessage(PUINT8 pFilePath, PUINT8 pBuf);
UINT8 SetFileMessage(PUINT8 pFilePath, PUINT8 pBuf);

void SysUpGetFileMesg(UINT8 FileType, UINT8 FileNumber, PUINT8 pUpSpace, PUINT32 FileAddr, PUINT8 String);
UINT8 SysUpGetDWINFile(PUINT8 pMatchList);
UINT8 SysUpFileMatch(PUINT8 pSource, PUINT8 pDest, PUINT8 pResult, PUINT32 pFileSize);
void SendUpPackToDGUS(UINT32 AddrDgusHead, UINT32 AddrDgusMesg, PUINT8 BufHead, PUINT8 BufMesg, UINT16 MesgSize);
void SysUpPcakSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize);
void SysUpFileSend(PUINT8 pPath, UINT8 UpSpace, UINT32 AddrDgusPck,UINT32 AddrFileSave, UINT32 FileSize);
void SysUpWaitOsFinishRead(UINT32 AddrDgus);

#endif