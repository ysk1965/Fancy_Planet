#include "stdafx.h"
#include "Shader.h"
#include "Object.h"

StaticObject::StaticObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, TCHAR *pstrFileName, UINT nObjects, UINT nType) : m_nObjects(nObjects), m_nType(nType)
{
	if (m_nType == 0)
	{
		m_xmf4x4World = new XMFLOAT4X4[m_nObjects];

		for (int i = 0; i < m_nObjects; i++)
		{
			m_xmf4x4World[i] = Matrix4x4::Identity();
		}
	}
	if (m_nType == 1)
	{
		m_sto_e = new STO_E[m_nObjects];

		for (int i = 0; i < m_nObjects; i++)
		{
			m_sto_e[i].xmf4x4World = Matrix4x4::Identity();
			m_sto_e[i].xmf4Color = XMFLOAT4(0.2f, 0.6f, 0.2f, 1.0f);
			m_sto_e[i].fAlpha = 1.0f;
		}
	}

	m_pMesh = new CMesh(pd3dDevice, pd3dCommandList, pstrFileName);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
StaticObject::~StaticObject()
{
	if (m_xmf4x4World)
		delete[] m_xmf4x4World;

	if (m_pd3dcbGameObjects)
		m_pd3dcbGameObjects->Release();
}
void StaticObject::SetBase(int Num, int Color)
{

	m_dwdBaseTime[Num] = GetTickCount();
	Base[Num] = true;

	if (Color == 0)
	{
		m_sto_e[Num].xmf4Color = XMFLOAT4(0.9f, 0.2f, 0.2f, 1.0f);
	}
	else if (Color == 1)
	{
		m_sto_e[Num].xmf4Color = XMFLOAT4(0.2f, 0.2f, 0.9f, 1.0f);
	}
	else
	{
		m_sto_e[Num].xmf4Color = XMFLOAT4(0.2f, 0.9f, 0.2f, 1.0f);
	}
}
void StaticObject::UpdateBase()
{
	for (int i = 0; i < 3; i++)
	{
		float fTime = (GetTickCount() - m_dwdBaseTime[i]) / 1000.0f;
		if (Base[i])
		{
			if (m_sto_e[i].fAlpha < 0.5f)
			{
				m_sto_e[i].fAlpha = fTime / 10.0f;
			}
			else
			{
				m_sto_e[i].fAlpha = 0.6f;
			}
		}
	}
}
void StaticObject::SetShaderVariable(UINT nIndex, void* p)
{
	if (m_nType == 1)
	{
		memcpy(&m_sto_e[nIndex], p, sizeof(STO_E));
	}
}
void StaticObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nType == 0)
	{
		UINT ncbElementBytes = ((sizeof(XMFLOAT4X4) + 255) & ~255); //256의 배수
		m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * (m_nObjects)
			, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

		m_pd3dcbGameObjects->Map(0, NULL, (void **)&m_pcbMappedGameObjects);
	}
	else if (m_nType == 1)
	{
		UINT ncbElementBytes = ((sizeof(STO_E) + 255) & ~255); //256의 배수
		m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * (m_nObjects)
			, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

		m_pd3dcbGameObjects->Map(0, NULL, (void **)&m_pcbMappedGameObjects1);
	}
}
void StaticObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nType == 0)
	{
		for (int i = 0; i < m_nObjects; i++)
			XMStoreFloat4x4(&m_pcbMappedGameObjects[i], XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World[i])));
	}
	else if (m_nType == 1)
	{
		for (int i = 0; i < m_nObjects; i++)
		{
			XMStoreFloat4x4(&m_pcbMappedGameObjects1[i].xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_sto_e[i].xmf4x4World)));
			m_pcbMappedGameObjects1[i].xmf4Color = m_sto_e[i].xmf4Color;
			m_pcbMappedGameObjects1[i].fAlpha = m_sto_e[i].fAlpha;
		}
	}
}
void StaticObject::SetChangeColor(ID3D12GraphicsCommandList *pd3dCommandList, const DWORD& dwd_mColorTime)
{
	float fTime = (dwd_mColorTime - m_dwdColorTime) / 1000.0f;
	float fValue = fTime / 5.0f;

	if (fTime > 5.0f)
	{
		m_dwdColorTime = GetTickCount();
		m_xmf3NewColor = XMFLOAT3((rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f);
	}
	else
	{
		XMFLOAT3 xmf3Color = Vector3::Subtract(m_xmf3NewColor, m_xmf3CurrentColor);
		XMFLOAT3 xmf3Add = XMFLOAT3(xmf3Color.x * fValue, xmf3Color.y * fValue, xmf3Color.z * fValue);

		m_xmf3CurrentColor = Vector3::Add(xmf3Add, m_xmf3CurrentColor);

		for (int i = 0; i < m_nObjects; i++)
		{
			m_sto_e[i].xmf4Color = XMFLOAT4(m_xmf3CurrentColor.x, m_xmf3CurrentColor.y, m_xmf3CurrentColor.z, 1.0f);
			m_sto_e[i].fAlpha = -3.2 * fValue * (fValue - 1);
		}
	}
	UpdateShaderVariables(pd3dCommandList);
}
void StaticObject::SetPosition(UINT nIndex, const XMFLOAT3& xmf3Pos)
{
	if (m_nType == 0)
	{
		m_xmf4x4World[nIndex]._41 = xmf3Pos.x;
		m_xmf4x4World[nIndex]._42 = xmf3Pos.y;
		m_xmf4x4World[nIndex]._43 = xmf3Pos.z;
	}
	else if (m_nType == 1)
	{
		m_sto_e[nIndex].xmf4x4World._41 = xmf3Pos.x;
		m_sto_e[nIndex].xmf4x4World._42 = xmf3Pos.y;
		m_sto_e[nIndex].xmf4x4World._43 = xmf3Pos.z;
	}
}
void StaticObject::SetScale(UINT nIndex, float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	if (m_nType == 0)
	{
		m_xmf4x4World[nIndex] = Matrix4x4::Multiply(mtxScale, m_xmf4x4World[nIndex]);
	}
	else if (m_nType == 1)
	{
		m_sto_e[nIndex].xmf4x4World = Matrix4x4::Multiply(mtxScale, m_sto_e[nIndex].xmf4x4World);
	}
}
void StaticObject::Rotate(UINT nIndex, float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	if (m_nType == 0)
	{
		m_xmf4x4World[nIndex] = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World[nIndex]);
	}
	else if (m_nType == 1)
	{
		m_sto_e[nIndex].xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_sto_e[nIndex].xmf4x4World);
	}
}
void StaticObject::Render(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootShaderResourceView(10, m_pd3dcbGameObjects->GetGPUVirtualAddress());

	if (m_pMesh)
		m_pMesh->Render(pd3dCommandList, m_nObjects);
}