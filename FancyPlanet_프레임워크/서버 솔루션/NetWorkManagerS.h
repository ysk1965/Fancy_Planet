#pragma once
#include "stdafx.h"
#define SERVERPORT 9000
#define BUFSIZE 0
class NetWorkManager
{
public:
	NetWorkManager();
	~NetWorkManager();
	int InitNetWorkManager();
	void err_display(const char *msg);
	void err_quit(const char *msg);
	int recvn(SOCKET s, char *buf, int len, int flags);
	int AcceptClient();
private:

	WSADATA			m_wsaData;
	SOCKET			listen_sock;
	SOCKADDR_IN		Serveraddr;
	int				retval;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	char buf[BUFSIZE+1];

};