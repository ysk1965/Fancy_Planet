#include "stdafx.h"
#include "Scene.h"
#include "Object.h"
#include "CCloth.h"

CScene::CScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking)
	: m_pPxPhysicsSDK(pPxPhysicsSDK)
	, m_pPxScene(pPxScene)
	, m_pPxControllerManager(pPxControllerManager)
	, m_pPxMaterial(NULL)
	, m_pCooking(pCooking)
{
}

CScene::~CScene()
{
}

void CScene::BuildObject()
{
}


void CScene::ReleaseObjects()
{
}

//////////////////////////////////////
//									//
//			TerrainScene			//
//									//
//////////////////////////////////////


PhysXScene::PhysXScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking)
{
	m_pPxPhysicsSDK = pPxPhysicsSDK;
	m_pPxScene = pPxScene;
	m_pPxControllerManager = pPxControllerManager;
	m_pCooking = pCooking;
}

void PhysXScene::BuildObject()
{
	//PxMaterial
	m_pPxMaterial = m_pPxPhysicsSDK->createMaterial(0.5f, 0.5f, 0.2f); //정지 마찰계수 운동마찰계수, 반발계수
											   //Plane바닥 생성X
	PxRigidStatic* groundPlane = PxCreatePlane(*m_pPxPhysicsSDK, PxPlane(0, 1, 0, 0), *m_pPxMaterial);
	groundPlane->setName("Ground");
	m_pPxScene->addActor(*groundPlane);

	PxRigidStatic* Ceiling = PxCreatePlane(*m_pPxPhysicsSDK, PxPlane(0, -1, 0, 0), *m_pPxMaterial);
	Ceiling->setGlobalPose(PxTransform(0,1000,0,PxQuat(Ceiling->getGlobalPose().q)));
	groundPlane->setName("Ceiling");
	m_pPxScene->addActor(*Ceiling);


	PxRigidStatic* Fence1 = PxCreatePlane(*m_pPxPhysicsSDK, PxPlane(-1, 0, 0, 0), *m_pPxMaterial);
	XMFLOAT4 m_xmf4MakeFence;
	XMVECTOR v = XMQuaternionRotationRollPitchYaw(0, 0, 90.f);
	XMStoreFloat4(&m_xmf4MakeFence, v);
	Fence1->setGlobalPose(PxTransform(0, 500 , 1028, PxQuat(PxReal(m_xmf4MakeFence.y), PxVec3(0, 1, 0))));
	groundPlane->setName("Fence1");
	//m_pPxScene->addActor(*Fence1);

	PxRigidStatic* Fence2 = PxCreatePlane(*m_pPxPhysicsSDK, PxPlane(-1, 0, 0,2056), *m_pPxMaterial);
	XMFLOAT4 m_xmf4MakeFence2;
	XMVECTOR v2 = XMQuaternionRotationRollPitchYaw(0, 0, 90.f);
	XMStoreFloat4(&m_xmf4MakeFence2, v2);
	Fence2->setGlobalPose(PxTransform(2056, 500, 1028, PxQuat(PxReal(m_xmf4MakeFence2.y), PxVec3(0, 1, 0))));
	groundPlane->setName("Fence2");
	//m_pPxScene->addActor(*Fence2);

	m_pPlayer = new CPlayer[MAX_USER];

	// Terrain생성
	XMFLOAT3 xmf3Scale(8.0f, 3.0f, 8.0f);
	m_pTerrain = new CHeightMapTerrain(_T("HeightMap2.raw"),
		257, 257, 257, 257, xmf3Scale, XMFLOAT4(0, 0, 0, 0), m_pPxPhysicsSDK, m_pPxScene,
		m_pPxControllerManager, m_pCooking);

	for(int i=0; i<MAX_USER; ++i)
		m_pPlayer[i].setTerrain(m_pTerrain);
	//Object Test
	//CGameObject a[100];
	//StaticActor a[125];
	//DynamicActor b[125];

	//for (int i = 0; i < 1; i++) {
	//   for (int j = 0; j < 1; j++) {
	//      for (int k = 0; k < 1; k++) {
	//         a[i*j].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(5, 5, 5),
	//            XMFLOAT3(-i * 10, -45 + k * 10, -j * 10), m_pCooking, "Test");
	//      }
	//   }
	//}


	RockActor c[OBJECTS_NUMBER];
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 6; j++) {
			c[i*j].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, 6,
				XMFLOAT3(719 + (i * 137), 300, 930 + (j * 103)), m_pCooking, "Test");
		}
	}

	Station station[3];

	station[0].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(40, 40, 40),
		XMFLOAT3(830, 125, 500), m_pCooking, "Station1");
	station[1].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(40, 40, 40),
		XMFLOAT3(1540, 140, 1440), m_pCooking, "Station2");
	station[2].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(40, 40, 40),
		XMFLOAT3(430, 125, 1490), m_pCooking, "Station3");


	Station Dom[3];

	Dom[0].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(70, 3, 70),
		XMFLOAT3(650, 0, 980), m_pCooking, "Dom1");							
	Dom[1].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(70, 3, 70),
		XMFLOAT3(1235, 0, 860), m_pCooking, "Dom2");						
	Dom[2].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(70, 3, 70),
		XMFLOAT3(980, 0, 1400), m_pCooking, "Dom3");

	strcpy(base[0].name, "Dom1");
	base[0].m_xmf3Size = XMFLOAT3(70, 3, 70);
	base[0].m_iState = NEUTRAL;
	base[0].m_xmf3Position = XMFLOAT3(650, 0, 980);

	strcpy(base[1].name, "Dom2");
	base[1].m_xmf3Size = XMFLOAT3(70, 3, 70);
	base[1].m_iState = NEUTRAL;
	base[1].m_xmf3Position = XMFLOAT3(1235, 0, 860);


	strcpy(base[2].name, "Dom3");
	base[2].m_xmf3Size = XMFLOAT3(70, 3, 70);
	base[2].m_iState = NEUTRAL;
	base[2].m_xmf3Position = XMFLOAT3(900, 0, 1400);


	Station Background[10];
	Background[0].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(430, 90, 700), m_pCooking, "Background1");			
	Background[1].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(700, 60, 1210), m_pCooking, "Background2");					
	Background[2].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(600, 110, 400), m_pCooking, "Background3");					
	Background[3].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(230, 125, 1500), m_pCooking, "Background4");					
	Background[4].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(890, 115, 250), m_pCooking, "Background5");					
	Background[5].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(1400, 70, 300), m_pCooking, "Background6");					
	Background[6].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(1220, 70, 1350), m_pCooking, "Background7");					
	Background[7].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(1520, 55, 350), m_pCooking, "Background8");					
	Background[8].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(1650, 75, 600), m_pCooking, "Background9");					  
	Background[9].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(20,20,20),
		XMFLOAT3(1675, 110, 1290), m_pCooking, "Background10");

	CCloth cloth[1];

	cloth[0].BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(1.0f, 1.0f, 1.0f), m_pCooking, "PhysicsCloth");
	cloth[0].ClothSetPosition(XMFLOAT3(694, 20, 1017));
	cloth[0].ClothSetRotate(XMFLOAT3((float)D3DXToRadian(0.f), (float)D3DXToRadian(0.f), (float)D3DXToRadian(0.f)));
	cloth[0].SetWind(PxVec3(1.0f, 0.1f, 0.0f), 40.0f, PxVec3(0.0f, 10.0f, 10.0f));

	m_pCloth = &cloth[0];

	Team[BLUETeam].m_iTeam = BLUETeam;
	Team[REDTeam].m_iTeam = REDTeam;
	Team[GREENTeam].m_iTeam = GREENTeam;
}
void PhysXScene::update(const float timeelapsed)
{
	m_pCloth->Update(timeelapsed);

}
