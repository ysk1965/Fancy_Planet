#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <stack>
#include <sstream>

using namespace std;

AnimationController::AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation,
	XMFLOAT4X4* pBindPoses) : m_nAnimation(nAnimation), m_pBindPoses(pBindPoses)
{
	m_pAnimation = new ANIMATION[m_nAnimation];
}
void AnimationController::SetObject(CAnimationObject* pObject)
{
	m_pRootObject = pObject;
	m_ppBoneObject = pObject->m_pAnimationFactors->m_ppBoneObject;
}

AnimationController::~AnimationController()
{
	if (m_pd3dcbBoneTransforms)
	{
		m_pd3dcbBoneTransforms->Unmap(0, NULL);
		m_pd3dcbBoneTransforms->Release();
	}
	if (m_ppBoneObject)
		delete[] m_ppBoneObject;
	if (m_pAnimation)
		delete[] m_pAnimation;
}
void AnimationController::ChangeAnimation(int iNewState)
{
	if (iNewState >= 0)
	{
		m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame = m_pRootObject->m_pAnimationFactors->m_fCurrentFrame;
		m_pRootObject->m_pAnimationFactors->m_iNewState = iNewState;
		m_pRootObject->m_pAnimationFactors->m_iSaveState = m_pRootObject->m_pAnimationFactors->m_iState;
		m_pRootObject->m_pAnimationFactors->m_iState = -1;
	}
	else
	{
		m_pRootObject->m_pAnimationFactors->m_iState = m_pRootObject->m_pAnimationFactors->m_iNewState;
	}

	m_pRootObject->m_pAnimationFactors->m_chronoStart = chrono::system_clock::now();
	m_pRootObject->m_pAnimationFactors->m_fCurrentFrame = 0.0f;
}
void AnimationController::GetCurrentFrame()
{
	m_pRootObject->m_pAnimationFactors->ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_pRootObject->m_pAnimationFactors->m_chronoStart);
	float fCurrentTime = m_pRootObject->m_pAnimationFactors->ms.count() /1000.0f;
	float fQuotient = fCurrentTime / m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].fTime;
	
	m_pRootObject->m_pAnimationFactors->m_fCurrentFrame = (fCurrentTime / m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].fTime - (UINT)fQuotient) * m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].nFrame;
}
SRT AnimationController::Interpolate(int iBoneNum)
{
	SRT result;
	float prev = (UINT)m_pRootObject->m_pAnimationFactors->m_fCurrentFrame;
	float next = (UINT)(m_pRootObject->m_pAnimationFactors->m_fCurrentFrame + 1);

	result.S = XMFLOAT3(1.0f, 1.0f, 1.0f);

	if (prev == m_pRootObject->m_pAnimationFactors->m_fCurrentFrame)
	{
		result.R = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat;
		result.T = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation;
	}
	else
	{
		float m;
		float b;
		float x = m_pRootObject->m_pAnimationFactors->m_fCurrentFrame - prev;
				
		m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.x - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.x;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.x;
		result.R.x = m * x + b;

		m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.y - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.y;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.y;
		result.R.y = m * x + b;

		m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.z - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.z;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.z;
		result.R.z = m * x + b;

		m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.w - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.w;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.w;
		result.R.w = m * x + b;

		result.R = Vector4::Normalize(result.R);

		m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.x - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.x;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.x;
		result.T.x = m * x + b;

		m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.y - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.y;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.y;
		result.T.y = m * x + b;

		m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.z - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.z;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.z;
		result.T.z = m * x + b;
	}

	return result;
}
SRT AnimationController::Interpolate(int iBoneNum, float fTime)
{
	SRT result;
	float fTimeRate = fTime / CHANGE_TIME; // 0 ~ 1사이값
	 
	float m;
	float b;

	result.S = XMFLOAT3(1.0f, 1.0f, 1.0f);  
	XMStoreFloat4(&result.R, XMQuaternionSlerp(XMLoadFloat4(&m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].RotationQuat)
		, XMLoadFloat4(&m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iNewState].pFrame[iBoneNum].RotationQuat), fTimeRate));

	m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iNewState].pFrame[iBoneNum].Translation.x - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.x;
	b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.x;
	result.T.x = m * fTimeRate + b;

	m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iNewState].pFrame[iBoneNum].Translation.y - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.y;
	b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.y;
	result.T.y = m * fTimeRate + b;

	m = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iNewState].pFrame[iBoneNum].Translation.z - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.z;
	b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.z;
	result.T.z = m * fTimeRate + b;
	return result;
}
void AnimationController::SetToParentTransforms()
{
	stack<CAnimationObject*> FrameStack;
	FrameStack.push(m_pRootObject);

	float fTime = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_pRootObject->m_pAnimationFactors->m_chronoStart).count() / 1000.0f;

	if (m_pRootObject->m_pAnimationFactors->m_iState == -1 && (fTime > CHANGE_TIME))
		ChangeAnimation(-1);

	GetCurrentFrame(); // 현재 프레임 계산
	
	for (int i = 0;; i++)
	{
		if (FrameStack.empty())
			break;

		CAnimationObject* TargetFrame = FrameStack.top();
		FrameStack.pop();

		if (TargetFrame != m_pRootObject)
		{
			SRT srt;

			if(m_pRootObject->m_pAnimationFactors->m_iState != -1)
				srt = Interpolate(i);
			else
				srt = Interpolate(i, fTime);

			XMVECTOR S = XMLoadFloat3(&srt.S);
			XMVECTOR P = XMLoadFloat3(&srt.T);
			XMVECTOR Q = XMLoadFloat4(&srt.R);

			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

			XMStoreFloat4x4(&TargetFrame->m_xmf4x4ToParentTransform, XMMatrixAffineTransformation(S, zero, Q, P));
		}

		if(TargetFrame->m_pSibling)
			FrameStack.push(TargetFrame->m_pSibling);  
		if(TargetFrame->m_pChild)
			FrameStack.push(TargetFrame->m_pChild);
	}	
}
void AnimationController::SetToRootTransforms()
{
	stack<CAnimationObject*> FrameStack;
	FrameStack.push(m_pRootObject->m_pChild);

	while (1)
	{
		if (FrameStack.empty())
			break;

		CAnimationObject* TargetFrame = FrameStack.top();
		FrameStack.pop();
		
		if(TargetFrame->m_pParent != NULL && TargetFrame != m_pRootObject->m_pChild)
			TargetFrame->m_xmf4x4ToRootTransform = Matrix4x4::Multiply(TargetFrame->m_xmf4x4ToParentTransform, TargetFrame->m_pParent->m_xmf4x4ToRootTransform);
		else
			TargetFrame->m_xmf4x4ToRootTransform = TargetFrame->m_xmf4x4ToParentTransform;

		if (TargetFrame->m_pChild)
			FrameStack.push(TargetFrame->m_pChild);
		if (TargetFrame->m_pSibling)
			FrameStack.push(TargetFrame->m_pSibling);
	}
}
void AnimationController::AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList)
{
	SetToParentTransforms(); // 시간에 맞추어 m_xmf4x4ToParentTransform에 맞는 값을 넣어준다. 
							// 현재 시간에 상관없이 계속 프레임이 진행되게 만들었다.
	SetToRootTransforms();

	for (UINT i = 0; i < m_pRootObject->m_pAnimationFactors->m_nBindpos ; i++)
	{
		XMMATRIX offset = XMLoadFloat4x4(&m_pBindPoses[i]); // 본오프셋 행렬
		XMMATRIX toRoot = XMLoadFloat4x4(&m_ppBoneObject[i]->m_xmf4x4ToRootTransform);
		XMMATRIX finalTransform = XMMatrixMultiply(XMMatrixTranspose(offset), (toRoot));

		XMStoreFloat4x4(&m_pRootObject->m_BoneTransforms->m_xmf4x4BoneTransform[i], XMMatrixTranspose(finalTransform));
	}
}


