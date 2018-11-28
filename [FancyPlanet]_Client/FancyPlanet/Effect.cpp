#include "stdafx.h"
#include "Effect.h"
#include "Object.h"
#include "Player.h"

Effect::Effect(UINT nMeshes, UINT nObjects) : m_nMeshes(nMeshes), m_nObjects(nObjects)
{
	m_pEffects = new EffectConstant[m_nObjects];
	m_pEffectsInfo = new EffectInfo[m_nObjects];
	m_ppMeshes = new CMesh*[m_nMeshes];
	m_xmf3Pos = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_fTime = 0.3f;
}

Effect::~Effect()
{
}

void Effect::SetMesh(CMesh* pMesh, UINT nMeshIndex)
{
	if (pMesh)
		m_ppMeshes[nMeshIndex] = pMesh;
}

void Effect::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(EffectConstant) + 255) & ~255); //256의 배수
	m_pd3dcbEffect = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects
		, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbEffect->Map(0, NULL, (void **)&m_pcbMappedEffect);
}

void Effect::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(EffectConstant) + 255) & ~255);

	for (int i = 0; i < m_nObjects; i++)
	{
		XMStoreFloat4x4(&m_pcbMappedEffect[i].m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_pEffects[i].m_xmf4x4World)));

		m_pcbMappedEffect[i].m_xmf4Emissive = m_pEffects[i].m_xmf4Emissive;
		m_pcbMappedEffect[i].m_xmf4Color = m_pEffects[i].m_xmf4Color;
		m_pcbMappedEffect[i].m_fAlpha = m_pEffects[i].m_fAlpha;
	}

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbEffect->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootShaderResourceView(0, d3dGpuVirtualAddress);
}

void Effect::SetParticlePosition(UINT nIndex, const XMFLOAT3& xmf3Position)
{
	m_pEffects[nIndex].m_xmf4x4World._41 = xmf3Position.x;
	m_pEffects[nIndex].m_xmf4x4World._42 = xmf3Position.y;
	m_pEffects[nIndex].m_xmf4x4World._43 = xmf3Position.z;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BillboardEffect::BillboardEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : Effect(nMeshes, nObjects)
{
	m_pPlayer = pPlayer;
	for (int i = 0; i < m_nObjects; i++)
		SetParticlePosition(i, XMFLOAT3(rand() % 4 - 8 + m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().y, rand() % 4 - 8 + +m_pPlayer->GetPosition().z));
}
BillboardEffect::~BillboardEffect()
{
}
void BillboardEffect::Animate(float fHeight, const DWORD& dwdTime, const unsigned long& nCurrentFrameRate)
{
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_pEffects[i].m_xmf4x4World._42 >= rand() % 10 + fHeight + 15)
		{
			SetParticlePosition(i, XMFLOAT3(rand() % 4, 0, rand() % 4));
		}
		m_pEffects[i].m_xmf4x4World._41 += (rand() % 10) / 10.0f - 0.5;
		m_pEffects[i].m_xmf4x4World._42 += (rand() % 10) / 10.0f;
		m_pEffects[i].m_xmf4x4World._43 += (rand() % 10) / 10.0f - 0.5;
	}
}
void BillboardEffect::SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& right)
{
	if (m_bEffectFlag)
	{
		// 카메라의 Right벡터를 넣어준다.
		for (int i = 0; i < m_nObjects; i++)
		{
			XMFLOAT3 xmf3Right = right;
			XMFLOAT3 xmf3Position(m_pEffects[i].m_xmf4x4World._41, m_pEffects[i].m_xmf4x4World._42, m_pEffects[i].m_xmf4x4World._43);
			//타겟에서 카메라위치를 빼서 카메라를 향하는 Look벡터를 만든다.
			XMFLOAT3 xmf3Look = Vector3::Subtract(xmf3Target, xmf3Position);
			//Right벡터와 Look벡터를 외적하여 Up벡터를 구한다.
			XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);

			m_pEffects[i].m_xmf4x4World._11 = xmf3Right.x, m_pEffects[i].m_xmf4x4World._12 = xmf3Right.y, m_pEffects[i].m_xmf4x4World._13 = xmf3Right.z;
			m_pEffects[i].m_xmf4x4World._21 = xmf3Up.x, m_pEffects[i].m_xmf4x4World._22 = xmf3Up.y, m_pEffects[i].m_xmf4x4World._23 = xmf3Up.z;
			m_pEffects[i].m_xmf4x4World._31 = xmf3Look.x, m_pEffects[i].m_xmf4x4World._32 = xmf3Look.y, m_pEffects[i].m_xmf4x4World._33 = xmf3Look.z;
		}
	}
}
void BillboardEffect::Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex)
{
	if (m_bEffectFlag)
	{
		UpdateShaderVariables(pd3dCommandList);

		if (m_ppMeshes)
		{
			if (m_ppMeshes[nMeshIndex])
				m_ppMeshes[nMeshIndex]->Render(pd3dCommandList, m_nObjects);
		}
	}
}

