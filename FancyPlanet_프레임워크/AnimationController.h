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
	XMFLOAT3 *Translation;
	XMFLOAT4 *RotationQuat;
};
struct ANIMATION
{
	UINT nTime; // 전체 프레임 수
	FRAME* pFrame; // 한프레임에 각 뼈에 대한 변환
};
class AnimationController
{
private:
	UINT m_nAnimation = 0;
	UINT m_nState = IDLE;
	UINT m_nCurrentFrame = 0;

	ID3D12Resource					*m_pd3dcbBoneTransforms = NULL;

	BONE_TRANSFORMS								*m_pBoneTransforms = NULL;
	ID3D12DescriptorHeap			*m_pd3dBoneTransformsDescriptorHeap = NULL;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dBoneTransformsCPUDescriptorStartHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dBoneTransformsGPUDescriptorStartHandle;
public:
	XMFLOAT4X4								*m_pBindPoses;

	UINT GetAnimationCount()
	{
		return m_nAnimation;
	}
	AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation);
	~AnimationController();
	void Interpolate(float fTime, XMFLOAT4X4* m_xmf4x4ToParentTransforms, UINT nBindPoses);
	ID3D12Resource* CreateBoneTransformVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateBoneTransformDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews);
	void CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride);
	void AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList, UINT nBindPoses);
	ANIMATION *m_pAnimation;
};