CTexture::CTexture(int nTextures, UINT nTextureType, int nSamplers)
{
	m_nTextureType = nTextureType;
	m_nTextures = nTextures;
	if (m_nTextures > 0)
	{
		m_pRootArgumentInfos = new SRVROOTARGUMENTINFO[m_nTextures];
		m_ppd3dTextureUploadBuffers = new ID3D12Resource*[m_nTextures];
		m_ppd3dTextures = new ID3D12Resource*[m_nTextures];
	}

	m_nSamplers = nSamplers;
	if (m_nSamplers > 0) 
		m_pd3dSamplerGpuDescriptorHandles = new D3D12_GPU_DESCRIPTOR_HANDLE[m_nSamplers];
}

CTexture::~CTexture()
{
	if (m_ppd3dTextures)
	{
		for (int i = 0; i < m_nTextures; i++) 
			if (m_ppd3dTextures[i]) 
				m_ppd3dTextures[i]->Release();
	}

	if (m_pRootArgumentInfos)
	{
		delete[] m_pRootArgumentInfos;
	}

	if (m_pd3dSamplerGpuDescriptorHandles) delete[] m_pd3dSamplerGpuDescriptorHandles;
}

void CTexture::SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle)
{
	m_pRootArgumentInfos[nIndex].m_nRootParameterIndex = nRootParameterIndex;
	m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle = d3dSrvGpuDescriptorHandle;
}

