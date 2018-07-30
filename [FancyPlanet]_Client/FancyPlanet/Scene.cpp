#include "stdafx.h"
#include "Scene.h"
#include <stack>

CScene::CScene()
{
}

CScene::~CScene()
{
	ReleaseObjects();
}
CAnimationObject* CScene::FindBoneTypeObject(CAnimationObject* pRootObject, UINT nBoneTypeMesh)
{
	stack<CAnimationObject*> FrameStack;

	FrameStack.push(pRootObject);

	while (!FrameStack.empty())
	{
		CAnimationObject* pFindObject = FrameStack.top();
		FrameStack.pop();

		if (nBoneTypeMesh == pFindObject->GetBoneType())
			return pFindObject;

		if (pFindObject->m_pChild)
			FrameStack.push(pFindObject->m_pChild);
		if (pFindObject->m_pSibling)
			FrameStack.push(pFindObject->m_pSibling);
	}
	return NULL;
}
void CScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType)
{
}

void CScene::SetLightsAndMaterialsShaderVariables(ID3D12Resource* pLights, ID3D12Resource* pMaterials, ID3D12Resource* pShadowMap)
{
	m_pd3dcbLights = pLights;
	m_pd3dcbMaterials = pMaterials;
	m_pd3dDepthStencilBuffer = pShadowMap;
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

TerrainAndSkyBoxScene::TerrainAndSkyBoxScene()
{
}
TerrainAndSkyBoxScene::~TerrainAndSkyBoxScene()
{
	ReleaseObjects();
}
void TerrainAndSkyBoxScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	XMFLOAT3 xmf3Scale(8.0f, 8.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.5f, 0.0f, 0.0f);

	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		_T("../Assets/Image/Terrain/HeightMap2.raw", ), 257, 257, 257, 257, xmf3Scale, xmf4Color, pShadowMap);

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

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(8, d3dcbLightsGpuVirtualAddress);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(7, d3dcbMaterialsGpuVirtualAddress);

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

		pd3dCommandList->SetGraphicsRootDescriptorTable(9, m_pTerrain->m_pMaterial->m_pShader->GetShadowGPUDescriptorHandle());

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

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[6];

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

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 10;
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[10];

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

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[7].Descriptor.ShaderRegister = 7; //Materials
	pd3dRootParameters[7].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[8].Descriptor.ShaderRegister = 8; //Lights
	pd3dRootParameters[8].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5];
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[3];

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

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 16;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

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

