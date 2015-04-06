#pragma once
class ConnectInfo
{
public:
	ConnectInfo(void);
	~ConnectInfo(void);
     char ipAddress[15];  //连接ip地址
	 int port;      //连接端口
	 char note[50];		//上线备注、分类
	static ConnectInfo* getConnectInfo();//获取该类型实例
	static ConnectInfo *pConnectInfo;
};

