#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include <iostream>
#include <fstream>

using namespace std;

CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers)
{
	m_nTextureType = nTextureType;
	m_nTextures = nTextures;
	if (m_nTextures > 0)
	{
		m_pRootArgumentInfos = new SRVROOTARGUMENTINFO[m_nTextures];
		m_ppd3dTextureUploadBuffers = new ID3D12Resource*[m_nTextures];
		m_ppd3dTextures = new ID3D12Resource*[m_nTextures];
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) 
		m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) 
			if (m_ppd3dTextures[i]) 
				m_ppd3dTextures[i]->Release();
	}

	if (m_pRootArgumentInfos)
	{
		delete[] m_pRootArgumentInfos;
	}

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[0].m_nRootParameterIndex, m_pRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[i].m_nRootParameterIndex, m_pRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle);
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) 
			if (m_ppd3dTextureUploadBuffers[i]) 
				m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::ReleaseShaderVariables()
{
}

ID3D12Resource *CTexture::CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	if (m_pTexture) 
		m_pTexture->Release();
	if (m_pShader) 
		m_pShader->Release();
}

void CMaterial::SetTexture(CTexture *pTexture)
{
	if (m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CMaterial::SetShader(CShader *pShader)
{
	if (m_pShader) 
		m_pShader->Release();

	m_pShader = pShader;

	if (m_pShader) 
		m_pShader->AddRef();
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pTexture) 
		m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

void CMaterial::ReleaseShaderVariables()
{
	if (m_pShader) 
		m_pShader->ReleaseShaderVariables();
	if (m_pTexture) 
		m_pTexture->ReleaseShaderVariables();
}

void CMaterial::ReleaseUploadBuffers()
{
	if (m_pTexture)
		m_pTexture->ReleaseUploadBuffers();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CGameObject::CGameObject(int nMeshes)
{
	m_xmf4x4ToParentTransform = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh*[m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;
	}
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
	if (m_pMaterial) m_pMaterial->Release();

	if (m_pSibling) 
		delete m_pSibling;
	if (m_pChild) 
		delete m_pChild;
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
	if (m_pMaterial) m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
}

ID3D12Resource *CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObject->Map(0, NULL, (void **)&m_pcbMappedGameObject);

	return(m_pd3dcbGameObject);
}
void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
	}
	
	if (m_pMaterial) 
		m_pMaterial->ReleaseShaderVariables();
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	if (m_pMaterial) 
		m_pcbMappedGameObject->m_nMaterial = m_pMaterial->m_nReflection;
}

void CGameObject::UpdateAnimationVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	
	
}

void CGameObject::Animate(float fTimeElapsed)
{
	if (m_pSibling) 
		m_pSibling->Animate(fTimeElapsed);
	if (m_pChild) 
		m_pChild->Animate(fTimeElapsed);
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

	if (m_pAnimationController)
		m_pAnimationController->AdvanceAnimation(pd3dCommandList);

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	if (m_nMeshes > 0)
	{
		SetRootParameter(pd3dCommandList, iRootParameterIndex);

		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}

	if (m_pSibling)
		m_pSibling->Render(pd3dCommandList, 2, pCamera);
	if (m_pChild)
		m_pChild->Render(pd3dCommandList, 2, pCamera);
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera, UINT nInstances)
{
	OnPrepareRender();

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	SetRootParameter(pd3dCommandList, iRootParameterIndex);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) 
				m_ppMeshes[i]->Render(pd3dCommandList, nInstances);
		}
	}
}
void CGameObject::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pMaterial) 
		m_pMaterial->ReleaseUploadBuffers();

	if (m_pSibling) 
		m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) 
		m_pChild->ReleaseUploadBuffers();
}

void CGameObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParentTransform, *pxmf4x4Parent) : m_xmf4x4ToParentTransform;

	if (m_pSibling) 
		m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) 
		m_pChild->UpdateTransform(&m_xmf4x4World);
}

