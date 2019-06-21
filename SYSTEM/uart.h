/******************************************************************************
*																			  *
*                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾					  *
*																			  *
*******************************************************************************
* �� �� ��   : uart.c													      *
* �� �� ��   : V1.0															  *
* ��    ��   : chenxianyue													  *
* ��������   : 2019��6��4��													  *
* ��������   : ����5�жϴ�ӡ�ַ���			  *
* �޸���ʷ   :																  *
* ��    ��   : 																  *
* ��    ��   : 																  *
* �޸�����   : 																  *
******************************************************************************/

#include "t5los8051.h"
#include "sys.h"

#ifndef _UART_H_
#define _UART_H_

void UART5_Init(void);

void UART5_Sendbyte(UINT8 dat);

void UART5_SendString(PUINT8 String);

UINT8 UART5_Recivebyte(void);

void SendString(PUINT8 String, UINT32 BUFFSIZE);

#endif