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
float g_fTime = 0.f;
float g_time = 0.f;
int g_gamestate = 0;
float g_gravity = 8.f;
std::chrono::system_clock::time_point start;
struct Room
{
	int roomnumb;
	int gamestate;
	int user;
};
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
	int m_packet_size;  // 지금 조립하고 있는 패킷의 크기
	int	m_prev_packet_size; // 지난번 recv에서 완성되지 않아서 저장해 놓은 패킷의 앞부분의 크기
	int m_roomnumb;
	int m_health;
	bool m_isready;
	int m_scene_state;
	char m_packet[MAX_PACKET_SIZE];
	int m_animstate;
	float m_gravity;
	Client()
	{
		m_gravity = 8.f;
		m_isready = false;
		m_isconnected = false;
		m_roomnumb = 0;
		m_scene_state = 0;
		m_health = 100;
		m_pos = {};

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
	std::wcout << L"  에러" << lpMsgBuf << std::endl;
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
void timer_thread()
{
	while (1)
	{
		Sleep(1);
		std::chrono::milliseconds ms;
		ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - start);
		if (g_gamestate == 1)
		{
			if (ms.count() > 1000)
			{
				g_time += 1.f;
				sc_packet_time time_packet;
				time_packet.size = sizeof(sc_packet_time);
				time_packet.type = SC_TIME;
				time_packet.m_ftime = g_time;

				printf("GAME TIME : %f \n", time_packet.m_ftime);
				for (int i = 0; i < MAX_USER; ++i)
				{
					if (true == g_clients[i].m_isconnected)//접속중이고
						SendPacket(i, &time_packet);//패킷 전송
				}
				if (g_time == 30.f)
				{
					sc_gravity_change gravity_packet;

					gravity_packet.size = sizeof(sc_gravity_change);
					gravity_packet.type = SC_GRAVITY_CHANGE;
					gravity_packet.gravity_state = rand() % 3 + 1;
					printf("중력 변경! %d\n", gravity_packet.gravity_state);
					for (int i = 0; i < MAX_USER; ++i)
					{
						if (true == g_clients[i].m_isconnected)//접속중이고
							SendPacket(i, &gravity_packet);//패킷 전송
					}
				}
				start = chrono::system_clock::now();
			}
			if (g_time == 60.f)
			{
				g_time = 0;
				g_gamestate = 0;
				sc_packet_scene_change scene_packet;
				scene_packet.size = sizeof(sc_packet_scene_change);
				scene_packet.type = SC_SCENE_CHANGE;
				scene_packet.scenestate = 0;
				for (int i = 0; i < MAX_USER; ++i)
				{
					if (true == g_clients[i].m_isconnected)//접속중이고
						SendPacket(i, &scene_packet);//패킷 전송
				}
			}
		}
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
	gh_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0); // 의미없는 파라메터, 마지막은 알아서 쓰레드를 만들어준다.
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
		ready_packet.size = sizeof(ready_packet);
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
				}
			}
		}
		if (ready_player == 2)
		{
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (g_clients[i].m_isready == true)
				{
					sc_packet_scene_change start_packet;
					start_packet.type = SC_SCENE_CHANGE;
					start_packet.size = sizeof(start_packet);
					start_packet.scenestate = 1;
					start_packet.id = i;
					SendPacket(i, &start_packet);
					printf("게임시작 패킷 전송. \n ");
					g_gamestate = 1;
				}
			}
		}
		break;
	}
	case CS_POS:
	{
		cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(packet);
		//int roomnumb = p->roomnumb;
		//printf("Move Client's Room Number: %d\n", roomnumb);
		sc_packet_pos pos_packet;

		g_clients[id].m_pos._11 = p->m_pos._11;
		g_clients[id].m_pos._12 = p->m_pos._12;
		g_clients[id].m_pos._13 = p->m_pos._13;
		g_clients[id].m_pos._21 = p->m_pos._21;
		g_clients[id].m_pos._22 = p->m_pos._22;
		g_clients[id].m_pos._23 = p->m_pos._23;
		g_clients[id].m_pos._31 = p->m_pos._31;
		g_clients[id].m_pos._32 = p->m_pos._32;
		g_clients[id].m_pos._33 = p->m_pos._33;
		g_clients[id].m_pos._41 = p->m_pos._41;
		g_clients[id].m_pos._42 = p->m_pos._42;
		g_clients[id].m_pos._43 = p->m_pos._43;

		g_clients[id].m_animstate = p->animstate;
		pos_packet.id = id;

		pos_packet.type = SC_POS;
		pos_packet.m_pos._11 = g_clients[id].m_pos._11;
		pos_packet.m_pos._12 = g_clients[id].m_pos._12;
		pos_packet.m_pos._13 = g_clients[id].m_pos._13;
		pos_packet.m_pos._21 = g_clients[id].m_pos._21;
		pos_packet.m_pos._22 = g_clients[id].m_pos._22;
		pos_packet.m_pos._23 = g_clients[id].m_pos._23;
		pos_packet.m_pos._31 = g_clients[id].m_pos._31;
		pos_packet.m_pos._32 = g_clients[id].m_pos._32;
		pos_packet.m_pos._33 = g_clients[id].m_pos._33;
		pos_packet.m_pos._41 = g_clients[id].m_pos._41;
		pos_packet.m_pos._42 = g_clients[id].m_pos._42;
		pos_packet.m_pos._43 = g_clients[id].m_pos._43;
		pos_packet.animstate = g_clients[id].m_animstate;
		pos_packet.size = sizeof(sc_packet_pos);

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)//접속중이고
				if (i != id)//이 패킷을 보낸 클라가 아니며

					SendPacket(i, &pos_packet);//패킷 전송
		}
		break;
	}
	case CS_SHOT:
	{
		cs_packet_shot *p = reinterpret_cast<cs_packet_shot *>(packet);

		sc_packet_shot shot_packet;

		g_clients[id].m_animstate = p->animstate;

		shot_packet.type = SC_SHOT;
		shot_packet.animstate = g_clients[id].m_animstate;
		shot_packet.getprojectile = p->getprojectile;
		shot_packet.id = p->id;
		shot_packet.size = sizeof(sc_packet_shot);
		//printf("Client [ %d ] Move To : %f %f %f\n", id, p->m_pos._41, p->m_pos._42, p->m_pos._43);
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)//접속중이고
			{
				if (i != id)
				{
					SendPacket(i, &shot_packet);//패킷 전송
					printf("%d 클라이언트가 공격했다!\n", id);
				}
			}
		}
		break;
	}
	case CS_GRAVITY_TEST:
	{
		cs_gravity_test *p = reinterpret_cast<cs_gravity_test *>(packet);

		sc_gravity_change gravity_packet;

		g_gravity = 2.f - (float)(rand() % 10);
		 gravity_packet.type = SC_GRAVITY_CHANGE;
		 gravity_packet.gravity_state = g_gravity;
		 gravity_packet.size = sizeof(sc_gravity_change);
		//printf("Client [ %d ] Move To : %f %f %f\n", id, p->m_pos._41, p->m_pos._42, p->m_pos._43);
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)//접속중이고
			{
				SendPacket(i, &gravity_packet);//패킷 전송
				printf("중력 수치 변경 : %f\n", gravity_packet.gravity_state);
			}
		}
		break;
	}
	case CS_PLAYER_STATE_CHANGE:
	{
		cs_packet_character_state *p = reinterpret_cast<cs_packet_character_state *>(packet);

		sc_packet_character_state state_packet;

		g_gravity = 2.f - (float)(rand() % 10);
		state_packet.type = SC_PLAYER_STATE_CHANGE;
		state_packet.state = p->state;
		state_packet.id = p->id;
		state_packet.size = sizeof(sc_packet_character_state);
		//printf("Client [ %d ] Move To : %f %f %f\n", id, p->m_pos._41, p->m_pos._42, p->m_pos._43);
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)//접속중이고
			{
				SendPacket(i, &state_packet);//패킷 전송
			}
		}
		break;
	}
	case CS_ATTACKED:
	{
		cs_packet_attacked *p = reinterpret_cast<cs_packet_attacked*>(packet);

		sc_packet_attacked attacked_packet;

		g_gravity = 2.f - (float)(rand() % 10);
		attacked_packet.type = SC_ATTACKED;
		g_clients[id].m_health -= 50;
		attacked_packet.id = id;
		attacked_packet.hp = g_clients[id].m_health;
		attacked_packet.size = sizeof(sc_packet_attacked);
		//printf("Client [ %d ] Move To : %f %f %f\n", id, p->m_pos._41, p->m_pos._42, p->m_pos._43);
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)//접속중이고
			{
				SendPacket(i, &attacked_packet);//패킷 전송
			}
		}
		break;
	}
	default:
	{

		printf("Unkown Packet Type from Client [%d]\n", id);
		return;
	}

	}
}