void CTexture::SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle)
{
	m_pd3dSamplerGpuDescriptorHandles[nIndex] = d3dSamplerGpuDescriptorHandle;
}

void CTexture::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_nTextureType == RESOURCE_TEXTURE2D_ARRAY)
	{
		pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[0].m_nRootParameterIndex, m_pRootArgumentInfos[0].m_d3dSrvGpuDescriptorHandle);
	}
	else
	{
		for (int i = 0; i < m_nTextures; i++)
		{
			pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[i].m_nRootParameterIndex, m_pRootArgumentInfos[i].m_d3dSrvGpuDescriptorHandle);
		}
	}
}

void CTexture::UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, int nIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(m_pRootArgumentInfos[nIndex].m_nRootParameterIndex, m_pRootArgumentInfos[nIndex].m_d3dSrvGpuDescriptorHandle);
}

void CTexture::ReleaseUploadBuffers()
{
	if (m_ppd3dTextureUploadBuffers)
	{
		for (int i = 0; i < m_nTextures; i++) 
			if (m_ppd3dTextureUploadBuffers[i]) 
				m_ppd3dTextureUploadBuffers[i]->Release();
		delete[] m_ppd3dTextureUploadBuffers;
		m_ppd3dTextureUploadBuffers = NULL;
	}
}

void CTexture::ReleaseShaderVariables()
{
}

ID3D12Resource *CTexture::CreateTexture(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nWidth, UINT nHeight, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE *pd3dClearValue, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTexture2DResource(pd3dDevice, pd3dCommandList, nWidth, nHeight, dxgiFormat, d3dResourceFlags, d3dResourceStates, pd3dClearValue);
	return(m_ppd3dTextures[nIndex]);
}

void CTexture::LoadTextureFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, wchar_t *pszFileName, UINT nIndex)
{
	m_ppd3dTextures[nIndex] = ::CreateTextureResourceFromFile(pd3dDevice, pd3dCommandList, pszFileName, &m_ppd3dTextureUploadBuffers[nIndex], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial::CMaterial()
{
}

CMaterial::~CMaterial()
{
	if (m_pTexture) 
		m_pTexture->Release();
	if (m_pShader) 
		m_pShader->Release();
}

void CMaterial::SetTexture(CTexture *pTexture)
{
	if (m_pTexture) 
		m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) 
		m_pTexture->AddRef();
}

void CMaterial::SetShader(CShader *pShader)
{
	if (m_pShader) 
		m_pShader->Release();

	m_pShader = pShader;

	if (m_pShader) 
		m_pShader->AddRef();
}

void CMaterial::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	if (m_pTexture) 
		m_pTexture->UpdateShaderVariables(pd3dCommandList);
}

void CMaterial::ReleaseShaderVariables()
{
	if (m_pShader) 
		m_pShader->ReleaseShaderVariables();
	if (m_pTexture) 
		m_pTexture->ReleaseShaderVariables();
}

void CMaterial::ReleaseUploadBuffers()
{
	if (m_pTexture)
		m_pTexture->ReleaseUploadBuffers();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CGameObject::CGameObject(int nMeshes)
{
	m_xmf4x4World = Matrix4x4::Identity();

	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;

	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh*[m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)	
			m_ppMeshes[i] = NULL;
	}
}

