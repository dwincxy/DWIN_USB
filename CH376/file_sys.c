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
#include <string.h>

/********************************�ڲ���������*********************************/
UINT8	Wait376Interrupt(void);
UINT8	Query376Interrupt(void);
void	CH376SetFileName(PUINT8 name);
UINT8	CH376SeparatePath(PUINT8 path);
UINT8	CH376FileOpenDir(PUINT8 PathName, UINT8 StopName);
UINT8	CH376FileCreate(PUINT8 PathName);
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
	while (Query376Interrupt() == FALSE);	/* һֱ���ж� */
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

UINT8 CH376FileOpen(PUINT8 name)  /* �ڸ�Ŀ¼���ߵ�ǰĿ¼�´��ļ�����Ŀ¼(�ļ���) */
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
		else CH376SetFileName(PathName);  /* û��·���ָ���,�Ǹ�Ŀ¼���ߵ�ǰĿ¼�µ��ļ�����Ŀ¼,���ý�Ҫ�������ļ����ļ��� */
	}
	return (CH376SendCmdWaitInt(CMD0H_FILE_ERASE));
}

UINT8 CH376FileOpenPath(PUINT8 PathName)  /* �򿪶༶Ŀ¼�µ��ļ�����Ŀ¼(�ļ���),֧�ֶ༶Ŀ¼·��,֧��·���ָ���,·�����Ȳ�����255���ַ� */
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

UINT8 CH376DiskQuery(PUINT32 DiskFree) 		//��ѯ����ʣ��ռ���Ϣ��������
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
	UINT8 pBuf[64];
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