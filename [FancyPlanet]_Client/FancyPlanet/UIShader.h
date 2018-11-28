#pragma once
#include "Shader.h"

enum
{
	MINIMAP = 0,
	MINIMAP_UI = 1,
	ARROW = 2,
	CROSS = 3,
	SCOREBOARD = 4,
	TIME_NUMBER = 5,
	GRAVITY = 6,
	POINTER = 7,
	HP_NUMBER = 8,
	SCORE_NUMBER = 9,
	SKILL1 = 10,
	SKILL2 = 11,
	SKILL1_COVER = 12,
	SKILL2_COVER = 13,
	LOBBY = 14,
	OCCUPATION_GAZE = 15,
	OCCUPATION_GAZE_COVER = 16,
	SELECT = 17,
	END_W = 17,
	END_L = 18,
};

enum
{
	H_1 = 8,
	H_2 = 9,
	P_1 = 10,
	P_2 = 11,
	Z_1 = 12,
	Z_2 = 13
};

struct UI_INFO
{
	XMFLOAT2 xmf2Map1;
	XMFLOAT2 xmf2Map2;
	XMFLOAT2 xmf2Degree;
	XMFLOAT2 xmf2Arrow;
	XMFLOAT4 xmf4TimeNumber[3];
	XMFLOAT4 xmf4HPNumber[3];
	XMFLOAT4 xmf4ScoreNumber[6];
	XMFLOAT2 xmf2SKill;
	XMFLOAT2 xmf2Point;
	XMFLOAT4 CoverColor;
	XMFLOAT4 xmf4Select;
	float	 fGaze;
	float	 fGravity;
};

class UIShader
{
public:
	UIShader(CPlayer* pPlayer, UINT nCharaterType);
	virtual~UIShader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(D3D12_RESOURCE_DESC d3dResourceDesc, UINT nTextureType);

	D3D12_RASTERIZER_DESC CreateRasterizerState();
	D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	void CreateShader(ID3D12Device *pd3dDevice, UINT nRenderTargets);
	void CreatePSO(ID3D12Device *pd3dDevice, UINT nRenderTargets, UINT nIndex);

	D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, UINT nIndex);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob, UINT nType);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	void CreateSrvDescriptorHeaps(ID3D12Device *pd3dDevice, int nShaderResourceViews);
	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, CTexture *pTexture);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void RobbyRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	virtual void EndRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, bool bResult);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void CalculateMiniMap();
	void CalculateArrow();
	XMFLOAT4& CalculateNumberTexture(UINT xmf4Number);
	void CalculatePointer(int nGravity);
	void SetAndCalculateHPLocation(UINT nTime);
	void SetOccupationGauge(float fTime);
	void SetAndCalculateTimeLocation(UINT nTime);
	void SetAndCalculateScoreLocation(UINT *pRed, UINT *pBlue, UINT *pGreen);
	void SetSkillCover(float fTime1, float fTime2);
	void SetGameStart(UINT nCharaterType);
	void SetSelect(UINT nCharacter);
private:
	CTexture * m_pTexture;
	UINT m_nCharaterType;

	int								m_nPipelineStates = 0;
	ID3D12PipelineState				**m_ppd3dPipelineStates = NULL;

	ID3D12DescriptorHeap			*m_pd3dSrvDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;

	ID3D12RootSignature				*m_pd3dGraphicsRootSignature = NULL;
	bool									m_bLobby;
	bool									m_bGaze;
protected:
	CPlayer * m_pPlayer;

	UI_INFO		 m_ui;
	ID3D12Resource					*m_pd3dcbUI = NULL;
	UI_INFO		*m_pcbMappedUI = NULL;
};
