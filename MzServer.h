#include"StdAfx.h"
DWORD __stdcall ConnectThread(LPVOID lparam);
DWORD __stdcall RuningThread(LPVOID lparam);
DWORD __stdcall FileManageThread(LPVOID lparam);
DWORD __stdcall WindowManagerThread(LPVOID lpparam);
DWORD __stdcall MessageBoxThread(LPVOID lpparam);
DWORD __stdcall ProcessManagerThread(LPVOID lpparam);