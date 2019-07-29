#include "StaticActor.h"

StaticActor::StaticActor()
{

}

StaticActor::~StaticActor()
{

}

void StaticActor::BuildObject(PxPhysics * pPxPhysics, PxScene * pPxScene, PxMaterial * pPxMaterial,
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

	//m_pPxActor = PxCreateDynamic(*pPxPhysics, _PxTransform, _PxBoxGeometry, *pPxMaterial, 10.0f);

	m_pPxActor = pPxPhysics->createRigidStatic(_PxTransform);
	m_pPxActor->setName(name);
	PxShape* hafshape = m_pPxActor->createShape(_PxBoxGeometry, *pPxMaterial);

	//m_pPxActor->createShape(_PxBoxGeometry, pPxMaterial);



	pPxScene->addActor(*m_pPxActor);


	//m_pPxActor->setRigidDynamicFlag(PxRigidBodyFlag::eENABLE_CCD, true);
}

void StaticActor::SetPosition(XMFLOAT3 & xmf3Position)
{
	PxTransform PxTrans = m_pPxActor->getGlobalPose();
	PxTrans.p = PxVec3(xmf3Position.x, xmf3Position.y, xmf3Position.z);
	m_pPxActor->setGlobalPose(PxTrans);
}

XMFLOAT3 StaticActor::GetPosition()
{
	return XMFLOAT3(m_xmf3Position);
}

void StaticActor::SetRotation(XMFLOAT4& pxmf4Quaternion)
{
	PxTransform PxTransform = m_pPxActor->getGlobalPose();

	PxTransform.q *= PxQuat(pxmf4Quaternion.x, PxVec3(1, 0, 0));
	PxTransform.q *= PxQuat(pxmf4Quaternion.z, PxVec3(0, 1, 0));
	PxTransform.q *= PxQuat(pxmf4Quaternion.y, PxVec3(0, 0, 1));

	m_pPxActor->setGlobalPose(PxTransform);
}

XMFLOAT4 StaticActor::GetRotation()
{
	return XMFLOAT4(m_xmf4Quaternion);
}

Station::Station()
{
}

Station::~Station()
{
}

void Station::BuildObject(PxPhysics * pPxPhysics, PxScene * pPxScene, PxMaterial * pPxMaterial, XMFLOAT3 vScale, XMFLOAT3 vPosition, PxCooking * pCooking, const char * name)
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

	m_pPxActor = pPxPhysics->createRigidStatic(_PxTransform);
	m_pPxActor->setName(name);
	PxShape* hafshape = m_pPxActor->createShape(_PxBoxGeometry, *pPxMaterial);
	pPxScene->addActor(*m_pPxActor);
}