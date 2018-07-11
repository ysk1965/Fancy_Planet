#pragma once

#include "Object.h"
#include "Camera.h"
#include "Player.h"

#define MAP_SIZE 2056.0f
#define PLAYER_MAP_RANGE 200.0f
#define MINI_MAP_SIZE 0.25f;
#define ARROW_CENTER -0.75f


class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets);

	void CreateCbvAndSrvDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, int nShaderResourceViews);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	ID3D12RootSignature *GetGraphicsRootSignature() 
	{
		return(m_pd3dGraphicsRootSignature); 
	}
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, void *pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }
	virtual void FrustumCulling(CCamera* pCamera) {}
	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);
	void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() 
	{
		return(m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() { return(m_pd3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart()); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUCbvDescriptorStartHandle() { return(m_d3dCbvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUCbvDescriptorStartHandle() { return(m_d3dCbvGPUDescriptorStartHandle); }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSrvDescriptorStartHandle() { return(m_d3dSrvCPUDescriptorStartHandle); }
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSrvDescriptorStartHandle() { return(m_d3dSrvGPUDescriptorStartHandle); }
	ID3D12DescriptorHeap* GetDescriptorHeap()
	{
		return m_pd3dDescriptorHeap;
	}

	void SetGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList);
	void SetPipelineState(ID3D12GraphicsCommandList *pd3dCommandList);
	void SetDescriptorHeaps(ID3D12GraphicsCommandList *pd3dCommandList);

protected:
	ID3D12PipelineState				**m_ppd3dPipelineStates = NULL;
	int								m_nPipelineStates = 0;

	ID3D12DescriptorHeap			*m_pd3dDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dCbvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorStartHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dSrvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorStartHandle;

	ID3D12RootSignature				*m_pd3dGraphicsRootSignature = NULL;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CDefferredTexturedShader : public CShader
{
public:
	CDefferredTexturedShader();
	virtual ~CDefferredTexturedShader();
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CDefferredLightingTexturedShader : public CShader
{
public:
	CDefferredLightingTexturedShader();
	virtual ~CDefferredLightingTexturedShader();
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSpecularLightingTexturedShader : public CShader
{
public:
	CSpecularLightingTexturedShader();
	virtual ~CSpecularLightingTexturedShader();
	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CTerrainShader : public CShader
{
public:
	CTerrainShader();
	virtual ~CTerrainShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRendererSpecularMeshShader : public CShader
{
public:
	CRendererSpecularMeshShader();
	virtual ~CRendererSpecularMeshShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets);

private:
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CRendererMeshShader : public CShader
{
public:
	CRendererMeshShader();
	virtual ~CRendererMeshShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets);
	
private:
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CObjectShader : public CShader
{
public:
	CObjectShader();
	virtual ~CObjectShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets);

private:
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBoxShader : public CShader
{
public:
	CSkyBoxShader();
	virtual ~CSkyBoxShader();

	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature);
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;
	float 					m_fFalloff;
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int						m_nType;
	float					m_fRange;
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
};

struct MATERIAL
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular; //(r,g,b,a=power)
	XMFLOAT4				m_xmf4Emissive;
};

struct MATERIALS
{
	MATERIAL				m_pReflections[MAX_MATERIALS];
};

class CTextureToFullScreenShader : public CShader
{
public:
	CTextureToFullScreenShader(ID3D12Resource* pd3dDepthStencilBuffer);
	virtual ~CTextureToFullScreenShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob **ppd3dShaderBlob, bool bIsShadow);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob **ppd3dShaderBlob);
	virtual void CreateShaderResourceViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CTexture *pTexture, UINT nRootParameterStartIndex, bool bAutoIncrement);
	virtual void CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nRenderTargets = 1);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CPlayer* pPlayer, void *pContext = NULL);
	virtual void ReleaseObjects();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	void UpdateShadowShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, SHADOW_INFO* pCameraInfo);
	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void SetMaterial(int nIndex, MATERIAL *pMaterial);
	void BuildLightsAndMaterials();
	virtual void ReleaseShaderVariables();
	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
protected:
	CTexture						*m_pTexture;
private:
	CPlayer						*m_pPlayer = NULL;
	LIGHTS						*m_pLights = NULL;

	ID3D12Resource				*m_pd3dcbLights = NULL;
	LIGHTS						*m_pcbMappedLights = NULL;

	ID3D12Resource				*m_pd3dcbMaterials = NULL;
	MATERIALS					*m_pcbMappedMaterials = NULL;

	ID3D12Resource				*m_pd3dcbShadow = NULL;
	SHADOW_INFO		*m_pcbMappedShadow = NULL;

	MATERIALS					*m_pMaterials = NULL;
	int							m_nMaterials = 0;

	ID3D12Resource* m_pd3dDepthStencilBuffer = NULL;
	D3D12_GPU_DESCRIPTOR_HANDLE m_d3dShadowGPUDescriptorHandle;
};
class CShadowShader
{
public:
	CShadowShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWndClientWidth, UINT nWndClientHeight);
	virtual ~CShadowShader();

	void CreateDsvDescriptorHeaps(ID3D12Device *pd3dDevice);
	void CreateResource(ID3D12Device *pd3dDevice, UINT nWndClientWidth, UINT nWndClientHeight);
	void CreateDepthStencilView(ID3D12Device *pd3dDevice);

	void UpdateTransform();
	void ClearDepthStencilView(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList);
	void SwapResource(ID3D12GraphicsCommandList *pd3dCommandList, bool bType);
	void SetPlayer(CPlayer* pPlayer);
	SHADOW_INFO* GetShadowInfo();
	ID3D12Resource* GetShadowMap()
	{
		return m_pd3dDepthStencilBuffer;
	}
private:
	ID3D12DescriptorHeap * m_pd3dDSVDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dDsvGPUDescriptorStartHandle;

	ID3D12Resource* m_pd3dDepthStencilBuffer = NULL;
	BoundingSphere mSceneBounds;

	SHADOW_INFO m_ShdowInfo;

	CPlayer* m_pPlayer = NULL;

	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT m_d3dScissorRect;

	XMFLOAT3 m_xmf3Lightdir = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	float m_fLightRotationAngle = 0.0f;
};