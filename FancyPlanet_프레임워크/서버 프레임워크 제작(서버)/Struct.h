#pragma once
#include "stdafx.h"
#define MAX_PACKET_SIZE 512
#pragma pack(push, 1) 
struct stOverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped;
	WSABUF m_wsaBuf;
	unsigned char m_IOCPbuf[MAX_PACKET_SIZE]; // IOCP send/recv 버퍼 
	unsigned char m_packet_buf[MAX_PACKET_SIZE];// recv되는패킷이조립되는버퍼 
	int m_nRemainLen; // 미완료된패킷크기를저장 
	bool m_eOperation; // Send중인가Recv중인가. 
	int receiving_packet_size, received; // 지금 조립 중인 패킷의 크기와 받은 양 저장 
};

struct stClientChessInfo 
{ //인게임에서의 클라이언트 정보
	int id;
	SOCKET m_socketClient;
	stOverlappedEx m_stRecvOverlappedEx;
	wchar_t name[20];
	BYTE x; 
	BYTE z; 
};
#pragma pack(pop)