#include "stdafx.h"
#include "GameFramework.h"


void CGameFramework::SendBulletPacket()
{
	std::chrono::milliseconds ms;

	ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_bulletstart);
	
	if (m_iSceneState == INGAME)
	{
		if (g_my_info.state != DIED)
		{
			bool flag = false;

			if (m_nCharacterType == SOLDIER)
			{
				if (ms.count() > 700)
				{
					printf("발사\n");
					m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::HUMAN_SHOT));
					m_FmodSound.Update();
					flag = true;
				}
			}
			else if (m_nCharacterType == DRONE)
			{
				if (ms.count() > 500)
				{
					m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::DRONE_SHOT));
					m_FmodSound.Update();
					flag = true;
				}
			}
			else
			{
				if (ms.count() > 1000)
				{
					m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::CREATURE_SHOT));
					m_FmodSound.Update();
					flag = true;
				}
			}
			
			if (flag)
			{
				cs_packet_shot * my_pos_packet = reinterpret_cast<cs_packet_shot *>(send_buffer);
				my_pos_packet->size = sizeof(cs_packet_shot);
				send_wsabuf.len = sizeof(cs_packet_shot);
				my_pos_packet->id = g_myid;
				my_pos_packet->type = CS_SHOT;
				my_pos_packet->m_xmf3CameraLookAt = m_pCamera->GetLookVector();
				send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				m_bulletstart = chrono::system_clock::now();
			}
		}
	}
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	static bool bJumpState = false;
	bool bProcessedByScene = false;

	bool bPlayerKeySwitch = m_pPlayer->GetKeySwitch();

	std::chrono::milliseconds ms;
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - start);

	if (bPlayerKeySwitch&&g_my_info.state != DIED)
	{
		if (GetKeyboardState(pKeysBuffer) && m_ppScenes[CHARACTER])
			bProcessedByScene = m_ppScenes[CHARACTER]->ProcessInput(pKeysBuffer);


		if (!bProcessedByScene)
		{
			DWORD dwDirection = 0;
			
			if (pKeysBuffer[87] & 0xF0)// && m_isMoveInput == false)
			{
				dwDirection |= DIR_FORWARD;

				cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
				p->size = sizeof(cs_packet_pos);
				p->type = CS_POS;
				p->id = g_myid;
				p->keyInputState = FORWARD;

				g_player_info[g_myid].anim_state = FORWARD;
				g_my_info.anim_state = FORWARD;

				send_wsabuf.len = sizeof(cs_packet_pos);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				m_isMoveInput = true;
				keystate = true;
			}
			else if (pKeysBuffer[83] & 0xF0)//&& !m_isMoveInput)
			{
				dwDirection |= DIR_BACKWARD;

				cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
				p->size = sizeof(cs_packet_pos);
				p->type = CS_POS;
				p->id = g_myid;
				p->keyInputState = BACKWARD;
				//g_player_info[g_myid].anim_state = BACKWARD;
				g_my_info.anim_state = BACKWARD;
				send_wsabuf.len = sizeof(cs_packet_pos);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);


				m_isMoveInput = true;
				keystate = true;
			}
			else if (pKeysBuffer[65] & 0xF0)//&&!m_isMoveInput)
			{
				dwDirection |= DIR_LEFT;

				//g_player_info[g_myid].anim_state = LEFT;
				g_my_info.anim_state = LEFT;
				cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
				p->size = sizeof(cs_packet_pos);
				p->type = CS_POS;
				p->id = g_myid;
				p->keyInputState = LEFT;
				send_wsabuf.len = sizeof(cs_packet_pos);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				m_isMoveInput = true;
				keystate = true;
			}
			else if (pKeysBuffer[68] & 0xF0)//&&!m_isMoveInput)
			{
				dwDirection |= DIR_RIGHT;

				g_my_info.anim_state = static_cast<int>(RIGHT);

				cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
				p->size = sizeof(cs_packet_pos);
				p->type = CS_POS;
				p->id = g_myid;
				p->keyInputState = RIGHT;
				send_wsabuf.len = sizeof(cs_packet_pos);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				m_isMoveInput = true;
				keystate = true;
			}
			if (pKeysBuffer[VK_NEXT] & 0xF0)
			{
				dwDirection |= DIR_DOWN;
				//IsInput = true;
				keystate = true;
			}
			if (pKeysBuffer[VK_SPACE] & 0xF0) // jump
			{
				cs_packet_jump *p = reinterpret_cast<cs_packet_jump *>(send_buffer);
				p->size = sizeof(cs_packet_jump);
				p->type = CS_JUMP;
				p->id = g_myid;
				send_wsabuf.len = sizeof(cs_packet_jump);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				if (!m_pPlayer->GetJumpState())
				{
					if (m_nCharacterType == DRONE) {
						m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::DRONE_JUMP));
					}
					else if (m_nCharacterType == SOLDIER) {
						m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::HUMAN_JUMP));
					}
					else if (m_nCharacterType == CREATURE) {
						m_FmodSound.PlaySoundEffect(static_cast<int>(EFFECTSOUND::CREATURE_JUMP));
					}
				}
				keystate = true;
			}

			if (pKeysBuffer[VK_LSHIFT] & 0xF0 && m_pPlayer->GetSkill1Time() >= 1.f)
			{
				cs_packet_move_skill *p = reinterpret_cast<cs_packet_move_skill *>(send_buffer);
				p->size = sizeof(cs_packet_move_skill);
				p->type = CS_MOVE_SKILL;
				p->id = g_myid;

				p->charactertype = m_nCharacterType;

				send_wsabuf.len = sizeof(cs_packet_move_skill);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				keystate = true;
				m_pPlayer->UseSkill(0);
			}
			if (pKeysBuffer[VK_LCONTROL] & 0xF0 && m_pPlayer->GetSkill2Time() >= 1.f)
			{
				cs_packet_attack_skill *p = reinterpret_cast<cs_packet_attack_skill*> (send_buffer);
				p->size = sizeof(cs_packet_attack_skill);
				p->type = CS_ATTACK_SKILL;
				p->id = g_myid;
				p->charactertype = m_nCharacterType;
				send_wsabuf.len = sizeof(cs_packet_attack_skill);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				m_pPlayer->UseSkill(1);
			}
			
			float cxDelta = 0.0f, cyDelta = 0.0f;
			POINT ptCursorPos;

			if (true) // GetCapture() == m_hWnd [카메라 고정]
			{
				if(m_iSceneState == INGAME)
					GetCursorPos(&ptCursorPos);

				if (GetCapture() == m_hWnd)
				{
					m_ppScenes[EFFECT]->SetParticle(0, m_nCharacterType, 1, NULL);
					SendBulletPacket();
				}

				cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
				cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
				SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);

			}

			if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
			{
				if (cxDelta || cyDelta)
				{
					m_pPlayer->Rotate(cyDelta / 4, 0.0f, -cxDelta / 4);
					m_pPlayer->Rotate(cyDelta / 4, cxDelta / 4, 0.0f);

					cs_packet_rotate* p = reinterpret_cast<cs_packet_rotate *>(send_buffer);
					send_wsabuf.len = sizeof(cs_packet_rotate);
					send_wsabuf.buf = reinterpret_cast<char *>(p);
					p->size = sizeof(cs_packet_rotate);
					p->type = CS_ROTATE;
					p->id = g_myid;
					p->xmvUP = m_pPlayer->GetUpVector();
					p->xmvRight = m_pPlayer->GetRightVector();
					p->xmvLook = m_pPlayer->GetLookVector();
					DWORD iobyte;
					WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				}

				//if (dwDirection)
				//	m_pPlayer->Move(dwDirection, 620.0f * m_GameTimer.GetTimeElapsed(), true);
			}
		}
	}
	if (!m_isMoveInput)//키가 눌리지 않았을때?
	{
		if (g_my_info.state != DIED)
		{
			if (!m_pPlayer->GetJumpState())//키가 안눌려있고 죽지도않고 점프상태도아니다.
			{
				NotifyIdleState();
			}
		}
	}
	bJumpState = m_pPlayer->GetJumpState();
	m_pPlayer->SetPosition(XMFLOAT3(g_my_info.pos._41, g_my_info.pos._42, g_my_info.pos._43));
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_ppScenes)
		for (int i = 0; i < NUM_SUBSETS; i++)
			m_ppScenes[i]->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_F9:
		{
			BOOL bFullScreenState = FALSE;
			m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
			m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

			DXGI_MODE_DESC dxgiTargetParameters;
			dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			dxgiTargetParameters.Width = m_nWndClientWidth;
			dxgiTargetParameters.Height = m_nWndClientHeight;
			dxgiTargetParameters.RefreshRate.Numerator = 60;
			dxgiTargetParameters.RefreshRate.Denominator = 1;
			dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

			OnResizeBackBuffers();

			break;
		}

		default:
		{

			break;

		}
		break;

		case 'R':
		{
			cs_packet_ready * my_packet = reinterpret_cast<cs_packet_ready *>(send_buffer);
			my_packet->size = sizeof(my_packet);
			send_wsabuf.len = sizeof(my_packet);
			my_packet->type = CS_READY;
			my_packet->state = g_my_info.m_isReady;

			DWORD iobyte;
			printf("packet : READY  \n");
			if (m_iSceneState == INGAMEROOM)//레디패킷은 로비일대만
			{
				int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				if (ret) {
					int error_code = WSAGetLastError();
					//printf("Error while sending packet [%d]", error_code);
				}
			}
			break;
		}

		case 'Z':
		{
			cs_gravity_test * my_packet = reinterpret_cast<cs_gravity_test *>(send_buffer);
			my_packet->size = sizeof(my_packet);
			send_wsabuf.len = sizeof(my_packet);
			my_packet->type = CS_GRAVITY_TEST;

			DWORD iobyte;
			int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			if (ret) {
				int error_code = WSAGetLastError();
				//printf("Error while sending packet [%d]", error_code);

			}
			break;
		}
		case 'W':
		{
			//g_player_info[g_myid].anim_state = static_cast<int>(SOLDIER::ANIMATION_IDLE_STATE);
			g_my_info.anim_state = static_cast<int>(SOLDIER::ANIMATION_IDLE_STATE);

			cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
			p->size = sizeof(cs_packet_pos);
			p->type = CS_POS;
			p->id = g_myid;
			p->keyInputState = NONE;
			send_wsabuf.len = sizeof(cs_packet_pos);
			send_wsabuf.buf = reinterpret_cast<char *>(p);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			m_isMoveInput = false;
			break;
		}
		case 'A':
		{
			//g_player_info[g_myid].anim_state = static_cast<int>(SOLDIER::ANIMATION_IDLE_STATE);
			g_my_info.anim_state = static_cast<int>(SOLDIER::ANIMATION_IDLE_STATE);

			cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
			p->size = sizeof(cs_packet_pos);
			p->type = CS_POS;
			p->id = g_myid;
			p->keyInputState = NONE;
			send_wsabuf.len = sizeof(cs_packet_pos);
			send_wsabuf.buf = reinterpret_cast<char *>(p);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			m_isMoveInput = false;
			break;
		}
		case 'S':
		{
			//g_player_info[g_myid].anim_state = static_cast<int>(SOLDIER::ANIMATION_IDLE_STATE);
			g_my_info.anim_state = static_cast<int>(SOLDIER::ANIMATION_IDLE_STATE);

			cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
			p->size = sizeof(cs_packet_pos);
			p->type = CS_POS;
			p->id = g_myid;
			p->keyInputState = NONE;
			send_wsabuf.len = sizeof(cs_packet_pos);
			send_wsabuf.buf = reinterpret_cast<char *>(p);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			m_isMoveInput = false;
			break;
		}
		case 'D':
		{
			//g_player_info[g_myid].anim_state = static_cast<int>(SOLDIER::ANIMATION_IDLE_STATE);
			g_my_info.anim_state = static_cast<int>(SOLDIER::ANIMATION_IDLE_STATE);

			cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
			p->size = sizeof(cs_packet_pos);
			p->type = CS_POS;
			p->id = g_myid;
			p->keyInputState = NONE;
			send_wsabuf.len = sizeof(cs_packet_pos);
			send_wsabuf.buf = reinterpret_cast<char *>(p);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			m_isMoveInput = false;
			break;
		}

		case '1':
		{
			if (m_iSceneState == INGAMEROOM)//방 내에서 캐릭터 변경.
			{
				cs_packet_characterChange *p = reinterpret_cast<cs_packet_characterChange *>(send_buffer);
				p->size = sizeof(cs_packet_characterChange);
				p->type = CS_CHARACTER_CHANGE;
				p->id = g_myid;
				p->m_iCharacterType = SOLDIER;
				//m_nCharacterType = SOLDIER;
				send_wsabuf.len = sizeof(cs_packet_characterChange);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				keystate = true;
			}
			break;
		}
		case '2':
		{
			if (m_iSceneState == INGAMEROOM)
			{
				cs_packet_characterChange *p = reinterpret_cast<cs_packet_characterChange *>(send_buffer);
				p->size = sizeof(cs_packet_characterChange);
				p->type = CS_CHARACTER_CHANGE;
				p->id = g_myid;
				p->m_iCharacterType = DRONE;
				//m_nCharacterType = DRONE;
				send_wsabuf.len = sizeof(cs_packet_characterChange);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				keystate = true;
			}
			break;
		}
		case '3':
		{
			if (m_iSceneState == INGAMEROOM)
			{
				cs_packet_characterChange *p = reinterpret_cast<cs_packet_characterChange *>(send_buffer);
				p->size = sizeof(cs_packet_characterChange);
				p->type = CS_CHARACTER_CHANGE;
				p->id = g_myid;
				p->m_iCharacterType = CREATURE;
				//m_nCharacterType = CREATURE;
				send_wsabuf.len = sizeof(cs_packet_characterChange);
				send_wsabuf.buf = reinterpret_cast<char *>(p);
				DWORD iobyte;
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				keystate = true;
			}
			break;

		}
		}
	case WM_KEYDOWN:
	{
		break;
	}
	default:
	{
		break;
	}
	}
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
	{
		OnResizeBackBuffers();
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}
void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_ppScenes)
		for (int i = 0; i < NUM_SUBSETS; i++)
			m_ppScenes[i]->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	{
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);

		if (LOWORD(lParam) >= 430 && LOWORD(lParam) <= 600)
		{
			if (HIWORD(lParam) >= 650 && HIWORD(lParam) <= 710)
			{
				if (m_iSceneState == INGAMEROOM)
				{
					m_bReady = !m_bReady;
					
					cs_packet_ready * my_packet = reinterpret_cast<cs_packet_ready *>(send_buffer);
					my_packet->size = sizeof(my_packet);
					send_wsabuf.len = sizeof(my_packet);
					my_packet->type = CS_READY;
					my_packet->state = g_my_info.m_isReady;

					DWORD iobyte;
					printf("packet : READY  \n");
					if (m_iSceneState == INGAMEROOM)//레디패킷은 로비일대만
					{
						int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

						if (ret) {
							int error_code = WSAGetLastError();
							//printf("Error while sending packet [%d]", error_code);
						}
					}
					break;
					printf("게임 시작\n");
				}
			}
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{

		break;
	}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;

	}
}