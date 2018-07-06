#include "stdafx.h"
#include "Scene.h"
#include <stack>

CScene::CScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking)
	: m_pPxPhysicsSDK(pPxPhysicsSDK)
	, m_pPxScene(pPxScene)
	, m_pPxControllerManager(pPxControllerManager)
	, m_pPxMaterial(NULL)
	, m_pCooking(pCooking)
{
}

CScene::~CScene()
{
}
CAnimationObject* CScene::FindBoneTypeObject(CAnimationObject* pRootObject, UINT nBoneTypeMesh)
{
	stack<CAnimationObject*> FrameStack;

	FrameStack.push(pRootObject);

	while (!FrameStack.empty())
	{
		CAnimationObject* pFindObject = FrameStack.top();
		FrameStack.pop();

		if (pFindObject->IsRendererMesh() && nBoneTypeMesh == pFindObject->GetBoneType())
			return pFindObject;

		if (pFindObject->m_pChild)
			FrameStack.push(pFindObject->m_pChild);
		if (pFindObject->m_pSibling)
			FrameStack.push(pFindObject->m_pSibling);
	}
	return NULL;
}
void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CScene::ReleaseObjects()
{
}

pair<XMFLOAT3, XMFLOAT3> CScene::GetProjectilePos(UINT nShell)
{
	return pair<XMFLOAT3, XMFLOAT3>();
}

