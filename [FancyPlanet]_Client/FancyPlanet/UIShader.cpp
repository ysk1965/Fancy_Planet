#include "stdafx.h"
#include "UIShader.h"
#include <math.h>


UIShader::UIShader(CPlayer* pPlayer, UINT nCharaterType)
{
	m_bLobby = false;
	m_nCharaterType = nCharaterType;
	m_pPlayer = pPlayer;
	m_bGaze = false;
}
UIShader::~UIShader()
{
	if (m_ppd3dPipelineStates)
	{
		for (int i = 0; i < m_nPipelineStates; i++)
			if (m_ppd3dPipelineStates[i])
				m_ppd3dPipelineStates[i]->Release();
		delete[] m_ppd3dPipelineStates;
	}

	if (m_pd3dGraphicsRootSignature)
		m_pd3dGraphicsRootSignature->Release();
}
D3D12_SHADER_BYTECODE UIShader::CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob)
{
	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *pd3dErrorBlob = NULL;
	HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszShaderName, pszShaderProfile, nCompileFlags, 0, ppd3dShaderBlob, &pd3dErrorBlob);

	if (pd3dErrorBlob)
	{
		OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
		pd3dErrorBlob->Release();
	}

	D3D12_SHADER_BYTECODE d3dShaderByteCode;
	d3dShaderByteCode.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
	d3dShaderByteCode.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();

	return(d3dShaderByteCode);
}
void UIShader::CreateShader(ID3D12Device *pd3dDevice, UINT nRenderTargets)
{
	m_nPipelineStates = 18;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];

	for (int i = 0; i < m_nPipelineStates; i++)
		CreatePSO(pd3dDevice, nRenderTargets, i);
}
D3D12_SHADER_RESOURCE_VIEW_DESC UIShader::GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc;
	d3dShaderResourceViewDesc.Format = d3dResourceDesc.Format;
	d3dShaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	switch (nTextureType)
	{
	case RESOURCE_TEXTURE2D: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 1)
	case RESOURCE_TEXTURE2D_ARRAY:
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		d3dShaderResourceViewDesc.Texture2D.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2D.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_TEXTURE2DARRAY: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize != 1)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		d3dShaderResourceViewDesc.Texture2DArray.MipLevels = -1;
		d3dShaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.Texture2DArray.PlaneSlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
		d3dShaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
		d3dShaderResourceViewDesc.Texture2DArray.ArraySize = d3dResourceDesc.DepthOrArraySize;
		break;
	case RESOURCE_TEXTURE_CUBE: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)(d3dResourceDesc.DepthOrArraySize == 6)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		d3dShaderResourceViewDesc.TextureCube.MipLevels = -1;
		d3dShaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
		d3dShaderResourceViewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		break;
	case RESOURCE_BUFFER: //(d3dResourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		d3dShaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		d3dShaderResourceViewDesc.Buffer.FirstElement = 0;
		d3dShaderResourceViewDesc.Buffer.NumElements = 0;
		d3dShaderResourceViewDesc.Buffer.StructureByteStride = 0;
		d3dShaderResourceViewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		break;
	}
	return(d3dShaderResourceViewDesc);
}
D3D12_INPUT_LAYOUT_DESC UIShader::CreateInputLayout()
{
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = NULL;
	d3dInputLayoutDesc.NumElements = 0;

	return(d3dInputLayoutDesc);
}
D3D12_RASTERIZER_DESC UIShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}
D3D12_DEPTH_STENCIL_DESC UIShader::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
	d3dDepthStencilDesc.DepthEnable = FALSE;
	d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.StencilEnable = FALSE;
	d3dDepthStencilDesc.StencilReadMask = 0x00;
	d3dDepthStencilDesc.StencilWriteMask = 0x00;
	d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	d3dDepthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	return(d3dDepthStencilDesc);
}
D3D12_BLEND_DESC CreateBlendState(bool bType)
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	if (bType)
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
	else
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

	return(d3dBlendDesc);
}
void UIShader::CreatePSO(ID3D12Device *pd3dDevice, UINT nRenderTargets, UINT nIndex)
{
	ID3DBlob *pd3dVertexShaderBlob = NULL, *pd3dPixelShaderBlob = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = m_pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob, nIndex);

	if (nIndex == SKILL1_COVER || nIndex == SKILL2_COVER)
		d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, 1);
	else if (nIndex == OCCUPATION_GAZE_COVER)
		d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, 2);
	else
		d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob, 0);

	d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();

	if (nIndex == SKILL1_COVER || nIndex == SKILL2_COVER || nIndex == OCCUPATION_GAZE_COVER)
		d3dPipelineStateDesc.BlendState = CreateBlendState(false);
	else
		d3dPipelineStateDesc.BlendState = CreateBlendState(true);

	d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = nRenderTargets;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState),
		(void **)&m_ppd3dPipelineStates[nIndex]);

	if (pd3dVertexShaderBlob)
		pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob)
		pd3dPixelShaderBlob->Release();

	if (d3dPipelineStateDesc.InputLayout.pInputElementDescs)
		delete[] d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}
