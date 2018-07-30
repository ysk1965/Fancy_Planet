#pragma once
#include "Effect.h"
#include <chrono>
#include <stack>
enum
{
	NOT_ALL = 0, // 중간에 애니메이션 가능 (구현)
	ALL = 1, // 애니메이션이 끝나야 다른 얘니메이션이 가능 (구현)
	MODIFIABLE = 2, // 애니메이션 시간이 변경될수 있음
	NOT_CYCLE = 3, // 한번만 재생되고 마지막 모습에서 정지
	CAN_BACK_PLAY = 4, // 뒤로도 재생이 필요한 애니메이션(구현)
	CYCLE_NEED_TIME = 5, // 싸이클이 필요하지만 처음과 끝이 같지 않는 애니메이션 
	CONTINUOUS_PLAYBACK = 6, // 재생이 끊기지 않고, 다음 인덱스 애니메이션이 자동 실행
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
class AnimationResourceFactors;
class AnimationTimeFactor;
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
	float fTime;  // 애니메이션 시간
	UINT nFrame; // 전체 프레임 수
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
template <typename T>
class Animation_Stack
{
public:
	Animation_Stack(UINT BoneNum);
	~Animation_Stack();
	void push(T* Tvalue);
	T* top();
	bool empty();
private:
	T * *m_ppTarr;
	UINT m_nLastIndex;
	UINT m_nCapacity;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class AnimationController
{
private:
	UINT m_nAnimation = 0;

	CAnimationObject* m_pRootObject = NULL;
	CAnimationObject* m_pFactorObject = NULL;
	CAnimationObject **m_ppBoneObject = NULL;

	Animation_Stack<CAnimationObject> *m_Animation_Stack;
public:
	UINT m_nBone = 0;

	XMFLOAT4X4								*m_pBindPoses;

	UINT GetAnimationCount()
	{
		return m_nAnimation;
	}

	AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation, UINT nBone);
	~AnimationController();
	void GetCurrentFrame(bool bStop);
	void Interpolate(int iBoneNum, float fRendererScale, SRT& result);
	void Interpolate(int iBoneNum, float fTime, float fRendererScale, SRT& result);
	void SetToParentTransforms();
	//해당하는 본인덱스의 오브젝트를 리턴한다.
	void SetToRootTransforms();
	void AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList, bool bOnce);
	void ChangeAnimation(int iNewState);
	void SetObject(CAnimationObject* pRootObject, CAnimationObject* pAnimationFactorObject);
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
		, ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName, UINT nObjects, ID3D12Resource* pShadowMap);
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
	ID3D12Resource * m_pd3dcbGameObjects = NULL; // 샘플 오브젝트만 이값을 가지고 있다.
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
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, UINT RootParameterIndex, CGameObject** ppObjects);
	virtual void UpdateAnimationVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPrepareRender();
	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex);

	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera, UINT nInstances, CGameObject** ppObjects);
	void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, UINT nInstances, CGameObject** ppObjects);
	void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex);
	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseUploadBuffers();
	virtual void ChangeAnimation(int newState) {};
	virtual void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL) {};

	virtual void LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nType, UINT nSub, UINT nObject, ID3D12Resource* pShadowMap);

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
	UINT m_nRendererMesh = 0; // 몇개의 랜더러매쉬를 가지는지
	float m_RndererScale = 1; // 랜더러 오브젝트의 스케일
	int m_nSkinnedMeshRenderer = -1; // 몇번째 스킨랜더러 매쉬인지
