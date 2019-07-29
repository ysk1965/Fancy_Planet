#pragma once
#include "Player.h"
#include "Object.h"
#include "StaticActor.h"
#include "DynamicActor.h"
#include "CCloth.h"
extern BASE base[STATION_NUMBER];
extern TEAM Team[Team_NUMBER];
class CScene
{
public:
	CScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	CScene() {};
	~CScene();

	void BuildObject();
	void ReleaseObjects();

	CPlayer* m_pPlayer;
protected:
	PxPhysics * m_pPxPhysicsSDK;
	PxScene*				m_pPxScene;
	PxControllerManager*	m_pPxControllerManager;
	PxMaterial*				m_pPxMaterial;
	PxCooking*				m_pCooking;
};

class PhysXScene : public CScene
{
public:
	PhysXScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	virtual void BuildObject();
	void update(const float timeelapsed);
	CCloth* m_pCloth;
private:
	CHeightMapTerrain * m_pTerrain = NULL;
};