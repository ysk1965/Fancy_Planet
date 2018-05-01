#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include <WinSock2.h>
#include <windows.h>   // include important windows stuff

#pragma comment(lib, "ws2_32.lib")
#include "Timer.h"
#include "protocol.h"
#include <math.h>
#include <thread>
#include <vector>
#include <array>
#include <iostream>

using namespace DirectX;
using namespace std;

HANDLE gh_iocp;
float g_fTime = 0.0;
int g_time = 0;

int g_roomnumb[5] = { 0 };
int g_ridx = 0;
struct EXOVER {
	WSAOVERLAPPED m_over;
	char m_iobuf[MAX_BUFF_SIZE];
	WSABUF m_wsabuf;
	bool is_recv;
};

class Client {
public:
	SOCKET m_s;
	bool m_isconnected;
	XMFLOAT4X4 m_pos;
	EXOVER m_rxover;
	int m_packet_size;  // ���� �����ϰ� �ִ� ��Ŷ�� ũ��
	int	m_prev_packet_size; // ������ recv���� �ϼ����� �ʾƼ� ������ ���� ��Ŷ�� �պκ��� ũ��
	int m_roomnumb;
	bool m_isready;
	int m_scene_state;
	char m_packet[MAX_PACKET_SIZE];

	Client()
	{
		bool m_isready = false;
		m_isconnected = false;
		int m_roomnumb = 0;
		int m_scene_state = 0;
		m_pos = {  };
		m_pos._42 = 212.0f;
		ZeroMemory(&m_rxover.m_over, sizeof(WSAOVERLAPPED));
		m_rxover.m_wsabuf.buf = m_rxover.m_iobuf;
		m_rxover.m_wsabuf.len = sizeof(m_rxover.m_wsabuf.buf);
		m_rxover.is_recv = true;
		m_prev_packet_size = 0;
	}
};

array <Client, MAX_USER> g_clients;

void error_display(const char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"  ����" << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	while (true);
}

void ErrorDisplay(const char * location)
{
	error_display(location, WSAGetLastError());
}

void SendPacket(int id, void *ptr)
{
	char *packet = reinterpret_cast<char *>(ptr);
	EXOVER *s_over = new EXOVER;
	s_over->is_recv = false;
	memcpy(s_over->m_iobuf, packet, packet[0]);
	s_over->m_wsabuf.buf = s_over->m_iobuf;
	s_over->m_wsabuf.len = s_over->m_iobuf[0];
	ZeroMemory(&s_over->m_over, sizeof(WSAOVERLAPPED));
	int res = WSASend(g_clients[id].m_s, &s_over->m_wsabuf, 1, NULL, 0,
		&s_over->m_over, NULL);
	if (0 != res) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) error_display("Send Error! ", err_no);
	}
}
void DisconnectPlayer(int id)
{
	sc_packet_remove_player p;
	p.id = id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	for (int i = 0; i < MAX_USER; ++i) {
		if (false == g_clients[i].m_isconnected)continue;
		if (i == id)continue;
		SendPacket(i, &p);
	}
	closesocket(g_clients[id].m_s);
	g_clients[id].m_isconnected = false;
}

void initialize()
{
	gh_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0); // �ǹ̾��� �Ķ����, �������� �˾Ƽ� �����带 ������ش�.
	std::wcout.imbue(std::locale("korean"));

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
}

void StartRecv(int id)
{
	unsigned long r_flag = 0;
	ZeroMemory(&g_clients[id].m_rxover.m_over, sizeof(WSAOVERLAPPED));
	int ret = WSARecv(g_clients[id].m_s, &g_clients[id].m_rxover.m_wsabuf, 1,
		NULL, &r_flag, &g_clients[id].m_rxover.m_over, NULL);
	if (0 != ret) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no) error_display("Recv Error", err_no);
	}
}



