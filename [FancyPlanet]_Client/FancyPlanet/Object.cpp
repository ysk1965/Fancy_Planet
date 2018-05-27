#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <stack>
#include <sstream>
#include <float.h> 

using namespace std;

AnimationController::AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation) : m_nAnimation(nAnimation)
{
	m_pAnimation = new ANIMATION[m_nAnimation];
}
void AnimationController::SetObject(CAnimationObject* pObject)
{
	m_pRootObject = pObject;
	m_ppBoneObject = pObject->m_pAnimationFactors->m_ppBoneObject;
}
void AnimationController::SetBindPoses(XMFLOAT4X4* pBindPoses)
{
	m_pBindPoses = pBindPoses;
}
AnimationController::~AnimationController()
{
	if (m_ppBoneObject)
		delete[] m_ppBoneObject;
	if (m_pAnimation)
		delete[] m_pAnimation;
}
void AnimationController::ChangeAnimation(int iNewState)
{
	if (iNewState != CHANG_INDEX && m_pRootObject->m_pAnimationFactors->m_iState != CHANG_INDEX)
	{
		m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame = m_pRootObject->m_pAnimationFactors->m_fCurrentFrame;
		m_pRootObject->m_pAnimationFactors->m_iNewState = iNewState;

		if (m_pRootObject->m_pAnimationFactors->m_iState > 0)
			m_pRootObject->m_pAnimationFactors->m_iSaveState = m_pRootObject->m_pAnimationFactors->m_iState;
		else
			m_pRootObject->m_pAnimationFactors->m_iSaveState = -m_pRootObject->m_pAnimationFactors->m_iState;

		m_pRootObject->m_pAnimationFactors->m_iState = CHANG_INDEX;
	}
	else if (iNewState == CHANG_INDEX)
	{
		m_pRootObject->m_pAnimationFactors->m_iState = m_pRootObject->m_pAnimationFactors->m_iNewState;
	}

	m_pRootObject->m_pAnimationFactors->m_chronoStart = chrono::system_clock::now();
	m_pRootObject->m_pAnimationFactors->m_fCurrentFrame = 0.0f;
}
void AnimationController::GetCurrentFrame(bool bStop)
{
	if (m_pRootObject->m_pAnimationFactors->m_iState != CHANG_INDEX && m_pRootObject->m_pAnimationFactors->m_iState < 0)
	{
		m_pRootObject->m_pAnimationFactors->ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_pRootObject->m_pAnimationFactors->m_chronoStart);
		float fCurrentTime = m_pRootObject->m_pAnimationFactors->ms.count() / 1000.0f;
		float fQuotient = fCurrentTime / m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].fTime;

		m_pRootObject->m_pAnimationFactors->m_fCurrentFrame = (1.0f - (fCurrentTime / m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].fTime - (UINT)fQuotient)) // 0 ~1 사이값
			* m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].nFrame;

		if (!bStop)
		{
			m_pRootObject->m_pAnimationFactors->m_fCurrentFrame = (1.0f - (fCurrentTime / m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].fTime - (UINT)fQuotient)) // 0 ~1 사이값
				* m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].nFrame;
		}
		else
			m_pRootObject->m_pAnimationFactors->m_fCurrentFrame = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].nFrame;
	}
	else
	{
		m_pRootObject->m_pAnimationFactors->ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_pRootObject->m_pAnimationFactors->m_chronoStart);
		float fCurrentTime = m_pRootObject->m_pAnimationFactors->ms.count() / 1000.0f;
		float fQuotient = fCurrentTime / m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].fTime;

		if(!bStop)
			m_pRootObject->m_pAnimationFactors->m_fCurrentFrame = (fCurrentTime / m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].fTime - (UINT)fQuotient) * m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].nFrame;
		else
			m_pRootObject->m_pAnimationFactors->m_fCurrentFrame = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].nFrame;
	}
}
SRT AnimationController::Interpolate(int iBoneNum, float fRendererScale)
{
	SRT result;

	float prev;
	float next;

	result.S = XMFLOAT3(fRendererScale, fRendererScale, fRendererScale);

	if (m_pRootObject->m_pAnimationFactors->m_iState > 0)
	{
		prev = (UINT)m_pRootObject->m_pAnimationFactors->m_fCurrentFrame;
		next = (UINT)(m_pRootObject->m_pAnimationFactors->m_fCurrentFrame + 1);

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
	}
	else
	{
		prev = (UINT)m_pRootObject->m_pAnimationFactors->m_fCurrentFrame;
		next = (UINT)(m_pRootObject->m_pAnimationFactors->m_fCurrentFrame - 1);

		if (next < 0)
			next = 0;


		if (prev == m_pRootObject->m_pAnimationFactors->m_fCurrentFrame)
		{
			result.R = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat;
			result.T = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation;
		}
		else
		{
			float m;
			float b;
			float x = m_pRootObject->m_pAnimationFactors->m_fCurrentFrame - prev;

			m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.x - m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.x;
			b = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.x;
			result.R.x = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.y - m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.y;
			b = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.y;
			result.R.y = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.z - m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.z;
			b = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.z;
			result.R.z = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.w - m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.w;
			b = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.w;
			result.R.w = m * x + b;

			result.R = Vector4::Normalize(result.R);

			m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.x - m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.x;
			b = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.x;
			result.T.x = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.y - m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.y;
			b = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.y;
			result.T.y = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.z - m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.z;
			b = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.z;
			result.T.z = m * x + b;
		}
	}
	return result;
}
SRT AnimationController::Interpolate(int iBoneNum, float fTime, float fRendererScale)
{
	SRT result;
	float fTimeRate = fTime / CHANGE_TIME; // 0 ~ 1사이값

	float m;
	float b;

	result.S = XMFLOAT3(fRendererScale, fRendererScale, fRendererScale);

	if (m_pRootObject->m_pAnimationFactors->m_iNewState >= 0)
	{
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
	}
	else if (m_pRootObject->m_pAnimationFactors->m_iNewState < 0)
	{
		XMStoreFloat4(&result.R, XMQuaternionSlerp(XMLoadFloat4(&m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].RotationQuat)
			, XMLoadFloat4(&m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iNewState].pFrame[m_nBone*m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iNewState].nFrame + iBoneNum].RotationQuat), fTimeRate));

		m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iNewState].pFrame[m_nBone*m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iNewState].nFrame + iBoneNum].Translation.x - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.x;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.x;
		result.T.x = m * fTimeRate + b;

		m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iNewState].pFrame[m_nBone*m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iNewState].nFrame + iBoneNum].Translation.y - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.y;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.y;
		result.T.y = m * fTimeRate + b;

		m = m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iNewState].pFrame[m_nBone*m_pAnimation[-m_pRootObject->m_pAnimationFactors->m_iNewState].nFrame + iBoneNum].Translation.z - m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.z;
		b = m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationFactors->m_fSaveLastFrame + iBoneNum].Translation.z;
		result.T.z = m * fTimeRate + b;
	}
	return result;
}
void AnimationController::SetToParentTransforms()
{

	float fTime = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_pRootObject->m_pAnimationFactors->m_chronoStart).count() / 1000.0f;
	bool bStop = false;

	if (m_pRootObject->m_pAnimationFactors->m_iState != CHANGE_TIME)
	{
		if (m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].fTime < fTime)
		{
			if (m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].nType == ALL ||
				m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].nType == PLAY_NOW)
			{
				ChangeAnimation(0);
				fTime = 0;
			}
			else if (m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].nType == CONTINUOUS_PLAYBACK)
			{
				ChangeAnimation(m_pRootObject->m_pAnimationFactors->m_iState + 1);
				fTime = 0;
			}
			else if (m_pAnimation[m_pRootObject->m_pAnimationFactors->m_iState].nType == NOT_CYCLE)
			{
				bStop = true;
				fTime = 0;
			}
		}
	}
	
	if (m_pRootObject->m_pAnimationFactors->m_iState == CHANG_INDEX && (fTime > CHANGE_TIME))
	{
		ChangeAnimation(CHANG_INDEX);
	}

	if (m_pRootObject->m_pAnimationFactors->m_iState != CHANG_INDEX)		
			GetCurrentFrame(bStop); // 현재 프레임 계산

	stack<CAnimationObject*> FrameStack;
	FrameStack.push(m_pRootObject); // 루트 노드 부터 시작
	for (int i = 0;; i++)
	{// 스택에 더이상 노드가 없으면 루프를 빠져 나온다.
		if (FrameStack.empty())
			break;
		// 애니메이션 데이터를 저장할 때, 깊이우선 방식으로 저장했기
		// 그 순서대로 노드를 돌면서 행렬을 채워 넣는다.
		CAnimationObject* TargetFrame = FrameStack.top();
		FrameStack.pop();

		if (TargetFrame != m_pRootObject)
		{
			SRT srt;
			float fRendererScale = TargetFrame->GetRndererScale();
			// 애니메이션과 애니메이션 사이는 특정시간동안 보간하여
			// 애니메이션 전환을 자연스럽게 한다.
			if (m_pRootObject->m_pAnimationFactors->m_iState != CHANG_INDEX)
				srt = Interpolate(i, fRendererScale);
			else
				srt = Interpolate(i, fTime, fRendererScale);

			XMVECTOR S = XMLoadFloat3(&srt.S);
			XMVECTOR P = XMLoadFloat3(&srt.T);
			XMVECTOR Q = XMLoadFloat4(&srt.R);

			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			// 애니메이션데이터로 부터 얻어진 Position값과 Quaternion값을 가지고 행렬을 만들어, ToParent행렬에 대입.
			XMStoreFloat4x4(&TargetFrame->m_xmf4x4ToParentTransform, XMMatrixAffineTransformation(S, zero, Q, P));
		}

		if (TargetFrame->m_pSibling)
			FrameStack.push(TargetFrame->m_pSibling);
		if (TargetFrame->m_pChild)
			FrameStack.push(TargetFrame->m_pChild);
	}
}
void AnimationController::SetToRootTransforms()
{// 깊이 우선 탐색을 실시한다.
	stack<CAnimationObject*> FrameStack;
	FrameStack.push(m_pRootObject->m_pChild);

	while (1)
	{
		if (FrameStack.empty())
			break;

		CAnimationObject* TargetFrame = FrameStack.top();
		FrameStack.pop();
		// 루트의 자식 노드부터 시작해서, 위부터 아래로 ToParent행렬을 곱해서 갱신해, ToRoot행렬을 만든다.
		if (TargetFrame->m_pParent != NULL && TargetFrame != m_pRootObject->m_pChild)
			TargetFrame->m_xmf4x4ToRootTransform =  
			Matrix4x4::Multiply(TargetFrame->m_xmf4x4ToParentTransform, TargetFrame->m_pParent->m_xmf4x4ToRootTransform);
		else // 위로 부모 노드가 없으면 자신의 ToParent행렬이 곧 ToRoot행렬이 된다.
			TargetFrame->m_xmf4x4ToRootTransform = TargetFrame->m_xmf4x4ToParentTransform;
		
		if (TargetFrame->m_pSibling)
			FrameStack.push(TargetFrame->m_pSibling);
		if (TargetFrame->m_pChild)
			FrameStack.push(TargetFrame->m_pChild);
	}
}
void AnimationController::AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList)
{
	SetToParentTransforms(); // 시간에 맞추어 m_xmf4x4ToParentTransform에 맞는 값을 넣어준다. 
							 // 현재 시간에 상관없이 계속 프레임이 진행되게 만들었다.
	SetToRootTransforms();

	for (UINT i = 0; i < m_pRootObject->m_pAnimationFactors->m_nBindpos; i++)
	{
		// 월드 좌표계에서 해당 노드의 좌표계로 이동시키는 행렬
		XMMATRIX offset = XMLoadFloat4x4(&m_pBindPoses[i]); 
		// 애니메이션 정보를 담고, 해당 노드의 좌표계에서 월드 좌표계로 이동시키는 행렬
		XMMATRIX toRoot = XMLoadFloat4x4(&m_ppBoneObject[i]->m_xmf4x4ToRootTransform); 
		XMMATRIX finalTransform = XMMatrixMultiply(XMMatrixTranspose(offset), (toRoot));

		// 상수버퍼는 제한이 있기때문에 한번에 32개의 행렬밖에 쉐이더로 넘겨주지 못한다.
		// 하여 상수버퍼 3개를 사용해 값을 전달한다. 
		if (i < BONE_TRANSFORM_NUM)
			XMStoreFloat4x4(&m_pRootObject->m_BoneTransforms->m_xmf4x4BoneTransform[i], XMMatrixTranspose(finalTransform));
		else if (i >= BONE_TRANSFORM_NUM && i < (BONE_TRANSFORM_NUM2 + BONE_TRANSFORM_NUM))
			XMStoreFloat4x4(&m_pRootObject->m_BoneTransforms2->m_xmf4x4BoneTransform[i - BONE_TRANSFORM_NUM], XMMatrixTranspose(finalTransform));
		else
			XMStoreFloat4x4(&m_pRootObject->m_BoneTransforms3->m_xmf4x4BoneTransform[i - (BONE_TRANSFORM_NUM2 + BONE_TRANSFORM_NUM)], XMMatrixTranspose(finalTransform));
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

TCHAR* char2tchar(char * asc)
{
	TCHAR* ptszUni = NULL;
	int nLen = MultiByteToWideChar(CP_ACP, 0, asc, strlen(asc), NULL, NULL);
	ptszUni = new TCHAR[nLen + 1];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, asc, strlen(asc), ptszUni, nLen);
	ptszUni[nLen] = '\0';
	return ptszUni;
}

CGameObject::CGameObject(int nMeshes, UINT nObjects)
{
	m_xmf4x4World = Matrix4x4::Identity();

	m_nMeshes = nMeshes;
	m_nObjects = nObjects;
	m_ppMeshes = NULL;

	if (m_nMeshes > 0)
	{
		m_ppMeshes = new CMesh*[m_nMeshes];
		for (int i = 0; i < m_nMeshes; i++)
			m_ppMeshes[i] = NULL;
	}
}
CGameObject::CGameObject(int nMeshes, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName, UINT nObjects) :m_nObjects(nObjects)
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
	ifstream fi;

	fi.open(pstrFileName, ostream::binary);

	if (!fi)
	{
		exit(1);
	}

	LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fi, GET_TYPE, 1);

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
	if (m_pMaterial)
		m_pMaterial->Release();

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

