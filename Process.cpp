#include "stdafx.h"
#include "Process.h"
//#include <Psapi.h>
//#pragma comment(lib,"Psapi.lib")
#include "psapi.h"
#include "ConnectInfo.h"
BOOL RecvKeyInfo();

CProcessEnumerator m_ProcessEnumViewer;
vector<ProcsInfo> m_ProcsInfo;
SOCKET ConnSocket;

int PEnd;
CProcessEnumerator::CProcessEnumerator(){
	m_OSVersion = 0;
	m_hInstLib = NULL;
	m_hInstLib2 = NULL;

	m_lpfEnumProcesses = NULL;
	m_lpfEnumProcessModules = NULL;
	m_lpfGetModuleFileNameEx = NULL;
	m_lpfVDMEnumTaskWOWEx = NULL;
}
CProcessEnumerator::~CProcessEnumerator(){
	if (m_hInstLib != NULL)FreeLibrary(m_hInstLib);
	if (m_hInstLib2 != NULL)FreeLibrary(m_hInstLib2);
}

BOOL CProcessEnumerator::EnumProcs(char *ProcsInfo){
	if (m_OSVersion == VER_PLATFORM_WIN32_NT){
		return EnumWinNTProcs(ProcsInfo);
	}
	else{
		return FALSE;
	}
	return TRUE;
}

BOOL CProcessEnumerator::Initialize(DWORD OSVersion){
	m_OSVersion = OSVersion;
	if (m_OSVersion == VER_PLATFORM_WIN32_NT){
		return InitializeWinNT();
	}
	return TRUE;
}

BOOL CProcessEnumerator::InitializeWinNT(){
	m_hInstLib = LoadLibraryA("PSAPI.DLL");
	if (m_hInstLib == NULL)return FALSE;
	m_hInstLib2 = LoadLibraryA("VDMDBG.DLL");
	if (m_hInstLib2 == NULL)return FALSE;
	m_lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *, DWORD, DWORD *))
		GetProcAddress(m_hInstLib, "EnumProcesses");
	m_lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,
		DWORD, LPDWORD))GetProcAddress(m_hInstLib,
		"EnumProcessModules");
	m_lpfGetModuleFileNameEx = (DWORD(WINAPI *)(HANDLE, HMODULE,
		LPTSTR, DWORD))GetProcAddress(m_hInstLib,
		"GetModuleFileNameExA");
	m_lpfVDMEnumTaskWOWEx = (INT(WINAPI *)(DWORD, TASKENUMPROCEX,
		LPARAM))GetProcAddress(m_hInstLib2, "VDMEnumTaskWOOWEx");
	if (m_lpfEnumProcesses == NULL ||
		m_lpfEnumProcessModules == NULL ||
		m_lpfGetModuleFileNameEx == NULL ||
		m_lpfVDMEnumTaskWOWEx == NULL){
		return FALSE;
	}
	return TRUE;
}

BOOL CProcessEnumerator::EnumWinNTProcs(char *ProcsInfo){
	LPDWORD lpdwPIDs;
	DWORD dwSize, dwSize2, dwIndex, lenInfo=0;
	HMODULE hMod;
	HANDLE hProcess;
	char szFilePath[MAX_PATH];
	ENUMINFOSTRUCT sInfo;
	dwSize2 = 256 * sizeof(DWORD);
	lpdwPIDs = NULL;
	do{
		if (lpdwPIDs){
			HeapFree(GetProcessHeap(), 0, lpdwPIDs);
			dwSize2 *= 2;
		}
		lpdwPIDs = (LPDWORD) HeapAlloc(GetProcessHeap(), 0, dwSize2);
		if (lpdwPIDs == NULL){
			return FALSE;
		}
		if (!m_lpfEnumProcesses(lpdwPIDs, dwSize2, &dwSize)){
			HeapFree(GetProcessHeap(), 0, lpdwPIDs);
			return FALSE;
		}
	} while (dwSize == dwSize2);
	dwSize /= sizeof(DWORD);
	/*TCHAR tmp[15];
	wsprintf(tmp, "%d", dwSize);
	MessageBox(0, tmp, "", MB_OK);*/
	for (dwIndex = 0; dwIndex < dwSize; dwIndex++){
		szFilePath[0] = 0;
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwPIDs[dwIndex]);
		if (hProcess != NULL){
			/*if (m_lpfEnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwSize2)){
				if (!m_lpfGetModuleFileNameEx(hProcess,
					hMod, szFilePath, sizeof(szFilePath))){
					szFilePath[0] = 0;
				}
			}*/
			GetModuleFileNameEx(hProcess, 0, szFilePath, MAX_PATH);
			CloseHandle(hProcess);
		}
		DWORD lastErr = GetLastError();
		if (strlen(szFilePath) != 0){
			char szFileName[MAX_PATH];
			char szFileExt[MAX_PATH];
			char szPIDs[MAX_PATH];
			_splitpath(szFilePath, NULL, NULL, szFileName, szFileExt);
			strcat(szFileName, szFileExt);
			itoa(lpdwPIDs[dwIndex], szPIDs,10);
			DWORD lenName = strlen(szFileName);
			DWORD lenPIDs = strlen(szPIDs);
			DWORD lenPath = strlen(szFilePath);
			DWORD x;
			for (x = 0; x < lenName; x++)ProcsInfo[lenInfo++] = szFileName[x];
			ProcsInfo[lenInfo++] = ';';
			for (x = 0; x < lenPIDs; x++)ProcsInfo[lenInfo++] = szPIDs[x];
			ProcsInfo[lenInfo++] = ';';
			for (x = 0; x < lenPath; x++)ProcsInfo[lenInfo++] = szFilePath[x];
			ProcsInfo[lenInfo++] = ';';
		}
	}
	ProcsInfo[lenInfo] = 0;
	HeapFree(GetProcessHeap(), 0, lpdwPIDs);
	return TRUE;
}

