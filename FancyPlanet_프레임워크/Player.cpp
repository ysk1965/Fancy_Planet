//-----------------------------------------------------------------------------
// File: CPlayer.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Player.h"
#include "Shader.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CPlayer

CPlayer::CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, void *pContext, int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pCamera = new CCamera;

	if (m_pCamera) 
		m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	// Px
	m_fFallAcceleration = 4.0f;

	//CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	//SetPosition(XMFLOAT3(0, pTerrain->GetHeight(200, 300), 0));
	//SetPlayerUpdatedContext(pTerrain);
	//SetCameraUpdatedContext(pTerrain);
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) 
		delete m_pCamera;
}

void CPlayer::ReleaseShaderVariables()
{
	if (m_pCamera) 
		m_pCamera->ReleaseShaderVariables();

	if (m_pd3dcbPlayer)
	{
		m_pd3dcbPlayer->Unmap(0, NULL);
		m_pd3dcbPlayer->Release();
	}

	CGameObject::ReleaseShaderVariables();
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedPlayer->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbPlayer->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(ROOT_PARAMETER_PLAYER, d3dGpuVirtualAddress);
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity) // 1st
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		//m_pPxCharacterController->move(PxVec3(xmf3Shift.x, xmf3Shift.y, xmf3Shift.z) * m_fSpeed * m_fTimeDelta, 0, m_fTimeDelta, PxControllerFilters()); //Pxplayer Move
		m_pPxCharacterController->move(PxVec3(xmf3Shift.x, xmf3Shift.y, xmf3Shift.z) * m_fSpeed, 0, NULL, PxControllerFilters()); //Pxplayer Move
		//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		//m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +35.0f)
			{
				x -= (m_fPitch - 35.0f);
				m_fPitch = +35.0f;
			}
			if (m_fPitch < -70.0f)
			{
				x -= (m_fPitch + 70.0f);
				m_fPitch = -70.0f;
			}
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f)
				m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f)
				m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f)
			{
				z -= (m_fRoll - 20.0f);
				m_fRoll = +20.0f;
			}
			if (m_fRoll < -20.0f)
			{
				z -= (m_fRoll + 20.0f);
				m_fRoll = -20.0f;
			}
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	if ((nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) 
			{
				x -= (m_fPitch - 89.0f); 
				m_fPitch = +89.0f;
			}
			if (m_fPitch < -89.0f)
			{
				x -= (m_fPitch + 89.0f);
				m_fPitch = -89.0f; 
			}
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f)
				m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f)
				m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) 
			{
				z -= (m_fRoll - 20.0f);
				m_fRoll = +20.0f; 
			}
			if (m_fRoll < -20.0f)
			{
				z -= (m_fRoll + 20.0f);
				m_fRoll = -20.0f;
			}
		}
		m_pCamera->Rotate(x, y, z);
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