void CGameObject::LoadFrameHierarchyFromFile(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
	ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nType, UINT nSub)
{
	int m_nType = -1;
	int nSubMesh = 0;
	static UINT nRendererMesh = 0;

	if (nType != SKIP)
		InFile.read((char*)&m_nType, sizeof(int)); // 타입 받기
	else
	{
		m_nType = SKINNEDMESH;
	}
	XMFLOAT3 *pxmf3ParentsPosition = NULL;

	if (m_nType == SKINNEDMESH || m_nType == RENDERMESH)
	{

		ResizeMeshes(1);

		XMFLOAT3 *pxmf3Positions = NULL;
		XMFLOAT2 *pxmf2TextureCoords0 = NULL;
		XMFLOAT3 *pxmf3Normals = NULL;
		XMFLOAT3 *pxmf3Tangents = NULL;

		char* pstrAlbedoTextureName = NULL;
		char* pstrNormalTextureName = NULL;
		UINT *pIndices = NULL;

		int nVertices = 0, nIndices = 0;

		CMaterial *pMaterial = NULL;

		InFile.read((char*)&nSubMesh, sizeof(int)); // 서브매쉬 갯수 받기

		InFile.read((char*)&m_nDrawType, sizeof(int)); // 그리기 타입 받기

		InFile.read((char*)&nVertices, sizeof(int)); // 정점 갯수 받기
		InFile.read((char*)&nIndices, sizeof(int)); // 정점 갯수 받기

		pIndices = new UINT[nIndices];
		pxmf3Positions = new XMFLOAT3[nVertices];
		pxmf2TextureCoords0 = new XMFLOAT2[nVertices];
		pxmf3Normals = new XMFLOAT3[nVertices];
		pxmf3Tangents = new XMFLOAT3[nVertices];

		InFile.read((char*)pIndices, sizeof(UINT) * nIndices); // 인덱스 받기
		InFile.read((char*)pxmf3Positions, sizeof(XMFLOAT3) * nVertices); // 정점 받기
		InFile.read((char*)pxmf2TextureCoords0, sizeof(XMFLOAT2) * nVertices); // uv 받기

		if (m_nDrawType > 1)
		{
			InFile.read((char*)pxmf3Normals, sizeof(XMFLOAT3) * nVertices); // 법선 받기
			InFile.read((char*)pxmf3Tangents, sizeof(XMFLOAT3) * nVertices); // 탄젠트 받기
		}
		int Namesize;

		InFile.read((char*)&Namesize, sizeof(int)); // 디퓨즈맵이름 받기
		pstrAlbedoTextureName = new char[Namesize];
		InFile.read(pstrAlbedoTextureName, sizeof(char)*Namesize);

		if (m_nDrawType > 1)
		{
			InFile.read((char*)&Namesize, sizeof(int)); // 노말맵이름 받기
			pstrNormalTextureName = new char[Namesize];
			InFile.read(pstrNormalTextureName, sizeof(char)*Namesize);
		}

		CMesh* pMesh = NULL;

		pMesh = new CMeshIlluminatedTexturedTBN(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf3Tangents, pxmf3Normals, pxmf2TextureCoords0, nIndices, pIndices);

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

		if (pIndices)
			delete[] pIndices;

		pMaterial = new CMaterial();

		TCHAR pstrPathName[128] = { '\0' };
		TCHAR* pstrFileName = char2tchar(pstrAlbedoTextureName);
		_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
		_tcscat_s(pstrPathName, 128, pstrFileName);
		_tcscat_s(pstrPathName, 128, _T(".dds"));

		CTexture *pTexture = new CTexture(m_nDrawType, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 0);

		if (pstrFileName)
			delete[] pstrFileName;

		if (m_nDrawType > 1)
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

		if (m_nDrawType == 1 && m_nType == SKINNEDMESH)
			pShader = new CDefferredTexturedShader();
		else if (m_nDrawType == 2 && m_nType == SKINNEDMESH)
			pShader = new CDefferredLightingTexturedShader();
		else
			pShader = new CObjectShader();

		CreateShaderVariables(pd3dDevice, pd3dCommandList);
		pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 4);
		pShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_nDrawType);
		pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 2, true);
		SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());

		pMaterial->SetShader(pShader);

		if (pMaterial)
			SetMaterial(pMaterial);
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

