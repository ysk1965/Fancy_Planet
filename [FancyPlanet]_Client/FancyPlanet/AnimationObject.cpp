#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <float.h> 

CAnimationObject::CAnimationObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nMeshes, TCHAR *pstrFileName, AnimationController* pAnimationController, UINT nObjects, UINT& nAnimationFactor, ID3D12Resource* pShadowMap) : CGameObject(nMeshes, 0)
{
	m_xmf4x4ToParentTransform = Matrix4x4::Identity();

	LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrFileName, pAnimationController, nObjects, nAnimationFactor, pShadowMap);
}
CAnimationObject::~CAnimationObject()
{
	if (m_pBindPoses)
		delete[] m_pBindPoses;

	if (m_pAnimationFactors)
		delete m_pAnimationFactors;

	if (m_pAnimationController)
		delete m_pAnimationController;

	if (m_pSibling)
		delete m_pSibling;
	if (m_pChild)
		delete m_pChild;
}
void CAnimationObject::ReleaseUploadBuffers()
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

	if (m_pSibling)
		m_pSibling->ReleaseUploadBuffers();
	if (m_pChild)
		m_pChild->ReleaseUploadBuffers();
}
void CAnimationObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera)
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
CAnimationObject* CAnimationObject::FindAnimationFactorObject(CAnimationObject* pRootObject, int nIndex)
{
	stack<CAnimationObject*> FrameStack;

	FrameStack.push(pRootObject);

	while (!FrameStack.empty())
	{
		CAnimationObject* pFindObject = FrameStack.top();
		FrameStack.pop();

		if (pFindObject->m_pAnimationFactors != NULL && pFindObject->GetSkinnedMeshRendererNum() == nIndex)
			return pFindObject;

		if (pFindObject->m_pChild)
			FrameStack.push(pFindObject->m_pChild);
		if (pFindObject->m_pSibling)
			FrameStack.push(pFindObject->m_pSibling);
	}
	return NULL;
}
void CAnimationObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances
	, CAnimationObject** ppObject, UINT nIndex, CPlayer* pPlayer, UINT nCharacterIndex)
{
	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	if (m_ppMeshes)
	{
		int AnimationFactorIndex = -1;
		// 그림자 맵

		for (int i = 0; i < nIndex; i++)
		{
			if (ppObject[nInstances - 1]->m_ppAnimationFactorObjects[i]->m_iBoneIndex == m_iBoneIndex)
			{
				AnimationFactorIndex = i;
				break;
			}
		}

		for (int i = 0; i< nInstances; i++)
		{
			ppObject[i]->m_pAnimationController->SetObject(ppObject[i], ppObject[i]->m_ppAnimationFactorObjects[AnimationFactorIndex]);
			ppObject[i]->m_pAnimationController->AdvanceAnimation(pd3dCommandList, false);

			pd3dCommandList->SetGraphicsRootDescriptorTable(13, m_pMaterial->m_pShader->GetShadowGPUDescriptorHandle());

			if (i == nCharacterIndex && ppObject[i]->m_pPlayer != NULL)
			{
				ppObject[nCharacterIndex]->m_xmf4x4ToParentTransform = pPlayer->m_xmf4x4World;

				XMFLOAT4X4 Scale = Matrix4x4::Identity();
				Scale._11 = Scale._22 = Scale._33 = ppObject[nCharacterIndex]->GetPlayerScale();
				ppObject[nCharacterIndex]->m_xmf4x4ToParentTransform = Matrix4x4::Multiply(Scale, ppObject[nCharacterIndex]->m_xmf4x4ToParentTransform);
			}
		}
		ppObject[nInstances - 1]->m_ppAnimationFactorObjects[AnimationFactorIndex]->m_pAnimationFactors->UpdateShaderVariables(pd3dCommandList, nInstances, ppObject, AnimationFactorIndex);

		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList, nInstances);
		}
	}

	if (m_pSibling)
		m_pSibling->Render(pd3dCommandList, pCamera, nInstances, ppObject, nIndex, pPlayer, nCharacterIndex);

	if (m_pChild)
		m_pChild->Render(pd3dCommandList, pCamera, nInstances, ppObject, nIndex, pPlayer, nCharacterIndex);
}
void CAnimationObject::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances, CAnimationObject** ppObject, UINT nIndex)
{
	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
		}
	}

	if (m_ppMeshes)
	{

		int AnimationFactorIndex = -1;

		for (int i = 0; i < nIndex; i++)
		{
			if (ppObject[nInstances - 1]->m_ppAnimationFactorObjects[i]->m_iBoneIndex == m_iBoneIndex)
			{
				AnimationFactorIndex = i;
				break;
			}
		}

		ppObject[nInstances - 1]->m_ppAnimationFactorObjects[AnimationFactorIndex]->m_pAnimationFactors->UpdateShaderVariables(pd3dCommandList, nInstances, ppObject, AnimationFactorIndex);

		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList, nInstances);
		}
	}

	if (m_pSibling)
		m_pSibling->ShadowRender(pd3dCommandList, pCamera, nInstances, ppObject, nIndex);

	if (m_pChild)
		m_pChild->ShadowRender(pd3dCommandList, pCamera, nInstances, ppObject, nIndex);
}
TCHAR* CAnimationObject::CharToTCHAR(char * asc)
{
	TCHAR* ptszUni = NULL;
	int nLen = MultiByteToWideChar(CP_ACP, 0, asc, strlen(asc), NULL, NULL);
	ptszUni = new TCHAR[nLen + 1];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, asc, strlen(asc), ptszUni, nLen);
	ptszUni[nLen] = '\0';
	return ptszUni;
}
void CAnimationObject::LoadAnimation(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ifstream& InFile, AnimationController* pAnimationController)
{
	UINT nAnimation = 0;
	UINT nBone = 0;
	CAnimationObject* pFindObjecct = GetRootObject();

	if (pFindObjecct)
	{
		InFile.read((char*)&nAnimation, sizeof(UINT)); // 애니메이션 수
		InFile.read((char*)&nBone, sizeof(UINT)); // 애니메이션 수
		pAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, nAnimation, nBone);

		for (int i = 0; i < pAnimationController->GetAnimationCount(); i++)
		{
			InFile.read((char*)&pAnimationController->m_pAnimation[i].nFrame, sizeof(UINT)); // 프레임 수 
			InFile.read((char*)&pAnimationController->m_pAnimation[i].fTime, sizeof(float)); // 애니메이션 시간 (30fps 기준)
			InFile.read((char*)&pAnimationController->m_pAnimation[i].nType, sizeof(UINT)); // 애니메이션 타입

			if (i == 12)
				pAnimationController->m_pAnimation[i].nType = 1;
			pAnimationController->m_pAnimation[i].pFrame
				= new FRAME[(pAnimationController->m_pAnimation[i].nFrame + 1) * pAnimationController->m_nBone];
			InFile.read((char*)pAnimationController->m_pAnimation[i].pFrame
				, sizeof(FRAME) * pAnimationController->m_nBone *
				(pAnimationController->m_pAnimation[i].nFrame + 1));
		}
		pFindObjecct->m_pAnimationController = pAnimationController;
	}
}
int CAnimationObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
	ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nType, UINT nSub, CMesh* pCMesh, UINT nObjects, UINT nAnimationFactor, ID3D12Resource* pShadowMap)
{
	int m_nType = -1;
	int nSubMesh = 0;
	static UINT nRendererMesh = 0;
	CMesh* pMesh = NULL;

	if (nType != SKIP)
		InFile.read((char*)&m_nType, sizeof(int)); // 타입 받기
	else
	{
		m_nType = SKINNEDMESH;
	}
	XMFLOAT3 *pxmf3ParentsPosition = NULL;

	if (m_nType == SKINNEDMESH || m_nType == RENDERMESH)
	{
		if (m_nType == RENDERMESH)
		{
			m_bRendererMesh = true;
			m_nRendererMesh = nRendererMesh++;
		}

		ResizeMeshes(1);

		XMFLOAT3 *pxmf3Positions = NULL;
		XMFLOAT2 *pxmf2TextureCoords0 = NULL;
		XMFLOAT3 *pxmf3Normals = NULL;
		XMFLOAT3 *pxmf3Tangents = NULL;
		XMFLOAT3 *pxmf3BoneWeights = NULL;
		XMINT4 *pxmi4BoneIndices = NULL;

		char* pstrAlbedoTextureName = NULL;
		char* pstrNormalTextureName = NULL;
		char* pstrSpecularTextureName = NULL;
		UINT *pIndices = NULL;

		int nVertices = 0, nIndices = 0, nSubIndices = 0, nStartIndex = 0;

		CMaterial *pMaterial = NULL;

		InFile.read((char*)&nSubMesh, sizeof(int)); // 서브매쉬 갯수 받기

		InFile.read((char*)&m_nSkinnedMeshRenderer, sizeof(int)); // 몇번째 스킨메쉬랜더러 인지 받기

		InFile.read((char*)&m_nDrawType, sizeof(int)); // 그리기 타입 받기

		InFile.read((char*)&nStartIndex, sizeof(int)); // 인덱스 시작 위치
		InFile.read((char*)&nSubIndices, sizeof(int)); // 서브인덱스 갯수 받기

		if ((nSubMesh > 1 && nSub == 1) || nSubMesh == 1)
		{
			InFile.read((char*)&nVertices, sizeof(int)); // 정점 갯수 받기
			InFile.read((char*)&nIndices, sizeof(int)); // 전체 인덱스 갯수 받기


			pIndices = new UINT[nIndices];
			pxmf3Positions = new XMFLOAT3[nVertices];
			pxmf2TextureCoords0 = new XMFLOAT2[nVertices];
			pxmf3Normals = new XMFLOAT3[nVertices];
			pxmf3Tangents = new XMFLOAT3[nVertices];
			pxmf3BoneWeights = new XMFLOAT3[nVertices];
			pxmi4BoneIndices = new XMINT4[nVertices];

			InFile.read((char*)pIndices, sizeof(UINT) * nIndices); // 인덱스 받기
			InFile.read((char*)pxmf3Positions, sizeof(XMFLOAT3) * nVertices); // 정점 받기
			InFile.read((char*)pxmf2TextureCoords0, sizeof(XMFLOAT2) * nVertices); // uv 받기

			if (m_nDrawType > 1)
			{
				InFile.read((char*)pxmf3Normals, sizeof(XMFLOAT3) * nVertices); // 법선 받기
				InFile.read((char*)pxmf3Tangents, sizeof(XMFLOAT3) * nVertices); // 탄젠트 받기
			}
			if (m_nType == SKINNEDMESH)
			{
				InFile.read((char*)pxmf3BoneWeights, sizeof(XMFLOAT3) * nVertices); // 본가중치 받기
				InFile.read((char*)pxmi4BoneIndices, sizeof(XMINT4) * nVertices); // 본인덱스 받기

				int nBindPoses;

				InFile.read((char*)&nBindPoses, sizeof(int)); // 본포즈 길이 받기

				if (m_pAnimationFactors == NULL)
					m_pAnimationFactors = new AnimationResourceFactors(pd3dDevice, pd3dCommandList, nBindPoses, nObjects);

				m_pBindPoses = new XMFLOAT4X4[nBindPoses];

				InFile.read((char*)m_pBindPoses, sizeof(XMFLOAT4X4) * nBindPoses); // 본포즈 받기
				InFile.read((char*)m_pAnimationFactors->m_pBoneIndexList, sizeof(UINT) * nBindPoses);

				GetRootObject()->m_ppAnimationFactorObjects[nAnimationFactor] = this;
				nAnimationFactor++;
			}


			if (nVertices > 0 && m_nDrawType > 1 && m_nType == SKINNEDMESH)
				pMesh = new CAnimationMesh(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf3Tangents
					, pxmf3Normals, pxmf2TextureCoords0, pxmf3BoneWeights, pxmi4BoneIndices, nIndices, nSubIndices, pIndices);
			else if (nVertices > 0 && m_nType == SKINNEDMESH)
				pMesh = new CMeshAnimationTextured(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf2TextureCoords0, pxmf3BoneWeights, pxmi4BoneIndices, nIndices, nSubIndices, pIndices);
			else if (m_nType == RENDERMESH)
				pMesh = new CRendererMesh(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf3Tangents, pxmf3Normals, pxmf2TextureCoords0, m_nRendererMesh, nIndices, nSubIndices, pIndices);

			pMesh->SetStartIndex(nStartIndex);

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
		}
		else if (nSubMesh >= nSub)
		{
			pMesh = new EmptyMesh(pCMesh->GetVertexBuffer(), pCMesh->GetIndexBuffer(), pCMesh->GetStride(), pCMesh->m_nVertices, nStartIndex, pCMesh->GetnIndices(), nSubIndices);

			if (pMesh)
				SetMesh(0, pMesh);
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
		if (m_nDrawType > 2)
		{
			InFile.read((char*)&Namesize, sizeof(int)); // 노말맵이름 받기
			pstrSpecularTextureName = new char[Namesize];
			InFile.read(pstrSpecularTextureName, sizeof(char)*Namesize);
		}

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

		if (m_nDrawType > 2)
		{
			pstrFileName = char2tchar(pstrSpecularTextureName);
			_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
			_tcscat_s(pstrPathName, 128, pstrFileName);
			_tcscat_s(pstrPathName, 128, _T(".dds"));

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 2);
		}

		if (pstrAlbedoTextureName)
			delete[] pstrAlbedoTextureName;

		if (pstrNormalTextureName)
			delete[] pstrNormalTextureName;

		if (pstrSpecularTextureName)
			delete[] pstrSpecularTextureName;

		pMaterial->SetTexture(pTexture);

		CShader* pShader = NULL;

		if (m_nDrawType == 1 && m_nType == SKINNEDMESH)
			pShader = new CDefferredTexturedShader();
		else if (m_nDrawType == 2 && m_nType == SKINNEDMESH)
			pShader = new CDefferredLightingTexturedShader();
		else if (m_nDrawType == 3 && m_nType == SKINNEDMESH)
			pShader = new CSpecularLightingTexturedShader();
		else if (m_nDrawType == 2 && m_nType == RENDERMESH)
			pShader = new CRendererMeshShader();
		else if (m_nDrawType == 3 && m_nType == RENDERMESH)
			pShader = new CRendererSpecularMeshShader();

		pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 1);
		pShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_nDrawType);
		pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 5, true, pShadowMap);
		SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());

		pMaterial->SetShader(pShader);

		if (pMaterial)
			SetMaterial(pMaterial);
	}

	{
		if (m_nType == RENDERMESH)
		{
			InFile.read((char*)&m_RndererScale, sizeof(float));
		}

		char* pstrFrameName = NULL;
		UINT nFrameNameSize;
		InFile.read((char*)&nFrameNameSize, sizeof(UINT));

		pstrFrameName = new char[nFrameNameSize];
		InFile.read(pstrFrameName, sizeof(char)*nFrameNameSize); // 프레임 이름 사이즈 받기

		TCHAR* pstrFileName = char2tchar(pstrFrameName);// 프레임 이름 받기
		_tcscat_s(m_strFrameName, 128, pstrFileName);

		InFile.read((char*)&m_nBoneType, sizeof(int));
		InFile.read((char*)&m_iBoneIndex, sizeof(int)); // 본 인덱스 받기

		delete[] pstrFrameName;

		if (nSubMesh <= nSub)
		{
			bool bChild = false;
			InFile.read((char*)&bChild, sizeof(bool));

			if (bChild)
			{
				int nChild = 0;
				InFile.read((char*)&nChild, sizeof(int));

				for (int i = 0; i < nChild; i++)
				{
					CAnimationObject *pChild = new CAnimationObject(0);
					SetChild(pChild);
					nAnimationFactor = pChild->LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile, GET_TYPE, 1, NULL, nObjects, nAnimationFactor, pShadowMap);
				}
			}
		}
		if (nSubMesh > nSub)
		{
			CAnimationObject *pSibling = new CAnimationObject(0);
			SetChild(pSibling);
			nAnimationFactor = pSibling->LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile, SKIP, ++nSub, pMesh, nObjects, nAnimationFactor, pShadowMap);

		}
	}

	return nAnimationFactor;
}
void CAnimationObject::LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName, AnimationController* pAnimationController, UINT nObjects, UINT& nAnimationFactor, ID3D12Resource* pShadowMap)
{
	ifstream fi;

	fi.open(pstrFileName, ostream::binary);

	if (!fi)
	{
		exit(1);
	}
	m_bRoot = true;

	fi.read((char*)&nAnimationFactor, sizeof(UINT));

	m_ppAnimationFactorObjects = new CAnimationObject*[nAnimationFactor];

	LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fi, GET_TYPE, 1, NULL, nObjects, 0, pShadowMap);

	bool bAnimation;
	fi.read((char*)&bAnimation, sizeof(bool));

	if (bAnimation)
	{
		m_pAnimationTime = new AnimationTimeFactor();
		LoadAnimation(pd3dDevice, pd3dCommandList, fi, pAnimationController);
	}
	fi.close(); // 파일 닫기
}
CAnimationObject* CAnimationObject::GetRootObject()
{
	CAnimationObject* pRootObject = this;

	while (1)
	{
		if (pRootObject->m_pParent == NULL)
			break;
		else
			pRootObject = pRootObject->m_pParent;
	}

	return pRootObject;
}

void CAnimationObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParentTransform, *pxmf4x4Parent) : m_xmf4x4ToParentTransform;

	if (m_pSibling)
		m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild)
		m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CAnimationObject::SetChild(CAnimationObject *pChild)
{
	if (m_pChild)
	{
		CAnimationObject* pLinkObject = m_pChild;
		while (1)
		{
			if (pLinkObject->m_pSibling)
			{
				pLinkObject = pLinkObject->m_pSibling;
			}
			else
			{
				pLinkObject->m_pSibling = pChild;
				break;
			}
		}
	}
	else
	{
		m_pChild = pChild;
	}
	if (pChild)
		pChild->m_pParent = this;
}

void CAnimationObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParentTransform._41 = x;
	m_xmf4x4ToParentTransform._42 = y;
	m_xmf4x4ToParentTransform._43 = z;
}
void CAnimationObject::SetPosition(XMFLOAT3& xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}
void CAnimationObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParentTransform);
}

void CAnimationObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void CAnimationObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void CAnimationObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}
void CAnimationObject::SetRotation(XMFLOAT4& pxmf4Quaternion)
{
	m_xmf4x4ToParentTransform._11 = pxmf4Quaternion.x;
	m_xmf4x4ToParentTransform._13 = pxmf4Quaternion.y;
	m_xmf4x4ToParentTransform._31 = pxmf4Quaternion.z;
	m_xmf4x4ToParentTransform._33 = pxmf4Quaternion.z;
}
void CAnimationObject::ChangeAnimation(int newState)
{
	m_pAnimationController->SetObject(this, NULL);
	if (m_pAnimationTime->m_iState == newState)
	{
		return;
	}
	if (m_pAnimationTime->m_iState == CHANG_INDEX 
		&& (m_pAnimationController->m_pAnimation[newState].nType != PLAY_NOW 
			&& m_pAnimationController->m_pAnimation[newState].nType != ALL))
		return;

	if (newState == PLAY_NOW)
		m_pAnimationTime->m_iState = 0;

	if (newState < 0) // 뒤로 재생되어야 하는 애니메이션이 맞는지 확인
	{
		if (m_pAnimationController->m_pAnimation[-newState].nType != CAN_BACK_PLAY)
			return;
	}
	if (m_pAnimationTime->m_iState >= 0)
	{
		if (m_pAnimationController->m_pAnimation[m_pAnimationTime->m_iState].nType == ALL)
		{
			if (m_pAnimationController->m_pAnimation[newState].nType != ALL
				&& m_pAnimationController->m_pAnimation[newState].nType != NOT_CYCLE)
				return;
		}
		if (m_pAnimationController->m_pAnimation[m_pAnimationTime->m_iState].nType == PLAY_NOW)
		{
			return;
		}
		if (m_pAnimationController->m_pAnimation[m_pAnimationTime->m_iState].nType == CONTINUOUS_PLAYBACK)
			return;

		if (m_pAnimationController->m_pAnimation[m_pAnimationTime->m_iState].nType == NOT_CYCLE)
			return;
	}
	else
	{
		m_pAnimationTime->m_iState = 0;
	}
	if (m_pAnimationController)
	{
		m_pAnimationController->ChangeAnimation(newState);
	}
}