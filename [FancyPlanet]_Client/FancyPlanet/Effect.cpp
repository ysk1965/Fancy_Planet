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
	XMFLOAT4X4 WorldTransform;

	for (int i = 0; i < m_nObjects; i++)
	{
		WorldTransform = m_pEffects[i].m_xmf4x4World;
		XMStoreFloat4x4(&m_pcbMappedEffect[i].m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&WorldTransform)));

		m_pcbMappedEffect[i].m_xmf4Emissive = m_pEffects[i].m_xmf4Emissive;
		m_pcbMappedEffect[i].m_xmf3Color = m_pEffects[i].m_xmf3Color;
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
void BillboardEffect::Animate(float fHeight, const DWORD& dwdTime)
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
	// 카메라의 Right벡터를 넣어준다.
	for (int i = 0; i < m_nObjects; i++)
	{
		XMFLOAT3 xmf3Right = right;
		XMFLOAT3 xmf3Position(m_pEffects[i].m_xmf4x4World._41, m_pEffects[i].m_xmf4x4World._42, m_pEffects[i].m_xmf4x4World._43 );
		//타겟에서 카메라위치를 빼서 카메라를 향하는 Look벡터를 만든다.
		XMFLOAT3 xmf3Look = Vector3::Subtract(xmf3Target, xmf3Position);
		//Right벡터와 Look벡터를 외적하여 Up벡터를 구한다.
		XMFLOAT3 xmf3Up = Vector3::CrossProduct(xmf3Look, xmf3Right, true);

		m_pEffects[i].m_xmf4x4World._11 = xmf3Right.x, m_pEffects[i].m_xmf4x4World._12 = xmf3Right.y, m_pEffects[i].m_xmf4x4World._13 = xmf3Right.z;
		m_pEffects[i].m_xmf4x4World._21 = xmf3Up.x, m_pEffects[i].m_xmf4x4World._22 = xmf3Up.y, m_pEffects[i].m_xmf4x4World._23 = xmf3Up.z;
		m_pEffects[i].m_xmf4x4World._31 = xmf3Look.x, m_pEffects[i].m_xmf4x4World._32 = xmf3Look.y, m_pEffects[i].m_xmf4x4World._33 = xmf3Look.z;
	}
}
void BillboardEffect::Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex)
{
	UpdateShaderVariables(pd3dCommandList);

	if (m_ppMeshes)
	{
		if (m_ppMeshes[nMeshIndex])
			m_ppMeshes[nMeshIndex]->Render(pd3dCommandList, m_nObjects);
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
	m_xmf3Dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_bEffectFlag = false;

	m_fTime = 0.5f;
}
MeshEffect::~MeshEffect()
{
}
void MeshEffect::SetTarget(XMFLOAT3& xmf3Target)
{
	if (!m_bEffectFlag)
	{
		m_bEffectFlag = true;
		XMFLOAT3 xmf3Pos = m_pPlayer->GetPosition();

		m_pEffects[0].m_xmf4x4World._41 = (xmf3Pos.x + xmf3Target.x) / 2.0f;
		m_pEffects[0].m_xmf4x4World._42 = (xmf3Pos.y + 25 + xmf3Target.y) / 2.0f;
		m_pEffects[0].m_xmf4x4World._43 = (xmf3Pos.z + xmf3Target.z) / 2.0f;

		m_xmf3Dir = Vector3::Normalize(Vector3::Subtract(xmf3Target, xmf3Pos));

		XMFLOAT3 xmf3Right;
		XMFLOAT3 xmf3Up;
		XMStoreFloat3(&xmf3Right, XMVector3Orthogonal(XMLoadFloat3(&m_xmf3Dir)));
		xmf3Up = Vector3::CrossProduct(xmf3Right, m_xmf3Dir);
		xmf3Up = Vector3::Normalize(xmf3Up);

		m_pEffects[0].m_xmf4x4World._11 = xmf3Right.x, m_pEffects[0].m_xmf4x4World._12 = xmf3Right.y, m_pEffects[0].m_xmf4x4World._13 = xmf3Right.z;
		m_pEffects[0].m_xmf4x4World._21 = xmf3Up.x, m_pEffects[0].m_xmf4x4World._22 = xmf3Up.y, m_pEffects[0].m_xmf4x4World._23 = xmf3Up.z;
		m_pEffects[0].m_xmf4x4World._31 = m_xmf3Dir.x, m_pEffects[0].m_xmf4x4World._32 = m_xmf3Dir.y, m_pEffects[0].m_xmf4x4World._33 = m_xmf3Dir.z;

		float fLength = Vector3::Length(Vector3::Subtract(xmf3Target, xmf3Pos));

		XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
		xmf4x4Scale._11 = m_pEffectsInfo[0].fScale, xmf4x4Scale._22 = m_pEffectsInfo[0].fScale, xmf4x4Scale._33 = fLength;
		m_pEffects[0].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[0].m_xmf4x4World);

		m_dwdLength = GetTickCount();
	}
};
void MeshEffect::Aniamate(const DWORD& dwdTime)
{
	if (m_bEffectFlag)
	{
		float fTime = (dwdTime - m_dwdLength) / 1000.0f;

		if (fTime > m_fTime)
		{
			m_bEffectFlag = false;
			return;
		}

		float fValue = (m_fTime - fTime) / m_fTime; // 0 ~ 1

		m_pEffects[0].m_xmf3Color = XMFLOAT4((1.0f - fValue) + 0.2f, fValue + 1.0f, fValue + 0.3f, 1);
		m_pEffectsInfo[0].fScale = -4.0f * fValue * (fValue - 1.0f) * 3.0f + 1.0f;
		m_pEffects[0].m_fAlpha = -4.0f * fValue * (fValue - 1.0f);
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

	for (int i = 0; i < m_nObjects; i++)
	{
		Init(i, dwTime, true);
	}
}
SparkEffect::~SparkEffect()
{}
void SparkEffect::SetParticle(UINT nType, const XMFLOAT3& xmf3Pos, const XMFLOAT3& xmf3Dir)
{
	if (!m_bEffectFlag)
	{
		m_dwdLength = GetTickCount();

		m_bEffectFlag = true;

		XMFLOAT3 m_xmf3Dir = xmf3Dir;
		m_xmf3Dir.x /= 50.0f;
		m_xmf3Dir.y /= 50.0f;
		m_xmf3Dir.z /= 50.0f;


		for (int i = 0; i < m_nObjects; i++)
		{
			Init(i, m_dwdLength, false);

			SetParticlePosition(i, XMFLOAT3(xmf3Pos.x, xmf3Pos.y, xmf3Pos.z));
			m_pEffectsInfo[i].xmf3Acceleration = m_xmf3Dir;
			m_pEffects[i].m_xmf4Emissive = XMFLOAT4(0.4f, 0.2f, 0.1f, 0.0f);
		}
	}
}
void SparkEffect::Init(UINT nIndex, const DWORD& dwdTime, bool bflag)
{
	m_pEffectsInfo[nIndex].xmf3Veclocity.x = (rand() % 100 - 50.0f) / 1000.0f;
	m_pEffectsInfo[nIndex].xmf3Veclocity.y = (rand() % 100 - 50.0f) / 1000.0f;
	m_pEffectsInfo[nIndex].xmf3Veclocity.z = (rand() % 100 - 50.0f) / 1000.0f;

	if (bflag)
	{
		SetParticlePosition(nIndex, XMFLOAT3(rand() % 4 + m_pPlayer->GetPosition().x , +m_pPlayer->GetPosition().y, rand() % 4 + +m_pPlayer->GetPosition().z));

		m_pEffectsInfo[nIndex].xmf3Acceleration.x = (rand() % 100 - 50.0f) / 1000000.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.y = (rand() % 100 - 50.0f) / 1000000.0f;
		m_pEffectsInfo[nIndex].xmf3Acceleration.z = (rand() % 100 - 50.0f) / 1000000.0f;
	}
	
	m_pEffectsInfo[nIndex].dwdTime = dwdTime;
	m_pEffectsInfo[nIndex].fTotalTime = ((rand() % 300) / 100) + 2.5f;
	m_pEffectsInfo[nIndex].fScale = ((rand() % 100) / 1000) + 0.995;
}
void SparkEffect::Animate(float fHeight, const DWORD& dwdTime)
{
	if (m_bEffectFlag)
	{
		if ((dwdTime - m_dwdLength) / 1000.0f > m_fTime)
		{
			m_bEffectFlag = false;
		}
		XMFLOAT3 xmf3Acceleration;
		xmf3Acceleration.x = (rand() % 100 - 50.0f) / 500000.0f;
		xmf3Acceleration.y = (rand() % 100 - 50.0f) / 5000.0f;
		xmf3Acceleration.z = (rand() % 100 - 50.0f) / 500000.0f;

		for (int i = 0; i < m_nObjects; i++)
		{
			float fTime = (dwdTime - m_pEffectsInfo[i].dwdTime) / 1000.0f;

			m_pEffectsInfo[i].xmf3Acceleration = Vector3::Add(m_pEffectsInfo[i].xmf3Acceleration, xmf3Acceleration);
			m_pEffectsInfo[i].xmf3Veclocity = Vector3::Add(m_pEffectsInfo[i].xmf3Veclocity, m_pEffectsInfo[i].xmf3Acceleration);

			m_pEffects[i].m_xmf4x4World._41 += m_pEffectsInfo[i].xmf3Veclocity.x;
			m_pEffects[i].m_xmf4x4World._42 += m_pEffectsInfo[i].xmf3Veclocity.y;
			m_pEffects[i].m_xmf4x4World._43 += m_pEffectsInfo[i].xmf3Veclocity.z;

			XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
			xmf4x4Scale._11 = m_pEffectsInfo[i].fScale, xmf4x4Scale._22 = m_pEffectsInfo[i].fScale, xmf4x4Scale._33 = m_pEffectsInfo[i].fScale;
			m_pEffects[i].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[i].m_xmf4x4World);

			float fValue = (m_fTime - fTime) / m_fTime;

			m_pEffects[i].m_xmf3Color = XMFLOAT4(fValue + 1.0f, fValue + 0.3f, fValue + 0.1f, 1);
			m_pEffects[i].m_fAlpha = fValue;
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
void SmokeEffect::Animate(float fHeight, const DWORD& dwdTime)
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DroneEffect::DroneEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer) : BillboardEffect(nMeshes, nObjects, pPlayer)
{
	DWORD dwTime = GetTickCount();
	m_bEffectFlag = true;
	for (int i = 0; i < m_nObjects; i++)
	{
		Init(i, dwTime, true);
	}
}
DroneEffect::~DroneEffect()
{}

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
	m_pEffectsInfo[nIndex].fScale = 1.0f + ((rand()%6000 - 10000)/ 10000.0f);

	XMFLOAT3 xmf3P = m_pPlayer->GetPosition();

	SetParticlePosition(nIndex, XMFLOAT3(xmf3P.x + ((rand() % 10) - 5), xmf3P.y + ((rand() % 10) - 5) + 20, xmf3P.z + ((rand() % 10) - 5)));
}
void DroneEffect::Animate(float fHeight, const DWORD& dwdTime)
{
	if (m_bEffectFlag)
	{
		XMFLOAT3 xmf3Acceleration;
		xmf3Acceleration.x = (rand() % 100 - 50.0f) / 1000000.0f;
		xmf3Acceleration.y = (rand() % 100 -50.0f) / 1000000.0f;
		xmf3Acceleration.z = (rand() % 100 - 50.0f) / 1000000.0f;

		for (int i = 0; i < m_nObjects; i++)
		{
			float fTime = (dwdTime - m_pEffectsInfo[i].dwdTime) / 1000.0f;

			if (fTime > m_pEffectsInfo[i].fTotalTime)
			{
				Init(i, dwdTime, true);
			}

			m_pEffectsInfo[i].xmf3Acceleration = Vector3::Add(m_pEffectsInfo[i].xmf3Acceleration, xmf3Acceleration);
			m_pEffectsInfo[i].xmf3Veclocity = Vector3::Add(m_pEffectsInfo[i].xmf3Veclocity, m_pEffectsInfo[i].xmf3Acceleration);

			m_pEffects[i].m_xmf4x4World._41 += m_pEffectsInfo[i].xmf3Veclocity.x;
			m_pEffects[i].m_xmf4x4World._42 += m_pEffectsInfo[i].xmf3Veclocity.y;
			m_pEffects[i].m_xmf4x4World._43 += m_pEffectsInfo[i].xmf3Veclocity.z;

			XMFLOAT4X4 xmf4x4Scale = Matrix4x4::Identity();
			xmf4x4Scale._11 = m_pEffectsInfo[i].fScale, xmf4x4Scale._22 = m_pEffectsInfo[i].fScale, xmf4x4Scale._33 = m_pEffectsInfo[i].fScale;
			m_pEffects[i].m_xmf4x4World = Matrix4x4::Multiply(xmf4x4Scale, m_pEffects[i].m_xmf4x4World);

			float fValue = (m_pEffectsInfo[i].fTotalTime - fTime) / m_pEffectsInfo[i].fTotalTime;

			m_pEffects[i].m_xmf3Color = XMFLOAT4(fValue * 0.2f, fValue * 0.3f, fValue * 0.6f, 1);
			m_pEffects[i].m_fAlpha = -4.0f * fValue * (fValue - 1.0f);
		}
	}
}