CGameObject::~CGameObject()
{
	ReleaseShaderVariables();

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
	}
	if (m_pMaterial) m_pMaterial->Release();

}

void CGameObject::ResizeMeshes(int nMeshes)
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = NULL;
		}
		delete[] m_ppMeshes;
		m_ppMeshes = NULL;
	}

	m_nMeshes = nMeshes;
	m_ppMeshes = NULL;
	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh*[m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;
	}
}

void CGameObject::SetMesh(int nIndex, CMesh *pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) 
			m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) 
			pMesh->AddRef();
	}
}

void CGameObject::SetShader(CShader *pShader)
{
	if (!m_pMaterial)
	{
		CMaterial *pMaterial = new CMaterial();
		SetMaterial(pMaterial);
	}
	if (m_pMaterial) 
		m_pMaterial->SetShader(pShader);
}

void CGameObject::SetMaterial(CMaterial *pMaterial)
{
	if (m_pMaterial) 
		m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
}

ID3D12Resource *CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObject->Map(0, NULL, (void **)&m_pcbMappedGameObject);

	return(m_pd3dcbGameObject);
}
void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
	}
	
	if (m_pMaterial) 
		m_pMaterial->ReleaseShaderVariables();
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	
	if (m_pMaterial) 
		m_pcbMappedGameObject->m_nMaterial = m_pMaterial->m_nReflection;
}

void CGameObject::UpdateAnimationVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
	
	
}

void CGameObject::Animate(float fTimeElapsed)
{
}


void CGameObject::SetRootParameter(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex)
{
	pd3dCommandList->SetGraphicsRootDescriptorTable(iRootParameterIndex, m_d3dCbvGPUDescriptorHandle);
}

void CGameObject::OnPrepareRender()
{
}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera)
{
	if (!m_bActive)
		return;

	OnPrepareRender();

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	if (m_nMeshes > 0)
	{
		SetRootParameter(pd3dCommandList, iRootParameterIndex);

		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}

}

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera, UINT nInstances)
{
	OnPrepareRender();

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	SetRootParameter(pd3dCommandList, iRootParameterIndex);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) 
				m_ppMeshes[i]->Render(pd3dCommandList, nInstances);
		}
	}
}
void CGameObject::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) 
				m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pMaterial) 
		m_pMaterial->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4World = Matrix4x4::Multiply(mtxScale, m_xmf4x4World);
}
void CGameObject::SetRotation(XMFLOAT4& pxmf4Quaternion)
{
	m_xmf4x4World._11 = pxmf4Quaternion.x;
	m_xmf4x4World._13 = pxmf4Quaternion.y;
	m_xmf4x4World._31 = pxmf4Quaternion.z;
	m_xmf4x4World._33 = pxmf4Quaternion.z;
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}




