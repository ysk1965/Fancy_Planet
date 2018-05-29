#include "stdafx.h"
#include "ComputShader.h"

D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob)
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

D3D12_SHADER_RESOURCE_VIEW_DESC CComputShader::GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType)
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

void CComputShader::CreateUavAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nUnorderedAccessViews, int nShaderResourceViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nUnorderedAccessViews + nShaderResourceViews;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dUavSrvDescriptorHeap);

	for (int i = 0; i < m_nTexture; i++)
	{
		m_d3dUavCPUDescriptorStartHandle[i].ptr = m_pd3dUavSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (gnCbvSrvDescriptorIncrementSize * i);
		m_d3dUavGPUDescriptorStartHandle[i].ptr = m_pd3dUavSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + (gnCbvSrvDescriptorIncrementSize * i);
		m_d3dSrvCPUDescriptorStartHandle[i].ptr = m_d3dUavCPUDescriptorStartHandle[i].ptr + (::gnCbvSrvDescriptorIncrementSize * nUnorderedAccessViews);
		m_d3dSrvGPUDescriptorStartHandle[i].ptr = m_d3dUavGPUDescriptorStartHandle[i].ptr + (::gnCbvSrvDescriptorIncrementSize * nUnorderedAccessViews);
	}
}
void CComputShader::CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture)
{
	int nTextures = pTexture->GetTextureCount();

	for (int i = 0; i < nTextures; i++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE d3dSrvCPUDescriptorHandle = m_d3dSrvCPUDescriptorStartHandle[i];
		
		int nTextureType = pTexture->GetTextureType();
	
		ID3D12Resource *pShaderResource = pTexture->GetTexture(i);
		D3D12_RESOURCE_DESC d3dResourceDesc = pShaderResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC d3dShaderResourceViewDesc = GetShaderResourceViewDesc(d3dResourceDesc, nTextureType);
		pd3dDevice->CreateShaderResourceView(pShaderResource, &d3dShaderResourceViewDesc, d3dSrvCPUDescriptorHandle);
	}
}
void CComputShader::CreateUnorderedAccessViews(ID3D12Device *pd3dDevice, CTexture *pTexture)
{
	int nTextures = pTexture->GetTextureCount();

	for (int n = 0; n < nTextures; n++)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE d3dUavCPUDescriptorHandle = m_d3dUavCPUDescriptorStartHandle[n];

		ID3D12Resource *pResource = pTexture->GetTexture(n);

		pd3dDevice->CreateUnorderedAccessView(pResource, nullptr, NULL, d3dUavCPUDescriptorHandle);
	}
}
CComputShader::CComputShader(UINT nWndClientWidth, UINT nWndClientHeight)
{
	m_nWndClientWidth = nWndClientWidth;
	m_nWndClientHeight = nWndClientHeight;

	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];
}


CComputShader::~CComputShader()
{
	if (m_pd3dTexA)
		m_pd3dTexA->Release();

	if (m_pd3dTexB)
		m_pd3dTexB->Release();
}

void CComputShader::CreateComputeRootSignature(ID3D12Device *pd3dDevice)
{

	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 0;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 0; 
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0]);
	pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1]);
	
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	CD3DX12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(CD3DX12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	HRESULT hresult = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	hresult = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&pd3dComputeRootSignature);
	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();
}

void CComputShader::BuildPSO(ID3D12Device *pd3dDevice)
{
	ID3DBlob *pd3dComputeShaderBlob = NULL;
	D3D12_COMPUTE_PIPELINE_STATE_DESC d3dPipelineStateDesc = {};

	d3dPipelineStateDesc.pRootSignature = pd3dComputeRootSignature;
	d3dPipelineStateDesc.CS = CreateComputeShader(&pd3dComputeShaderBlob);
	d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateComputePipelineState(&d3dPipelineStateDesc, IID_PPV_ARGS(&m_ppd3dPipelineStates[0]));

}

void CComputShader::BuildTextures(ID3D12Device *pd3dDevice)
{
	m_pTexture = new CTexture(m_nTexture, RESOURCE_TEXTURE2D, 0);
	
	//버퍼를 만든다.
	m_pd3dTexA = m_pTexture->ComputeCreateTexture(pd3dDevice, m_nWndClientWidth, m_nWndClientHeight
		, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, NULL, 0);
	m_pd3dTexA->AddRef();

	m_pd3dTexB = m_pTexture->ComputeCreateTexture(pd3dDevice, m_nWndClientWidth, m_nWndClientHeight
		, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON, NULL, 1);
	m_pd3dTexB->AddRef();
	
	CreateUavAndSrvDescriptorHeaps(pd3dDevice, m_nTexture, m_nTexture);
	CreateUnorderedAccessViews(pd3dDevice, m_pTexture);
	CreateShaderResourceViews(pd3dDevice, m_pTexture);
}
D3D12_SHADER_BYTECODE CComputShader::CreateComputeShader(ID3DBlob **ppd3dShaderBlob)
{
	return CompileShaderFromFile(L"Compute.hlsl", "CS", "cs_5_1", ppd3dShaderBlob);
}

void CComputShader::Compute(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pBackBuffer)
{
	pd3dCommandList->SetComputeRootSignature(pd3dComputeRootSignature);
	pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);
	pd3dCommandList->SetDescriptorHeaps(1, &m_pd3dUavSrvDescriptorHeap);

	::SynchronizeResourceTransition(pd3dCommandList, m_pd3dTexA, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

	pd3dCommandList->CopyResource(m_pd3dTexA, pBackBuffer);

	::SynchronizeResourceTransition(pd3dCommandList, m_pd3dTexA, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	::SynchronizeResourceTransition(pd3dCommandList, m_pd3dTexB, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	pd3dCommandList->SetComputeRootDescriptorTable(0, m_d3dSrvGPUDescriptorStartHandle[A]);
	pd3dCommandList->SetComputeRootDescriptorTable(1, m_d3dUavGPUDescriptorStartHandle[B]);
	
	// How many groups do we need to dispatch to cover a row of pixels, where each
	// group covers 256 pixels (the 256 is defined in the ComputeShader).
	UINT nGroupsX = (UINT)ceilf(m_nWndClientWidth / 256.0f);

	pd3dCommandList->Dispatch(nGroupsX, m_nWndClientHeight, 1);
	
	::SynchronizeResourceTransition(pd3dCommandList, m_pd3dTexA, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, m_pd3dTexB, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	::SynchronizeResourceTransition(pd3dCommandList, pBackBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

	pd3dCommandList->CopyResource(pBackBuffer, m_pd3dTexB);

	::SynchronizeResourceTransition(pd3dCommandList, m_pd3dTexB, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
	::SynchronizeResourceTransition(pd3dCommandList, pBackBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
}