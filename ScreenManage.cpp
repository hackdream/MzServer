#include "ScreenManage.h"
#include <stdio.h>
#include "ConnectInfo.h"
#include "./zlib.h"
#pragma comment(lib,"./zlib.lib")	//ͼ����������ѹ��ʹ��zlib�⺯��

int g_ScreenBits;//ͼ���λ��

CScreenManage::CScreenManage(void)
{
	g_ScreenBits = 8;
}


CScreenManage::~CScreenManage(void)
{

}




DWORD __stdcall ScreenManageThread(LPVOID lparam)//�̴߳�����Ļ����
{
	/* DWORD *pParam   =   (DWORD   *)lparam; 
	CScreenManage Screen;
	Screen.m_MainSocket = *pParam;
	SOCKET MainSocket = Screen.m_MainSocket;
	*/
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port = htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr= inet_addr(ConnectInfo::getConnectInfo()->ipAddress);

	SOCKET MainSocket = socket(AF_INET, SOCK_STREAM, 0);//���½���һ��ר�ŵ�socket�Ϳͻ��˽��н���
	if(connect(MainSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR)
	{
		closesocket(MainSocket);
		return 0;//connect error
	}

	//================================================================================
	MsgHead msgHead;
	char *chBuffer = new char[1536 * 1024]; //���ݽ����� 1.5MB

	//send socket type 
	msgHead.dwCmd = CMD_SCREENDLG_SHOW;
	msgHead.dwSize = 0;
	if(!SendMsg(MainSocket, NULL, &msgHead))
	{
		if(chBuffer != NULL)
			delete []chBuffer;
		return 0; 
	}
	// ::MessageBox(NULL, "��һ��while(1)ǰû�б���", "DD", MB_OK);
	while(1)
	{
		//��������
		if(!RecvMsg(MainSocket, chBuffer, &msgHead))
			break;

		//��������
		switch(msgHead.dwCmd)
		{
		case CMD_GETFIRST_SCREEN://�����ȡ��Ļ��Ϣ
			{
				//::MessageBox(NULL, "���ڴ���bmpͼ��","",MB_OK);
				//����Ϊ��Ļ��Ϣ�Ļ�ȡ ��һֱ��ȡ�����͡�
				DWORD *pDWORD = new DWORD;
				*pDWORD = MainSocket;
				g_ScreenBits = msgHead.dwExtend1;
				CreateThread(NULL, NULL, SendScreen, pDWORD, NULL, NULL);

				break;
			}
		case CMD_CHANGE_SCREEN_BITS:
			{
				g_ScreenBits = msgHead.dwExtend1;
				break;
			}

		default:
			{
				::MessageBox(NULL, "������˼��screen�յ����������Ϣ","", MB_OK);
				break;
			} 
		}
		//char str[111];
		//if(!SendMsg(MainSocket, chBuffer, &msgHead))//MSGHEAD Ϊ7
		//	break;
	}


	if(chBuffer != NULL)
		delete[] chBuffer;
	closesocket(MainSocket);
	return 0;
}





DWORD __stdcall SendScreen(LPVOID lparam)//�̴߳�����Ļ����
{

	DWORD *pParam   =   (DWORD   *)lparam; 
	SOCKET MainSocket =*pParam; 
	DWORD dwLastSend;

	int ScreenBits = 0;
	BYTE * pLastData = new BYTE [2048 * 1536 *3 * 2];//Ϊ�ֱ���2048 * 1536 ��λΪ24λ��3���ֽڵĴ�С�� 2��  
	BYTE * pChanged = new BYTE [2048 *1536 * 3 * 2];

	HWND hWnd = GetDesktopWindow();//�����Ļ��HWND.
	HDC hScreenDC = GetDC(hWnd);   //�����Ļ��HDC.

	RECT rect; 
	//�ú�������ָ�����ڵı߿���εĳߴ硣�óߴ����������Ļ�������Ͻǵ���Ļ���������
	GetWindowRect(hWnd,&rect);
	SIZE screensize;
	screensize.cx=rect.right-rect.left;
	screensize.cy=rect.bottom-rect.top;
	//CreateCompatibleBitmap�ú���������ָ�����豸hScreenDC������ص��豸���ݵ�λͼ��
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hScreenDC,screensize.cx,screensize.cy);

	HDC MemDC = CreateCompatibleDC(hScreenDC);//��ǰ��Ļ��DC

	BOOL  IsFirst = TRUE;

	memset(pLastData, 0, 2048 * 1536 *3 * 2);


	while(1)
	{

		if(ScreenBits != g_ScreenBits)
		{
			ScreenBits = g_ScreenBits;
			IsFirst = TRUE;
		}
		HWND hWnd = GetDesktopWindow(); 
		GetWindowRect(hWnd,&rect);
		int ChangedSize = 0;//�ı��˵����ݵĴ�С

		//����Ļͼ���͵�MemDC�е�λͼ��
		HGDIOBJ hOldBMP = ::SelectObject(MemDC,hBitmap);
		::BitBlt(MemDC,0,0,screensize.cx,screensize.cy,hScreenDC,rect.left,rect.top,SRCCOPY);
		::SelectObject(MemDC,hOldBMP);
		/*********��λͼINFOHEADER ������ɫ�壩��ͼ������ ����pData***********************/
		dwLastSend = GetTickCount();
		HDC hDC =::CreateDC("DISPLAY",NULL,NULL,NULL);
		int iBits = ::GetDeviceCaps(hDC, BITSPIXEL) * ::GetDeviceCaps(hDC, PLANES);//��ǰ�ֱ�����ÿ��������ռ�ֽ���
		::DeleteDC(hDC);

		DWORD   dwPaletteSize=0;	//��ɫ���С�� λͼ�������ֽڴ�С 
		if (ScreenBits<= 8)		
			dwPaletteSize = (1 <<  ScreenBits) *	sizeof(RGBQUAD);	

		BITMAP  bm;        //λͼ���Խṹ
		::GetObject(hBitmap, sizeof(bm), (LPSTR)&bm);

		BITMAPINFOHEADER   bi;       //λͼ��Ϣͷ�ṹ     
		bi.biSize            = sizeof(BITMAPINFOHEADER);  
		bi.biWidth           = bm.bmWidth;
		bi.biHeight          = bm.bmHeight;
		bi.biPlanes          = 1;
		bi.biBitCount        = ScreenBits;
		bi.biCompression     = BI_RGB; //BI_RGB��ʾλͼû��ѹ��
		bi.biSizeImage       = 0;
		bi.biXPelsPerMeter   = 0;
		bi.biYPelsPerMeter   = 0;
		bi.biClrUsed         = 0;
		bi.biClrImportant    = 0;

		DWORD dwBmBitsSize = ((bm.bmWidth *  ScreenBits+31)/32) * 4 * bm.bmHeight;//λͼ���ݵĴ�С

		int DataPalInfoSize = dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER);//λͼ����FILEHDEAR��  ����3���ִ�С�ĺ�

		//	HANDLE hDib = ::GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));  //1Ϊλͼ���ݷ����ڴ�  
		LPBITMAPINFOHEADER pData = (LPBITMAPINFOHEADER)new BYTE [DataPalInfoSize];  //2
		//LPBITMAPINFOHEADER pData = (LPBITMAPINFOHEADER)GlobalLock(hDib);  //new BYTE [DataPalInfoSize];//3
		*pData = bi;//2
		HANDLE hPal = ::GetStockObject(DEFAULT_PALETTE);  // �����ɫ�� 
		HANDLE  hOldPal=NULL; 
		if (hPal) 
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC,(HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}
		//�����ݱ�����ָ��(LPSTR)lpbi + sizeof(BITMAPINFOHEADER)+dwPaletteSizeָ���λ��
		//::GetDIBits(hDC, hBitmap, 0, (UINT) bm.bmHeight,(LPSTR)pData + sizeof(BITMAPINFOHEADER)+dwPaletteSize,(LPBITMAPINFO)&bi,DIB_RGB_COLORS);// ��ȡ�õ�ɫ�����µ�����ֵ
		::GetDIBits(hDC, hBitmap, 0, (UINT) bm.bmHeight,(LPSTR)pData + sizeof(BITMAPINFOHEADER)+dwPaletteSize,(BITMAPINFO*)pData,DIB_RGB_COLORS);// ��ȡ�õ�ɫ�����µ�����ֵ

		//���ļ���Ϣͷ���浽bi
		bi = *pData;
		int DataSize = bi.biSizeImage;//λͼ�����ݵ��ֽ���  ����������
		//memcpy(pData, &bi, sizeof(bi));
		if (hOldPal)//�ָ���ɫ��
		{
			SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}	
		//(BITMAPINFO*)pData
		/****************************�Ƚϵõ��仯������***********************************/



		BYTE *p1, *p2;
		int OneLineByte = DataSize / bm.bmHeight;  //һ�е��ֽ���
		BYTE *NowScreenOff = (BYTE*)pData + sizeof(BITMAPINFOHEADER)+dwPaletteSize;
		BYTE *LastScreenOff = pLastData + sizeof(BITMAPINFOHEADER)+dwPaletteSize;
		if(!IsFirst)
		{

			rect.top = 99999; rect.left = 99999; rect.bottom = -99999; rect.right = -99999; 

			for(int i = 0; i< bm.bmHeight ; i += 5)//û���5�� ���һ������  ע�����ﵥλ������
			{
				for(int j = 0; j < OneLineByte; j += 2)//bm.bmWidthBytesһ�е��ֽ��� 
				{
					p1 = (BYTE*)(NowScreenOff + i * OneLineByte + j);
					p2 = (BYTE*)(LastScreenOff + i * OneLineByte + j);
					if(*p1 != *p2)
					{
						if(rect.top > i)
						{ 
							rect.top = i;
							rect.top =  rect.top > 5 ? (rect.top - 5) : 0; 
						}
						if(rect.bottom < i)
						{
							rect.bottom = i;
							if( i+ 5 >= bm.bmHeight) rect.bottom = bm.bmHeight -1;
						}

						//j / (g_ScreenBits/8) Ϊj�ֽ����ڵ����ص�ı��  leftӦ��΢��ǰЩ �ò��������΢������ȷ ����-2 
						if(rect.left > (j-2) * 8 /ScreenBits )//��ʵ��(j-2) / (ScreenBits/8)) 
						{
							rect.left = (j-2) * 8 /ScreenBits;
							if(rect.left < 0) rect.left = 0;
						}
						//left��΢����Щ�� ���Բ���2 ���Ҽ�1������ ע�������� rect����������
						if(rect.right < j * 8 /ScreenBits + 1)//��ʵ��j/(ScreenBits/8) +1)
						{
							rect.right = j * 8 /ScreenBits + 1;
							if(rect.right >= bm.bmWidth) rect.right = bm.bmWidth - 1;
						}


					}

				}
			}
			//�������Ĳ��Ǻܾ�׼  �������⼸����΢�����·�Χ 
			rect.top = rect.top - 5;
			if(rect.top < 0) rect.top =0;
			rect.bottom += 5;
			if(rect.bottom >= bm.bmHeight) rect.bottom = bm.bmHeight -1;
			rect.left -= 5;
			if(rect.left < 0) rect.left =0;
			rect.right += 5;
			if(rect.right >= bm.bmWidth) rect.right = bm.bmWidth - 1;

			//����bmp��Ϣͷ ��ɫ�� �Լ� �仯����Ĵ�С�����ݵ�pChanged
			memcpy(pChanged, pData, sizeof(BITMAPINFOHEADER)+dwPaletteSize);
			ChangedSize = sizeof(BITMAPINFOHEADER)+dwPaletteSize;
			memcpy(pChanged + ChangedSize, &rect, sizeof(rect));
			ChangedSize += sizeof(rect);
			BYTE *Dst, *Src;
			int BitSz;
			BitSz = (rect.right - rect.left + 1) * ScreenBits / 8;
			for(int i = rect.top; i <= rect.bottom  ; i++) 
			{
				Dst = pChanged + ChangedSize;
				Src = NowScreenOff + i * OneLineByte + rect.left * ScreenBits / 8;
				memcpy(Dst, Src, BitSz);//����ÿһ�б仯������
				ChangedSize += BitSz;
			}
			//һ�д�����Ϻ� �����֡��ͼ����������һ֡���бȽ�
			memcpy(pLastData, pData, DataPalInfoSize); //�����ݱ��浽pLastData
		}


		/*************************������Ϣ******************************/
		BITMAPFILEHEADER   bmfHdr; //λͼ�ļ�ͷ�ṹ     
		bmfHdr.bfType = 0x4D42;  // "BM"  	// ����λͼ�ļ�ͷ
		DWORD dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;  
		bmfHdr.bfSize = dwDIBSize;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

		//ѹ����pDataCompressd
		DWORD CompressdSize;
		BYTE* pDataCompressd;
		if(!IsFirst)
		{
			CompressdSize = compressBound(ChangedSize); // ѹ����ĳ����ǲ��ᳬ��CompressdSize  
			pDataCompressd = new BYTE [CompressdSize];
			pDataCompressd[0] = IsFirst;
			compress(pDataCompressd + 1, &CompressdSize, (BYTE*)pChanged, ChangedSize);
		}
		else
		{
			CompressdSize = compressBound(DataPalInfoSize); // ѹ����ĳ����ǲ��ᳬ��CompressdSize  
			pDataCompressd = new BYTE [CompressdSize];
			pDataCompressd[0] = IsFirst;
			compress(pDataCompressd + 1, &CompressdSize, (BYTE*)pData, DataPalInfoSize);		 
		}
		MsgHead MsgSend;
		MsgSend.dwCmd = CMD_SCREEN_TO_SHOW;
		MsgSend.dwSize = CompressdSize + 1;
		MsgSend.dwExtend1 = bmfHdr.bfSize;
		MsgSend.dwExtend2 = bmfHdr.bfOffBits;
		if(!SendMsg(MainSocket, (char*)pDataCompressd, &MsgSend))
		{
			::DeleteObject(MemDC);
			::ReleaseDC(hWnd,hScreenDC);
			DeleteObject(hBitmap);

			delete [] pLastData;
			delete [] pDataCompressd;
			delete [] pData;
			delete [] pChanged;
			//::MessageBox(NULL, "����ʧ��","",MB_OK);
			return 0;
		}
		IsFirst = FALSE;

		delete [] pDataCompressd;
		delete [] pData;
		if ((GetTickCount() - dwLastSend) < 110)
			Sleep(100);

	}

	return 0;
}


