#define WIN32_LEAN_AND_MEAN  
#define INITGUID

#include "stdafx.h"
#include "Timer.h"
#include "protocol.h"

using namespace chrono;
array <Client, MAX_USER> g_clients;
XMFLOAT3 g_xmf3ObjectsPos[OBJECTS_NUMBER];
XMFLOAT4 g_xmf3ObjectsQuaternion[OBJECTS_NUMBER];
CPhysXFramework gPxFramework;
BASE base[STATION_NUMBER];
TEAM Team[Team_NUMBER];
ROOM Room[3];

using namespace DirectX;
using namespace std;

HANDLE gh_iocp;
float g_fTime = 0.f;
float g_time = 0.f;
int g_gamestate = 0;
float g_gravity = 8.f;
std::chrono::system_clock::time_point timer_start;



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
	s_over->m_event = EVT_SEND;
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


	timer_start = chrono::system_clock::now();
	while (1)
	{
		if (Room[0].m_iState == INGAME)
		{

			Sleep(1);

			for (int i = 0; i < Team_NUMBER; ++i)
			{
				if (Team[i].m_iNumberOfterritories == STATION_NUMBER)//한 팀이 모든 점령지를 점령했다면
				{
					Team[i].m_isWin = true;//바로 승패를 가르고 게임을 끝냅니다.
				}

			}


			std::chrono::milliseconds ms;
			ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - timer_start);
			if (ms.count() >= 1000)
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
						if (g_clients[i].m_iRoomNumb == 0)
							SendPacket(i, &time_packet);//패킷 전송
				}
				if (g_time == 30.f) // 바꿈
				{
					sc_gravity_change gravity_packet;


					gravity_packet.size = sizeof(sc_gravity_change);
					gravity_packet.type = SC_GRAVITY_CHANGE;

					gravity_packet.gravity_state = (rand() % 2) - 12;
					gPxFramework.SetGravity(float(gravity_packet.gravity_state));
					gPxFramework.GravityChangeState = true;
					printf("중력 변경! %f\n", gravity_packet.gravity_state);
					for (int i = 0; i < MAX_USER; ++i)
					{
						if (true == g_clients[i].m_isconnected)//접속중이고
							if (g_clients[i].m_iRoomNumb == 0)
								SendPacket(i, &gravity_packet);//패킷 전송
					}
				}
				if (g_time == 90.f) // 바꿈
				{
					sc_gravity_change gravity_packet;


					gravity_packet.size = sizeof(sc_gravity_change);
					gravity_packet.type = SC_GRAVITY_CHANGE;

					gravity_packet.gravity_state = (rand() % 2) - 12;
					gPxFramework.SetGravity(float(gravity_packet.gravity_state));
					gPxFramework.GravityChangeState = true;
					printf("중력 변경! %f\n", gravity_packet.gravity_state);
					for (int i = 0; i < MAX_USER; ++i)
					{
						if (true == g_clients[i].m_isconnected)//접속중이고
							if (g_clients[i].m_iRoomNumb == 0)
								SendPacket(i, &gravity_packet);//패킷 전송
					}
				}
				if (g_time == 135.f) // 바꿈
				{
					sc_gravity_change gravity_packet;


					gravity_packet.size = sizeof(sc_gravity_change);
					gravity_packet.type = SC_GRAVITY_CHANGE;

					gravity_packet.gravity_state = (rand() % 2) - 12;
					gPxFramework.SetGravity(float(gravity_packet.gravity_state));
					gPxFramework.GravityChangeState = true;
					printf("중력 변경! %f\n", gravity_packet.gravity_state);
					for (int i = 0; i < MAX_USER; ++i)
					{
						if (true == g_clients[i].m_isconnected)//접속중이고
							if (g_clients[i].m_iRoomNumb == 0)
								SendPacket(i, &gravity_packet);//패킷 전송
					}
				}

				if (g_time == GAMETIME)
				{

					if (Team[BLUETeam].m_iNumberOfterritories == Team[BLUETeam].m_iNumberOfterritories == Team[GREENTeam].m_iNumberOfterritories)
					{
						if (Team[BLUETeam].m_iKill > Team[REDTeam].m_iKill)
						{
							if (Team[BLUETeam].m_iKill > Team[GREENTeam].m_iKill)
								Team[BLUETeam].m_isWin = true;
							else
								Team[GREENTeam].m_isWin = true;
						}
						else
						{
							if (Team[REDTeam].m_iKill > Team[GREENTeam].m_iKill)
								Team[REDTeam].m_isWin = true;
							else
								Team[GREENTeam].m_isWin = true;
						}
					}
					else
					{
						if (Team[BLUETeam].m_iNumberOfterritories > Team[REDTeam].m_iNumberOfterritories)
						{
							if (Team[BLUETeam].m_iNumberOfterritories > Team[GREENTeam].m_iNumberOfterritories)
								Team[BLUETeam].m_isWin = true;
							else
								Team[GREENTeam].m_isWin = true;
						}
						else
						{
							if (Team[REDTeam].m_iNumberOfterritories > Team[GREENTeam].m_iNumberOfterritories)
								Team[REDTeam].m_isWin = true;
							else
								Team[GREENTeam].m_isWin = true;
						}
					}
					int winner = 0;
					for (int i = 1; i < Team_NUMBER; ++i)
					{

						if (Team[i].m_iNumberOfterritories > Team[winner].m_iNumberOfterritories)
						{
							winner = i;
						}

					}
					Team[winner].m_isWin = true;


					g_time = 0;

				}
				timer_start = chrono::system_clock::now();


			}


			for (int i = 0; i < Team_NUMBER; ++i)
			{
				if (Team[i].m_isWin)
				{
					for (int j = 0; j < MAX_USER; ++j)
					{
						if (g_clients[j].m_isconnected&&g_clients[j].m_iRoomNumb == 0)
						{
							EXOVER *ex = new EXOVER;

							if (i == BLUETeam)
								ex->m_event = EVT_BLUETeam_WIN;
							else if (i == REDTeam)
								ex->m_event = EVT_REDTeam_WIN;
							else
								ex->m_event = EVT_GREENTeam_WIN;

							PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);

							for (int k = 0; k < STATION_NUMBER; ++k)
								base[k].Initialize();
							for (int k = 0; k < Team_NUMBER; ++k)//점령지와 팀스코어를 초기상태로.
								Team[k].Initialize();

						}
					}
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
	Room[g_clients[id].m_iRoomNumb].m_iUser -= 1;
	if (g_clients[id].m_isReady == true)
		Room[g_clients[id].m_iRoomNumb].m_iReadyUser -= 1;
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
	cs_packet_pos *p = reinterpret_cast<cs_packet_pos*>(packet);
	switch (p->type)
	{

	case CS_REQUEST_OBJECT:
	{
		sc_packet_loadObject loadObjectPacket;  // 클라이언트(캐릭터)가 접속했다는 패킷을 하나 설정합니다.

		loadObjectPacket.size = sizeof(sc_packet_loadObject);//패킷사이즈
		loadObjectPacket.type = SC_LOAD_OBJECT;//패킷타입
		for (int i = 0; i < OBJECTS_NUMBER; ++i)
		{
			loadObjectPacket.m_xmf3ObjectPosition = g_xmf3ObjectsPos[i];
			loadObjectPacket.m_xmf4ObjectQuaternion = g_xmf3ObjectsQuaternion[i];
			loadObjectPacket.m_ObjectID = (unsigned char)i;

			Sleep(1);
			SendPacket(id, &loadObjectPacket);
		}
		break;
	}

	case CS_READY:
	{
		int roomnumb = g_clients[id].m_iRoomNumb;

		if (true == g_clients[id].m_isReady)
		{
			g_clients[id].m_isReady = false;
			Room[roomnumb].m_iReadyUser -= 1;
			printf("[%d]방의 [%d]플레이어 레디 해제.\n", roomnumb, id);
		}
		else
		{
			g_clients[id].m_isReady = true;
			Room[roomnumb].m_iReadyUser += 1;
			printf("[%d]방의 [%d]플레이어 레디.\n", roomnumb, id);
		}


		sc_packet_ready ready_packet;
		ready_packet.id = id;
		ready_packet.size = sizeof(ready_packet);
		ready_packet.type = SC_READY;
		ready_packet.isReadyState = g_clients[id].m_isReady;

		for (int i = 0; i < MAX_USER; ++i)
			if (true == g_clients[i].m_isconnected)
				if (roomnumb == g_clients[i].m_iRoomNumb)
					SendPacket(i, &ready_packet);

		if (Room[roomnumb].m_iUser >= 1)//유저수는 두 명 이상. 일단 한명은 테스트용
		{
			if (Room[roomnumb].m_iUser == Room[roomnumb].m_iReadyUser)//방의 총 유저수와 레디한 유저 수가 같다면.
			{
				EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
				ex->m_event = EVT_LOADING_START;
				PostQueuedCompletionStatus(gh_iocp, 1, id, &ex->m_over);
			}
		}
		break;

	}
	case CS_POS:
	{
		cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(packet);

		CPlayer * player = gPxFramework.GetPlayersInfo();

		switch (p->keyInputState)//피직스 이동연산 처리. XMFLOAT4x4하나를 리턴해야함.
		{
		case FORWARD:
		{
			g_clients[p->id].m_animstate = FORWARD;
			player[p->id].SetMoveState(FORWARD);
			//printf("[%d] 클라이언트의 전진이동 패킷\n ", p->id);
			break;
		}
		case BACKWARD:
		{
			g_clients[p->id].m_animstate = BACKWARD;
			player[p->id].SetMoveState(BACKWARD);
			//printf("[%d] 클라이언트의 후진이동 패킷\n ", p->id);
			break;
		}
		case LEFT:
		{
			g_clients[p->id].m_animstate = LEFT;
			player[p->id].SetMoveState(LEFT);
			//printf("[%d] 클라이언트의 좌측이동 패킷\n ", p->id);
			break;
		}
		case RIGHT:
		{
			g_clients[p->id].m_animstate = RIGHT;
			player[p->id].SetMoveState(RIGHT);
			//printf("[%d] 클라이언트의 우측이동 패킷\n ", p->id);
			break;
		}
		case NONE:
		{
			g_clients[p->id].m_animstate = NONE;
			player[p->id].SetMoveState(NONE);
			//printf("[%d] 클라이언트 IDLE 돌입\n", p->id);
		}
		default:
			break;

			//연산
		}

		break;
	}
	case CS_ATTACK_SKILL:
	{
		cs_packet_attack_skill *p = reinterpret_cast<cs_packet_attack_skill *>(packet);

		CPlayer* player = gPxFramework.GetPlayersInfo();

		player[p->id].SetAttackSkillState(true);

		break;
	}
	case CS_ROTATE:
	{
		cs_packet_rotate *p = reinterpret_cast<cs_packet_rotate *>(packet);

		CPlayer *player = gPxFramework.GetPlayersInfo();

		player[p->id].SetLookVector(p->xmvLook);
		player[p->id].SetRightVector(p->xmvRight);
		player[p->id].SetUpVector(p->xmvUP);

		break;
	}
	case CS_JUMP:
	{
		cs_packet_jump *p = reinterpret_cast<cs_packet_jump *>(packet);
		CPlayer *player = gPxFramework.GetPlayersInfo();

		player[p->id].Jump();

		sc_packet_pos pos_packet;
		pos_packet.type = SC_JUMP;
		pos_packet.size = sizeof(sc_packet_pos);
		pos_packet.id = p->id;
		pos_packet.keyInputState = JUMPING;
		pos_packet.m_xmf3Position.x = g_clients[p->id].m_pos._41;
		pos_packet.m_xmf3Position.y = g_clients[p->id].m_pos._42;
		pos_packet.m_xmf3Position.z = g_clients[p->id].m_pos._43;

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)//접속중이고
				SendPacket(i, &pos_packet);//패킷 전송

		}
		break;
	}
	case CS_SHOT:
	{
		cs_packet_shot *p = reinterpret_cast<cs_packet_shot *>(packet);

		CPlayer *player = gPxFramework.GetPlayersInfo();

		player[p->id].SetAttackState(true, p->m_xmf3CameraLookAt);

		sc_packet_shot shot_packet;


		shot_packet.type = SC_SHOT;
		shot_packet.id = p->id;
		shot_packet.size = sizeof(sc_packet_shot);
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected&&g_clients[i].m_iRoomNumb == g_clients[p->id].m_iRoomNumb)//접속중이고
			{

				SendPacket(i, &shot_packet);//패킷 전송
											//printf("%d 클라이언트가 공격했다!\n", id);

			}
		}
		break;
	}
	case CS_MOVE_SKILL:
	{
		cs_packet_move_skill *p = reinterpret_cast<cs_packet_move_skill*>(packet);
		CPlayer *player = gPxFramework.GetPlayersInfo();
		if (p->charactertype == DRONE)
		{
			player[p->id].UsingTeleport();
		}
		else if (p->charactertype == SOLDIER)
			player[p->id].UsingDoubleJump();
		else if (p->charactertype == CREATURE)
		{
			player[p->id].UsingRapidMove();
			player[p->id].m_MoveSkillCoolTimer = chrono::system_clock::now();
		}
		sc_packet_move_skill skill_packet;
		skill_packet.id = p->id;
		skill_packet.type = SC_MOVE_SKILL_ON;
		skill_packet.size = sizeof(sc_packet_move_skill);
		skill_packet.charactertype = p->charactertype;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == g_clients[i].m_isconnected)//접속중이고
			{
				if (g_clients[p->id].m_iRoomNumb == g_clients[i].m_iRoomNumb&&  g_clients[i].m_scene_state == INGAME)
				{
					SendPacket(i, &skill_packet);//패킷 전송
					printf("[%d]가 이동스킬(SHIFT) 사용\n", p->id);
				}
			}
		}
		break;
	}
	case CS_LOADING_COMPLETE:
	{
		cs_loading_complete *p = reinterpret_cast<cs_loading_complete *>(packet);
		Room[g_clients[p->id].m_iRoomNumb].m_iLoadingCompleteUser += 1;

		if (Room[g_clients[p->id].m_iRoomNumb].m_iLoadingCompleteUser == Room[g_clients[p->id].m_iRoomNumb].m_iUser)
		{
			EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
			ex->m_event = EVT_GAME_START;
			PostQueuedCompletionStatus(gh_iocp, 1, p->id, &ex->m_over);
		}
		break;
	}
	case CS_CHARACTER_CHANGE:
	{
		cs_packet_characterChange *p = reinterpret_cast<cs_packet_characterChange*>(packet);
		CPlayer *player = gPxFramework.GetPlayersInfo();
		if (g_clients[p->id].m_iCharacterType != p->m_iCharacterType  && g_clients[p->id].m_scene_state == INGAMEROOM)
		{
			g_clients[p->id].m_iCharacterType = p->m_iCharacterType;
			player[p->id].SetCharacterType(p->m_iCharacterType);
			g_clients[p->id].m_iTeam = p->m_iCharacterType;
			player[p->id].SetPlayerTeam(p->m_iCharacterType);
			sc_packet_characterChange CharacterChangePacket;

			CharacterChangePacket.size = sizeof(sc_packet_characterChange);
			CharacterChangePacket.type = SC_CHARACTER_CHANGE;
			CharacterChangePacket.id = p->id;
			CharacterChangePacket.m_iCharacterType = g_clients[p->id].m_iCharacterType;
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (g_clients[p->id].m_iRoomNumb == g_clients[i].m_iRoomNumb)
					if (true == g_clients[i].m_isconnected)//접속중이고
						SendPacket(i, &CharacterChangePacket);//패킷 전송

			}
		}
		break;
	}
	case CS_TEAM_CHANGE:
	{

		cs_packet_teamChange *p = reinterpret_cast<cs_packet_teamChange *>(packet);
		CPlayer *player = gPxFramework.GetPlayersInfo();
		if (g_clients[p->id].m_iTeam == BLUETeam && g_clients[p->id].m_scene_state == INGAMEROOM)
		{
			g_clients[p->id].m_iTeam = REDTeam;
			player[p->id].SetPlayerTeam(REDTeam);
			sc_packet_teamChange TeamChangePacket;
			TeamChangePacket.size = sizeof(sc_packet_teamChange);
			TeamChangePacket.type = SC_TEAM_CHANGE;
			TeamChangePacket.id = p->id;
			TeamChangePacket.m_iTeamToChange = REDTeam;
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (g_clients[p->id].m_iRoomNumb == g_clients[i].m_iRoomNumb)
					if (true == g_clients[i].m_isconnected)//접속중이고
						SendPacket(i, &TeamChangePacket);//패킷 전송

			}

		}
		else if (g_clients[p->id].m_iTeam == REDTeam && g_clients[p->id].m_scene_state == INGAMEROOM)
		{
			g_clients[p->id].m_iTeam = GREENTeam;
			player[p->id].SetPlayerTeam(GREENTeam);

			sc_packet_teamChange TeamChangePacket;
			TeamChangePacket.size = sizeof(sc_packet_teamChange);
			TeamChangePacket.type = SC_TEAM_CHANGE;
			TeamChangePacket.id = p->id;
			TeamChangePacket.m_iTeamToChange = GREENTeam;
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (g_clients[p->id].m_iRoomNumb == g_clients[i].m_iRoomNumb)
					if (true == g_clients[i].m_isconnected)//접속중이고
						SendPacket(i, &TeamChangePacket);//패킷 전송
			}
		}
		else
		{
			g_clients[p->id].m_iTeam = BLUETeam;
			player[p->id].SetPlayerTeam(BLUETeam);

			sc_packet_teamChange TeamChangePacket;
			TeamChangePacket.size = sizeof(sc_packet_teamChange);
			TeamChangePacket.type = SC_TEAM_CHANGE;
			TeamChangePacket.id = p->id;
			TeamChangePacket.m_iTeamToChange = BLUETeam;
			for (int i = 0; i < MAX_USER; ++i)
			{
				if (g_clients[p->id].m_iRoomNumb == g_clients[i].m_iRoomNumb)
					if (true == g_clients[i].m_isconnected)//접속중이고
						SendPacket(i, &TeamChangePacket);//패킷 전송
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
											   //printf("중력 수치 변경 : %f\n", gravity_packet.gravity_state);
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

		break;
	}
	default:
	{

		printf("Unkown Packet Type from Client [%d]\n", id);
		return;
	}

	}
}
void SendMovePacket(int ID)
{
	sc_packet_pos pos_packet;
	pos_packet.type = SC_POS;
	pos_packet.size = sizeof(sc_packet_pos);
	pos_packet.id = ID;
	pos_packet.keyInputState = g_clients[ID].m_animstate;
 
	pos_packet.m_xmf3Position.x = g_clients[ID].m_pos._41;
	pos_packet.m_xmf3Position.y = g_clients[ID].m_pos._42;
	pos_packet.m_xmf3Position.z = g_clients[ID].m_pos._43;
	pos_packet.m_xmf4Quaternion = g_clients[ID].m_quat;
 
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == g_clients[i].m_isconnected&&g_clients[ID].m_iRoomNumb == g_clients[i].m_iRoomNumb)//접속중이고
			SendPacket(i, &pos_packet);//패킷 전송

	}
}
void SendRotatePacket(int ID)
{
	sc_packet_rotate pos_packet;
	pos_packet.type = SC_POS;
	pos_packet.size = sizeof(sc_packet_pos);
	pos_packet.id = ID;
	pos_packet.m_xmf4Quaternion = g_clients[ID].m_quat;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == g_clients[i].m_isconnected&&g_clients[ID].m_iRoomNumb == g_clients[i].m_iRoomNumb)
			SendPacket(i, &pos_packet);//패킷 전송

	}
}
void SendOccupationPacket(int ID, int OccupiedBase,int Team)
{
	sc_packet_stepBase p;
	p.type = SC_STEP_BASE;
	p.size = sizeof(sc_packet_stepBase);
	p.m_iOccupiedBase = OccupiedBase - 3;
	p.m_iTeam = Team;
	p.m_fPercentOfOccupation = base[OccupiedBase - 3].m_fStepTime;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == g_clients[i].m_isconnected)//접속중이고
			SendPacket(i, &p);//패킷 전송

	}
}