void ProcessPacket(int id, char *packet)
{
	//XMFLOAT4X4 pos = g_clients[id].m_pos;

	switch (packet[1])
	{
	case CS_READY:
	{
		printf("Client[ %d ] Ready Packet Process\n", id);
		printf("%d", sizeof(packet));
		if (true == g_clients[id].m_isready)
			g_clients[id].m_isready = false;
		else
			g_clients[id].m_isready = true;
		int roomnumb = g_clients[id].m_roomnumb;
		sc_packet_ready ready_packet;
		ready_packet.id = id;
		ready_packet.size = sizeof(sc_packet_ready);
		ready_packet.type = SC_READY;
		ready_packet.state = g_clients[id].m_isready;

		for (int i = 0; i < MAX_USER; ++i)
			if (true == g_clients[i].m_isconnected)
					SendPacket(i, &ready_packet);

		int ready_player = 0; 
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)
			{
				if (g_clients[i].m_isready == true)
				{
					ready_player += 1;
					if (ready_player == 2)
					{
						sc_packet_scene_change start_packet;
						start_packet.type = SC_SCENE_CHANGE;
						start_packet.size = sizeof(sc_packet_ready);
						start_packet.scenestate = 1;
						start_packet.id = i;
						SendPacket(i, &start_packet);
						printf("���ӽ��� ��Ŷ ����. \n ");
					}
				}
			}
		}
		break;
	}
	case CS_POS:
	{
		cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(packet);
		int roomnumb = p->roomnumb;
		printf("Move Client's Room Number: %d\n", roomnumb);
		sc_packet_pos pos_packet;

		g_clients[id].m_pos = p->m_pos;
		pos_packet.id = id;
		pos_packet.size = sizeof(sc_packet_pos);
		pos_packet.type = SC_POS;
		pos_packet.m_pos = g_clients[id].m_pos;

		printf("Client [ %d ] Move To : %f %f %f\n", id, p->m_pos._41, p->m_pos._42, p->m_pos._43);
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)//�������̰�
				if (id != i)//�� ��Ŷ�� ���� Ŭ�� �ƴϸ�
					if (g_clients[i].m_roomnumb == roomnumb)// ��Ŷ ���� Ŭ��� ���� ���� Ŭ�󿡰Ը�
						SendPacket(i, &pos_packet);//��Ŷ ����
		}


	}
	break;


	default:
		printf("Unkown Packet Type from Client [%d]\n", id);
		return;
	}
}




void worker_thread()
{
	while (true)
	{
		unsigned long io_size;
		unsigned long long iocp_key; // 64 ��Ʈ integer , �츮�� 64��Ʈ�� �������ؼ� 64��Ʈ
		WSAOVERLAPPED *over;
		BOOL ret = GetQueuedCompletionStatus(gh_iocp, &io_size, &iocp_key, &over, INFINITE);
		int key = static_cast<int>(iocp_key);
		printf("WT::Network I/O with Client [ %d ]\n", key);
		if (FALSE == ret) {
			cout << "Error in GQCS\n";
			DisconnectPlayer(key);
			continue;
		}
		if (0 == io_size) {
			DisconnectPlayer(key);

			continue;
		}

		EXOVER *p_over = reinterpret_cast<EXOVER *>(over);
		if (true == p_over->is_recv) {
			//cout << "WT:Packet From Client [" << key << "]\n";
			int work_size = io_size;
			char *wptr = p_over->m_iobuf;
			while (0 < work_size) {
				int p_size;
				if (0 != g_clients[key].m_packet_size)
					p_size = g_clients[key].m_packet_size;
				else {
					p_size = wptr[0];
					g_clients[key].m_packet_size = p_size;
				}
				int need_size = p_size - g_clients[key].m_prev_packet_size;
				if (need_size <= work_size) {
					memcpy(g_clients[key].m_packet
						+ g_clients[key].m_prev_packet_size, wptr, need_size);
					ProcessPacket(key, g_clients[key].m_packet);
					g_clients[key].m_prev_packet_size = 0;
					g_clients[key].m_packet_size = 0;
					work_size -= need_size;
					wptr += need_size;
				}
				else {
					memcpy(g_clients[key].m_packet + g_clients[key].m_prev_packet_size, wptr, work_size);
					g_clients[key].m_prev_packet_size += work_size;
					work_size = -work_size;
					wptr += work_size;
				}
			}
			StartRecv(key);
		}
		else {  // Send ��ó��
			//cout << "WT:A packet was sent to Client[" << key << "]\n";
			delete p_over;
		}
	}
}

