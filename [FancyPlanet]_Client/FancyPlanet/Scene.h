#pragma once
#include "UIShader.h"
#include "Effect.h"
#include "protocol.h"

#define MESH_NUM 2
#define MESH_NUM2 5

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
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
	virtual void ReleaseObjects();
	void SetLightsAndMaterialsShaderVariables(ID3D12Resource* pLights, ID3D12Resource* pMaterials, ID3D12Resource* pShadowMap);

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
	virtual void ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid, int Type) {};
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables() {};
	virtual void SetVectorProjectile(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3Direction) {
	};
	virtual void SetParticle(UINT nType, int nCharacterType)
	{
	};

	virtual void SetCharacter(UINT nObjects, CAnimationObject** ppSoldier, CAnimationObject** ppDrone)
	{};
	virtual UINT GetCharatersNum()
	{
		return 0;
	};
	virtual CAnimationObject** GetSoldierObjects() { return NULL; };
	virtual CAnimationObject** GetDroneObjects() { return NULL; };
	void SetCharacter(UINT nObjects, CGameObject** ppSoldier, CGameObject** ppDrone)
	{};
	virtual void ChangeAnimation(int newState, int Type) {};
	virtual CHeightMapTerrain * GetTerrain() { return NULL; };
	void FrustumCulling(CCamera *pCamera);
	CPlayer					*m_pPlayer;
protected:
	ID3D12Resource				*m_pd3dcbLights = NULL;
	ID3D12Resource				*m_pd3dcbMaterials = NULL;
	ID3D12Resource				*m_pd3dDepthStencilBuffer = NULL;

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;
	XMFLOAT3 m_xmf3ObjectsPos[OBJECTS_NUMBER];
	int									m_nDivision = 0;
};
 
//지형과 스카이박스를 그리는 씬 (카메라, 플레이어)
class TerrainAndSkyBoxScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
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

class EffectScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
	virtual void ReleaseObjects();

	virtual void SetParticle(UINT nType, int nCharacterType);
	void Update(CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);

	void SetCharacter(UINT nObjects, CAnimationObject** ppSoldier, CAnimationObject** ppDrone)
	{
		m_nObjects = nObjects;

		m_ppSoldier = new CAnimationObject*[nObjects];
		m_ppDrone = new CAnimationObject*[nObjects];

		m_ppSoldier = ppSoldier;
		m_ppDrone = ppDrone;
	};

	EffectScene();
	~EffectScene();
private:
	CMesh * *m_ppMeshes;
	UINT		m_nMeshes;

	UINT m_nObjects = 0;
	CAnimationObject** m_ppSoldier = NULL;
	CAnimationObject** m_ppDrone = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGPUDescriptorHandle;
	SparkEffect *m_pSparkEffect = NULL;
	MeshEffect *m_pMeshEffect = NULL;
	DroneEffect** m_ppDroneEffect = NULL;

	CEffectShader *m_pFlareShader = NULL;
	MeshEffectShader *m_pMeshEffectShader = NULL;
};

//애니메이션이 들어간 오브젝트를 그리는 씬
class CharacterScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
	virtual void ReleaseObjects();
	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables();
	virtual void ChangeAnimation(int newState, int Type);
	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void CopyObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CAnimationObject* pSample, CAnimationObject** ppObjects, UINT nSize, UINT nAnimationFactor);
	virtual void ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid, int Type);
	CAnimationObject* FindAnimationFactorObject(CAnimationObject* pRootObject, int nIndex);

	CAnimationObject** GetSoldierObjects()
	{
		return m_ppSoldierObjects;
	}
	CAnimationObject** GetDroneObjects()
	{
		return m_ppDroneObjects;
	}
	UINT GetCharatersNum()
	{
		return m_nObjects;
	};
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
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
	virtual void ReleaseObjects();
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);

	virtual void ReleaseShaderVariables();
	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	virtual void ReleaseUploadBuffers();
	virtual void SetObjectsVectorFromPacket(XMFLOAT3 pos, int n_ObjectIdx);
	ObjectScene();
	~ObjectScene();

	CGameObject **m_ppSampleObjects = NULL;
private:
	UINT m_nObjects = 0;

	CGameObject** m_ppAsteroid0 = NULL;
	CGameObject** m_ppAsteroid1 = NULL;
	CGameObject** m_ppAsteroid2 = NULL;
	CGameObject** m_ppAsteroid3 = NULL;
	CGameObject** m_ppAsteroid4 = NULL;

	float m_fObjectUpdateTime=0.f;
	bool m_isObjectLoad = false;
};