void SendGameResultPacket(int ID, int Winner)
{
	sc_packet_gameResult p;
	p.type = SC_GAME_RESULT;
	p.size = sizeof(sc_packet_gameResult);
	p.m_iWinTeam = Winner;

	if (true == g_clients[ID].m_isconnected)//접속중이고
		SendPacket(ID, &p);//패킷 전송


	if (Room[g_clients[ID].m_iRoomNumb].m_iState != GAMERESULT)
		Room[g_clients[ID].m_iRoomNumb].m_iState = GAMERESULT;
	g_clients[ID].m_scene_state = GAMERESULT;


	sc_packet_scene_change start_packet;
	start_packet.type = SC_SCENE_CHANGE;
	start_packet.size = sizeof(start_packet);
	start_packet.scenestate = GAMERESULT;
	start_packet.id = ID;
	SendPacket(ID, &start_packet);
	if (BLUETeam == Winner)
		printf("게임 결과 출력. 블루 팀 승리\n ");
	else if (REDTeam == Winner)
		printf("게임 결과 출력. 레드 팀 승리\n ");
	else
		printf("게임 결과 출력. 그린 팀 승리\n");
}





void SendDamagedPacket(int ID, XMFLOAT3 pos)
{

	sc_packet_attacked attacked_packet;

	attacked_packet.type = SC_ATTACKED;
	attacked_packet.id = ID;
	attacked_packet.hp = g_clients[ID].m_iHP;
	attacked_packet.size = sizeof(sc_packet_attacked);
	attacked_packet.m_xmfPos = pos;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == g_clients[i].m_isconnected)//접속중이고
		{
			SendPacket(i, &attacked_packet);//패킷 전송
		}
	}
	printf("피격 패킷 전송\n");
}

