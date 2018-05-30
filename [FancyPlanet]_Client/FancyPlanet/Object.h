#pragma once
#include "Mesh.h"
#include "Camera.h"
#include <chrono>

enum 
{
	NOT_ALL = 0, // �߰��� �ִϸ��̼� ���� (����)
	ALL = 1, // �ִϸ��̼��� ������ �ٸ� ��ϸ��̼��� ���� (����)
	MODIFIABLE = 2, // �ִϸ��̼� �ð��� ����ɼ� ����
	NOT_CYCLE = 3, // �ѹ��� ����ǰ� ������ ������� ����
	CAN_BACK_PLAY = 4, // �ڷε� ����� �ʿ��� �ִϸ��̼�(����)
	CYCLE_NEED_TIME = 5, // ����Ŭ�� �ʿ������� ó���� ���� ���� �ʴ� �ִϸ��̼� 
	CONTINUOUS_PLAYBACK = 6, // ����� ������ �ʰ�, ���� �ε��� �ִϸ��̼��� �ڵ� ����
	PLAY_NOW = 7
};

enum
{
	SKINNEDMESH = 0,
	RENDERMESH = 1,
	LAYER = 2,
	END = 3,
	GET_TYPE = 4,
	SKIP = 5
};


#define DIR_FORWARD					0x01
#define DIR_BACKWARD				0x02
#define DIR_LEFT					0x04
#define DIR_RIGHT					0x08
#define DIR_UP						0x10
#define DIR_DOWN					0x20

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

#define BONE_TRANSFORM_NUM 31
#define BONE_TRANSFORM_NUM2 32
#define BONE_TRANSFORM_NUM3 24
#define RENDERER_MESH_WORLD_TRANSFORM 8

#define CHANGE_TIME 0.2f

#define CHANG_INDEX -500

class CShader;
class CAnimationObject;
class AnimationFactors; 
class CPhysXObject;

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World; 
};
struct SRVROOTARGUMENTINFO
{
	UINT							m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

struct BONE_TRANSFORMS
{
	XMFLOAT4X4	m_xmf4x4World;
	XMFLOAT4X4 m_xmf4x4BoneTransform[BONE_TRANSFORM_NUM];
};
struct BONE_TRANSFORMS2
{
	XMFLOAT4X4 m_xmf4x4BoneTransform[BONE_TRANSFORM_NUM2];
};
struct BONE_TRANSFORMS3
{
	XMFLOAT4X4 m_xmf4x4BoneTransform[BONE_TRANSFORM_NUM3];
	XMFLOAT4X4	m_xmf4x4RedererMeshWorld[RENDERER_MESH_WORLD_TRANSFORM];
};
struct FRAME
{
	XMFLOAT3 Translation;
	XMFLOAT4 RotationQuat; 
};
struct ANIMATION
{
	float fTime;  // �ִϸ��̼� �ð�
	UINT nFrame; // ��ü ������ ��
	UINT nType;
	FRAME* pFrame; // [Frame * BoneIndex]
};

typedef struct SRT
{
	XMFLOAT3 S;
	XMFLOAT4 R;
	XMFLOAT3 T;
}SRT;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class AnimationController
{
private:
	UINT m_nAnimation = 0;

	CAnimationObject* m_pRootObject = NULL;
	CAnimationObject **m_ppBoneObject = NULL;

public:
	UINT m_nBone = 0;

	XMFLOAT4X4								*m_pBindPoses;

	UINT GetAnimationCount()
	{
		return m_nAnimation;
	}

	AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation);
	~AnimationController();
	void GetCurrentFrame(bool bStop);
	SRT Interpolate(int iBoneNum, float fRendererScale);
	SRT Interpolate(int iBoneNum, float fTime, float fRendererScale);
	void SetToParentTransforms();
	//�ش��ϴ� ���ε����� ������Ʈ�� �����Ѵ�.
	void SetToRootTransforms();
	void AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList);
	void ChangeAnimation(int iNewState);
	void SetObject(CAnimationObject* pObject);
	void SetBindPoses(XMFLOAT4X4* pBindPoses);

	ANIMATION *m_pAnimation;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CTexture
{
public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;
	UINT							m_nTextureType = RESOURCE_TEXTURE2D;
	int								m_nTextures = 0;
	ID3D12Resource					**m_ppd3dTextures = NULL;
	ID3D12Resource					**m_ppd3dTextureUploadBuffers;
	SRVROOTARGUMENTINFO				*m_pRootArgumentInfos = NULL;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		*m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	ID3D12Resource *ComputeCreateTexture(ID3D12Device *pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex);
	ID3D12Resource *CreateTexture(ID3D12Device *pd3dDevice, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex);
	SRVROOTARGUMENTINFO& GetArgumentInfos(UINT nIndex)
	{
		return m_pRootArgumentInfos[nIndex];
	}

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex);
	void ReleaseShaderVariables();

	void LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex);

	int GetTextureCount() { return(m_nTextures); }
	ID3D12Resource *GetTexture(int nIndex) 
	{
		return(m_ppd3dTextures[nIndex]);
	}
	UINT GetTextureType() 
	{
		return(m_nTextureType);
	}

	void ReleaseUploadBuffers();	
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4						m_xmf4Albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	UINT							m_nReflection = 0;
	CTexture						*m_pTexture = NULL;
	CShader							*m_pShader = NULL;

	void SetAlbedo(XMFLOAT4 xmf4Albedo) { m_xmf4Albedo = xmf4Albedo; }
	void SetReflection(UINT nReflection) 
	{
		m_nReflection = nReflection; 
	}
	void SetTexture(CTexture *pTexture);
	void SetShader(CShader *pShader);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();
};