void BillboardEffect::Init(UINT nIndex, const DWORD& dwdTime, bool bflag)
{}
void BillboardEffect::SetParticle(UINT nType)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MeshEffect::MeshEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : Effect(nMeshes, nObjects)
{
	m_pPlayer = pPlayer;
	m_xmf3Target = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_bEffectFlag = false;

	m_fTime = 0.4f;
}
MeshEffect::~MeshEffect()
{
}
void MeshEffect::SetTarget(XMFLOAT3& xmf3Target)
{
	if (!m_bEffectFlag)
	{
		m_bEffectFlag = true;
		m_xmf3Target = xmf3Target;

		m_dwdLength = GetTickCount();
	}
};
void MeshEffect::Animate(const DWORD& dwdTime)
{
	if (m_bEffectFlag)
	{
		XMFLOAT3 xmf3Pos = m_pPlayer->GetPosition();

		m_pEffects[0].m_xmf4x4World._41 = (xmf3Pos.x + m_xmf3Target.x) / 2.0f;
		m_pEffects[0].m_xmf4x4World._42 = (xmf3Pos.y + 35 + m_xmf3Target.y) / 2.0f;
		m_pEffects[0].m_xmf4x4World._43 = (xmf3Pos.z + m_xmf3Target.z) / 2.0f;

		XMFLOAT3 xmf3Dir = Vector3::Normalize(Vector3::Subtract(m_xmf3Target, xmf3Pos));

		XMFLOAT3 xmf3Right;
		XMFLOAT3 xmf3Up;
		XMStoreFloat3(&xmf3Right, XMVector3Orthogonal(XMLoadFloat3(&xmf3Dir)));
		xmf3Up = Vector3::CrossProduct(xmf3Right, xmf3Dir);
		xmf3Up = Vector3::Normalize(xmf3Up);

		m_pEffects[0].m_xmf4x4World._11 = xmf3Right.x, m_pEffects[0].m_xmf4x4World._12 = xmf3Right.y, m_pEffects[0].m_xmf4x4World._13 = xmf3Right.z;
		m_pEffects[0].m_xmf4x4World._21 = xmf3Up.x, m_pEffects[0].m_xmf4x4World._22 = xmf3Up.y, m_pEffects[0].m_xmf4x4World._23 = xmf3Up.z;
		m_pEffects[0].m_xmf4x4World._31 = xmf3Dir.x, m_pEffects[0].m_xmf4x4World._32 = xmf3Dir.y, m_pEffects[0].m_xmf4x4World._33 = xmf3Dir.z;

		float fLength = Vector3::Length(Vector3::Subtract(m_xmf3Target, xmf3Pos));

		XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
		xmf4x4Scale._11 = m_pEffectsInfo[0].fScale, xmf4x4Scale._22 = m_pEffectsInfo[0].fScale, xmf4x4Scale._33 = fLength;
		m_pEffects[0].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[0].m_xmf4x4World);


		float fTime = (dwdTime - m_dwdLength) / 1000.0f;

		if (fTime > m_fTime)
		{
			m_bEffectFlag = false;
			return;
		}

		float fValue = (m_fTime - fTime) / m_fTime; // 0 ~ 1

		m_pEffects[0].m_xmf4Color = XMFLOAT4((1.0f - fValue) + 0.2f, fValue + 1.0f, fValue + 0.3f, 1);
		m_pEffectsInfo[0].fScale = -4.0f * fValue * (fValue - 1.0f) + 1.0f;
		m_pEffects[0].m_fAlpha = 0.3f;
	}
}
void MeshEffect::Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex)
{
	if (m_bEffectFlag)
	{
		UpdateShaderVariables(pd3dCommandList);

		if (m_ppMeshes)
		{
			if (m_ppMeshes[nMeshIndex])
				m_ppMeshes[nMeshIndex]->Render(pd3dCommandList, m_nObjects);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SparkEffect::SparkEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : BillboardEffect(nMeshes, nObjects, pPlayer)
{
	DWORD dwTime = GetTickCount();
	m_xmf3OriginPos = new XMFLOAT3[nObjects];

	for (int i = 0; i < m_nObjects; i++)
	{
		Init(i, dwTime, true);
	}
	m_fTime = 0.08f;
}
SparkEffect::~SparkEffect()
{}
void SparkEffect::SetParticle()
{
	if (!m_bEffectFlag)
	{
		m_dwdLength = GetTickCount();

		m_bEffectFlag = true;

		XMFLOAT3 xmf3Dir = m_pPlayer->GetLook();
		XMFLOAT3 xmf3Up = m_pPlayer->GetUp();
		XMFLOAT3 xmf3Right = m_pPlayer->GetRight();
		float fDirScalar = 6.0f;
		float fUpScalar = 16.0f;
		float fRightScalar = 1.2f;

		m_xmfDifferenc.x = xmf3Dir.x * fDirScalar + xmf3Up.x * fUpScalar + xmf3Right.x * fRightScalar;
		m_xmfDifferenc.y = xmf3Dir.y * fDirScalar + xmf3Up.y * fUpScalar + xmf3Right.y * fRightScalar;
		m_xmfDifferenc.z = xmf3Dir.z * fDirScalar + xmf3Up.z * fUpScalar + xmf3Right.z * fRightScalar;

		

		for (int i = 0; i < m_nObjects; i++)
		{
			Init(i, m_dwdLength, false);

			float randValue = ((rand() % 1000) / 100.0f) + 5.1f;

			XMFLOAT3 originalValue = xmf3Dir;

			originalValue.x /= randValue;
			originalValue.y /= randValue;
			originalValue.z /= randValue;

			m_pEffectsInfo[i].xmf3Acceleration = originalValue;
			m_pEffects[i].m_xmf4Emissive = XMFLOAT4(0.4f, 0.2f, 0.1f, 0.0f);
		}
	}
}
void SparkEffect::Init(UINT nIndex, const DWORD& dwdTime, bool bflag)
{
	m_pEffectsInfo[nIndex].xmf3Veclocity.x = (rand() % 100 - 50.0f) / 1000.0f;
	m_pEffectsInfo[nIndex].xmf3Veclocity.y = (rand() % 100 - 50.0f) / 1000.0f;
	m_pEffectsInfo[nIndex].xmf3Veclocity.z = (rand() % 100 - 50.0f) / 1000.0f;
	SetParticlePosition(nIndex, XMFLOAT3(rand() % 4 - 2 + m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().y, rand() % 4 - 2 + m_pPlayer->GetPosition().z));

	m_xmf3OriginPos[nIndex] = XMFLOAT3(0.0f, 0.0f, 0.0f);

	m_pEffectsInfo[nIndex].dwdTime = dwdTime;
	m_pEffectsInfo[nIndex].fTotalTime = ((rand() % 300) / 100) + 2.5f;
	m_pEffectsInfo[nIndex].fScale = ((rand() % 100) / 1000) + 0.095;
}
void SparkEffect::Animate(const DWORD& dwdTime, const unsigned long& nCurrentFrameRate)
{
	if (m_bEffectFlag)
	{
		if ((dwdTime - m_dwdLength) / 1000.0f > m_fTime)
		{
			m_bEffectFlag = false;
		}
		XMFLOAT3 xmf3Acceleration;
		xmf3Acceleration.x = ((rand() % 100 - 50.0f) / 10000.0f) / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.y = ((rand() % 100 - 50.0f) / 10000.0f) / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.z = ((rand() % 100 - 50.0f) / 10000.0f) / nCurrentFrameRate * AdjustmentValue;
		
		XMFLOAT3 pos;
		if (m_pPlayer->GetPlayer() == NULL)
			pos = m_pPlayer->GetPosition();
		else
			pos = m_pPlayer->GetPlayer()->GetPosition();

		for (int i = 0; i < m_nObjects; i++)
		{
			float fTime = (dwdTime - m_pEffectsInfo[i].dwdTime) / 1000.0f;

			m_pEffectsInfo[i].xmf3Acceleration = Vector3::Add(m_pEffectsInfo[i].xmf3Acceleration, xmf3Acceleration);
			m_pEffectsInfo[i].xmf3Veclocity = Vector3::Add(m_pEffectsInfo[i].xmf3Veclocity, m_pEffectsInfo[i].xmf3Acceleration);

			m_xmf3OriginPos[i].x += (m_pEffectsInfo[i].xmf3Veclocity.x / nCurrentFrameRate * AdjustmentValue);
			m_xmf3OriginPos[i].y += (m_pEffectsInfo[i].xmf3Veclocity.y / nCurrentFrameRate * AdjustmentValue);
			m_xmf3OriginPos[i].z += (m_pEffectsInfo[i].xmf3Veclocity.z / nCurrentFrameRate * AdjustmentValue);

			m_pEffects[i].m_xmf4x4World._41 = m_xmf3OriginPos[i].x + pos.x + m_xmfDifferenc.x;
			m_pEffects[i].m_xmf4x4World._42 = m_xmf3OriginPos[i].y + pos.y + m_xmfDifferenc.y;
			m_pEffects[i].m_xmf4x4World._43 = m_xmf3OriginPos[i].z + pos.z + m_xmfDifferenc.z;

			XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
			xmf4x4Scale._11 = m_pEffectsInfo[i].fScale, xmf4x4Scale._22 = m_pEffectsInfo[i].fScale, xmf4x4Scale._33 = m_pEffectsInfo[i].fScale;
			m_pEffects[i].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[i].m_xmf4x4World);

			float fValue = (m_fTime - fTime) / m_fTime;

			m_pEffects[i].m_xmf4Color = XMFLOAT4((1 - fValue) + 1.0f, (1 - fValue) + 0.3f, (1 - fValue) + 0.1f, 1);
			m_pEffects[i].m_fAlpha = -4.0f * fValue * (fValue - 1.0f);
		}
	}
}
void SparkEffect::Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex)
{
	if (m_bEffectFlag)
	{
		UpdateShaderVariables(pd3dCommandList);

		if (m_ppMeshes)
		{
			if (m_ppMeshes[nMeshIndex])
				m_ppMeshes[nMeshIndex]->Render(pd3dCommandList, m_nObjects);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SmokeEffect::SmokeEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : BillboardEffect(nMeshes, nObjects, pPlayer)
{
	DWORD dwTime = GetTickCount();

	for (int i = 0; i < m_nObjects; i++)
	{
		Init(i, dwTime);
	}
}
SmokeEffect::~SmokeEffect()
{
}
void SmokeEffect::Init(UINT nIndex, const DWORD& dwdTime)
{
}
void SmokeEffect::Animate(float fHeight, const DWORD& dwdTime, const unsigned long& nCurrentFrameRate)
{
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_pEffects[i].m_xmf4x4World._42 >= rand() % 10 + fHeight + 15)
		{
			SetParticlePosition(i, XMFLOAT3(rand() % 4, 0, rand() % 4));
		}
		m_pEffects[i].m_xmf4x4World._41 += (((rand() % 10) / 10.0f - 0.5) / nCurrentFrameRate);
		m_pEffects[i].m_xmf4x4World._42 += (((rand() % 10) / 10.0f) / nCurrentFrameRate);
		m_pEffects[i].m_xmf4x4World._43 += (((rand() % 10) / 10.0f - 0.5) / nCurrentFrameRate);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DroneEffect::DroneEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : BillboardEffect(nMeshes, nObjects, pPlayer)
{
	DWORD dwTime = GetTickCount();
	m_bEffectFlag = true;
	bIsHit = false;
	for (int i = 0; i < m_nObjects; i++)
	{
		Init(i, dwTime, true);
	}
}
DroneEffect::~DroneEffect()
{
}

void DroneEffect::Init(UINT nIndex, const DWORD& dwdTime, bool bflag)
{
	m_pEffectsInfo[nIndex].xmf3Veclocity.x = (rand() % 100 - 50.0f) / 1000.0f;
	m_pEffectsInfo[nIndex].xmf3Veclocity.y = -(rand() % 100) / 1000.0f;
	m_pEffectsInfo[nIndex].xmf3Veclocity.z = (rand() % 100 - 50.0f) / 1000.0f;

	if (bflag)
	{
		m_pEffectsInfo[nIndex].xmf3Acceleration.x = (rand() % 100 - 50.0f) / 10000.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.y = -(rand() % 100) / 10000.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.z = (rand() % 100 - 50.0f) / 10000.0f;
	}

	m_pEffectsInfo[nIndex].dwdTime = dwdTime;
	m_pEffectsInfo[nIndex].fTotalTime = ((rand() % 2000) / 1000) + 1.5f;
	m_pEffectsInfo[nIndex].fScale = 1.0f + ((rand() % 6000 - 10000) / 10000.0f);

	XMFLOAT3 xmf3P = m_pPlayer->GetPosition();

	SetParticlePosition(nIndex, XMFLOAT3(xmf3P.x + ((rand() % 20) - 10), xmf3P.y + ((rand() % 10) - 5) + 25, xmf3P.z + ((rand() % 20) - 10)));
}
void DroneEffect::Animate(float fHeight, const DWORD& dwdTime, const unsigned long& nCurrentFrameRate)
{
	if (bIsHit)
	{
		float fTime = (dwdTime - m_dwHitTime) / 1000.0f;

		if (fTime > DRONE_HIT_TIME)
			bIsHit = false;
	}

	if (m_bEffectFlag)
	{
		XMFLOAT3 xmf3Acceleration;
		xmf3Acceleration.x = (rand() % 100 - 50.0f) / 1000000.0f / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.y = (rand() % 100 - 50.0f) / 1000000.0f / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.z = (rand() % 100 - 50.0f) / 1000000.0f / nCurrentFrameRate * AdjustmentValue;

		for (int i = 0; i < m_nObjects; i++)
		{
			float fTime = (dwdTime - m_pEffectsInfo[i].dwdTime) / 1000.0f;

			if (fTime > m_pEffectsInfo[i].fTotalTime)
			{
				Init(i, dwdTime, true);
			}

			m_pEffectsInfo[i].xmf3Acceleration = Vector3::Add(m_pEffectsInfo[i].xmf3Acceleration, xmf3Acceleration);
			m_pEffectsInfo[i].xmf3Veclocity = Vector3::Add(m_pEffectsInfo[i].xmf3Veclocity, m_pEffectsInfo[i].xmf3Acceleration);

			m_pEffects[i].m_xmf4x4World._41 += (m_pEffectsInfo[i].xmf3Veclocity.x / nCurrentFrameRate * AdjustmentValue);
			m_pEffects[i].m_xmf4x4World._42 += (m_pEffectsInfo[i].xmf3Veclocity.y / nCurrentFrameRate * AdjustmentValue);
			m_pEffects[i].m_xmf4x4World._43 += (m_pEffectsInfo[i].xmf3Veclocity.z / nCurrentFrameRate * AdjustmentValue);

			XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
			xmf4x4Scale._11 = m_pEffectsInfo[i].fScale, xmf4x4Scale._22 = m_pEffectsInfo[i].fScale, xmf4x4Scale._33 = m_pEffectsInfo[i].fScale;
			m_pEffects[i].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[i].m_xmf4x4World);

			float fValue = (m_pEffectsInfo[i].fTotalTime - fTime) / m_pEffectsInfo[i].fTotalTime;

			if (!bIsHit)
				m_pEffects[i].m_xmf4Color = XMFLOAT4(fValue * 0.4f, fValue * 0.6f, fValue * 0.2f, 1);
			else
				m_pEffects[i].m_xmf4Color = XMFLOAT4(fValue * 0.8f, fValue * 0.4f, fValue * 0.2f, 1);

			m_pEffects[i].m_fAlpha = -4.0f * fValue * (fValue - 1.0f);
		}
	}
}
void DroneEffect::SetHit()
{
	if (!bIsHit)
	{
		m_dwHitTime = GetTickCount();
		bIsHit = true;
	}
}

TeleportEffect::TeleportEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : BillboardEffect(nMeshes, nObjects, pPlayer)
{
	DWORD dwTime = GetTickCount();
	m_bEffectFlag = false;
	for (int i = 0; i < m_nObjects; i++)
	{
		Init(i, dwTime, true);
	}
	m_fTime = 0.5f;
}
TeleportEffect::~TeleportEffect()
{}
void TeleportEffect::SetParticle()
{
	if (!m_bEffectFlag)
	{
		m_dwdLength = GetTickCount();
		m_bEffectFlag = true;
	}
}

void TeleportEffect::Init(UINT nIndex, const DWORD& dwdTime, bool bflag)
{
	m_pEffectsInfo[nIndex].dwdTime = dwdTime;
	m_pEffectsInfo[nIndex].fTotalTime = ((rand() % 4) / 10.0f);
	m_pEffectsInfo[nIndex].fScale = rand() % 10;

	XMFLOAT3 xmf3P = m_pPlayer->GetPosition();
	SetParticlePosition(nIndex, XMFLOAT3(xmf3P.x + ((rand() % 15) - 7.5), xmf3P.y + ((rand() % 20) - 10) + 20, xmf3P.z + ((rand() % 15) - 7.5)));
}
void TeleportEffect::Animate(const DWORD& dwdTime)
{
	if ((GetTickCount() - m_dwdLength) / 1000.0f > m_fTime)
		m_bEffectFlag = false;

	if (m_bEffectFlag)
	{
		for (int i = 0; i < m_nObjects; i++)
		{
			float fTime = (dwdTime - m_pEffectsInfo[i].dwdTime) / 1000.0f;

			if (fTime > m_pEffectsInfo[i].fTotalTime)
			{
				Init(i, dwdTime, true);
			}

			float fValue = (m_pEffectsInfo[i].fTotalTime - fTime) / m_pEffectsInfo[i].fTotalTime;

			XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
			xmf4x4Scale._11 = m_pEffectsInfo[i].fScale, xmf4x4Scale._22 = m_pEffectsInfo[i].fScale, xmf4x4Scale._33 = m_pEffectsInfo[i].fScale;
			m_pEffects[i].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[i].m_xmf4x4World);

			m_pEffects[i].m_xmf4Color = XMFLOAT4(fValue * 0.2f, (1 - fValue) * 0.2f, fValue * 0.8f, 1);
			m_pEffects[i].m_fAlpha = -4.0f * fValue * (fValue - 1.0f);
		}
	}
}

JumpEffect::JumpEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : BillboardEffect(nMeshes, nObjects, pPlayer)
{
	DWORD dwTime = GetTickCount();
	m_bEffectFlag = false;
	for (int i = 0; i < m_nObjects; i++)
	{
		Init(i, dwTime, true);
	}
}
JumpEffect::~JumpEffect()
{}

void JumpEffect::Init(UINT nIndex, const DWORD& dwdTime, bool bflag)
{
	m_pEffectsInfo[nIndex].xmf3Veclocity.x = (rand() % 100 - 50.0f) / 100.0f;
	m_pEffectsInfo[nIndex].xmf3Veclocity.y = -(rand() % 100) / 100.0f;
	m_pEffectsInfo[nIndex].xmf3Veclocity.z = (rand() % 100 - 50.0f) / 100.0f;

	if (bflag)
	{
		m_pEffectsInfo[nIndex].xmf3Acceleration.x = (rand() % 100 - 50.0f) / 10000.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.y = -(rand() % 100) / 10000.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.z = (rand() % 100 - 50.0f) / 10000.0f;
	}

	m_pEffectsInfo[nIndex].dwdTime = dwdTime;
	m_pEffectsInfo[nIndex].fTotalTime = ((rand() % 2000) / 1000) + 1.5f;
	m_pEffectsInfo[nIndex].fScale = 1.0f + ((rand() % 6000) / 10000.0f);

	XMFLOAT3 xmf3P = m_pPlayer->GetPosition();

	SetParticlePosition(nIndex, XMFLOAT3(xmf3P.x, xmf3P.y - 10.0f, xmf3P.z));
}
void JumpEffect::SetParticle()
{
	if (!m_bEffectFlag)
	{
		m_bEffectFlag = true;

		m_dwdLength = GetTickCount();

		for (int i = 0; i < m_nObjects; i++)
		{
			Init(i, m_dwdLength, true);
		}
	}
}
void JumpEffect::Animate(const DWORD& dwdTime, const unsigned long& nCurrentFrameRate)
{
	if (m_bEffectFlag)
	{
		XMFLOAT3 xmf3Acceleration;
		xmf3Acceleration.x = ((rand() % 100 - 50.0f) / 1000000.0f) / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.y = ((rand() % 100 - 50.0f) / 1000000.0f) / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.z = ((rand() % 100 - 50.0f) / 1000000.0f) / nCurrentFrameRate * AdjustmentValue;

		for (int i = 0; i < m_nObjects; i++)
		{
			float fTime = (dwdTime - m_dwdLength) / 1000.0f;

			if (fTime > m_fTime)
			{
				m_bEffectFlag = false;
				return;
			}

			m_pEffectsInfo[i].xmf3Acceleration = Vector3::Add(m_pEffectsInfo[i].xmf3Acceleration, xmf3Acceleration);
			m_pEffectsInfo[i].xmf3Veclocity = Vector3::Add(m_pEffectsInfo[i].xmf3Veclocity, m_pEffectsInfo[i].xmf3Acceleration);

			m_pEffects[i].m_xmf4x4World._41 += (m_pEffectsInfo[i].xmf3Veclocity.x / nCurrentFrameRate * AdjustmentValue);
			m_pEffects[i].m_xmf4x4World._42 += (m_pEffectsInfo[i].xmf3Veclocity.y / nCurrentFrameRate * AdjustmentValue);
			m_pEffects[i].m_xmf4x4World._43 += (m_pEffectsInfo[i].xmf3Veclocity.z / nCurrentFrameRate * AdjustmentValue);

			XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
			xmf4x4Scale._11 = 2.0f, xmf4x4Scale._22 = 2.0f + m_pEffectsInfo[i].fScale, xmf4x4Scale._33 = 2.0f;
			m_pEffects[i].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[i].m_xmf4x4World);

			float fValue = (m_pEffectsInfo[i].fTotalTime - fTime) / m_pEffectsInfo[i].fTotalTime;

			m_pEffects[i].m_xmf4Color = XMFLOAT4(fValue * 0.2f, fValue * 0.2f, fValue + 0.8f, 1);
			m_pEffects[i].m_fAlpha = -4.0f * fValue * (fValue - 1.0f);
		}
	}
}

SphereEffect::SphereEffect(UINT nMeshes, UINT nObjects, CAnimationObject * pPlayer) : MeshEffect(nMeshes, nObjects, pPlayer)
{
	m_fTime = 1.0f;
}

SphereEffect::~SphereEffect()
{
}
void SphereEffect::Init(UINT nIndex)
{
	if (nIndex == 0)
	{
		m_pEffectsInfo[nIndex].xmf3Veclocity.x = 0.0f;
		m_pEffectsInfo[nIndex].xmf3Veclocity.y = 0.0f;
		m_pEffectsInfo[nIndex].xmf3Veclocity.z = 0.0f;

		m_pEffectsInfo[nIndex].xmf3Acceleration.x = 0.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.y = 0.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.z = 0.0f;

		m_pEffectsInfo[nIndex].fScale = 1.0f;

		m_pEffects[nIndex].m_xmf4x4World = Matrix4x4::Identity();

		SetParticlePosition(nIndex, m_xmf3Target);
	}
	else
	{
		m_pEffectsInfo[nIndex].xmf3Veclocity.x = (rand() % 100 - 50.0f) / 200.0f;
		m_pEffectsInfo[nIndex].xmf3Veclocity.y = (rand() % 100 - 50.0f) / 200.0f;
		m_pEffectsInfo[nIndex].xmf3Veclocity.z = (rand() % 100 - 50.0f) / 200.0f;

		m_pEffectsInfo[nIndex].xmf3Acceleration.x = (rand() % 100 - 50.0f) / 5000.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.y = (rand() % 100 - 50.0f) / 5000.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.z = (rand() % 100 - 50.0f) / 5000.0f;

		m_pEffectsInfo[nIndex].fScale = 2.5f;

		SetParticlePosition(nIndex, m_xmf3Target);
	}
}
void SphereEffect::SetTarget(XMFLOAT3& xmf3Target)
{
	if (!m_bEffectFlag)
	{
		m_bEffectFlag = true;
		m_xmf3Target = xmf3Target;

		m_dwdLength = GetTickCount();
		for (int i = 0; i < m_nObjects; i++)
		{
			Init(i);
		}
	}
};
void SphereEffect::Animate(const DWORD& dwdTime, const unsigned long& nCurrentFrameRate)
{
	if (m_bEffectFlag)
	{
		XMFLOAT3 xmf3Acceleration;
		xmf3Acceleration.x = ((rand() % 100 - 50.0f) / 100000.0f) / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.y = ((rand() % 100 - 50.0f) / 100000.0f) / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.z = ((rand() % 100 - 50.0f) / 100000.0f) / nCurrentFrameRate * AdjustmentValue;

		float fTime = (dwdTime - m_dwdLength) / 1000.0f;

		m_pEffectsInfo[0].fScale += (0.003f / nCurrentFrameRate * AdjustmentValue);

		if (fTime > m_fTime)
		{
			m_bEffectFlag = false;
			return;
		}

		for (int i = 0; i < m_nObjects; i++)
		{
			m_pEffectsInfo[i].xmf3Acceleration = Vector3::Add(m_pEffectsInfo[i].xmf3Acceleration, xmf3Acceleration);
			m_pEffectsInfo[i].xmf3Veclocity = Vector3::Add(m_pEffectsInfo[i].xmf3Veclocity, m_pEffectsInfo[i].xmf3Acceleration);

			m_pEffects[i].m_xmf4x4World._41 += (m_pEffectsInfo[i].xmf3Veclocity.x / nCurrentFrameRate * AdjustmentValue);
			m_pEffects[i].m_xmf4x4World._42 += (m_pEffectsInfo[i].xmf3Veclocity.y / nCurrentFrameRate * AdjustmentValue);
			m_pEffects[i].m_xmf4x4World._43 += (m_pEffectsInfo[i].xmf3Veclocity.z / nCurrentFrameRate * AdjustmentValue);

			XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
			xmf4x4Scale._11 = m_pEffectsInfo[i].fScale, xmf4x4Scale._22 = m_pEffectsInfo[i].fScale, xmf4x4Scale._33 = m_pEffectsInfo[i].fScale;
			m_pEffects[i].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[i].m_xmf4x4World);

			float fValue = (m_fTime - fTime) / m_fTime;

			if (i == 0)
				m_pEffects[i].m_xmf4Color = XMFLOAT4((1 - fValue) * 0.7f, (fValue) * 0.9f, fValue * 0.4f, 1);
			else
				m_pEffects[i].m_xmf4Color = XMFLOAT4(fValue * 0.9f, (1 - fValue) * 0.6f, fValue * 0.4f, 1);

			m_pEffects[i].m_fAlpha = -4.0f * fValue * (fValue - 1.0f);
		}
	}
}

