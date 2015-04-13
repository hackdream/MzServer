#include "Voice.h"
#include "Command.h"
#include <mmeapi.h>
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include "ConnectInfo.h"

#pragma comment ( lib, "Winmm.lib" )
SOCKET ConnSocket1;
static const double		RecordTime						=	0.2;	// 录音长度（秒）
static const unsigned	SampleRate						=	16000;	// 比特率

const int buff_num=2;
#define	CANNELS							2
#define BITS_PER_SAMPLE					16
#define WAVEBUFFER_SIZE					((unsigned)(SampleRate*BITS_PER_SAMPLE/8*CANNELS*RecordTime))


void CALLBACK waveInProc (HWAVEIN hwi, UINT uMsg, UINT , DWORD_PTR dwParam1, DWORD_PTR dwParam2);
void CALLBACK waveOutProc (HWAVEOUT hwo, UINT uMsg, UINT, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
DWORD WINAPI Thread_WaveIn (LPVOID);
DWORD WINAPI Thread_WaveOut (LPVOID);

HWAVEIN wIn = 0;
WAVEFORMATEX wf = {0};
HWAVEOUT wOut = 0;

int DataSize = 0;

WAVEHDR waveInHeader[16] = {0};
WAVEHDR waveOutHeader[16] = {0};
unsigned waveInSafeBuffIndex = 0;


int ReadyIndex, NextIndex, SafeIndex;

HANDLE Event_WaveInFull;
HANDLE Event_WaveOutDone;
HANDLE T_WaveIn;
HANDLE T_WaveOut;


int DoAudio ()
{

	wf.cbSize = sizeof(wf);
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = CANNELS;
	wf.wBitsPerSample = BITS_PER_SAMPLE;
	wf.nBlockAlign = wf.nChannels  * wf.wBitsPerSample / 8;
	wf.nSamplesPerSec = SampleRate;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

	int retv = 0;
	{
		ZeroMemory (&waveOutHeader, sizeof(waveOutHeader));
		DataSize = WAVEBUFFER_SIZE;
		for(int i=0;i<buff_num;i++)
		{
			waveOutHeader[i].dwBufferLength = WAVEBUFFER_SIZE;
			waveOutHeader[i].dwFlags = 0;
			waveOutHeader[i].lpData = new char[WAVEBUFFER_SIZE];
		}


		retv = waveOutOpen ( &wOut, WAVE_MAPPER, &wf, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION );

		for(int i=0;i<buff_num;i++)
			waveOutPrepareHeader ( wOut , &waveOutHeader[i] , sizeof( WAVEHDR ) );

		Event_WaveOutDone = CreateEvent ( 0 , 0 , 0 , 0 );
	}
	waveInStart (wIn);
	SetEvent ( Event_WaveInFull );
	SetEvent ( Event_WaveOutDone );
	return retv;
}

void CALLBACK waveOutProc ( HWAVEOUT hwo, UINT uMsg, UINT, DWORD_PTR dwParam1, DWORD_PTR dwParam2 )
{
	if ( uMsg == WOM_DONE )
	{
		SetEvent ( Event_WaveOutDone );
	}
}

DWORD WINAPI Thread_WaveOut (LPVOID)
{
	SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_TIME_CRITICAL);
	while ( 1 )
	{

		WaitForSingleObject ( Event_WaveOutDone , -1 );
		static int index = 0;
		index = (index+1)%buff_num;
		waveOutWrite ( wOut , &waveOutHeader[index] , sizeof( WAVEHDR ) );
	}
}
sockaddr_in ServerAddr;
SOCKET socket_client;
int init_socket()
{
	WSADATA WSAdata;
	if(WSAStartup(MAKEWORD(1,1),&WSAdata)!=0)
	{
		printf("初始化失败\n");
		return -1;
	}
	return 0;
}
int create_socket()
{
	socket_client=socket(AF_INET,SOCK_STREAM,0);
	if(socket_client == SOCKET_ERROR)
	{
		printf("创建socket失败\n");
		WSACleanup();
		return -1;
	}
	return 0;
}
int call_server()
{
	ServerAddr.sin_family      = AF_INET;
	ServerAddr.sin_port        = htons(8100);
	ServerAddr.sin_addr.s_addr =  inet_addr(ConnectInfo::getConnectInfo()->ipAddress);
	while(connect(socket_client,(struct sockaddr*)&ServerAddr,sizeof(ServerAddr)) == SOCKET_ERROR)
		printf("Connect...\n");
	return 0;
}
DWORD __stdcall VoiceThread(LPVOID lparam){
	init_socket();
	create_socket();
	call_server();

	DoAudio ();
	MsgHead msgHead;

	SetEvent ( Event_WaveOutDone );
	while (1)
	{
		static int index = 0;

		WaitForSingleObject ( Event_WaveOutDone , -1 );

		if (index-1==-1)
		{
			char x[100000]="0";
			if (recv(socket_client,(char *)&x,WAVEBUFFER_SIZE,0)<=0)
			{
				closesocket(socket_client);
				TerminateThread(T_WaveOut, 0);
				for(int i=0;i<buff_num;i++)
				{
					delete waveOutHeader[i].lpData;
				}
				return 0; 
			}
			memcpy ( waveOutHeader[buff_num-1].lpData , x , WAVEBUFFER_SIZE );
			waveOutWrite ( wOut , &(WAVEHDR)waveOutHeader[buff_num-1] , sizeof( WAVEHDR ) );
		}
		else
		{
			char xx[100000]="0";
			if (recv(socket_client,(char *)&xx,WAVEBUFFER_SIZE,0)<=0)
			{
				closesocket(socket_client);
				TerminateThread(T_WaveOut, 0);
				for(int i=0;i<buff_num;i++)
				{
					delete waveOutHeader[i].lpData;
				}
				return 0; 
			}
			memcpy ( waveOutHeader[index-1].lpData , xx , WAVEBUFFER_SIZE );
			waveOutWrite ( wOut , &(WAVEHDR)waveOutHeader[index-1] , sizeof( WAVEHDR ) );
		}
		index = (index+1)%buff_num;
	}
}
