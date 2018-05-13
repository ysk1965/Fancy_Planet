#include "stdafx.h"
#include "UIShader.h"
#include <math.h>

MiniMapShader::MiniMapShader(CPlayer * pPlayer)
{
	m_pPlayer = pPlayer;
}

MiniMapShader::~MiniMapShader()
{
	ReleaseShaderVariables();
}

D3D12_INPUT_LAYOUT_DESC MiniMapShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

void MiniMapShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Map Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //UI
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}

D3D12_SHADER_BYTECODE MiniMapShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIVS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE MiniMapShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}

void MiniMapShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void MiniMapShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/map.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 2, true);
}

void MiniMapShader::ReleaseShaderVariables()
{
	m_pd3dcbUI->Unmap(0, NULL);
}

void MiniMapShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;

}
void MiniMapShader::CalculateMiniMap()
{
	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();

	if ((xmf3Position.x >= 0 && xmf3Position.x < PLAYER_MAP_RANGE) &&  // 1
		(xmf3Position.z >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.z < MAP_SIZE))
	{
		m_ui.xmf2Map1.x = 0.0f;
		m_ui.xmf2Map1.y = 0.0f;
		m_ui.xmf2Map2.x = (2 * PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map2.y = (2 * PLAYER_MAP_RANGE) / MAP_SIZE;
	}
	else if ((xmf3Position.x >= PLAYER_MAP_RANGE && xmf3Position.x < (MAP_SIZE - PLAYER_MAP_RANGE)) && // 2
		(xmf3Position.z >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.z < MAP_SIZE))
	{
		m_ui.xmf2Map1.x = (xmf3Position.x - PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map1.y = 0.0f;
		m_ui.xmf2Map2.x = (xmf3Position.x + PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map2.y = (2 * PLAYER_MAP_RANGE) / MAP_SIZE;
	}
	else if ((xmf3Position.x >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.x < MAP_SIZE) && // 3
		(xmf3Position.z >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.z < MAP_SIZE))
	{
		m_ui.xmf2Map1.x = (MAP_SIZE - (2 * PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map1.y = 0.0f;
		m_ui.xmf2Map2.x = 1.0f;
		m_ui.xmf2Map2.y = (2 * PLAYER_MAP_RANGE) / MAP_SIZE;
	}
	else if ((xmf3Position.x >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.x < MAP_SIZE) && // 4
		(xmf3Position.z >= PLAYER_MAP_RANGE && xmf3Position.z < (MAP_SIZE - PLAYER_MAP_RANGE)))
	{
		m_ui.xmf2Map1.x = (MAP_SIZE - (2 * PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map1.y = (MAP_SIZE - (xmf3Position.z + PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map2.x = 1.0f;
		m_ui.xmf2Map2.y = (MAP_SIZE - (xmf3Position.z - PLAYER_MAP_RANGE)) / MAP_SIZE;
	}
	else if ((xmf3Position.x >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.x < MAP_SIZE) && // 5
		(xmf3Position.z >= 0 && xmf3Position.z < PLAYER_MAP_RANGE))
	{
		m_ui.xmf2Map1.x = (MAP_SIZE - (2 * PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map1.y = (MAP_SIZE - (2 * PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map2.x = 1.0f;
		m_ui.xmf2Map2.y = 1.0f;
	}
	else if ((xmf3Position.x >= PLAYER_MAP_RANGE && xmf3Position.x < (MAP_SIZE - PLAYER_MAP_RANGE)) && // 6
		(xmf3Position.z >= 0 && xmf3Position.z < PLAYER_MAP_RANGE))
	{
		m_ui.xmf2Map1.x = (xmf3Position.x - PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map1.y = (MAP_SIZE - (2 * PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map2.x = (xmf3Position.x + PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map2.y = 1.0f;
	}
	else if ((xmf3Position.x >= 0 && xmf3Position.x < PLAYER_MAP_RANGE) && // 7
		(xmf3Position.z >= 0 && xmf3Position.z < PLAYER_MAP_RANGE))
	{
		m_ui.xmf2Map1.x = 0.0f;
		m_ui.xmf2Map1.y = (MAP_SIZE - (2 * PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map2.x = (2 * PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map2.y = 1.0f;
	}
	else if ((xmf3Position.x >= 0 && xmf3Position.x < PLAYER_MAP_RANGE) && // 8
		(xmf3Position.z >= PLAYER_MAP_RANGE && xmf3Position.z < (MAP_SIZE - PLAYER_MAP_RANGE)))
	{
		m_ui.xmf2Map1.x = 0.0f;
		m_ui.xmf2Map1.y = (MAP_SIZE - (xmf3Position.z + PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map2.x = (2 * PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map2.y = (MAP_SIZE - (xmf3Position.z - PLAYER_MAP_RANGE)) / MAP_SIZE;
	}
	else // 9
	{
		m_ui.xmf2Map1.x = (xmf3Position.x - PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map1.y = (MAP_SIZE - (xmf3Position.z + PLAYER_MAP_RANGE)) / MAP_SIZE;
		m_ui.xmf2Map2.x = (xmf3Position.x + PLAYER_MAP_RANGE) / MAP_SIZE;
		m_ui.xmf2Map2.y = (MAP_SIZE - (xmf3Position.z - PLAYER_MAP_RANGE)) / MAP_SIZE;
	}
}

void MiniMapShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbUI = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbUI->Map(0, NULL, (void **)&m_pcbMappedUI);
}

void MiniMapShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedUI, &m_ui, sizeof(UI_INFO));
}

void MiniMapShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CalculateMiniMap();

	CShader::Render(pd3dCommandList, pCamera);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dcbUIGpuVirtualAddress);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ArrowShader::ArrowShader(CPlayer * pPlayer) : MiniMapShader(pPlayer)
{
	m_pPlayer = pPlayer;
}

ArrowShader::~ArrowShader()
{
	ReleaseShaderVariables();
}

D3D12_INPUT_LAYOUT_DESC ArrowShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

void ArrowShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Map Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //UI
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}

void ArrowShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbUI = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbUI->Map(0, NULL, (void **)&m_pcbMappedUI);
}

void ArrowShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedUI, &m_ui, sizeof(UI_INFO));
}

D3D12_SHADER_BYTECODE ArrowShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "ARROW_VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ArrowShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}

void ArrowShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void ArrowShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/arrow.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 1, true);
}

void ArrowShader::ReleaseShaderVariables()
{
}

void ArrowShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;

}
void ArrowShader::CalculateArrow()
{
	XMFLOAT3 xmf3Result;
	XMFLOAT3 xmf3Look = m_pPlayer->GetLook();
	XMFLOAT3 xmf3Roll = XMFLOAT3(0.0f, 0.0f, 1.0f);
	xmf3Look.y = 0.0f;
	xmf3Look = Vector3::Normalize(xmf3Look);
	XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&xmf3Look), XMLoadFloat3(&xmf3Roll)));

	float fSin = XMScalarSin(XMScalarACos(xmf3Result.x));

	if (xmf3Look.x < 0)
	{
		m_ui.xmf2Degree.x = xmf3Result.x;
		m_ui.xmf2Degree.y = -fSin;
	}
	else
	{
		m_ui.xmf2Degree.x = xmf3Result.x;
		m_ui.xmf2Degree.y = fSin;
	}

	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();

	if ((xmf3Position.x >= 0 && xmf3Position.x < PLAYER_MAP_RANGE) &&  // 1
		(xmf3Position.z >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.z < MAP_SIZE))
	{
		float fx = (1.0f - (xmf3Position.x / PLAYER_MAP_RANGE)) * MINI_MAP_SIZE;
		float fz = ((xmf3Position.z - (MAP_SIZE - PLAYER_MAP_RANGE)) / PLAYER_MAP_RANGE) * MINI_MAP_SIZE;

		m_ui.xmf2Arrow.x = ARROW_CENTER - fx;
		m_ui.xmf2Arrow.y = ARROW_CENTER + fz;
	}
	else if ((xmf3Position.x >= PLAYER_MAP_RANGE && xmf3Position.x < (MAP_SIZE - PLAYER_MAP_RANGE)) && // 2
		(xmf3Position.z >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.z < MAP_SIZE))
	{
		float fz = ((xmf3Position.z - (MAP_SIZE - PLAYER_MAP_RANGE)) / PLAYER_MAP_RANGE) * MINI_MAP_SIZE;

		m_ui.xmf2Arrow.x = ARROW_CENTER;
		m_ui.xmf2Arrow.y = ARROW_CENTER + fz;
	}
	else if ((xmf3Position.x >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.x < MAP_SIZE) && // 3
		(xmf3Position.z >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.z < MAP_SIZE))
	{
		float fx = ((xmf3Position.x - (MAP_SIZE - PLAYER_MAP_RANGE)) / PLAYER_MAP_RANGE) * MINI_MAP_SIZE;
		float fz = ((xmf3Position.z - (MAP_SIZE - PLAYER_MAP_RANGE)) / PLAYER_MAP_RANGE) * MINI_MAP_SIZE;

		m_ui.xmf2Arrow.x = ARROW_CENTER + fx;
		m_ui.xmf2Arrow.y = ARROW_CENTER + fz;
	}
	else if ((xmf3Position.x >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.x < MAP_SIZE) && // 4
		(xmf3Position.z >= PLAYER_MAP_RANGE && xmf3Position.z < (MAP_SIZE - PLAYER_MAP_RANGE)))
	{
		float fx = ((xmf3Position.x - (MAP_SIZE - PLAYER_MAP_RANGE)) / PLAYER_MAP_RANGE) * MINI_MAP_SIZE;

		m_ui.xmf2Arrow.x = ARROW_CENTER + fx;
		m_ui.xmf2Arrow.y = ARROW_CENTER;
	}
	else if ((xmf3Position.x >= (MAP_SIZE - PLAYER_MAP_RANGE) && xmf3Position.x < MAP_SIZE) && // 5
		(xmf3Position.z >= 0 && xmf3Position.z < PLAYER_MAP_RANGE))
	{
		float fx = ((xmf3Position.x - (MAP_SIZE - PLAYER_MAP_RANGE)) / PLAYER_MAP_RANGE) * MINI_MAP_SIZE;
		float fz = (1.0f - (xmf3Position.z / PLAYER_MAP_RANGE)) * MINI_MAP_SIZE;

		m_ui.xmf2Arrow.x = ARROW_CENTER + fx;
		m_ui.xmf2Arrow.y = ARROW_CENTER - fz;
	}
	else if ((xmf3Position.x >= PLAYER_MAP_RANGE && xmf3Position.x < (MAP_SIZE - PLAYER_MAP_RANGE)) && // 6
		(xmf3Position.z >= 0 && xmf3Position.z < PLAYER_MAP_RANGE))
	{
		float fz = (1.0f - (xmf3Position.z / PLAYER_MAP_RANGE)) * MINI_MAP_SIZE;

		m_ui.xmf2Arrow.x = ARROW_CENTER;
		m_ui.xmf2Arrow.y = ARROW_CENTER - fz;
	}
	else if ((xmf3Position.x >= 0 && xmf3Position.x < PLAYER_MAP_RANGE) && // 7
		(xmf3Position.z >= 0 && xmf3Position.z < PLAYER_MAP_RANGE))
	{
		float fx = (1.0f - (xmf3Position.x / PLAYER_MAP_RANGE)) * MINI_MAP_SIZE;
		float fz = (1.0f - (xmf3Position.z / PLAYER_MAP_RANGE)) * MINI_MAP_SIZE;

		m_ui.xmf2Arrow.x = ARROW_CENTER - fx;
		m_ui.xmf2Arrow.y = ARROW_CENTER - fz;
	}
	else if ((xmf3Position.x >= 0 && xmf3Position.x < PLAYER_MAP_RANGE) && // 8
		(xmf3Position.z >= PLAYER_MAP_RANGE && xmf3Position.z < (MAP_SIZE - PLAYER_MAP_RANGE)))
	{
		float fx = (1.0f - (xmf3Position.x / PLAYER_MAP_RANGE)) * MINI_MAP_SIZE;

		m_ui.xmf2Arrow.x = ARROW_CENTER - fx;
		m_ui.xmf2Arrow.y = ARROW_CENTER;
	}
	else // 9
	{
		m_ui.xmf2Arrow.x = ARROW_CENTER;
		m_ui.xmf2Arrow.y = ARROW_CENTER;
	}
}

void ArrowShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CalculateArrow();

	CShader::Render(pd3dCommandList, pCamera);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dcbUIGpuVirtualAddress);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


NumberShader::NumberShader() : MiniMapShader(NULL)
{
	start = chrono::system_clock::now();
}

NumberShader::~NumberShader()
{
	ReleaseShaderVariables();
}

D3D12_INPUT_LAYOUT_DESC NumberShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

void NumberShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Number Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //UI
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}


void NumberShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbUI = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbUI->Map(0, NULL, (void **)&m_pcbMappedUI);
}

D3D12_SHADER_BYTECODE NumberShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "NUMBER_VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE NumberShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}

void NumberShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void NumberShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/number.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 1, true);
}

void NumberShader::ReleaseShaderVariables()
{
}

void NumberShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;
}


XMFLOAT4& NumberShader::CalculateNumberTexture(UINT nNumber)
{
	XMFLOAT4 xmf4Result;


	xmf4Result.x = (nNumber % 5) / 5.0f;
	xmf4Result.y = ((UINT)(nNumber / 5)) / 2.0f;
	xmf4Result.z = (((nNumber) % 5) + 1) / 5.0f;
	xmf4Result.w = ((UINT)(nNumber / 5) + 1) / 2.0f;

	return xmf4Result;
}

void NumberShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dcbUIGpuVirtualAddress);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CrossShader::CrossShader()
{
}

CrossShader::~CrossShader()
{
}

D3D12_INPUT_LAYOUT_DESC CrossShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

void CrossShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Map Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[1];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}


D3D12_SHADER_BYTECODE CrossShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "CROSS_VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE CrossShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}

void CrossShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void CrossShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/cross.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 0, true);
}


void CrossShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;

}
void CrossShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

MiniUIShader::MiniUIShader()
{
}

MiniUIShader::~MiniUIShader()
{
}

D3D12_INPUT_LAYOUT_DESC MiniUIShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

void MiniUIShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Map Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[1];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}


D3D12_SHADER_BYTECODE MiniUIShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "MINIUI_VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE MiniUIShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}

void MiniUIShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void MiniUIShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/MiniMapUI.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 0, true);
}


void MiniUIShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;

}
void MiniUIShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

ScoreBoardShader::ScoreBoardShader()
{
}

ScoreBoardShader::~ScoreBoardShader()
{
}

D3D12_INPUT_LAYOUT_DESC ScoreBoardShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

void ScoreBoardShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Map Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[1];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}


D3D12_SHADER_BYTECODE ScoreBoardShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "SCOREBOARD_VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE ScoreBoardShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}

void ScoreBoardShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void ScoreBoardShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Scoreboard.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 0, true);
}


void ScoreBoardShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;

}
void ScoreBoardShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}


///////////////////////////////////////////////////////////////////////////////////

GravityBarShader::GravityBarShader()
{
}

GravityBarShader::~GravityBarShader()
{
}

D3D12_INPUT_LAYOUT_DESC GravityBarShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

void GravityBarShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Map Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[1];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}


D3D12_SHADER_BYTECODE GravityBarShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "GRAVITYBAR_VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE GravityBarShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}

void GravityBarShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void GravityBarShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/GravityBar.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 0, true);
}


void GravityBarShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;

}
void GravityBarShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}


///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////

GravityPointerShader::GravityPointerShader(PxScene* pPxScene)
{
	m_pPxScene = pPxScene;
	m_ui.fGravity = m_pPxScene->getGravity().y;
}

GravityPointerShader::~GravityPointerShader()
{
	ReleaseShaderVariables();
}

D3D12_INPUT_LAYOUT_DESC GravityPointerShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}

void GravityPointerShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Map Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //UI
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[1].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDesc;
	::ZeroMemory(&d3dSamplerDesc, sizeof(D3D12_STATIC_SAMPLER_DESC));
	d3dSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDesc.MipLODBias = 0;
	d3dSamplerDesc.MaxAnisotropy = 1;
	d3dSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDesc.MinLOD = 0;
	d3dSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDesc.ShaderRegister = 0;
	d3dSamplerDesc.RegisterSpace = 0;
	d3dSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 1;
	d3dRootSignatureDesc.pStaticSamplers = &d3dSamplerDesc;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	HRESULT hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&m_pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}


