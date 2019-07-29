#pragma once
#include "Object.h"

class StaticActor : CGameObject {
protected:
	PxRigidStatic* m_pPxActor;
public:
	StaticActor();
	~StaticActor();

	virtual void	BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial,
		XMFLOAT3 vScale, XMFLOAT3 vPosition, PxCooking* pCooking, const char* name);
	void SetPosition(XMFLOAT3 & xmf3Position);
	XMFLOAT3 GetPosition();
	void SetRotation(XMFLOAT4 & pxmf4Quaternion);
	XMFLOAT4 GetRotation();
};

class Station : StaticActor {
public:
	Station();
	~Station();

	virtual void BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial,
		XMFLOAT3 vScale, XMFLOAT3 vPosition, PxCooking* pCooking, const char* name);
};