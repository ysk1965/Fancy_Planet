//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "PhysXFramework.h"
extern HANDLE gh_iocp;
CPhysXFramework::CPhysXFramework()
{
}

CPhysXFramework::~CPhysXFramework()
{
}

bool CPhysXFramework::OnCreate()
{
	m_GameTimer.Start();
	InitializePhysxEngine();
	BuildObjects();
	return(true);
}

void CPhysXFramework::OnDestroy()
{
	m_GameTimer.Stop();
	ReleasePhysxEngine();
}

void CPhysXFramework::BuildObjects()
{
	//
	for (int i = 0; i < 3; i++) {
		base[i].m_iOccupiedState = NEUTRAL;
	}
	// 씬
	m_pScene = new PhysXScene(m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	m_pScene->BuildObject();

	m_pPlayer = new CPlayer[MAX_USER];
	// Player
	for (int i = 0; i < MAX_USER; i++) {
		//m_pScene->m_pPlayer[i] = new CPlayer();
		m_pPlayer[i] = m_pScene->m_pPlayer[i];

		m_pPlayer[i].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, m_pPxControllerManager, NULL, i);

		//cout << "I =" << i << endl;
		//cout << "Name = " <<m_pPlayer[i].getPxController()->getActor()->getName() << endl;
		m_pPlayer[i].SetMoveState(g_clients[i].m_animstate);
		m_pPlayer[i].SetPosition(XMFLOAT3(m_pPlayer[i].GetWorldMatrix()._41, m_pPlayer[i].GetWorldMatrix()._42, m_pPlayer[i].GetWorldMatrix()._43)); // 포지션 정해줘야 함.
		g_clients[i].m_pos = m_pPlayer[i].GetWorldMatrix();
	}

	m_GameTimer.Reset();
}