D3D12_SHADER_BYTECODE GravityPointerShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "GRAVITYPOINTER_VS", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE GravityPointerShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}

void GravityPointerShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbUI = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbUI->Map(0, NULL, (void **)&m_pcbMappedUI);
}


void GravityPointerShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_ui.fGravity = m_pPxScene->getGravity().y;

	::memcpy(m_pcbMappedUI, &m_ui, sizeof(UI_INFO));
}

void GravityPointerShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	CShader::CreateShader(pd3dDevice, pd3dGraphicsRootSignature, nRenderTargets);
}

void GravityPointerShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/GravityPointer.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 1, true);
}


void GravityPointerShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;

}
void GravityPointerShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CalculatePointer();

	CShader::Render(pd3dCommandList, pCamera);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dcbUIGpuVirtualAddress);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void GravityPointerShader::ReleaseShaderVariables()
{
	m_pd3dcbUI->Unmap(0, NULL);
}

void GravityPointerShader::CalculatePointer()
{
	if (m_ui.fGravity == 2) {
		m_ui.xmf2Arrow.y = 0.2;
	}
	else if (m_ui.fGravity == 1) {
		m_ui.xmf2Arrow.y = 0.1;
	}
	else if (m_ui.fGravity == 0) {
		m_ui.xmf2Arrow.y = 0.0;
	}
	else if (m_ui.fGravity == -1) {
		m_ui.xmf2Arrow.y = -0.1;
	}
	else if (m_ui.fGravity == -2) {
		m_ui.xmf2Arrow.y = -0.2;
	}
	else if (m_ui.fGravity == -3) {
		m_ui.xmf2Arrow.y = -0.3;
	}
	else if (m_ui.fGravity == -4) {
		m_ui.xmf2Arrow.y = -0.4;
	}
	else if (m_ui.fGravity == -5) {
		m_ui.xmf2Arrow.y = -0.5;
	}
	else if (m_ui.fGravity == -6) {
		m_ui.xmf2Arrow.y = -0.6;
	}
	else if (m_ui.fGravity == -7) {
		m_ui.xmf2Arrow.y = -0.7;
	}
	else if (m_ui.fGravity == -8) {
		m_ui.xmf2Arrow.y = -0.8;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TimeNumberShader::TimeNumberShader() :NumberShader()
{
}
TimeNumberShader::~TimeNumberShader()
{}
D3D12_SHADER_BYTECODE TimeNumberShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "SCORENUMBER_VS", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE TimeNumberShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}
void TimeNumberShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/number.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 1, true);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void TimeNumberShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedUI, &m_ui, sizeof(UI_INFO));
}
void TimeNumberShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbUI = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbUI->Map(0, NULL, (void **)&m_pcbMappedUI);
}
void TimeNumberShader::SetAndCalculateScoreLocation(UINT nTime)
{
	UINT nTimes[3];

	nTimes[0] = nTime / 100;
	nTimes[1] = (nTime - (nTimes[0] * 100)) / 10;
	nTimes[2] = nTime % 10;

	for (int i = 0; i < 3; i++)
	{
		m_ui.xmf4TimeNumber[i] = CalculateNumberTexture(nTimes[i]);
	}
}

void TimeNumberShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dcbUIGpuVirtualAddress);

	std::chrono::seconds ms;

	ms = std::chrono::duration_cast<std::chrono::seconds>(chrono::system_clock::now() - start);

	UINT nTime = ms.count();

	if (nTime > 999)
	{
		start = chrono::system_clock::now();
		nTime = 0;
	}
	SetAndCalculateScoreLocation(nTime);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(18, 1, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HPNumberShader::HPNumberShader() :NumberShader()
{
}
HPNumberShader::~HPNumberShader()
{}
D3D12_SHADER_BYTECODE HPNumberShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "HP_NUMBER_VS", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE HPNumberShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
}
void HPNumberShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/number.dds", 0);

	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_pTexture->GetTextureCount());
	CreateShaderResourceViews(pd3dDevice, pd3dCommandList, m_pTexture, 1, true);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void HPNumberShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedUI, &m_ui, sizeof(UI_INFO));
}
void HPNumberShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbUI = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbUI->Map(0, NULL, (void **)&m_pcbMappedUI);
}
void HPNumberShader::SetAndCalculateHPLocation(UINT nTime)
{
	UINT nTimes[3];

	nTimes[0] = nTime / 100;
	nTimes[1] = (nTime - (nTimes[0] * 100)) / 10;
	nTimes[2] = nTime % 10;
	
	for (int i = 0; i < 3; i++)
	{
		m_ui.xmf4HPNumber[i] = CalculateNumberTexture(nTimes[i]);
	}

	if (nTimes[0] == 0)
	{
		m_ui.xmf4HPNumber[0].x = 0.0f;
		m_ui.xmf4HPNumber[0].y = 0.0f;
		m_ui.xmf4HPNumber[0].z = 0.0f;
		m_ui.xmf4HPNumber[0].w = 0.0f;
	}
}

void HPNumberShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dCbvSrvDescriptorHeap);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(0, d3dcbUIGpuVirtualAddress);

	SetAndCalculateHPLocation(100);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pTexture)
		m_pTexture->UpdateShaderVariables(pd3dCommandList);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(18, 1, 0, 0);
}