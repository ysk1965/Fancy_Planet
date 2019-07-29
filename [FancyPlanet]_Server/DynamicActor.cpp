#include "DynamicActor.h"

DynamicActor::DynamicActor()
{

}

DynamicActor::~DynamicActor()
{

}

void DynamicActor::BuildObject(PxPhysics * pPxPhysics, PxScene * pPxScene, PxMaterial * pPxMaterial,
	XMFLOAT3 vScale, XMFLOAT3 vPosition, PxCooking * pCooking, const char * name)
{
	//바운딩박스
	XMFLOAT3 vMin = (XMFLOAT3(1, 1, 1));
	vMin.x *= vScale.x;	 vMin.y *= vScale.y;  vMin.z *= vScale.z;

	XMFLOAT3 vMax = (XMFLOAT3(1, 1, 1));
	vMax.x *= vScale.x;	 vMax.y *= vScale.y;  vMax.z *= vScale.z;

	XMFLOAT3 _d3dxvExtents = // 피직스 범위
		XMFLOAT3((abs(vMin.x) + abs(vMax.x)) / 2, (abs(vMin.y) + abs(vMax.y)) / 2, (abs(vMin.z) + abs(vMax.z)) / 2);

	PxTransform _PxTransform(vPosition.x, vPosition.y, vPosition.z);

	PxBoxGeometry _PxBoxGeometry(_d3dxvExtents.x, _d3dxvExtents.y, _d3dxvExtents.z);

	m_pPxActor = PxCreateDynamic(*pPxPhysics, _PxTransform, _PxBoxGeometry, *pPxMaterial, 10.0f);

	m_pPxActor->setMass(4.f);
	m_pPxActor->setName(name);
	m_pPxActor->setAngularDamping(1000);
	m_pPxActor->setLinearDamping(1000);

	//_PxTransform.p.y = vPosition.y - 1;
	//m_pPxActor->setCMassLocalPose(_PxTransform);
	PxShape* hafshape = m_pPxActor->createShape(_PxBoxGeometry, *pPxMaterial);

	//m_pPxActor->createShape(_PxBoxGeometry, pPxMaterial);

	pPxScene->addActor(*m_pPxActor);


	//m_pPxActor->setRigidDynamicFlag(PxRigidBodyFlag::eENABLE_CCD, true);
}

void DynamicActor::SetPosition(XMFLOAT3 & xmf3Position)
{
	PxTransform PxTrans = m_pPxActor->getGlobalPose();
	PxTrans.p = PxVec3(xmf3Position.x, xmf3Position.y, xmf3Position.z);
	m_pPxActor->setGlobalPose(PxTrans);
}

XMFLOAT3 DynamicActor::GetPosition()
{
	if (m_pPxActor != NULL) {
		XMFLOAT3 PxPos;
		PxPos.x = m_pPxActor->getGlobalPose().p.x;
		PxPos.y = m_pPxActor->getGlobalPose().p.y;
		PxPos.z = m_pPxActor->getGlobalPose().p.z;

		return PxPos;
	}
}

void DynamicActor::SetRotation(XMFLOAT4& pxmf4Quaternion)
{
	PxTransform PxTransform = m_pPxActor->getGlobalPose();

	PxTransform.q *= PxQuat(pxmf4Quaternion.x, PxVec3(1, 0, 0));
	PxTransform.q *= PxQuat(pxmf4Quaternion.z, PxVec3(0, 1, 0));
	PxTransform.q *= PxQuat(pxmf4Quaternion.y, PxVec3(0, 0, 1));

	m_pPxActor->setGlobalPose(PxTransform);
}

XMFLOAT4 DynamicActor::GetRotation()
{
	return XMFLOAT4(m_xmf4Quaternion);
}

RockActor::RockActor()
{
}

RockActor::~RockActor()
{
}

void RockActor::BuildObject(PxPhysics * pPxPhysics, PxScene * pPxScene, PxMaterial * pPxMaterial, float fScale, XMFLOAT3 vPosition, PxCooking * pCooking, const char * name)
{
	PxTransform _PxTransform(vPosition.x, vPosition.y, vPosition.z);

	//PxSphereGeometry _PxSphereGeomeyry(fScale);

	//m_pPxActor = PxCreateDynamic(*pPxPhysics, _PxTransform, _PxSphereGeomeyry, *pPxMaterial, 10.0f);




	////바운딩박스
	//XMFLOAT3 vMin = (XMFLOAT3(1, 1, 1));
	//vMin.x *= fScale;	 vMin.y *= fScale;  vMin.z *= fScale;

	//XMFLOAT3 vMax = (XMFLOAT3(1, 1, 1));
	//vMax.x *= fScale;	 vMax.y *= fScale;  vMax.z *= fScale;

	//XMFLOAT3 _d3dxvExtents = // 피직스 범위
	//	XMFLOAT3((abs(vMin.x) + abs(vMax.x)) / 2, (abs(vMin.y) + abs(vMax.y)) / 2, (abs(vMin.z) + abs(vMax.z)) / 2);

	//PxTransform _PxTransform(vPosition.x, vPosition.y, vPosition.z);

	//PxBoxGeometry _PxBoxGeometry(_d3dxvExtents.x, _d3dxvExtents.y, _d3dxvExtents.z);

	PxSphereGeometry _PxSphereGeometry(fScale);

	m_pPxActor = PxCreateDynamic(*pPxPhysics, _PxTransform, _PxSphereGeometry, *pPxMaterial, 10.0f);

	m_pPxActor->setMass(4.f);
	m_pPxActor->setName(name);
	m_pPxActor->setAngularDamping(0.5f);
	m_pPxActor->setLinearDamping(0.5f);
	m_pPxActor->setSleepThreshold(50);
	// PxShape* hafshape = m_pPxActor->createShape(_PxBoxGeometry, *pPxMaterial);

	//m_pPxActor->createShape(_PxBoxGeometry, pPxMaterial);

	pPxScene->addActor(*m_pPxActor);


	//m_pPxActor->setRigidDynamicFlag(PxRigidBodyFlag::eENABLE_CCD, true);
}