void CPlayer::Update(float fTimeElapsed)
{
	//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));
	//float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	//float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	//if (fLength > m_fMaxVelocityXZ)
	//{
	//	m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
	//	m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	//}
	//float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	//fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	//if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//Move(m_xmf3Velocity, false);

	//if (m_pPlayerUpdatedContext)
	//	OnPlayerUpdateCallback(fTimeElapsed);

	////		Camera		//
	//DWORD nCurrentCameraMode = m_pCamera->GetMode();

	//if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
	//	m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	//if (m_pCameraUpdatedContext)
	//	OnCameraUpdateCallback(fTimeElapsed);
	//if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
	//	m_pCamera->SetLookAt(m_xmf3Position);

	//m_pCamera->RegenerateViewMatrix();
	////		Camera		//

	//fLength = Vector3::Length(m_xmf3Velocity);

	//float fDeceleration = (m_fFriction * fTimeElapsed);
	//if (fDeceleration > fLength)
	//	fDeceleration = fLength;
	//m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));

	//////////////////////////////////////////////////////////////
	//															//
	//							PhysX							//
	//															//
	//////////////////////////////////////////////////////////////

	m_fTimeDelta = fTimeElapsed;
	
	//PhysX�� ���� �������ش�. �߷�
	m_fFallvelocity -= m_fFallAcceleration * fTimeElapsed * 50.f;
	
	if (m_fFallvelocity < -1000.f)
		m_fFallvelocity = -1000.f;
	
	m_pPxCharacterController->move(PxVec3(0, 1.f, 0) * fTimeElapsed * m_fFallvelocity, 0, fTimeElapsed, PxControllerFilters());
	
	PxControllerState   m_pPxState;
	
	//������ ��ü�� ���°��� m_pPxState�� �־��ش�.
	m_pPxCharacterController->getState(m_pPxState);
	
	//���� �浹�ϰų� �Ʒ��� �浹�ϸ� m_fFallvelocity = 0.0f
	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN ||
		m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_UP)
		m_fFallvelocity = 0.f;
	
	////���� PhysX�� ������ ��ü�� ��������� ������ش�.
	m_xmf3Position = XMFLOAT3((float)m_pPxCharacterController->getFootPosition().x, (float)m_pPxCharacterController->getFootPosition().y, (float)m_pPxCharacterController->getFootPosition().z); //��Ʈ�ѷ��� �� ��ġ CCT ����� �Ʒ� �κ�
	
	float m_fRevice = -100.0f; //Player�� Y������(���� ���鿡 �ȹ����� ����)
	
	XMMATRIX matTrans = XMMatrixTranslation(m_xmf3Position.x, m_xmf3Position.y + m_fRevice, m_xmf3Position.z);
	
	XMMATRIX matRotX = XMMatrixRotationX((float)D3DXToRadian(m_fPitch));
	XMMATRIX matRotY = XMMatrixRotationY((float)D3DXToRadian(m_fYaw));
	XMMATRIX matRotZ = XMMatrixRotationZ((float)D3DXToRadian(m_fRoll));
	
	XMStoreFloat4x4(&m_xmf4x4World, matRotX * matRotY * matRotZ * matTrans);
	
	DWORD nCurrentCameraMode = m_pCamera->GetMode();
	
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA || FIRST_PERSON_CAMERA)
		m_pCamera->Update(m_xmf3Position, fTimeElapsed);
	if (m_pCameraUpdatedContext)
		OnCameraUpdateCallback(fTimeElapsed);
	if (nCurrentCameraMode == THIRD_PERSON_CAMERA)
		m_pCamera->SetLookAt(m_xmf3Position);
	
	m_pCamera->RegenerateViewMatrix();
}

CCamera *CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	}
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));

		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera)
		delete m_pCamera;

	return(pNewCamera);
}

CCamera *CPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;

	if (nCurrentCameraMode == nNewCameraMode)
		return(m_pCamera);

	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, -400.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -0.1f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 15.0f, -30.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
		break;
	default:
		break;
	}
	Update(fTimeElapsed);

	return(m_pCamera);
}

void CPlayer::Animate(float fTimeElapsed)
{
	//	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f);
	//	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

void CPlayer::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//if (nCameraMode == THIRD_PERSON_CAMERA) CGameObject::Render(pd3dCommandList, pCamera);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

void CPlayer::BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial, PxControllerManager *pPxControllerManager, void* pContext)
{
	m_pScene = pPxScene;

	PxCapsuleControllerDesc	PxCapsuledesc;

	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)pContext;
	SetPlayerUpdatedContext(pTerrain);
	SetCameraUpdatedContext(pTerrain);
	PxCapsuledesc.position = PxExtendedVec3(0, pTerrain->GetHeight(200, 300), 0);

	PxCapsuledesc.radius = 6.0f;
	PxCapsuledesc.height = 9.0f;
	//ĳ���Ͱ� �ö� ���ִ� ��ֹ��� �ִ� ���̸� �����մϴ�. 
	PxCapsuledesc.stepOffset = 10.f;

	//ĳ�� �� ���� ����.
	//������ ����Ű�� ���� ĳ���ϴ� ��Ʈ�ѷ� �ֺ��� �����Դϴ�.  
	//�̰��� 1.0f���� Ŀ�������� �ʹ� ũ�� �ʾƾ��ϸ�, 2.0f���� ���ƾ��մϴ�.
	PxCapsuledesc.volumeGrowth = 1.9f;
	//ĳ���Ͱ� �ɾ� �� ���ִ� �ִ� ���. 
	PxCapsuledesc.slopeLimit = cosf(XMConvertToRadians(15.f));
	//PxCapsuledesc.nonWalkableMode = PxControllerNonWalkableMode::eFORCE_SLIDING;
	PxCapsuledesc.upDirection = PxVec3(0, 1, 0);
	PxCapsuledesc.contactOffset = 0.001f; // ���� ����
	pPxMaterial = pPxPhysics->createMaterial(0.5f, 0.5f, 0.5f);
	PxCapsuledesc.material = pPxMaterial;
	PxCapsuledesc.behaviorCallback = this;
	PxCapsuledesc.reportCallback = this;

	//if (pPxControllerManager->createController(PxCapsuledesc) == NULL)
	//{
	//	MSG_BOX(L"PhysicsSDK Initialize Failed");
	//	// ���и޼��� �ʿ��ϸ� �ֱ�
	//}

	m_pPxCharacterController = pPxControllerManager->createController(PxCapsuledesc);

	m_pCamera = ChangeCamera(SPACESHIP_CAMERA, 0.0f);
}

