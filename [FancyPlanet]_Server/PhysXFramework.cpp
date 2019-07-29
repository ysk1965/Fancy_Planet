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
	// ��
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
		m_pPlayer[i].SetPosition(XMFLOAT3(m_pPlayer[i].GetWorldMatrix()._41, m_pPlayer[i].GetWorldMatrix()._42, m_pPlayer[i].GetWorldMatrix()._43)); // ������ ������� ��.
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
		printf("%f %f %f�� �׶��Ƽ ���� \n", m_pv3Gravity.x, m_pv3Gravity.y, m_pv3Gravity.z);
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
		//cout << "[ " << i << "]" << " �˻� ���Դϴ�. //" << base[i].m_iStepCnt << endl;
		if (base[i].m_iOccupiedState != NEUTRAL)
		{// �������� �������̸� �˻��ض�

			if (base[i].m_fStepTime == base[i].m_fPriorStepTime) {
				base[i].m_iStepCnt++;
			}
			base[i].m_fPriorStepTime = base[i].m_fStepTime; // �����ð��� ���� ���ο� �ð����� ������Ʈ �մϴ�.

			if (base[i].m_iStepCnt > diff) {
				base[i].m_iStepCnt = 0;
				base[i].m_fPriorStepTime = 0;
				base[i].m_iState = NEUTRAL; // �߸�,
				base[i].m_iOccupiedState = NEUTRAL;
				base[i].m_fStepTime = 0.f;
				EXOVER *ex = new EXOVER; // �� �����Ҵ��ؼ� �������� ������ ������� �޸� ũ�������ϱ�...
				if (i == 1)
					ex->m_event = EVT_OCCUPATION_BASE1;
				else if (i == 2)
					ex->m_event = EVT_OCCUPATION_BASE2;
				else
					ex->m_event = EVT_OCCUPATION_BASE3;

				ex->m_object = NEUTRAL;
				printf("������ �ʱ�ȭ\n");
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
					EXOVER *ex = new EXOVER; // �� �����Ҵ��ؼ� �������� ������ ������� �޸� ũ�������ϱ�...
					ex->m_event = EVT_MOVE;
					PostQueuedCompletionStatus(gh_iocp, 1, i, &ex->m_over);
					m_pPlayer[i].m_fFrameAdvanceTime = 0.f;

				}

				//printf("[%d]�� Ŭ�� ��ǥ : %f %f %f\n",i, c[i].m_pos._41, c[i].m_pos._42, c[i].m_pos._43);
				//printf("[%d]�� Ŭ�� ��ǥ : %f %f %f\n", i, m_pPlayer[i]->getFootPosition().x, m_pPlayer[i]->getFootPosition().y, m_pPlayer[i]->getFootPosition().z);
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

						if (true == g_clients[j].m_isconnected&&g_clients[j].m_scene_state == INGAME)//�������̰� �ΰ��� ���� ����!
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

							if (true == g_clients[j].m_isconnected&&g_clients[j].m_scene_state == INGAME)//�������̰� �ΰ��� ���� ����!
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


		///////////////////////////////////////////���� ���////////////////////////////////////////////////
		//1. �ð����������� ������ �������� �¸�.
		//2. ������ ��� �� �����ϸ� ���ѽð��� ���� �ʾƵ� ���и� ������ �¸�.
		//3. �������� 3������ �� �� �ϳ��� �����ߴٸ� ų ���� ���� ���� �¸�


	}
}
	void CPhysXFramework::InitializePhysxEngine()
	{
		m_pPxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_PxDefaultAllocatorCallback, m_PxDefaultErrorCallback);
		// �Ŀ�̼� Ŭ������ �ν��Ͻ��� �ۼ�

		PxTolerancesScale PxScale = PxTolerancesScale();
		// �ùķ��̼��� ����Ǵ� �������� �����ϴ� Ŭ����
		// �ùķ��̼� �������� �����Ϸ��� ����� �⺻ �߷� ���� �����Ϸ��� ���̵�, �������� �ùķ��̼ǿ����� bounceThreshold�� �����ؾ��� ���� ����.

		m_pPxPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pPxFoundation, PxScale, false);
		// PhysX SDK�� �ν��Ͻ� ����


		//Cooking Init
		PxCookingParams params(PxScale); // �޽� Cooking�� ������ ��ġ�� �Ű� ���� ����
		params.meshWeldTolerance = 0.001f; //�޽� ���� ����
		params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES |
			PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES | PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES);
		// �޽� ��ó�� �Ű� ����, �ɼ� �����ϴµ� ���
		m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pPxFoundation, params);
		// Cooking �������̽��� �ν��Ͻ��� ����

		PxInitExtensions(*m_pPxPhysicsSDK); // PhysXExtensions ���̺귯�� �ʱ�ȭ
		PxSceneDesc sceneDesc(m_pPxPhysicsSDK->getTolerancesScale()); // Scene�� ������ Ŭ����
		sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f); // �߷� ��

		if (!sceneDesc.cpuDispatcher)
		{
			PxDefaultCpuDispatcher* pCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
			// CPU Task �������� �⺻ ���� (�⺻ ������ ���� �׸��� �ʱ�ȭ)
			sceneDesc.cpuDispatcher = pCpuDispatcher;
		}

		if (!sceneDesc.filterShader)
			sceneDesc.filterShader = PxDefaultSimulationFilterShader;
		// ������ ���� ���̴� ����

		m_pPxScene = m_pPxPhysicsSDK->createScene(sceneDesc); // �� ����

		m_pPxControllerManager = PxCreateControllerManager(*m_pPxScene); // ��Ʈ�ѷ� ����
		//m_pPxControllerManager->setTessellation(true, 10);

																		 // Physx Visual Debugger
		if (NULL == m_pPxPhysicsSDK->getPvdConnectionManager())
		{
			return;
		}
		PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();
		m_pPxPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
		m_pPVDConnection = PxVisualDebuggerExt::createConnection(m_pPxPhysicsSDK->getPvdConnectionManager(), "127.0.0.1", 5425, 1000, connectionFlags);
		// PVD ����
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

