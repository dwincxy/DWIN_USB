/******************************************************************************
																	
                  ��Ȩ���� (C), 2019, �������ĿƼ����޹�˾	
																			  
*******************************************************************************
�� �� ��   : file_sys.c
�� �� ��   : V1.0
��    ��   : chenxianyue
��������   : 2019��6��4��
��������   : CH376��������ӿ�
�޸���ʷ   :
��    ��   : 
��    ��   : 
�޸�����   : 	
******************************************************************************/
#include "file_sys.h"
#include "string.h"
#include "t5los8051.h"
#include "driver/usb/para_port.h"
#include "driver/uart/uart.h"
UINT8 xdata GlobalBuf[64];
/********************************�ڲ���������*********************************/
UINT8	Wait376Interrupt(void);
UINT8	Query376Interrupt(void);
void	CH376SetFileName(PUINT8 name);
UINT8	CH376DeleteFile(PUINT8 pName);
UINT8	CH376SeparatePath(PUINT8 path);
UINT8	CH376FileOpenDir(PUINT8 PathName, UINT8 StopName);
void	CH376WriteVar8(UINT8 var, UINT8 dat);
UINT8	CH376ReadVar8(UINT8 var);
void	CH376WriteVar32(UINT8 var, UINT32 dat);
UINT32	CH376ReadVar32(UINT8 var);
UINT32	CH376Read32bitDat(void);
void	CH376Write32bitDat(UINT32 mData);
void	CH376EndDirInfo(void);
UINT8	CH376SendCmdWaitInt(UINT8 mCmd);
UINT8	CH376GetIntStatus(void);
UINT8	CH376DiskReqSense(void);
UINT8	CH376DiskWriteSec(PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount);
UINT8	CH376DiskReadSec(PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount);
void	CH376WriteHostBlock(PUINT8 buf, UINT8 len);
UINT8	CH376ReadBlock(PUINT8 buf);  
UINT8	CH376DiskQuery(PUINT32 DiskFree);
UINT8	CH376DiskCapacity(PUINT32 DiskCap);
UINT8	CH376WriteReqBlock(PUINT8 pbuf);
UINT8	CH376CheckNameSum(PUINT8 pDirName);
UINT8	CH376ByteLocate(UINT32 offset);
UINT8	CH376ByteRead(PUINT8 pbuf, UINT16 ReqCount, PUINT16 pRealCount);
UINT8	CH376LocateInUpDir(PUINT8 pPathName);
/*********************************����ʵ��*************************************/
void AlphabetTransfrom(PUINT8 name)			/* Сд�ļ���ͳһת��Ϊ��д�ļ��� */
{
	PUINT8 c;
	for(c = name; *c != 0; c++)
	{
		if((*c >= 'a') && (*c <= 'z')) *c = *c -32;
	}
}

UINT8 Wait376Interrupt(void)
{
	UINT32 i = 0;
	while (Query376Interrupt() == FALSE)
	{
		if(i++ == 0xFFFFF) break;
	}	/* һֱ���ж� */
	return (CH376GetIntStatus());			/* ��⵽�ж� */	
}

UINT8 Query376Interrupt(void)
{
	return (CH376_INT_WIRE ? FALSE : TRUE);  /* CH376���ж����Ų�ѯ */
}

void CH376SetFileName(PUINT8 name)  /* ���ý�Ҫ�������ļ����ļ��� */
{
	UINT8	c;
	xWriteCH376Cmd(CMD10_SET_FILE_NAME);
	c = *name;
	xWriteCH376Data(c);
	while (c) 
	{
		name ++;
		c = *name;
		if (c == DEF_SEPAR_CHAR1 || c == DEF_SEPAR_CHAR2) c = 0;  /* ǿ�н��ļ�����ֹ */
		xWriteCH376Data (c);
	}	
}

UINT8 CH376Error(void)
{
	CH376CloseFile(0);
	return DWIN_ERROR;
}

UINT8 CH376DeleteFile(PUINT8 pName)		/* �ڸ�Ŀ¼���ߵ�ǰĿ¼��ɾ���ļ�����Ŀ¼(�ļ���) */
{
	CH376SetFileName(pName);
	return (CH376SendCmdWaitInt(CMD0H_FILE_ERASE));
}	

UINT8 CH376FileOpen(PUINT8 name)	/* �ڸ�Ŀ¼���ߵ�ǰĿ¼�´��ļ�����Ŀ¼(�ļ���) */
{
	CH376SetFileName(name);
	return (CH376SendCmdWaitInt(CMD0H_FILE_OPEN));
}

UINT8 CH376SeparatePath(PUINT8 path)  /* ��·���з�������һ���ļ�������Ŀ¼(�ļ���)��,�������һ���ļ�������Ŀ¼�����ֽ�ƫ�� */
{
	PUINT8	pName;
	for (pName = path; *pName != 0; ++ pName);
	while (*pName != DEF_SEPAR_CHAR1 && *pName != DEF_SEPAR_CHAR2 && pName != path) 
		pName --;
	if (pName != path) pName ++;
	return (pName - path);
}

UINT8 CH376FileOpenDir(PUINT8 PathName, UINT8 StopName)  /* �򿪶༶Ŀ¼�µ��ļ�����Ŀ¼���ϼ�Ŀ¼,֧�ֶ༶Ŀ¼·��,֧��·���ָ���,·�����Ȳ�����255���ַ� */
/* StopName ָ�����һ���ļ�������Ŀ¼�� */
{
	UINT8 i, s;
	s = 0;
	i = 1;
	while (1) 
	{
		while (PathName[i] != DEF_SEPAR_CHAR1 && PathName[i] != DEF_SEPAR_CHAR2 && PathName[i] != 0) ++ i;
		if (PathName[i]) i ++;
		else i = 0;  /* ·������ */
		s = CH376FileOpen(&PathName[s]);
		if (i && i != StopName) 
		{  /* ·����δ���� */
			if (s != ERR_OPEN_DIR) 
			{
				if (s == USB_INT_SUCCESS) return ERR_FOUND_NAME;
				else if (s == ERR_MISS_FILE) return ERR_MISS_DIR;
				else return s;  /* �������� */
			}
			s = i;  /* ����һ��Ŀ¼��ʼ���� */
		}
		else return s;  /* ·������,USB_INT_SUCCESSΪ�ɹ����ļ�,ERR_OPEN_DIRΪ�ɹ���Ŀ¼(�ļ���),����Ϊ�������� */
	}
}

