#include "stdafx.h"
#include "Scene.h"

EffectScene::EffectScene()
{
	m_nMeshes = 6;
	m_ppMeshes = new CMesh*[m_nMeshes];
}
EffectScene::~EffectScene()
{
	ReleaseObjects();
}
void EffectScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_ppMeshes[0] = new CRectMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 0.3f, 0.3f, 0.0f);
	m_ppMeshes[1] = new CRectMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 2.5f, 2.5f, 0.0f);
	m_ppMeshes[2] = new CCubeMesh(pd3dDevice, pd3dCommandList, 0.1f, 0.1f, 1.0f);
	m_ppMeshes[3] = new CRectMeshIlluminatedTextured(pd3dDevice, pd3dCommandList, 0.5f, 0.5f, 0.0f);
	m_ppMeshes[4] = new CMesh(pd3dDevice, pd3dCommandList, L"../Assets/Sphere.bss_mesh");
	m_ppMeshes[5] = new CMesh(pd3dDevice, pd3dCommandList, L"../Assets/DronSkill2.bss_mesh");

	m_pFlareShader = new CEffectShader();
	m_pFlareShader->BuildShader(pd3dDevice, pd3dCommandList);
	m_pFlareShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 1);

	m_pMeshEffectShader = new MeshEffectShader();
	m_pMeshEffectShader->BuildShader(pd3dDevice, pd3dCommandList);
	m_pMeshEffectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature, 1);

	m_ppDroneEffect = new DroneEffect*[m_nObjects];
	m_ppSparkEffect = new SparkEffect*[m_nObjects];
	m_ppMeshEffect = new MeshEffect*[m_nObjects];
	m_ppTeleportEffect = new TeleportEffect*[m_nObjects];
	m_ppJumpEffect = new JumpEffect*[m_nObjects];
	m_ppSphereEffect = new SphereEffect*[m_nObjects];
	m_ppDroneSkillEffect = new DroneSkillEffect*[m_nObjects];
	m_ppBloodEffect = new BloodEffect*[m_nObjects * 3];

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppDroneEffect[i] = new DroneEffect(1, 100, m_ppDrone[i]);
		m_ppDroneEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppDroneEffect[i]->SetMesh(m_ppMeshes[3], 0);

		m_ppSparkEffect[i] = new SparkEffect(1, 200, m_ppSoldier[i]);
		m_ppSparkEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppSparkEffect[i]->SetMesh(m_ppMeshes[0], 0);

		m_ppMeshEffect[i] = new MeshEffect(1, 1, m_ppDrone[i]);
		m_ppMeshEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppMeshEffect[i]->SetMesh(m_ppMeshes[2], 0);

		m_ppTeleportEffect[i] = new TeleportEffect(1, 100, m_ppDrone[i]);
		m_ppTeleportEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppTeleportEffect[i]->SetMesh(m_ppMeshes[3], 0);

		m_ppJumpEffect[i] = new JumpEffect(1, 100, m_ppSoldier[i]);
		m_ppJumpEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppJumpEffect[i]->SetMesh(m_ppMeshes[3], 0);

		m_ppSphereEffect[i] = new SphereEffect(2, 501, m_ppSoldier[i]);
		m_ppSphereEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppSphereEffect[i]->SetMesh(m_ppMeshes[4], 0);
		m_ppSphereEffect[i]->SetMesh(m_ppMeshes[3], 1);

		m_ppDroneSkillEffect[i] = new DroneSkillEffect(1, 1, m_ppDrone[i]);
		m_ppDroneSkillEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppDroneSkillEffect[i]->SetMesh(m_ppMeshes[5], 0);

		m_ppBloodEffect[i] = new BloodEffect(1, 30, m_pPlayer->GetRenderObject());
		m_ppBloodEffect[i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppBloodEffect[i]->SetMesh(m_ppMeshes[0], 0);

		m_ppBloodEffect[m_nObjects * 1 + i] = new BloodEffect(1, 30, m_pPlayer->GetRenderObject());
		m_ppBloodEffect[m_nObjects * 1 + i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppBloodEffect[m_nObjects * 1 + i]->SetMesh(m_ppMeshes[0], 0);

		m_ppBloodEffect[m_nObjects * 2 + i] = new BloodEffect(1, 30, m_pPlayer->GetRenderObject());
		m_ppBloodEffect[m_nObjects * 2 + i]->CreateShaderVariables(pd3dDevice, pd3dCommandList);
		m_ppBloodEffect[m_nObjects * 2 + i]->SetMesh(m_ppMeshes[0], 0);
	}
}
void EffectScene::ReleaseObjects()
{
	for (int i = 0; i < m_nObjects; i++)
	{
		if (m_ppSparkEffect[i])
			delete m_ppSparkEffect[i];
		if (m_ppMeshEffect[i])
			delete m_ppMeshEffect[i];
		if (m_ppTeleportEffect[i])
			delete m_ppTeleportEffect[i];
		if (m_ppJumpEffect[i])
			delete m_ppJumpEffect[i];
		if (m_ppDroneEffect[i])
			delete m_ppDroneEffect[i];
		if (m_ppSphereEffect[i])
			delete m_ppSphereEffect[i];
		if (m_ppDroneSkillEffect[i])
			delete m_ppDroneSkillEffect[i];
		if (m_ppBloodEffect[m_nObjects * SOLDIER + i])
			delete m_ppBloodEffect[m_nObjects * SOLDIER + i];
		if (m_ppBloodEffect[m_nObjects * DRONE + i])
			delete m_ppBloodEffect[m_nObjects * DRONE + i];
		if (m_ppBloodEffect[m_nObjects * CREATURE + i])
			delete m_ppBloodEffect[m_nObjects * CREATURE + i];
	}
	if (m_ppSparkEffect)
		delete[] m_ppSparkEffect;
	if (m_ppMeshEffect)
		delete[] m_ppMeshEffect;
	if (m_ppTeleportEffect)
		delete[] m_ppTeleportEffect;
	if (m_ppJumpEffect)
		delete[] m_ppJumpEffect;
	if (m_ppDroneEffect)
		delete[] m_ppDroneEffect;
	if (m_ppSphereEffect)
		delete[] m_ppSphereEffect;
	if (m_ppBloodEffect)
		delete[] m_ppBloodEffect;
	if (m_ppDroneSkillEffect)
		delete[] m_ppDroneSkillEffect;

	for (int i = 0; i < m_nMeshes; i++)
		m_ppMeshes[i]->Release();

	delete[] m_ppMeshes;
}
void EffectScene::UpdateEffect(const unsigned long& nCurrentFrameRate)
{
	DWORD dwTime = GetTickCount();

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppBloodEffect[i]->Animate(dwTime, nCurrentFrameRate);
		m_ppBloodEffect[m_nObjects + i]->Animate(dwTime, nCurrentFrameRate);
		m_ppBloodEffect[m_nObjects * 2 + i]->Animate(dwTime, nCurrentFrameRate);

		m_ppSparkEffect[i]->Animate(dwTime, nCurrentFrameRate);
		m_ppJumpEffect[i]->Animate(dwTime, nCurrentFrameRate);
		m_ppSphereEffect[i]->Animate(dwTime, nCurrentFrameRate);
		m_ppDroneEffect[i]->Animate(0, dwTime, nCurrentFrameRate);

		m_ppTeleportEffect[i]->Animate(dwTime);
		m_ppMeshEffect[i]->Animate(dwTime);
		m_ppDroneSkillEffect[i]->Animate(dwTime, m_ppDrone[i]->GetLook());
	}
}
void EffectScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_pFlareShader)
		m_pFlareShader->OnPrepareRender(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);


	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppBloodEffect[i]->Render(pd3dCommandList, 0);
		m_ppBloodEffect[m_nObjects + i]->Render(pd3dCommandList, 0);
		m_ppBloodEffect[m_nObjects * 2 + i]->Render(pd3dCommandList, 0);
	}

	pd3dCommandList->OMSetStencilRef(1);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppSphereEffect[i]->Render(pd3dCommandList, 1);
		m_ppSparkEffect[i]->Render(pd3dCommandList, 0);
		m_ppJumpEffect[i]->Render(pd3dCommandList, 0);
		m_ppDroneEffect[i]->Render(pd3dCommandList, 0);
	}
	if (m_pMeshEffectShader)
		m_pMeshEffectShader->OnPrepareRender(pd3dCommandList);

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppTeleportEffect[i]->Render(pd3dCommandList, 0);
		m_ppMeshEffect[i]->Render(pd3dCommandList, 0);
		m_ppSphereEffect[i]->Render(pd3dCommandList, 0);
		m_ppDroneSkillEffect[i]->Render(pd3dCommandList, 0);
	}
}