void SphereEffect::Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex)
{
	if (m_bEffectFlag)
	{
		UpdateShaderVariables(pd3dCommandList);

		if (m_ppMeshes)
		{
			if (m_ppMeshes[nMeshIndex])
			{
				if (nMeshIndex == 0)
					m_ppMeshes[0]->Render(pd3dCommandList, 1);
				else
					m_ppMeshes[1]->Render(pd3dCommandList, m_nObjects - 1);
			}
		}
	}
}
void SphereEffect::SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& right)
{
	if (m_bEffectFlag)
	{
		// 카메라의 Right벡터를 넣어준다.
		for (int i = 1; i < m_nObjects; i++)
		{
			XMFLOAT3 xmf3Right = right;
			XMFLOAT3 xmf3Position(m_pEffects[i].m_xmf4x4World._41, m_pEffects[i].m_xmf4x4World._42, m_pEffects[i].m_xmf4x4World._43);
			//타겟에서 카메라위치를 빼서 카메라를 향하는 Look벡터를 만든다.
			XMFLOAT3 xmf3Look = Vector3::Subtract(xmf3Target, xmf3Position);
			//Right벡터와 Look벡터를 외적하여 Up벡터를 구한다.
			XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);

			m_pEffects[i].m_xmf4x4World._11 = xmf3Right.x, m_pEffects[i].m_xmf4x4World._12 = xmf3Right.y, m_pEffects[i].m_xmf4x4World._13 = xmf3Right.z;
			m_pEffects[i].m_xmf4x4World._21 = xmf3Up.x, m_pEffects[i].m_xmf4x4World._22 = xmf3Up.y, m_pEffects[i].m_xmf4x4World._23 = xmf3Up.z;
			m_pEffects[i].m_xmf4x4World._31 = xmf3Look.x, m_pEffects[i].m_xmf4x4World._32 = xmf3Look.y, m_pEffects[i].m_xmf4x4World._33 = xmf3Look.z;
		}
	}
}

