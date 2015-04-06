#include "ConnectInfo.h"
#include <iostream>
using namespace std;

ConnectInfo::ConnectInfo(void)
{
	strcpy(ipAddress, "127.000.000.001");
	port = 8000;	
	strcpy(note, "12345678901234567890123456789012345678901234567890");
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