void CGameObject::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObjects = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes * (m_nObjects)
		, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObjects->Map(0, NULL, (void **)&m_pcbMappedGameObjects);
}
void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObjects)
	{
		m_pd3dcbGameObjects->Unmap(0, NULL);
		m_pd3dcbGameObjects->Release();
	}

	if (m_pMaterial)
		m_pMaterial->ReleaseShaderVariables();
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList, UINT RootParameterIndex, CPhysXObject** ppObjects)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	if (RootParameterIndex != -1)
		pd3dCommandList->SetGraphicsRootShaderResourceView(RootParameterIndex, m_pd3dcbGameObjects->GetGPUVirtualAddress());

	for (int i = 0; i < m_nObjects; i++)
	{
		CB_GAMEOBJECT_INFO *pbMappedcbGameObject = (CB_GAMEOBJECT_INFO *)((UINT8 *)m_pcbMappedGameObjects + (i * ncbElementBytes));
		if (ppObjects != NULL)
			XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&ppObjects[i]->m_xmf4x4World)));
		else
			XMStoreFloat4x4(&pbMappedcbGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	}
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
		}
		if (m_pMaterial->m_pTexture)
		{
			m_pMaterial->m_pTexture->UpdateShaderVariables(pd3dCommandList);
		}
	}

	UpdateShaderVariables(pd3dCommandList, -1, NULL);

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