EffectScene::EffectScene()
{
	m_nMeshes = 3;
	m_ppMeshes = new CMesh*[m_nMeshes];
}
EffectScene::~EffectScene()
{
	ReleaseObjects();
}
void EffectScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_pFlareShader = new CEffectShader();
	m_pFlareShader->BuildShader(pd3dDevice, pd3dCommandList);
	m_pFlareShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 1);

	m_pMeshEffectShader = new MeshEffectShader();
	m_pMeshEffectShader->BuildShader(pd3dDevice, pd3dCommandList);
	m_pMeshEffectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 1);

	m_ppMeshes[0] = new CRectMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 2.01f, 2.01f, 0.0f);
	m_ppMeshes[1] = new CRectMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 1.51f, 1.51f, 0.0f);
	m_ppMeshes[2] = new CCubeMesh(pd3dDevice, pd3dCommandList, 0.5f, 0.5f, 1.0f);

	m_ppDroneEffect = new DroneEffect*[m_nObjects];

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppDroneEffect[i] = new DroneEffect(1, 100, m_ppDrone[i]);
		m_ppDroneEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppDroneEffect[i]->SetMesh(m_ppMeshes[1], 0);
	}

	m_pSparkEffect = new SparkEffect(1, 10, m_ppSoldier[m_nObjects - 1]);
	m_pSparkEffect->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	m_pMeshEffect = new MeshEffect(1, 1, m_ppDrone[m_nObjects - 1]);
	m_pMeshEffect->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		
	m_pSparkEffect->SetMesh(m_ppMeshes[0], 0);
	m_pMeshEffect->SetMesh(m_ppMeshes[2], 0);
}
void EffectScene::ReleaseObjects()
{
	if(m_ppSoldier)
		delete[] m_ppSoldier;
	if(m_ppDrone)
		delete[]m_ppDrone;

	if(m_ppDroneEffect)
		delete[] m_ppDroneEffect;
	if(m_pSparkEffect)
		delete m_pSparkEffect;
	if(m_pMeshEffect)
		delete m_pMeshEffect;

	for (int i = 0; i < m_nMeshes; i++)
		m_ppMeshes[i]->Release();

	delete[] m_ppMeshes;
}
void EffectScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	DWORD dwTime = GetTickCount();

	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_pFlareShader)
		m_pFlareShader->OnPrepareRender(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);
	
	//printf("ÇÃ·¹ÀÌ¾î À§Ä¡: %lf, %lf, %lf\n", m_pPlayer->GetPosition().x, m_pPlayer->GetPosition().y, m_pPlayer->GetPosition().z);

	m_pSparkEffect->Animate(m_pPlayer->GetPosition().y, dwTime);
	Update(pCamera);

	pd3dCommandList->OMSetStencilRef(1);
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppDroneEffect[i]->Animate(0, dwTime);
		m_ppDroneEffect[i]->Render(pd3dCommandList, 0);
	}
	m_pSparkEffect->Render(pd3dCommandList, 0);

	if (m_pMeshEffectShader)
		m_pMeshEffectShader->OnPrepareRender(pd3dCommandList);

	m_pMeshEffect->Aniamate(dwTime);
	m_pMeshEffect->Render(pd3dCommandList, 0);
}
void EffectScene::SetParticle(UINT nType, int nCharacterType)
{
	if (nType == 0)
	{
		if (nCharacterType == 0)
		{
			float Scalar = 10.0f;
			CAnimationObject* pFindObject = FindBoneTypeObject(m_pPlayer->GetRenderObject(), 1);
			XMFLOAT3 p = m_pPlayer->GetLook();

			XMFLOAT4X4 WorldTransform = pFindObject->m_xmf4x4World;
			XMFLOAT3 xmf3Position = XMFLOAT3(WorldTransform._41 + Scalar * p.x, WorldTransform._42 + Scalar * p.y, WorldTransform._43 + Scalar * p.z);

			m_pSparkEffect->SetParticle(0, xmf3Position, p);
		}
		else if (nCharacterType == 1)
		{
			XMFLOAT3 pos = m_ppSoldier[m_nObjects - 1]->GetPosition();
			pos.y -= 10.0f;
			m_pMeshEffect->SetTarget(pos);
		}
	}
}
void EffectScene::Update(CCamera* pCamera)
{
	XMFLOAT3 xmf3CP = pCamera->GetPosition();
	XMFLOAT3 xmf3CR = pCamera->GetRightVector();
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppDroneEffect[i]->SetLookAt(xmf3CP, xmf3CR);
	}
	m_pSparkEffect->SetLookAt(xmf3CP, xmf3CR);
}

void EffectScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
}

ID3D12RootSignature *EffectScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[6];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 2;
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[4].Descriptor.ShaderRegister = 7; //Materials
	pd3dRootParameters[4].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[5].Descriptor.ShaderRegister = 8; //Lights
	pd3dRootParameters[5].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
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

