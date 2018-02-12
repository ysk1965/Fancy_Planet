#pragma once

enum
{
	IDLE,
	WALK,
	RUN,
	ATTACK
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
class AnimationController
{
private:
	UINT m_nAnimation = 0;
	UINT m_nState = IDLE;
	UINT m_nCurrentFrame = 0;
	UINT m_nBindpos = 0;

	ID3D12Resource					*m_pd3dcbBoneTransforms = NULL;

	BONE_TRANSFORMS								*m_pBoneTransforms = NULL;
	ID3D12DescriptorHeap			*m_pd3dBoneTransformsDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dBoneTransformsCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dBoneTransformsGPUDescriptorStartHandle;

	XMFLOAT4X4* m_pxmf4x4toParentTransforms = NULL;
	XMFLOAT4X4* m_pxmf4x4toRootTransforms = NULL;
public:
	// 정점 위치를 뼈 로컬 공간으로 옮김으로써 사실상 정점을 뼈에 부모로 묶는다.
	XMFLOAT4X4								*m_pBindPoses;

	UINT GetAnimationCount()
	{
		return m_nAnimation;
	}
	AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation,
		XMFLOAT4X4* pBindPoses, UINT nBindPos);
	~AnimationController();
	void Interpolate(float fTime, XMFLOAT4X4* m_xmf4x4ToParentTransforms);
	ID3D12Resource* CreateBoneTransformVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateBoneTransformDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	void AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList);
	ANIMATION *m_pAnimation;
};

