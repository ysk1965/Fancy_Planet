#include "stdafx.h"
#include "Scene.h"

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
	m_nObjects = OBJECTS_NUMBER / 5;

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

	m_pSkyBox = new CSkyBox(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
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
	if (m_pSkyBox)
		delete m_pSkyBox;

	delete[] m_ppAsteroid0;
	delete[] m_ppAsteroid1;
	delete[] m_ppAsteroid2;
	delete[] m_ppAsteroid3;
	delete[] m_ppAsteroid4;
}
int cnt = 0;
void ObjectScene::SetObjectsVectorFromPacket(XMFLOAT3& pos, XMFLOAT4& quat, int n_ObjectIdx)
{
	m_xmf3ObjectsPos[n_ObjectIdx] = pos;
	m_xmf4ObjectsQuaternion[n_ObjectIdx] = quat;

}
int idx = 0;
void ObjectScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera, int Scene)
{
	m_fObjectUpdateTime += fTimeElapsed;
	if (Scene == INGAME)
	{
		for (int i = 0; i < OBJECTS_NUMBER / 5; ++i)
		{
			XMFLOAT3 Scale = XMFLOAT3(1.f, 1.f, 1.f);
			XMVECTOR S = XMLoadFloat3(&Scale); // 항상 곱해주는 스케읽밧
			XMVECTOR P = XMLoadFloat3(&m_xmf3ObjectsPos[i]); // 서버에서 보내준 포지션값
			XMVECTOR Q = XMLoadFloat4(&m_xmf4ObjectsQuaternion[i]); // 서버에서 보내준 쿼터니언값
			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 마지막 _x4자리 
			XMStoreFloat4x4(&m_ppAsteroid0[i]->m_xmf4x4World, XMMatrixAffineTransformation(S, zero, Q, P));

			P = XMLoadFloat3(&m_xmf3ObjectsPos[i + 6]); // 서버에서 보내준 포지션값
			Q = XMLoadFloat4(&m_xmf4ObjectsQuaternion[i + 6]); // 서버에서 보내준 쿼터니언값
			XMStoreFloat4x4(&m_ppAsteroid1[i]->m_xmf4x4World, XMMatrixAffineTransformation(S, zero, Q, P));

			P = XMLoadFloat3(&m_xmf3ObjectsPos[i + 12]); // 서버에서 보내준 포지션값
			Q = XMLoadFloat4(&m_xmf4ObjectsQuaternion[i + 12]); // 서버에서 보내준 쿼터니언값
			XMStoreFloat4x4(&m_ppAsteroid2[i]->m_xmf4x4World, XMMatrixAffineTransformation(S, zero, Q, P));

			P = XMLoadFloat3(&m_xmf3ObjectsPos[i + 18]); // 서버에서 보내준 포지션값
			Q = XMLoadFloat4(&m_xmf4ObjectsQuaternion[i + 18]); // 서버에서 보내준 쿼터니언값
			XMStoreFloat4x4(&m_ppAsteroid3[i]->m_xmf4x4World, XMMatrixAffineTransformation(S, zero, Q, P));

			P = XMLoadFloat3(&m_xmf3ObjectsPos[i + 24]); // 서버에서 보내준 포지션값
			Q = XMLoadFloat4(&m_xmf4ObjectsQuaternion[i + 24]); // 서버에서 보내준 쿼터니언값
			XMStoreFloat4x4(&m_ppAsteroid4[i]->m_xmf4x4World, XMMatrixAffineTransformation(S, zero, Q, P));
		}
	}
}
void ObjectScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < MESH_NUM2; i++)
	{
		m_ppSampleObjects[i]->ReleaseUploadBuffers();
	}

	if (m_pSkyBox)
		m_pSkyBox->ReleaseUploadBuffers();
}
void ObjectScene::ReleaseShaderVariables()
{
}
void ObjectScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(7, d3dcbLightsGpuVirtualAddress);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(6, d3dcbMaterialsGpuVirtualAddress);

	if (m_pSkyBox)
	{
		if (m_pSkyBox->m_pMaterial)
			m_pSkyBox->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);
		if (m_pSkyBox->m_pMaterial)
			m_pSkyBox->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);

		m_pSkyBox->Render(pd3dCommandList, pCamera);
	}

	if (m_ppAsteroid0[m_nObjects - 1]->m_pMaterial)
		m_ppAsteroid0[m_nObjects - 1]->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);

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
void ObjectScene::SetSkyBoxPosition(const XMFLOAT3& xmf3Pos)
{
	if (m_pSkyBox)
		m_pSkyBox->SetPosition(xmf3Pos.x, xmf3Pos.y, xmf3Pos.z);
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

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[6];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; // 모델 디퓨즈맵
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 1; // 모델 노말맵
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 2; // 모델 스펙큘러맵
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 10; // 모델 스펙큘러맵
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 9;
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 2;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[10];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1; //Object
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5];
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //디퓨즈
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1]; //노말
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2]; //스펙큘러
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV; // 인스턴싱
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
	pd3dRootParameters[8].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3]; //스펙큘러
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1; //skyBoxTextrue
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];
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
	HRESULT Hresult = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}