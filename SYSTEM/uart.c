/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : uart.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : 串口5中断打印字符串
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "uart.h"

/*****************************************************************************
 函 数 名  : UART5_Init
 功能描述  : 串口5初始化 115200波特率
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void UART5_Init(void)
{
    SCON3T=0x80;
    SCON3R=0x80;
    BODE3_DIV_H=0x00;
    BODE3_DIV_L=0xE0;
}
/*****************************************************************************
 函 数 名  : UART5_Sendbyte
 功能描述  : 串口5发送单个字节
 输入参数  : UINT8 dat  将发送的字节数据	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void UART5_Sendbyte(UINT8 dat)
{	
	SBUF3_TX = dat;    
	while((SCON3T & 0x01) == 0);
	SCON3T = 0x80;    
}
/*****************************************************************************
 函 数 名  : UART5_SendString
 功能描述  : 串口5发送字符串, '\0'自动截至
 输入参数  : PUINT8 pBuf      将发送的缓冲区	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void UART5_SendString(PUINT8 pBuf)
{
	while(*pBuf != '\0')
	{
		UART5_Sendbyte(*pBuf++);	
	}
}
/*****************************************************************************
 函 数 名  : SendString
 功能描述  : 串口5发送指定长度的缓冲数据
 输入参数  : PUINT8 pBuf      将发送的缓冲区
             UINT32 BUFFSIZE  数据字节长度	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void SendString(PUINT8 pBuf, UINT32 BUFFSIZE)
{
	UINT32 i = 0;
	for (i = 0; i < BUFFSIZE; i++)
	{
		UART5_Sendbyte(*pBuf++);	
	}
}