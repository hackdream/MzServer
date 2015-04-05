

#ifndef COMMAND_HEADER
#define COMMAND_HEADER
#include "Command.h"
#endif

 
DWORD  __stdcall CmdManageThread(LPVOID lparam);
static DWORD WINAPI WriteConsoleThread(LPVOID lparam);
static DWORD WINAPI ReadConsoleThread(LPVOID lparam);
void Clear();

#define KEY_SREEN   0x1002
#define KEY_NO_CTRL   0x1003
#define KEY_CTRL_C   0x1004
#define KEY_CTRL_BRAK 0x1005



typedef struct head{
DWORD packHead;   //0x12345678
DWORD packType; // 0x1002   0x1003   0x1004   0x1005
DWORD SreenBufferSize; //包大小
DWORD var_10004; //0x0
}HEAD;
//16字节

typedef struct key_info{
HEAD packhead;
INPUT_RECORD Key; //20
}KEY_INFO;

typedef struct console_buffer{   
UINT wCodePageID; 
DWORD Unkown1;   //'\0'
DWORD Unkown2; //'\0'
CONSOLE_SCREEN_BUFFER_INFO csb; //22字节   
UCHAR Character[8000]; 
WORD Attribute[8001];
}CONSOLE_BUFFER;

typedef struct sreen_info{
HEAD packhead; //16   
CONSOLE_BUFFER consoleBuffer;
}SREEN_INFO;