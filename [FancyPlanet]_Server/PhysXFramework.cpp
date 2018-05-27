//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "PhysXFramework.h"
CPhysXFramework::CPhysXFramework()
{
}

CPhysXFramework::~CPhysXFramework()
{
}

bool CPhysXFramework::OnCreate()
{
	InitializePhysxEngine();

	return(true);
}

void CPhysXFramework::OnDestroy()
{
	ReleasePhysxEngine();
}

void CPhysXFramework::BuildObjects()
{

}


void CPhysXFramework::FrameAdvance()
{
	// PhysX Update
	if (m_pPxScene)
	{
		m_pPxScene->simulate(1 / 10.f);
		m_pPxScene->fetchResults(true);
	}

	//m_pPxPhysicsSDK->getVisualDebugger()->updateCamera("PhysXCamera",
	//	PxVec3(m_pPlayer->GetCamera()->GetPosition().x, m_pPlayer->GetCamera()->GetPosition().y, m_pPlayer->GetCamera()->GetPosition().z),
	//	PxVec3(m_pPlayer->GetCamera()->GetUpVector().x, m_pPlayer->GetCamera()->GetUpVector().y, m_pPlayer->GetCamera()->GetUpVector().z),
	//	PxVec3(
	//		m_pPlayer->GetCamera()->GetPosition().x + m_pPlayer->GetLookVector().x,
	//		m_pPlayer->GetCamera()->GetPosition().y + m_pPlayer->GetLookVector().y,
	//		m_pPlayer->GetCamera()->GetPosition().z + m_pPlayer->GetLookVector().z
	//	));
}

void CPhysXFramework::InitializePhysxEngine()
{
	m_pPxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_PxDefaultAllocatorCallback, m_PxDefaultErrorCallback);
	// �Ŀ�̼� Ŭ������ �ν��Ͻ��� �ۼ�

	PxTolerancesScale PxScale = PxTolerancesScale();
	// �ùķ��̼��� ����Ǵ� �������� �����ϴ� Ŭ����
	// �ùķ��̼� �������� �����Ϸ��� ����� �⺻ �߷� ���� �����Ϸ��� ���̵�, �������� �ùķ��̼ǿ����� bounceThreshold�� �����ؾ��� ���� ����.

	//if (FAILED(m_pPxPhysicsSDK == NULL))
	//{
	//	MSG_BOX(L"PhysicsSDK Initialize Failed");
	//	// ���и޼��� �ʿ��ϸ� �ֱ�
	//}

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