BloodEffect::BloodEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : BillboardEffect(nMeshes, nObjects, pPlayer)
{
	m_fTime = 0.4f;
}
BloodEffect::~BloodEffect()
{

}
void BloodEffect::SetTarget(XMFLOAT3& xmf3Target)
{
	if (!m_bEffectFlag)
	{
		m_bEffectFlag = true;
		m_xmf3Target = xmf3Target;

		m_dwdLength = GetTickCount();

		XMFLOAT3 pp = m_pPlayer->GetPosition();
		XMFLOAT3 tp = m_xmf3Target;

		pp.y = 0;
		tp.y = 0;

		XMFLOAT3 xmf3Dir = Vector3::Subtract(pp, tp);
		xmf3Dir = Vector3::Normalize(xmf3Dir);

		xmf3Dir.x = xmf3Dir.x / 6;
		xmf3Dir.y = xmf3Dir.y / 6;
		xmf3Dir.z = xmf3Dir.z / 6;

		for (int i = 0; i < m_nObjects; i++)
		{
			Init(i, xmf3Dir);
		}
	}
}
void BloodEffect::Init(UINT nIndex, const XMFLOAT3& xmf3Dir)
{
	m_pEffectsInfo[nIndex].xmf3Veclocity = xmf3Dir;

	m_pEffectsInfo[nIndex].xmf3Acceleration.x = (rand() % 100 - 50.0f) / 10000.0f;
	m_pEffectsInfo[nIndex].xmf3Acceleration.y = -(rand() % 100) / 10000.0f;
	m_pEffectsInfo[nIndex].xmf3Acceleration.z = (rand() % 100 - 50.0f) / 10000.0f;

	SetParticlePosition(nIndex, m_xmf3Target);
}
void BloodEffect::Animate(const DWORD& dwdTime, const unsigned long& nCurrentFrameRate)
{
	if (m_bEffectFlag)
	{
		XMFLOAT3 xmf3Acceleration;
		xmf3Acceleration.x = ((rand() % 100 - 50.0f) / 100000.0f) / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.y = -((rand() % 100) / 1000000.0f) / nCurrentFrameRate * AdjustmentValue;
		xmf3Acceleration.z = ((rand() % 100 - 50.0f) / 100000.0f) / nCurrentFrameRate * AdjustmentValue;

		float fTime = (dwdTime - m_dwdLength) / 1000.0f;

		if (fTime > m_fTime)
		{
			m_bEffectFlag = false;
			return;
		}

		for (int i = 0; i < m_nObjects; i++)
		{
			m_pEffectsInfo[i].xmf3Acceleration = Vector3::Add(m_pEffectsInfo[i].xmf3Acceleration, xmf3Acceleration);
			m_pEffectsInfo[i].xmf3Veclocity = Vector3::Add(m_pEffectsInfo[i].xmf3Veclocity, m_pEffectsInfo[i].xmf3Acceleration);

			m_pEffects[i].m_xmf4x4World._41 += (m_pEffectsInfo[i].xmf3Veclocity.x / nCurrentFrameRate * AdjustmentValue);
			m_pEffects[i].m_xmf4x4World._42 += (m_pEffectsInfo[i].xmf3Veclocity.y / nCurrentFrameRate * AdjustmentValue);
			m_pEffects[i].m_xmf4x4World._43 += (m_pEffectsInfo[i].xmf3Veclocity.z / nCurrentFrameRate * AdjustmentValue);

			XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
			xmf4x4Scale._11 = 1.0, xmf4x4Scale._22 = 1.05, xmf4x4Scale._33 = 1.0;
			m_pEffects[i].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[i].m_xmf4x4World);

			float fValue = (m_fTime - fTime) / m_fTime;

			m_pEffects[i].m_xmf4Color = XMFLOAT4(0.2f, 0.0f, 0.0f, 1);
			m_pEffects[i].m_fAlpha = -2.0f * fValue * (fValue - 1.0f);
		}
	}
}
void BloodEffect::Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex)
{
	if (m_bEffectFlag)
	{
		UpdateShaderVariables(pd3dCommandList);

		if (m_ppMeshes)
		{
			if (m_ppMeshes[nMeshIndex])
				m_ppMeshes[nMeshIndex]->Render(pd3dCommandList, m_nObjects);
		}
	}
}


