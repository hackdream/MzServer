#include "ConnectInfo.h"
#include <iostream>
using namespace std;

ConnectInfo::ConnectInfo(void)
{
	strcpy(ipAddress, "127.0.0.1");
	port = 8000;	
	strcpy(note, "student");	
}


ConnectInfo::~ConnectInfo(void)
{
	delete pConnectInfo;
}

ConnectInfo* ConnectInfo::pConnectInfo = NULL;
ConnectInfo* ConnectInfo::getConnectInfo()
{
	if(pConnectInfo == NULL){
		pConnectInfo = new ConnectInfo();
	}
	return pConnectInfo;
}