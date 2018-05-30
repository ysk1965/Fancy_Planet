#pragma once
#include "Shader.h"

#define MAP_SIZE 2056.0f
#define PLAYER_MAP_RANGE 200.0f
#define MINI_MAP_SIZE 0.25f;
#define ARROW_CENTER -0.75f

enum
{
	MINIMAP_UI = 0,
	ARROW = 1,
	MINIMAP = 2,
	CROSS = 3,
	TIME_NUMBER = 4,
	SCOREBOARD = 5,
	POINTER = 6,
	HP_NUMBER = 7,
	GRAVITY = 8
};

struct UI_INFO
{
	XMFLOAT2 xmf2Map1;
	XMFLOAT2 xmf2Map2;
	XMFLOAT2 xmf2Degree;
	XMFLOAT2 xmf2Arrow;
	XMFLOAT4 xmf4TimeNumber[3];
	XMFLOAT4 xmf4HPNumber[3];
	XMFLOAT2 xmf2Point;
	float	 fGravity;
};

class UIShader
{
public:
	UIShader(CPlayer * pPlayer);
	virtual~UIShader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType);

	D3D12_RASTERIZER_DESC CreateRasterizerState();
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	void CreateShader(ID3D12Device *pd3dDevice, UINT nRenderTargets);
	void CreatePSO(ID3D12Device *pd3dDevice, UINT nRenderTargets, UINT nIndex);

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, UINT nIndex);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	void CreateSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nShaderResourceViews);
	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void CalculateMiniMap();
	void CalculateArrow();
	XMFLOAT4& CalculateNumberTexture(UINT xmf4Number);
	void CalculatePointer();
	void SetAndCalculateHPLocation(UINT nTime);
	void SetAndCalculateScoreLocation(UINT nTime);
private:
	CTexture * m_pTexture;

	int								m_nPipelineStates = 0;
	ID3D12PipelineState				**m_ppd3dPipelineStates = NULL;

	ID3D12DescriptorHeap			*m_pd3dSrvDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;

	ID3D12RootSignature				*m_pd3dGraphicsRootSignature = NULL;
protected:
	CPlayer * m_pPlayer;

	UI_INFO		 m_ui;
	ID3D12Resource					*m_pd3dcbUI = NULL;
	UI_INFO		*m_pcbMappedUI = NULL;
};
