
#include"stdafx.h"
#include "NetWorkManager.h"
NetWorkManager* NetWorkManager::instance = new NetWorkManager;
HANDLE NetWorkManager::m_hCompletionPort = NULL;


NetWorkManager::~NetWorkManager()
{
	closesocket(listen_sock);
	closesocket(client_sock);
	WSACleanup();
}

void NetWorkManager::InitNetWorkManager()
{


	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0); //completionport 생성
	HANDLE hResult = NULL;

	for (int i = 0; i < 4; ++i)
	{
		DWORD ThreadId;
		hResult = CreateThread(NULL, 0, Worker_Thread, (LPVOID)i, 0, (LPDWORD)&ThreadId);
		if (hResult == NULL)
		{
			printf("워커스레드 생성 실패\n");

		}
	}

	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);//WSA소켓 생성

	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");
	printf(" bind, listen 완료\n");

	//return retval;
}

//bool PacketProcess(const unsigned char* pBuf, stClientInfo* pClientInfo)

DWORD WINAPI NetWorkManager::Worker_Thread(void* arg)
{

	//HANDLE hcp = (HANDLE)arg;
	int retval;
	DWORD dwIOSize;
	stClientChessInfo* ChessClientInfo = new stClientChessInfo;//구조체 클래스 생성
	while (1)
	{
		retval = GetQueuedCompletionStatus(m_hCompletionPort, &dwIOSize, (PULONG_PTR)arg, (LPOVERLAPPED*)ChessClientInfo, INFINITE);
		int addrlen = sizeof(clientaddr);
		switch (ChessClientInfo->m_stRecvOverlappedEx.m_eOperation)
		{
		case true:
		{
			WSASend(ChessClientInfo->m_socketClient, &(ChessClientInfo->m_stRecvOverlappedEx.m_wsaBuf),
				1, NULL, 0, &ChessClientInfo->m_stRecvOverlappedEx.m_wsaOverlapped, NULL);
			delete ChessClientInfo;
		}
		//return 1;
		case false:
		{
			WSARecv(ChessClientInfo->m_socketClient, &(ChessClientInfo->m_stRecvOverlappedEx.m_wsaBuf),
				1, NULL, 0, &ChessClientInfo->m_stRecvOverlappedEx.m_wsaOverlapped, NULL);
		}
		//return 1;
		}
	}
}

void NetWorkManager::AcceptClient()
{
	int key = 0;
	while (1)
	{	// accept()

		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);


		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
		}

		HANDLE hResult = CreateIoCompletionPort((HANDLE)client_sock, m_hCompletionPort, (DWORD)client_sock, 0);
		if (hResult)
		{
			key = +1;
		}
		stClientChessInfo *ClientChessInfo = new stClientChessInfo;
		if (ClientChessInfo == NULL)
			err_quit("클라이언트 구조체 생성 실패");

		ZeroMemory(&(ClientChessInfo->m_stRecvOverlappedEx), sizeof(ClientChessInfo->m_stRecvOverlappedEx));
		ClientChessInfo->m_socketClient = client_sock;
		ClientChessInfo->m_stRecvOverlappedEx.m_wsaBuf = { 0 };
		DWORD recvbytes;
		DWORD flags = 0;
			retval = WSARecv(client_sock, &(ClientChessInfo->m_stRecvOverlappedEx.m_wsaBuf), 1
			, &recvbytes, &flags, &(ClientChessInfo->m_stRecvOverlappedEx.m_wsaOverlapped), NULL);
		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				err_display("WSARecv()");
			}
		}

		else printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		// 클라이언트와 데이터 통신

	}


}
void NetWorkManager::err_quit(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	//exit(1);
}

// 소켓 함수 오류 출력
void NetWorkManager::err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	//DisplayText("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 사용자 정의 데이터 수신 함수
int NetWorkManager::recvn(SOCKET s, char *buf, int len, int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (len - left);
}
