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
	// 파운데이션 클래스의 인스턴스를 작성

	PxTolerancesScale PxScale = PxTolerancesScale();
	// 시뮬레이션이 실행되는 스케일을 정의하는 클래스
	// 시뮬레이션 스케일을 변경하려면 장면의 기본 중력 값을 변경하려는 것이됨, 안정적인 시뮬레이션에서는 bounceThreshold를 변경해야할 수도 있음.

	//if (FAILED(m_pPxPhysicsSDK == NULL))
	//{
	//	MSG_BOX(L"PhysicsSDK Initialize Failed");
	//	// 실패메세지 필요하면 넣기
	//}

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