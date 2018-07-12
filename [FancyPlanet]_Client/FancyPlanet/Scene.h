#pragma once
#include "UIShader.h"
#include "Camera.h"
#include "protocol.h"

#define MESH_NUM 2
#define MESH_NUM2 6

class CMesh;

class CScene
{
public:
	CScene();
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	CAnimationObject* FindBoneTypeObject(CAnimationObject* pRootObject, UINT nBoneTypeMesh);
	virtual void SetObjectsVectorFromPacket(XMFLOAT3 pos, int n_ObjectIdx) {};
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual pair<XMFLOAT3, XMFLOAT3> GetProjectilePos(UINT nShell);

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice) { return NULL; };
	ID3D12RootSignature *GetGraphicsRootSignature()
	{
		return(m_pd3dGraphicsRootSignature);
	}
	void SetGraphicsRootSignature(ID3D12GraphicsCommandList *pd3dCommandList)
	{
		pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	}
	bool ProcessInput(UCHAR *pKeysBuffer);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera) {};
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL) = 0;
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo) = 0;
	virtual void SkyBoxRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) {}
	virtual void ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid) {};
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables() {};
	virtual void SetVectorProjectile(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction) {
	};
	virtual int SetProjectile(XMFLOAT3& xmf3Direction)
	{
		return 0;
	};
	virtual void ChangeAnimation(int newState) {};
	virtual CHeightMapTerrain * GetTerrain() { return NULL; };
	void FrustumCulling(CCamera *pCamera);
	CPlayer					*m_pPlayer;
protected:

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;
	XMFLOAT3 m_xmf3ObjectsPos[OBJECTS_NUMBER];
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
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables() {};
	virtual CHeightMapTerrain * GetTerrain()
	{
		return m_pTerrain;
	};
	TerrainAndSkyBoxScene();
	~TerrainAndSkyBoxScene();

protected:
private:
	CHeightMapTerrain			*m_pTerrain = NULL;
	CSkyBox						*m_pSkyBox = NULL;
};

//마지막순서로 그려줘야할 씬
class UIScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);

	UIScene();
	~UIScene();
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
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables();
	virtual void ChangeAnimation(int newState);
	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void CopyObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CAnimationObject* pSample, CAnimationObject** ppObjects, UINT nSize, UINT nAnimationFactor);
	virtual void ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid);
	CAnimationObject* FindAnimationFactorObject(CAnimationObject* pRootObject, int nIndex);

	CharacterScene();
	~CharacterScene();

private:
	CAnimationObject **m_ppSampleAnimationObjects = NULL;

	CAnimationObject	 **m_ppSoldierObjects = NULL;
	CAnimationObject	 **m_ppDroneObjects = NULL;

	UINT*						m_pnAnimationFactor = NULL;

	int							m_nObjects = 0;
	AnimationController **m_ppAnimationController = NULL;
};

class ObjectScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);

	virtual pair<XMFLOAT3, XMFLOAT3> GetProjectilePos(UINT nShell);
	virtual void SetVectorProjectile(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction);
	
	virtual int SetProjectile(XMFLOAT3& xmf3Direction);
	virtual void ReleaseShaderVariables();
	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	virtual void ReleaseUploadBuffers();
	virtual void SetObjectsVectorFromPacket(XMFLOAT3 pos, int n_ObjectIdx);
	ObjectScene();
	~ObjectScene();

	CGameObject **m_ppSampleObjects = NULL;
	DWORD		m_fReloadtime = 0;
private:
	UINT m_nObjects = 0;

	vector<CGameObject*> m_vBullet;

	CGameObject** m_ppShell = NULL;

	CGameObject** m_ppAsteroid0 = NULL;
	CGameObject** m_ppAsteroid1 = NULL;
	CGameObject** m_ppAsteroid2 = NULL;
	CGameObject** m_ppAsteroid3 = NULL;
	CGameObject** m_ppAsteroid4 = NULL;

	CGameObject** m_ppPxObjects = NULL;

	bool m_isObjectLoad = false;
};