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
TCHAR szWinName[]="MyWin";//��������
TCHAR str[255]="";//����������ַ���
bool isExist;
CWnd *pWnd;



//����screen�õ���
BITMAPFILEHEADER   bmfHdr; 
BYTE * pData;// ����λͼ������
BYTE * pChanged;//ͼ��仯������
BYTE * pCompress;
int m_ChangedOff;//��ȡ�仯������ʱʹ�õ�ƫ����
int m_DataOff;// ���±���֮ǰ����bufferʱ�õ�ƫ����
BOOL IsFirst;//�ж��ǲ���ͼ��ĵ�һ֡
char strFilePath[111] ;// λͼͼ�񱣴��λ��
int m_InfoSize;
int m_ScreenBits;
