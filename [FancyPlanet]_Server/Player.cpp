//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------


#include "Player.h"

// CPlayer

CPlayer::CPlayer()
{
	m_fFallAcceleration = 2.0f;
}

void CPlayer::BuildObject(PxPhysics * pPxPhysics, PxScene * pPxScene, PxMaterial * pPxMaterial, PxControllerManager * pPxControllerManager, void * pContext, int id)
{
	m_pScene = pPxScene;

	PxCapsuleControllerDesc   PxCapsuledesc;


	m_ID = id;


	if (m_iTeam == GREENTeam)
		PxCapsuledesc.position = (PxExtendedVec3(0.f, 0.f, 0.f));//테스트
	else if (m_iTeam == BLUETeam)
		PxCapsuledesc.position = (PxExtendedVec3(1540.f, 190.f, 1440.f));
	else if (m_iTeam == REDTeam)
		PxCapsuledesc.position = (PxExtendedVec3(830.f, 175.f, 500.f));

	PxCapsuledesc.radius = 10.0f;
	PxCapsuledesc.height = 15.0f;
	PxCapsuledesc.stepOffset = 30.f;
	PxCapsuledesc.volumeGrowth = 1.9f;
	PxCapsuledesc.slopeLimit = cosf(XMConvertToRadians(25.f));
	//PxCapsuledesc.nonWalkableMode = PxControllerNonWalkableMode::eFORCE_SLIDING;
	PxCapsuledesc.upDirection = PxVec3(0, 1, 0);
	PxCapsuledesc.contactOffset = 0.001f; // 접촉 변수
	pPxMaterial = pPxPhysics->createMaterial(0.5f, 0.5f, 0.5f);
	PxCapsuledesc.material = pPxMaterial;
	PxCapsuledesc.behaviorCallback = this;
	PxCapsuledesc.reportCallback = this;


	m_pPxCharacterController = pPxControllerManager->createController(PxCapsuledesc);



	string m_strName = "P_";
	string str = to_string(m_ID);


	m_strName.append(str);
	strcpy(g_clients[id].m_name, m_strName.c_str());
	m_pPxCharacterController->getActor()->setName(g_clients[id].m_name);
	//cout << m_pPxCharacterController->getActor()->getName() << " : "<<m_ID<<m_iTeam << " 팀 "<< endl;


	m_xmf4x4World._14 = 0.f;
	m_xmf4x4World._24 = 0.f;
	m_xmf4x4World._34 = 0.f;
	m_xmf4x4World._44 = 1.f;
	m_xmf4x4World._11 = 10.f;
	m_xmf4x4World._12 = 0.f;
	m_xmf4x4World._13 = 0.f;
	m_xmf4x4World._21 = 0.f;
	m_xmf4x4World._22 = 10.f;
	m_xmf4x4World._23 = 0.f;
	m_xmf4x4World._31 = 0.f;
	m_xmf4x4World._32 = 0.f;
	m_xmf4x4World._33 = 10.f;
	m_xmf4x4World._41 = PxCapsuledesc.position.x;
	m_xmf4x4World._42 = PxCapsuledesc.position.y;
	m_xmf4x4World._43 = PxCapsuledesc.position.z;

	g_clients[id].m_pos = m_xmf4x4World;
}
void CPlayer::setTerrain(CHeightMapTerrain* pTerrain)
{
	m_pTerrain = pTerrain;
}
void CPlayer::SetPosition(XMFLOAT3 vPosition)
{
	m_pPxCharacterController->setPosition(PxExtendedVec3(vPosition.x, vPosition.y, vPosition.z));
	//setFootPosition(PxExtendedVec3(vPosition.x, vPosition.y, vPosition.z));

}
void CPlayer::SetRotatePosition(XMFLOAT3 Look, XMFLOAT3 Up, XMFLOAT3 Right, const array <Client, MAX_USER> &arr)
{

}