void CScene::ReleaseUploadBuffers()
{
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput(UCHAR *pKeysBuffer)
{
	return(false);
}

void CScene::FrustumCulling(CCamera* pCamera)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TerrainAndSkyBoxScene::TerrainAndSkyBoxScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking)
	: CScene(pPxPhysicsSDK, pPxScene, pPxControllerManager, pCooking)
{
	m_pPxPhysicsSDK = pPxPhysicsSDK;
	m_pPxScene = pPxScene;
	m_pPxControllerManager = pPxControllerManager;
	m_pCooking = pCooking;
}
TerrainAndSkyBoxScene::~TerrainAndSkyBoxScene()
{
	ReleaseObjects();
}
void TerrainAndSkyBoxScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	XMFLOAT3 xmf3Scale(8.0f, 2.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.5f, 0.0f, 0.0f);

	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		_T("../Assets/Image/Terrain/HeightMap2.raw"), 257, 257, 257, 257, xmf3Scale, xmf4Color,
		m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	//m_pTerrain->SetPosition(-);
	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
}
void TerrainAndSkyBoxScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature)
		m_pd3dGraphicsRootSignature->Release();

	if (m_pTerrain)
		delete m_pTerrain;

	if (m_pSkyBox)
		delete m_pSkyBox;
}
void TerrainAndSkyBoxScene::ReleaseUploadBuffers()
{
	if (m_pTerrain)
		m_pTerrain->ReleaseUploadBuffers();

	if (m_pSkyBox)
		m_pSkyBox->ReleaseUploadBuffers();
}
void TerrainAndSkyBoxScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{

}
void TerrainAndSkyBoxScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pSkyBox)
	{
		if (m_pSkyBox->m_pMaterial)
			m_pSkyBox->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
		if (m_pSkyBox->m_pMaterial)
			m_pSkyBox->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);
	
		m_pSkyBox->Render(pd3dCommandList, pCamera);
	}
	if (m_pTerrain)
	{
		if (m_pTerrain->m_pMaterial)
			m_pTerrain->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
		if (m_pTerrain->m_pMaterial)
			m_pTerrain->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);
		m_pTerrain->Render(pd3dCommandList, 2, pCamera);
	}
}
void TerrainAndSkyBoxScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	m_pTerrain->m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
	pCamera->ShadowUpdateShaderVariables(pd3dCommandList, pCameraInfo);
	m_pTerrain->ShadowRender(pd3dCommandList, 2);
}
ID3D12RootSignature *TerrainAndSkyBoxScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[5];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 4;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 5;
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 6;
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 9;
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[7];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Player
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1; //Object
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1; //Terrain(Base)
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1; //Terrain(Detail)
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1; //Terrain(Normal)
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3];
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1; //skyBoxTextrue
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UIScene::UIScene()
{
}
UIScene::~UIScene()
{
	ReleaseObjects();
}
void UIScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}
void UIScene::ReleaseObjects()
{
}
void UIScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
}
void UIScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CharacterScene::CharacterScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking)
	: CScene(pPxPhysicsSDK, pPxScene, pPxControllerManager, pCooking)
{
	m_ppAnimationController = new AnimationController*[MESH_NUM];
	m_ppSampleAnimationObjects = new CAnimationObject*[MESH_NUM];
}
CharacterScene::~CharacterScene()
{
	ReleaseObjects();
	ReleaseShaderVariables();
}
void CharacterScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_nObjects = 3;

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_ppSampleAnimationObjects[0] = new CAnimationObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0, L"../Assets/NewAstronaut.bss", m_ppAnimationController[0]);

	m_ppSoldierObjects = new CAnimationObject*[m_nObjects];

	CopyObject(m_ppSampleAnimationObjects[0], m_ppSoldierObjects, m_nObjects - 1);
	m_ppSoldierObjects[m_nObjects - 1] = m_ppSampleAnimationObjects[0];
	m_ppSoldierObjects[m_nObjects - 1]->SetPlayer(m_pPlayer);
	m_pPlayer->SetRenderObject(m_ppSoldierObjects[m_nObjects - 1]);
	
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppSoldierObjects[i]->m_pAnimationFactors->SetBoneObject(m_ppSoldierObjects[i]);
		m_ppSoldierObjects[i]->SetPosition(10, 200, 10);
		if (i != m_nObjects - 1)
			m_ppSoldierObjects[i]->SetScale(50.0f, 50.0f, 50.0f);
		else
			m_ppSoldierObjects[m_nObjects - 1]->SetPlayerScale(10);
	}
}
void CharacterScene::ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (PlayerArray[i].m_isconnected)
		{
			m_ppSoldierObjects[i]->m_xmf4x4ToParentTransform = PlayerArray[i].pos;
			m_ppSoldierObjects[i]->ChangeAnimation(PlayerArray[i].anim_state);

		}
	}
}
void CharacterScene::CopyObject(CAnimationObject* pSample, CAnimationObject** ppObjects, UINT nSize)
{
	if (nSize <= 0)
		return;

	stack<CAnimationObject*> FrameStack;
	stack<CAnimationObject*> ObjectsStack;

	FrameStack.push(pSample);
	CAnimationObject** ppSiblingObjects = new CAnimationObject*[nSize];
	CAnimationObject** ppChildObjects = new CAnimationObject*[nSize];
	CAnimationObject** ppEmptyObjects = new CAnimationObject*[nSize];
	CAnimationObject* TargetFrame = NULL;
	// -1 = ·çÆ®, 0 = ÇüÁ¦, 1 = ÀÚ½Ä

	for (int i = 0; i < nSize; i++)
	{
		ppObjects[i] = new CAnimationObject(0);
		ppEmptyObjects[i] = ppObjects[i];

		ppEmptyObjects[i]->m_BoneTransforms = new BONE_TRANSFORMS();
		ppEmptyObjects[i]->m_BoneTransforms2 = new BONE_TRANSFORMS2();
		ppEmptyObjects[i]->m_BoneTransforms3 = new BONE_TRANSFORMS2();
	}
	for (int i = 0; i < nSize; i++)
	{
		ObjectsStack.push(ppObjects[i]);
	}

	while (1)
	{
		if (FrameStack.empty())
			break;

		TargetFrame = FrameStack.top();
		FrameStack.pop();
		for (int i = 0; i < nSize; i++)
		{
			ppEmptyObjects[i] = ObjectsStack.top();
			ObjectsStack.pop();
		}
		for (int i = 0; i < nSize; i++)
		{
			*ppEmptyObjects[i] = *TargetFrame;
		}
		if (TargetFrame->m_pChild)
		{
			for (int i = 0; i < nSize; i++)
			{
				if (TargetFrame->m_pAnimationFactors)
				{
					ppEmptyObjects[i]->m_pAnimationFactors = new AnimationFactors(TargetFrame->m_pAnimationFactors->m_nBindpos);
				}
				ppChildObjects[i] = new CAnimationObject(0);
				ppEmptyObjects[i]->m_pChild = ppChildObjects[i];
				ppChildObjects[i]->m_pParent = ppEmptyObjects[i];
			}
		}
		if (TargetFrame->m_pSibling)
		{
			for (int i = 0; i < nSize; i++)
			{
				if (TargetFrame->m_pAnimationFactors)
				{
					ppEmptyObjects[i]->m_pAnimationFactors = new AnimationFactors(TargetFrame->m_pAnimationFactors->m_nBindpos);
				}
				ppChildObjects[i] = new CAnimationObject(0);
				ppEmptyObjects[i]->m_pSibling = ppChildObjects[i];
				ppChildObjects[i]->m_pParent = ppEmptyObjects[i]->m_pParent;
			}
		}

		if (TargetFrame->m_pChild)
		{
			for (int i = 0; i < nSize; i++)
			{
				ObjectsStack.push(ppEmptyObjects[i]->m_pChild);
			}
			FrameStack.push(TargetFrame->m_pChild);
		}
		if (TargetFrame->m_pSibling)
		{
			for (int i = 0; i < nSize; i++)
			{
				ObjectsStack.push(ppEmptyObjects[i]->m_pSibling);
			}
			FrameStack.push(TargetFrame->m_pSibling);
		}
	}
	delete[] ppEmptyObjects;
	delete[] ppSiblingObjects;
	delete[] ppChildObjects;
}
void CharacterScene::ReleaseObjects()
{
	if (m_ppSampleAnimationObjects)
	{
		for (int i = 0; i<MESH_NUM; i++)
			delete m_ppSampleAnimationObjects[i];

		delete[] m_ppSampleAnimationObjects;
	}

}
void CharacterScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < MESH_NUM; i++)
	{
		m_ppSampleAnimationObjects[i]->ReleaseUploadBuffers();
	}
}
void CharacterScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(BONE_TRANSFORMS) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * m_nObjects
		, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void **)&m_pcbMappedGameObjects);
	
	UINT ncbElementBytes2 = ((sizeof(BONE_TRANSFORMS2) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbGameObjects2 = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes2 * m_nObjects
		, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects2->Map(0, NULL, (void **)&m_pcbMappedGameObjects2);

	m_pd3dcbGameObjects3 = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes2 * m_nObjects
		, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects3->Map(0, NULL, (void **)&m_pcbMappedGameObjects3);
}
void CharacterScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(BONE_TRANSFORMS) + 255) & ~255);
	UINT ncbElementBytes2 = ((sizeof(BONE_TRANSFORMS2) + 255) & ~255);
	UINT ncbElementBytes3 = ((sizeof(BONE_TRANSFORMS3) + 255) & ~255);

	pd3dCommandList->SetGraphicsRootShaderResourceView(8, m_pd3dcbGameObjects->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootShaderResourceView(9, m_pd3dcbGameObjects2->GetGPUVirtualAddress());
	pd3dCommandList->SetGraphicsRootShaderResourceView(10, m_pd3dcbGameObjects3->GetGPUVirtualAddress());

	for (int i = 0; i < m_nObjects; i++)
	{
		BONE_TRANSFORMS *pbMappedcbGameObject = (BONE_TRANSFORMS *)((UINT8 *)m_pcbMappedGameObjects + (i * ncbElementBytes));
		XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_ppSoldierObjects[i]->m_xmf4x4World)));
		::memcpy(pbMappedcbGameObject->m_xmf4x4BoneTransform, &m_ppSoldierObjects[i]->m_BoneTransforms->m_xmf4x4BoneTransform, sizeof(XMFLOAT4X4) * BONE_TRANSFORM_NUM);

		BONE_TRANSFORMS2*pbMappedcbGameObject2 = (BONE_TRANSFORMS2 *)((UINT8 *)m_pcbMappedGameObjects2 + (i * ncbElementBytes2));
		::memcpy(pbMappedcbGameObject2->m_xmf4x4BoneTransform, &m_ppSoldierObjects[i]->m_BoneTransforms2->m_xmf4x4BoneTransform, sizeof(XMFLOAT4X4) * BONE_TRANSFORM_NUM2);

		BONE_TRANSFORMS3*pbMappedcbGameObject3 = (BONE_TRANSFORMS3 *)((UINT8 *)m_pcbMappedGameObjects3 + (i * ncbElementBytes3));
		::memcpy(pbMappedcbGameObject3->m_xmf4x4BoneTransform, &m_ppSoldierObjects[i]->m_BoneTransforms3->m_xmf4x4BoneTransform, sizeof(XMFLOAT4X4) * BONE_TRANSFORM_NUM3);
		
		for (int k = 0; k < RENDERER_MESH_WORLD_TRANSFORM; k++)
		{
			CAnimationObject* RendererMeshObject = FindMeshRendererObject(m_ppSoldierObjects[i], k);
			if (RendererMeshObject)
			{
				XMFLOAT4X4 result = Matrix4x4::Multiply(RendererMeshObject->m_xmf4x4ToRootTransform, m_ppSoldierObjects[i]->m_xmf4x4World);
				
				XMStoreFloat4x4(&pbMappedcbGameObject3->m_xmf4x4RedererMeshWorld[k], XMMatrixTranspose(XMLoadFloat4x4(&result)));
			}
		}
	}
}