void SendNotifyDieMessage(int ID, int Slayer_ID)
{
	sc_packet_character_state die_packet;

	die_packet.type = SC_PLAYER_STATE_CHANGE;
	die_packet.id = ID;
	die_packet.slayer_id = Slayer_ID;
	die_packet.state = DIED;
	die_packet.size = sizeof(sc_packet_character_state);
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == g_clients[i].m_isconnected)//접속중이고
		{
			SendPacket(i, &die_packet);//패킷 전송
		}
	}
	printf("%d Player Died by %d!\n", ID, Slayer_ID);
}

void SendSceneChangePacket(int ID, int Scene)
{
	sc_packet_scene_change scene_packet;

	scene_packet.type = SC_SCENE_CHANGE;
	scene_packet.id = ID;
	scene_packet.scenestate = Scene;
	scene_packet.size = sizeof(sc_packet_scene_change);
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == g_clients[i].m_isconnected)//접속중이고
		{
			SendPacket(i, &scene_packet);//패킷 전송
		}
	}

}
int SoldierIdx = 0;
int DroneIdx = 0;
int CreatureIdx = 0;
void SendLoadingStartPacket(int RoomNum)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_isReady == true)//레디상태이고
		{
			if (g_clients[i].m_iRoomNumb == RoomNum)//같은방이면
			{
				sc_packet_put_player p; // 클라이언트(캐릭터)가 접속했다는 패킷을 하나 설정합니다.
				p.id = i;//아이디
				p.size = sizeof(sc_packet_put_player);//패킷사이즈
				p.type = SC_PUT_PLAYER;//패킷타입
				p.m_characterType = g_clients[i].m_iCharacterType;
				p.m_pos = g_clients[i].m_pos;
				p.m_iTeam = g_clients[i].m_iTeam;
				SendPacket(i, &p);//모든 인덱스를 검색해 접속해있는 클라이언트들에게 패킷을 전달합니다.
				printf("[%d] Player Character Load\n", i);
				for (int j = 0; j < MAX_USER; ++j)
				{
					if (true == g_clients[j].m_isconnected&& g_clients[i].m_iRoomNumb == g_clients[j].m_iRoomNumb)
					{
						if (i == j) continue;
						p.id = j;
						p.m_pos = g_clients[j].m_pos;
						p.m_iTeam = g_clients[j].m_iTeam;
						p.m_characterType = g_clients[j].m_iCharacterType;
						SendPacket(i, &p);

					}

				}

			}
		}
	}

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_iRoomNumb == RoomNum && g_clients[i].m_isconnected)//0번 방인데 접속 초기값을 0으로 초기화해서 문제좀있다
		{


			g_clients[i].m_scene_state = LOADING;
			sc_packet_scene_change start_packet;

			start_packet.type = SC_SCENE_CHANGE;
			start_packet.size = sizeof(start_packet);
			start_packet.scenestate = LOADING;

			start_packet.id = i;
			SendPacket(i, &start_packet);
			printf("[%d] Client 로딩씬으로 씬을 전환.\n ", i);
		}

	}

	Room[RoomNum].m_iState = LOADING;

}
void SendMoveSKillOffPacket(int ID)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_isconnected&&g_clients[i].m_scene_state == INGAME)//게임중이고
		{
			if (g_clients[ID].m_iRoomNumb == g_clients[i].m_iRoomNumb)//같은방이면
			{
				sc_packet_move_skill p;
				p.charactertype = g_clients[ID].m_iCharacterType;
				p.id = ID;
				p.size = sizeof(sc_packet_move_skill);
				p.type = SC_MOVE_SKILL_OFF;
				SendPacket(i, &p);
				printf("[%d]의 이동스킬(SHIFT) 종료\n", ID);
			}
		}
	}
}
void SendRespawnPacket(int ID)
{
	sc_packet_character_state die_packet;

	die_packet.type = SC_PLAYER_STATE_CHANGE;
	die_packet.id = ID;
	die_packet.state = ALIVED;
	die_packet.size = sizeof(sc_packet_character_state);
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == g_clients[i].m_isconnected)//접속중이고
		{
			if (g_clients[ID].m_iRoomNumb == g_clients[i].m_iRoomNumb)
				SendPacket(i, &die_packet);//패킷 전송
		}
	}
	printf("%d Player Respawned!\n", ID);
}