CGameObject *CGameObject::FindObject()
{
	CGameObject *pFrameObject = NULL;
	if (m_nMeshes > 0) 
		return(this);

	if (m_pSibling) 
		if (pFrameObject = m_pSibling->FindObject())
			return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindObject())
		return(pFrameObject);

	return(NULL);
}

void CGameObject::SetWorldPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetWorldPosition(XMFLOAT3 xmf3Position)
{
	SetWorldPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}
void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParentTransform._41 = x;
	m_xmf4x4ToParentTransform._42 = y;
	m_xmf4x4ToParentTransform._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetLocalPosition(XMFLOAT3 xmf3Position)
{
	XMMATRIX mtxTranslation = XMMatrixTranslation(xmf3Position.x, xmf3Position.y, xmf3Position.z);
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(m_xmf4x4ToParentTransform, mtxTranslation);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParentTransform);
	if (m_pSibling)
		m_pSibling->SetScale(x, y, z);
}

void CGameObject::SetLocalScale(float x, float y, float z)
{
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
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void CGameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}


void CGameObject::LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName)
{
	ifstream fi;

	fi.open(pstrFileName, ostream::binary);

	if (!fi) 
	{ 
		exit(1);
	}

	LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fi, 0, 1);
	LoadAnimation(pd3dDevice, pd3dCommandList, fi);
	fi.close(); // 파일 닫기
}
void CGameObject::LoadAnimation(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ifstream& InFile)
{
	UINT nAnimation = 0;

	CGameObject* pFindObjecct = FindObject();

	if (pFindObjecct)
	{
		InFile.read((char*)&nAnimation, sizeof(UINT));
		pFindObjecct->m_pAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, nAnimation, m_pBindPoses, m_nBindPoses);

		for (int i = 0; i < pFindObjecct->m_pAnimationController->GetAnimationCount(); i++)
		{
			InFile.read((char*)&pFindObjecct->m_pAnimationController->m_pAnimation[i].nTime, sizeof(UINT));
			pFindObjecct->m_pAnimationController->m_pAnimation[i].pFrame = new FRAME[pFindObjecct->m_pAnimationController->m_pAnimation[i].nTime];

			InFile.read((char*)pFindObjecct->m_pAnimationController->m_pAnimation[i].pFrame, sizeof(FRAME) * pFindObjecct->m_pAnimationController->m_pAnimation[i].nTime);
		}
	}
}
TCHAR* char2tchar(char * asc)
{
	TCHAR* ptszUni = NULL;
	int nLen = MultiByteToWideChar(CP_ACP, 0, asc, strlen(asc), NULL, NULL);
	ptszUni = new TCHAR[nLen + 1];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, asc, strlen(asc), ptszUni, nLen);
	ptszUni[nLen] = '\0';
	return ptszUni;
}
void CGameObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, 
	ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nFrame, UINT nSub)
{
	int nType = -1;
	int nSubMesh = 0;

	if(nSub == 1)
		InFile.read((char*)&nType, sizeof(int)); // 타입 받기
	else
	{
		nType = 0;
	}
	XMFLOAT3 *pxmf3ParentsPosition = NULL;

	if (nType == 0)
	{
		ResizeMeshes(1);

		XMFLOAT3 *pxmf3Positions = NULL;
		XMFLOAT2 *pxmf2TextureCoords0 = NULL;
		XMFLOAT3 *pxmf3Normals = NULL;
		XMFLOAT3 *pxmf3Tangents = NULL;
		XMFLOAT3 *pxmf3BoneWeights = NULL;
		XMINT4 *pxmi4BoneIndices = NULL;

		char* pstrAlbedoTextureName = NULL;
		char* pstrNormalTextureName = NULL;
		UINT *pnIndices = NULL;

		int nVertices = 0, nIndices = 0;

		CMesh *pMesh = NULL;
		CMaterial *pMaterial = NULL;

		int nDrawType = 0;
		InFile.read((char*)&nSubMesh, sizeof(int)); // 서브매쉬 갯수 받기

		InFile.read((char*)&nDrawType, sizeof(int)); // 그리기 타입 받기

		InFile.read((char*)&nVertices, sizeof(int)); // 정점 갯수 받기
		pxmf3Positions = new XMFLOAT3[nVertices];
		pxmf2TextureCoords0 = new XMFLOAT2[nVertices];
		pxmf3Normals = new XMFLOAT3[nVertices];
		pxmf3Tangents = new XMFLOAT3[nVertices];
		pxmf3BoneWeights = new XMFLOAT3[nVertices];
		pxmi4BoneIndices = new XMINT4[nVertices];

		InFile.read((char*)pxmf3Positions, sizeof(XMFLOAT3) * nVertices); // 정점 받기
		InFile.read((char*)pxmf2TextureCoords0, sizeof(XMFLOAT2) * nVertices); // uv 받기

		if (nDrawType > 1)
		{
			InFile.read((char*)pxmf3Normals, sizeof(XMFLOAT3) * nVertices); // 법선 받기
			InFile.read((char*)pxmf3Tangents, sizeof(XMFLOAT3) * nVertices); // 탄젠트 받기
		}
		InFile.read((char*)pxmf3BoneWeights, sizeof(XMFLOAT3) * nVertices); // 본가중치 받기
		InFile.read((char*)pxmi4BoneIndices, sizeof(XMINT4) * nVertices); // 본인덱스 받기
		InFile.read((char*)&m_nBindPoses, sizeof(int));

		m_pBindPoses = new XMFLOAT4X4[m_nBindPoses];

		InFile.read((char*)m_pBindPoses, sizeof(XMFLOAT4X4)*m_nBindPoses);
		int Namesize;
		InFile.read((char*)&Namesize, sizeof(int)); // 디퓨즈맵이름 받기
		pstrAlbedoTextureName = new char[Namesize];
		InFile.read(pstrAlbedoTextureName, sizeof(char)*Namesize);

		if (nDrawType > 1)
		{
			InFile.read((char*)&Namesize, sizeof(int)); // 노말맵이름 받기
			pstrNormalTextureName = new char[Namesize];
			InFile.read(pstrNormalTextureName, sizeof(char)*Namesize);
		}

		if (nVertices > 0 && nDrawType > 1)
			pMesh = new CAnimationMesh(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf3Tangents
				, pxmf3Normals, pxmf2TextureCoords0, pxmf3BoneWeights, pxmi4BoneIndices, nIndices, pnIndices);
		else if (nVertices > 0)
			pMesh = new CMeshAnimationTextured(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf2TextureCoords0, pxmf3BoneWeights, pxmi4BoneIndices, nIndices, pnIndices);

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

		if (pnIndices)
			delete[] pnIndices;

		pMaterial = new CMaterial();

		TCHAR pstrPathName[128] = { '\0' };
		TCHAR* pstrFileName = char2tchar(pstrAlbedoTextureName);
		_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
		_tcscat_s(pstrPathName, 128, pstrFileName);
		_tcscat_s(pstrPathName, 128, _T(".dds"));

		CTexture *pTexture = new CTexture(nDrawType, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 0);

		if (pstrFileName)
			delete[] pstrFileName;

		if (nDrawType > 1)
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

		UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

		ID3D12Resource *pd3dcbResource = CreateShaderVariables(pd3dDevice, pd3dCommandList);
		
		CShader* pShader = NULL;

		if (nDrawType == 1)
			pShader = new CDefferredTexturedShader();
		else if (nDrawType == 2)
			pShader = new CDefferredLightingTexturedShader();

		pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 4);
		pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		pShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, nDrawType);
		pShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, pd3dcbResource, ncbElementBytes);
		pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 5, true);
		SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());

		pMaterial->SetShader(pShader);

		if (pMaterial)
			SetMaterial(pMaterial);
	}
	if (nType <= 1)
	{
		if (nSubMesh <= nSub)
		{
			pxmf3ParentsPosition = new XMFLOAT3();
			InFile.read((char*)pxmf3ParentsPosition, sizeof(XMFLOAT3));
			
			if(nType == 0)
				SetLocalPosition(*pxmf3ParentsPosition);

			delete pxmf3ParentsPosition;
			bool bChild = false;
			InFile.read((char*)&bChild, sizeof(bool));

			if (bChild)
			{
				int nChild = 0;
				InFile.read((char*)&nChild, sizeof(int));

				for (int i = 0; i < nChild; i++)
				{
					CGameObject *pChild = new CGameObject(0);
					pChild->LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile, nFrame + 1, 1);
					SetChild(pChild);
				}
			}
		}
		if (nSubMesh > nSub) // 하나의 매쉬가 여러개의 서브매쉬를 가지면 형제로 지정한다. 
		{
			CGameObject *pSibling = new CGameObject(0);
			pSibling->LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile, nFrame + 1, nSub + 1);

			SetChild(pSibling);
		}
	}
}