void CPhysXFramework::FrameAdvance()
{
	int cnt = 0;

	m_GameTimer.Tick(0.0f);

	if (GravityChangeState)
	{
		m_pPxScene->setGravity(m_pv3Gravity);
		printf("%f %f %f로 그라비티 변경 \n", m_pv3Gravity.x, m_pv3Gravity.y, m_pv3Gravity.z);
		GravityChangeState = false;
	}
	// PhysX Update
	if (m_pPxScene)
	{
		m_pPxScene->simulate(1 / 300.f);
		m_pPxScene->fetchResults(true);
	}
	// Dom Update
	const int diff = 300;
	for (int i = 0; i < 3; i++) 
	{
		//cout << "[ " << i << "]" << " 검사 중입니다. //" << base[i].m_iStepCnt << endl;
		if (base[i].m_iOccupiedState != NEUTRAL)
		{// 누군가가 점령중이면 검사해라

			if (base[i].m_fStepTime == base[i].m_fPriorStepTime) {
				base[i].m_iStepCnt++;
			}
			base[i].m_fPriorStepTime = base[i].m_fStepTime; // 이전시간은 이제 새로운 시간으로 업데이트 합니다.

			if (base[i].m_iStepCnt > diff) {
				base[i].m_iStepCnt = 0;
				base[i].m_fPriorStepTime = 0;
				base[i].m_iState = NEUTRAL; // 중립,
				base[i].m_iOccupiedState = NEUTRAL;
				base[i].m_fStepTime = 0.f;
				EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
				if (i == 1)
					ex->m_event = EVT_OCCUPATION_BASE1;
				else if (i == 2)
					ex->m_event = EVT_OCCUPATION_BASE2;
				else
					ex->m_event = EVT_OCCUPATION_BASE3;

				ex->m_object = NEUTRAL;
				printf("점령지 초기화\n");
				PostQueuedCompletionStatus(gh_iocp, 1, i, &ex->m_over);


			}
		}
	}

	// Player Update
	for (int i = 0; i < MAX_USER; i++) {
		if (g_clients[i].m_isconnected)
		{
			m_pPlayer[i].Update(m_GameTimer.GetTimeElapsed());
			if (g_clients[i].m_scene_state == INGAMEROOM)
			{
				g_clients[i].m_pos._41 = m_pPlayer[i].GetPosition().x;
				g_clients[i].m_pos._42 = m_pPlayer[i].GetPosition().y;
				g_clients[i].m_pos._43 = m_pPlayer[i].GetPosition().z;
				g_clients[i].m_quat = m_pPlayer[i].GetQuaternion();
			}
			else if (g_clients[i].m_scene_state == INGAME)
			{
				

				m_pPlayer[i].SetMoveState(g_clients[i].m_animstate);

			
				g_clients[i].m_pos._41 = m_pPlayer[i].GetPosition().x;
				g_clients[i].m_pos._42 = m_pPlayer[i].GetPosition().y;
				g_clients[i].m_pos._43 = m_pPlayer[i].GetPosition().z;
				g_clients[i].m_quat = m_pPlayer[i].GetQuaternion();
				 
				m_pPlayer[i].m_fFrameAdvanceTime += m_GameTimer.GetTimeElapsed();
				if (m_pPlayer[i].m_fFrameAdvanceTime >= 0.025f)
				{
					EXOVER *ex = new EXOVER; // 꼭 동적할당해서 보내야함 보내고 사라지면 메모리 크러쉬나니까...
					ex->m_event = EVT_MOVE;
					PostQueuedCompletionStatus(gh_iocp, 1, i, &ex->m_over);
					m_pPlayer[i].m_fFrameAdvanceTime = 0.f;

				}

				//printf("[%d]번 클라 좌표 : %f %f %f\n",i, c[i].m_pos._41, c[i].m_pos._42, c[i].m_pos._43);
				//printf("[%d]번 클라 좌표 : %f %f %f\n", i, m_pPlayer[i]->getFootPosition().x, m_pPlayer[i]->getFootPosition().y, m_pPlayer[i]->getFootPosition().z);
			}
		}

		PxRigidDynamic *buffer[OBJECTS_NUMBER];
		m_pPxScene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC, (PxActor**)buffer, (PxU32)OBJECTS_NUMBER, 0);
		//printf("sizeof PxActor :%d, sizeof PxActor**:%d", sizeof(PxActor), sizeof((PxActor**)buffer)); 
		m_fUpdateInterpolationTime += GetTimeElapsed();


		if (m_fUpdateInterpolationTime > 0.050f)
		{

			for (int i = 0; i < OBJECTS_NUMBER; ++i)
			{
				if (m_iObjectUpdateCount % 15 == 0)
				{
					g_xmf3ObjectsPos[i].x = (float)buffer[i]->getGlobalPose().p.x;
					g_xmf3ObjectsPos[i].y = (float)buffer[i]->getGlobalPose().p.y;
					g_xmf3ObjectsPos[i].z = (float)buffer[i]->getGlobalPose().p.z;

					g_xmf3ObjectsQuaternion[i].x = (float)buffer[i]->getGlobalPose().q.x;
					g_xmf3ObjectsQuaternion[i].y = (float)buffer[i]->getGlobalPose().q.y;
					g_xmf3ObjectsQuaternion[i].z = (float)buffer[i]->getGlobalPose().q.z;
					g_xmf3ObjectsQuaternion[i].w = (float)buffer[i]->getGlobalPose().q.w;

					for (int j = 0; j < MAX_USER; ++j)
					{

						if (true == g_clients[j].m_isconnected&&g_clients[j].m_scene_state == INGAME)//접속중이고 인게임 씬일 때만!
						{


							if (160000.f > ((m_pPlayer[j].GetWorldMatrix()._41 - g_xmf3ObjectsPos[i].x)*(m_pPlayer[j].GetWorldMatrix()._41 - g_xmf3ObjectsPos[i].x)
								+ ((m_pPlayer[j].GetWorldMatrix()._42 - g_xmf3ObjectsPos[i].y)*(m_pPlayer[j].GetWorldMatrix()._42 - g_xmf3ObjectsPos[i].y))
								+ ((m_pPlayer[j].GetWorldMatrix()._43 - g_xmf3ObjectsPos[i].z)*(m_pPlayer[j].GetWorldMatrix()._43 - g_xmf3ObjectsPos[i].z))))
							{
								EXOVER *ex = new EXOVER;
								ex->m_event = EVT_MOVE_OBJECT;
								ex->m_object = i;
								PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);


							}
						}
					}
				}
				else
				{
					if (!buffer[i]->isSleeping())
					{
						g_xmf3ObjectsPos[i].x = (float)buffer[i]->getGlobalPose().p.x;
						g_xmf3ObjectsPos[i].y = (float)buffer[i]->getGlobalPose().p.y;
						g_xmf3ObjectsPos[i].z = (float)buffer[i]->getGlobalPose().p.z;

						g_xmf3ObjectsQuaternion[i].x = (float)buffer[i]->getGlobalPose().q.x;
						g_xmf3ObjectsQuaternion[i].y = (float)buffer[i]->getGlobalPose().q.y;
						g_xmf3ObjectsQuaternion[i].z = (float)buffer[i]->getGlobalPose().q.z;
						g_xmf3ObjectsQuaternion[i].w = (float)buffer[i]->getGlobalPose().q.w;
						for (int j = 0; j < MAX_USER; ++j)
						{

							if (true == g_clients[j].m_isconnected&&g_clients[j].m_scene_state == INGAME)//접속중이고 인게임 씬일 때만!
							{


								if (160000.f > ((m_pPlayer[j].GetWorldMatrix()._41 - g_xmf3ObjectsPos[i].x)*(m_pPlayer[j].GetWorldMatrix()._41 - g_xmf3ObjectsPos[i].x)
									+ ((m_pPlayer[j].GetWorldMatrix()._42 - g_xmf3ObjectsPos[i].y)*(m_pPlayer[j].GetWorldMatrix()._42 - g_xmf3ObjectsPos[i].y))
									+ ((m_pPlayer[j].GetWorldMatrix()._43 - g_xmf3ObjectsPos[i].z)*(m_pPlayer[j].GetWorldMatrix()._43 - g_xmf3ObjectsPos[i].z))))
								{
									EXOVER *ex = new EXOVER;
									ex->m_event = EVT_MOVE_OBJECT;
									ex->m_object = i;
									PostQueuedCompletionStatus(gh_iocp, 1, j, &ex->m_over);

								}
							}

						}


					}

				}

			}
			m_fUpdateInterpolationTime = 0.f;
			m_iObjectUpdateCount++;
		}


		m_pPxPhysicsSDK->getVisualDebugger()->updateCamera("PhysXCamera",
			PxVec3(0, 1000, 0),
			PxVec3(0, 1, 0),
			PxVec3(0, 1, 0));


		///////////////////////////////////////////게임 결과////////////////////////////////////////////////
		//1. 시간끝날때까지 점령지 많은팀이 승리.
		//2. 점령지 모두 다 점령하면 제한시간이 되지 않아도 승패를 가르고 승리.
		//3. 점령지가 3개지만 각 팀 하나씩 점령했다면 킬 수가 많은 팀이 승리


	}
}
	void CPhysXFramework::InitializePhysxEngine()
	{
		m_pPxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_PxDefaultAllocatorCallback, m_PxDefaultErrorCallback);
		// 파운데이션 클래스의 인스턴스를 작성

		PxTolerancesScale PxScale = PxTolerancesScale();
		// 시뮬레이션이 실행되는 스케일을 정의하는 클래스
		// 시뮬레이션 스케일을 변경하려면 장면의 기본 중력 값을 변경하려는 것이됨, 안정적인 시뮬레이션에서는 bounceThreshold를 변경해야할 수도 있음.

		m_pPxPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pPxFoundation, PxScale, false);
		// PhysX SDK의 인스턴스 생성


		//Cooking Init
		PxCookingParams params(PxScale); // 메쉬 Cooking에 영향을 미치는 매개 변수 설명
		params.meshWeldTolerance = 0.001f; //메쉬 용접 공차
		params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES |
			PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES | PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES);
		// 메쉬 전처리 매개 변수, 옵션 제어하는데 사용
		m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pPxFoundation, params);
		// Cooking 인터페이스의 인스턴스를 생성

		PxInitExtensions(*m_pPxPhysicsSDK); // PhysXExtensions 라이브러리 초기화
		PxSceneDesc sceneDesc(m_pPxPhysicsSDK->getTolerancesScale()); // Scene의 설명자 클래스
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f); // 중력 값

		if (!sceneDesc.cpuDispatcher)
		{
			PxDefaultCpuDispatcher* pCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
			// CPU Task 디스패쳐의 기본 구현 (기본 디스패쳐 생성 그리고 초기화)
			sceneDesc.cpuDispatcher = pCpuDispatcher;
		}

		if (!sceneDesc.filterShader)
			sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		// 간단한 필터 셰이더 구현

		m_pPxScene = m_pPxPhysicsSDK->createScene(sceneDesc); // 씬 생성

		m_pPxControllerManager = PxCreateControllerManager(*m_pPxScene); // 컨트롤러 생성
		//m_pPxControllerManager->setTessellation(true, 10);

																		 // Physx Visual Debugger
		if (NULL == m_pPxPhysicsSDK->getPvdConnectionManager())
		{
			return;
		}
		PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();
		m_pPxPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
		m_pPVDConnection = PxVisualDebuggerExt::createConnection(m_pPxPhysicsSDK->getPvdConnectionManager(), "127.0.0.1", 5425, 1000, connectionFlags);
		// PVD 셋팅
	}

	void CPhysXFramework::ReleasePhysxEngine()
	{
		if (m_pPVDConnection)
			m_pPVDConnection->release();

		if (m_pPxControllerManager)
			m_pPxControllerManager->release();

		if (m_pPxScene)
			m_pPxScene->release();
		if (m_pPxFoundation)
			m_pPxFoundation->release();
		if (m_pPxPhysicsSDK)
			m_pPxPhysicsSDK->release();
		if (m_pCooking)
			m_pCooking->release();
	}

	void CPhysXFramework::SendPacket(int id, void *ptr)
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
			//if (WSA_IO_PENDING != err_no) error_display("Send Error! ", err_no);
		}
	}