public:
	int GetSkinnedMeshRendererNum()
	{
		return m_nSkinnedMeshRenderer;
	}
	void SetSkinnedMeshRendererNum(int nSkinnedMeshRenderer)
	{
		m_nSkinnedMeshRenderer = nSkinnedMeshRenderer;
	}
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
		UINT nMeshes, TCHAR *pstrFileName, AnimationController* pAnimationController, UINT nObjects, UINT& nAnimationFactor, ID3D12Resource* pShadowMap);
	CAnimationObject(int nMeshes) : CGameObject(nMeshes, 0)
	{
		m_xmf4x4ToRootTransform = Matrix4x4::Identity();
		m_xmf4x4ToParentTransform = Matrix4x4::Identity();
	};
	virtual ~CAnimationObject();
	void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances, CAnimationObject** ppObject, UINT nIndex, CPlayer* pPlayer);
	virtual void ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances, CAnimationObject** ppObject, UINT nIndex);
	void LoadAnimation(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ifstream& InFile, AnimationController* pAnimationController);
	virtual int LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nType, UINT nSub, CMesh* pCMesh, UINT nObjects, UINT nAnimationFactor, ID3D12Resource* pShadowMap);
	void LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature
		, TCHAR *pstrFileName, AnimationController* pAnimationController, UINT nObjects, UINT& nAnimationFactor, ID3D12Resource* pShadowMap);
	virtual void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL);
	CAnimationObject* FindAnimationFactorObject(CAnimationObject* pRootObject, int nIndex);
	CAnimationObject* GetRootObject();
	XMFLOAT4X4* GetBindPoses()
	{
		return m_pBindPoses;
	};
	void SetBindPoses(XMFLOAT4X4* pBindPoses)
	{
		m_pBindPoses = pBindPoses;
	}
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

	CAnimationObject					**m_ppAnimationFactorObjects;
	CAnimationObject 					*m_pParent = NULL;
	CAnimationObject 					*m_pChild = NULL;
	CAnimationObject 					*m_pSibling = NULL;
	// 부모좌표계로 변환하는 행렬
	XMFLOAT4X4						m_xmf4x4ToRootTransform;
	// 현재 노드에서 월드좌표계로 변환하는 행렬
	XMFLOAT4X4						m_xmf4x4ToParentTransform;

	TCHAR							m_strFrameName[256] = { '\0' };
	bool							m_bRoot = false;
	int								m_iBoneIndex = -1;
	// 
	AnimationController				*m_pAnimationController = NULL;
	AnimationResourceFactors				*m_pAnimationFactors = NULL;
	AnimationTimeFactor						*m_pAnimationTime = NULL;
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
		SetSkinnedMeshRendererNum(other.GetSkinnedMeshRendererNum());
		if (other.GetBindPoses() != NULL)
			SetBindPoses(other.GetBindPoses());
		if (other.m_pAnimationController)
			m_pAnimationController = other.m_pAnimationController;

		return *this;
	}
};
class AnimationTimeFactor
{
public:
	AnimationTimeFactor()
	{
		m_iNewState = 0;
		m_iSaveState = 0;

		m_fCurrentFrame = 0;
		m_fSaveLastFrame = 0;
		m_iState = 0;

		m_chronoStart = chrono::system_clock::now();
	}

	int m_iNewState;
	int m_iSaveState;

	float m_fCurrentFrame;
	float m_fSaveLastFrame;
	int m_iState;

