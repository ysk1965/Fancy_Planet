#pragma once
#include "Shader.h"

enum {A= 0, B= 1};

class CComputShader
{
	CTexture *m_pTexture = NULL;
	static const UINT            m_nTexture = 2;
	UINT m_nWndClientWidth = 0;
	UINT m_nWndClientHeight = 0;

	ID3D12PipelineState** m_ppd3dPipelineStates = NULL;
	ID3D12RootSignature* pd3dComputeRootSignature = NULL;
	UINT m_nPipelineStates = 1;

	ID3D12DescriptorHeap	* m_pd3dUavSrvDescriptorHeap = NULL;

	ID3D12Resource* m_pd3dTexA = NULL;
	ID3D12Resource* m_pd3dTexB = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dUavCPUDescriptorStartHandle[m_nTexture];
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGPUDescriptorStartHandle[m_nTexture];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle[m_nTexture];
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle[m_nTexture];
	
	float m_fValue;
public:
	CComputShader(UINT m_nWndClientWidth, UINT nWndClientHeight);
	~CComputShader();

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType);

	void CreateComputeRootSignature(ID3D12Device *pd3dDevice);
	void CreateUavAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture);
	void CreateUnorderedAccessViews(ID3D12Device *pd3dDevice, CTexture *pTexture);

	void BuildPSO(ID3D12Device *pd3dDevice);
	void BuildTextures(ID3D12Device *pd3dDevice);
	D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob **ppd3dShaderBlob);

	void Compute(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pBackBuffer);

	void SetValue(float fValue)
	{
		m_fValue = fValue;
	}
};

