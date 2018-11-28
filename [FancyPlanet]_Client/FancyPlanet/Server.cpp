#include "stdafx.h"
#include "GameFramework.h"

void CGameFramework::InitNetwork(HWND main_window)
{
	string s_ip;
	//원속함수들을 세팅합니다.
	WSADATA   wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	printf("Server IP INPUT : ");

	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int Result = WSAConnect(g_mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	int retval = WSAAsyncSelect(g_mysocket, main_window, WM_SOCKET, FD_CLOSE | FD_READ);
	if (retval) {
		int err_code = WSAGetLastError();
		printf("AsyncSelect Error [%d]\n", err_code);
	}
	printf("retval : %d\n", retval);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = 4000;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = 4000;
}
void CGameFramework::ProcessPacket(char *ptr)
{

	static bool first_time = true;
	sc_packet_pos *p = reinterpret_cast<sc_packet_pos*>(ptr);
	switch (p->type)//패킷의 1번 인덱스에는 패킷의 종류에 대한 정보가 들어있습니다.
	{
	case SC_PUT_PLAYER://클라이언트 정보에 대한 패킷.
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;

		if (id == g_myid) { // 내 아이디와 패킷의 아이디가 같다면? == 위의 조건문을 돌았다 == 내 정보
			g_player_info[id].m_isconnected = true;
			g_player_info[id].m_iTeam = my_packet->m_iTeam;
						
			g_my_info.pos = my_packet->m_pos;
			g_player_info[id].pos = my_packet->m_pos;
			g_player_info[id].m_iCharacterType = my_packet->m_characterType;
			g_my_info.m_iTeam = my_packet->m_iTeam;

			printf("나의 팀 : %d (0 : 블루팀, 1 : 레드팀 2 : 그린팀) \n", my_packet->m_iTeam);
		}
		else {
			g_player_info[id].m_isconnected = true;
			g_player_info[id].pos = my_packet->m_pos;
			g_player_info[id].m_iTeam = my_packet->m_iTeam;
			g_player_info[id].m_iCharacterType = my_packet->m_characterType;
			printf("Other Client [%d] is Connect to Server", id);
		}

		break;
	}
	case SC_POS://좌표값에대한 패킷.
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;

		if (g_myid == id)
		{
			g_my_info.pos._41 = my_packet->m_xmf3Position.x;			
			g_my_info.pos._42 = my_packet->m_xmf3Position.y;
			g_my_info.pos._43 = my_packet->m_xmf3Position.z;
		}

		g_player_info[id].pos._41 = my_packet->m_xmf3Position.x;
		g_player_info[id].pos._42 = my_packet->m_xmf3Position.y;
		g_player_info[id].pos._43 = my_packet->m_xmf3Position.z;
		g_player_info[id].quat = my_packet->m_xmf4Quaternion;

		g_player_info[id].move_state = my_packet->keyInputState;



		if (g_player_info[id].m_iCharacterType == SOLDIER)
		{
			if (g_player_info[id].move_state == FORWARD)
				g_player_info[id].anim_state = static_cast<int>(SOLDIER::ANIMATION_MOVE_FORWARD);
			else if (g_player_info[id].move_state == BACKWARD)
				g_player_info[id].anim_state = static_cast<int>(SOLDIER::ANIMATION_MOVE_BACKWARD);
			else if (g_player_info[id].move_state == LEFT)
				g_player_info[id].anim_state = static_cast<int>(SOLDIER::ANIMATION_MOVE_LEFT);
			else if (g_player_info[id].move_state == RIGHT)
				g_player_info[id].anim_state = static_cast<int>(SOLDIER::ANIMATION_MOVE_RIGHT);
			else if (g_player_info[id].move_state == NONE)
				g_player_info[id].anim_state = NONE;

		}
		else if (g_player_info[id].m_iCharacterType == DRONE)
		{
			if (g_player_info[id].move_state == FORWARD)
				g_player_info[id].anim_state = static_cast<int>(DRONE::ANIMATION_MOVE_FORWARD);
			else if (g_player_info[id].move_state == BACKWARD)
				g_player_info[id].anim_state = static_cast<int>(DRONE::ANIMATION_MOVE_BACKWARD);
			else if (g_player_info[id].move_state == LEFT)
				g_player_info[id].anim_state = static_cast<int>(DRONE::ANIMATION_MOVE_LEFT);
			else if (g_player_info[id].move_state == RIGHT)
				g_player_info[id].anim_state = static_cast<int>(DRONE::ANIMATION_MOVE_RIGHT);
			else if (g_player_info[id].move_state == NONE)
				g_player_info[id].anim_state = NONE;
		}
		else if (g_player_info[id].m_iCharacterType == CREATURE)
		{
			if (g_player_info[id].move_state == FORWARD)
				g_player_info[id].anim_state = static_cast<int>(CREATURE::ANIMATION_MOVE_FORWARD);
			else if (g_player_info[id].move_state == BACKWARD)
				g_player_info[id].anim_state = static_cast<int>(CREATURE::ANIMATION_MOVE_BACKWARD);
			else if (g_player_info[id].move_state == LEFT)
				g_player_info[id].anim_state = static_cast<int>(CREATURE::ANIMATION_MOVE_LEFT);
			else if (g_player_info[id].move_state == RIGHT)
				g_player_info[id].anim_state = static_cast<int>(CREATURE::ANIMATION_MOVE_RIGHT);
			else if (g_player_info[id].move_state == NONE)
				g_player_info[id].anim_state = NONE;
			
		}

		if (g_player_info[my_packet->id].m_iCharacterType == CREATURE)
		{
			if (g_player_info[my_packet->id].m_isMoveSKillState)
			{
				if (g_player_info[id].move_state != NONE)
					m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_MOVE_RUN2), g_player_info, my_packet->id);
			}
		}
		if (g_myid != id)
		{
			if (!g_player_info[my_packet->id].m_isJumpState)
				m_ppScenes[CHARACTER]->ChangeAnimation(g_player_info[id].anim_state, g_player_info, my_packet->id);
		}
		else
		{
			if (g_player_info[id].anim_state == NONE)
			{
				m_ppScenes[CHARACTER]->ChangeAnimation(NONE, g_player_info, my_packet->id);
			}
			else if(g_player_info[my_packet->id].m_iCharacterType == CREATURE)
			{
				m_ppScenes[CHARACTER]->ChangeAnimation(NONE, g_player_info, my_packet->id);
			}
		}
		break;
	}
	case SC_ROTATE:
	{

		break;
	}

	case SC_REMOVE_PLAYER://접속 종료에 따른 패킷
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int id = my_packet->id;
		if (id == g_myid) {//내 캐릭터의 종료라면??

		}
		else {//다른 캐릭터의 종료라면??

			g_player_info[id].m_isconnected = false;
			printf("[%d] 클라이언트 접속 종료\n", id);
		}
		break;
	}
	case SC_READY://레디상태에 대한 체크
	{
		sc_packet_ready *my_packet = reinterpret_cast<sc_packet_ready *>(ptr);
		int id = my_packet->id;
		if (id == g_myid)
		{
			g_my_info.m_isReady = my_packet->isReadyState;

		}
		g_player_info[my_packet->id].m_isReady = my_packet->isReadyState;

		break;
	}
	case SC_SCENE_CHANGE:
	{
		sc_packet_scene_change *my_packet = reinterpret_cast<sc_packet_scene_change *>(ptr);
		g_my_info.m_scene = my_packet->scenestate;
		m_iSceneState = my_packet->scenestate;
		if (m_iSceneState == INGAME)//게임시작 패킷일 때
		{
			//m_FmodSound.StopSoundBG(static_cast<int>(BACKSOUND::BACKGROUND_ROBBY));
			//m_FmodSound.PlaySoundBG(static_cast<int>(BACKSOUND::BACKGROUND_INGAME));
			m_FmodSound.Update();

			printf("GameStart!\n");
		}
		else if (my_packet->scenestate == INGAMEROOM)
		{
			//m_FmodSound.StopSoundBG(static_cast<int>(BACKSOUND::BACKGROUND_INGAME));
			//m_FmodSound.PlaySoundBG(static_cast<int>(BACKSOUND::BACKGROUND_ROBBY));
			m_FmodSound.Update();

			g_my_info.m_scene = INGAMEROOM;
			m_fgametime = 0.f;
			m_fgravity = 0.f;

		}
		else if (my_packet->scenestate == LOADING)
		{
			printf("로딩중...\n");
		}


		break;
	}
	case SC_SHOT:
	{
		sc_packet_shot *my_packet = reinterpret_cast<sc_packet_shot *>(ptr);

		if (g_myid != my_packet->id)
		{
			if (!g_player_info[my_packet->id].m_isJumpState)
			{
				if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER)
				{
					m_ppScenes[EFFECT]->SetParticle(0, g_player_info[my_packet->id].m_iCharacterType, g_player_info[my_packet->id].Character_idx, NULL);
					m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(SOLDIER::ANIMATION_SHOT), g_player_info, my_packet->id);
				}
				else if (g_player_info[my_packet->id].m_iCharacterType == DRONE)
				{
					if (g_player_info[my_packet->id].move_state == FORWARD)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_MOVE_FORWARD_SHOT), g_player_info, my_packet->id);
					else if (g_player_info[my_packet->id].move_state == BACKWARD)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_MOVE_BACKWARD_SHOT), g_player_info, my_packet->id);
					else if (g_player_info[my_packet->id].move_state == LEFT)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_MOVE_LEFT_SHOT), g_player_info, my_packet->id);
					else if (g_player_info[my_packet->id].move_state == RIGHT)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_MOVE_RIGHT_SHOT), g_player_info, my_packet->id);
					else
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_SHOT), g_player_info, my_packet->id);
				}
				else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE)
				{
					if (g_player_info[my_packet->id].move_state == FORWARD)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_MOVE_FORWARD_SHOT), g_player_info, my_packet->id);
					else if (g_player_info[my_packet->id].move_state == BACKWARD)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_MOVE_BACKWARD_SHOT), g_player_info, my_packet->id);
					else if (g_player_info[my_packet->id].move_state == LEFT)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_MOVE_LEFT_SHOT), g_player_info, my_packet->id);
					else if (g_player_info[my_packet->id].move_state == RIGHT)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_MOVE_RIGHT_SHOT), g_player_info, my_packet->id);
					else
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_SHOT), g_player_info, my_packet->id);
				}
			}
			else
			{
				if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER)
				{
					m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(SOLDIER::ANIMATION_SHOT), g_player_info, my_packet->id);
				}
				else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE)
					m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_MOVE_JUMP_SHOT), g_player_info, my_packet->id);
				else
					m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_SHOT), g_player_info, my_packet->id);
			}
		}
		else
		{
			if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER)
			{
				m_ppScenes[EFFECT]->SetParticle(0, g_player_info[my_packet->id].m_iCharacterType, g_player_info[my_packet->id].Character_idx, NULL);
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(SOLDIER::ANIMATION_SHOT), g_player_info, my_packet->id);
			}
			else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE)
			{
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_SHOT), g_player_info, my_packet->id);
			}
			else
			{
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_SHOT), g_player_info, my_packet->id);
			}
		}
		break;
	}
	case SC_ATTACK_SKILL:
	{
		sc_packet_attack_skill *my_packet = reinterpret_cast<sc_packet_attack_skill *>(ptr);
		if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER)
		{
			m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::HUMANSKILL_1));
			m_ppScenes[EFFECT]->SetParticle(1, SOLDIER, g_player_info[my_packet->id].Character_idx, &my_packet->m_xmf3ExplosivePos);

			m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(SOLDIER::ANIMATION_SHOT), g_player_info, my_packet->id);
		}
		else if (g_player_info[my_packet->id].m_iCharacterType == DRONE)
		{
			m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::DRONESKILL_1));
			m_ppScenes[EFFECT]->SetParticle(3, g_player_info[my_packet->id].m_iCharacterType, g_player_info[my_packet->id].Character_idx, NULL);

			if (g_player_info[my_packet->id].anim_state == FORWARD)
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_MOVE_FORWARD_SHOT), g_player_info, my_packet->id);
			else if (g_player_info[my_packet->id].anim_state == BACKWARD)
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_MOVE_BACKWARD_SHOT), g_player_info, my_packet->id);
			else if (g_player_info[my_packet->id].anim_state == LEFT)
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_MOVE_LEFT_SHOT), g_player_info, my_packet->id);
			else if (g_player_info[my_packet->id].anim_state == RIGHT)
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_MOVE_RIGHT_SHOT), g_player_info, my_packet->id);
			else
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_SHOT), g_player_info, my_packet->id);
		}
		else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE)
		{
			m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::CREATURESKILL_1));
			m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_SHOT3), g_player_info, my_packet->id);
		}
		break;
	}

	case SC_CHARACTER_CHANGE:
	{
		sc_packet_characterChange *packet = reinterpret_cast<sc_packet_characterChange*>(ptr);
		g_player_info[packet->id].m_iCharacterType = packet->m_iCharacterType;
		g_player_info[packet->id].m_iTeam = packet->m_iCharacterType;
		if (packet->id == g_myid)
		{
			m_nCharacterType = packet->m_iCharacterType;
			g_my_info.m_iCharacterType = packet->m_iCharacterType;
			if (g_my_info.m_iCharacterType == SOLDIER)
			{
				m_nCharacterType = SOLDIER;

				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::ROBBY_CHANGECHARACTER));
				m_FmodSound.Update();
				printf("캐릭터 변경 : SOLDIER\n");
			}
			else if (g_my_info.m_iCharacterType == DRONE)
			{
				m_nCharacterType = DRONE;
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::ROBBY_CHANGECHARACTER));
				m_FmodSound.Update();
				printf("캐릭터 변경 : DRONE\n");
			}
			else
			{
				m_nCharacterType = CREATURE;
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::ROBBY_CHANGECHARACTER));
				m_FmodSound.Update();
				printf("캐릭터 변경 : CREATURE\n");
			}
		}
		break;
	}

	case SC_TIME:
	{
		sc_packet_time *my_packet = reinterpret_cast<sc_packet_time *>(ptr);
		m_fgametime = my_packet->m_ftime;
		printf("게임 시간 : %f\n", m_fgametime);

		break;
	}
	case SC_GAME_RESULT:

	{
		sc_packet_gameResult *my_packet = reinterpret_cast<sc_packet_gameResult *>(ptr);
		g_my_info.m_scene = GAMERESULT;
		m_iSceneState = GAMERESULT;

		m_nIsLobby = 3;

		m_pCamera = m_pPlayer->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

		if (g_my_info.m_iTeam == my_packet->m_iWinTeam)
		{
			m_bIsVictory = true;
			printf("승리!\n");
		}
		else
		{
			m_bIsVictory = false;
			printf("패배!\n");
		}
		break;
	}
	case SC_GRAVITY_CHANGE:
	{
		m_pComputeShader->SetComputeEffect(1);
		sc_gravity_change *my_packet = reinterpret_cast<sc_gravity_change *>(ptr);
		m_fgravity = my_packet->gravity_state;
		printf("중력 변경 %f\n", m_fgravity);
		break;
	}
	case SC_JUMP:
	{

		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;
		if (g_myid == id)
		{
			g_my_info.pos._41 = my_packet->m_xmf3Position.x;
			g_my_info.pos._42 = my_packet->m_xmf3Position.y;
			g_my_info.pos._43 = my_packet->m_xmf3Position.z;

			if (my_packet->keyInputState == JUMPING)
			{
				m_pPlayer->SetJumpState(true);
				g_player_info[id].m_isJumpState = true;
			}
		}
		g_player_info[id].pos._41 = my_packet->m_xmf3Position.x;
		g_player_info[id].pos._42 = my_packet->m_xmf3Position.y;
		g_player_info[id].pos._43 = my_packet->m_xmf3Position.z;



		break;
	}
	case SC_MOVE_SKILL_ON:
	{
		sc_packet_move_skill *my_packet = reinterpret_cast<sc_packet_move_skill*>(ptr);
		if (g_myid == my_packet->id)
		{
			if (g_my_info.m_iCharacterType == SOLDIER)//더블점프
			{
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::HUMANSKILL_2));
				printf("내 캐릭터의 더블점프 시작.\n", my_packet->id);

			}
			else if (g_my_info.m_iCharacterType == CREATURE)//비전이동
			{
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::CREATURESKILL_2));
				printf("내 캐릭터의 고속이동 시작.\n", my_packet->id);
			}
			else if (g_my_info.m_iCharacterType == DRONE)//고속이동
			{
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::DRONESKILL_2));
				printf("내 캐릭터의 비전이동 시작.\n", my_packet->id);
			}
			m_pPlayer->SetMoveSKillState(true);
		}

		if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER)
		{
			printf("[%d]플레이어의 더블점프 시작.\n", my_packet->id);
		}
		else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE)
		{
			printf("[%d]플레이어의 고속이동 시작.\n", my_packet->id);
		}
		else if (g_player_info[my_packet->id].m_iCharacterType == DRONE)
		{
			printf("[%d]플레이어의 비전이동 시작.\n", my_packet->id);
		}
		g_player_info[my_packet->id].m_isMoveSKillState = true;

		break;
	}
	case SC_JUMP_OFF:
	{
		sc_packet_jump_off *my_packet = reinterpret_cast<sc_packet_jump_off*>(ptr);
		g_player_info[my_packet->id].m_isJumpState = false;
		if (my_packet->id == g_myid)
		{
			m_pPlayer->SetJumpState(false);
			g_my_info.m_isJumpState = false;
		}
		break;
	}
	case SC_MOVE_SKILL_OFF:
	{
		sc_packet_move_skill *my_packet = reinterpret_cast<sc_packet_move_skill*>(ptr);
		if (g_myid == my_packet->id)
		{
			if (g_my_info.m_iCharacterType == SOLDIER)//더블점프
			{
				//각각의 스킬에맞는 이펙트나,쿨타임 처리
				printf("내 캐릭터의 더블점프 종료.\n", my_packet->id);
			}
			else if (g_my_info.m_iCharacterType == CREATURE)//비전이동
			{
				printf("내 캐릭터의 고속이동 종료.\n", my_packet->id);
			}
			else if (g_my_info.m_iCharacterType == DRONE)//고속이동
			{
				printf("내 캐릭터의 비전이동 종료.\n", my_packet->id);
			}
			m_pPlayer->SetMoveSKillState(false);
		}

		if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER)
		{
			printf("[%d]플레이어의 더블점프 종료.\n", my_packet->id);
			g_player_info[my_packet->id].m_isMoveSKillState = false;
		}
		else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE)
		{
			printf("[%d]플레이어의 고속이동 종료.\n", my_packet->id);
			g_player_info[my_packet->id].m_isMoveSKillState = false;
		}
		else if (g_player_info[my_packet->id].m_iCharacterType == DRONE)
		{
			printf("[%d]플레이어의 비전이동 종료.\n", my_packet->id);
		}


		break;
	}
	case SC_ATTACKED:
	{
		sc_packet_attacked *my_packet = reinterpret_cast<sc_packet_attacked*>(ptr);
		g_player_info[my_packet->id].m_iHP = my_packet->hp;
		if (my_packet->id == g_myid && g_my_info.state != DIED)
		{
			g_my_info.m_iHP = my_packet->hp;
			m_pComputeShader->SetComputeEffect(0);
		}
		if (g_player_info[my_packet->id].state != DIED)
		{
			if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER && !g_player_info[my_packet->id].m_isJumpState)
			{
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::HUMAN_SHOT));
				m_ppScenes[EFFECT]->SetParticle(2, SOLDIER, g_player_info[my_packet->id].Character_idx, &my_packet->m_xmfPos);
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(SOLDIER::ANIMATION_HIT), g_player_info, my_packet->id);
			}
			else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE && !g_player_info[my_packet->id].m_isJumpState)
			{
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::CREATURE_SHOT));
				m_ppScenes[EFFECT]->SetParticle(2, CREATURE, g_player_info[my_packet->id].Character_idx, &my_packet->m_xmfPos);
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_HIT), g_player_info, my_packet->id);
			}
			else
			{
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::DRONE_SHOT));
				m_ppScenes[EFFECT]->SetParticle(4, DRONE, g_player_info[my_packet->id].Character_idx, &my_packet->m_xmfPos);
				m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_HIT), g_player_info, my_packet->id);
			}
		}
		printf("%d 클라이언트가 공격받았습니다. HP : %d\n", my_packet->id, my_packet->hp);
		printf("공격받은 위치 %f %f %f\n", my_packet->m_xmfPos.x, my_packet->m_xmfPos.y, my_packet->m_xmfPos.z);


		break;
	}
	case SC_PLAYER_STATE_CHANGE:
	{
		sc_packet_character_state *my_packet = reinterpret_cast<sc_packet_character_state*>(ptr);
		g_player_info[my_packet->id].state = my_packet->state;

		if (my_packet->state == DIED)
		{
			Team[g_player_info[my_packet->slayer_id].m_iTeam].m_iKill += 1;
			g_player_info[my_packet->id].state = my_packet->state;
			if (my_packet->id == g_myid)
			{
				printf("내 캐릭터가 죽었습니다\n");
				g_my_info.m_iHP = 0;
				g_my_info.state = my_packet->state;
				if (m_iSceneState == INGAME)
				{
					if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER && !g_player_info[my_packet->id].m_isJumpState) {
						m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::HUMAN_DIED));
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(SOLDIER::ANIMATION_DIED), g_player_info, my_packet->id);
					}
					else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE && !g_player_info[my_packet->id].m_isJumpState) {
						m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::CREATURE_DIED));
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_DIED), g_player_info, my_packet->id);
					}
					else {
						m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::DRONE_DIED));
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_DIED), g_player_info, my_packet->id);
					}
				}
			}
			else
			{
				printf("플레이어 [%d]가 죽었습니다", my_packet->id);
				//다른 클라가 죽었을 때에 대한 처리
				if (m_iSceneState == INGAME)
				{
					if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER && !g_player_info[my_packet->id].m_isJumpState)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(SOLDIER::ANIMATION_DIED), g_player_info, my_packet->id);
					else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE && !g_player_info[my_packet->id].m_isJumpState)
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_DIED), g_player_info, my_packet->id);
					else
						m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(DRONE::ANIMATION_DIED), g_player_info, my_packet->id);
				}
			}
		}
		else if (my_packet->state == ALIVED)
		{
			if (my_packet->id == g_myid)
			{
				printf("내 캐릭터가 부활했습니다.\n");
				g_my_info.state = my_packet->state;
				g_my_info.m_iHP = 100;
				if (m_iSceneState == INGAME)
				{
					if (m_nCharacterType == DRONE)
					{
						m_ppScenes[CHARACTER]->ResetCharacter(g_player_info[my_packet->id].Character_idx, m_nCharacterType);
					}
					else if (m_nCharacterType == SOLDIER)
					{
						m_ppScenes[CHARACTER]->ResetCharacter(g_player_info[my_packet->id].Character_idx, m_nCharacterType);
					}
					else if (m_nCharacterType == CREATURE)
					{
						m_ppScenes[CHARACTER]->ResetCharacter(g_player_info[my_packet->id].Character_idx, m_nCharacterType);
					}
				}
			}
			else
			{
				printf("플레이어 [%d]가 부활했습니다.\n", my_packet->id);
				//다른 클라가 죽었을 때에 대한 처리
				if (m_iSceneState == INGAME)
				{
					if (g_player_info[my_packet->id].m_iCharacterType == DRONE)
					{
						m_ppScenes[CHARACTER]->ResetCharacter(g_player_info[my_packet->id].Character_idx, DRONE);
					}
					else if (g_player_info[my_packet->id].m_iCharacterType == SOLDIER)
					{
						m_ppScenes[CHARACTER]->ResetCharacter(g_player_info[my_packet->id].Character_idx, SOLDIER);
					}
					else if (g_player_info[my_packet->id].m_iCharacterType == CREATURE)
					{
						m_ppScenes[CHARACTER]->ResetCharacter(g_player_info[my_packet->id].Character_idx, CREATURE);
					}
				}
			}
			g_player_info[my_packet->id].m_iHP = 100;
		}
		break;
	}
	case SC_LOAD_OBJECT:
	{
		sc_packet_loadObject *my_packet = reinterpret_cast<sc_packet_loadObject*>(ptr);


		m_xmf3ObjectsPos[my_packet->m_ObjectID] = my_packet->m_xmf3ObjectPosition;
		m_xmf4ObjectsQuaternion[my_packet->m_ObjectID] = my_packet->m_xmf4ObjectQuaternion;

		break;
	}

	case SC_MOVE_OBJECT:
	{
		sc_packet_updateObject *my_packet = reinterpret_cast<sc_packet_updateObject*>(ptr);


		m_xmf3ObjectsPos[my_packet->m_ObjectID] = my_packet->m_xmf3ObjectPosition;
		m_xmf4ObjectsQuaternion[my_packet->m_ObjectID] = my_packet->m_xmf4ObjectQuaternion;
		//printf("[%d] 오브젝트 이동\n", my_packet->m_ObjectID);

		break;
	}
	case SC_LOAD_STATIC_OBJECT:
	{
		sc_packet_loadStaticObject *my_packet = reinterpret_cast<sc_packet_loadStaticObject*>(ptr);
		g_myid = my_packet->m_iMyID;

		g_my_info.m_iTeam = my_packet->m_iTeam;
		g_my_info.m_iCharacterType = my_packet->m_iCharacter;
		m_nCharacterType = g_my_info.m_iCharacterType;
		g_player_info[g_myid].m_iTeam = my_packet->m_iTeam;
		g_player_info[g_myid].m_iCharacterType = my_packet->m_iCharacter;


		printf("건물정보 %s, 위치  %f %f %f \n", my_packet->m_ObjectID, my_packet->m_xmf3ObjectPosition.x, my_packet->m_xmf3ObjectPosition.y, my_packet->m_xmf3ObjectPosition.z);
		cs_packet_requestObject * my_pos_packet = reinterpret_cast<cs_packet_requestObject *>(send_buffer);
		my_pos_packet->size = sizeof(cs_packet_requestObject);
		send_wsabuf.len = sizeof(cs_packet_requestObject);
		my_pos_packet->type = CS_REQUEST_OBJECT;

		DWORD iobyte;
		WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		break;
	}
	case SC_STEP_BASE:
	{
		sc_packet_stepBase *my_packet = reinterpret_cast<sc_packet_stepBase*>(ptr);
		printf("점령지 [%d]가 [%d]팀에 의해 점령 중. 점령율 : %f 퍼센트 \n", my_packet->m_iOccupiedBase, my_packet->m_iTeam, my_packet->m_fPercentOfOccupation);
		Base[my_packet->m_iOccupiedBase].m_fStepTime = my_packet->m_fPercentOfOccupation;

		if (my_packet->m_iTeam == g_my_info.m_iTeam)
		{
			if (Base[my_packet->m_iOccupiedBase].m_fStepTime < 0.5f) {
				m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::INGAME_CHARGE));
			}
			m_pUIShader->SetOccupationGauge(Base[my_packet->m_iOccupiedBase].m_fStepTime);
		}
		if (my_packet->m_iTeam == NEUTRAL)
			m_pUIShader->SetOccupationGauge(0);
		break;
	}
	case SC_OCCUPATION_COMPLETE:
	{
		sc_occupation_complete *my_packet = reinterpret_cast<sc_occupation_complete*>(ptr);
		if (Base[my_packet->m_iOccupiedBase].m_iState != NEUTRAL)
			Team[Base[my_packet->m_iOccupiedBase].m_iState].m_iNumberOfterritories -= 1;

		Base[my_packet->m_iOccupiedBase].m_iState = g_player_info[my_packet->id].m_iTeam;
		m_ppScenes[TERRAIN]->BaseSetting(my_packet->m_iOccupiedBase, g_player_info[my_packet->id].m_iTeam);
		Base[my_packet->m_iOccupiedBase].m_fStepTime = 0.f;
		Base[my_packet->m_iOccupiedBase].m_iOccupiedState = NEUTRAL;
		Team[g_player_info[my_packet->id].m_iTeam].m_iNumberOfterritories += 1;
		if (g_player_info[my_packet->id].m_iTeam == g_my_info.m_iTeam)
		{
			m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::INGAME_COMPLETE));
			m_pUIShader->SetOccupationGauge(Base[my_packet->m_iOccupiedBase].m_fStepTime);
		}
		printf("점령지 [%d]가 [%d]에 의해 점령되었습니다.\n", my_packet->m_iOccupiedBase, my_packet->id);
		break;
	}
	case SC_TOUCH_OBJECT:
	{
		sc_packet_attack_skill *my_packet = reinterpret_cast<sc_packet_attack_skill*>(ptr);
		if (g_player_info[my_packet->id].m_iCharacterType == DRONE)
		{
			m_ppScenes[EFFECT]->SetParticle(0, DRONE, g_player_info[my_packet->id].Character_idx, &my_packet->m_xmf3ExplosivePos);
		}
		printf("%f %f %f에 있는 다이나믹 오브젝트를 터치함\n", my_packet->m_xmf3ExplosivePos.x, my_packet->m_xmf3ExplosivePos.y, my_packet->m_xmf3ExplosivePos.z);
		break;
	}
	default:
	{
		printf("Unknown PACKET type [%d] size [%d]\n", p->size, p->type);
		return;
	}

	}
}

void CGameFramework::ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte)
	{
		if (0 == in_packet_size)
			in_packet_size = ptr[0];

		if (iobyte + saved_packet_size >= in_packet_size)
		{
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);

			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}