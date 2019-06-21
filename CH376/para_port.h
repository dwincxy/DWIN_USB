/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : para_port.c
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��4��
��������   : CH376���ڳ�ʼ���Լ���T5L����ͨѶʵ��
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/

#include "t5los8051.h"
#include "system/sys.h"
#ifndef _PARA_PORT_H_
#define _PARA_PORT_H_	

void CH376_PORT_INIT(void);			/* ��ʼ������IO */
UINT8 xReadCH376Status(void);		/* ��CH376��״̬ */
void xWriteCH376Cmd(UINT8 mCmd);  	/* ��CH376д���� */
void xWriteCH376Data(UINT8 mData);	/* ��CH376д���� */
UINT8 xReadCH376Data(void);			/* ��CH376������ */

#endif