void CPlayer::Fire()
{

	printf("Fire [%d]\n", m_ID);
	if (m_isAttackSkillState&&g_clients[m_ID].m_iCharacterType == CREATURE)
	{
		EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모qq리 크러쉬나니까...
		ex->m_event = EVT_ATTACK_SKILL_ON;
		PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);
	}
	PxScene* pScene = m_pScene;
	XMFLOAT3 m_vCameraEye = GetPosition();
	XMFLOAT3 m_vCameraLookAt = m_xmf3Look;

	PxVec3 origin = PxVec3(m_vCameraEye.x + (15 * m_vCameraLookAt.x), m_vCameraEye.y + 18.f + (15 * m_vCameraLookAt.y), m_vCameraEye.z + (15 * m_vCameraLookAt.z));
	PxVec3 unitDir = PxVec3(m_vCameraLookAt.x, m_vCameraLookAt.y, m_vCameraLookAt.z);

	PxReal maxDistance;
	if (g_clients[m_ID].m_iCharacterType == CREATURE)
		maxDistance = 100;
	else
		maxDistance = 1000;

	const PxU32 bufferSize = 256;
	PxRaycastHit hitBuffer[bufferSize];
	PxRaycastBuffer buf;
	PxQueryFilterData PxFilter;
	PxFilter.flags = PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC;

	bool status = pScene->raycast(origin, unitDir, maxDistance, buf, PxHitFlag::eDEFAULT, PxFilter);

	EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
	ex->m_event = EVT_RAY_HIT_DYNAMIC_OBJECT;
	ex->m_xmf3ExplosivePos = XMFLOAT3((float)buf.block.position.x, (float)buf.block.position.y, (float)buf.block.position.z);
	PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);

	if (status)
	{ // 맞은 Actor를 찾음
		PxRigidDynamic* FirstHitActor = buf.block.actor->is<PxRigidDynamic>();
		if (FirstHitActor) {
			cout << FirstHitActor->getName() << endl;
			if (FirstHitActor != NULL && FirstHitActor->getName() != NULL && FirstHitActor->getName() != m_pPxCharacterController->getActor()->getName())
			{
				string actor = FirstHitActor->getName();
				if (FirstHitActor->isRigidDynamic()) {
					if (actor == "Test")
					{
						if (m_isAttackSkillState&&g_clients[m_ID].m_iCharacterType == CREATURE)
							FirstHitActor->addForce(unitDir * 160000);
						else
							FirstHitActor->addForce(unitDir * 80000);
						FirstHitActor->setAngularVelocity(unitDir);
					}
				}

				for (int j = 0; j < MAX_USER; ++j)
				{

					if (g_clients[j].m_scene_state == INGAME && g_clients[j].m_isconnected == true)//접속중이고 게임중일때만
					{
						if (actor == (string)g_clients[j].m_name)//클라이언트 이름과 충돌체 이름 비교
						{
							if (m_iTeam != g_clients[j].m_iTeam)//팀이 다를때만 피격처리를 해야겠지
							{
								if (m_isAttackSkillState&&g_clients[m_ID].m_iCharacterType == CREATURE)
								{
									g_clients[j].m_iHP -= 40;
									EXOVER *ex1 = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
									ex1->m_event = EVT_ATTACKED;
									PostQueuedCompletionStatus(gh_iocp, 1, j, &ex1->m_over);
									printf("플레이어 [%s]가 40의 데미지를 받음\n", g_clients[j].m_name);
									m_isAttackSkillState = false;


								}
								else
								{
									g_clients[j].m_iHP -= 20;

									printf("플레이어 [%s]가 20의 데미지를 받음\n", g_clients[j].m_name);
									EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
									ex->m_event = EVT_ATTACKED;
									ex->m_xmf3ExplosivePos = XMFLOAT3((float)FirstHitActor->getGlobalPose().p.x, (float)FirstHitActor->getGlobalPose().p.y, (float)FirstHitActor->getGlobalPose().p.z);
									PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);
								}


								if (g_clients[j].m_iHP <= 0 && g_clients[j].m_isAliveState == true)
								{
									g_clients[j].m_iHP = 0;
									g_clients[j].m_isAliveState = false;
									EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
									ex->m_event = EVT_PLAYER_DIE;
									ex->m_object = m_ID;
									PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);
									Team[m_iTeam].m_iKill += 1;


								}
							}
						}
					}
				}
			}
		}
	}
}

void CPlayer::Rotate()
{
	PxTransform PxTrans = m_pPxCharacterController->getActor()->getGlobalPose();



	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;

	XMVECTOR v = XMQuaternionRotationMatrix(XMLoadFloat4x4(&m_xmf4x4World));
	XMFLOAT4 xmfQuat;
	XMStoreFloat4(&xmfQuat, v);

	PxTrans.q *= PxQuat(PxReal(xmfQuat.x), PxVec3(1, 0, 0));
	PxTrans.q *= PxQuat(PxReal(xmfQuat.y), PxVec3(0, 1, 0));
	PxTrans.q *= PxQuat(PxReal(xmfQuat.z), PxVec3(0, 0, 1));
	//cout << xmfQuat.x << xmfQuat.y << xmfQuat.z << endl;
	m_pPxCharacterController->getActor()->setGlobalPose(PxTrans);

	XMFLOAT4X4 matrix = m_xmf4x4World;
	matrix._11 = m_xmf3Right.x;
	matrix._12 = m_xmf3Right.y;
	matrix._13 = m_xmf3Right.z;
	matrix._21 =0.f;
	matrix._22 = m_xmf3Up.y;
	matrix._23 = 0.f;
	matrix._31 = 0.f;
	matrix._32 = 0.f;
	matrix._33 = m_xmf3Look.z;

	XMVECTOR v2 = XMQuaternionRotationMatrix(XMLoadFloat4x4(&matrix));
	
	XMFLOAT4 xmfQuat2;
	XMStoreFloat4(&xmfQuat2, v2);

	
	m_xmf4Quaternion.x = xmfQuat2.x;
	m_xmf4Quaternion.y = xmfQuat2.y;
	m_xmf4Quaternion.z = xmfQuat2.z;
	m_xmf4Quaternion.w = xmfQuat2.w;


}
void CPlayer::Jump()
{
	PxControllerState   m_pPxState;

	//피직스 객체의 상태값을 m_pPxState에 넣어준다.
	m_pPxCharacterController->getState(m_pPxState);


	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN && !m_isJumpState && !m_isDoubleJumpState)
	{
		m_fFallvelocity = 100.f;
		m_isJumpState = true;
	}
}