UINT8 CH376DirCreate(PUINT8 PathName)		/* �ڸ�Ŀ¼���ߵ�ǰĿ¼����Ŀ¼ */
{
	CH376SetFileName(PathName); 
	CH376WriteVar32(VAR_CURRENT_CLUST, 0);
	return (CH376SendCmdWaitInt(CMD0H_DIR_CREATE));		
}

UINT8 CH376FileDeletePath(PUINT8 PathName)	/* ɾ���ļ�,����Ѿ�����ֱ��ɾ��,��������ļ����ȴ���ɾ��,֧�ֶ༶Ŀ¼·�� */
{
	UINT8 s;
	if (PathName ) 
	{  /* �ļ���δ�� */
		for (s = 1; PathName[s] != DEF_SEPAR_CHAR1 && PathName[s] != DEF_SEPAR_CHAR2 && PathName[s] != 0; ++ s);  /* ������һ��·���ָ�������·�������� */
		if (PathName[s]) 
		{
			s = CH376FileOpenPath(PathName);
			if (s != USB_INT_SUCCESS && s != ERR_OPEN_DIR) return s;  /* �������� */
		}
		else CH376SetFileName(PathName);  	/* û��·���ָ���,�Ǹ�Ŀ¼���ߵ�ǰĿ¼�µ��ļ�����Ŀ¼,���ý�Ҫ�������ļ����ļ��� */
	}
	return (CH376SendCmdWaitInt(CMD0H_FILE_ERASE));
}

UINT8 CH376FileOpenPath(PUINT8 PathName)  	/* �򿪶༶Ŀ¼�µ��ļ�����Ŀ¼(�ļ���),֧�ֶ༶Ŀ¼·��,֧��·���ָ���,·�����Ȳ�����255���ַ� */
{
	return (CH376FileOpenDir(PathName, 0xFF));
}

UINT8 CH376FileCreate(PUINT8 PathName)		/* �ڸ�Ŀ¼���ߵ�ǰĿ¼�����ļ� */
{
	CH376SetFileName(PathName);
	return (CH376SendCmdWaitInt(CMD0H_FILE_CREATE));
}

UINT8 CH376FileCreatePath(PUINT8 PathName)  /* �½��༶Ŀ¼�µ��ļ�,֧�ֶ༶Ŀ¼·��,֧��·���ָ���,·�����Ȳ�����255���ַ� */
{
	UINT8 s;
	UINT8 Name;
	Name = CH376SeparatePath(PathName);
	if (Name) 
	{
		s = CH376FileOpenDir(PathName, Name);
		if (s != ERR_OPEN_DIR) 
		{
			if (s == USB_INT_SUCCESS) return (ERR_FOUND_NAME);
			else if (s == ERR_MISS_FILE) return (ERR_MISS_DIR);
			else return s;  /* �������� */
		}
	}
	return (CH376FileCreate(&PathName[Name]));  /* �ڸ�Ŀ¼���ߵ�ǰĿ¼���½��ļ� */
}

UINT8 CH376CloseFile(UINT8 param)		/* �ļ��ر� */
{
	xWriteCH376Cmd(CMD1H_FILE_CLOSE);
	xWriteCH376Data(param);
	return (Wait376Interrupt());
}

void CH376WriteVar8(UINT8 var, UINT8 dat)  /* дCH376оƬ�ڲ���8λ���� */
{
	xWriteCH376Cmd(CMD20_WRITE_VAR8);
	xWriteCH376Data(var);
	xWriteCH376Data(dat);
}

UINT8 CH376ReadVar8(UINT8 var)  /* ��CH376оƬ�ڲ���8λ���� */
{
	UINT8	c0;
	xWriteCH376Cmd(CMD11_READ_VAR8);
	xWriteCH376Data(var);
	c0 = xReadCH376Data();
	return c0;
}

void CH376WriteVar32(UINT8 var, UINT32 dat)  /* дCH376оƬ�ڲ���32λ���� */
{
	xWriteCH376Cmd(CMD50_WRITE_VAR32);
	xWriteCH376Data(var);
	xWriteCH376Data((UINT8)dat);
	xWriteCH376Data((UINT8)((UINT16)dat >> 8));
	xWriteCH376Data((UINT8)( dat >> 16 ));
	xWriteCH376Data((UINT8)( dat >> 24 ));
}

UINT32 CH376ReadVar32(UINT8 var)  /* ��CH376оƬ�ڲ���32λ���� */
{
	xWriteCH376Cmd(CMD14_READ_VAR32);
	xWriteCH376Data(var);
	return (CH376Read32bitDat());  /* ��CH376оƬ��ȡ32λ�����ݲ��������� */
}

UINT32 CH376Read32bitDat(void)  /* ��CH376оƬ��ȡ32λ�����ݲ��������� */
{
	UINT8	c0, c1, c2, c3;
	c0 = xReadCH376Data();
	c1 = xReadCH376Data();
	c2 = xReadCH376Data();
	c3 = xReadCH376Data();
	return (c0 | (UINT16)c1 << 8 | (UINT32)c2 << 16 | (UINT32)c3 << 24);
}

void CH376Write32bitDat(UINT32 mData)  /* ��CH376оƬ����32λ�����ݲ��������� */
{
	xWriteCH376Data((UINT8)mData);
	xWriteCH376Data((UINT8)(mData >> 8));
	xWriteCH376Data((UINT8)(mData >> 16));
	xWriteCH376Data((UINT8)(mData >> 24));
}

