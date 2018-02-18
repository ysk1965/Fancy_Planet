#pragma once
#include "Mesh.h"
#include "Camera.h"

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

class CShader;
class CGameObject;

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
	XMFLOAT4X4 m_xmf4x4BoneTransform[96];
};
struct FRAME
{
	XMFLOAT3 Translation;
	XMFLOAT4 RotationQuat;
};
struct ANIMATION
{
	UINT nTime; // 전체 프레임 수
	FRAME* pFrame; // [Frame * BoneIndex]
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class AnimationController
{
private:
	UINT m_nAnimation = 0;
	UINT m_nState = 0;
	UINT num = 0;

	float m_fCurrentFrame = 0;
	UINT m_nBindpos = 0;
	UINT *m_pBoneIndex = NULL;

	ID3D12Resource					*m_pd3dcbBoneTransforms = NULL;

	CGameObject											**m_pBoneObject = NULL;
	BONE_TRANSFORMS								*m_pBoneTransforms = NULL;
	BONE_TRANSFORMS								  m_BoneTransforms;
	ID3D12DescriptorHeap			*m_pd3dBoneTransformsDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dBoneTransformsCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dBoneTransformsGPUDescriptorStartHandle;

	CGameObject* m_pRootObject = NULL;
public:
	XMFLOAT4X4								*m_pBindPoses;

	UINT GetAnimationCount()
	{
		return m_nAnimation;
	}
	AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation,
		XMFLOAT4X4* pBindPoses, UINT nBindPos, CGameObject* pRootObject);
	~AnimationController();
	void Interpolate(float fTime);
	//해당하는 본인덱스의 오브젝트를 리턴한다.
	CGameObject* GetFindObject(CGameObject* pFindObject, UINT nBoneIndex);
	void AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList);
	// 계층관계로 인덱스를 정렬한다. 
	void LineUPBoneIndex(CGameObject* pObject);

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
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

public:
	TCHAR							m_pstrFrameName[256];
	bool							m_bActive = true;
	bool							m_bRoot = false;

	XMFLOAT4X4						m_xmf4x4ToParentTransform;
	XMFLOAT4X4						m_xmf4x4World;
	XMFLOAT4X4						m_xmf4x4ToRootTransform;

	CMesh							**m_ppMeshes;
	int								m_nMeshes;
	int							m_nBoneIndex = -1;
	UINT						m_nBindPoses = 0;
	XMFLOAT4X4* m_pBindPoses = NULL;

	CMaterial						*m_pMaterial = NULL;

	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dCbvGPUDescriptorHandle;
	AnimationController				*m_pAnimationController;

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

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();


	void SetWorldPosition(XMFLOAT3 xmf3Position);
	void SetWorldPosition(float x, float y, float z);
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);
	void SetLocalPosition(XMFLOAT3& xmf3Position);
	void SetScale(float x, float y, float z);
	void SetLocalScale(float x, float y, float z);
	void SetRotation(XMFLOAT4& pxmf4Quaternion);
	void SetLocalRotation(XMFLOAT4& pxmf4Quaternion);
	void SetChild(CGameObject *pChild);

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3 *pxmf3Axis, float fAngle);
	void Rotate(XMFLOAT4 *pxmf4Quaternion);
	void LoadAnimation(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ifstream& InFile);
	void LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, 
		ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nFrame, UINT nSub);
	void LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName);
	void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL);
	CGameObject* GetRootObject();
public:
	CGameObject 					*m_pParent = NULL;
	CGameObject 					*m_pChild = NULL;
	CGameObject 					*m_pSibling = NULL;

	CGameObject *GetParent() { return(m_pParent); }
	CGameObject *FindObject();
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
class CharaterObject : public CGameObject
{
public:
	CharaterObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nMeshes, TCHAR *pstrFileName);
	~CharaterObject();
};