BOOL CheckOSVersion(DWORD &OSVersion){
	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (!GetVersionEx(&osver)){
		return FALSE;
	}
	OSVersion = osver.dwPlatformId;
	if ((OSVersion == VER_PLATFORM_WIN32_NT) ||
		(OSVersion == VER_PLATFORM_WIN32_WINDOWS)){
		return TRUE;
	}
	return FALSE;
}

bool ImprovePrivilege() {
	HANDLE hToken;
	bool fOk = false;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_ALL_ACCESS, &hToken)) {
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(0, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, 0, &tp, sizeof(tp), 0, 0);
		fOk = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return fOk;
}

BOOL Endtask(DWORD dwProcessID){
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
	if (hProcess != NULL){
		DWORD ExitCode;
		GetExitCodeProcess(hProcess, &ExitCode);
		if (TerminateProcess(hProcess, ExitCode) == FALSE)return FALSE;
	}
	return TRUE;
}

DWORD __stdcall ProcessdThread(LPVOID lparam){
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr = inet_addr(ConnectInfo::getConnectInfo()->ipAddress);
	ConnSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(ConnSocket, (PSOCKADDR) &LocalAddr, sizeof(LocalAddr)) == SOCKET_ERROR){
		closesocket(ConnSocket);
		return 0;
	}

	MsgHead msgHead;
	msgHead.dwCmd = CMD_PROCESS_SHOW;
	msgHead.dwSize = 0;
	if (!SendMsg(ConnSocket, NULL, &msgHead)){
		closesocket(ConnSocket);
		return 0;
	}
	PEnd = 0;
	char *chBuffer = new char[25005];
	chBuffer[0] = 0;
	DWORD OSVersion = 0;
	CheckOSVersion(OSVersion);
	m_ProcessEnumViewer.Initialize(OSVersion);
	if (!ImprovePrivilege()) {
		MessageBox(0, "Failed in adjust privilege!", "Runned in admin!", MB_OK);
	}
	while (1){
		if (!RecvMsg(ConnSocket, chBuffer, &msgHead))break;
		MsgHead MsgSend;
		switch (msgHead.dwCmd){
			case CMD_REFRESH:
			{
				m_ProcessEnumViewer.EnumProcs(chBuffer);
				MsgSend = msgHead;
				MsgSend.dwSize = strlen(chBuffer);
				SendMsg(ConnSocket, chBuffer, &MsgSend);
				break;
			}
			case CMD_ENDTASK:
			{
				Endtask(msgHead.dwExtend1);
				m_ProcessEnumViewer.EnumProcs(chBuffer);
				MsgSend.dwCmd = msgHead.dwCmd;
				MsgSend.dwSize = strlen(chBuffer);
				SendMsg(ConnSocket, chBuffer, &MsgSend);
				break;
			}
		}
	}
}

DWORD WINAPI ReadConsoleThread(LPVOID lparam){
	unsigned long BytesRead = 0;
	int bufsize = 1000;
	char *pReadBuffer = new char[1024];
	DWORD TotalBytesAvail;
	while (!PEnd){
		Sleep(100);
		//while (1){}
	}
	return TRUE;
}

//void Clear(){
//	closesocket(ConnSocket);
//	//CloseHandle(m_hReadPipeHandle);
//	
//	End = 1;
//}