void CGameObject::Render(ID3D12GraphicsCommandList *pd3dCommandList, int iRootParameterIndex, CCamera *pCamera, UINT nInstances, CPhysXObject** ppObjects)
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

	UpdateShaderVariables(pd3dCommandList, iRootParameterIndex, ppObjects);

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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color,
	PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking) : CGameObject(0, 1)
{
	// PxHeightField : x축을 따라 x축, y축을 따라... z축을 따라... 스케일 된 PxTriangleMesh로 모양을 만든다.
	// 먼저 만들어야 할 것들
	PxRigidActor* myActor = NULL;
	PxTriangleMesh* myTriMesh = NULL;
	PxMaterial* myMaterial = NULL;

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
			pHeightMapGridMesh = new CHeightMapGridMesh(pPxPhysicsSDK, pPxScene, pPxControllerManager, pCooking, myActor, myTriMesh, myMaterial,
				pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(x + (z*cxBlocks), pHeightMapGridMesh);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pTerrainTexture = new CTexture(3, RESOURCE_TEXTURE2D, 0);

	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/Base_Texture.dds", 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/Detail_Texture.dds", 1);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/Terrain/NormalMap3.dds", 2);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CTerrainShader *pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 4);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pTerrainShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 3);
	pTerrainShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbGameObjects, ncbElementBytes);
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
CSkyBox::CSkyBox(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature) : CGameObject(6, 1)
{
	CTexturedRectMesh *pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 200.0f, 200.0f, 0.0f, 0.0f, 0.0f, +100.0f);
	SetMesh(0, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 200.0f, 200.0f, 0.0f, 0.0f, 0.0f, -100.0f);
	SetMesh(1, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.0f, 200.0f, 200.0f, -100.0f, 0.0f, 0.0f);
	SetMesh(2, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 0.0f, 200.0f, 200.0f, +100.0f, 0.0f, 0.0f);
	SetMesh(3, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 200.0f, 0.0f, 200.0f, 0.0f, +100.0f, 0.0f);
	SetMesh(4, pSkyBoxMesh);
	pSkyBoxMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 200.0f, 0.0f, 200.0f, 0.0f, -100.0f, 0.0f);
	SetMesh(5, pSkyBoxMesh);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	CTexture *pSkyBoxTexture = new CTexture(6, RESOURCE_TEXTURE2D, 0);

	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/SkyBox/SkyBox_Front.dds", 0);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/SkyBox/SkyBox_Back.dds", 1);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/SkyBox/SkyBox_Left.dds", 2);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/SkyBox/SkyBox_Right.dds", 3);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/SkyBox/SkyBox_Top.dds", 4);
	pSkyBoxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"../Assets/Image/SkyBox/SkyBox_Bottom.dds", 5);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수

	CSkyBoxShader *pSkyBoxShader = new CSkyBoxShader();
	pSkyBoxShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pSkyBoxShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pSkyBoxShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 6);
	pSkyBoxShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbGameObjects, ncbElementBytes);
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

			UpdateShaderVariables(pd3dCommandList, -1, NULL);
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
					m_pMaterial->m_pTexture->UpdateShaderVariable(pd3dCommandList, i);
			}
			if (m_ppMeshes[i])
				m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAnimationObject::CAnimationObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList
	, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nMeshes, TCHAR *pstrFileName, AnimationController* pAnimationController) : CGameObject(nMeshes, 0)
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

	if (m_BoneTransforms)
		delete m_BoneTransforms;

	if (m_BoneTransforms2)
		delete m_BoneTransforms2;

	if (m_BoneTransforms3)
		delete m_BoneTransforms3;

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
		pAnimationController = new AnimationController(pd3dDevice, pd3dCommandList, nAnimation);
		InFile.read((char*)&pAnimationController->m_nBone, sizeof(UINT)); // 본의 갯수

		pFindObjecct->m_pAnimationFactors->SetBoneObject(pFindObjecct);

		for (int i = 0; i < pAnimationController->GetAnimationCount(); i++)
		{
			InFile.read((char*)&pAnimationController->m_pAnimation[i].nFrame, sizeof(UINT)); // 프레임 수 
			InFile.read((char*)&pAnimationController->m_pAnimation[i].fTime, sizeof(float)); // 애니메이션 시간 (30fps 기준)
			InFile.read((char*)&pAnimationController->m_pAnimation[i].nType, sizeof(UINT)); // 애니메이션 타입

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
	ID3D12RootSignature *pd3dGraphicsRootSignature, ifstream& InFile, UINT nType, UINT nSub, CMesh* pCMesh)
{
	int m_nType = -1;
	int nSubMesh = 0;
	static UINT nRendererMesh = 0;
	CMesh* pMesh = NULL;

	if (nType != SKIP)
		InFile.read((char*)&m_nType, sizeof(int)); // 타입 받기
	else
	{
		m_nType = SKINNEDMESH;
	}
	XMFLOAT3 *pxmf3ParentsPosition = NULL;

	if (m_nType == SKINNEDMESH || m_nType == RENDERMESH)
	{
		if (m_nType == RENDERMESH)
		{
			m_bRendererMesh = true;
			m_nRendererMesh = nRendererMesh++;
		}

		ResizeMeshes(1);

		XMFLOAT3 *pxmf3Positions = NULL;
		XMFLOAT2 *pxmf2TextureCoords0 = NULL;
		XMFLOAT3 *pxmf3Normals = NULL;
		XMFLOAT3 *pxmf3Tangents = NULL;
		XMFLOAT3 *pxmf3BoneWeights = NULL;
		XMINT4 *pxmi4BoneIndices = NULL;

		char* pstrAlbedoTextureName = NULL;
		char* pstrNormalTextureName = NULL;
		char* pstrSpecularTextureName = NULL;
		UINT *pIndices = NULL;

		int nVertices = 0, nIndices = 0, nSubIndices = 0, nStartIndex = 0;

		CMaterial *pMaterial = NULL;

		InFile.read((char*)&nSubMesh, sizeof(int)); // 서브매쉬 갯수 받기

		InFile.read((char*)&m_nDrawType, sizeof(int)); // 그리기 타입 받기

		InFile.read((char*)&nStartIndex, sizeof(int)); // 인덱스 시작 위치
		InFile.read((char*)&nSubIndices, sizeof(int)); // 서브인덱스 갯수 받기

		if((nSubMesh > 1 && nSub == 1) || nSubMesh == 1)
		{
			InFile.read((char*)&nVertices, sizeof(int)); // 정점 갯수 받기
			InFile.read((char*)&nIndices, sizeof(int)); // 전체 인덱스 갯수 받기

			pIndices = new UINT[nIndices];
			pxmf3Positions = new XMFLOAT3[nVertices];
			pxmf2TextureCoords0 = new XMFLOAT2[nVertices];
			pxmf3Normals = new XMFLOAT3[nVertices];
			pxmf3Tangents = new XMFLOAT3[nVertices];
			pxmf3BoneWeights = new XMFLOAT3[nVertices];
			pxmi4BoneIndices = new XMINT4[nVertices];

			InFile.read((char*)pIndices, sizeof(UINT) * nIndices); // 인덱스 받기
			InFile.read((char*)pxmf3Positions, sizeof(XMFLOAT3) * nVertices); // 정점 받기
			InFile.read((char*)pxmf2TextureCoords0, sizeof(XMFLOAT2) * nVertices); // uv 받기

			if (m_nDrawType > 1)
			{
				InFile.read((char*)pxmf3Normals, sizeof(XMFLOAT3) * nVertices); // 법선 받기
				InFile.read((char*)pxmf3Tangents, sizeof(XMFLOAT3) * nVertices); // 탄젠트 받기
			}
			if (m_nType == SKINNEDMESH)
			{
				InFile.read((char*)pxmf3BoneWeights, sizeof(XMFLOAT3) * nVertices); // 본가중치 받기
				InFile.read((char*)pxmi4BoneIndices, sizeof(XMINT4) * nVertices); // 본인덱스 받기

				int nBindPoses;

				InFile.read((char*)&nBindPoses, sizeof(int)); // 본포즈 길이 받기

				if (GetRootObject()->m_pAnimationFactors == NULL)
					GetRootObject()->m_pAnimationFactors = new AnimationFactors(nBindPoses);

				GetRootObject()->m_pBindPoses = new XMFLOAT4X4[nBindPoses];

				InFile.read((char*)GetRootObject()->m_pBindPoses, sizeof(XMFLOAT4X4) * nBindPoses); // 본포즈 받기
			}
			

			if (nVertices > 0 && m_nDrawType > 1 && m_nType == SKINNEDMESH)
				pMesh = new CAnimationMesh(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf3Tangents
					, pxmf3Normals, pxmf2TextureCoords0, pxmf3BoneWeights, pxmi4BoneIndices, nSubIndices, pIndices);
			else if (nVertices > 0 && m_nType == SKINNEDMESH)
				pMesh = new CMeshAnimationTextured(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf2TextureCoords0, pxmf3BoneWeights, pxmi4BoneIndices, nSubIndices, pIndices);
			else if (m_nType == RENDERMESH)
				pMesh = new CRendererMesh(pd3dDevice, pd3dCommandList, nVertices, pxmf3Positions, pxmf3Tangents, pxmf3Normals, pxmf2TextureCoords0, m_nRendererMesh, nSubIndices, pIndices);

			pMesh->SetStartIndex(nStartIndex);

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

			if (pIndices)
				delete[] pIndices;
		}
		else if(nSubMesh > nSub)
		{
			pMesh = new EmptyMesh(pCMesh->GetVertexBuffer(), pCMesh->GetIndexBuffer(), pCMesh->GetStride(), pCMesh->m_nVertices, nStartIndex, nSubIndices);
			pMesh->SetStartIndex(nStartIndex);
		}
		
		int Namesize;

		InFile.read((char*)&Namesize, sizeof(int)); // 디퓨즈맵이름 받기
		pstrAlbedoTextureName = new char[Namesize];
		InFile.read(pstrAlbedoTextureName, sizeof(char)*Namesize);

		if (m_nDrawType > 1)
		{
			InFile.read((char*)&Namesize, sizeof(int)); // 노말맵이름 받기
			pstrNormalTextureName = new char[Namesize];
			InFile.read(pstrNormalTextureName, sizeof(char)*Namesize);
		}
		if (m_nDrawType > 2)
		{
			InFile.read((char*)&Namesize, sizeof(int)); // 노말맵이름 받기
			pstrSpecularTextureName = new char[Namesize];
			InFile.read(pstrSpecularTextureName, sizeof(char)*Namesize);
		}

		pMaterial = new CMaterial();

		TCHAR pstrPathName[128] = { '\0' };
		TCHAR* pstrFileName = char2tchar(pstrAlbedoTextureName);
		_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
		_tcscat_s(pstrPathName, 128, pstrFileName);
		_tcscat_s(pstrPathName, 128, _T(".dds"));

		CTexture *pTexture = new CTexture(m_nDrawType, RESOURCE_TEXTURE2D, 0);
		pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 0);

		if (pstrFileName)
			delete[] pstrFileName;

		if (m_nDrawType > 1)
		{
			pstrFileName = char2tchar(pstrNormalTextureName);
			_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
			_tcscat_s(pstrPathName, 128, pstrFileName);
			_tcscat_s(pstrPathName, 128, _T(".dds"));

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 1);
		}

		if (m_nDrawType > 2)
		{
			pstrFileName = char2tchar(pstrSpecularTextureName);
			_tcscpy_s(pstrPathName, 128, _T("../Assets/"));
			_tcscat_s(pstrPathName, 128, pstrFileName);
			_tcscat_s(pstrPathName, 128, _T(".dds"));

			pTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, pstrPathName, 2);
		}

		if (pstrAlbedoTextureName)
			delete[] pstrAlbedoTextureName;

		if (pstrNormalTextureName)
			delete[] pstrNormalTextureName;

		if (pstrSpecularTextureName)
			delete[] pstrSpecularTextureName;

		pMaterial->SetTexture(pTexture);

		CShader* pShader = NULL;

		if (m_nDrawType == 1 && m_nType == SKINNEDMESH)
			pShader = new CDefferredTexturedShader();
		else if (m_nDrawType == 2 && m_nType == SKINNEDMESH)
			pShader = new CDefferredLightingTexturedShader();
		else if (m_nDrawType == 3 && m_nType == SKINNEDMESH)
			pShader = new CSpecularLightingTexturedShader();
		else if (m_nDrawType == 2 && m_nType == RENDERMESH)
			pShader = new CRendererMeshShader();
		else if (m_nDrawType == 3 && m_nType == RENDERMESH)
			pShader = new CRendererSpecularMeshShader();

		pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature, 4);
		pShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 0, m_nDrawType);
		pShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTexture, 5, true);
		SetCbvGPUDescriptorHandle(pShader->GetGPUCbvDescriptorStartHandle());

		pMaterial->SetShader(pShader);

		if (pMaterial)
			SetMaterial(pMaterial);
	}

	{
		if (m_nType == RENDERMESH)
		{
			InFile.read((char*)&m_RndererScale, sizeof(float));
		}

		char* pstrFrameName = NULL;
		UINT nFrameNameSize;
		InFile.read((char*)&nFrameNameSize, sizeof(UINT));

		pstrFrameName = new char[nFrameNameSize];
		InFile.read(pstrFrameName, sizeof(char)*nFrameNameSize); // 프레임 이름 사이즈 받기

		TCHAR* pstrFileName = char2tchar(pstrFrameName);// 프레임 이름 받기
		_tcscat_s(m_strFrameName, 128, pstrFileName);

		InFile.read((char*)&m_nBoneType, sizeof(int));
		InFile.read((char*)&m_iBoneIndex, sizeof(int)); // 본 인덱스 받기

		delete[] pstrFrameName;

		if (nSubMesh <= nSub)
		{
			bool bChild = false;
			InFile.read((char*)&bChild, sizeof(bool));

			if (bChild)
			{
				int nChild = 0;
				InFile.read((char*)&nChild, sizeof(int));

				for (int i = 0; i < nChild; i++)
				{
					CAnimationObject *pChild = new CAnimationObject(0);
					SetChild(pChild);
					pChild->LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile, GET_TYPE, 1, NULL);
				}
			}
		}
		if (nSubMesh > nSub)
		{
			CAnimationObject *pSibling = new CAnimationObject(0);
			SetChild(pSibling);
			pSibling->LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile, SKIP, ++nSub, pMesh);

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

	LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, fi, GET_TYPE, 1, NULL);

	bool bAnimation;
	fi.read((char*)&bAnimation, sizeof(bool));

	if (bAnimation)
	{
		m_BoneTransforms = new BONE_TRANSFORMS();
		m_BoneTransforms2 = new BONE_TRANSFORMS2();
		m_BoneTransforms3 = new BONE_TRANSFORMS2();
		LoadAnimation(pd3dDevice, pd3dCommandList, fi, pAnimationController);
	}
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
void CAnimationObject::ChangeAnimation(int newState)
{
	m_pAnimationController->SetObject(this);
	if (m_pAnimationFactors->m_iState == newState)
	{
		return;
	}
	if (m_pAnimationFactors->m_iState == CHANG_INDEX && newState != PLAY_NOW)
		return;

	if (newState == PLAY_NOW)
		m_pAnimationFactors->m_iState = 0;

	if (newState < 0) // 뒤로 재생되어야 하는 애니메이션이 맞는지 확인
	{
		if (m_pAnimationController->m_pAnimation[-newState].nType != CAN_BACK_PLAY)
			return;
	}
	if (m_pAnimationController->m_pAnimation[m_pAnimationFactors->m_iState].nType == ALL)
	{
		return;
	}
	if (m_pAnimationController->m_pAnimation[m_pAnimationFactors->m_iState].nType == PLAY_NOW)
	{
		return;
	}
	if (m_pAnimationController->m_pAnimation[m_pAnimationFactors->m_iState].nType == CONTINUOUS_PLAYBACK)
		return;

	if (m_pAnimationController->m_pAnimation[m_pAnimationFactors->m_iState].nType == NOT_CYCLE)
		return;

	if (m_pAnimationController)
	{
		m_pAnimationController->ChangeAnimation(newState);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPhysXObject::CPhysXObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, UINT nObjects) : CGameObject(1, 0)
{
}

CPhysXObject::CPhysXObject(int nMeshes, ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList,
	ID3D12RootSignature *pd3dGraphicsRootSignature, TCHAR *pstrFileName, UINT nObjects)
	: CGameObject(nMeshes, pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, pstrFileName, nObjects)
{
}

CPhysXObject::CPhysXObject(int nMeshes) : CGameObject(nMeshes, 0)
{
}

CPhysXObject::~CPhysXObject()
{
}
void CPhysXObject::SetPosition(XMFLOAT3& xmf3Position)
{
	if (m_pPxActor != NULL) {
		CGameObject::SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);

		PxTransform PxTransform = m_pPxActor->getGlobalPose();

		PxTransform.p = PxVec3(xmf3Position.x, xmf3Position.y, xmf3Position.z);

		m_pPxActor->setGlobalPose(PxTransform);
	}
}