void CPlayer::UsingDoubleJump()
{

	if (g_clients[m_ID].m_iCharacterType == SOLDIER && m_isJumpState && !m_isDoubleJumpState)//더블점프
	{
		m_fFallvelocity = 150.f;
		//m_isJumpState = false;
		m_isDoubleJumpState = true;
		printf("더블점프\n");
	}
}
void CPlayer::Update(float fTimeElapsed)
{

	if (g_clients[m_ID].m_scene_state == INGAMEROOM)
	{
		if (m_iTeam == GREENTeam)
			m_pPxCharacterController->setFootPosition(PxExtendedVec3(1540.f, 190.f, 1440.f));//테스트
		else if (m_iTeam == BLUETeam)
			m_pPxCharacterController->setFootPosition(PxExtendedVec3(430.f, 245.f, 1490.f));
		else
			m_pPxCharacterController->setFootPosition(PxExtendedVec3(830.f, 175.f, 500.f));

		m_xmf4x4World._41 = m_pPxCharacterController->getFootPosition().x;
		m_xmf4x4World._42 = m_pPxCharacterController->getFootPosition().y;
		m_xmf4x4World._43 = m_pPxCharacterController->getFootPosition().z;
		m_xmf4Quaternion.x = m_pPxCharacterController->getActor()->getGlobalPose().q.x;
		m_xmf4Quaternion.y = m_pPxCharacterController->getActor()->getGlobalPose().q.y;
		m_xmf4Quaternion.z = m_pPxCharacterController->getActor()->getGlobalPose().q.z;
		m_xmf4Quaternion.w = m_pPxCharacterController->getActor()->getGlobalPose().q.w;
	}
	if (g_clients[m_ID].m_scene_state == INGAME)
	{
		m_fTimeElapsed = fTimeElapsed;
		m_fInterpolationTime += fTimeElapsed;
		//Fire(fTimeElapsed);
		// 중력 관련 + 바닥면 충돌 확인
		m_fFallvelocity -= m_fFallAcceleration * fTimeElapsed * 50.f;

		if (m_fFallvelocity < -1000.0f)
			m_fFallvelocity = -10.0f;

		Rotate();
		PxMove(60.f, fTimeElapsed);//서버
		if (m_isRapidMoveState)
		{
			m_fMoveSkillTimeCheck += fTimeElapsed;

			if ((float)chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_MoveSkillCoolTimer).count() / 1000.f > m_fMoveSkillCoolTime)
			{
				m_isRapidMoveState = false;
				EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
				ex->m_event = EVT_MOVE_SKILL_OFF;
				PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);
			}
		}

		if (m_isFlashState)
		{
			DroneFlash();
		}
		if (m_isAttackSkillState)
		{
			if (g_clients[m_ID].m_iCharacterType == SOLDIER || g_clients[m_ID].m_iCharacterType == CREATURE)
				UsingAttackSkill();
			else
			{
				m_fRazerbuf = m_fRazerbuf + m_fTimeElapsed;
				if (m_fRazerbuf < m_fRazerTime)
				{
					m_fRazerTickbuf += fTimeElapsed;
					if (m_fRazerTick < m_fRazerTickbuf)
					{
						UsingRazer(fTimeElapsed);
						m_fRazerTickbuf = 0.f;

					}
				}
				else
				{
					m_fRazerbuf = 0.f;
					m_isAttackSkillState = false;
				}
			}
		}
		if (m_isAttackState)
		{
			Fire();
			m_isAttackState = false;
		}


		
		m_pPxCharacterController->move(PxVec3(0, 1.f, 0) * fTimeElapsed * m_fFallvelocity, 0, fTimeElapsed, PxControllerFilters());


		if (g_clients[m_ID].m_isAliveState == false)
		{
			m_fRespawnTime += fTimeElapsed;

			if (m_fRespawnTime > 7.f)
			{
				m_fRespawnTime = 0.f;
				g_clients[m_ID].m_isAliveState = true;
				g_clients[m_ID].m_iHP = 100;
				g_clients[m_ID].m_animstate = NONE;
				if (m_iTeam == GREENTeam)
					m_pPxCharacterController->setFootPosition(PxExtendedVec3(1540.f, 190.f, 1440.f));
				else if (m_iTeam == BLUETeam)
					m_pPxCharacterController->setFootPosition(PxExtendedVec3(430.f, 245.f, 1490.f));//테스트
				else
					m_pPxCharacterController->setFootPosition(PxExtendedVec3(830.f, 175.f, 500.f));
				m_xmf4x4World._41 = m_pPxCharacterController->getFootPosition().x;
				m_xmf4x4World._42 = m_pPxCharacterController->getFootPosition().y;
				m_xmf4x4World._43 = m_pPxCharacterController->getFootPosition().z;

				EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
				ex->m_event = EVT_RESPAWN_PLAYER;
				PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);

				EXOVER *ex1 = new EXOVER;
				ex1->m_event = EVT_MOVE;
				PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex1->m_over);
			}

		
		}
		PxControllerState m_pPxState;

		m_pPxCharacterController->getState(m_pPxState);
		if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN ||
			m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_UP)
		{
			m_fFallvelocity = 0.f;


		}

		if (IsOnGround())
		{
			if (m_isJumpState)
			{
				m_isJumpState = false;
				EXOVER *ex1 = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
				ex1->m_event = EVT_JUMP_OFF;
				PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex1->m_over);
				if (m_isDoubleJumpState)
				{
					EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
					ex->m_event = EVT_MOVE_SKILL_OFF;
					PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);
					m_isDoubleJumpState = false;
				}
			}
		}
	}
}