void CH376EndDirInfo(void)  /* �ڵ���CH376DirInfoRead��ȡFAT_DIR_INFO�ṹ֮��Ӧ��֪ͨCH376���� */
{
	CH376WriteVar8(0x0D, 0x00);
}

UINT8 CH376SendCmdWaitInt(UINT8 mCmd)  /* �����������,�ȴ��ж� */
{
	xWriteCH376Cmd(mCmd);
	return (Wait376Interrupt());
}

UINT8 CH376GetIntStatus(void)  /* ��ȡ�ж�״̬��ȡ���ж����� */
{
	UINT8 i, Data;
	xWriteCH376Cmd(CMD01_GET_STATUS);
	for (i=100; i!=0; i--)
	{
		Data = 	xReadCH376Data();
		if(Data != 0xFF) break;
	} 
	return Data;
}

UINT8 CH376DiskConnect(void)  /* ���U���Ƿ�����,��֧��SD�� */
{
	return (CH376SendCmdWaitInt(CMD0H_DISK_CONNECT));
}

UINT8 CH376DiskReqSense(void)  /* ���USB�洢������ */
{
	UINT8	s;;
	s = CH376SendCmdWaitInt(CMD0H_DISK_R_SENSE);
	return s;
}

UINT8 CH376DiskMount(void)  /* ��ʼ�����̲����Դ����Ƿ���� */
{
	return (CH376SendCmdWaitInt(CMD0H_DISK_MOUNT));
}

UINT8 CH376GetDiskStatus(void)  /* ��ȡ���̺��ļ�ϵͳ�Ĺ���״̬ */
{
	return (CH376ReadVar8(VAR_DISK_STATUS));
}

UINT32 CH376GetFileSize(void)  /* ��ȡ��ǰ�ļ����� */
{
	return(CH376ReadVar32(VAR_FILE_SIZE));
}

UINT8 CH376SectorWrite(PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount)	
/* ������Ϊ��λ�ڵ�ǰλ��д�����ݿ�,��֧��SD�� */
{
	UINT8 	s, cnt;
	UINT32	StaSec;

	if (RealCount) *RealCount = 0;
	do
	{
		xWriteCH376Cmd(CMD1H_SEC_WRITE);
		xWriteCH376Data(ReqCount);
		s = Wait376Interrupt();
		if(USB_INT_SUCCESS != s) return s;
		xWriteCH376Cmd(CMD01_RD_USB_DATA0);
		xReadCH376Data();
		cnt = xReadCH376Data();				/* ��д������ */
		xReadCH376Data();
		xReadCH376Data();
		xReadCH376Data();
		StaSec = CH376Read32bitDat();
		if (cnt == 0) break;
		s = CH376DiskWriteSec(buf, StaSec, cnt);
		if (s != USB_INT_SUCCESS) return s;
		buf += cnt * DEF_SECTOR_SIZE;
		if (RealCount) *RealCount += cnt;
		ReqCount -= cnt; 		
	}
	while (ReqCount);
	
	return s;
}

UINT8 CH376SectorRead(PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount)		/* ������Ϊ��λ�ӵ�ǰλ�ö�ȡ���ݿ�,��֧��SD�� */
{
	UINT8	s, cnt;
	UINT32	StaSec;
	if (RealCount) *RealCount = 0;
	do
	{
		xWriteCH376Cmd(CMD1H_SEC_READ);
		xWriteCH376Data(ReqCount);
		s = Wait376Interrupt();
		if (s != USB_INT_SUCCESS) return s;
		xWriteCH376Cmd(CMD01_RD_USB_DATA0);
		xReadCH376Data();						/* ��������sizeof(CH376_CMD_DATA.SectorRead) */
		cnt = xReadCH376Data();					/* CH376_CMD_DATA.SectorRead.mSectorCount */
		xReadCH376Data();
		xReadCH376Data();
		xReadCH376Data();
		StaSec = CH376Read32bitDat();			/* CH376_CMD_DATA.SectorRead.mStartSector,��CH376оƬ��ȡ32λ�����ݲ��������� */
		if (cnt == 0) break;
		s = CH376DiskReadSec(buf, StaSec, cnt);	/* ��U�̶�ȡ������������ݿ鵽������ */
		if (s != USB_INT_SUCCESS) return s;
		buf += cnt * DEF_SECTOR_SIZE;
		if (RealCount) *RealCount += cnt;
		ReqCount -= cnt;
	}
	while (ReqCount);
	return s;
}

UINT8 CH376SecLocate(UINT32 offset)		//������Ϊ��λ�ƶ���ǰ�ļ�ָ��
										//����ƫ����0��ʾ���ļ�ָ���ƶ����ļ���ͷ
										//����ƫ����FFFFFFFFH��ʾ���ļ�ָ���ƶ����ļ�ĩβ ���ֻ�ܵ�0X00FF FFFF
{
	xWriteCH376Cmd(CMD4H_SEC_LOCATE);
	xWriteCH376Data((UINT8)offset);
	xWriteCH376Data((UINT8)((UINT16)offset>>8));
	xWriteCH376Data((UINT8)(offset>>16));
	xWriteCH376Data(0);  /* ��������ļ��ߴ� */
	return (Wait376Interrupt());
}

