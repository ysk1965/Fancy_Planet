#pragma once

#include "stdafx.h"
#define SERVERPORT 9000
#define BUFSIZE 0
class NetWorkManager
{
private:
	NetWorkManager() {};
	NetWorkManager(const NetWorkManager& other);

	static NetWorkManager* instance;

public:

	static NetWorkManager* GetInstance()
	{
		return instance;
	}
	

	~NetWorkManager();
	void InitNetWorkManager();
	void err_display(const char *msg);
	void err_quit(const char *msg);
	int recvn(SOCKET s, char *buf, int len, int flags);
	void AcceptClient();
	//bool PacketProcess(const unsigned char* pBuf, stClientInfo* pClientInfo);
	static DWORD WINAPI Worker_Thread(void *arg);
private:
	static HANDLE  m_hCompletionPort;
	
	
	SOCKET			listen_sock;
	
	SOCKADDR_IN		Serveraddr;
	int				retval;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	
};