void CPlayer::UsingRazer(float timeelapse)
{
	PxTransform PxTrans = m_pPxCharacterController->getActor()->getGlobalPose();

	XMFLOAT3 m_vCameraEye = GetPosition();
	XMFLOAT3 m_vCameraLookAt = m_xmf3Look;
	printf("Tick\n");

	PxVec3 origin = PxVec3(m_vCameraEye.x + (15 * m_vCameraLookAt.x), m_vCameraEye.y + 18.f + (15 * m_vCameraLookAt.y), m_vCameraEye.z + (15 * m_vCameraLookAt.z));
	PxVec3 unitDir = PxVec3(m_vCameraLookAt.x, m_vCameraLookAt.y, m_vCameraLookAt.z);

	PxReal maxDistance = 1000;

	const PxU32 bufferSize = 256;

	PxRaycastBuffer buf;

	PxQueryFilterData PxFilter;
	PxFilter.flags = PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC;

	bool status = m_pScene->raycast(origin, unitDir, maxDistance, buf, PxHitFlag::eDEFAULT, PxFilter);

	EXOVER *ex2 = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
	ex2->m_event = EVT_RAY_HIT_DYNAMIC_OBJECT;
	ex2->m_xmf3ExplosivePos = XMFLOAT3((float)buf.block.position.x, (float)buf.block.position.y, (float)buf.block.position.z);
	PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex2->m_over);

	if (status) { // 맞은 Actor를 찾음
		PxVec3 dir = (buf.block.position - origin).getNormalized();

		EXOVER *ex1 = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
		ex1->m_event = EVT_ATTACK_SKILL_ON;
		ex1->m_xmf3ExplosivePos = XMFLOAT3(buf.block.position.x, buf.block.position.y, buf.block.position.z);
		PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex1->m_over);

		PxRigidDynamic* FirstHitActor = buf.block.actor->is<PxRigidDynamic>();

		if (FirstHitActor)
		{
			string FirstHitActorName = FirstHitActor->getName();
			if (FirstHitActor != NULL && FirstHitActor->getName() != NULL && FirstHitActor->getName() != m_pPxCharacterController->getActor()->getName())
			{
				if (FirstHitActor->isRigidDynamic())
				{
					if (FirstHitActorName == "Test")
					{
						FirstHitActor->addForce(dir.getNormalized() * 40000, PxForceMode::eFORCE, false);
						FirstHitActor->setAngularVelocity(unitDir);
					}
				}

				for (int j = 0; j < MAX_USER; ++j)
				{
					if (g_clients[j].m_scene_state == INGAME && g_clients[j].m_isconnected == true)//접속중이고 게임중일때만
					{
						if (!strncmp(buf.block.actor->getName(), g_clients[j].m_name, 3))//클라이언트 이름과 충돌체 이름 비교
						{
							if (m_iTeam != g_clients[j].m_iTeam)//팀이 다를때만 피격처리를 해야겠지
							{
								if (m_isAttackSkillState&&g_clients[m_ID].m_iCharacterType == DRONE)
								{
									g_clients[j].m_iHP -= 15;

									printf("플레이어 [%s]가 10의 데미지를 받음\n", g_clients[j].m_name);
									EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
									ex->m_event = EVT_ATTACKED;
									ex->m_xmf3ExplosivePos = XMFLOAT3((float)FirstHitActor->getGlobalPose().p.x, (float)FirstHitActor->getGlobalPose().p.y, (float)FirstHitActor->getGlobalPose().p.z);
									PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);

									if (g_clients[j].m_iHP <= 0 && g_clients[j].m_isAliveState == true)
									{
										g_clients[j].m_iHP = 0;
										g_clients[j].m_isAliveState = false;
										EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
										ex->m_event = EVT_PLAYER_DIE;
										ex->m_object = m_ID;
										PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);
										Team[m_iTeam].m_iKill += 1;


									}


								}

							}
						}
					}
				}
			}
		}
	}
}

