#pragma once
#include "Object.h"
class CCloth : public CGameObject
{
public:
	CCloth();
	~CCloth();

private:
	PxCloth					 *m_pCloth;
	PxVec3                   m_WindDir;
	PxVec3                   m_WindRange;
	PxReal                   m_WindStrength;
	PxReal                   m_Time;
	wstring					 m_strBufferName;
	int						 m_iVtxCount;
	float					 m_fWindCount;

public:
	virtual int Update(const float& fTimeDelta);
public:
	virtual void	BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial, XMFLOAT3 vScale, PxCooking* pCooking, const char* name);
	PxClothMeshDesc	CreateMeshGrid(PxVec3 dirU, PxVec3 dirV, PxU32 numU, PxU32 numV,
		vector<PxVec4>& vertices, vector<PxU32>& indices, vector<PxVec2>& texcoords);
	void SetWind(const PxVec3 &dir, PxReal strength, const PxVec3& range);
	PxF32 WindRand(PxF32 a, PxF32 b);

	void UpdateWind(const float& fTimeDelta);

	void ClothPhysXUpdate(const float & fTimeDelta);

	void ClothSetPosition(XMFLOAT3 vPosition);
	void ClothSetRotate(XMFLOAT3 vRot);

	void SetBufferName(wstring strName) { m_strBufferName = strName; }

};