CharacterScene::CharacterScene()
{
	m_ppAnimationController = new AnimationController*[MESH_NUM];
	m_ppSampleAnimationObjects = new CAnimationObject*[MESH_NUM];
	m_pnAnimationFactor = new UINT[MESH_NUM];
}
CharacterScene::~CharacterScene()
{
	ReleaseObjects();
	ReleaseShaderVariables();
}
CAnimationObject* CharacterScene::FindAnimationFactorObject(CAnimationObject* pRootObject, int nIndex)
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
void CharacterScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_nObjects = 2;

	m_ppSampleAnimationObjects[0] = new CAnimationObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0, L"../Assets/SpaceSoldier.bss", m_ppAnimationController[0], m_nObjects, m_pnAnimationFactor[0], pShadowMap);
	m_ppSampleAnimationObjects[1] = new CAnimationObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0, L"../Assets/SpaceDrone.bss", m_ppAnimationController[1], m_nObjects, m_pnAnimationFactor[1], pShadowMap);

	m_ppSoldierObjects = new CAnimationObject*[m_nObjects];
	m_ppDroneObjects = new CAnimationObject*[m_nObjects];

	CopyObject(pd3dDevice, pd3dCommandList, m_ppSampleAnimationObjects[0], m_ppSoldierObjects, m_nObjects - 1, 0);
	CopyObject(pd3dDevice, pd3dCommandList, m_ppSampleAnimationObjects[1], m_ppDroneObjects, m_nObjects - 1, 1);

	m_ppSoldierObjects[m_nObjects - 1] = m_ppSampleAnimationObjects[0];
	m_ppDroneObjects[m_nObjects - 1] = m_ppSampleAnimationObjects[1];
	
	if(nCharaterType == 0)
	{
		m_ppSoldierObjects[m_nObjects - 1]->SetPlayer(m_pPlayer);
		m_pPlayer->SetRenderObject(m_ppSoldierObjects[m_nObjects - 1]);
	}
	else if(nCharaterType == 1)
	{
		m_ppDroneObjects[m_nObjects - 1]->SetPlayer(m_pPlayer);
		m_pPlayer->SetRenderObject(m_ppDroneObjects[m_nObjects - 1]);
	}

	for (int i = 0; i < m_nObjects; i++)
	{
		for (int j = 0; j < m_pnAnimationFactor[0]; j++)
		{
			m_ppSoldierObjects[i]->m_ppAnimationFactorObjects[j]->m_pAnimationFactors->SetBoneObject(m_ppSoldierObjects[i]);
		}
		for (int j = 0; j < m_pnAnimationFactor[1]; j++)
		{
			m_ppDroneObjects[i]->m_ppAnimationFactorObjects[j]->m_pAnimationFactors->SetBoneObject(m_ppDroneObjects[i]);
		}

		m_ppSoldierObjects[i]->SetPosition(10 * i + 100, 0, 10 * i + 100);
		m_ppDroneObjects[i]->SetPosition(10 * i + 100 , 0, 10 * i + 100);

		m_ppSoldierObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
		m_ppDroneObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
	}
	m_ppSoldierObjects[m_nObjects - 1]->SetPosition(644.644531, 16.000999, 944.775269);
	if (nCharaterType == 0)
	{
		m_ppSoldierObjects[m_nObjects - 1]->SetPlayerScale(10);
	}
	else if (nCharaterType == 1)
	{
		m_ppDroneObjects[m_nObjects - 1]->SetPlayerScale(10);
	}

}

