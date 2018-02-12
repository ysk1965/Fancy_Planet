#include "stdafx.h"
#include "AnimationController.h"


AnimationController::AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation, 
	XMFLOAT4X4* pBindPoses, UINT nBindPos) : m_nAnimation(nAnimation), m_pBindPoses(pBindPoses), m_nBindpos(nBindPos)
{
	m_pAnimation = new ANIMATION[m_nAnimation];

	UINT ncbElementBytes = (((sizeof(BONE_TRANSFORMS)) + 255) & ~255);

	ID3D12Resource *pd3dcbBoneTransformsResource = CreateBoneTransformVariables(pd3dDevice, pd3dCommandList);

	CreateBoneTransformDescriptorHeaps(pd3dDevice, pd3dCommandList, 1);
	CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, pd3dcbBoneTransformsResource, ncbElementBytes);

	m_pxmf4x4toParentTransforms = new XMFLOAT4X4[m_nBindpos];
	m_pxmf4x4toRootTransforms = new XMFLOAT4X4[m_nBindpos];
}


AnimationController::~AnimationController()
{
	if (m_pd3dcbBoneTransforms)
	{
		m_pd3dcbBoneTransforms->Unmap(0, NULL);
		m_pd3dcbBoneTransforms->Release();
	}
}
void AnimationController::Interpolate(float fTime, XMFLOAT4X4* m_xmf4x4ToParentTransforms)
{
	for (int i = 0; i < m_nBindpos; i++)
	{
		XMFLOAT3 Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

		XMVECTOR S = XMLoadFloat3(&Scale);
		XMVECTOR P = XMLoadFloat3(&m_pAnimation[0].pFrame[m_nBindpos*m_nCurrentFrame + i].Translation);
		XMVECTOR Q = XMLoadFloat4(&m_pAnimation[0].pFrame[m_nBindpos*m_nCurrentFrame + i].RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&m_xmf4x4ToParentTransforms[i], XMMatrixAffineTransformation(S, zero, Q, P));

	}
	m_nCurrentFrame++;
	if (m_nCurrentFrame == m_pAnimation[0].nTime)
		m_nCurrentFrame = 0;
}

ID3D12Resource *AnimationController::CreateBoneTransformVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(BONE_TRANSFORMS) + 255) & ~255); //256의 배수
	m_pd3dcbBoneTransforms = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbBoneTransforms->Map(0, NULL, (void **)&m_pBoneTransforms);

	return(m_pd3dcbBoneTransforms);
}
void AnimationController::CreateBoneTransformDescriptorHeaps(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, int nConstantBufferViews)
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	d3dDescriptorHeapDesc.NumDescriptors = nConstantBufferViews; //CBVs
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dBoneTransformsDescriptorHeap);

	m_d3dBoneTransformsCPUDescriptorStartHandle = m_pd3dBoneTransformsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_d3dBoneTransformsGPUDescriptorStartHandle = m_pd3dBoneTransformsDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
}

void AnimationController::CreateConstantBufferViews(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, int nConstantBufferViews, ID3D12Resource *pd3dConstantBuffers, UINT nStride)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = pd3dConstantBuffers->GetGPUVirtualAddress();
	D3D12_CONSTANT_BUFFER_VIEW_DESC d3dCBVDesc;
	d3dCBVDesc.SizeInBytes = nStride;
	for (int j = 0; j < nConstantBufferViews; j++)
	{
		d3dCBVDesc.BufferLocation = d3dGpuVirtualAddress + (nStride * j);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCbvCPUDescriptorHandle;
		d3dCbvCPUDescriptorHandle.ptr = m_d3dBoneTransformsCPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j);
		pd3dDevice->CreateConstantBufferView(&d3dCBVDesc, d3dCbvCPUDescriptorHandle);
	}
}
void AnimationController::AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(8, m_d3dBoneTransformsGPUDescriptorStartHandle);

	Interpolate(0.0f, m_pxmf4x4toParentTransforms);
	
	m_pxmf4x4toRootTransforms[0] = m_pxmf4x4toParentTransforms[0];

	for (int i = 1; i < m_nBindpos; i++)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&m_pxmf4x4toParentTransforms[i]);

		XMMATRIX parentToRoot = XMLoadFloat4x4(&m_pxmf4x4toRootTransforms[i -1]);

		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

		XMStoreFloat4x4(&m_pxmf4x4toRootTransforms[i], toRoot);
	}

	for (UINT i = 0; i < m_nBindpos; i++)
	{
		XMMATRIX offset = XMLoadFloat4x4(&m_pBindPoses[i]); // 해당 뼈의 영역으로 점들을 이동시킨다. 
		XMMATRIX toRoot = XMLoadFloat4x4(&m_pxmf4x4toRootTransforms[i]);
		XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);
		XMStoreFloat4x4(&m_pBoneTransforms->m_xmf4x4BoneTransform[i], XMMatrixTranspose(finalTransform));
	}
}