class CGameObject
{
public:
	CGameObject(int nMeshes, UINT nObjects);
	CGameObject(int nMeshes, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
		, ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName, UINT nObjects);
	virtual ~CGameObject();

private:
	int								m_nReferences = 0;
	UINT							m_nObjects = 0;
public:
	void AddRef() 
	{
		m_nReferences++; 
	}
	void Release() 
	{
		if (--m_nReferences <= 0) 
			delete this; 
	}

public:
	XMFLOAT3						m_Direction;
	bool							m_bActive = true;
	bool							m_bShooted = false;
	DWORD							m_dAlivedtime = 0;

	XMFLOAT4X4						m_xmf4x4World;

	CMesh							**m_ppMeshes;
	int								m_nMeshes;

	CMaterial						*m_pMaterial = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

protected:
	ID3D12Resource					*m_pd3dcbGameObjects = NULL; // ���� ������Ʈ�� �̰��� ������ �ִ�.
	CB_GAMEOBJECT_INFO				*m_pcbMappedGameObjects = NULL;

	bool m_bRendererMesh = false;

	UINT							m_nDrawType = -1;
	UINT							m_nBoneType = 0;
public:
	void SetMesh(int nIndex, CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetMaterial(CMaterial *pMaterial);
	void ResizeMeshes(int nMeshes);

	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(m_d3dCbvGPUDescriptorHandle); }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, UINT RootParameterIndex, CPhysXObject** ppObjects);
	virtual void UpdateAnimationVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPrepareRender();
	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex);
	
	virtual void Animate(float fTimeElapsed); 
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera, UINT nInstances, CPhysXObject** ppObjects);
	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseUploadBuffers();
	virtual void ChangeAnimation(int newState) {};
	virtual void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL){};

	virtual void LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nType, UINT nSub);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void Move(float fSpeed)
	{
		SetPosition(m_xmf4x4World._41 + m_Direction.x * fSpeed, m_xmf4x4World._42 + m_Direction.y * fSpeed, m_xmf4x4World._43 + m_Direction.z * fSpeed);
	}

	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3& xmf3Position);
	virtual void SetScale(float x, float y, float z);
	virtual void SetRotation(XMFLOAT4& pxmf4Quaternion);
	
	virtual void MoveStrafe(float fDistance = 1.0f);


	
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	virtual void Rotate(XMFLOAT4 *pxmf4Quaternion);
	
};

class CAnimationObject : public CGameObject
{
private:
	CPlayer * m_pPlayer = NULL;
	UINT m_nScale = 1;
	XMFLOAT4X4* m_pBindPoses = NULL;
	UINT m_nRendererMesh = 0;
	float m_RndererScale = 1;
public:
	CPlayer * GetPlayer()
	{
		return m_pPlayer;
	}
	void SetRendererMeshNum(UINT index)
	{
		m_nRendererMesh = index;
	}
	int GetRendererMeshNum()
	{
		return m_nRendererMesh;
	}
	int GetBoneType()
	{
		return m_nBoneType;
	}
	void SetRendererMesh(bool b)
	{
		m_bRendererMesh = b;
	}
	bool IsRendererMesh()
	{
		return m_bRendererMesh;
	}
	UINT GetPlayerScale()
	{
		return m_nScale;
	}
	void SetPlayerScale(UINT nScale)
	{
		m_nScale = nScale;
	}
	void SetPlayer(CPlayer* pPlayer)
	{
		m_pPlayer = pPlayer;
	}
	void SetRndererScale(float fScale)
	{
		m_RndererScale = fScale;
	}
	float GetRndererScale()
	{
		return m_RndererScale;
	}

	CAnimationObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature,
		UINT nMeshes, TCHAR *pstrFileName, AnimationController* pAnimationController);
	CAnimationObject(int nMeshes) : CGameObject(nMeshes, 0)
	{
		m_xmf4x4ToRootTransform = Matrix4x4::Identity();
		m_xmf4x4ToParentTransform = Matrix4x4::Identity();
	};
	virtual ~CAnimationObject();
	void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances);
	void LoadAnimation(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ifstream& InFile, AnimationController* pAnimationController);
	virtual void LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nType, UINT nSub, CMesh* pCMesh);
	void LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature
		, TCHAR *pstrFileName, AnimationController* pAnimationController);
	virtual void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL);
	CAnimationObject* GetRootObject();
	XMFLOAT4X4* GetBindPoses()
	{
		return m_pBindPoses;
	};
	TCHAR* CharToTCHAR(char * asc);
	void SetChild(CAnimationObject *pChild);
	virtual void ChangeAnimation(int newState);
	
	virtual void SetPosition(float x, float y, float z);
	virtual void SetPosition(XMFLOAT3& xmf3Position);
	virtual void SetScale(float x, float y, float z);
	virtual void SetRotation(XMFLOAT4& pxmf4Quaternion);
	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	virtual void Rotate(XMFLOAT4 *pxmf4Quaternion);

	CAnimationObject 					*m_pParent = NULL;
	CAnimationObject 					*m_pChild = NULL;
	CAnimationObject 					*m_pSibling = NULL;
	// �θ���ǥ��� ��ȯ�ϴ� ���
	XMFLOAT4X4						m_xmf4x4ToRootTransform;
	// ���� ��忡�� ������ǥ��� ��ȯ�ϴ� ���
	XMFLOAT4X4						m_xmf4x4ToParentTransform;

	BONE_TRANSFORMS			*m_BoneTransforms;
	BONE_TRANSFORMS2			*m_BoneTransforms2;
	BONE_TRANSFORMS2			*m_BoneTransforms3;

	TCHAR							m_strFrameName[256] = { '\0' };
	bool							m_bRoot = false;
	int								m_iBoneIndex = -1;
	// 
	AnimationController				*m_pAnimationController = NULL;
	AnimationFactors				*m_pAnimationFactors = NULL;

	CAnimationObject& operator=(CAnimationObject& other)
	{
		if (this == &other)
			return *this;

		memcpy(m_strFrameName, other.m_strFrameName, sizeof(TCHAR) * 256);
		m_bRoot = other.m_bRoot;
		m_iBoneIndex = other.m_iBoneIndex;
		SetRendererMesh(other.IsRendererMesh());
		SetRendererMeshNum(other.GetRendererMeshNum());
		SetRndererScale(other.GetRndererScale());
		if (other.m_pAnimationController)
			m_pAnimationController = other.m_pAnimationController;

		return *this;
	}
};

class AnimationFactors
{
public:
	AnimationFactors(UINT nBindpos)
	{
		m_nBindpos = nBindpos;
		m_chronoStart = chrono::system_clock::now();
		m_ppBoneObject = new CAnimationObject*[m_nBindpos];
		m_iState = 0;
		m_iNewState = 0;
		m_iSaveState = 0;
	}
	CAnimationObject* GetFindObject(CAnimationObject* pFindObject, UINT nBoneIndex)
	{
		CAnimationObject* pResult = NULL;
		if (pFindObject->m_iBoneIndex == nBoneIndex)
			return pFindObject;

		if (pFindObject->m_pSibling)
			pResult = GetFindObject(pFindObject->m_pSibling, nBoneIndex);
		if (pFindObject->m_pChild && pResult == NULL)
			pResult = GetFindObject(pFindObject->m_pChild, nBoneIndex);

		return pResult;
	}
	void SetBoneObject(CAnimationObject* pRoot)
	{
		for (int i = 0; i < m_nBindpos; i++) 
		{
			m_ppBoneObject[i] = GetFindObject(pRoot, i);
		}
	}
	std::chrono::system_clock::time_point m_chronoStart;
	std::chrono::milliseconds ms;

	CAnimationObject **m_ppBoneObject = NULL;
	int m_iState = 0;
	int m_iNewState = 0;
	int m_iSaveState = 0;

	float m_fCurrentFrame = 0;
	float m_fSaveLastFrame = 0;

	UINT m_nBindpos = 0;
};
class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color,
		PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage				*m_pHeightMapImage;

	int							m_nWidth;
	int							m_nLength;

	XMFLOAT3					m_xmf3Scale;

public:
	float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad) * m_xmf3Scale.y); } //World
	XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }

};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBox : public CGameObject
{
public:
	CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);
	virtual ~CSkyBox();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CPhysXObject : public CGameObject
{
protected:
	PxRigidDynamic * m_pPxActor; // ���̳��� ��ü
	CMesh *m_pMesh = NULL;
	CShader *m_pShader = NULL;
public:
	CPhysXObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nObjects);
	CPhysXObject(int nMeshes, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName, UINT nObjects);
	CPhysXObject(int nMeshes);

	virtual ~CPhysXObject();

	virtual void SetPosition(XMFLOAT3& xmf3Position);
	void UpdateDirectPos(const XMFLOAT3& xmf3Position, const XMFLOAT3& DirShooting);
	virtual void SetRotation(XMFLOAT4& pxmf4Quaternion);

	void shooting(float fpower);
	void GetPhysXPos();
	void AddForceAtLocalPos(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup);
	void AddForceAtPosInternal(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup);


	// Px
	virtual void	BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial, XMFLOAT3 vScale, PxCooking* pCooking, const char* name);
	void			PhysXUpdate(const float& fTimeDelta);
	virtual int		Update(const float& fTimeDelta);
};