UINT8 CH376DiskWriteSec(PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount)  
/* ���������еĶ�����������ݿ�д��U��,��֧��SD�� */
/* iLbaStart ��д�������ʼ��������, iSectorCount ��д��������� */
{
	UINT8	s, err;
	UINT16	mBlockCount;
	for (err = 0; err != 3; ++ err) 
	{  /* �������� */  	
		xWriteCH376Cmd(CMD5H_DISK_WRITE);
		xWriteCH376Data((UINT8)iLbaStart);
		xWriteCH376Data((UINT8)((UINT16)iLbaStart >> 8));
		xWriteCH376Data((UINT8)(iLbaStart >> 16));
		xWriteCH376Data((UINT8)(iLbaStart >> 24));
		xWriteCH376Data(iSectorCount);
		for (mBlockCount = iSectorCount * DEF_SECTOR_SIZE / CH376_DAT_BLOCK_LEN; mBlockCount != 0; --mBlockCount)
		{
			s = Wait376Interrupt(); 
			if ( s == USB_INT_DISK_WRITE) 
			{
				CH376WriteHostBlock(buf, CH376_DAT_BLOCK_LEN);  /* ��USB�����˵�ķ��ͻ�����д�����ݿ� */
				xWriteCH376Cmd(CMD0H_DISK_WR_GO);
				buf += CH376_DAT_BLOCK_LEN;
			}
			else break;  /* ���ش���״̬ */
		}
		if (mBlockCount == 0) 
		{
			s = Wait376Interrupt();  /* �ȴ��жϲ���ȡ״̬ */
			if (s == USB_INT_SUCCESS) return USB_INT_SUCCESS;  /* �����ɹ� */
		}
		if (s == USB_INT_DISCONNECT) return s;  /* U�̱��Ƴ� */
		CH376DiskReqSense();  /* ���USB�洢������ */
	}
	return s;  /* ����ʧ�� */
}

UINT8 CH376DiskReadSec(PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount)  /* ��U�̶�ȡ������������ݿ鵽������,��֧��SD�� */
/* iLbaStart ��׼����ȡ��������ʼ������, iSectorCount ��׼����ȡ�������� */
{
	UINT8	s, err;
	UINT16	mBlockCount;
	for (err = 0; err != 3; ++err)
	{  /* �������� */
		xWriteCH376Cmd(CMD5H_DISK_READ);  /* ��USB�洢�������� */
		xWriteCH376Data((UINT8)iLbaStart);  /* LBA�����8λ */
		xWriteCH376Data((UINT8)((UINT16)iLbaStart >> 8));
		xWriteCH376Data((UINT8)(iLbaStart >> 16));
		xWriteCH376Data((UINT8)(iLbaStart >> 24));  /* LBA�����8λ */
		xWriteCH376Data(iSectorCount);  /* ������ */
		for (mBlockCount = iSectorCount * DEF_SECTOR_SIZE / CH376_DAT_BLOCK_LEN; mBlockCount != 0; --mBlockCount)
		{  /* ���ݿ���� */
			s = Wait376Interrupt();  /* �ȴ��жϲ���ȡ״̬ */
			if (s == USB_INT_DISK_READ)
			{  /* USB�洢�������ݿ�,�������ݶ��� */
				s = CH376ReadBlock(buf);  /* �ӵ�ǰ�����˵�Ľ��ջ�������ȡ���ݿ�,���س��� */
				xWriteCH376Cmd(CMD0H_DISK_RD_GO);  /* ����ִ��USB�洢���Ķ����� */
				buf += s;
			}
			else break;  /* ���ش���״̬ */
		}
		if (mBlockCount == 0)
		{
			s = Wait376Interrupt();  /* �ȴ��жϲ���ȡ״̬ */
			if (s == USB_INT_SUCCESS) return USB_INT_SUCCESS;  /* �����ɹ� */
		}
		if (s == USB_INT_DISCONNECT) return s;  /* U�̱��Ƴ� */
		CH376DiskReqSense();  /* ���USB�洢������ */
	}
	return s;  /* ����ʧ�� */
}

void CH376WriteHostBlock(PUINT8 buf, UINT8 len)  /* ��USB�����˵�ķ��ͻ�����д�����ݿ� */
{
	xWriteCH376Cmd(CMD10_WR_HOST_DATA);
	xWriteCH376Data(len);  /* ���� */
	if (len) 
	{
		do 
		{
			xWriteCH376Data(*buf);
			buf ++;
		} 
		while (--len);
	}
}

UINT8 CH376ReadBlock(PUINT8 buf)  /* �ӵ�ǰ�����˵�Ľ��ջ�������ȡ���ݿ�,���س��� */
{
	UINT8 s, l;
	xWriteCH376Cmd(CMD01_RD_USB_DATA0);
	s = l = xReadCH376Data();
	if (l)
	{
		do
		{
			*buf = xReadCH376Data();
			buf ++;
		}
		while (--l);
	}
	return s;
}

UINT8 CH376DiskQuery(PUINT32 DiskFree) 		/* ��ѯ����ʣ��ռ���Ϣ��������	*/
{
	UINT8	s, c0, c1, c2, c3;
	s = CH376SendCmdWaitInt(CMD0H_DISK_QUERY);
	if (s == USB_INT_SUCCESS) 
	{
		xWriteCH376Cmd(CMD01_RD_USB_DATA0);
		xReadCH376Data();  // ��������sizeof
		xReadCH376Data();  // DiskQuery.mTotalSector
		xReadCH376Data();
		xReadCH376Data();
		xReadCH376Data();
		c0 = xReadCH376Data();  // DiskQuery.mFreeSector
		c1 = xReadCH376Data();
		c2 = xReadCH376Data();
		c3 = xReadCH376Data();
		*DiskFree = c0 | (UINT16)c1 << 8 | (UINT32)c2 << 16 | (UINT32)c3 << 24;
		xReadCH376Data();  // DiskQuery.mDiskFat
	}
	else *DiskFree = 0;
	return s;
}

UINT8 CH376DiskCapacity(PUINT32 DiskCap)  /* ��ѯ������������,������ */
{
	UINT8	s;
	s = CH376SendCmdWaitInt(CMD0H_DISK_CAPACITY);
	if (s == USB_INT_SUCCESS) 
	{
		xWriteCH376Cmd(CMD01_RD_USB_DATA0);
		xReadCH376Data();  /* ��������sizeof(CH376_CMD_DATA.DiskCapacity) */
		*DiskCap = CH376Read32bitDat();  /* CH376_CMD_DATA.DiskCapacity.mDiskSizeSec,��CH376оƬ��ȡ32λ�����ݲ��������� */
	}
	else *DiskCap = 0;
	return s;
}

