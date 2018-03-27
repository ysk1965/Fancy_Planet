#include "stdafx.h"
#include <iostream>

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE    512
#define GRID_SIZE 100
#define WINDOW_SIZE 800
// 소켓 함수 오류 출력 후 종료

DWORD WINAPI Worker_Thread(LPVOID pCompletionPort);

void err_quit(const char *msg)
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
void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

int main(int argc, char *argv[])
{
	int retval;
	WSADATA			m_wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
		err_quit("WSAStartup() error!");
	printf("네트워크매니저 클래스 객체 생성");

	HANDLE m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	HANDLE hResult = NULL;

	for (int i = 0; i < 4; ++i)
	{
		DWORD ThreadId;
		hResult = CreateThread(NULL, 0, Worker_Thread, (LPVOID)m_hCompletionPort, 0, (LPDWORD)&ThreadId);
		if (hResult == NULL)
		{
			printf("워커스레드 생성 실패\n");

		}
	}

	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

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

	stClientChessInfo* ClientChessInfo = new stClientChessInfo;

	while (1)
	{	// accept()

		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		ClientChessInfo->m_socketClient = client_sock;

		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
		}

		HANDLE hResult = CreateIoCompletionPort((HANDLE)client_sock,
			m_hCompletionPort, (DWORD)client_sock, 0);

		if (ClientChessInfo == NULL)
			err_quit("클라이언트 구조체 생성 실패");

		ZeroMemory(&(ClientChessInfo->m_stRecvOverlappedEx), sizeof(ClientChessInfo->m_stRecvOverlappedEx));
		ClientChessInfo->m_socketClient = client_sock;
		ClientChessInfo->m_stRecvOverlappedEx.m_wsaBuf = { 0 };
		DWORD recvbytes;
		DWORD flags = 0;

		retval = WSARecv(ClientChessInfo->m_socketClient,
			&(ClientChessInfo->m_stRecvOverlappedEx.m_wsaBuf),
			1,
			&recvbytes,
			&flags,
			&(ClientChessInfo->m_stRecvOverlappedEx.m_wsaOverlapped), NULL);

		if (retval == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				err_display("WSARecv()");
			}
		}

		
			
			
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));


	}

	// closesocket()
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();

	return 0;
}

DWORD WINAPI Worker_Thread(LPVOID pCompletionPort)
{

	HANDLE m_hCompletionPort = pCompletionPort;
	int retval;
	DWORD dwIOSize;
	stClientChessInfo* ChessClientInfo = new stClientChessInfo;//구조체 클래스 생성

	while (1)
	{
		retval = GetQueuedCompletionStatus(m_hCompletionPort,
			&dwIOSize,
			(PULONG_PTR)m_hCompletionPort,
			(LPOVERLAPPED*)ChessClientInfo,
			INFINITE);

		if (dwIOSize == 0)
		{
			closesocket(ChessClientInfo->m_socketClient);
			//delete ChessClientInfo;
		}

		switch (ChessClientInfo->m_stRecvOverlappedEx.m_eOperation)
		{
		case true:
		{
			WSASend(ChessClientInfo->m_socketClient, &(ChessClientInfo->m_stRecvOverlappedEx.m_wsaBuf),
				1, NULL, 0, &ChessClientInfo->m_stRecvOverlappedEx.m_wsaOverlapped, NULL);
			delete ChessClientInfo;
		}

		case false:
		{
			WSARecv(ChessClientInfo->m_socketClient, &(ChessClientInfo->m_stRecvOverlappedEx.m_wsaBuf),
				1, NULL, 0, &ChessClientInfo->m_stRecvOverlappedEx.m_wsaOverlapped, NULL);
		}


		}
	}
}