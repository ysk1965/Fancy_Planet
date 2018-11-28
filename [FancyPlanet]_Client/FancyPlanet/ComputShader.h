#pragma once
#include "Shader.h"
enum {Mul = 0, Horizontal = 1, Vertical = 2, Add = 3 };
enum {A= 0, B= 1, OriginScene = 2};
enum { Input = 0, EMISSIVE = 1, SCENE = 2, Output = 3};

#define DAMAGE_TIME 0.5f;
#define VIBRATION_TIME 1.5f

struct ComputeConstant
{
	float fRed;
	UINT fVibration;
};
class CComputShader
{
	CTexture *m_pTexture = NULL;
	static const UINT            m_nTexture = 3;
	static const UINT            m_nBlurCount = 2;

	ID3D12PipelineState** m_ppd3dPipelineStates = NULL;
	ID3D12RootSignature* pd3dComputeRootSignature = NULL;
	UINT m_nPipelineStates = 1;

	ID3D12DescriptorHeap	* m_pd3dUavSrvDescriptorHeap = NULL;

	ID3D12Resource* m_pd3dTexA = NULL;
	ID3D12Resource* m_pd3dTexB = NULL;
	ID3D12Resource* m_pd3dOriginSceneTex = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dUavCPUDescriptorStartHandle[m_nTexture];
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dUavGPUDescriptorStartHandle[m_nTexture];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle[m_nTexture];
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle[m_nTexture];
	
	D3D12_CPU_DESCRIPTOR_HANDLE    m_d3dStencilMapCPUHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE    m_d3dStencilMapGPUHandle;
	
	ComputeConstant m_Value;

	float m_fDamage = 1.0f;
	float m_fVibration = 1.0f;

	DWORD m_dwDamage;
	DWORD m_dwVibration;

	UINT m_nWndClientWidth;
	UINT m_nWndClientHeight;
public:
	CComputShader(ID3D12Device *pd3dDevice);
	~CComputShader();

	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType);
	
	void CalcComputeEffect();
	void SetComputeEffect(UINT nIndex);
	void CreateComputeRootSignature(ID3D12Device *pd3dDevice);
	void CreateUavAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nConstantBufferViews, int nShaderResourceViews);
	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture, ID3D12Resource* pStencilMap);
	void CreateUnorderedAccessViews(ID3D12Device *pd3dDevice, CTexture *pTexture);

	void ReleaseShaderVariable();
	void BuildPSO(ID3D12Device *pd3dDevice);
	void BuildTextures(ID3D12Device *pd3dDevice, ID3D12Resource* pStencilMap, UINT m_nWndClientWidth, UINT nWndClientHeight);
	D3D12_SHADER_BYTECODE CreateComputeShader(ID3DBlob **ppd3dShaderBlob, UINT nIndex);

	void Compute(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pBackBuffer);

};