void CPhysXObject::UpdateDirectPos(const XMFLOAT3& xmf3Position, const XMFLOAT3& DirShooting) 
{
	PxVec3 DxDir;
	DxDir.x = DirShooting.x;
	DxDir.y = DirShooting.y;
	DxDir.z = DirShooting.z;

	m_pPxActor->setLinearVelocity(DxDir * 100, true);
	
	XMFLOAT3 PxPos;

	if (xmf3Position.x - m_pPxActor->getGlobalPose().p.x > 10.0f || xmf3Position.x - m_pPxActor->getGlobalPose().p.x < -10.0f)
		PxPos.x = xmf3Position.x + DirShooting.x * 10;
	else
		PxPos.x = m_pPxActor->getGlobalPose().p.x;

	if (xmf3Position.y - m_pPxActor->getGlobalPose().p.y > 10.0f || xmf3Position.y - m_pPxActor->getGlobalPose().p.y < -10.0f)
		PxPos.y = xmf3Position.y + DirShooting.y * 10;
	else
		PxPos.y = m_pPxActor->getGlobalPose().p.y;

	if (xmf3Position.z - m_pPxActor->getGlobalPose().p.z > 10.0f || xmf3Position.z - m_pPxActor->getGlobalPose().p.z < -10.0f)
		PxPos.z = xmf3Position.z + DirShooting.z * 10;
	else
		PxPos.z = m_pPxActor->getGlobalPose().p.z;

	CGameObject::SetPosition(PxPos);
}

