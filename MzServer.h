//#include"StdAfx.h"
DWORD __stdcall ConnectThread(LPVOID lparam);
DWORD __stdcall RuningThread(LPVOID lparam);
DWORD __stdcall FileManageThread(LPVOID lparam);
DWORD __stdcall WindowManagerThread(LPVOID lpparam);
DWORD __stdcall MessageBoxThread(LPVOID lpparam);
DWORD __stdcall ProcessManagerThread(LPVOID lpparam);

DWORD __stdcall OpenDlg(LPVOID lpparam);


LRESULT CALLBACK WindowsFunc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void getScreenData(SOCKET Socket);



HWND hwndButton;
HWND hWnd;
HINSTANCE hInst;
TCHAR szWinName[]="MyWin";//窗口类名
TCHAR str[255]="";//保存输出的字符串
bool isExist;
CWnd *pWnd;



//接受screen用到的
BITMAPFILEHEADER   bmfHdr; 
BYTE * pData;// 保存位图的数据
BYTE * pChanged;//图像变化的数据
BYTE * pCompress;
int m_ChangedOff;//提取变化的数据时使用的偏移量
int m_DataOff;// 更新保存之前数据buffer时用到偏移量
BOOL IsFirst;//判断是不是图像的第一帧
char strFilePath[111] ;// 位图图像保存的位置
int m_InfoSize;
int m_ScreenBits;
