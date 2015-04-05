#pragma once
#include<winsock2.h>

#ifndef COMMAND_HEADER
#define COMMAND_HEADER
#include "Command.h"
#endif

class CScreenManage
{
public:
	CScreenManage(void);
	~CScreenManage(void);
    
public:
	SOCKET m_MainSocket;

};

 

DWORD __stdcall ScreenManageThread(LPVOID lparam);

DWORD __stdcall SendScreen(LPVOID lparam);