void CPhysXObject::SetRotation(XMFLOAT4& pxmf4Quaternion)
{
	//if (m_pPxActor != NULL) {
	m_xmf4x4World._11 = pxmf4Quaternion.x;
	m_xmf4x4World._13 = pxmf4Quaternion.y;
	m_xmf4x4World._31 = pxmf4Quaternion.z;
	m_xmf4x4World._33 = pxmf4Quaternion.z;

	PxTransform PxTransform = m_pPxActor->getGlobalPose();

	PxTransform.q *= PxQuat(pxmf4Quaternion.x, PxVec3(1, 0, 0));
	PxTransform.q *= PxQuat(pxmf4Quaternion.z, PxVec3(0, 1, 0));
	PxTransform.q *= PxQuat(pxmf4Quaternion.y, PxVec3(0, 0, 1));

	m_pPxActor->setGlobalPose(PxTransform);
	//}
}

int CPhysXObject::Update(const float& fTimeDelta)
{

	CPhysXObject::Update(fTimeDelta);

	PhysXUpdate(fTimeDelta);

	return 0;
}

void CPhysXObject::PhysXUpdate(const float & fTimeDelta)
{
	PxTransform pT = m_pPxActor->getGlobalPose();
	PxMat44 m = PxMat44(pT);
	PxVec3 RotatedOffset = m.rotate(PxVec3(0.0f, 0.0f, 0.0f));
	m.setPosition(PxVec3(m.getPosition() + RotatedOffset));

}

