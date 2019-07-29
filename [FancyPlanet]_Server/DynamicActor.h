#pragma once
#include "Object.h"

class DynamicActor : CGameObject {
protected:
	PxRigidDynamic* m_pPxActor;

public:
	DynamicActor();
	~DynamicActor();

	virtual void	BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial,
		XMFLOAT3 vScale, XMFLOAT3 vPosition, PxCooking* pCooking, const char* name);
	void SetPosition(XMFLOAT3 & xmf3Position);
	XMFLOAT3 GetPosition();
	void SetRotation(XMFLOAT4 & pxmf4Quaternion);
	XMFLOAT4 GetRotation();
};

class BulletActor : DynamicActor {

};

class RockActor : DynamicActor {
public:
	RockActor();
	~RockActor();
	void BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial,
		float fScale, XMFLOAT3 vPosition, PxCooking* pCooking, const char* name);
};