void EffectScene::SetParticle(UINT nType, UINT nCharacterType, UINT nIndex, XMFLOAT3* pxmf3Target)
{
	if (nType == 0)
	{
		if (nCharacterType == SOLDIER)
		{
			m_ppSparkEffect[nIndex]->SetParticle();
		}
		else if (nCharacterType == DRONE && pxmf3Target != NULL)
		{
			XMFLOAT3 pos = *pxmf3Target;
			m_ppMeshEffect[nIndex]->SetTarget(pos);
		}
	}
	else if (nType == 1)
	{
		if (nCharacterType == SOLDIER)
			m_ppSphereEffect[nIndex]->SetTarget(*pxmf3Target);
		else if (nCharacterType == DRONE)
			m_ppTeleportEffect[nIndex]->SetParticle();
	}
	else if (nType == 2 && pxmf3Target != NULL)
	{
		m_ppBloodEffect[nCharacterType*m_nObjects + nIndex]->SetTarget(*pxmf3Target);
	}
	else if (nType == 3)
	{
		if (nCharacterType == SOLDIER)
			m_ppJumpEffect[nIndex]->SetParticle();
		else if (nCharacterType == DRONE)
			m_ppDroneSkillEffect[nIndex]->SetTarget();
	}
	else if (nType == 4)
	{
		if (nCharacterType == DRONE)
			m_ppDroneEffect[nIndex]->SetHit();
	}
}
void EffectScene::Update(CCamera* pCamera)
{
	XMFLOAT3 xmf3CP = XMFLOAT3(pCamera->GetPosition().x + pCamera->GetOffset().z, pCamera->GetPosition().y + pCamera->GetOffset().y, pCamera->GetPosition().z + pCamera->GetOffset().z);
	XMFLOAT3 xmf3CR = pCamera->GetRightVector();
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppDroneEffect[i]->SetLookAt(xmf3CP, xmf3CR);
		m_ppSparkEffect[i]->SetLookAt(xmf3CP, xmf3CR);
		m_ppJumpEffect[i]->SetLookAt(xmf3CP, xmf3CR);
		m_ppTeleportEffect[i]->SetLookAt(xmf3CP, xmf3CR);
		m_ppSphereEffect[i]->SetLookAt(xmf3CP, xmf3CR);

		m_ppBloodEffect[i]->SetLookAt(xmf3CP, xmf3CR);
		m_ppBloodEffect[m_nObjects + i]->SetLookAt(xmf3CP, xmf3CR);
		m_ppBloodEffect[m_nObjects * 2 + i]->SetLookAt(xmf3CP, xmf3CR);
	}
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