void CPhysXObject::BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial, XMFLOAT3 vScale, PxCooking* pCooking, const char* name)
{

	//바운딩박스
	XMFLOAT3 vMin = (XMFLOAT3(1, 1, 1));
	vMin.x *= vScale.x;	 vMin.y *= vScale.y;  vMin.z *= vScale.z;

	XMFLOAT3 vMax = (XMFLOAT3(1, 1, 1));
	vMax.x *= vScale.x;	 vMax.y *= vScale.y;  vMax.z *= vScale.z;

	if (name == "Bullet") {
		XMFLOAT3 tempMultiply = (XMFLOAT3(1, 1, 1));
		tempMultiply.x = 100;
		tempMultiply.y = 100;
		tempMultiply.z = 100;

		CGameObject::SetScale(tempMultiply.x, tempMultiply.y, tempMultiply.z);
	}

	XMFLOAT3 _d3dxvExtents = // 피직스 범위
		XMFLOAT3((abs(vMin.x) + abs(vMax.x)) / 2, (abs(vMin.y) + abs(vMax.y)) / 2, (abs(vMin.z) + abs(vMax.z)) / 2);

	//객체의 최종행렬
	PxTransform _PxTransform(0, 0, 0);
	//_PxTransform.q = PxQuat(3, 3, 3, 3);

	PxSphereGeometry _PxSphereGeom(15);


	PxBoxGeometry _PxBoxGeometry(_d3dxvExtents.x, _d3dxvExtents.y, _d3dxvExtents.z);
	m_pPxActor = PxCreateDynamic(*pPxPhysics, _PxTransform, _PxSphereGeom, *pPxMaterial, 10.0f);
	if (m_pPxActor != NULL) {
		m_pPxActor->setName(name);
		pPxScene->addActor(*m_pPxActor);
	}
}