void CPlayer::UsingAttackSkill()
{
	if (g_clients[m_ID].m_iCharacterType == SOLDIER)
	{
		//m_pScene->lockWrite();
		//printf("발사: %lf, %lf, %lf\n", xmf3Position.x, xmf3Position.y, xmf3Position.z);
		PxTransform PxTrans = m_pPxCharacterController->getActor()->getGlobalPose();

		XMFLOAT3 m_vCameraEye = GetPosition();
		XMFLOAT3 m_vCameraLookAt = m_xmf3Look;


		PxVec3 origin = PxVec3(m_vCameraEye.x + (15 * m_vCameraLookAt.x), m_vCameraEye.y + 18.f + (15 * m_vCameraLookAt.y), m_vCameraEye.z + (15 * m_vCameraLookAt.z));
		PxVec3 unitDir = PxVec3(m_vCameraLookAt.x, m_vCameraLookAt.y, m_vCameraLookAt.z);

		PxReal maxDistance = 1000;

		const PxU32 bufferSize = 256;

		PxRaycastBuffer buf;

		PxQueryFilterData PxFilter;
		PxFilter.flags = PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC;

		bool status = m_pScene->raycast(origin, unitDir, maxDistance, buf, PxHitFlag::eDEFAULT, PxFilter);

		const PxReal bombRange = 100;
		bool isBoom = false;


		if (status) { // 맞은 Actor를 찾음
			cout << "x : " << buf.block.position.x << endl;
			cout << "y : " << buf.block.position.y << endl;
			cout << "z : " << buf.block.position.z << endl << endl;

			EXOVER *ex1 = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
			ex1->m_event = EVT_ATTACK_SKILL_ON;
			ex1->m_xmf3ExplosivePos = XMFLOAT3(buf.block.position.x, buf.block.position.y, buf.block.position.z);
			PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex1->m_over);


			PxRigidDynamic* FirstHitActor = buf.block.actor->is<PxRigidDynamic>();

			//cout << FirstHitActor->getName() << endl;

			if (FirstHitActor) {
				if (FirstHitActor != NULL && FirstHitActor->getName() != NULL && FirstHitActor->getName() != m_pPxCharacterController->getActor()->getName())
				{
					for (int j = 0; j < MAX_USER; ++j)
					{
						if (g_clients[j].m_scene_state == INGAME && g_clients[j].m_isconnected == true)//접속중이고 게임중일때만
						{
							if (!strncmp(buf.block.actor->getName(), g_clients[j].m_name, 3))//클라이언트 이름과 충돌체 이름 비교
							{
								if (m_iTeam != g_clients[j].m_iTeam)//팀이 다를때만 피격처리를 해야겠지
								{
									if (m_isAttackSkillState&&g_clients[m_ID].m_iCharacterType == SOLDIER)
									{
										g_clients[j].m_iHP -= 30;

										printf("플레이어 [%s]가 30의 데미지를 받음\n", g_clients[j].m_name);
										EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
										ex->m_event = EVT_ATTACKED;
										ex->m_xmf3ExplosivePos = XMFLOAT3((float)FirstHitActor->getGlobalPose().p.x, (float)FirstHitActor->getGlobalPose().p.y, (float)FirstHitActor->getGlobalPose().p.z);
										PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);

										if (g_clients[j].m_iHP <= 0 && g_clients[j].m_isAliveState == true)
										{
											g_clients[j].m_iHP = 0;
											g_clients[j].m_isAliveState = false;
											EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
											ex->m_event = EVT_PLAYER_DIE;
											ex->m_object = m_ID;
											PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);
											Team[m_iTeam].m_iKill += 1;


										}


									}
								}
							}
						}
					}
				}
			}
			PxVec3 dir = (buf.block.position - origin).getNormalized();

			//const PxU32 bufferSize = 256;
			//PxRaycastHit hitBuffer[bufferSize];
			//PxRaycastBuffer BombHitBuffer(hitBuffer, bufferSize);

			const int maxBuffer = 26;

			PxQueryFilterData PxFilter;
			PxFilter.flags = PxQueryFlag::eDYNAMIC;

			PxVec3 pts[maxBuffer];
			pts[0] = PxVec3(1, 0, 0);
			pts[1] = PxVec3(0, 1, 0);
			pts[2] = PxVec3(0, 0, 1);
			pts[3] = PxVec3(1, 0, 1);
			pts[4] = PxVec3(1, 1, 0);
			pts[5] = PxVec3(0, 1, 1);
			pts[6] = PxVec3(1, 1, 1);

			pts[7] = PxVec3(-1, 0, 0);
			pts[8] = PxVec3(0, -1, 0);
			pts[9] = PxVec3(0, 0, -1);
			pts[10] = PxVec3(-1, 0, -1);
			pts[11] = PxVec3(-1, -1, 0);
			pts[12] = PxVec3(0, -1, -1);
			pts[13] = PxVec3(-1, -1, -1);

			pts[14] = PxVec3(-1, 0, 1);
			pts[15] = PxVec3(1, 0, -1);
			pts[16] = PxVec3(-1, 1, 0);
			pts[17] = PxVec3(1, -1, 0);
			pts[18] = PxVec3(0, -1, 1);
			pts[19] = PxVec3(0, 1, -1);

			pts[20] = PxVec3(-1, 1, 1);
			pts[21] = PxVec3(1, -1, 1);
			pts[22] = PxVec3(1, 1, -1);
			pts[23] = PxVec3(-1, -1, 1);
			pts[24] = PxVec3(1, -1, -1);
			pts[25] = PxVec3(-1, 1, -1);


			PxRaycastBuffer BombHitBuffer[maxBuffer];

			for (int i = 0; i < maxBuffer; i++) {
				// 먼저 맞은 Actor에서 Ray를 발사
				isBoom = m_pScene->raycast(buf.block.position + pts[i].getNormalized(), pts[i].getNormalized(), bombRange, BombHitBuffer[i], PxHitFlag::eDEFAULT, PxFilter);
				if (isBoom == true) {
					PxRigidDynamic* HitBoomActor = BombHitBuffer[i].block.actor->is<PxRigidDynamic>();
					PxVec3 dir = (HitBoomActor->getGlobalPose().p - buf.block.position);
					//PxRigidBodyExt::addForceAtPos(*HitBoomActor, dir, HitBoomActor->getGlobalPose().p, PxForceMode::eIMPULSE, true);
					string test;
					test = HitBoomActor->getName();
					cout << "TEST : " << test << endl;
					if (test == "Test") {
						HitBoomActor->addForce(dir.getNormalized() * 80000, PxForceMode::eFORCE, true);
						HitBoomActor->setAngularVelocity(unitDir);
					}
					for (int j = 0; j < MAX_USER; ++j)
					{
						if (g_clients[j].m_scene_state == INGAME && g_clients[j].m_isconnected == true)//접속중이고 게임중일때만
						{
							if (!strncmp(buf.block.actor->getName(), g_clients[j].m_name, 3))//클라이언트 이름과 충돌체 이름 비교
							{
								if (m_iTeam != g_clients[j].m_iTeam)//팀이 다를때만 피격처리를 해야겠지
								{
									if (m_isAttackSkillState&&g_clients[m_ID].m_iCharacterType == SOLDIER)
									{
										g_clients[j].m_iHP -= 10;

										printf("플레이어 [%s]가 10의 데미지를 받음\n", g_clients[j].m_name);
										EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
										ex->m_event = EVT_ATTACKED;
										PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);
										m_isAttackSkillState = false;

										if (g_clients[j].m_iHP <= 0 && g_clients[j].m_isAliveState == true)
										{
											g_clients[j].m_iHP = 0;
											g_clients[j].m_isAliveState = false;
											EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
											ex->m_event = EVT_PLAYER_DIE;
											ex->m_object = m_ID;
											PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);
											Team[m_iTeam].m_iKill += 1;


										}

									}
								}
							}
						}
					}
					isBoom = false;
				}
			}
			cout << endl;
		}
		//m_pScene->unlockWrite();
	}
	else if (g_clients[m_ID].m_iCharacterType == CREATURE)
	{
		Fire();
	}
	SetAttackSkillState(false);

}