UINT8 CH376MatchFile(PUINT8 String, PUINT8 PathName, P_FAT_NAME MatchLish)	/* ƥ���ļ� */
{
	UINT8 s, FileCount, i;
	UINT8 xdata pBuf[64];
	PUINT8 pNameBuf;
	P_FAT_DIR_INFO pDir;
	if (NULL == String) return (DWIN_NULL_POINT);
	s = CH376FileOpenPath(PathName);
	if (ERR_OPEN_DIR != s) return s;		/* �򿪵Ĳ���Ŀ¼����Ŀ¼������ */	
	CH376SetFileName(String);
	xWriteCH376Cmd(CMD0H_FILE_OPEN);
	for (FileCount = 0; FileCount < DIR_FILE_MAX; FileCount++)
	{
		s = Wait376Interrupt();
		if (USB_INT_DISK_READ == s)
		{
			CH376ReadBlock(pBuf);
			xWriteCH376Cmd(CMD0H_FILE_ENUM_GO);
			pDir = (P_FAT_DIR_INFO)pBuf;
			if (pDir -> DIR_Name[0] == '.') continue;	/* . .. ֱ������ */
			if (pDir -> DIR_Name[0] == 0x05) pDir -> DIR_Name[0] = 0xE5;
			pNameBuf = MatchLish -> NAME;
			for (i = 0; i < 11; i++)					/* ת���ɱ�׼�ļ��� */
			{
				if (pDir -> DIR_Name[i] != 0x20) 
				{  /* ��Ч�ַ� */
					if (i == 8) 
					{
						*pNameBuf++ = '.';
					}
					*pNameBuf = pDir -> DIR_Name[i];  /* �����ļ�����һ���ַ� */
					pNameBuf++;
				}
			}
			*pNameBuf = 0;
			MatchLish -> Attr = pDir -> DIR_Attr;
			MatchLish++;
		}
		else if (ERR_MISS_FILE == s) break;		/* û���ҵ������ƥ���ļ� */ 		
	}
	if (DIR_FILE_MAX == FileCount)  CH376EndDirInfo();
	return s;
}

UINT8 CH376GetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir)
{
	UINT8 xdata Buf[64];
	UINT8 Status = 0;
	P_FAT_DIR_INFO pFile;
	memset(Buf, 0, sizeof(Buf));
	Status = CH376FileOpenPath(pFilePath);			
	if (Status != USB_INT_SUCCESS && Status != ERR_OPEN_DIR) 
	{
		CH376CloseFile(0);
		return DWIN_ERROR;
	}
	xWriteCH376Cmd(CMD1H_DIR_INFO_READ);
	xWriteCH376Data(0xFF);		
	Status = Wait376Interrupt();	
	if (Status != USB_INT_SUCCESS)
	{
		CH376CloseFile(0);
		return DWIN_ERROR;
	}
	CH376ReadBlock(Buf);
	pFile = (P_FAT_DIR_INFO)Buf;
	/* ���ݴ��С�˶��� ��Ϊ��˶����DGUS��Ӧ */
	pDir -> DIR_Attr 		=	(pFile -> DIR_Attr) ;
	pDir -> DIR_CrtTime 	=	(pFile -> DIR_CrtTime << 8) | (pFile -> DIR_CrtTime >> 8);
	pDir -> DIR_CrtDate 	=	(pFile -> DIR_CrtDate << 8) | (pFile -> DIR_CrtDate >> 8);
	pDir -> DIR_LstAccDate	=	(pFile -> DIR_LstAccDate << 8) | (pFile -> DIR_LstAccDate >> 8);
	pDir -> DIR_WrtTime 	=	(pFile -> DIR_WrtTime << 8) | (pFile -> DIR_WrtTime >> 8);
	pDir -> DIR_WrtDate 	=	(pFile -> DIR_WrtDate << 8) | (pFile -> DIR_WrtDate >> 8);
	pDir -> DIR_FileSize 	= 	((pFile -> DIR_FileSize >> 24) & 0x000000FF) | ((pFile -> DIR_FileSize >>  8) & 0x0000FF00) |
								((pFile -> DIR_FileSize <<  8) & 0x00FF0000) | ((pFile -> DIR_FileSize << 24) & 0xFF000000);
	CH376CloseFile(0);
	return DWIN_OK;
}