DroneSkillEffect::DroneSkillEffect(UINT nMeshes, UINT nObjects, CAnimationObject * pPlayer) :MeshEffect(nMeshes, nObjects, pPlayer)
{
	m_fTime = 2.0f;
	m_bEffectFlag = false;
}
DroneSkillEffect::~DroneSkillEffect()
{
}

void DroneSkillEffect::SetTarget()
{
	if (!m_bEffectFlag)
	{
		m_dwdLength = GetTickCount();
		m_bEffectFlag = true;
	}
}
void DroneSkillEffect::Animate(const DWORD& dwdTime, XMFLOAT3& xmf3Dir)
{
	if (m_bEffectFlag)
	{
		XMFLOAT3 xmf3Pos = m_pPlayer->GetPosition();
		XMFLOAT3 up = m_pPlayer->GetUp();

		float zMul = 100.0f;
		float yMul = 15.0f;

		m_pEffects[0].m_xmf4x4World._41 = xmf3Pos.x + xmf3Dir.x * zMul + up.x * yMul;
		m_pEffects[0].m_xmf4x4World._42 = xmf3Pos.y + xmf3Dir.y * zMul + up.y * yMul;
		m_pEffects[0].m_xmf4x4World._43 = xmf3Pos.z + xmf3Dir.z * zMul + up.z * yMul;

		XMFLOAT3 xmf3Right;
		XMFLOAT3 xmf3Up;
		XMStoreFloat3(&xmf3Right, XMVector3Orthogonal(XMLoadFloat3(&xmf3Dir)));
		xmf3Up = Vector3::CrossProduct(xmf3Right, xmf3Dir);
		xmf3Up = Vector3::Normalize(xmf3Up);

		m_pEffects[0].m_xmf4x4World._11 = xmf3Right.x, m_pEffects[0].m_xmf4x4World._12 = xmf3Right.y, m_pEffects[0].m_xmf4x4World._13 = xmf3Right.z;
		m_pEffects[0].m_xmf4x4World._21 = xmf3Up.x, m_pEffects[0].m_xmf4x4World._22 = xmf3Up.y, m_pEffects[0].m_xmf4x4World._23 = xmf3Up.z;
		m_pEffects[0].m_xmf4x4World._31 = xmf3Dir.x, m_pEffects[0].m_xmf4x4World._32 = xmf3Dir.y, m_pEffects[0].m_xmf4x4World._33 = xmf3Dir.z;

		float fTime = (dwdTime - m_dwdLength) / 1000.0f;

		if (fTime > m_fTime)
		{
			m_bEffectFlag = false;
			return;
		}

		float fValue = (m_fTime - fTime) / m_fTime; // 0 ~ 1

		m_pEffects[0].m_xmf4Color = XMFLOAT4((1.0f - fValue) + 0.2f, fValue + 1.0f, fValue + 0.3f, 1);
		m_pEffectsInfo[0].fScale = -4.0f * fValue * (fValue - 1.0f);
		m_pEffects[0].m_fAlpha = 0.3f;

		XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
		xmf4x4Scale._11 = 30 * m_pEffectsInfo[0].fScale, xmf4x4Scale._22 = 30 * m_pEffectsInfo[0].fScale, xmf4x4Scale._33 = 500;
		m_pEffects[0].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[0].m_xmf4x4World);
	}
}