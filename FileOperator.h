#if !defined(AFX_FILEOPERATOR_H)
#define AFX_FILEOPERATOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifndef COMMAND_HEADER
#define COMMAND_HEADER
#include "Command.h"
#endif
#include <atlstr.h>
///////////////////////////////////////////////////////////
//
//����Ӳ����Ϣ
void FileListDirver(char *pBuf, LPMsgHead lpMsgHead);

//�����׽��������ض˽������� ������Ϣ
DWORD MainFileManage();

 

//ö���ļ�·��
void FileListDirectory(char *pBuf, LPMsgHead lpMsgHead);

int fileToServer(char *pLocalPath, LPMsgHead lpMsgHead, SOCKET  FileSocket);


//�ļ�ɾ��
void fileDelete(char *pBuf);
void deleteDirectory(CString path);

//�����ļ�
void FileExec(char *pBuf, LPMsgHead);

// �����ļ�
void recvClientFile(char *pLocalPath, LPMsgHead lpMsgHead, SOCKET  FileSocket);
/*
//ճ���ļ�
void FilePaste(char *pBuf, LPMsgHead lpMsgHead);

//�������ļ�
void FileReName(char *pBuf, LPMsgHead lpMsgHead);
 */
#endif // !defined(AFX_FILEOPERATOR_H)