void CPlayer::DroneFlash()
{
	//cout << m_pTerrain->GetGridMesh()->OnGetHeight(330, 330, m_pTerrain->GetGridImage()) << endl;
	const float FlashSize = 60;
	const float Revise = 20;

	XMFLOAT3 m_vCameraLookAt = m_xmf3Look;

	//m_pScene->lockWrite();
	if (g_clients[m_ID].m_iCharacterType == DRONE)// 점멸
	{
		XMFLOAT3 NewPosition;
		NewPosition.x = m_xmf4x4World._41 + m_vCameraLookAt.x * FlashSize;
		NewPosition.z = m_xmf4x4World._43 + m_vCameraLookAt.z * FlashSize;
		//NewPosition.y = m_pTerrain->GetGridMesh()->OnGetHeight(NewPosition.x, NewPosition.z, m_pTerrain->GetGridImage());
		NewPosition.y = Revise + 3 * m_pTerrain->GetGridImage()->GetHeight(NewPosition.x, NewPosition.z, true);

		cout << endl;
		cout << "x = " << NewPosition.x << endl;
		cout << "z = " << NewPosition.z << endl;
		cout << "y = " << NewPosition.y << endl;
		cout << endl;

		SetPosition(NewPosition);

		m_isFlashState = false;
		EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
		ex->m_event = EVT_MOVE_SKILL_OFF;
		PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);

		//PxTransform PxTrans;
		//PxTrans.p.x = NewPosition.x;
		//PxTrans.p.y = NewPosition.y;
		//PxTrans.p.z = NewPosition.z;
		//
		//m_pPxCharacterController->getActor()->setKinematicTarget(PxTrans);

	}
	//m_pScene->unlockWrite();
	return;
}
bool CPlayer::IsOnGround(void)
{
	PxControllerState   m_pPxState;

	//피직스 객체의 상태값을 m_pPxState에 넣어준다.
	m_pPxCharacterController->getState(m_pPxState);

	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN)
		return true;
	return false;
}

