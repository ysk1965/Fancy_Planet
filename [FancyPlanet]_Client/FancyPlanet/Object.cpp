#include "stdafx.h"
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <float.h> 
#include "Object.h"

using namespace std;

TCHAR*CGameObject::char2tchar(char * asc)
{
	TCHAR* ptszUni = NULL;
	int nLen = MultiByteToWideChar(CP_ACP, 0, asc, strlen(asc), NULL, NULL);
	ptszUni = new TCHAR[nLen + 1];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, asc, strlen(asc), ptszUni, nLen);
	ptszUni[nLen] = '\0';
	return ptszUni;
}

CGameObject::CGameObject(int nMeshes, UINT nObjects)
{
	m_xmf4x4World = Matrix4x4::Identity();

	m_nMeshes = nMeshes;
	m_nObjects = nObjects;
	m_ppMeshes = NULL;

	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh*[m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)
			m_ppMeshes[i] = NULL;
	}
}
CGameObject::CGameObject(int nMeshes, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName, UINT nObjects, ID3D12Resource* pShadowMap) :m_nObjects(nObjects)
{
	m_xmf4x4World = Matrix4x4::Identity();

	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;

	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh*[m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)
			m_ppMeshes[i] = NULL;
	}
	ifstream fi;

	fi.open(pstrFileName, ostream::binary);

	if (!fi)
	{
		exit(1);
	}

	LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fi, GET_TYPE, 1, nObjects, pShadowMap);

}
CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}
	if (m_pMaterial)
		m_pMaterial->Release();

}

void CGameObject::ResizeMeshes(int nMeshes)
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
		m_ppMeshes = NULL;
	}

	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh*[m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;
	}
}

void CGameObject::SetMesh(int nIndex, CMesh *pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex])
			m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;

		if (pMesh)
			pMesh->AddRef();
	}
}

void CGameObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
	ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nType, UINT nSub, UINT nObject, ID3D12Resource* pShadowMap)
{
	int m_nType = -1;
	int nSubMesh = 0;
	static UINT nRendererMesh = 0;

	if (nType != SKIP)
		InFile.read((char*)&m_nType, sizeof(int)); // 타입 받기
	else
	{
		m_nType = SKINNEDMESH;
	}
	XMFLOAT3 *pxmf3ParentsPosition = NULL;

	if (m_nType == SKINNEDMESH || m_nType == RENDERMESH)
	{

		ResizeMeshes(1);

		XMFLOAT3 *pxmf3Positions = NULL;
		XMFLOAT2 *pxmf2TextureCoords0 = NULL;
		XMFLOAT3 *pxmf3Normals = NULL;
		XMFLOAT3 *pxmf3Tangents = NULL;

		char* pstrAlbedoTextureName = NULL;
		char* pstrNormalTextureName = NULL;
		UINT *pIndices = NULL;

		int nVertices = 0, nIndices = 0;

		CMaterial *pMaterial = NULL;

		InFile.read((char*)&nSubMesh, sizeof(int)); // 서브매쉬 갯수 받기

		InFile.read((char*)&m_nDrawType, sizeof(int)); // 그리기 타입 받기

		InFile.read((char*)&nVertices, sizeof(int)); // 정점 갯수 받기
		InFile.read((char*)&nIndices, sizeof(int)); // 정점 갯수 받기

		pIndices = new UINT[nIndices];
		pxmf3Positions = new XMFLOAT3[nVertices];
		pxmf2TextureCoords0 = new XMFLOAT2[nVertices];
		pxmf3Normals = new XMFLOAT3[nVertices];
		pxmf3Tangents = new XMFLOAT3[nVertices];

		InFile.read((char*)pIndices, sizeof(UINT) * nIndices); // 인덱스 받기
		InFile.read((char*)pxmf3Positions, sizeof(XMFLOAT3) * nVertices); // 정점 받기
		InFile.read((char*)pxmf2TextureCoords0, sizeof(XMFLOAT2) * nVertices); // uv 받기

		if (m_nDrawType > 1)
		{
			InFile.read((char*)pxmf3Normals, sizeof(XMFLOAT3) * nVertices); // 법선 받기
			InFile.read((char*)pxmf3Tangents, sizeof(XMFLOAT3) * nVertices); // 탄젠트 받기
		}
		int Namesize;

		InFile.read((char*)&Namesize, sizeof(int)); // 디퓨즈맵이름 받기
		pstrAlbedoTextureName = new char[Namesize];
		InFile.read(pstrAlbedoTextureName, sizeof(char)*Namesize);

		if (m_nDrawType > 1)
		{
			InFile.read((char*)&Namesize, sizeof(int)); // 노말맵이름 받기
			pstrNormalTextureName = new char[Namesize];
			InFile.read(pstrNormalTextureName, sizeof(char)*Namesize);
		}

		CMesh* pMesh = NULL;

		pMesh = new CMeshIlluminatedTexturedTBN(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf3Tangents, pxmf3Normals, pxmf2TextureCoords0, nIndices, pIndices);

		if (pMesh)
			SetMesh(0, pMesh);
		else
			ResizeMeshes(0);

		if (pxmf3Positions)
			delete[] pxmf3Positions;

		if (pxmf2TextureCoords0)
			delete[] pxmf2TextureCoords0;

		if (pxmf3Normals)
			delete[] pxmf3Normals;

		if (pxmf3Tangents)
			delete[] pxmf3Tangents;

		if (pIndices)
			delete[] pIndices;

		pMaterial = new CMaterial();

		TCHAR pstrPathName[128] = { '\0' };
		TCHAR* pstrFileName = char2tchar(pstrAlbedoTextureName);
		_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
		_tcscat_s(pstrPathName, 128, pstrFileName);
		_tcscat_s(pstrPathName, 128, _T(".dds"));

		CTexture *pTexture = new CTexture(m_nDrawType, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 0);

		if (pstrFileName)
			delete[] pstrFileName;

		if (m_nDrawType > 1)
		{
			pstrFileName = char2tchar(pstrNormalTextureName);
			_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
			_tcscat_s(pstrPathName, 128, pstrFileName);
			_tcscat_s(pstrPathName, 128, _T(".dds"));

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 1);
		}

		if (pstrAlbedoTextureName)
			delete[] pstrAlbedoTextureName;

		if (pstrNormalTextureName)
			delete[] pstrNormalTextureName;

		pMaterial->SetTexture(pTexture);

		CShader* pShader = NULL;

		if (m_nDrawType == 1 && m_nType == SKINNEDMESH)
			pShader = new CDefferredTexturedShader();
		else if (m_nDrawType == 2 && m_nType == SKINNEDMESH)
			pShader = new CDefferredLightingTexturedShader();
		else
			pShader = new CObjectShader();

		CreateShaderVariables(pd3dDevice, pd3dCommandList);
		pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 1);
		pShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_nDrawType);
		pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 2, true, pShadowMap);
		SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());

		pMaterial->SetShader(pShader);

		if (pMaterial)
			SetMaterial(pMaterial);
	}
}