void CGameObject::SetChild(CGameObject *pChild)
{
	if (m_pChild)
	{
		if (pChild) 
			pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
	if (pChild) 
		pChild->m_pParent = this;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject(0)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	
		m_ppMeshes[i] = NULL;

	CHeightMapGridMesh *pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(x + (z*cxBlocks), pHeightMapGridMesh);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pTerrainTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0);

	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/Base_Texture.dds", 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/Detail_Texture_7.dds", 1);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/NormalMap3.dds", 2);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); 

	CTerrainShader *pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 4);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pTerrainShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 3);
	pTerrainShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbGameObject, ncbElementBytes);
	pTerrainShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTerrainTexture, 3, true);

	CMaterial *pTerrainMaterial = new CMaterial();
	pTerrainMaterial->SetTexture(pTerrainTexture);

	SetMaterial(pTerrainMaterial);

	SetCbvGPUDescriptorHandle(pTerrainShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pTerrainShader);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) 
		delete m_pHeightMapImage;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject(6)
{
	CTexturedRectMesh *pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 0.0f, 0.0f, 0.0f, +10.0f);
	SetMesh(0, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 0.0f, 0.0f, 0.0f, -10.0f);
	SetMesh(1, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.0f, 20.0f, 20.0f, -10.0f, 0.0f, 0.0f);
	SetMesh(2, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.0f, 20.0f, 20.0f, +10.0f, 0.0f, 0.0f);
	SetMesh(3, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 0.0f, 20.0f, 0.0f, +10.0f, 0.0f);
	SetMesh(4, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 0.0f, 20.0f, 0.0f, -10.0f, 0.0f);
	SetMesh(5, pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/SkyBox/starbox_1024.dds", 0);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수

	CSkyBoxShader *pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pSkyBoxShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 1);
	pSkyBoxShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbGameObject, ncbElementBytes);
	pSkyBoxShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pSkyBoxTexture, 6, false);

	CMaterial *pSkyBoxMaterial = new CMaterial();
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);

	SetMaterial(pSkyBoxMaterial);

	SetCbvGPUDescriptorHandle(pSkyBoxShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pSkyBoxShader);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetWorldPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	OnPrepareRender();

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
	}

	pd3dCommandList->SetGraphicsRootDescriptorTable(2, m_d3dCbvGPUDescriptorHandle);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_pMaterial)
			{
				if (m_pMaterial->m_pTexture) 
					m_pMaterial->m_pTexture->UpdateShaderVariable(pd3dCommandList, 0);
			}
			if (m_ppMeshes[i]) 
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CharaterObject::CharaterObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nMeshes, TCHAR *pstrFileName) : CGameObject(nMeshes)
{
	LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrFileName);

	SetScale(200.0f, 200.0f, 200.0f);
}
CharaterObject::~CharaterObject()
{
}