bool CPlayer::IsOnGround(void)
{
	PxControllerState   m_pPxState;

	//������ ��ü�� ���°��� m_pPxState�� �־��ش�.
	m_pPxCharacterController->getState(m_pPxState);

	if (m_pPxState.collisionFlags == PxControllerCollisionFlag::eCOLLISION_DOWN)
		return true;
	return false;
}

//void CPlayer::SetPxPosition(XMFLOAT3 vPosition)
//{
//	m_pPxCharacterController->setFootPosition(PxExtendedVec3(vPosition.x, vPosition.y, vPosition.z));
//}

void CPlayer::onShapeHit(const PxControllerShapeHit & hit)
{
	PxRigidDynamic* actor = hit.shape->getActor()->is<PxRigidDynamic>();

	if (actor)
	{
		if (actor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
			return;

		const PxVec3 upVector = hit.controller->getUpDirection();
		const PxF32 dp = hit.dir.dot(upVector);

		if (fabsf(dp) < 1e-3f)
		{
			const PxTransform globalPose = actor->getGlobalPose();
			const PxVec3 localPos = globalPose.transformInv(toVec3(hit.worldPos));
		}
	}
}

void CPlayer::AddForceAtLocalPos(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup)
{
	//transform pos to world space
	const PxVec3 globalForcePos = body.getGlobalPose().transform(pos);

	AddForceAtPosInternal(body, force, globalForcePos, mode, wakeup);
}

void CPlayer::AddForceAtPosInternal(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup)
{
	const PxTransform globalPose = body.getGlobalPose();
	const PxVec3 centerOfMass = globalPose.transform(body.getCMassLocalPose().p);

	const PxVec3 torque = (pos - centerOfMass).cross(force);
	body.addForce(force, mode, wakeup);
	body.addTorque(torque, mode, wakeup);
}

//�������� ���� �÷��׸� �˻��ϴ� �Լ�
//CCT�� ��翡 ������ CCT�� ������ ����ڰ� ����� ���� �� �� �ֽ��ϴ�. 
//�� �Լ��� ���ϴ� ������ ���ϴ� PxControllerBehaviorFlag�÷��׸� �˻��Ѵ�.
PxControllerBehaviorFlags CPlayer::getBehaviorFlags(const PxShape& shape, const PxActor& actor)
{
	const char* actorName = actor.getName();

	if (actor.getType() == PxActorType::eRIGID_DYNAMIC)
		//return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | PxControllerBehaviorFlag::eCCT_SLIDE;
		return PxControllerBehaviorFlag::eCCT_SLIDE;

	return PxControllerBehaviorFlags(0);
}

PxControllerBehaviorFlags CPlayer::getBehaviorFlags(const PxController &)
{
	return PxControllerBehaviorFlags(0);
}

PxControllerBehaviorFlags CPlayer::getBehaviorFlags(const PxObstacle &)
{
	return PxControllerBehaviorFlags(0);
}

void CPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	CHeightMapTerrain *pTerrain = (CHeightMapTerrain *)m_pPlayerUpdatedContext;
	XMFLOAT3 xmf3Scale = pTerrain->GetScale();
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	int z = (int)(xmf3PlayerPosition.z / xmf3Scale.z);
	bool bReverseQuad = ((z % 2) != 0);
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z, bReverseQuad);
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}

void CPlayer::PxMove(float speed, float fTimeElapsed)
{
	m_pPxCharacterController->move(PxVec3(m_xmf3Velocity.x, m_xmf3Velocity.y, m_xmf3Velocity.z) * speed * fTimeElapsed * 1.4f, 0, fTimeElapsed, PxControllerFilters());
}