void CGameObject::SetShader(CShader *pShader)
{
	if (!m_pMaterial)
	{
		CMaterial *pMaterial = new CMaterial();
		SetMaterial(pMaterial);
	}
	if (m_pMaterial)
		m_pMaterial->SetShader(pShader);
}

void CGameObject::SetMaterial(CMaterial *pMaterial)
{
	if (m_pMaterial)
		m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
}

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{// 리소스 만들기
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * (m_nObjects)
		, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void **)&m_pcbMappedGameObjects);
}
void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	if (m_pMaterial)
		m_pMaterial->ReleaseShaderVariables();
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, UINT RootParameterIndex, CGameObject** ppObjects)
{// 쉐이더 변수 업데이트
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	if (RootParameterIndex != -1)
		pd3dCommandList->SetGraphicsRootShaderResourceView(RootParameterIndex, m_pd3dcbGameObjects->GetGPUVirtualAddress());

	for (int i = 0; i < m_nObjects; i++)
	{
		if (ppObjects != NULL)
			XMStoreFloat4x4(&m_pcbMappedGameObjects[i].m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&ppObjects[i]->m_xmf4x4World)));
		else
			XMStoreFloat4x4(&m_pcbMappedGameObjects[i].m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	}
}

void CGameObject::UpdateAnimationVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CGameObject::Animate(float fTimeElapsed)
{
}


void CGameObject::SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(iRootParameterIndex, m_d3dCbvGPUDescriptorHandle);
}

void CGameObject::OnPrepareRender()
{
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera)
{
	if (!m_bActive)
		return;

	OnPrepareRender();

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	UpdateShaderVariables(pd3dCommandList, -1, NULL);

	if (m_nMeshes > 0)
	{
		SetRootParameter(pd3dCommandList, iRootParameterIndex);

		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}

}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera, UINT nInstances, CGameObject** ppObjects)
{
	if (m_pMaterial)
	{
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	UpdateShaderVariables(pd3dCommandList, iRootParameterIndex, ppObjects);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList, nInstances);
		}
	}
}

void CGameObject::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, UINT nInstances, CGameObject** ppObjects)
{
	UpdateShaderVariables(pd3dCommandList, iRootParameterIndex, ppObjects);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList, nInstances);
		}
	}
}

void CGameObject::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex)
{
	if (!m_bActive)
		return;

	UpdateShaderVariables(pd3dCommandList, -1, NULL);

	if (m_nMeshes > 0)
	{
		SetRootParameter(pd3dCommandList, iRootParameterIndex);

		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pMaterial)
		m_pMaterial->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4World = Matrix4x4::Multiply(mtxScale, m_xmf4x4World);
}
void CGameObject::SetRotation(XMFLOAT4& pxmf4Quaternion)
{
	m_xmf4x4World._11 = pxmf4Quaternion.x;
	m_xmf4x4World._13 = pxmf4Quaternion.y;
	m_xmf4x4World._31 = pxmf4Quaternion.z;
	m_xmf4x4World._33 = pxmf4Quaternion.z;
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}


