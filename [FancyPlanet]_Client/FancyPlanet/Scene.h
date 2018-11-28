#pragma once
#include "UIShader.h"
#include "Effect.h"
#include "protocol.h"

#define MESH_NUM 3
#define MESH_NUM2 5

#define STATIC_OBJECT_NUM 10
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
	virtual void StartGameSetting(array <PLAYER_INFO, MAX_USER>& PlayerArray, int myId) {};
	virtual void ResetCharacter(UINT nIndex, UINT nCharacterType)
	{}
	virtual void SetSkyBoxPosition(const XMFLOAT3& xmf3Pos)
	{}
	virtual pair<XMFLOAT3, XMFLOAT3> GetProjectilePos(UINT nShell);
	virtual void Update(CCamera* pCamera) {}
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
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera, int scene) {};
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL) = 0;
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo) = 0;
	virtual void SkyBoxRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera) {}
	virtual void ModelsSetPosition(array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid, int Type) {};
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables() {};
	virtual void SetObjectsVectorFromPacket(XMFLOAT3& pos, XMFLOAT4& quat, int n_ObjectIdx) {
	};
	virtual void SetParticle(UINT nType, UINT nCharacterType, UINT nIndex, XMFLOAT3* xmf3Target)
	{
	};
	virtual void UpdateEffect(const unsigned long& nCurrentFrameRate) {};
	virtual void SelectCharacter(int nCharaterType) {};
	virtual void SetCharacter(UINT nObjects, CAnimationObject** ppSoldier, CAnimationObject** ppDrone, CAnimationObject** ppAliens)
	{};
	virtual void BaseSetting(UINT Num, UINT nColor) {}
	virtual UINT GetCharatersNum()
	{
		return 0;
	};
	virtual CAnimationObject** GetSoldierObjects() { return NULL; };
	virtual CAnimationObject** GetDroneObjects() { return NULL; };
	virtual CAnimationObject** GetAlienObjects() { return NULL; };

	virtual void ChangeAnimation(int newState, array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid) {};
	virtual void ChangeAnimationInLobby(int newState, UINT nCharacterType) {};

	virtual CHeightMapTerrain * GetTerrain() { return NULL; };
	void FrustumCulling(CCamera *pCamera);
	CPlayer               *m_pPlayer;
protected:
	ID3D12Resource * m_pd3dcbLights = NULL;
	ID3D12Resource            *m_pd3dcbMaterials = NULL;
	ID3D12Resource            *m_pd3dDepthStencilBuffer = NULL;

	ID3D12RootSignature         *m_pd3dGraphicsRootSignature = NULL;
	XMFLOAT3 m_xmf3ObjectsPos[OBJECTS_NUMBER];
	XMFLOAT4 m_xmf4ObjectsQuaternion[OBJECTS_NUMBER];

	int                           m_nDivision = 0;
};

class TerrainAndSkyBoxScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
	virtual void ReleaseObjects();

	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera, int scene);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables() {};
	virtual CHeightMapTerrain* GetTerrain()
	{
		return m_pTerrain;
	};
	virtual void ObjectsSetting(ID3D12GraphicsCommandList *pd3dCommandList);
	TerrainAndSkyBoxScene();
	~TerrainAndSkyBoxScene();
	void BaseSetting(UINT Num, UINT nColor);

protected:
private:
	CHeightMapTerrain * m_pTerrain = NULL;
	StaticObjectShader *m_pStaticObjectShader = NULL;
	StaticObject **m_ppStaticObjects = NULL;
};

class EffectScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
	virtual void ReleaseObjects();

	virtual void SetParticle(UINT nType, UINT nCharacterType, UINT nIndex, XMFLOAT3* pxmf3Target);
	virtual void Update(CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void UpdateEffect(const unsigned long& nCurrentFrameRate);
	void SetCharacter(UINT nObjects, CAnimationObject** ppSoldier, CAnimationObject** ppDrone, CAnimationObject** ppAliens)
	{
		m_nObjects = nObjects;

		m_ppSoldier = new CAnimationObject*[nObjects];
		m_ppDrone = new CAnimationObject*[nObjects];
		m_ppAliens = new CAnimationObject*[nObjects];

		m_ppSoldier = ppSoldier;
		m_ppDrone = ppDrone;
		m_ppAliens = ppAliens;
	};

	EffectScene();
	~EffectScene();
private:
	CMesh * *m_ppMeshes;
	UINT      m_nMeshes;

	UINT m_nObjects = 0;
	CAnimationObject** m_ppSoldier = NULL;
	CAnimationObject** m_ppDrone = NULL;
	CAnimationObject** m_ppAliens = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE      m_d3dSrvGPUDescriptorHandle;
	SparkEffect** m_ppSparkEffect = NULL;
	MeshEffect** m_ppMeshEffect = NULL;
	TeleportEffect** m_ppTeleportEffect = NULL;
	DroneEffect** m_ppDroneEffect = NULL;
	JumpEffect** m_ppJumpEffect = NULL;
	SphereEffect** m_ppSphereEffect = NULL;
	BloodEffect** m_ppBloodEffect = NULL;
	DroneSkillEffect** m_ppDroneSkillEffect = NULL;

	CEffectShader* m_pFlareShader = NULL;
	MeshEffectShader* m_pMeshEffectShader = NULL;
};

//애니메이션이 들어간 오브젝트를 그리는 씬
class CharacterScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
	virtual void ReleaseObjects();
	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera, int scene);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	virtual void ReleaseUploadBuffers();
	virtual void ReleaseShaderVariables();
	virtual void ChangeAnimationInLobby(int newState, UINT nCharacterType);
	virtual void ChangeAnimation(int newState, array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid);
	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void CopyObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CAnimationObject* pSample, CAnimationObject** ppObjects, UINT nSize, UINT nAnimationFactor);
	virtual void ModelsSetPosition(array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid, int Type);
	CAnimationObject* FindAnimationFactorObject(CAnimationObject* pRootObject, int nIndex);
	virtual void SelectCharacter(int nCharaterType);
	void CalcCharacterIndex(array <PLAYER_INFO, MAX_USER>& PlayerArray);
	virtual void ResetCharacter(UINT nIndex, UINT nCharacterType);
	void SetPlayer(array <PLAYER_INFO, MAX_USER>& PlayerArray, UINT myid);
	virtual void StartGameSetting(array <PLAYER_INFO, MAX_USER>& PlayerArray, int myId);

	virtual CAnimationObject** GetSoldierObjects()
	{
		return m_ppSoldierObjects;
	}
	virtual CAnimationObject** GetDroneObjects()
	{
		return m_ppDroneObjects;
	}
	virtual CAnimationObject** GetAlienObjects()
	{
		return m_ppAliens;
	}
	UINT GetCharatersNum()
	{
		return m_nObjects;
	};
	CharacterScene();
	~CharacterScene();

private:
	CAnimationObject * *m_ppSampleAnimationObjects = NULL;

	CAnimationObject    **m_ppSoldierObjects = NULL;
	CAnimationObject    **m_ppDroneObjects = NULL;
	CAnimationObject    **m_ppAliens = NULL;

	UINT*                  m_pnAnimationFactor = NULL;
	int m_nCharacterIndex = 0;
	int                     m_nObjects = 0;
	AnimationController **m_ppAnimationController = NULL;

};
class ObjectScene : public CScene
{
public:
	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType);
	virtual void ReleaseObjects();
	virtual void AnimateObjects(float fTimeElapsed, CCamera *pCamera, int scene);

	virtual void ReleaseShaderVariables();
	virtual ID3D12RootSignature *CreateGraphicsRootSignature(ID3D12Device *pd3dDevice);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo);
	virtual void ReleaseUploadBuffers();
	virtual void SetObjectsVectorFromPacket(XMFLOAT3& pos, XMFLOAT4& quat, int n_ObjectIdx);
	virtual void SetSkyBoxPosition(const XMFLOAT3& xmf3Pos);
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

	CSkyBox						*m_pSkyBox = NULL;

	float m_fObjectUpdateTime = 0.f;
	bool m_isObjectLoad = false;
};