TCHAR* char2tchar(char * asc)
{
	TCHAR* ptszUni = NULL;
	int nLen = MultiByteToWideChar(CP_ACP, 0, asc, strlen(asc), NULL, NULL);
	ptszUni = new TCHAR[nLen + 1];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, asc, strlen(asc), ptszUni, nLen);
	ptszUni[nLen] = '\0';
	return ptszUni;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color) : CGameObject(0)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	
		m_ppMeshes[i] = NULL;

	CHeightMapGridMesh *pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(x + (z*cxBlocks), pHeightMapGridMesh);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pTerrainTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0);

	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/Base_Texture.dds", 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/Detail_Texture_7.dds", 1);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/NormalMap3.dds", 2);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); 

	CTerrainShader *pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 4);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pTerrainShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 3);
	pTerrainShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbGameObject, ncbElementBytes);
	pTerrainShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTerrainTexture, 3, true);

	CMaterial *pTerrainMaterial = new CMaterial();
	pTerrainMaterial->SetTexture(pTerrainTexture);

	SetMaterial(pTerrainMaterial);

	SetCbvGPUDescriptorHandle(pTerrainShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pTerrainShader);
}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) 
		delete m_pHeightMapImage;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject(6)
{
	CTexturedRectMesh *pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 0.0f, 0.0f, 0.0f, +10.0f);
	SetMesh(0, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 0.0f, 0.0f, 0.0f, -10.0f);
	SetMesh(1, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.0f, 20.0f, 20.0f, -10.0f, 0.0f, 0.0f);
	SetMesh(2, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.0f, 20.0f, 20.0f, +10.0f, 0.0f, 0.0f);
	SetMesh(3, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 0.0f, 20.0f, 0.0f, +10.0f, 0.0f);
	SetMesh(4, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 20.0f, 0.0f, 20.0f, 0.0f, -10.0f, 0.0f);
	SetMesh(5, pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pSkyBoxTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0);

	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/SkyBox/starbox_1024.dds", 0);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수

	CSkyBoxShader *pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pSkyBoxShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 1);
	pSkyBoxShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbGameObject, ncbElementBytes);
	pSkyBoxShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pSkyBoxTexture, 6, false);

	CMaterial *pSkyBoxMaterial = new CMaterial();
	pSkyBoxMaterial->SetTexture(pSkyBoxTexture);

	SetMaterial(pSkyBoxMaterial);

	SetCbvGPUDescriptorHandle(pSkyBoxShader->GetGPUCbvDescriptorStartHandle());

	SetShader(pSkyBoxShader);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	XMFLOAT3 xmf3CameraPos = pCamera->GetPosition();
	SetPosition(xmf3CameraPos.x, xmf3CameraPos.y, xmf3CameraPos.z);

	OnPrepareRender();

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
	}

	pd3dCommandList->SetGraphicsRootDescriptorTable(2, m_d3dCbvGPUDescriptorHandle);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_pMaterial)
			{
				if (m_pMaterial->m_pTexture) 
					m_pMaterial->m_pTexture->UpdateShaderVariable(pd3dCommandList, 0);
			}
			if (m_ppMeshes[i]) 
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAnimationObject::CAnimationObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nMeshes, TCHAR *pstrFileName, AnimationController* pAnimationController, UINT nAnimationNumber) : CGameObject(nMeshes), m_iAnimationNum(nAnimationNumber)
{
	m_xmf4x4ToParentTransform = Matrix4x4::Identity();

	LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrFileName, pAnimationController);
}
CAnimationObject::~CAnimationObject()
{
	if (m_pBindPoses)
		delete[] m_pBindPoses;

	if (m_pAnimationFactors)
		delete m_pAnimationFactors;

	if (m_pAnimationController)
		delete m_pAnimationController;

	if (m_pSibling)
		delete m_pSibling;
	if (m_pChild)
		delete m_pChild;
}
void CAnimationObject::ReleaseUploadBuffers()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->ReleaseUploadBuffers();
		}
	}

	if (m_pMaterial)
		m_pMaterial->ReleaseUploadBuffers();

	if (m_pSibling)
		m_pSibling->ReleaseUploadBuffers();
	if (m_pChild)
		m_pChild->ReleaseUploadBuffers();
}
void CAnimationObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera)
{
	if (!m_bActive)
		return;

	OnPrepareRender();

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
			m_pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	if (m_pAnimationController)
	{
		m_pAnimationController->SetObject(this);
		m_pAnimationController->AdvanceAnimation(pd3dCommandList);
	}

	if (m_nMeshes > 0)
	{
		SetRootParameter(pd3dCommandList, iRootParameterIndex);

		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}

	if (m_pSibling)
		m_pSibling->Render(pd3dCommandList, 2, pCamera);
	if (m_pChild)
		m_pChild->Render(pd3dCommandList, 2, pCamera);
}
void CAnimationObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera, UINT nInstances)
{
	OnPrepareRender();

	if (m_pMaterial)
	{
		if (m_pMaterial->m_pShader)
		{
			m_pMaterial->m_pShader->Render(pd3dCommandList, pCamera);
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}
	
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList, nInstances);
		}
	}

	if (m_pSibling)
		m_pSibling->Render(pd3dCommandList, pCamera, nInstances);
	if (m_pChild)
		m_pChild->Render(pd3dCommandList, pCamera, nInstances);
}
TCHAR* CAnimationObject::CharToTCHAR(char * asc)
{
	TCHAR* ptszUni = NULL;
	int nLen = MultiByteToWideChar(CP_ACP, 0, asc, strlen(asc), NULL, NULL);
	ptszUni = new TCHAR[nLen + 1];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, asc, strlen(asc), ptszUni, nLen);
	ptszUni[nLen] = '\0';
	return ptszUni;
}
void CAnimationObject::LoadAnimation(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ifstream& InFile, AnimationController* pAnimationController)
{
	UINT nAnimation = 0;

	CAnimationObject* pFindObjecct = GetRootObject();

	if (pFindObjecct)
	{
		InFile.read((char*)&nAnimation, sizeof(UINT)); // 애니메이션 수
		pAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, nAnimation
			, pFindObjecct->m_pBindPoses);
		InFile.read((char*)&pAnimationController->m_nBone, sizeof(UINT)); // 본의 갯수

		pFindObjecct->m_pAnimationFactors->SetBoneObject(pFindObjecct);

		for (int i = 0; i < pAnimationController->GetAnimationCount(); i++)
		{
			InFile.read((char*)&pAnimationController->m_pAnimation[i].nFrame, sizeof(UINT)); // 프레임 수 
			InFile.read((char*)&pAnimationController->m_pAnimation[i].fTime, sizeof(float)); // 애니메이션 시간 (30fps 기준)
			pAnimationController->m_pAnimation[i].pFrame
				= new FRAME[(pAnimationController->m_pAnimation[i].nFrame + 1) * pAnimationController->m_nBone];
			InFile.read((char*)pAnimationController->m_pAnimation[i].pFrame
				, sizeof(FRAME) * pAnimationController->m_nBone *
				(pAnimationController->m_pAnimation[i].nFrame + 1));
		}
		pFindObjecct->m_pAnimationController = pAnimationController;
	}
}
void CAnimationObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
	ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nFrame, UINT nSub)
{
	int nType = -1;
	int nSubMesh = 0;

	if (nSub == 1)
		InFile.read((char*)&nType, sizeof(int)); // 타입 받기
	else
	{
		nType = 0;
	}
	XMFLOAT3 *pxmf3ParentsPosition = NULL;

	if (nType == 0)
	{
		ResizeMeshes(1);

		XMFLOAT3 *pxmf3Positions = NULL;
		XMFLOAT2 *pxmf2TextureCoords0 = NULL;
		XMFLOAT3 *pxmf3Normals = NULL;
		XMFLOAT3 *pxmf3Tangents = NULL;
		XMFLOAT3 *pxmf3BoneWeights = NULL;
		XMINT4 *pxmi4BoneIndices = NULL;

		char* pstrAlbedoTextureName = NULL;
		char* pstrNormalTextureName = NULL;
		UINT *pnIndices = NULL;

		int nVertices = 0, nIndices = 0;

		CMaterial *pMaterial = NULL;

		InFile.read((char*)&nSubMesh, sizeof(int)); // 서브매쉬 갯수 받기

		InFile.read((char*)&GetRootObject()->m_nDrawType, sizeof(int)); // 그리기 타입 받기

		InFile.read((char*)&nVertices, sizeof(int)); // 정점 갯수 받기
		pxmf3Positions = new XMFLOAT3[nVertices];
		pxmf2TextureCoords0 = new XMFLOAT2[nVertices];
		pxmf3Normals = new XMFLOAT3[nVertices];
		pxmf3Tangents = new XMFLOAT3[nVertices];
		pxmf3BoneWeights = new XMFLOAT3[nVertices];
		pxmi4BoneIndices = new XMINT4[nVertices];

		InFile.read((char*)pxmf3Positions, sizeof(XMFLOAT3) * nVertices); // 정점 받기
		InFile.read((char*)pxmf2TextureCoords0, sizeof(XMFLOAT2) * nVertices); // uv 받기

		if (GetRootObject()->m_nDrawType > 1)
		{
			InFile.read((char*)pxmf3Normals, sizeof(XMFLOAT3) * nVertices); // 법선 받기
			InFile.read((char*)pxmf3Tangents, sizeof(XMFLOAT3) * nVertices); // 탄젠트 받기
		}
		InFile.read((char*)pxmf3BoneWeights, sizeof(XMFLOAT3) * nVertices); // 본가중치 받기
		InFile.read((char*)pxmi4BoneIndices, sizeof(XMINT4) * nVertices); // 본인덱스 받기

		int nBindPoses;

		InFile.read((char*)&nBindPoses, sizeof(int)); // 본포즈 길이 받기

		GetRootObject()->m_pAnimationFactors = new AnimationFactors(nBindPoses);

		GetRootObject()->m_pBindPoses = new XMFLOAT4X4[nBindPoses];

		InFile.read((char*)GetRootObject()->m_pBindPoses, sizeof(XMFLOAT4X4) *nBindPoses); // 본포즈 받기

		int Namesize;

		InFile.read((char*)&Namesize, sizeof(int)); // 디퓨즈맵이름 받기
		pstrAlbedoTextureName = new char[Namesize];
		InFile.read(pstrAlbedoTextureName, sizeof(char)*Namesize);

		if (GetRootObject()->m_nDrawType > 1)
		{
			InFile.read((char*)&Namesize, sizeof(int)); // 노말맵이름 받기
			pstrNormalTextureName = new char[Namesize];
			InFile.read(pstrNormalTextureName, sizeof(char)*Namesize);
		}

		CMesh* pMesh = NULL;

		if (nVertices > 0 && GetRootObject()->m_nDrawType > 1)
			pMesh = new CAnimationMesh(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf3Tangents
				, pxmf3Normals, pxmf2TextureCoords0, pxmf3BoneWeights, pxmi4BoneIndices, nIndices, pnIndices);
		else if (nVertices > 0)
			pMesh = new CMeshAnimationTextured(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf2TextureCoords0, pxmf3BoneWeights, pxmi4BoneIndices, nIndices, pnIndices);

		if (pMesh)
			SetMesh(0, pMesh);
		else
			ResizeMeshes(0);

		if (pxmf3Positions)
			delete[] pxmf3Positions;

		if (pxmf2TextureCoords0)
			delete[] pxmf2TextureCoords0;

		if (pxmf3Normals)
			delete[] pxmf3Normals;

		if (pxmf3Tangents)
			delete[] pxmf3Tangents;

		if (pnIndices)
			delete[] pnIndices;

		pMaterial = new CMaterial();

		TCHAR pstrPathName[128] = { '\0' };
		TCHAR* pstrFileName = char2tchar(pstrAlbedoTextureName);
		_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
		_tcscat_s(pstrPathName, 128, pstrFileName);
		_tcscat_s(pstrPathName, 128, _T(".dds"));

		CTexture *pTexture = new CTexture(GetRootObject()->m_nDrawType, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 0);

		if (pstrFileName)
			delete[] pstrFileName;

		if (GetRootObject()->m_nDrawType > 1)
		{
			pstrFileName = char2tchar(pstrNormalTextureName);
			_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
			_tcscat_s(pstrPathName, 128, pstrFileName);
			_tcscat_s(pstrPathName, 128, _T(".dds"));

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 1);
		}

		if (pstrAlbedoTextureName)
			delete[] pstrAlbedoTextureName;

		if (pstrNormalTextureName)
			delete[] pstrNormalTextureName;

		pMaterial->SetTexture(pTexture);

		CShader* pShader = NULL;

		if (GetRootObject()->m_nDrawType == 1)
			pShader = new CDefferredTexturedShader();
		else if (GetRootObject()->m_nDrawType == 2)
			pShader = new CDefferredLightingTexturedShader();

		pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 4);
		pShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, GetRootObject()->m_nDrawType);
		pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 5, true);
		SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());

		pMaterial->SetShader(pShader);

		if (pMaterial)
			SetMaterial(pMaterial);
	}
	if (nType <= 1)
	{
		if (nSubMesh <= nSub)
		{
			char* pstrFrameName = NULL;
			UINT nFrameNameSize;
			InFile.read((char*)&nFrameNameSize, sizeof(UINT));

			pstrFrameName = new char[nFrameNameSize];
			InFile.read(pstrFrameName, sizeof(char)*nFrameNameSize); // 프레임 이름 사이즈 받기

			TCHAR* pstrFileName = char2tchar(pstrFrameName);// 프레임 이름 받기
			_tcscat_s(m_strFrameName, 128, pstrFileName);

			InFile.read((char*)&m_iBoneIndex, sizeof(int)); // 본 인덱스 받기

			delete[] pstrFrameName;

			bool bChild = false;
			InFile.read((char*)&bChild, sizeof(bool));

			if (bChild)
			{
				int nChild = 0;
				InFile.read((char*)&nChild, sizeof(int));

				for (int i = 0; i < nChild; i++)
				{
					CAnimationObject *pChild = new CAnimationObject(0, m_iAnimationNum);
					SetChild(pChild);
					pChild->LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile, nFrame + 1, 1);
				}
			}
		}
		if (nSubMesh > nSub)
		{
			CAnimationObject *pSibling = new CAnimationObject(0, m_iAnimationNum);
			SetChild(pSibling);
			pSibling->LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile, nFrame + 1, nSub + 1);

		}
	}
}
void CAnimationObject::LoadGeometryFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName, AnimationController* pAnimationController)
{  
	ifstream fi;

	fi.open(pstrFileName, ostream::binary);

	if (!fi)
	{
		exit(1);
	}
	m_bRoot = true;
	m_BoneTransforms = new BONE_TRANSFORMS();
	LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fi, 0, 1);
	LoadAnimation(pd3dDevice, pd3dCommandList, fi, pAnimationController);
	fi.close(); // 파일 닫기
}
CAnimationObject* CAnimationObject::GetRootObject()
{
	CAnimationObject* pRootObject = this;

	while (1)
	{
		if (pRootObject->m_pParent == NULL)
			break;
		else
			pRootObject = pRootObject->m_pParent;
	}

	return pRootObject;
}

