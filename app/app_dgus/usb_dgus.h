/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : usb_dgus.h
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��21��
��������   : USB�ӿ���صĵ�DGUSӦ�ó���ʵ��
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/
#ifndef _USB_DGUS_H_
#define _USB_DGUS_H_

#include "driver/system/sys.h"

void USBModule(void);
UINT8 CompareDgusRegValue(UINT32 AddrDgus, UINT8 Value);
void ReadDgusClientString(UINT32 AddrDgus, PUINT8 pData, PUINT16 pDataLen);
void WriteDgusClientString(UINT32 AddrDgus, PUINT8 pData, UINT16 DataLen);
void AckCreateOrDelPath(void);
void AckSearchFile(void);
void AckReadOrWriteFile(void);
#endif