void CPlayer::PxMove(float speed, float fTimeElapsed)
{

	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

	if (m_isMoveState == FORWARD)
	{
		//xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, 1);
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, 60.0f * fTimeElapsed);
	}
	else if (m_isMoveState == BACKWARD)
	{
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -60.0f * fTimeElapsed);
	}
	else if (m_isMoveState == LEFT)
	{

		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -60.0f * fTimeElapsed);
	}
	else if (m_isMoveState == RIGHT)
	{
		xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, 60.0f * fTimeElapsed);
	}
	else xmf3Shift = { 0.f, 0.f, 0.f };


	xmf3Shift = Vector3::Normalize(xmf3Shift);
	if (m_isRapidMoveState)//원시인의 이동스킬
	{
		speed = 120.f;

	}
	
	m_pPxCharacterController->move(PxVec3(xmf3Shift.x, xmf3Shift.y, xmf3Shift.z) *fTimeElapsed *speed * 1.4f, 0, fTimeElapsed, PxControllerFilters());

	m_xmf4x4World._41 = m_pPxCharacterController->getFootPosition().x;
	m_xmf4x4World._42 = m_pPxCharacterController->getFootPosition().y;
	m_xmf4x4World._43 = m_pPxCharacterController->getFootPosition().z;
	
	//printf("%f %f %f \n", m_pPxCharacterController->getFootPosition().x, m_pPxCharacterController->getFootPosition().y, m_pPxCharacterController->getFootPosition().z);
}


