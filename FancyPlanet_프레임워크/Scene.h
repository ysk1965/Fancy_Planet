#pragma once
#include "Shader.h"
#include "Camera.h"

class CScene
{
public:
	CScene();
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice) = 0;
	ID3D12RootSignature *GetGraphicsRootSignature() 
	{
		return(m_pd3dGraphicsRootSignature);
	}
	void SetGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList)
	{
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature); 
	}
	bool ProcessInput(UCHAR *pKeysBuffer);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera) = 0;
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, const unsigned long& nCurrentFrame, CCamera *pCamera = NULL) = 0;
	virtual void SkyBoxRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) {}
	virtual void ReleaseUploadBuffers();
	void FrustumCulling(CCamera *pCamera);

	CPlayer					*m_pPlayer; 
protected:

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;
	int							m_nShaders = 0;
	CShader					**m_ppShaders = NULL;
	
	int									m_nDivision = 0;
};
 
//지형과 스카이박스를 그리는 씬 (카메라, 플레이어)
class TerrainAndSkyBoxScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, const unsigned long& nCurrentFrame, CCamera *pCamera = NULL);
	virtual void ReleaseUploadBuffers();

	TerrainAndSkyBoxScene();
	~TerrainAndSkyBoxScene();
private:
	CHeightMapTerrain			*m_pTerrain = NULL;
	CSkyBox						*m_pSkyBox = NULL;
};

//마지막순서로 그려줘야할 씬
class EndObjectScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, const unsigned long& nCurrentFrame, CCamera *pCamera = NULL);
	virtual void ReleaseUploadBuffers();

	EndObjectScene();
	~EndObjectScene();
private:
};

//애니메이션이 들어간 오브젝트를 그리는 씬
class CharacterScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, const unsigned long& nCurrentFrame, CCamera *pCamera = NULL);
	virtual void ReleaseUploadBuffers();

	CharacterScene();
	~CharacterScene();
private:
	CGameObject					**m_ppObjects = NULL;
	int							m_nObjects = 0;
};

class ObjectScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, const unsigned long& nCurrentFrame, CCamera *pCamera = NULL);
	virtual void ReleaseUploadBuffers();

	ObjectScene();
	~ObjectScene();
private:
};