void CAnimationObject::UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4ToParentTransform, *pxmf4x4Parent) : m_xmf4x4ToParentTransform;

	if (m_pSibling)
		m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild)
		m_pChild->UpdateTransform(&m_xmf4x4World);
}

CAnimationObject *CAnimationObject::FindObject()
{
	CAnimationObject *pFrameObject = NULL;
	if (m_nMeshes > 0)
		return(this);

	if (m_pSibling)
		if (pFrameObject = m_pSibling->FindObject())
			return(pFrameObject);
	if (m_pChild)
		if (pFrameObject = m_pChild->FindObject())
			return(pFrameObject);

	return(NULL);
}

void CAnimationObject::SetChild(CAnimationObject *pChild)
{
	if (m_pChild)
	{
		CAnimationObject* pLinkObject = m_pChild;
		while (1)
		{
			if (pLinkObject->m_pSibling)
			{
				pLinkObject = pLinkObject->m_pSibling;
			}
			else
			{
				pLinkObject->m_pSibling = pChild;
				break;
			}
		}
	}
	else
	{
		m_pChild = pChild;
	}
	if (pChild)
		pChild->m_pParent = this;
}

void CAnimationObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4ToParentTransform._41 = x;
	m_xmf4x4ToParentTransform._42 = y;
	m_xmf4x4ToParentTransform._43 = z;
}

void CAnimationObject::SetPosition(XMFLOAT3& xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}
void CAnimationObject::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxScale, m_xmf4x4ToParentTransform);
}

void CAnimationObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void CAnimationObject::Rotate(XMFLOAT3 *pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}

void CAnimationObject::Rotate(XMFLOAT4 *pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4ToParentTransform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4ToParentTransform);
}
void CAnimationObject::SetRotation(XMFLOAT4& pxmf4Quaternion)
{
	m_xmf4x4ToParentTransform._11 = pxmf4Quaternion.x;
	m_xmf4x4ToParentTransform._13 = pxmf4Quaternion.y;
	m_xmf4x4ToParentTransform._31 = pxmf4Quaternion.z;
	m_xmf4x4ToParentTransform._33 = pxmf4Quaternion.z;
}
void CAnimationObject::ChangeAnimation()
{
	if (m_pAnimationController)
	{
		m_pAnimationController->ChangeAnimation(rand() % 4);
	}
	if (m_pSibling)
		m_pSibling->ChangeAnimation();
	if (m_pChild)
		m_pChild->ChangeAnimation();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