D3D12_SHADER_BYTECODE UIShader::CreateVertexShader(ID3DBlob **ppd3dShaderBlob, UINT nIndex)
{
	if (nIndex == MINIMAP_UI)
		return(CompileShaderFromFile(L"UI.hlsl", "MINIUI_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == ARROW)
		return(CompileShaderFromFile(L"UI.hlsl", "ARROW_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == MINIMAP)
		return(CompileShaderFromFile(L"UI.hlsl", "MINIMAP_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == CROSS)
		return(CompileShaderFromFile(L"UI.hlsl", "CROSS_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == TIME_NUMBER)
		return(CompileShaderFromFile(L"UI.hlsl", "TIME_NUMBER_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == SCOREBOARD)
		return(CompileShaderFromFile(L"UI.hlsl", "SCOREBOARD_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == POINTER)
		return(CompileShaderFromFile(L"UI.hlsl", "GRAVITYPOINTER_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == HP_NUMBER)
		return(CompileShaderFromFile(L"UI.hlsl", "HP_NUMBER_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == GRAVITY)
		return(CompileShaderFromFile(L"UI.hlsl", "GRAVITYBAR_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == SCORE_NUMBER)
		return(CompileShaderFromFile(L"UI.hlsl", "SCORE_NUMBER_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == SKILL1)
		return(CompileShaderFromFile(L"UI.hlsl", "SKILL1_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == SKILL2)
		return(CompileShaderFromFile(L"UI.hlsl", "SKILL2_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == SKILL1_COVER)
		return(CompileShaderFromFile(L"UI.hlsl", "SKILL1_COVER_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == SKILL2_COVER)
		return(CompileShaderFromFile(L"UI.hlsl", "SKILL2_COVER_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == LOBBY)
		return(CompileShaderFromFile(L"UI.hlsl", "LOBBY_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == OCCUPATION_GAZE)
		return(CompileShaderFromFile(L"UI.hlsl", "OCCUPATION_GAZE_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == OCCUPATION_GAZE_COVER)
		return(CompileShaderFromFile(L"UI.hlsl", "OCCUPATION_GAZE_COVER_VS", "vs_5_1", ppd3dShaderBlob));
	else if (nIndex == SELECT)
		return(CompileShaderFromFile(L"UI.hlsl", "SELECT_VS", "vs_5_1", ppd3dShaderBlob));
}
D3D12_SHADER_BYTECODE UIShader::CreatePixelShader(ID3DBlob **ppd3dShaderBlob, UINT nType)
{
	if (nType == 0)
		return(CompileShaderFromFile(L"UI.hlsl", "UIPS", "ps_5_1", ppd3dShaderBlob));
	else if (nType == 1)
		return(CompileShaderFromFile(L"UI.hlsl", "UI_COVER_PS", "ps_5_1", ppd3dShaderBlob));
	else if (nType == 2)
		return(CompileShaderFromFile(L"UI.hlsl", "OCCUPATION_GAZE_COVER_PS", "ps_5_1", ppd3dShaderBlob));
}
void UIShader::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0; //Map Texture
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[0].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0]; //Texture
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 0; //UI
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

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
void UIShader::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_pTexture = new CTexture(m_nPipelineStates + 1, RESOURCE_TEXTURE2D, 0);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/map.dds", MINIMAP);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/MiniMapUI.dds", MINIMAP_UI);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/arrow.dds", ARROW);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/cross.dds", CROSS);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Scoreboard.dds", SCOREBOARD);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/number.dds", TIME_NUMBER);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/GravityBar.dds", GRAVITY);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/GravityPointer.dds", POINTER);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Human_1R.dds", H_1);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Human_2R.dds", H_2);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Protoss_1B.dds", P_1);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Protoss_2B.dds", P_2);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Zerg_1G.dds", Z_1);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Zerg_2G.dds", Z_2);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Lobby.dds", LOBBY);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Dominatebar.dds", OCCUPATION_GAZE);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Selected.dds", SELECT - 1);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Result_win.dds", END_W);
	m_pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Result_lose.dds", END_L);

	CreateSrvDescriptorHeaps(pd3dDevice, m_pTexture->GetTextureCount());
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateShaderResourceViews(pd3dDevice, m_pTexture);
}
void UIShader::ReleaseObjects()
{
	if (m_pTexture)
		delete m_pTexture;
}
void UIShader::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	::memcpy(m_pcbMappedUI, &m_ui, sizeof(UI_INFO));
}
void UIShader::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(UI_INFO) + 255) & ~255); //256ÀÇ ¹è¼ö
	m_pd3dcbUI = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes,
		D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbUI->Map(0, NULL, (void **)&m_pcbMappedUI);
}
void UIShader::ReleaseShaderVariables()
{
	m_pd3dcbUI->Unmap(0, NULL);
}
void UIShader::SetAndCalculateHPLocation(UINT nTime)
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

	if (nTimes[0] == 0 && nTimes[1] == 0)
	{
		m_ui.xmf4HPNumber[1].x = 0.0f;
		m_ui.xmf4HPNumber[1].y = 0.0f;
		m_ui.xmf4HPNumber[1].z = 0.0f;
		m_ui.xmf4HPNumber[1].w = 0.0f;
	}
}
void UIShader::SetOccupationGauge(float fTime)
{
	if (fTime > 0.0f)
		m_bGaze = true;
	else
		m_bGaze = false;

	m_ui.fGaze = fTime / 5.0f;
}
void UIShader::SetAndCalculateTimeLocation(UINT nTime)
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

void UIShader::SetAndCalculateScoreLocation(UINT *pRed, UINT *pBlue, UINT *pGreen)
{
	m_ui.xmf4ScoreNumber[0] = CalculateNumberTexture(pRed[0]);
	m_ui.xmf4ScoreNumber[1] = CalculateNumberTexture(pRed[1]);
	m_ui.xmf4ScoreNumber[2] = CalculateNumberTexture(pBlue[0]);
	m_ui.xmf4ScoreNumber[3] = CalculateNumberTexture(pBlue[1]);
	m_ui.xmf4ScoreNumber[4] = CalculateNumberTexture(pGreen[0]);
	m_ui.xmf4ScoreNumber[5] = CalculateNumberTexture(pGreen[1]);
}
void UIShader::CalculateMiniMap()
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
void UIShader::CalculateArrow()
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
XMFLOAT4& UIShader::CalculateNumberTexture(UINT nNumber)
{
	XMFLOAT4 xmf4Result;

	xmf4Result.x = (nNumber % 5) / 5.0f;
	xmf4Result.y = ((UINT)(nNumber / 5)) / 2.0f;
	xmf4Result.z = (((nNumber) % 5) + 1) / 5.0f;
	xmf4Result.w = ((UINT)(nNumber / 5) + 1) / 2.0f;

	return xmf4Result;
}
void UIShader::CalculatePointer(int nGravity)
{
	float fValue = (nGravity + 11.0f) / 13.0f;

	m_ui.xmf2Point.x = (1.0f - fValue);
}
void UIShader::SetSkillCover(float fTime1, float fTime2)
{
	m_ui.xmf2SKill.x = fTime1;
	m_ui.xmf2SKill.y = fTime2;
}
void UIShader::SetGameStart(UINT nCharaterType)
{
	m_nCharaterType = nCharaterType;

	if (m_nCharaterType == 0)
		m_ui.CoverColor = XMFLOAT4(0.8f, 0.1f, 0.1f, 0.7f);
	else if (m_nCharaterType == 1)
		m_ui.CoverColor = XMFLOAT4(0.1f, 0.1f, 0.8f, 0.7f);
	else
		m_ui.CoverColor = XMFLOAT4(0.1f, 0.8f, 0.1f, 0.7f);

	m_bLobby = true;
}
void UIShader::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture)
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGPUDescriptorHandle = m_d3dSrvGPUDescriptorStartHandle;

	int nTextures = pTexture->GetTextureCount();
	int nTextureType = pTexture->GetTextureType();
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
}
void UIShader::CreateSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nShaderResourceViews;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dSrvDescriptorHeap);

	m_d3dSrvCPUDescriptorStartHandle = m_pd3dSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dSrvGPUDescriptorStartHandle = m_pd3dSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}
void UIShader::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	CalculateArrow();
	CalculateMiniMap();

	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescriptorHeap);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dcbUIGpuVirtualAddress);

	UpdateShaderVariables(pd3dCommandList);

	for (int i = 0; i < m_nPipelineStates; i++)
	{
		if (i != LOBBY && i != SELECT)
		{
			if (m_ppd3dPipelineStates)
				pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[i]);

			if (i == TIME_NUMBER || i == HP_NUMBER || i == SCORE_NUMBER)
				pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(TIME_NUMBER).m_d3dSrvGpuDescriptorHandle);
			else if (i == SKILL1)
			{
				if (m_nCharaterType == 0)
					pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(H_1).m_d3dSrvGpuDescriptorHandle);
				else if (m_nCharaterType == 1)
					pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(P_1).m_d3dSrvGpuDescriptorHandle);
				else
					pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(Z_1).m_d3dSrvGpuDescriptorHandle);
			}
			else if (i == SKILL2)
			{
				if (m_nCharaterType == 0)
					pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(H_2).m_d3dSrvGpuDescriptorHandle);
				else if (m_nCharaterType == 1)
					pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(P_2).m_d3dSrvGpuDescriptorHandle);
				else
					pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(Z_2).m_d3dSrvGpuDescriptorHandle);
			}
			else if (i != SKILL1_COVER && i != SKILL2_COVER && i != OCCUPATION_GAZE_COVER)
				pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(i).m_d3dSrvGpuDescriptorHandle);

			pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			if (i == TIME_NUMBER || i == HP_NUMBER)
				pd3dCommandList->DrawInstanced(18, 1, 0, 0);
			else if (i == SCORE_NUMBER)
				pd3dCommandList->DrawInstanced(36, 1, 0, 0);
			else if (i == SKILL1_COVER)
			{
				if (m_ui.xmf2SKill.x < 1.0f)
					pd3dCommandList->DrawInstanced(6, 1, 0, 0);
			}
			else if (i == SKILL2_COVER)
			{
				if (m_ui.xmf2SKill.y < 1.0f)
					pd3dCommandList->DrawInstanced(6, 1, 0, 0);
			}
			else if (i == OCCUPATION_GAZE_COVER || i == OCCUPATION_GAZE)
			{
				if (m_bGaze)
					pd3dCommandList->DrawInstanced(6, 1, 0, 0);
			}
			else
				pd3dCommandList->DrawInstanced(6, 1, 0, 0);
		}
	}
}
void UIShader::SetSelect(UINT nCharacter)
{
	m_ui.xmf4Select.x = 0.39f;
	m_ui.xmf4Select.z = 1.0f;

	if (nCharacter == SOLDIER)
	{
		m_ui.xmf4Select.y = 0.606f;
		m_ui.xmf4Select.w = 0.306f;
	}
	else if (nCharacter == DRONE)
	{
		m_ui.xmf4Select.y = 0.34f;
		m_ui.xmf4Select.w = 0.04f;
	}
	else
	{
		m_ui.xmf4Select.y = 0.07f;
		m_ui.xmf4Select.w = -0.24f;
	}
}
void UIShader::RobbyRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescriptorHeap);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dcbUIGpuVirtualAddress);

	UpdateShaderVariables(pd3dCommandList);

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[LOBBY]);

	pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(LOBBY).m_d3dSrvGpuDescriptorHandle);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);

	pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[SELECT]);

	pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(SELECT - 1).m_d3dSrvGpuDescriptorHandle);

	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}

void UIShader::EndRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bResult)
{
	if (m_pd3dGraphicsRootSignature)
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dSrvDescriptorHeap);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbUIGpuVirtualAddress = m_pd3dcbUI->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dcbUIGpuVirtualAddress);

	UpdateShaderVariables(pd3dCommandList);

	if (m_ppd3dPipelineStates)
		pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[LOBBY]);

	if (bResult)
		pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(END_W).m_d3dSrvGpuDescriptorHandle);
	else
		pd3dCommandList->SetGraphicsRootDescriptorTable(0, m_pTexture->GetArgumentInfos(END_L).m_d3dSrvGpuDescriptorHandle);


	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pd3dCommandList->DrawInstanced(6, 1, 0, 0);
}