void CPlayer::onShapeHit(const PxControllerShapeHit & hit)
{
	PxRigidDynamic* actor = hit.shape->getActor()->is<PxRigidDynamic>();
	PxRigidStatic* OccupiedTerritory = hit.shape->getActor()->is<PxRigidStatic>();
	//string collider;



	if (actor)
	{
		string change;
		change = actor->getName();

		//if (change == "Test") {
		//printf("OnShapeHIT!!![%s]\n", actor->getName());
		//}



		if (actor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
			return;
		const PxVec3 upVector = hit.controller->getUpDirection();
		const PxF32 dp = hit.dir.dot(upVector);


		if (fabsf(dp) < 1e-3f)
		{
			const PxTransform globalPose = actor->getGlobalPose();
			const PxVec3 localPos = globalPose.transformInv(toVec3(hit.worldPos));

			AddForceAtLocalPos(*actor, hit.dir * hit.length * 1, localPos, PxForceMode::eACCELERATION, true);
			//AddForceAtLocalPos(*actor, hit.dir * hit.length * pObject->GetMass(), localPos, PxForceMode::eACCELERATION, true);
		}
	}

	if (OccupiedTerritory&&OccupiedTerritory->getName())
	{
		string collider;
		collider = OccupiedTerritory->getName();

		//cout << collider << endl;
		if (collider != "HeightField")
		{
			if (collider == "Dom1" &&base[0].m_iState != m_iTeam)//충돌한것이 점령지1이고, 점령지가 이미 플레이어의 팀 점령지가 아닐 때
			{
				if (base[0].m_iOccupiedState != m_iTeam)//적이 점령 시도중이었다면 점령게이지를 0으로 만들어버리자.
					base[0].m_fStepTime = 0.f;


				EXOVER *ex = new EXOVER;
				ex->m_event = EVT_OCCUPATION_BASE1;
				ex->m_object = m_iTeam;
				base[0].m_iStepCnt = true;
				base[0].m_fStepTime += m_fTimeElapsed;//점령도를 점차 증가시키고5
				base[0].m_iOccupiedState = m_iTeam;//점령중인 진영은 이 플레이어의 팀으로 세팅
				if (m_fInterpolationTime >= 0.4f)
				{
					PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);//0.4초마다 점령진행 신호 전달.
					m_fInterpolationTime = 0.f;
					printf("Occupation! Player[%d], Dom1 , %f Percent \n", m_ID, base[0].m_fStepTime);
				}
				if (base[0].m_fStepTime >= 5.f)//점령 성공 조건이다.
				{
					if (base[0].m_iState != m_iTeam)//상대 점령지였으면 
						Team[base[0].m_iState].m_iNumberOfterritories -= 1;//상대팀의 점령지 카운트를 1 차감
					printf("플레이어 [%d]가 Dom1을 점령했습니다.\n", m_ID);
					base[0].m_fStepTime = 0.f;//점령도를 0으로 바꾸고
					base[0].m_iState = m_iTeam;//이 점령지의 소속은 이 플레이어의 팀이 된다.
					base[0].m_iOccupiedState = NEUTRAL;//이 점령지에 대한 점령 진행중인 팀은 당장은 없다(중립)
					Team[m_iTeam].m_iNumberOfterritories += 1;//이 플레이어가 속한 팀의 점령지 카운트를 1 증가시킨다.

					EXOVER *ex = new EXOVER;
					ex->m_event = EVT_OCCUPATION_COMPLETE;
					ex->m_object = 0;//0번인덱스의 베이스 
					PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);
				}
			}

			if (collider == "Dom2" &&base[1].m_iState != m_iTeam)//충돌한것이 점령지2이고, 점령지가 이미 플레이어의 팀 점령지가 아닐 때
			{
				if (base[1].m_iOccupiedState != m_iTeam)//적이 점령 시도중이었다면 점령게이지를 0으로 만들어버리자.
					base[1].m_fStepTime = 0.f;

				EXOVER *ex = new EXOVER;
				ex->m_event = EVT_OCCUPATION_BASE2;
				ex->m_object = m_iTeam;
				base[1].m_iStepCnt = true;
				base[1].m_fStepTime += m_fTimeElapsed;
				base[1].m_iOccupiedState = m_iTeam;
				if (m_fInterpolationTime >= 0.4f)
				{
					PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);
					m_fInterpolationTime = 0.f;
					printf("Occupation! Player[%d], Dom2 , %f Percent \n", m_ID, base[1].m_fStepTime);
				}
				if (base[1].m_fStepTime >= 5.f)
				{
					if (base[1].m_iState != m_iTeam)//상대 점령지였으면 상대팀의 점령지 카운트를 1 차감
						Team[base[1].m_iState].m_iNumberOfterritories -= 1;
					base[1].m_fStepTime = 0.f;
					base[1].m_iState = m_iTeam;
					base[1].m_iOccupiedState = NEUTRAL;
					printf("플레이어 [%d]가 Dom2을 점령했습니다.\n", m_ID);
					Team[m_iTeam].m_iNumberOfterritories += 1;

					EXOVER *ex = new EXOVER;
					ex->m_event = EVT_OCCUPATION_COMPLETE;
					ex->m_object = 1;//1번인덱스의 베이스 
					PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);
				}
			}

			if (collider == "Dom3" &&base[2].m_iState != m_iTeam)//충돌한것이 점령지3이고, 점령지가 이미 플레이어의 팀 점령지가 아닐 때
			{
				if (base[2].m_iOccupiedState != m_iTeam)//적이 점령 시도중이었다면 점령게이지를 0으로 만들어버리자.
					base[2].m_fStepTime = 0.f;

				EXOVER *ex = new EXOVER;
				ex->m_event = EVT_OCCUPATION_BASE3;
				ex->m_object = m_iTeam;
				base[2].m_iStepCnt = true;
				base[2].m_fStepTime += m_fTimeElapsed;
				base[2].m_iOccupiedState = m_iTeam;
				if (m_fInterpolationTime >= 0.4f)
				{
					PostQueuedCompletionStatus(gh_iocp, 1, m_ID, &ex->m_over);
					m_fInterpolationTime = 0.f;
					printf("Occupation! Player[%d], Dom3 , %f Percent \n", m_ID, base[2].m_fStepTime);
				}
				if (base[2].m_fStepTime >= 5.f)
				{

					if (base[2].m_iState != m_iTeam)//상대 점령지였으면 상대팀의 점령지 카운트를 1 차감
						Team[base[2].m_iState].m_iNumberOfterritories -= 1;
					base[2].m_fStepTime = 0.f;
					base[2].m_iState = m_iTeam;
					base[2].m_iOccupiedState = NEUTRAL;
					printf("플레이어 [%d]가 Dom3을 점령했습니다.\n", m_ID);
					Team[m_iTeam].m_iNumberOfterritories += 1;


					EXOVER *ex = new EXOVER;
					ex->m_event = EVT_OCCUPATION_COMPLETE;
					ex->m_object = 2;//2번인덱스의 베이스 
					PostQueuedCompletionStatus(gh_iocp, 2, m_ID, &ex->m_over);
				}
			}
		}
	}
}

void CPlayer::AddForceAtLocalPos(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup)
{
	//transform pos to world space
	const PxVec3 globalForcePos = body.getGlobalPose().transform(pos);

	AddForceAtPosInternal(body, force, globalForcePos, mode, wakeup);
}

void CPlayer::AddForceAtPosInternal(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup)
{
	const PxTransform globalPose = body.getGlobalPose();
	const PxVec3 centerOfMass = globalPose.transform(body.getCMassLocalPose().p);

	const PxVec3 torque = (pos - centerOfMass).cross(force);
	body.addForce(force, mode, wakeup);
	body.addTorque(torque, mode, wakeup);
}

PxControllerBehaviorFlags CPlayer::getBehaviorFlags(const PxShape& shape, const PxActor& actor)
{
	const char* actorName = actor.getName();

	if (actor.getType() == PxActorType::eRIGID_DYNAMIC)
		//return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | PxControllerBehaviorFlag::eCCT_SLIDE;
		return PxControllerBehaviorFlag::eCCT_SLIDE;

	return PxControllerBehaviorFlags(0);
}

PxControllerBehaviorFlags CPlayer::getBehaviorFlags(const PxController &)
{
	return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | PxControllerBehaviorFlag::eCCT_SLIDE;
}

PxControllerBehaviorFlags CPlayer::getBehaviorFlags(const PxObstacle &)
{
	return PxControllerBehaviorFlags(0);
}