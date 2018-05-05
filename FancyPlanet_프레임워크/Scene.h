#pragma once
#include "UIShader.h"
#include "Camera.h"
#include "protocol.h"

#define MESH_NUM 1
class CMesh;

class CScene
{
public:
	CScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
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
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL) = 0;
	virtual void SkyBoxRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) {}
	virtual void ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray) {};
	virtual void ReleaseUploadBuffers();
	virtual void SetProjectile(XMFLOAT3& xmf3Direction)
	{
	};
	virtual void ChangeAnimation(int newState) = 0;
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
	virtual void ReleaseUploadBuffers();
	virtual void ChangeAnimation(int newState) {}
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
class PhysXScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ReleaseUploadBuffers();
	virtual void ChangeAnimation(int newState) {}
	virtual void SetProjectile(XMFLOAT3& xmf3Direction);

	PxPhysics*  GetPhysicsSDK(void) { return m_pPxPhysicsSDK; }
	PxScene*	GetPhysicsScene(void) { return m_pPxScene; }
	PxControllerManager* GetPxControllerManager(void) { return m_pPxControllerManager; }
	PxMaterial* GetPxMaterial(void) { return m_pPxMaterial; }
	PxCooking*  GetCooking(void) { return m_pCooking; }

	PhysXScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	~PhysXScene();
protected:
	//씬은 게임 객체들의 집합이다. 게임 객체는 셰이더를 포함한다.
	CPhysXObject * *m_ppObjects = NULL;
	CAnimationObject **m_ppAniObjects = NULL;
	int m_nObjects = 0;
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
	virtual void ReleaseUploadBuffers();
	virtual void ChangeAnimation(int newState);
	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void CopyObject(CAnimationObject* pSample, CAnimationObject** ppObjects, UINT nSize);
	CAnimationObject* FindMeshRendererObject(CAnimationObject* pRootObject, UINT nRendererMesh);
	virtual void ModelsSetPosition(const array <PLAYER_INFO, MAX_USER>& PlayerArray);
	virtual void SetProjectile(XMFLOAT3& xmf3Direction);

	CharacterScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	~CharacterScene();

private:
	ID3D12Resource * m_pd3dcbGameObjects = NULL;
	BONE_TRANSFORMS				*m_pcbMappedGameObjects = NULL;

	ID3D12Resource					*m_pd3dcbGameObjects2 = NULL;
	BONE_TRANSFORMS2			*m_pcbMappedGameObjects2 = NULL;

	ID3D12Resource					*m_pd3dcbGameObjects3 = NULL;
	BONE_TRANSFORMS3				*m_pcbMappedGameObjects3 = NULL;

	CAnimationObject **m_ppSampleObjects = NULL;
	CAnimationObject	 **m_ppSoldierObjects = NULL;
	int							m_nObjects = 0;
	AnimationController **m_ppAnimationController = NULL;
};

class ObjectScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseObjects();

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ReleaseUploadBuffers();
	virtual void ChangeAnimation(int newState) {}

	ObjectScene(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	~ObjectScene();
private:
};