void CharacterScene::ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid, int Type)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (PlayerArray[i].m_isconnected)
		{
			if (Type == 0)
			{
				if (i != myid)
				{
					m_ppSoldierObjects[i]->m_xmf4x4ToParentTransform = PlayerArray[i].pos;
					m_ppSoldierObjects[i]->m_xmf4x4ToParentTransform._11 *= 10.f;
					m_ppSoldierObjects[i]->m_xmf4x4ToParentTransform._22 *= 10.f;
					m_ppSoldierObjects[i]->m_xmf4x4ToParentTransform._33 *= 10.f;

					m_ppSoldierObjects[i]->ChangeAnimation(PlayerArray[i].anim_state);
				}
			}
			else if(Type == 1)
			{
				if (i != myid)
				{
					m_ppDroneObjects[i]->m_xmf4x4ToParentTransform = PlayerArray[i].pos;
					m_ppDroneObjects[i]->m_xmf4x4ToParentTransform._11 *= 10.f;
					m_ppDroneObjects[i]->m_xmf4x4ToParentTransform._22 *= 10.f;
					m_ppDroneObjects[i]->m_xmf4x4ToParentTransform._33 *= 10.f;

					m_ppDroneObjects[i]->ChangeAnimation(PlayerArray[i].anim_state);
				}
			}
		}
	}
}
void CharacterScene::CopyObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CAnimationObject* pSample, CAnimationObject** ppObjects, UINT nSize, UINT nAnimationFactorIndex)
{
	if (nSize <= 0)
		return;
	UINT nAnimationFactorObject = -1;
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
			if (TargetFrame->m_ppAnimationFactorObjects != NULL)
				ppEmptyObjects[i]->m_ppAnimationFactorObjects = new CAnimationObject*[m_pnAnimationFactor[nAnimationFactorIndex]];

			if (TargetFrame->m_pAnimationFactors)
			{
				if (nAnimationFactorObject == -1)
				{
					for (int k = 0; k < 4; k++)
					{
						if (pSample->m_ppAnimationFactorObjects[k]->m_iBoneIndex == TargetFrame->m_iBoneIndex)
						{
							nAnimationFactorObject = k;
							break;
						}
					}
				}
				ppEmptyObjects[i]->m_pAnimationFactors = new AnimationResourceFactors(pd3dDevice, pd3dCommandList, TargetFrame->m_pAnimationFactors->m_nBindpos, 0);
				memcpy(ppEmptyObjects[i]->m_pAnimationFactors->m_pBoneIndexList, TargetFrame->m_pAnimationFactors->m_pBoneIndexList, sizeof(UINT) * ppEmptyObjects[i]->m_pAnimationFactors->m_nBindpos);
				ppObjects[i]->m_ppAnimationFactorObjects[nAnimationFactorObject] = ppEmptyObjects[i];
			}
			if (TargetFrame->m_pAnimationTime)
			{
				ppEmptyObjects[i]->m_pAnimationTime = new AnimationTimeFactor();
			}
		}
		nAnimationFactorObject = -1;
		if (TargetFrame->m_pChild)
		{
			for (int i = 0; i < nSize; i++)
			{

				ppChildObjects[i] = new CAnimationObject(0);
				ppEmptyObjects[i]->m_pChild = ppChildObjects[i];
				ppChildObjects[i]->m_pParent = ppEmptyObjects[i];
			}
		}
		if (TargetFrame->m_pSibling)
		{
			for (int i = 0; i < nSize; i++)
			{
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
}
void CharacterScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CharacterScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
}
void CharacterScene::ChangeAnimation(int newState, int Type)
{
	if (Type == 0)
	{
		m_ppSoldierObjects[m_nObjects - 1]->m_pAnimationController->SetObject(m_ppSoldierObjects[m_nObjects - 1], NULL);
		m_ppSoldierObjects[m_nObjects - 1]->ChangeAnimation(newState);
	}
	else if (Type == 1)
	{
		m_ppDroneObjects[m_nObjects - 1]->m_pAnimationController->SetObject(m_ppSoldierObjects[m_nObjects - 1], NULL);
		m_ppDroneObjects[m_nObjects - 1]->ChangeAnimation(newState);
	}
	else
	{
	}
}
void CharacterScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppSampleAnimationObjects[0]->m_ppAnimationFactorObjects[0]->m_pMaterial->m_pShader)
		m_ppSampleAnimationObjects[0]->m_ppAnimationFactorObjects[0]->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(12, d3dcbLightsGpuVirtualAddress);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(11, d3dcbMaterialsGpuVirtualAddress);

	m_pPlayer->OnPrepareRender();

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppSoldierObjects[i]->m_pAnimationController->SetObject(m_ppSoldierObjects[i], NULL);
		m_ppSoldierObjects[i]->m_pAnimationController->AdvanceAnimation(pd3dCommandList, true);
		m_ppDroneObjects[i]->m_pAnimationController->SetObject(m_ppDroneObjects[i], NULL);
		m_ppDroneObjects[i]->m_pAnimationController->AdvanceAnimation(pd3dCommandList, true);

		m_ppSoldierObjects[i]->UpdateTransform(NULL);
		m_ppDroneObjects[i]->UpdateTransform(NULL);
	}

	for (int k = 0; k < 2; k++)
	{
		if (k == 0)
			m_ppSampleAnimationObjects[k]->Render(pd3dCommandList, pCamera, m_nObjects, m_ppSoldierObjects, m_pnAnimationFactor[k], m_pPlayer);
		else if (k == 1)
		{
			pd3dCommandList->OMSetStencilRef(1);
			m_ppSampleAnimationObjects[k]->Render(pd3dCommandList, pCamera, m_nObjects, m_ppDroneObjects, m_pnAnimationFactor[k], m_pPlayer);
		}
	}
}
void CharacterScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->ShadowUpdateShaderVariables(pd3dCommandList, pCameraInfo);

	for (int k = 0; k < MESH_NUM; k++)
	{
		if (k == 0)
			m_ppSampleAnimationObjects[k]->ShadowRender(pd3dCommandList, pCamera, m_nObjects, m_ppSoldierObjects, m_pnAnimationFactor[k]);
		else if (k == 1)
			m_ppSampleAnimationObjects[k]->ShadowRender(pd3dCommandList, pCamera, m_nObjects, m_ppDroneObjects, m_pnAnimationFactor[k]);
	}
}
void CharacterScene::ReleaseShaderVariables()
{
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

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 0; // ±×¸²ÀÚ ¸Ê
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[14];

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

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Materials
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Lights
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[6];
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 16;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	pd3dSamplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

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

ObjectScene::ObjectScene()
{
	m_ppSampleObjects = new CGameObject*[MESH_NUM2];
}
ObjectScene::~ObjectScene()
{
	ReleaseObjects();
	ReleaseShaderVariables();
}
void ObjectScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType)
{
	m_nObjects = OBJECTS_NUMBER/5;

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_ppSampleObjects[0] = new CGameObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid5.bss", m_nObjects, pShadowMap);
	m_ppSampleObjects[1] = new CGameObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid1.bss", m_nObjects, pShadowMap);
	m_ppSampleObjects[2] = new CGameObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid2.bss", m_nObjects, pShadowMap);
	m_ppSampleObjects[3] = new CGameObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid3.bss", m_nObjects, pShadowMap);
	m_ppSampleObjects[4] = new CGameObject(0, pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"../Assets/Asteroid4.bss", m_nObjects, pShadowMap);

	m_ppAsteroid0 = new CGameObject*[m_nObjects];
	m_ppAsteroid1 = new CGameObject*[m_nObjects];
	m_ppAsteroid2 = new CGameObject*[m_nObjects];
	m_ppAsteroid3 = new CGameObject*[m_nObjects];
	m_ppAsteroid4 = new CGameObject*[m_nObjects];
	
	for (int i = 0; i < m_nObjects - 1; i++)
	{
		CGameObject* pAsteroid = new CGameObject(0, 0);
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid0[i] = pAsteroid;

		pAsteroid = new CGameObject(0, 0);
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid1[i] = pAsteroid;

		pAsteroid = new CGameObject(0, 0);
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid2[i] = pAsteroid;

		pAsteroid = new CGameObject(0, 0);
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid3[i] = pAsteroid;

		pAsteroid = new CGameObject(0, 0);
		pAsteroid->SetPosition(XMFLOAT3(rand() % 1700, 300, rand() % 1700));
		m_ppAsteroid4[i] = pAsteroid;
	}
	m_ppSampleObjects[0]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[1]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[2]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[3]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);
	m_ppSampleObjects[4]->CGameObject::SetPosition(rand() % 2056, rand() % 200 + 200, rand() % 2056);

	m_ppAsteroid0[m_nObjects - 1] = m_ppSampleObjects[0];
	m_ppAsteroid1[m_nObjects - 1] = m_ppSampleObjects[1];
	m_ppAsteroid2[m_nObjects - 1] = m_ppSampleObjects[2];
	m_ppAsteroid3[m_nObjects - 1] = m_ppSampleObjects[3];
	m_ppAsteroid4[m_nObjects - 1] = m_ppSampleObjects[4];
}
void ObjectScene::ReleaseObjects()
{
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

	delete[] m_ppAsteroid0;
	delete[] m_ppAsteroid1;
	delete[] m_ppAsteroid2;
	delete[] m_ppAsteroid3;
	delete[] m_ppAsteroid4;
}
int cnt = 0;
void ObjectScene::SetObjectsVectorFromPacket(XMFLOAT3 pos, int n_ObjectIdx)
{

	m_xmf3ObjectsPos[n_ObjectIdx] = pos;
	cnt++;


	if (cnt == 29)
		m_isObjectLoad = true;
}
int idx = 0;
void ObjectScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera)
{
	m_fObjectUpdateTime += fTimeElapsed;
	if (m_isObjectLoad == true)
	{
		if (m_fObjectUpdateTime >= 0.05f)
		{
			for (int i = 0; i < OBJECTS_NUMBER / 5; ++i)
			{
				m_ppAsteroid0[i]->CGameObject::SetPosition(m_xmf3ObjectsPos[i]);
				m_ppAsteroid1[i]->CGameObject::SetPosition(m_xmf3ObjectsPos[i + 6]);
				m_ppAsteroid2[i]->CGameObject::SetPosition(m_xmf3ObjectsPos[i + 12]);
				m_ppAsteroid3[i]->CGameObject::SetPosition(m_xmf3ObjectsPos[i + 18]);
				m_ppAsteroid4[i]->CGameObject::SetPosition(m_xmf3ObjectsPos[i + 24]);
				//printf("OBJECTS NUMBER :  %d \n", idx);
			}
			m_fObjectUpdateTime = 0.f;
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

	if (m_ppAsteroid0[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid0[m_nObjects - 1]->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(7, d3dcbLightsGpuVirtualAddress);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(6, d3dcbMaterialsGpuVirtualAddress);

	FrustumCulling(pCamera);

	if (m_ppAsteroid0[m_nObjects - 1]->m_pMaterial)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(8, m_ppAsteroid0[m_nObjects - 1]->m_pMaterial->m_pShader->GetShadowGPUDescriptorHandle());
	}
	if (m_ppAsteroid0[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid0[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid0[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects, m_ppAsteroid0);

	if (m_ppAsteroid1[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid1[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid1[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects, m_ppAsteroid1);

	if (m_ppAsteroid2[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid2[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid2[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects, m_ppAsteroid2);

	if (m_ppAsteroid3[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid3[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid3[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects, m_ppAsteroid3);

	if (m_ppAsteroid4[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid4[m_nObjects - 1]->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
	m_ppAsteroid4[m_nObjects - 1]->Render(pd3dCommandList, 5, pCamera, m_nObjects, m_ppAsteroid4);
}
void ObjectScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->ShadowUpdateShaderVariables(pd3dCommandList, pCameraInfo);

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

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[4];

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

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 10; // ¸ðµ¨ ½ºÆåÅ§·¯¸Ê
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[9];

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

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[6].Descriptor.ShaderRegister = 7; //Materials
	pd3dRootParameters[6].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[7].Descriptor.ShaderRegister = 8; //Lights
	pd3dRootParameters[7].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3]; //½ºÆåÅ§·¯
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 16;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	pd3dSamplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

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