#pragma once
#include "Shader.h"

#define MAP_SIZE 2056.0f
#define PLAYER_MAP_RANGE 200.0f
#define MINI_MAP_SIZE 0.25f;
#define ARROW_CENTER -0.75f

struct UI_INFO
{
	XMFLOAT2 xmf2Map1;
	XMFLOAT2 xmf2Map2;
	XMFLOAT2 xmf2Degree;
	XMFLOAT2 xmf2Arrow;
	XMFLOAT4 xmf4Number;
};

class MiniMapShader : public CShader
{
public:
	MiniMapShader(CPlayer * pPlayer);
	virtual ~MiniMapShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	void CalculateMiniMap();

private:
	CTexture * m_pTexture;

protected:
	CPlayer * m_pPlayer;

	UI_INFO		 m_ui;
	ID3D12Resource					*m_pd3dcbUI = NULL;
	UI_INFO		*m_pcbMappedUI = NULL;
};

class ArrowShader : public MiniMapShader
{
public:
	ArrowShader(CPlayer * pPlayer);
	virtual ~ArrowShader();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	void CalculateArrow();
private:
	CPlayer * m_pPlayer;
	CTexture*	 m_pTexture;
};

class NumberShader : public MiniMapShader
{
public:
	NumberShader();
	virtual ~NumberShader();

	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	XMFLOAT4& CalculateNumberTexture(UINT xmf4Number);
private:
	XMFLOAT4 m_xmf4Number;
	std::chrono::system_clock::time_point start;

	CPlayer * m_pPlayer;
	CTexture*	 m_pTexture;
};


class CrossShader : public CShader
{
public:
	CrossShader();
	virtual ~CrossShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void ReleaseObjects();
private:
	CTexture * m_pTexture;
};


//MapUIShader

class MiniUIShader : public CShader
{
public:
	MiniUIShader();
	virtual ~MiniUIShader();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

	void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void ReleaseObjects();
private:
	CTexture * m_pTexture;
};