UINT8 CH376SetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir)
{
	UINT8 xdata Buf[64];
	UINT8 Status = 0;
	P_FAT_DIR_INFO pFile;
	memset(Buf, 0, sizeof(Buf));
	pFile = (P_FAT_DIR_INFO)Buf;
	Status = CH376FileOpenPath(pFilePath);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	xWriteCH376Cmd(CMD1H_DIR_INFO_READ);
	xWriteCH376Data(0xFF);
	Status = Wait376Interrupt();
	xWriteCH376Cmd(CMD20_WR_OFS_DATA);	/* ������CH376�ڲ�������д���������� */
	/* 1�����޸���Ϣ���͵�BUF������ */
	/* 2������ƫ�Ƶ�ַ */
	/* 3��д��������ݳ��� */
	/* 4����д���8λ���� ��д���8λ���� */
	if (pDir -> DIR_Attr != 0)
	{
		pFile -> DIR_Attr =  pDir -> DIR_Attr;
		xWriteCH376Data(0x0B);
		xWriteCH376Data(1);
		xWriteCH376Data(pFile -> DIR_Attr);
	}
	if (pDir -> DIR_CrtTime != 0)
	{
		pFile -> DIR_CrtTime =  pDir -> DIR_CrtTime;
		xWriteCH376Data(0x0E);
		xWriteCH376Data(2);
		xWriteCH376Data(pFile -> DIR_CrtDate);
		xWriteCH376Data(pFile -> DIR_CrtDate >> 8);
	}
	if (pDir -> DIR_CrtDate != 0)
	{
		pFile -> DIR_CrtDate =  pDir -> DIR_CrtDate;
		xWriteCH376Data(0x10);
		xWriteCH376Data(2);
		xWriteCH376Data(pFile -> DIR_CrtDate);
		xWriteCH376Data(pFile -> DIR_CrtDate >> 8);
	}
	if (pDir -> DIR_WrtTime != 0)
	{
		pFile -> DIR_WrtTime =  pDir -> DIR_WrtTime;
		xWriteCH376Data(0x16);
		xWriteCH376Data(2);
		xWriteCH376Data(pFile -> DIR_WrtTime);
		xWriteCH376Data(pFile -> DIR_WrtTime >> 8);
	}
	if (pDir -> DIR_WrtDate != 0)
	{
		pFile -> DIR_WrtDate =  pDir -> DIR_WrtDate;
		xWriteCH376Data(0x18);
		xWriteCH376Data(2);
		xWriteCH376Data(pFile -> DIR_WrtDate);
		xWriteCH376Data(pFile -> DIR_WrtDate >> 8);
	}
	xWriteCH376Cmd(CMD0H_DIR_INFO_SAVE);	/* ���ͱ����ļ�Ŀ¼��Ϣ���� */
	Status = Wait376Interrupt();
	CH376CloseFile(0);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	return DWIN_OK;	
}
UINT8 CH376WriteReqBlock(PUINT8 pbuf)  /* ���ڲ�ָ��������д����������ݿ�,���س��� */
{
	UINT8	s, l;
	xWriteCH376Cmd(CMD01_WR_REQ_DATA);
	s = l = xReadCH376Data();  /* ���� */
	if (l) 
	{
		do 
		{
			xWriteCH376Data(*pbuf);
			pbuf ++;
		}
		while (-- l);
	}
	return s;
}
UINT8 CH376CheckNameSum(PUINT8 pDirName)  /* ���㳤�ļ����Ķ��ļ��������,����Ϊ��С����ָ����Ĺ̶�11�ֽڸ�ʽ */
{
	UINT8 NameLen;
	UINT8 CheckSum;
	CheckSum = 0;
	for (NameLen = 0; NameLen != 11; NameLen++) 
		CheckSum = (CheckSum & 1 ? 0x80 : 0x00) + (CheckSum >> 1) + *pDirName++;
	return CheckSum;
}
UINT8 CH376ByteLocate(UINT32 offset)  /* ���ֽ�Ϊ��λ�ƶ���ǰ�ļ�ָ�� */
{
	xWriteCH376Cmd(CMD4H_BYTE_LOCATE);
	xWriteCH376Data((UINT8)offset);
	xWriteCH376Data((UINT8)((UINT16)offset>>8));
	xWriteCH376Data((UINT8)(offset>>16));
	xWriteCH376Data((UINT8)(offset>>24));
	return Wait376Interrupt();
}
UINT8 CH376ByteRead(PUINT8 pbuf, UINT16 ReqCount, PUINT16 pRealCount)  /* ���ֽ�Ϊ��λ�ӵ�ǰλ�ö�ȡ���ݿ� */
{
	UINT8 s;
	xWriteCH376Cmd(CMD2H_BYTE_READ);
	xWriteCH376Data((UINT8)ReqCount);
	xWriteCH376Data((UINT8)(ReqCount>>8));
	if (pRealCount) *pRealCount = 0;
	while (1) 
	{
		s = Wait376Interrupt();
		if (s == USB_INT_DISK_READ) 
		{
			s = CH376ReadBlock(pbuf);  /* �ӵ�ǰ�����˵�Ľ��ջ�������ȡ���ݿ�,���س��� */
			xWriteCH376Cmd(CMD0H_BYTE_RD_GO);
			pbuf += s;
			if (pRealCount) *pRealCount += s;
		}
		else return s;  /* ���� */
	}
}
UINT8 CH376LocateInUpDir(PUINT8 pPathName)  /* ���ϼ�Ŀ¼(�ļ���)���ƶ��ļ�ָ�뵽��ǰ�ļ�Ŀ¼��Ϣ���ڵ����� */
{
	UINT8	s;
	xWriteCH376Cmd(CMD14_READ_VAR32);
	xWriteCH376Data(VAR_FAT_DIR_LBA);  /* ��ǰ�ļ�Ŀ¼��Ϣ���ڵ�����LBA��ַ */
	for (s = 4; s != 8; s ++) GlobalBuf[s] = xReadCH376Data();  /* ��ʱ������ȫ�ֻ�������,��ԼRAM */
	s = CH376SeparatePath(pPathName);  /* ��·���з�������һ���ļ�������Ŀ¼��,�������һ���ļ�������Ŀ¼����ƫ�� */
	if (s) s = CH376FileOpenDir(pPathName, s);  /* �Ƕ༶Ŀ¼,�򿪶༶Ŀ¼�µ����һ��Ŀ¼,�����ļ����ϼ�Ŀ¼ */
	else s = CH376FileOpen("/");  /* ��Ŀ¼�µ��ļ�,��򿪸�Ŀ¼ */
	if (s != ERR_OPEN_DIR) return s;
	*(PUINT32)(&GlobalBuf[0]) = 0;  /* Ŀ¼����ƫ��������,������ȫ�ֻ�������,��ԼRAM */
	while (1) 
	{  /* �����ƶ��ļ�ָ��,ֱ���뵱ǰ�ļ�Ŀ¼��Ϣ���ڵ�����LBA��ַƥ�� */
		s = CH376SecLocate(*(PUINT32)(&GlobalBuf[0]));  /* ������Ϊ��λ���ϼ�Ŀ¼���ƶ��ļ�ָ�� */
		if (s != USB_INT_SUCCESS) return s;
		CH376ReadBlock(&GlobalBuf[8]);  /* ���ڴ滺������ȡCH376_CMD_DATA.SectorLocate.mSectorLba���ݿ�,���س�������sizeof(CH376_CMD_DATA.SectorLocate) */
		if (*(PUINT32)(&GlobalBuf[8]) == *(PUINT32)(&GlobalBuf[4])) return USB_INT_SUCCESS;  /* �ѵ���ǰ�ļ�Ŀ¼��Ϣ���� */
		xWriteCH376Cmd(CMD50_WRITE_VAR32);
		xWriteCH376Data(VAR_FAT_DIR_LBA);  /* �õ�ǰһ������,����Ϊ�µ��ļ�Ŀ¼��Ϣ����LBA��ַ */
		for (s = 8; s != 12; s ++) xWriteCH376Data(GlobalBuf[s]);
		++*(PUINT32)(&GlobalBuf[0]);
	}
}
UINT8 CH376LongNameWrite(PUINT8 pbuf, UINT16 ReqCount)  /* ���ļ���ר�õ��ֽ�д�ӳ��� */
{
	UINT8 s;
	xWriteCH376Cmd(CMD2H_BYTE_WRITE);
	xWriteCH376Data((UINT8)ReqCount);
	xWriteCH376Data((UINT8)(ReqCount>>8));
	while (1) 
	{
		s = Wait376Interrupt();
		if (s == USB_INT_DISK_WRITE) 
		{
			if (pbuf) pbuf += CH376WriteReqBlock(pbuf);  /* ���ڲ�ָ��������д����������ݿ�,���س��� */
			else 
			{
				xWriteCH376Cmd(CMD01_WR_REQ_DATA);  /* ���ڲ�ָ��������д����������ݿ� */
				s = xReadCH376Data();  /* ���� */
				while (s--) xWriteCH376Data(0);  /* ���0 */
			}
			xWriteCH376Cmd(CMD0H_BYTE_WR_GO);
		}
		else return s;  /* ���� */
	}
}
UINT8 CH376CreateLongName(PUINT8 pPathName, PUINT8 pLongName)  /* �½����г��ļ������ļ�,�ر��ļ��󷵻�,LongName����·��������RAM�� */
{
	UINT8 s, i;
	UINT8 DirBlockCnt;		/* ���ļ���ռ���ļ�Ŀ¼�ṹ�ĸ��� */
	UINT16 count;			/* ��ʱ����,���ڼ���,�����ֽڶ��ļ���ʽ��ʵ�ʶ�ȡ���ֽ��� */
	UINT16 NameCount;		/* ���ļ����ֽڼ��� */
	UINT32 NewFileLoc;		/* ��ǰ�ļ�Ŀ¼��Ϣ���ϼ�Ŀ¼�е���ʼλ��,ƫ�Ƶ�ַ */
	for (count = 0; count < LONG_NAME_BUF_LEN; count += 2) if (*(PUINT16)(&pLongName[count]) == 0) break;  /* ������λ�� */
	if (count == 0 || count >= LONG_NAME_BUF_LEN || count > LONE_NAME_MAX_CHAR) return ERR_LONG_NAME_ERR;  /* ���ļ�����Ч */
	DirBlockCnt = count / LONG_NAME_PER_DIR;  /* ���ļ���ռ���ļ�Ŀ¼�ṹ�ĸ��� */
	i = count - DirBlockCnt * LONG_NAME_PER_DIR;
	if (i) 
	{  /* ����ͷ */
		if (++DirBlockCnt * LONG_NAME_PER_DIR > LONG_NAME_BUF_LEN) return ERR_LONG_BUF_OVER;  /* ��������� */
		count += 2;  /* ����0��������ĳ��� */
		i += 2;
		if (i < LONG_NAME_PER_DIR) 
		{  /* ��ĩ���ļ�Ŀ¼�ṹ���� */
			while (i++ < LONG_NAME_PER_DIR) pLongName[count++] = 0xFF;  /* ��ʣ��������Ϊ0xFF */
		}
	}
	s = CH376FileOpenPath(pPathName);  /* �򿪶༶Ŀ¼�µ��ļ� */
	if (s == USB_INT_SUCCESS) 
	{   /* ���ļ��������򷵻ش��� */
		s = ERR_NAME_EXIST;
		goto CH376CreateLongNameE;
	}
	if (s != ERR_MISS_FILE) return s;
	s = CH376FileCreatePath(pPathName);  /* �½��༶Ŀ¼�µ��ļ� */
	if (s != USB_INT_SUCCESS) return s;
	i = CH376ReadVar8(VAR_FILE_DIR_INDEX);  /* ��ʱ���ڱ��浱ǰ�ļ�Ŀ¼��Ϣ�������ڵ������� */
	s = CH376LocateInUpDir(pPathName);  /* ���ϼ�Ŀ¼���ƶ��ļ�ָ�뵽��ǰ�ļ�Ŀ¼��Ϣ���ڵ����� */
	if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;  /* û��ֱ�ӷ�������Ϊ����Ǵ��˸�Ŀ¼��ô����Ҫ�رպ���ܷ��� */
	NewFileLoc = CH376ReadVar32(VAR_CURRENT_OFFSET) + i * sizeof(FAT_DIR_INFO);  /* ���㵱ǰ�ļ�Ŀ¼��Ϣ���ϼ�Ŀ¼�е���ʼλ��,ƫ�Ƶ�ַ */
	s = CH376ByteLocate(NewFileLoc);  /* ���ϼ�Ŀ¼���ƶ��ļ�ָ�뵽��ǰ�ļ�Ŀ¼��Ϣ��λ�� */
	if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
	s = CH376ByteRead(&GlobalBuf[ sizeof(FAT_DIR_INFO)], sizeof(FAT_DIR_INFO), NULL);  /* ���ֽ�Ϊ��λ��ȡ����,��õ�ǰ�ļ���Ŀ¼��ϢFAT_DIR_INFO */
	if ( s != USB_INT_SUCCESS ) goto CH376CreateLongNameE;
	for (i = DirBlockCnt; i != 0; --i) 
	{  /* �������е��ļ�Ŀ¼�ṹ���ڴ�ų��ļ��� */
		s = CH376ByteRead(GlobalBuf, sizeof(FAT_DIR_INFO), &count);  /* ���ֽ�Ϊ��λ��ȡ����,�����һ���ļ�Ŀ¼��ϢFAT_DIR_INFO */
		if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
		if (count == 0) break;  /* �޷���������,�ϼ�Ŀ¼������ */
		if (GlobalBuf[0] && GlobalBuf[0] != 0xE5) 
		{  /* ����������ʹ�õ��ļ�Ŀ¼�ṹ,���ڳ��ļ����������Ӵ��,���Կռ䲻��,���������ǰλ�ò����ת�� */
			s = CH376ByteLocate(NewFileLoc);  /* ���ϼ�Ŀ¼���ƶ��ļ�ָ�뵽��ǰ�ļ�Ŀ¼��Ϣ��λ�� */
			if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
			GlobalBuf[0] = 0xE5;  /* �ļ�ɾ����־ */
			for (s = 1; s != sizeof(FAT_DIR_INFO); s ++) GlobalBuf[s] = GlobalBuf[sizeof(FAT_DIR_INFO) + s];
			s = CH376LongNameWrite(GlobalBuf, sizeof(FAT_DIR_INFO));  /* д��һ���ļ�Ŀ¼�ṹ,����ɾ��֮ǰ�½����ļ�,ʵ�����Ժ�Ὣ֮ת�Ƶ�Ŀ¼����ĩλ�� */
			if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
			do 
			{  /* ����������е��ļ�Ŀ¼�ṹ */
				s = CH376ByteRead(GlobalBuf, sizeof(FAT_DIR_INFO), &count);  /* ���ֽ�Ϊ��λ��ȡ����,�����һ���ļ�Ŀ¼��ϢFAT_DIR_INFO */
				if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
			} 
			while (count && GlobalBuf[0]);  /* �����Ȼ������ʹ�õ��ļ�Ŀ¼�ṹ������������,ֱ���ϼ�Ŀ¼������������δʹ�ù����ļ�Ŀ¼�ṹ */
			NewFileLoc = CH376ReadVar32(VAR_CURRENT_OFFSET);  /* ���ϼ�Ŀ¼�ĵ�ǰ�ļ�ָ����Ϊ��ǰ�ļ�Ŀ¼��Ϣ���ϼ�Ŀ¼�е���ʼλ�� */
			i = DirBlockCnt + 1;  /* ��Ҫ�Ŀ��е��ļ�Ŀ¼�ṹ�ĸ���,�������ļ�������һ���ͳ��ļ��� */
			if (count == 0) break;  /* �޷���������,�ϼ�Ŀ¼������ */
			NewFileLoc -= sizeof(FAT_DIR_INFO);  /* ���ص��ղ��������Ŀ��е��ļ�Ŀ¼�ṹ����ʼλ�� */
		}
	}
	if (i) 
	{  /* ���е��ļ�Ŀ¼�ṹ�����Դ�ų��ļ���,ԭ�����ϼ�Ŀ¼������,���������ϼ�Ŀ¼�ĳ��� */
		s = CH376ReadVar8(VAR_SEC_PER_CLUS);  /* ÿ�������� */
		if (s == 128) 
		{  /* FAT12/FAT16�ĸ�Ŀ¼,�����ǹ̶���,�޷������ļ�Ŀ¼�ṹ */
			s = ERR_FDT_OVER;  /* FAT12/FAT16��Ŀ¼�µ��ļ���Ӧ������512��,��Ҫ�������� */
			goto CH376CreateLongNameE;
		}
		count = s * DEF_SECTOR_SIZE;  /* ÿ���ֽ��� */
		if (count < i * sizeof(FAT_DIR_INFO)) count <<= 1;  /* һ�ز���������һ��,�������ֻ�ᷢ����ÿ��Ϊ512�ֽڵ������ */
		s = CH376LongNameWrite( NULL, count );  /* ���ֽ�Ϊ��λ��ǰλ��д��ȫ0���ݿ�,��������ӵ��ļ�Ŀ¼�� */
		if ( s != USB_INT_SUCCESS ) goto CH376CreateLongNameE;
	}
	s = CH376ByteLocate(NewFileLoc);  /* ���ϼ�Ŀ¼���ƶ��ļ�ָ�뵽��ǰ�ļ�Ŀ¼��Ϣ��λ�� */
	if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
	GlobalBuf[11] = ATTR_LONG_NAME;
	GlobalBuf[12] = 0x00;
	GlobalBuf[13] = CH376CheckNameSum(&GlobalBuf[sizeof(FAT_DIR_INFO)]);  /* ���㳤�ļ����Ķ��ļ�������� */
	GlobalBuf[26] = 0x00;
	GlobalBuf[27] = 0x00;
	for (s = 0; DirBlockCnt != 0;) 
	{  /* ���ļ���ռ�õ��ļ�Ŀ¼�ṹ���� */
		GlobalBuf[0] = s ? DirBlockCnt : DirBlockCnt | 0x40;  /* �״�Ҫ�ó��ļ�����ڱ�־ */
		DirBlockCnt--;
		NameCount = DirBlockCnt * LONG_NAME_PER_DIR;
		for (s = 1; s < sizeof( FAT_DIR_INFO ); s += 2) 
		{  /* ������ļ���,���ļ������ַ��ڴ�����UNICODE��С�˷�ʽ��� */
			if (s == 1 + 5 * 2) s = 14;  /* �ӳ��ļ����ĵ�һ��1-5���ַ������ڶ���6-11���ַ� */
			else if (s == 14 + 6 * 2) s = 28;  /* �ӳ��ļ����ĵڶ���6-11���ַ�����������12-13���ַ� */
			GlobalBuf[s] = pLongName[NameCount++];
			GlobalBuf[s + 1] = pLongName[NameCount++];
		}
		s = CH376LongNameWrite(GlobalBuf, sizeof(FAT_DIR_INFO));  /* ���ֽ�Ϊ��λд��һ���ļ�Ŀ¼�ṹ,���ļ��� */
		if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
	}
	s = CH376LongNameWrite(&GlobalBuf[ sizeof(FAT_DIR_INFO)], sizeof(FAT_DIR_INFO));  /* ���ֽ�Ϊ��λд��һ���ļ�Ŀ¼�ṹ,����ת������֮ǰ�½����ļ���Ŀ¼��Ϣ */
CH376CreateLongNameE:
	CH376CloseFile(FALSE);  /* ���ڸ�Ŀ¼�����Ҫ�ر� */
	return s;
}