CAnimationObject* CharacterScene::FindMeshRendererObject(CAnimationObject* pRootObject, UINT nRendererMesh)
{
	stack<CAnimationObject*> FrameStack;

	FrameStack.push(pRootObject);

	while (!FrameStack.empty())
	{
		CAnimationObject* pFindObject = FrameStack.top();
		FrameStack.pop();

		if (pFindObject->IsRendererMesh() && nRendererMesh == pFindObject->GetRendererMeshNum())
			return pFindObject;

		if (pFindObject->m_pChild)
			FrameStack.push(pFindObject->m_pChild);
		if (pFindObject->m_pSibling)
			FrameStack.push(pFindObject->m_pSibling);
	}
	return NULL;
}
void CharacterScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
}
void CharacterScene::ChangeAnimation(int newState)
{
	m_ppSoldierObjects[m_nObjects - 1]->m_pAnimationController->SetObject(m_ppSoldierObjects[m_nObjects - 1]);
	m_ppSoldierObjects[m_nObjects - 1]->ChangeAnimation(newState);
}
void CharacterScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	for (int k = 0; k < MESH_NUM; k++)
	{
		if (m_ppSampleAnimationObjects[k]->m_pMaterial)
			m_ppSampleAnimationObjects[k]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
		
		pCamera->SetViewportsAndScissorRects(pd3dCommandList);
		pCamera->UpdateShaderVariables(pd3dCommandList);

		FrustumCulling(pCamera);

		m_pPlayer->OnPrepareRender();

		for (int i = 0; i < m_nObjects; i++)
		{
			m_ppSoldierObjects[i]->m_pAnimationController->SetBindPoses(m_ppSampleAnimationObjects[k]->GetBindPoses());
			m_ppSoldierObjects[i]->m_pAnimationController->SetObject(m_ppSoldierObjects[i]);
			m_ppSoldierObjects[i]->m_pAnimationController->AdvanceAnimation(pd3dCommandList);

			if (i == m_nObjects - 1)
			{
				m_ppSoldierObjects[m_nObjects - 1]->m_xmf4x4ToParentTransform = m_pPlayer->m_xmf4x4World;
				XMFLOAT4X4 Scale = Matrix4x4::Identity();
				Scale._11 = Scale._22 = Scale._33 = m_ppSoldierObjects[m_nObjects - 1]->GetPlayerScale();
				m_ppSoldierObjects[m_nObjects - 1]->m_xmf4x4ToParentTransform = Matrix4x4::Multiply(Scale, m_ppSoldierObjects[m_nObjects - 1]->m_xmf4x4ToParentTransform);
			}
			m_ppSoldierObjects[i]->UpdateTransform(NULL);
		}

		UpdateShaderVariables(pd3dCommandList);

		m_ppSampleAnimationObjects[k]->Render(pd3dCommandList, pCamera, m_nObjects);
	}
}
void CharacterScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->ShadowUpdateShaderVariables(pd3dCommandList, pCameraInfo);

	for (int k = 0; k < MESH_NUM; k++)
	{
		UpdateShaderVariables(pd3dCommandList);
		m_ppSampleAnimationObjects[k]->ShadowRender(pd3dCommandList, pCamera, m_nObjects);
	}
}
void CharacterScene::ReleaseShaderVariables()
{
	m_pd3dcbGameObjects->Unmap(0, NULL);
	m_pd3dcbGameObjects->Release();

	m_pd3dcbGameObjects2->Unmap(0, NULL);
	m_pd3dcbGameObjects2->Release();

	m_pd3dcbGameObjects3->Unmap(0, NULL);
	m_pd3dcbGameObjects3->Release();
}
ID3D12RootSignature *CharacterScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[7];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8;
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 10; // ¸ðµ¨ µðÇ»Áî¸Ê
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 11; // ¸ðµ¨ ³ë¸»¸Ê
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 12; // ¸ðµ¨ ½ºÆåÅ§·¯¸Ê
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 13;
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[11];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Player
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1; //Object
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];//Test
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];//Test
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3];
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5];
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[8].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[8].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[9].Descriptor.ShaderRegister = 1;
	pd3dRootParameters[9].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[10].Descriptor.ShaderRegister = 2;
	pd3dRootParameters[10].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[1];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ObjectScene::ObjectScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking)
	: CScene(pPxPhysicsSDK, pPxScene, pPxControllerManager, pCooking)
{
	m_ppSampleObjects = new CPhysXObject*[MESH_NUM2];
	m_fReloadtime = GetTickCount();
}
ObjectScene::~ObjectScene()
{
	ReleaseObjects();
	ReleaseShaderVariables();
}
int ObjectScene::SetProjectile(XMFLOAT3& xmf3Direction)
{
	CAnimationObject* pFindObject = FindBoneTypeObject(m_pPlayer->GetRenderObject(), 1);

	XMFLOAT4X4 WorldTransform = pFindObject->m_xmf4x4World;
	XMFLOAT3 xmf3Look = XMFLOAT3(WorldTransform._31, WorldTransform._32, WorldTransform._33);
	float Scalar = 0.78f;
	UINT nShell;
	XMFLOAT3 xmf3ShellRight = XMFLOAT3(0, 0, 0);
	XMFLOAT3 xmf3ShellUp = XMFLOAT3(m_pPlayer->m_xmf4x4World._21, m_pPlayer->m_xmf4x4World._22, m_pPlayer->m_xmf4x4World._23);
	XMFLOAT3 xmf3ShellLook = XMFLOAT3(xmf3Direction.x, xmf3Direction.y, xmf3Direction.z);

	xmf3ShellUp = Vector3::Normalize(xmf3ShellUp);
	xmf3ShellLook = Vector3::Normalize(xmf3ShellLook);

	xmf3ShellRight = Vector3::CrossProduct(xmf3ShellUp, xmf3ShellLook);

	if ((GetTickCount() - m_fReloadtime) > (DWORD)550)
	{
		for (int i = 0; i < m_nObjects; i++)
		{
			if (!m_ppShell[i]->m_bShooted)
			{
				m_ppShell[i]->m_xmf4x4World._11 = xmf3ShellRight.x;
				m_ppShell[i]->m_xmf4x4World._12 = xmf3ShellRight.y;
				m_ppShell[i]->m_xmf4x4World._13 = xmf3ShellRight.z;

				m_ppShell[i]->m_xmf4x4World._21 = xmf3ShellUp.x;
				m_ppShell[i]->m_xmf4x4World._22 = xmf3ShellUp.y;
				m_ppShell[i]->m_xmf4x4World._23 = xmf3ShellUp.z;

				m_ppShell[i]->m_xmf4x4World._31 = xmf3ShellLook.x;
				m_ppShell[i]->m_xmf4x4World._32 = xmf3ShellLook.y;
				m_ppShell[i]->m_xmf4x4World._33 = xmf3ShellLook.z;

				XMFLOAT4X4 Scale = Matrix4x4::Identity();
				Scale._11 = Scale._22 = Scale._33 = 100;
				m_ppShell[i]->m_xmf4x4World = Matrix4x4::Multiply(m_ppShell[i]->m_xmf4x4World, Scale);

				XMFLOAT3 xmf3Position = XMFLOAT3(WorldTransform._41 + Scalar * xmf3Look.x, WorldTransform._42 + Scalar * xmf3Look.y, WorldTransform._43 + Scalar * xmf3Look.z);

				m_ppShell[i]->SetPosition(xmf3Position);
				m_ppShell[i]->m_Direction = xmf3ShellLook;
				m_ppShell[i]->m_bShooted = true;
				m_ppShell[i]->m_dAlivedtime = GetTickCount();
				for (int b = 0; b < m_vBullet.size(); b++)
				{
					if (m_vBullet[b] == NULL)
					{
						m_vBullet[b] = m_ppShell[i];
						nShell = b;
						break;
					}
				}
				m_fReloadtime = GetTickCount();
				printf("¹ß»ç: %lf, %lf, %lf\n", xmf3Position.x, xmf3Position.y, xmf3Position.z);
				break;
			}
		}
	}
	return nShell;
}
void ObjectScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_nObjects = 8;

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	//PxMaterial : Ç¥¸é Æ¯¼º ÁýÇÕÀ» ³ªÅ¸³»´Â ÀçÁú Å¬·¡½º
	m_pPxMaterial = m_pPxPhysicsSDK->createMaterial(0.5f, 0.5f, 0.2f); //1.Á¤Áö ¸¶Âû°è¼ö ¿îµ¿¸¶Âû°è¼ö, ¹Ý¹ß°è¼ö
																	   //Plane¹Ù´Ú »ý¼º
	PxRigidStatic* groundPlane = PxCreatePlane(*m_pPxPhysicsSDK, PxPlane(0, 1, 0, 0), *m_pPxMaterial);
	m_pPxScene->addActor(*groundPlane);
	
	m_ppSampleObjects[0] = new CPhysXObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Shell.bss", m_nObjects);
	m_ppSampleObjects[0]->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(1, 1, 1), m_pCooking, "Bullet");
	m_ppSampleObjects[1] = new CPhysXObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid5.bss", m_nObjects);
	m_ppSampleObjects[1]->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(1, 1, 1), m_pCooking, "Astroid0");
	m_ppSampleObjects[2] = new CPhysXObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid1.bss", m_nObjects);
	m_ppSampleObjects[2]->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(1, 1, 1), m_pCooking, "Astroid1");
	m_ppSampleObjects[3] = new CPhysXObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid2.bss", m_nObjects);
	m_ppSampleObjects[3]->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(1, 1, 1), m_pCooking, "Astroid2");
	m_ppSampleObjects[4] = new CPhysXObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid3.bss", m_nObjects);
	m_ppSampleObjects[4]->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(1, 1, 1), m_pCooking, "Astroid3");
	m_ppSampleObjects[5] = new CPhysXObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid4.bss", m_nObjects);
	m_ppSampleObjects[5]->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, XMFLOAT3(1, 1, 1), m_pCooking, "Astroid4");
	
	m_ppShell = new CPhysXObject*[m_nObjects];
	m_ppAsteroid0 = new CPhysXObject*[m_nObjects];
	m_ppAsteroid1 = new CPhysXObject*[m_nObjects];
	m_ppAsteroid2 = new CPhysXObject*[m_nObjects];
	m_ppAsteroid3 = new CPhysXObject*[m_nObjects];
	m_ppAsteroid4 = new CPhysXObject*[m_nObjects];

	m_vBullet.reserve(m_nObjects);
	m_vBullet.resize(m_nObjects);

	for (auto& bullet : m_vBullet)
		bullet = NULL;

	for (int i = 0; i < m_nObjects - 1; i++)
	{
		CPhysXObject* pShell = new CPhysXObject(0);
		pShell->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, 
			XMFLOAT3(10, 10, 10), m_pCooking, "Bullet");
		m_ppShell[i] = pShell;

		CPhysXObject* pAsteroid = new CPhysXObject(0);
		pAsteroid->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, 
			XMFLOAT3(1, 1, 1), m_pCooking, "Astroid0");
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid0[i] = pAsteroid;

		pAsteroid = new CPhysXObject(0);
		pAsteroid->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, 
			XMFLOAT3(1, 1, 1), m_pCooking, "Astroid1");
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid1[i] = pAsteroid;

		pAsteroid = new CPhysXObject(0);
		pAsteroid->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, 
			XMFLOAT3(1, 1, 1), m_pCooking, "Astroid2");
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid2[i] = pAsteroid;

		pAsteroid = new CPhysXObject(0);
		pAsteroid->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, 
			XMFLOAT3(1, 1, 1), m_pCooking, "Astroid3");
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid3[i] = pAsteroid;

		pAsteroid = new CPhysXObject(0);
		pAsteroid->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, 
			XMFLOAT3(1, 1, 1), m_pCooking, "Astroid4");
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid4[i] = pAsteroid;
	}
	m_ppSampleObjects[0]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[1]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[2]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[3]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[4]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[5]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	
	m_ppAsteroid0[m_nObjects - 1] = m_ppSampleObjects[1];
	m_ppAsteroid1[m_nObjects - 1] = m_ppSampleObjects[2];
	m_ppAsteroid2[m_nObjects - 1] = m_ppSampleObjects[3];
	m_ppAsteroid3[m_nObjects - 1] = m_ppSampleObjects[4];
	m_ppAsteroid4[m_nObjects - 1] = m_ppSampleObjects[5];

	m_ppShell[m_nObjects - 1] = m_ppSampleObjects[0];
	m_ppShell[m_nObjects - 1]->CGameObject::SetPosition(0, 0, 0);
}
void ObjectScene::ReleaseObjects()
{
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppShell[i])
			delete m_ppShell[i];
	}

	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppAsteroid0[i])
			delete m_ppAsteroid0[i];
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppAsteroid1[i])
			delete m_ppAsteroid1[i];
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppAsteroid2[i])
			delete m_ppAsteroid2[i];
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppAsteroid3[i])
			delete m_ppAsteroid3[i];
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppAsteroid4[i])
			delete m_ppAsteroid4[i];
	}

	delete[] m_ppShell;
	delete[] m_ppAsteroid0;
	delete[] m_ppAsteroid1;
	delete[] m_ppAsteroid2;
	delete[] m_ppAsteroid3;
	delete[] m_ppAsteroid4;
}
void ObjectScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	for (int i = 0; i < m_vBullet.size(); i++)
	{
		if (m_vBullet[i])
		{
			if (m_vBullet[i]->m_bShooted)
			{
				m_vBullet[i]->UpdateDirectPos(m_vBullet[i]->GetPosition(), m_vBullet[i]->m_Direction);
				if ((GetTickCount() - m_vBullet[i]->m_dAlivedtime) > 300000000)
				{
					m_vBullet[i]->m_bShooted = false;
					m_vBullet[i]->SetPosition(XMFLOAT3(0, 0, 0));
					m_vBullet[i] = NULL;
				}
			}
		}
	}

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppAsteroid0[i]->GetPhysXPos();
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppAsteroid1[i]->GetPhysXPos();
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppAsteroid2[i]->GetPhysXPos();
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppAsteroid3[i]->GetPhysXPos();
	}
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppAsteroid4[i]->GetPhysXPos();
	}
}
pair<XMFLOAT3, XMFLOAT3> ObjectScene::GetProjectilePos(UINT nShell)
{
	pair<XMFLOAT3, XMFLOAT3> Projectile;
	int i = 0; 

	if (nShell >= m_vBullet.size())
		nShell = m_vBullet.size() - 1;

	Projectile.first = XMFLOAT3(m_vBullet[nShell]->GetPosition().x, m_vBullet[nShell]->GetPosition().y, m_vBullet[nShell]->GetPosition().z);
	Projectile.second = XMFLOAT3(m_vBullet[nShell]->m_Direction.x, m_vBullet[nShell]->m_Direction.y, m_vBullet[nShell]->m_Direction.z);
	
	return Projectile;
}
void ObjectScene::SetVectorProjectile(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction)
{
	printf("VectorProjectile\n");
	CAnimationObject* pFindObject = FindBoneTypeObject(m_pPlayer->GetRenderObject(), 1);

	XMFLOAT4X4 WorldTransform = pFindObject->m_xmf4x4World;
	XMFLOAT3 xmf3Look = XMFLOAT3(WorldTransform._31, WorldTransform._32, WorldTransform._33);
	float Scalar = 0.78f;

	XMFLOAT3 xmf3ShellRight = XMFLOAT3(0, 0, 0);
	XMFLOAT3 xmf3ShellUp = XMFLOAT3(m_pPlayer->m_xmf4x4World._21, m_pPlayer->m_xmf4x4World._22, m_pPlayer->m_xmf4x4World._23);
	XMFLOAT3 xmf3ShellLook = XMFLOAT3(xmf3Direction.x, xmf3Direction.y, xmf3Direction.z);

	xmf3ShellUp = Vector3::Normalize(xmf3ShellUp);
	xmf3ShellLook = Vector3::Normalize(xmf3ShellLook);

	xmf3ShellRight = Vector3::CrossProduct(xmf3ShellUp, xmf3ShellLook);

	for (int i = 0; i < m_nObjects; i++)
	{
		if (!m_ppShell[i]->m_bShooted)
		{
			m_ppShell[i]->m_xmf4x4World._11 = xmf3ShellRight.x;
			m_ppShell[i]->m_xmf4x4World._12 = xmf3ShellRight.y;
			m_ppShell[i]->m_xmf4x4World._13 = xmf3ShellRight.z;

			m_ppShell[i]->m_xmf4x4World._21 = xmf3ShellUp.x;
			m_ppShell[i]->m_xmf4x4World._22 = xmf3ShellUp.y;
			m_ppShell[i]->m_xmf4x4World._23 = xmf3ShellUp.z;

			m_ppShell[i]->m_xmf4x4World._31 = xmf3ShellLook.x;
			m_ppShell[i]->m_xmf4x4World._32 = xmf3ShellLook.y;
			m_ppShell[i]->m_xmf4x4World._33 = xmf3ShellLook.z;

			XMFLOAT4X4 Scale = Matrix4x4::Identity();
			Scale._11 = Scale._22 = Scale._33 = 100;
			m_ppShell[i]->m_xmf4x4World = Matrix4x4::Multiply(m_ppShell[i]->m_xmf4x4World, Scale);

			m_ppShell[i]->SetPosition(xmf3Position);
			m_ppShell[i]->m_Direction = xmf3Direction;
			m_ppShell[i]->m_bShooted = true;
			m_vBullet.push_back(m_ppShell[i]);
			break;
		}
	}
}
void ObjectScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < MESH_NUM2; i++)
	{
		m_ppSampleObjects[i]->ReleaseUploadBuffers();
	}
}
void ObjectScene::ReleaseShaderVariables()
{
}
void ObjectScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppShell[m_nObjects - 1]->m_pMaterial)
		m_ppShell[m_nObjects - 1]->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	FrustumCulling(pCamera);

	if (m_ppShell[m_nObjects - 1]->m_pMaterial)
		m_ppShell[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppShell[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects * 4, m_ppShell);

	if (m_ppAsteroid0[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid0[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid0[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects * 4, m_ppAsteroid0);

	if (m_ppAsteroid1[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid1[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid1[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects * 4, m_ppAsteroid1);

	if (m_ppAsteroid2[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid2[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid2[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects * 4, m_ppAsteroid2);

	if (m_ppAsteroid3[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid3[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid3[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects * 4, m_ppAsteroid3);

	if (m_ppAsteroid4[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid4[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid4[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects * 4, m_ppAsteroid4);
}
void ObjectScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->ShadowUpdateShaderVariables(pd3dCommandList, pCameraInfo);

	m_ppShell[m_nObjects - 1]->m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
	m_ppShell[m_nObjects - 1]->ShadowRender(pd3dCommandList, 5, m_nObjects * 4, m_ppShell);

	m_ppAsteroid0[m_nObjects - 1]->m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
	m_ppAsteroid0[m_nObjects - 1]->ShadowRender(pd3dCommandList, 5, m_nObjects * 4, m_ppAsteroid0);

	m_ppAsteroid1[m_nObjects - 1]->m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
	m_ppAsteroid1[m_nObjects - 1]->ShadowRender(pd3dCommandList, 5, m_nObjects * 4, m_ppAsteroid1);

	m_ppAsteroid2[m_nObjects - 1]->m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
	m_ppAsteroid2[m_nObjects - 1]->ShadowRender(pd3dCommandList, 5, m_nObjects * 4, m_ppAsteroid2);

	m_ppAsteroid3[m_nObjects - 1]->m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
	m_ppAsteroid3[m_nObjects - 1]->ShadowRender(pd3dCommandList, 5, m_nObjects * 4, m_ppAsteroid3);

	m_ppAsteroid4[m_nObjects - 1]->m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
	m_ppAsteroid4[m_nObjects - 1]->ShadowRender(pd3dCommandList, 5, m_nObjects * 4, m_ppAsteroid4);
}
ID3D12RootSignature *ObjectScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; // ¸ðµ¨ µðÇ»Áî¸Ê
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1; // ¸ðµ¨ ³ë¸»¸Ê
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 2; // ¸ðµ¨ ½ºÆåÅ§·¯¸Ê
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[6];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Player
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //µðÇ»Áî
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //³ë¸»
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2]; //½ºÆåÅ§·¯
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV; // ÀÎ½ºÅÏ½Ì
	pd3dRootParameters[5].Descriptor.ShaderRegister = 3;
	pd3dRootParameters[5].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[1];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}