void SendGameStartPacket(int RoomNum)
{
	Room[RoomNum].m_iState = INGAME;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_iRoomNumb == RoomNum && g_clients[i].m_isconnected)//0번 방인데 접속 초기값을 0으로 초기화해서 문제좀있다
		{

			g_clients[i].m_scene_state = INGAME;
			sc_packet_scene_change start_packet;
			start_packet.type = SC_SCENE_CHANGE;
			start_packet.size = sizeof(start_packet);
			start_packet.scenestate = INGAME;
			start_packet.id = i;
			SendPacket(i, &start_packet);
			printf("[%d] Client 인게임으로 씬을 전환.\n ", i);
		}
	}
}
void SendMoveObjectPacket(int ID, int ObjectID)
{

	sc_packet_updateObject update_packet;
	update_packet.size = sizeof(sc_packet_updateObject);
	update_packet.type = SC_MOVE_OBJECT;
	update_packet.m_ObjectID = ObjectID;
	update_packet.m_xmf3ObjectPosition = g_xmf3ObjectsPos[ObjectID];
	update_packet.m_xmf4ObjectQuaternion = g_xmf3ObjectsQuaternion[ObjectID];
	SendPacket(ID, &update_packet);//패킷 전송
//	printf("[%d]업데이트\n", ObjectID);
}
void SendJumpOffPacket(int ID)
{
	sc_packet_jump_off jumpoff_packet;
	jumpoff_packet.size = sizeof(sc_packet_jump_off);
	jumpoff_packet.type = SC_JUMP_OFF;
	jumpoff_packet.id = ID;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_iRoomNumb == g_clients[ID].m_iRoomNumb && g_clients[i].m_isconnected)//0번 방인데 접속 초기값을 0으로 초기화해서 문제좀있다
			SendPacket(i, &jumpoff_packet);
	}
}
void SendOccupationComplete(int ID, int BaseNum)
{
	sc_occupation_complete packet;
	packet.size = sizeof(sc_occupation_complete);
	packet.type = SC_OCCUPATION_COMPLETE;
	packet.id = ID;
	packet.m_iOccupiedBase = BaseNum;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_iRoomNumb == g_clients[ID].m_iRoomNumb && g_clients[i].m_isconnected)//0번 방인데 접속 초기값을 0으로 초기화해서 문제좀있다
			SendPacket(i, &packet);
	}
}
void SendAttackSkillOnPacket(int ID, XMFLOAT3 pos)
{
	sc_packet_attack_skill packet;
	packet.size = sizeof(sc_packet_attack_skill);
	packet.type = SC_ATTACK_SKILL;
	packet.id = ID;
	packet.charactertype = g_clients[ID].m_iCharacterType;
	packet.m_xmf3ExplosivePos = pos;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_iRoomNumb == g_clients[ID].m_iRoomNumb && g_clients[i].m_isconnected)//0번 방인데 접속 초기값을 0으로 초기화해서 문제좀있다
			SendPacket(i, &packet);
	}
}
void SendTouchObjectPacket(int ID, XMFLOAT3 pos)
{
	sc_packet_attack_skill packet;
	packet.size = sizeof(sc_packet_attack_skill);
	packet.type = SC_TOUCH_OBJECT;
	packet.id = ID;
	packet.m_xmf3ExplosivePos = pos;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (g_clients[i].m_iRoomNumb == g_clients[ID].m_iRoomNumb && g_clients[i].m_isconnected)//0번 방인데 접속 초기값을 0으로 초기화해서 문제좀있다
			SendPacket(i, &packet);
	}
}
void worker_thread()
{
	while (true)
	{

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
		if (p_over->m_event == EVT_RECV) {
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
		else if (p_over->m_event == EVT_SEND) {  // Send 후처리
												 //cout << "WT:A packet was sent to Client[" << key << "]\n";
			delete p_over;
		}
		else if (p_over->m_event == EVT_MOVE)
		{
			SendMovePacket(key);
		}
		else if (p_over->m_event == EVT_OCCUPATION_BASE1 || p_over->m_event == EVT_OCCUPATION_BASE2 || p_over->m_event == EVT_OCCUPATION_BASE3)
		{
			SendOccupationPacket(key, p_over->m_event,p_over->m_object);
		}
		else if (p_over->m_event == EVT_ATTACKED)
		{
			SendDamagedPacket(key, p_over->m_xmf3ExplosivePos);
		}
		else if (p_over->m_event == EVT_PLAYER_DIE)
		{
			SendNotifyDieMessage(key, p_over->m_object);
		}
		else if (p_over->m_event == EVT_BLUETeam_WIN)
		{
			SendGameResultPacket(key, BLUETeam);
		}
		else if (p_over->m_event == EVT_REDTeam_WIN)
		{
			SendGameResultPacket(key, REDTeam);
		}
		else if (p_over->m_event == EVT_GREENTeam_WIN)
		{
			SendGameResultPacket(key, GREENTeam);
		}
		else if (p_over->m_event == EVT_LOADING_START)
		{
			SendLoadingStartPacket(g_clients[key].m_iRoomNumb);
		}
		else if (p_over->m_event == EVT_MOVE_SKILL_ON)
		{
			
		}
		else if (p_over->m_event == EVT_MOVE_SKILL_OFF)
		{
			SendMoveSKillOffPacket(key);
		}
		else if (p_over->m_event == EVT_RESPAWN_PLAYER)
		{
			SendRespawnPacket(key);
		}
		else if (p_over->m_event == EVT_GAME_START)
		{
			SendGameStartPacket(g_clients[key].m_iRoomNumb);
		}
		else if (p_over->m_event == EVT_MOVE_OBJECT)
		{
			SendMoveObjectPacket(key, p_over->m_object);
		}
		else if (p_over->m_event == EVT_JUMP_OFF)
		{
			SendJumpOffPacket(key);
		}
		else if (p_over->m_event == EVT_OCCUPATION_COMPLETE)
		{
			SendOccupationComplete(key, p_over->m_object);
		}
		else if (p_over->m_event == EVT_ATTACK_SKILL_ON)
		{
			SendAttackSkillOnPacket(key, p_over->m_xmf3ExplosivePos);
		}
		else if (p_over->m_event == EVT_RAY_HIT_DYNAMIC_OBJECT)
		{
			SendTouchObjectPacket(key, p_over->m_xmf3ExplosivePos);
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
	bind_addr.sin_addr.s_addr = INADDR_ANY;	// 0.0.0.0  아무대서나 오는 것을 다 받겠다.dh

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
		g_clients[id].m_animstate = 0;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(cs), gh_iocp, id, 0);
		g_clients[id].m_isconnected = true;// 이 아이디를 부여받은 클라이언트는 이제 서버와 연결이 됐습니다.
		g_clients[id].m_iRoomNumb = 0;
		StartRecv(id);
		Room[g_clients[id].m_iRoomNumb].m_iUser += 1;

		CPlayer *player = gPxFramework.GetPlayersInfo();
		if (id % 3 == 0)
		{
			player[id].SetPlayerTeam(BLUETeam);
			player[id].SetCharacterType(SOLDIER);
			g_clients[id].m_iTeam = BLUETeam;
			g_clients[id].m_iCharacterType = SOLDIER;
			//m_iEnemyTeam = REDTeam;
		}
		else if (id % 3 == 1)
		{
			player[id].SetPlayerTeam(REDTeam);
			player[id].SetCharacterType(DRONE);
			g_clients[id].m_iTeam = REDTeam;
			g_clients[id].m_iCharacterType = DRONE;
			//m_iEnemyTeam = BLUETeam;
		}
		else
		{
			player[id].SetPlayerTeam(GREENTeam);
			player[id].SetCharacterType(CREATURE);
			g_clients[id].m_iTeam = GREENTeam;
			g_clients[id].m_iCharacterType = CREATURE;
		}

		// 위의 정보를 서버에 받습니다.





		for (int i = 0; i < 3; ++i)
		{
			sc_packet_loadStaticObject object_packet;

			object_packet.size = sizeof(sc_packet_loadStaticObject);
			object_packet.type = SC_LOAD_STATIC_OBJECT;
			object_packet.m_iOccupationState = base[i].m_iState;
			object_packet.m_xmf3ObjectPosition = base[i].m_xmf3Position;
			object_packet.m_iTeam = g_clients[id].m_iTeam;
			object_packet.m_iCharacter = g_clients[id].m_iCharacterType;
			strcpy(object_packet.m_ObjectID, base[i].name);
			object_packet.m_iMyID = id;
			SendPacket(id, &object_packet);
		}

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (g_clients[i].m_isconnected)
			{
				sc_packet_put_player p;
				p.size = sizeof(sc_packet_put_player);
				p.type = SC_PUT_PLAYER;

				p.id = id;
				p.m_characterType = g_clients[id].m_iCharacterType;
				p.m_iTeam = g_clients[id].m_iTeam;
				p.m_pos = g_clients[id].m_pos;
				if (i != id)
					SendPacket(i, &p);

				p.id = i;
				p.m_characterType = g_clients[i].m_iCharacterType;
				p.m_iTeam = g_clients[i].m_iTeam;
				p.m_pos = g_clients[i].m_pos;
				SendPacket(id, &p);
			}
		}
	}
}

void physx_thread()
{
	system_clock::time_point calculate_start;
	//	float interpolation_time = 0.f; //현재 시간 - 이전 연산 시간

	while (true)
	{
		//연산처리
		gPxFramework.FrameAdvance(); 

	}
}

int main()
{
	vector <thread> w_threads;

	initialize();
	gPxFramework.OnCreate();


	//gPxFramework.BuildObjects();
	for (int i = 0; i < 4; ++i) {
		w_threads.push_back(thread{ worker_thread }); // 4인 이유는 쿼드코어 CPU 라서
	}
	thread p_thread{ physx_thread };
	//CreateAcceptThreads();
	thread a_thread{ accept_thread };
	CTimeMgr::GetInstance()->InitTime();

	thread t_thread{ timer_thread };

	for (auto& th : w_threads) th.join();
	a_thread.join();
	t_thread.join();
	p_thread.join();
}