void accept_thread()	//���� ������ ���� Ŭ���̾�Ʈ�� IOCP�� �ѱ�� ����
{
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN bind_addr;
	ZeroMemory(&bind_addr, sizeof(SOCKADDR_IN));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(MY_SERVER_PORT);
	bind_addr.sin_addr.s_addr = INADDR_ANY;	// 0.0.0.0  �ƹ��뼭�� ���� ���� �� �ްڴ�.

	::bind(s, reinterpret_cast<sockaddr *>(&bind_addr), sizeof(bind_addr));
	listen(s, 1000);

	while (true)
	{
		SOCKADDR_IN c_addr;
		ZeroMemory(&c_addr, sizeof(SOCKADDR_IN));
		c_addr.sin_family = AF_INET;
		c_addr.sin_port = htons(MY_SERVER_PORT);
		c_addr.sin_addr.s_addr = INADDR_ANY;	// 0.0.0.0  �ƹ��뼭�� ���� ���� �� �ްڴ�.
		int addr_size = sizeof(sockaddr);

		SOCKET cs = WSAAccept(s, reinterpret_cast<sockaddr *>(&c_addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == cs) {
			ErrorDisplay("In Accept Thread:WSAAccept()");
			continue;
		}
		cout << "New Client Connected!\n";
		int id = -1;
		for (int i = 0; i < MAX_USER; ++i)//�� �ڸ��� ã�� ���̵� �ε����� �������ݴϴ�.
			if (false == g_clients[i].m_isconnected) {
				id = i;
				break;
			}
		if (-1 == id) {//��� �ڸ��� á���Ƿ� ���� �Ұ��Դϴ�.
			cout << "MAX USER Exceeded\n";
			continue;
		}
		cout << "ID of new Client is [" << id << "]";//������ ���̵� ����ϰ� �ʱ�ȭ���ݴϴ�.
		g_clients[id].m_s = cs;
		g_clients[id].m_packet_size = 0;
		g_clients[id].m_prev_packet_size = 0;
		g_clients[id].m_pos = {};

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(cs), gh_iocp, id, 0);
		g_clients[id].m_isconnected = true;// �� ���̵� �ο����� Ŭ���̾�Ʈ�� ���� ������ ������ �ƽ��ϴ�.

		int roomsearch = 0;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected) {
				roomsearch += 1;
			}
		}
		if (roomsearch % 4 == 1)
		{

			g_ridx++;
		}
		printf("This Client Room Number is [%d]", g_ridx);

		g_clients[id].m_roomnumb = g_ridx;
		StartRecv(id);// ���� ������ ������ �޽��ϴ�.

		sc_packet_put_player p; // Ŭ���̾�Ʈ(ĳ����)�� �����ߴٴ� ��Ŷ�� �ϳ� �����մϴ�.
		p.id = (unsigned short)id;//���̵�
		p.size = sizeof(sc_packet_put_player);//��Ŷ������
		p.type = SC_PUT_PLAYER;//��ŶŸ��
		p.m_pos = g_clients[id].m_pos;
		p.roomnumb = g_clients[id].m_roomnumb;


		for (int i = 0; i < MAX_USER; ++i)//��� Ŭ���̾�Ʈ�� �����̴ϴ�.
		{
			if (true == g_clients[i].m_isconnected)
				SendPacket(i, &p);//��� �ε����� �˻��� �������ִ� Ŭ���̾�Ʈ�鿡�� ��Ŷ�� �����մϴ�.
		}
		for (int i = 0; i < MAX_USER; ++i) {
			if (true == g_clients[i].m_isconnected)
			{//i �ε����� ������ Ŭ���̾�Ʈ�� ���ٸ� �Ѿ��
				if (i == id) continue;//������ �ڽ��� Ŭ���̾�Ʈ�� �����ִ� �ε����� �Ѿ�ϴ�.
				p.id = (unsigned short)i;//���� ���ǵ��� ������ Ŭ���̾�Ʈ ������ �ϳ��ϳ� ������
				p.m_pos = g_clients[i].m_pos;//��ǥ
				p.roomnumb = g_clients[i].m_roomnumb;
				SendPacket(id, &p);
			}
		}

	}
}
void timer_thread()
{
	while (1)
	{
		CTimeMgr::GetInstance()->SetTime();//Ÿ�� ����
		g_fTime += 1.0f * CTimeMgr::GetInstance()->GetTime();
		if (g_fTime >= 1.0f)
		{
			g_fTime = 0.0f;
			g_time += 1;

			sc_packet_time time_packet;

			time_packet.size = sizeof(sc_packet_time);
			time_packet.type = SC_TIME;
			time_packet.m_ftime = g_time;
			printf("GAME TIME : %f \n", time_packet.m_ftime);
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (true == g_clients[i].m_isconnected)//�������̰�
					SendPacket(i, &time_packet);//��Ŷ ����
			}

		}
	}
}
int main()
{
	vector <thread> w_threads;
	initialize();
	//CreateWorkerThreads();	// ������ ���α��� �� �ȿ��� ���־�� �Ѵ�. �������� �ؼ� ������ �ؾ� ��. �������� ����� ����
	// ���� ����� �ƴ�.
	for (int i = 0; i < 4; ++i) w_threads.push_back(thread{ worker_thread }); // 4�� ������ �����ھ� CPU ��
																			  //CreateAcceptThreads();
	thread a_thread{ accept_thread };

	CTimeMgr::GetInstance()->InitTime();
	//thread t_thread{ timer_thread };

	for (auto& th : w_threads) th.join();
	a_thread.join();
	//t_thread.join();
}