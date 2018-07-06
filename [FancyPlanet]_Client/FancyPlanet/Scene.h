#pragma once
#include "UIShader.h"
#include "Camera.h"
#include "protocol.h"

#define MESH_NUM 1
#define MESH_NUM2 6

class CMesh;

class CScene
{
public:
	CScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	CScene() {};
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	CAnimationObject* FindBoneTypeObject(CAnimationObject* pRootObject, UINT nBoneTypeMesh);

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

	//Physx SDK Member Variables =========================
	PxPhysics * m_pPxPhysicsSDK;
	PxScene*						m_pPxScene;
	PxControllerManager*			m_pPxControllerManager;
	PxMaterial*						m_pPxMaterial;
	PxCooking*						m_pCooking;
	//=====================================================

	ID3D12RootSignature			*m_pd3dGraphicsRootSignature = NULL;

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
	TerrainAndSkyBoxScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	~TerrainAndSkyBoxScene();

protected:
	//Physx SDK Member Variables =========================
	PxPhysics*						m_pPxPhysicsSDK;
	PxScene*						m_pPxScene;
	PxControllerManager*			m_pPxControllerManager;
	PxMaterial*						m_pPxMaterial;
	PxCooking*						m_pCooking;
	//=====================================================
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
	void CopyObject(CAnimationObject* pSample, CAnimationObject** ppObjects, UINT nSize);
	CAnimationObject* FindMeshRendererObject(CAnimationObject* pRootObject, UINT nRendererMesh);
	virtual void ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid);

	CharacterScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	~CharacterScene();

private:
	ID3D12Resource * m_pd3dcbGameObjects = NULL;
	BONE_TRANSFORMS				*m_pcbMappedGameObjects = NULL;

	ID3D12Resource					*m_pd3dcbGameObjects2 = NULL;
	BONE_TRANSFORMS2			*m_pcbMappedGameObjects2 = NULL;

	ID3D12Resource					*m_pd3dcbGameObjects3 = NULL;
	BONE_TRANSFORMS3				*m_pcbMappedGameObjects3 = NULL;

	CPhysXObject		**m_ppPxObjects = NULL;
	CAnimationObject **m_ppSampleAnimationObjects = NULL;

	CAnimationObject	 **m_ppSoldierObjects = NULL;


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

	ObjectScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	~ObjectScene();

	CPhysXObject **m_ppSampleObjects = NULL;
	DWORD		m_fReloadtime = 0;
private:
	UINT m_nObjects = 0;

	vector<CPhysXObject*> m_vBullet;

	CPhysXObject** m_ppShell = NULL;

	CPhysXObject** m_ppAsteroid0 = NULL;
	CPhysXObject** m_ppAsteroid1 = NULL;
	CPhysXObject** m_ppAsteroid2 = NULL;
	CPhysXObject** m_ppAsteroid3 = NULL;
	CPhysXObject** m_ppAsteroid4 = NULL;

	CPhysXObject **m_ppPxObjects = NULL;
};