void CPhysXObject::shooting(float fpower) {
	PxVec3 m_PxDirection;
	m_PxDirection.x = m_Direction.x * fpower;
	m_PxDirection.y = m_Direction.y * fpower;
	m_PxDirection.z = m_Direction.z * fpower;

	m_pPxActor->setLinearVelocity(m_PxDirection);

	//m_pPxActor->addForce(m_PxDirection, PxForceMode::eFORCE);

	//PxRaycastBuffer Gunhit;
	//PxQueryFilterData Gunfd;
	//Gunfd.flags |= PxQueryFlag::ePOSTFILTER;

	//bool m_bTwoCheck = false;

	//PxVec3 StartPos = PxVec3();


	//PxVec3 BulletDir = m_PxDirection;

	//m_bTwoCheck = m_pScene->raycast(StartPos, BulletDir, 100, Gunhit, PxHitFlags(PxHitFlag::eDEFAULT), Gunfd);

	//m_pPxActor->addForce(m_PxDirection, PxForceMode::eFORCE);
	//PxRigidDynamic* actor = Gunhit.block.actor->is<PxRigidDynamic>();
	//const PxTransform globalPose = m_pPxActor->getGlobalPose();
	//const PxVec3 localPos = globalPose.transformInv((Gunhit.block.position));
	/*AddForceAtLocalPos(*m_pPxActor, m_PxDirection, localPos, PxForceMode::eACCELERATION, true);*/
}

void CPhysXObject::GetPhysXPos() {
	if (m_pPxActor != NULL) {
		XMFLOAT3 PxPos;
		PxPos.x = m_pPxActor->getGlobalPose().p.x; // + DirShooting.x * 1000;
		PxPos.y = m_pPxActor->getGlobalPose().p.y; // + DirShooting.y * 1000;
		PxPos.z = m_pPxActor->getGlobalPose().p.z; // + DirShooting.z * 1000;

		CGameObject::SetPosition(PxPos);
	}
}

void CPhysXObject::AddForceAtLocalPos(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup)
{
	//transform pos to world space
	const PxVec3 globalForcePos = body.getGlobalPose().transform(pos);

	AddForceAtPosInternal(body, force, globalForcePos, mode, wakeup);
}

void CPhysXObject::AddForceAtPosInternal(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup)
{
	const PxTransform globalPose = body.getGlobalPose();
	const PxVec3 centerOfMass = globalPose.transform(body.getCMassLocalPose().p);

	const PxVec3 torque = (pos - centerOfMass).cross(force);
	body.addForce(force, mode, wakeup);
	body.addTorque(torque, mode, wakeup);
}