#if !defined(AFX_FILEOPERATOR_H)
#define AFX_FILEOPERATOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef COMMAND_HEADER
#define COMMAND_HEADER
#include "Command.h"
#endif
///////////////////////////////////////////////////////////
//
//����Ӳ����Ϣ
void FileListDirver(char *pBuf, LPMsgHead lpMsgHead);

//�����׽��������ض˽������� ������Ϣ
DWORD MainFileManage();

 

//ö���ļ�·��
void FileListDirectory(char *pBuf, LPMsgHead lpMsgHead);
/*

//�ļ�ɾ��
void FileDelete(char *pBuf, LPMsgHead lpMsgHead);

//�����ļ�
void FileExec(char *pBuf, LPMsgHead lpMsgHead);

//ճ���ļ�
void FilePaste(char *pBuf, LPMsgHead lpMsgHead);

//�������ļ�
void FileReName(char *pBuf, LPMsgHead lpMsgHead);
 */
#endif // !defined(AFX_FILEOPERATOR_H)
