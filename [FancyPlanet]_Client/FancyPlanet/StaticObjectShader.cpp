#include "stdafx.h"
#include "Shader.h"

StaticObjectShader::StaticObjectShader()
{}
StaticObjectShader::~StaticObjectShader()
{}


void StaticObjectShader::CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 3;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	for (int i = 0; i < m_nPipelineStates; i++)
		CreatePSO(pd3dDevice, pd3dGraphicsRootSignature, i);
}
D3D12_BLEND_DESC StaticObjectShader::CreateBlendState(bool bFlag)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	if (bFlag)
	{
		d3dBlendDesc.AlphaToCoverageEnable = false;
		d3dBlendDesc.IndependentBlendEnable = false;
		d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
		d3dBlendDesc.RenderTarget[0].LogicOpEnable = false;
		d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	else
	{
		d3dBlendDesc.AlphaToCoverageEnable = FALSE;
		d3dBlendDesc.IndependentBlendEnable = FALSE;
		d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
		d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
		d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	return(d3dBlendDesc);
}
void StaticObjectShader::CreatePSO(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nIndex)
{
	ID3DBlob *pd3dVertexShaderBlob = NULL, *pd3dPixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;

	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob, nIndex + 1);
	d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, nIndex + 1);

	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();

	if (nIndex == 1 || nIndex == 2)
		d3dPipelineStateDesc.BlendState = CreateBlendState(true);
	else
		d3dPipelineStateDesc.BlendState = CreateBlendState(false);

	if (nIndex == 1 || nIndex == 2)
		d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(true);
	else
		d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState(false);

	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState),
		(void **)&m_ppd3dPipelineStates[nIndex]);

	if (nIndex == 0)
	{
		ID3DBlob *pd3dVertexShaderBlob2 = NULL;

		d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob2, 0);
		d3dPipelineStateDesc.RasterizerState.DepthBias = 100000;
		d3dPipelineStateDesc.RasterizerState.DepthBiasClamp = 0.0f;
		d3dPipelineStateDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
		d3dPipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(0, 0);
		d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		d3dPipelineStateDesc.NumRenderTargets = 0;

		hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState),
			(void **)&m_pd3dShadowPipelineState);

		if (pd3dVertexShaderBlob2)
			pd3dVertexShaderBlob2->Release();
	}
	if (pd3dVertexShaderBlob)
		pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob)
		pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs)
		delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

D3D12_DEPTH_STENCIL_DESC StaticObjectShader::CreateDepthStencilState(bool bFlag)
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = true;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dDepthStencilDesc.StencilEnable = bFlag;
	d3dDepthStencilDesc.StencilReadMask = 0xff;
	d3dDepthStencilDesc.StencilWriteMask = 0xff;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

	return(d3dDepthStencilDesc);
}
D3D12_INPUT_LAYOUT_DESC StaticObjectShader::CreateInputLayout()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC *pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	//정점 정보를 위한 입력 원소이다. 

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;
	return(d3dInputLayoutDesc);
}
D3D12_SHADER_BYTECODE StaticObjectShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob, UINT nIndex)
{
	if (nIndex == 0)
		return(CShader::CompileShaderFromFile(L"StaticObject.hlsl", "Shadow_Object_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == 1)
		return(CShader::CompileShaderFromFile(L"StaticObject.hlsl", "Object_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex >= 2)
		return(CShader::CompileShaderFromFile(L"StaticObject_Emissive.hlsl", "Object_VS", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE StaticObjectShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob, UINT nType)
{
	if (nType == 1)
		return(CShader::CompileShaderFromFile(L"StaticObject.hlsl", "Object_PS", "ps_5_1", ppd3dShaderBlob));
	else if (nType == 2)
		return(CShader::CompileShaderFromFile(L"StaticObject_Emissive.hlsl", "Object_PS", "ps_5_1", ppd3dShaderBlob));
	else if (nType == 3)
		return(CShader::CompileShaderFromFile(L"StaticObject_Emissive.hlsl", "Object_PS2", "ps_5_1", ppd3dShaderBlob));
}
void StaticObjectShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{

}
void StaticObjectShader::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, ID3D12Resource* pShadowMap)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;

	int nTextures = pTexture->GetTextureCount();
	int nTextureType = pTexture->GetTextureType();
	int nTotalTextures = nTextures;

	if (pShadowMap != NULL)
		nTotalTextures = nTextures + 1;

	for (int i = 0; i < nTextures; i++)
	{
		ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
		D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, d3dSrvCPUDescriptorHandle);
		pTexture->SetRootArgument(i, 0, d3dSrvGPUDescriptorHandle);

		d3dSrvCPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
		d3dSrvGPUDescriptorHandle.ptr += ::gnCbvSrvDescriptorIncrementSize;
	}
	if (nTotalTextures > nTextures)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceDsec;

		d3dShaderResourceDsec.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		d3dShaderResourceDsec.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceDsec.Texture2D.MipLevels = 1;
		d3dShaderResourceDsec.Texture2D.PlaneSlice = 0;
		d3dShaderResourceDsec.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceDsec.Texture2D.ResourceMinLODClamp = 0;
		d3dShaderResourceDsec.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		pd3dDevice->CreateShaderResourceView(pShadowMap, &d3dShaderResourceDsec, d3dSrvCPUDescriptorHandle);
		m_d3dShadowGPUDescriptorHandle = d3dSrvGPUDescriptorHandle;
	}
}
void StaticObjectShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap)
{
	m_pTexture = new CTexture(8, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Texture/D/Toadstool01_D.dds", 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Texture/N/Toadstool01_N.dds", 1);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Texture/D/Toadstool02_D.dds", 2);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Texture/N/Toadstool02_N.dds", 3);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Texture/E/Crystal_E.dds", 4);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Texture/N/Crystal_N.dds", 5); ;
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Texture/D/DryTree_D.dds", 6);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Texture/N/DryTree_N.dds", 7);

	CreateCbvAndSrvDescriptorHeaps(pd3dDevice, NULL, 0, m_pTexture->GetTextureCount());
	CreateShaderResourceViews(pd3dDevice, m_pTexture, pShadowMap);
}

void StaticObjectShader::OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, UINT nIndex, UINT nType)
{
	if (nIndex == 0)
		pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dDescriptorHeap);

	pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[nType]);

	if (nType == 0)
		pd3dCommandList->OMSetStencilRef(0);
	else if (nType == 1 || nType == 2)
		pd3dCommandList->OMSetStencilRef(1);

	if (nIndex <= 3)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(3, m_pTexture->GetArgumentInfos(2 * nIndex).m_d3dSrvGpuDescriptorHandle);
		pd3dCommandList->SetGraphicsRootDescriptorTable(4, m_pTexture->GetArgumentInfos(2 * nIndex + 1).m_d3dSrvGpuDescriptorHandle);
	}
}
void StaticObjectShader::OnPrepareShadowRender(ID3D12GraphicsCommandList *pd3dCommandList)
{
	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dDescriptorHeap);
	pd3dCommandList->SetPipelineState(m_pd3dShadowPipelineState);
}