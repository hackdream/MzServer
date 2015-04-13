#include<winsock2.h>
#include<mstcpip.h>
#include <vector>
#include <string>
using std::vector;
using std::string;
#define SOCKET_CONNECT WM_USER + 1//将这个定义为任意一个值 用来作传递系统消息时的连接
#define CMD_HEARTBEAT WM_USER + 2   //心跳包
#define CMD_FILEMANAGE WM_USER + 3 //文件管理的命令
#define SOKCET_FILEMANAGE WM_USER + 4 //文件处理
#define CMD_FILEDRIVER WM_USER + 5 //获取磁盘信息的指令
#define CMD_SUCCEED WM_USER + 6 //响应成功
#define CMD_FILEDIRECTORY WM_USER + 7//获取磁盘目录信息的命令`
#define CMD_DIRFLODERERR  WM_USER + 8 //文件夹不可读
#define CMD_SCREEN_REQUEST WM_USER + 888  //屏幕传输请求
#define CMD_SCREENDLG_SHOW  WM_USER+ 889//显示屏幕窗口
#define CMD_GETFIRST_SCREEN WM_USER + 890 //请求屏幕第一帧
#define CMD_SCREEN_TO_SHOW WM_USER + 891 //显示获得的屏幕信息
#define CMD_CHANGE_SCREEN_BITS WM_USER + 892 //改变位图像素位数
#define CMD_FAILED WM_USER + 9 //响应失败

// 屏幕模拟击键
#define CMD_CTRLALTDEL   WM_USER + 893 // Ctrl + Alt + del
#define CMD_KEYDOWN      WM_USER + 894 //WM_KEYDOWN 
#define CMD_KEYUP        WM_USER + 895 //WM_KEYUP
#define CMD_MOUSEMOVE    WM_USER + 896//WM_MOUSEMOVE
#define CMD_LBUTTONDOWN  WM_USER + 897//WM_LBUTTONDOWN
#define CMD_LBUTTONUP    WM_USER + 898//WM_LBUTTONUP
#define CMD_LBUTTONDBLCLK WM_USER + 899//WM_LBUTTONDBLCLK
#define CMD_RBUTTONDOWN   WM_USER + 900//WM_RBUTTONDOWN   
#define CMD_RBUTTONUP     WM_USER + 901//WM_RBUTTONUP
#define CMD_RBUTTONDBLCLK WM_USER + 902//WM_RBUTTONDBLCLK
#define CMD_SCREEN_PRESS WM_USER + 903

// cmd  命令 
#define CMD_CMD_SHELL_REQUEST WM_USER + 910
#define CMD_SHELLDLG_SHOW WM_USER + 911
#define CMD_PROCESS_SHOW WM_USER + 913
#define CMD_REFRESH WM_USER + 915
#define CMD_ENDTASK WM_USER + 916



//  file  文件管理相关
#define CMD_GETFILE WM_USER + 1000
#define	MAX_FILE_DATA_BUFFER_SIZE			1024 * 8 // 文件一次最大发送数据长度
#define CMD_FILE_DELETE WM_USER + 1001
#define CMD_FILE_EXECUTE WM_USER + 1002
#define CMD_FILE_TO_CLIENT WM_USER + 1003


//窗口管理
#define CMD_WINDOW_MANAGER_DLG_SHOW WM_USER + 1100
#define CMD_SHOW_WINDOW_LIST WM_USER + 1101
#define CMD_WINDOW_DELETE WM_USER + 1102


//语音命令
#define CMD_VOICE WM_USER+300

// 打开网页
#define CMD_OPEN_URL WM_USER + 1201

struct ProcsInfo{
	string strFileName;
	DWORD dwProcessID;
	string strFilePath;
};

typedef struct tagSysInfo//获得电脑的系统信息
{
	char computerName[64];        
	char memorySize[32];
	char osVersion[64];
	char cpuInfo[32];
    char hasCamera[10];
}SysInfo, *pSysInfo;

typedef struct tagMsgHead//命令的消息头
{
	DWORD dwCmd;
	DWORD dwSize;
	DWORD dwExtend1;
	DWORD dwExtend2;
}MsgHead,*LPMsgHead;



//硬盘结构体
typedef struct tagDriver
{
	char driver[4];     //盘符名称
	char display[64];   //描述
	int  drivertype;    //盘类型
}DriverInfo, *LPDriverInfo;
//文件信息结构体
typedef struct tagFileInfo
{
    int  iType;         //1-目录，2-文件，3-无效目录
	char cFileName[261]; //文件名
	char cAttrib[32];   //文件属性
	char cTime[32];     //时间
	char cSize[32];     //文件大小	
}FileInfo,*LPFileInfo;

typedef struct tagWindowInfo{
	DWORD dwProcessID;
	char  strTitle[512];
}WindowInfo, *LPWindowInfo;

BOOL RecvData(SOCKET s,char *data,int len);
BOOL RecvMsg(SOCKET s,char  *pBuf,LPMsgHead lpMsgHead);

BOOL SendData(SOCKET S,char*  data ,int len);
BOOL SendMsg(SOCKET s,char const *pBuf,LPMsgHead lpMsgHead);
BOOL GetSystemInfo( SysInfo& info);

