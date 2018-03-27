#pragma once
#include "stdafx.h"
#define MAX_PACKET_SIZE 512
#pragma pack(push, 1) 
struct stOverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped;
	WSABUF m_wsaBuf;
	unsigned char m_IOCPbuf[MAX_PACKET_SIZE]; // IOCP send/recv ���� 
	unsigned char m_packet_buf[MAX_PACKET_SIZE];// recv�Ǵ���Ŷ�������Ǵ¹��� 
	int m_nRemainLen; // �̿Ϸ����Ŷũ�⸦���� 
	bool m_eOperation; // Send���ΰ�Recv���ΰ�. 
	int receiving_packet_size, received; // ���� ���� ���� ��Ŷ�� ũ��� ���� �� ���� 
};

struct stClientChessInfo 
{ //�ΰ��ӿ����� Ŭ���̾�Ʈ ����
	int id;
	SOCKET m_socketClient;
	stOverlappedEx m_stRecvOverlappedEx;
	wchar_t name[20];
	BYTE x; 
	BYTE z; 
};
#pragma pack(pop)