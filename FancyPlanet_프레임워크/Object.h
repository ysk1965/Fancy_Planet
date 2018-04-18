#pragma once
#include "Mesh.h"
#include "Camera.h"
#include <chrono>

enum {
	NOT_ALL = 0, // 중간에 애니메이션 가능 (구현)
	ALL = 1, // 애니메이션이 끝나야 다른 얘니메이션이 가능 (구현)
	MODIFIABLE = 2, // 애니메이션 시간이 변경될수 있음
	NOT_CYCLE = 3, // 한번만 재생되고 마지막 모습에서 정지
	CAN_BACK_PLAY = 4, // 뒤로도 재생이 필요한 애니메이션(구현)
	CYCLE_NEED_TIME = 5 // 싸이클이 필요하지만 처음과 끝이 같지 않는 애니메이션 
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

#define CHANGE_TIME 0.2f

#define CHANG_INDEX -500

class CShader;
class CAnimationObject;
class AnimationFactors;

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4						m_xmf4x4World;
	UINT									m_nMaterial;
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
class AnimationController
{
private:
	UINT m_nAnimation = 0;

	ID3D12Resource					*m_pd3dcbBoneTransforms = NULL;

	ID3D12DescriptorHeap			*m_pd3dBoneTransformsDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dBoneTransformsCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dBoneTransformsGPUDescriptorStartHandle;

	CAnimationObject* m_pRootObject = NULL;
	CAnimationObject **m_ppBoneObject = NULL;

public:
	UINT m_nBone = 0;

	XMFLOAT4X4								*m_pBindPoses;

	UINT GetAnimationCount()
	{
		return m_nAnimation;
	}

	AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation,
		XMFLOAT4X4* pBindPoses);
	~AnimationController();
	void GetCurrentFrame();
	SRT Interpolate(int iBoneNum);
	SRT Interpolate(int iBoneNum, float fTime);
	void SetToParentTransforms();
	//해당하는 본인덱스의 오브젝트를 리턴한다.
	void SetToRootTransforms();
	void AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList);
	void ChangeAnimation(int iNewState);
	void SetObject(CAnimationObject* pObject);


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

	ID3D12Resource *CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex);
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
	CGameObject(int nMeshes);
	virtual ~CGameObject();

private:
	int								m_nReferences = 0;

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

	bool							m_bActive = true;

	XMFLOAT4X4						m_xmf4x4World;

	CMesh							**m_ppMeshes;
	int								m_nMeshes;

	CMaterial						*m_pMaterial = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;

protected:
	ID3D12Resource					*m_pd3dcbGameObject = NULL;
	CB_GAMEOBJECT_INFO				*m_pcbMappedGameObject = NULL;

public:
	void SetMesh(int nIndex, CMesh *pMesh);
	void SetShader(CShader *pShader);
	void SetMaterial(CMaterial *pMaterial);
	void ResizeMeshes(int nMeshes);

	void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
	void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(m_d3dCbvGPUDescriptorHandle); }

	virtual ID3D12Resource *CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void UpdateAnimationVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void OnPrepareRender();
	virtual void SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex);
	
	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera, UINT nInstances);
	virtual void BuildMaterials(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseUploadBuffers();
	virtual void ChangeAnimation(int newState) {};
	virtual void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL){};

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	
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

public:
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

	CAnimationObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature,
		UINT nMeshes, TCHAR *pstrFileName, AnimationController* pAnimationController);
	CAnimationObject(int nMeshes) : CGameObject(nMeshes)
	{
		m_xmf4x4ToRootTransform = Matrix4x4::Identity();
		m_xmf4x4ToParentTransform = Matrix4x4::Identity();
	};
	virtual ~CAnimationObject();
	void ReleaseUploadBuffers();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera = NULL);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances);
	void LoadAnimation(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ifstream& InFile, AnimationController* pAnimationController);
	void LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
		ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nFrame, UINT nSub);
	void LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature
		, TCHAR *pstrFileName, AnimationController* pAnimationController);
	virtual void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL);
	CAnimationObject* GetRootObject();
	CAnimationObject *FindObject();
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

	XMFLOAT4X4						m_xmf4x4ToRootTransform;
	XMFLOAT4X4						m_xmf4x4ToParentTransform;
	BONE_TRANSFORMS			*m_BoneTransforms;

	TCHAR							m_strFrameName[256] = { '\0' };
	bool							m_bRoot = false;
	int								m_iBoneIndex = -1;
	UINT							m_nDrawType = -1;
	XMFLOAT4X4* m_pBindPoses = NULL;

	AnimationController				*m_pAnimationController = NULL;
	AnimationFactors				*m_pAnimationFactors = NULL;

	CAnimationObject& operator=(const CAnimationObject& other)
	{
		if (this == &other)
			return *this;

		memcpy(m_strFrameName, other.m_strFrameName, sizeof(TCHAR) * 256);
		m_bRoot = other.m_bRoot;
		m_iBoneIndex = other.m_iBoneIndex;
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
		m_iState = 1;
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
	CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
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

