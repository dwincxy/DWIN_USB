/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : file_sys.h
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��4��
��������   : CH376��������ӿ�
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/
#include "ch376.h"
#include "para_port.h"
#include "t5los8051.h"
#include "system/uart.h"

#ifndef _FILE_SYS_H_
#define _FILE_SYS_H_
/********************************�궨��**************************************/
#define	ERR_USB_UNKNOWN					(0xFA)	/* δ֪����,��Ӧ�÷��������,����Ӳ�����߳������ */

#define DIR_FILE_MAX					(40)	/* Ŀ¼��ö������ļ��� */

/********************************�ṹ������***********************************/
typedef struct _FAT_NAME
{
	UINT8 NAME[20];							/* �ļ���/·���� ���·�� */
	UINT8 Attr;								/* �ļ����� */
}FAT_NAME, *P_FAT_NAME;

/********************************���⺯������*********************************/
void AlphabetTransfrom(PUINT8 name);		/* Сд�ļ���ͳһת��Ϊ��д�ļ��� */

/* �ļ����� */
UINT8 CH376FileOpen(PUINT8 name);			/* �ڸ�Ŀ¼���ߵ�ǰĿ¼�´��ļ�����Ŀ¼(�ļ���) */
UINT8 CH376DirCreate(PUINT8 PathName);		/* �ڸ�Ŀ¼���ߵ�ǰĿ¼����Ŀ¼ */
UINT8 CH376FileDeletePath(PUINT8 PathName);	/* ɾ���ļ�,����Ѿ�����ֱ��ɾ��,��������ļ����ȴ���ɾ��,֧�ֶ༶Ŀ¼·�� */
UINT8 CH376FileOpenPath(PUINT8 PathName);	/* �򿪶༶Ŀ¼�µ��ļ�����Ŀ¼(�ļ���),֧�ֶ༶Ŀ¼·��,֧��·���ָ���,·�����Ȳ�����255���ַ� */
UINT8 CH376FileCreatePath(PUINT8 PathName);	/* �½��༶Ŀ¼�µ��ļ�,֧�ֶ༶Ŀ¼·��,֧��·���ָ���,·�����Ȳ�����255���ַ� */
UINT8 CH376CloseFile(UINT8 param);			/* �ļ��ر� */
UINT8 CH376MatchFile(PUINT8 String, PUINT8 PathName, P_FAT_NAME MatchLish);	/* ƥ���ļ� */
UINT8 GetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir);				/* ��ȡ�ļ����� */
/* ������Ϣ */
UINT8 CH376DiskConnect(void);				/* ���U���Ƿ�����,��֧��SD�� */
UINT8 CH376DiskMount(void);					/* ��ʼ�����̲����Դ����Ƿ���� */
UINT8 CH376GetDiskStatus(void);				/* ��ȡ���̺��ļ�ϵͳ�Ĺ���״̬ */
/* ��������	*/
void CH376WriteVar32(UINT8 var, UINT32 dat);
UINT32 CH376GetFileSize(void);				/* ��ȡ��ǰ�ļ����� */											
UINT8 CH376SectorWrite(PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount);	/* ������Ϊ��λ�ڵ�ǰλ��д�����ݿ�,��֧��SD�� */
UINT8 CH376SectorRead(PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount);	/* ������Ϊ��λ�ӵ�ǰλ�ö�ȡ���ݿ�,��֧��SD�� */
UINT8 CH376SecLocate(UINT32 offset);		/* ������Ϊ��λ�ƶ���ǰ�ļ�ָ��	*/

#endif