void worker_thread()
{
	while (true)
	{
		Sleep(1);
		unsigned long io_size;
		unsigned long long iocp_key; // 64 비트 integer , 우리가 64비트로 컴파일해서 64비트
		WSAOVERLAPPED *over;
		BOOL ret = GetQueuedCompletionStatus(gh_iocp, &io_size, &iocp_key, &over, INFINITE);
		int key = static_cast<int>(iocp_key);
		//printf("WT::Network I/O with Client [ %d ]\n", key);
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
		else {  // Send 후처리
				//cout << "WT:A packet was sent to Client[" << key << "]\n";
			delete p_over;
		}
	}
}

void accept_thread()	//새로 접속해 오는 클라이언트를 IOCP로 넘기는 역할
{
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN bind_addr;
	ZeroMemory(&bind_addr, sizeof(SOCKADDR_IN));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(MY_SERVER_PORT);
	bind_addr.sin_addr.s_addr = INADDR_ANY;	// 0.0.0.0  아무대서나 오는 것을 다 받겠다.

	::bind(s, reinterpret_cast<sockaddr *>(&bind_addr), sizeof(bind_addr));
	listen(s, 1000);

	while (true)
	{
		
		SOCKADDR_IN c_addr;
		ZeroMemory(&c_addr, sizeof(SOCKADDR_IN));
		c_addr.sin_family = AF_INET;
		c_addr.sin_port = htons(MY_SERVER_PORT);
		c_addr.sin_addr.s_addr = INADDR_ANY;	// 0.0.0.0  아무대서나 오는 것을 다 받겠다.
		int addr_size = sizeof(sockaddr);

		SOCKET cs = WSAAccept(s, reinterpret_cast<sockaddr *>(&c_addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == cs) {
			ErrorDisplay("In Accept Thread:WSAAccept()");
			continue;
		}
		cout << "New Client Connected!\n";
		int id = -1;
		for (int i = 0; i < MAX_USER; ++i)//빈 자리를 찾아 아이디를 인덱스로 설정해줍니다.
			if (false == g_clients[i].m_isconnected) {
				id = i;
				break;
			}
		if (-1 == id) {//모든 자리가 찼으므로 접속 불가입니다.
			cout << "MAX USER Exceeded\n";
			continue;
		}
		cout << "ID of new Client is [" << id << "]";//설정한 아이디를 출력하고 초기화해줍니다.
		g_clients[id].m_s = cs;
		g_clients[id].m_packet_size = 0;
		g_clients[id].m_prev_packet_size = 0;
		g_clients[id].m_pos = {};
		g_clients[id].m_animstate = 0;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(cs), gh_iocp, id, 0);
		g_clients[id].m_isconnected = true;// 이 아이디를 부여받은 클라이언트는 이제 서버와 연결이 됐습니다.

		//printf("This Client Room Number is [%d]", g_ridx);

		g_clients[id].m_roomnumb = 0;
		StartRecv(id);// 위의 정보를 서버에 받습니다.

		sc_packet_put_player p; // 클라이언트(캐릭터)가 접속했다는 패킷을 하나 설정합니다.
		p.id = (unsigned short)id;//아이디
		p.size = sizeof(sc_packet_put_player);//패킷사이즈
		p.type = SC_PUT_PLAYER;//패킷타입
		g_clients[id].m_pos._11 = 10.0f;
		g_clients[id].m_pos._22 = 10.0f;
		g_clients[id].m_pos._33 = 10.0f;
		g_clients[id].m_pos._44 = 1.0f;
		g_clients[id].m_pos._42 = 212.0f;
		g_clients[id].m_gravity = g_gravity;
		p.m_pos._11 = g_clients[id].m_pos._11;
		p.m_pos._12 = g_clients[id].m_pos._12;
		p.m_pos._13 = g_clients[id].m_pos._13;
		p.m_pos._21 = g_clients[id].m_pos._21;
		p.m_pos._22 = g_clients[id].m_pos._22;
		p.m_pos._23 = g_clients[id].m_pos._23;
		p.m_pos._31 = g_clients[id].m_pos._31;
		p.m_pos._32 = g_clients[id].m_pos._32;
		p.m_pos._33 = g_clients[id].m_pos._33;
		p.m_pos._41 = g_clients[id].m_pos._41;
		p.m_pos._42 = g_clients[id].m_pos._42;
		p.m_pos._43 = g_clients[id].m_pos._43;

		p.roomnumb = g_clients[id].m_roomnumb;


		for (int i = 0; i < MAX_USER; ++i)//모든 클라이언트를 훑을겁니다.
		{
			if (true == g_clients[i].m_isconnected)
				SendPacket(i, &p);//모든 인덱스를 검색해 접속해있는 클라이언트들에게 패킷을 전달합니다.
		}
		for (int i = 0; i < MAX_USER; ++i) {
			if (true == g_clients[i].m_isconnected)
			{//i 인덱스에 접속한 클라이언트가 없다면 넘어가고
				if (i == id) continue;//접속한 자신의 클라이언트가 속해있는 인덱스도 넘어갑니다.
				p.id = (unsigned short)i;//위의 조건들을 제외한 클라이언트 정보를 하나하나 훑으며
				p.m_pos._11 = g_clients[i].m_pos._11;
				p.m_pos._12 = g_clients[i].m_pos._12;
				p.m_pos._13 = g_clients[i].m_pos._13;
				p.m_pos._21 = g_clients[i].m_pos._21;
				p.m_pos._22 = g_clients[i].m_pos._22;
				p.m_pos._23 = g_clients[i].m_pos._23;
				p.m_pos._31 = g_clients[i].m_pos._31;
				p.m_pos._32 = g_clients[i].m_pos._32;
				p.m_pos._33 = g_clients[i].m_pos._33;
				p.m_pos._41 = g_clients[id].m_pos._41;
				p.m_pos._42 = g_clients[id].m_pos._42;
				p.m_pos._43 = g_clients[id].m_pos._43;
				p.roomnumb = g_clients[i].m_roomnumb;
				SendPacket(id, &p);
			}
		}

	}
}

int main()
{
	vector <thread> w_threads;
	initialize();
	//CreateWorkerThreads();	// 쓰레드 조인까지 이 안에서 해주어야 한다. 전역변수 해서 관리를 해야 함. 전역변수 만드는 것은
	// 좋은 방법이 아님.
	for (int i = 0; i < 4; ++i) w_threads.push_back(thread{ worker_thread }); // 4인 이유는 쿼드코어 CPU 라서
																			  //CreateAcceptThreads();
	thread a_thread{ accept_thread };
	CTimeMgr::GetInstance()->InitTime();

	thread t_thread{ timer_thread };

	for (auto& th : w_threads) th.join();
	a_thread.join();
	t_thread.join();
}