	std::chrono::system_clock::time_point m_chronoStart;
	std::chrono::milliseconds ms;
};
class AnimationResourceFactors
{
public:
	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nObjects)
	{
		UINT ncbElementBytes = ((sizeof(BONE_TRANSFORMS) + 255) & ~255); //256의 배수
		m_ppResource[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * nObjects
			, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

		m_ppResource[0]->Map(0, NULL, (void **)&m_pcbMappedGameObjects);

		if (nResource > 1)
		{
			UINT ncbElementBytes = ((sizeof(BONE_TRANSFORMS2) + 255) & ~255); //256의 배수
			m_ppResource[1] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * nObjects
				, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

			m_ppResource[1]->Map(0, NULL, (void **)&m_pcbMappedGameObjects2);
		}
		if (nResource > 2)
		{
			UINT ncbElementBytes = ((sizeof(BONE_TRANSFORMS3) + 255) & ~255); //256의 배수
			m_ppResource[2] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * nObjects
				, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

			m_ppResource[2]->Map(0, NULL, (void **)&m_pcbMappedGameObjects3);
		}
	}

	CAnimationObject* FindMeshRendererObject(CAnimationObject* pRootObject, UINT nRendererMesh)
	{
		stack<CAnimationObject*> FrameStack;

		FrameStack.push(pRootObject);

		while (!FrameStack.empty())
		{
			CAnimationObject* pFindObject = FrameStack.top();
			FrameStack.pop();

			if (pFindObject->IsRendererMesh() && nRendererMesh == pFindObject->GetRendererMeshNum())
				return pFindObject;

			if (pFindObject->m_pChild)
				FrameStack.push(pFindObject->m_pChild);
			if (pFindObject->m_pSibling)
				FrameStack.push(pFindObject->m_pSibling);
		}
		return NULL;
	}
	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, UINT nObjects, CAnimationObject** ppObject, UINT Index)
	{
		UINT ncbElementBytes = ((sizeof(BONE_TRANSFORMS) + 255) & ~255);
		UINT ncbElementBytes2 = ((sizeof(BONE_TRANSFORMS2) + 255) & ~255);
		UINT ncbElementBytes3 = ((sizeof(BONE_TRANSFORMS3) + 255) & ~255);

		pd3dCommandList->SetGraphicsRootShaderResourceView(8, m_ppResource[0]->GetGPUVirtualAddress());
		if (nResource > 1)
		{
			pd3dCommandList->SetGraphicsRootShaderResourceView(9, m_ppResource[1]->GetGPUVirtualAddress());
		}
		if (nResource > 2)
		{
			pd3dCommandList->SetGraphicsRootShaderResourceView(10, m_ppResource[2]->GetGPUVirtualAddress());
		}
		for (int i = 0; i < nObjects; i++)
		{
			XMStoreFloat4x4(&m_pcbMappedGameObjects[i].m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&ppObject[i]->m_xmf4x4World)));
			::memcpy(m_pcbMappedGameObjects[i].m_xmf4x4BoneTransform, &ppObject[i]->m_ppAnimationFactorObjects[Index]->m_pAnimationFactors->m_BoneTransforms->m_xmf4x4BoneTransform, sizeof(XMFLOAT4X4) * BONE_TRANSFORM_NUM);

			if (nResource > 1)
			{
				::memcpy(m_pcbMappedGameObjects2[i].m_xmf4x4BoneTransform, &ppObject[i]->m_ppAnimationFactorObjects[Index]->m_pAnimationFactors->m_BoneTransforms2->m_xmf4x4BoneTransform, sizeof(XMFLOAT4X4) * BONE_TRANSFORM_NUM2);
			}
			if (nResource > 2)
			{
				::memcpy(m_pcbMappedGameObjects3[i].m_xmf4x4BoneTransform, &ppObject[i]->m_ppAnimationFactorObjects[Index]->m_pAnimationFactors->m_BoneTransforms3->m_xmf4x4BoneTransform, sizeof(XMFLOAT4X4) * BONE_TRANSFORM_NUM3);

				for (int k = 0; k < RENDERER_MESH_WORLD_TRANSFORM; k++)
				{
					CAnimationObject* RendererMeshObject = FindMeshRendererObject(ppObject[i], k);
					if (RendererMeshObject)
					{
						XMFLOAT4X4 result = Matrix4x4::Multiply(RendererMeshObject->m_xmf4x4ToRootTransform, ppObject[i]->m_xmf4x4World);

						XMStoreFloat4x4(&m_pcbMappedGameObjects3[i].m_xmf4x4RedererMeshWorld[k], XMMatrixTranspose(XMLoadFloat4x4(&result)));
					}
				}
			}
		}
	}
	AnimationResourceFactors(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nBindpos, UINT nObjects)
	{
		m_nBindpos = nBindpos;

		if (m_nBindpos <= BONE_TRANSFORM_NUM)
			nResource = 1;
		else if (m_nBindpos > BONE_TRANSFORM_NUM && m_nBindpos < (BONE_TRANSFORM_NUM2 + BONE_TRANSFORM_NUM))
			nResource = 2;
		else
			nResource = 3;

		if (nObjects != 0)
		{
			m_ppResource = new ID3D12Resource*[nResource];

			CreateShaderVariables(pd3dDevice, pd3dCommandList, nObjects);
		}
		m_BoneTransforms = new BONE_TRANSFORMS();
		ZeroMemory(m_BoneTransforms, sizeof(BONE_TRANSFORMS));
		if (nResource > 1)
		{
			m_BoneTransforms2 = new BONE_TRANSFORMS2();
			ZeroMemory(m_BoneTransforms2, sizeof(BONE_TRANSFORMS2));
		}
		if (nResource > 2)
		{
			m_BoneTransforms3 = new BONE_TRANSFORMS3();
			ZeroMemory(m_BoneTransforms3, sizeof(BONE_TRANSFORMS3));
		}

		m_pBoneIndexList = new UINT[nBindpos];
		m_ppBoneObject = new CAnimationObject*[m_nBindpos];
	}
	~AnimationResourceFactors()
	{
		if (m_BoneTransforms)
			delete m_BoneTransforms;

		if (m_BoneTransforms2)
			delete m_BoneTransforms2;

		if (m_BoneTransforms3)
			delete m_BoneTransforms3;

		if (m_ppBoneObject)
			delete[] m_ppBoneObject;
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
			m_ppBoneObject[i] = GetFindObject(pRoot, m_pBoneIndexList[i]);
		}
	}

	CAnimationObject **m_ppBoneObject = NULL;

	UINT *m_pBoneIndexList = NULL;
	UINT m_nBindpos = 0;

	BONE_TRANSFORMS			*m_BoneTransforms;
	BONE_TRANSFORMS2			*m_BoneTransforms2;
	BONE_TRANSFORMS3			*m_BoneTransforms3;

	BONE_TRANSFORMS				*m_pcbMappedGameObjects = NULL;
	BONE_TRANSFORMS2			*m_pcbMappedGameObjects2 = NULL;
	BONE_TRANSFORMS3				*m_pcbMappedGameObjects3 = NULL;

	ID3D12Resource** m_ppResource;
	UINT nResource;
};
class CHeightMapTerrain : public CGameObject
{
public:
	CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, ID3D12Resource* pShadowMap);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage * m_pHeightMapImage;

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
