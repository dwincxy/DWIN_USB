/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : uart.h
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : 串口5中断打印字符串
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "t5los8051.h"
#include "sys.h"

#ifndef _UART_H_
#define _UART_H_
/********************************对外函数声明*********************************/

/* 串口5初始化 1ms定时器 */
void UART5_Init(void);
/* 串口5发送单个字节 */
void UART5_Sendbyte(UINT8 dat);
/* 串口5发送字符串, '\0'自动截至 */
void UART5_SendString(PUINT8 String);
/* 串口5发送指定长度的缓冲数据 */
void SendString(PUINT8 String, UINT32 BUFFSIZE);

#endif