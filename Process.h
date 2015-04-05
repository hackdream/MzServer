#pragma once
#include <winSock2.h>
#include <vector>
#include <tlHelp32.h>
#include <vdmDbg.h>

#ifndef COMMAND_HEADER
#define COMMAND_HEADER
#include "Command.h"
#endif



typedef BOOL(CALLBACK *PROCENUMPROC)(DWORD, LPSTR);
typedef BOOL(WINAPI *LPFENUMPROCESSES)(DWORD *, DWORD cb, DWORD *);
typedef BOOL(WINAPI *LPFENUMPROCESSMODULES)(HANDLE, HMODULE *, DWORD, LPDWORD);
typedef DWORD(WINAPI *LPFGETMODULEFILENAMEEX)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef INT(WINAPI *LPFVDMENUMTASKWOWEX)(DWORD, TASKENUMPROCEX fp, LPARAM);

class CProcessEnumerator{
private:
	struct ENUMINFOSTRUCT{
		DWORD dwPID;
		PROCENUMPROC lpProc;
		DWORD lParam;
		BOOL bEnd;
	};

	HINSTANCE m_hInstLib;
	HINSTANCE m_hInstLib2;
	DWORD m_OSVersion;

	LPFENUMPROCESSES m_lpfEnumProcesses;
	LPFENUMPROCESSMODULES m_lpfEnumProcessModules;
	LPFGETMODULEFILENAMEEX m_lpfGetModuleFileNameEx;
	LPFVDMENUMTASKWOWEX m_lpfVDMEnumTaskWOWEx;
	
private:
	static BOOL Enum16(DWORD dwThreadId,
		WORD hMod16,
		WORD hTask16,
		PSZ pszModName,
		PSZ pszFielName,
		LPARAM lpUserDefined);
	BOOL EnumWinNTProcs(char *ProcsInfo);
	BOOL InitializeWinNT();

public:
	CProcessEnumerator();
	virtual ~CProcessEnumerator();
	BOOL EnumProcs(char *ProcsInfo);
	BOOL Initialize(DWORD OSVersion);
};




DWORD __stdcall ProcessdThread(LPVOID lparam);

DWORD __stdcall SendProcess(LPVOID lparam);

bool ImprovePrivilege();