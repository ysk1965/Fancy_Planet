#include "stdafx.h"
#include "Object.h"
#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <float.h> 

using namespace std;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
template <typename T>
Animation_Stack<T>::Animation_Stack(UINT BoneNum) : m_nCapacity(BoneNum)
{
	m_nLastIndex = 0;
	m_ppTarr = new T*[BoneNum];
	for (int i = 0; i < BoneNum; i++)
	{
		m_ppTarr[i] = NULL;
	}
}
template <typename T>
Animation_Stack<T>::~Animation_Stack()
{
	delete m_ppTarr;
}

template <typename T>
void Animation_Stack<T>::push(T* Tvalue)
{
	m_ppTarr[m_nLastIndex] = Tvalue;
	m_nLastIndex++;
}
template <typename T>
T* Animation_Stack<T>::top()
{
	assert(!empty());

	T* TopValue = m_ppTarr[m_nLastIndex - 1];
	m_ppTarr[m_nLastIndex - 1] = NULL;
	m_nLastIndex--;

	return TopValue;
}
template <typename T>
bool Animation_Stack<T>::empty()
{
	if (m_nLastIndex == 0)
		return true;
	else
		return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
AnimationController::AnimationController(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, UINT nAnimation, UINT nBone) : m_nAnimation(nAnimation), m_nBone(nBone)
{
	m_pAnimation = new ANIMATION[m_nAnimation];
	m_Animation_Stack = new Animation_Stack<CAnimationObject>(m_nBone);
}
void AnimationController::SetObject(CAnimationObject* pRootObject, CAnimationObject* pAnimationFactorObject)
{
	m_pRootObject = pRootObject;
	if (pAnimationFactorObject)
	{
		m_pFactorObject = pAnimationFactorObject;
		m_ppBoneObject = pAnimationFactorObject->m_pAnimationFactors->m_ppBoneObject;
		SetBindPoses(pAnimationFactorObject->GetBindPoses());
	}
}
void AnimationController::SetBindPoses(XMFLOAT4X4* pBindPoses)
{
	m_pBindPoses = pBindPoses;
}
AnimationController::~AnimationController()
{
	if (m_pAnimation)
		delete[] m_pAnimation;
}
void AnimationController::ChangeAnimation(int iNewState)
{
	if (iNewState != CHANG_INDEX && m_pRootObject->m_pAnimationTime->m_iState != CHANG_INDEX)
	{
		m_pRootObject->m_pAnimationTime->m_fSaveLastFrame = m_pRootObject->m_pAnimationTime->m_fCurrentFrame;
		m_pRootObject->m_pAnimationTime->m_iNewState = iNewState;

		if (m_pRootObject->m_pAnimationTime->m_iState > 0)
			m_pRootObject->m_pAnimationTime->m_iSaveState = m_pRootObject->m_pAnimationTime->m_iState;
		else
			m_pRootObject->m_pAnimationTime->m_iSaveState = -m_pRootObject->m_pAnimationTime->m_iState;

		m_pRootObject->m_pAnimationTime->m_iState = CHANG_INDEX;
	}
	else if (iNewState == CHANG_INDEX)
	{
		m_pRootObject->m_pAnimationTime->m_iState = m_pRootObject->m_pAnimationTime->m_iNewState;
	}

	m_pRootObject->m_pAnimationTime->m_chronoStart = chrono::system_clock::now();
	m_pRootObject->m_pAnimationTime->m_fCurrentFrame = 0.0f;
}
void AnimationController::GetCurrentFrame(bool bStop)
{
	if (m_pRootObject->m_pAnimationTime->m_iState != CHANG_INDEX && m_pRootObject->m_pAnimationTime->m_iState < 0)
	{
		m_pRootObject->m_pAnimationTime->ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_pRootObject->m_pAnimationTime->m_chronoStart);
		float fCurrentTime = m_pRootObject->m_pAnimationTime->ms.count() / 1000.0f;
		float fQuotient = fCurrentTime / m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].fTime;

		m_pRootObject->m_pAnimationTime->m_fCurrentFrame = (1.0f - (fCurrentTime / m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].fTime - (UINT)fQuotient)) // 0 ~1 ���̰�
			* m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].nFrame;

		if (!bStop)
		{
			m_pRootObject->m_pAnimationTime->m_fCurrentFrame = (1.0f - (fCurrentTime / m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].fTime - (UINT)fQuotient)) // 0 ~1 ���̰�
				* m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].nFrame;
		}
		else
			m_pRootObject->m_pAnimationTime->m_fCurrentFrame = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nFrame;
	}
	else
	{
		m_pRootObject->m_pAnimationTime->ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_pRootObject->m_pAnimationTime->m_chronoStart);
		float fCurrentTime = m_pRootObject->m_pAnimationTime->ms.count() / 1000.0f;
		float fQuotient = fCurrentTime / m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].fTime;

		if (!bStop)
			m_pRootObject->m_pAnimationTime->m_fCurrentFrame = (fCurrentTime / m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].fTime - (UINT)fQuotient) * m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nFrame;
		else
			m_pRootObject->m_pAnimationTime->m_fCurrentFrame = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nFrame;
	}
}
void AnimationController::Interpolate(int iBoneNum, float fRendererScale, SRT& result)
{
	float prev;
	float next;

	result.S = XMFLOAT3(fRendererScale, fRendererScale, fRendererScale);

	if (m_pRootObject->m_pAnimationTime->m_iState > 0)
	{
		prev = (UINT)m_pRootObject->m_pAnimationTime->m_fCurrentFrame;
		next = (UINT)(m_pRootObject->m_pAnimationTime->m_fCurrentFrame + 1);

		if (prev == m_pRootObject->m_pAnimationTime->m_fCurrentFrame)
		{
			result.R = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat;
			result.T = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation;
		}
		else
		{
			float m;
			float b;
			float x = m_pRootObject->m_pAnimationTime->m_fCurrentFrame - prev;

			m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.x - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.x;
			b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.x;
			result.R.x = m * x + b;

			m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.y - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.y;
			b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.y;
			result.R.y = m * x + b;

			m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.z - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.z;
			b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.z;
			result.R.z = m * x + b;

			m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.w - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.w;
			b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.w;
			result.R.w = m * x + b;

			result.R = Vector4::Normalize(result.R);

			m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.x - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.x;
			b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.x;
			result.T.x = m * x + b;

			m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.y - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.y;
			b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.y;
			result.T.y = m * x + b;

			m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.z - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.z;
			b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.z;
			result.T.z = m * x + b;
		}
	}
	else
	{
		prev = (UINT)m_pRootObject->m_pAnimationTime->m_fCurrentFrame;
		next = (UINT)(m_pRootObject->m_pAnimationTime->m_fCurrentFrame - 1);

		if (next < 0)
			next = 0;


		if (prev == m_pRootObject->m_pAnimationTime->m_fCurrentFrame)
		{
			result.R = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat;
			result.T = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation;
		}
		else
		{
			float m;
			float b;
			float x = m_pRootObject->m_pAnimationTime->m_fCurrentFrame - prev;

			m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.x - m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.x;
			b = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.x;
			result.R.x = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.y - m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.y;
			b = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.y;
			result.R.y = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.z - m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.z;
			b = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.z;
			result.R.z = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].RotationQuat.w - m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.w;
			b = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].RotationQuat.w;
			result.R.w = m * x + b;

			result.R = Vector4::Normalize(result.R);

			m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.x - m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.x;
			b = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.x;
			result.T.x = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.y - m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.y;
			b = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.y;
			result.T.y = m * x + b;

			m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)next + iBoneNum].Translation.z - m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.z;
			b = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iState].pFrame[m_nBone*(UINT)prev + iBoneNum].Translation.z;
			result.T.z = m * x + b;
		}
	}
}
void AnimationController::Interpolate(int iBoneNum, float fTime, float fRendererScale, SRT& result)
{
	float fTimeRate = fTime / CHANGE_TIME; // 0 ~ 1���̰�

	float m;
	float b;

	result.S = XMFLOAT3(fRendererScale, fRendererScale, fRendererScale);

	if (m_pRootObject->m_pAnimationTime->m_iNewState >= 0)
	{
		XMStoreFloat4(&result.R, XMQuaternionSlerp(XMLoadFloat4(&m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].RotationQuat)
			, XMLoadFloat4(&m_pAnimation[m_pRootObject->m_pAnimationTime->m_iNewState].pFrame[iBoneNum].RotationQuat), fTimeRate));

		m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iNewState].pFrame[iBoneNum].Translation.x - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.x;
		b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.x;
		result.T.x = m * fTimeRate + b;

		m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iNewState].pFrame[iBoneNum].Translation.y - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.y;
		b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.y;
		result.T.y = m * fTimeRate + b;

		m = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iNewState].pFrame[iBoneNum].Translation.z - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.z;
		b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.z;
		result.T.z = m * fTimeRate + b;
	}
	else if (m_pRootObject->m_pAnimationTime->m_iNewState < 0)
	{
		XMStoreFloat4(&result.R, XMQuaternionSlerp(XMLoadFloat4(&m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].RotationQuat)
			, XMLoadFloat4(&m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iNewState].pFrame[m_nBone*m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iNewState].nFrame + iBoneNum].RotationQuat), fTimeRate));

		m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iNewState].pFrame[m_nBone*m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iNewState].nFrame + iBoneNum].Translation.x - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.x;
		b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.x;
		result.T.x = m * fTimeRate + b;

		m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iNewState].pFrame[m_nBone*m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iNewState].nFrame + iBoneNum].Translation.y - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.y;
		b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.y;
		result.T.y = m * fTimeRate + b;

		m = m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iNewState].pFrame[m_nBone*m_pAnimation[-m_pRootObject->m_pAnimationTime->m_iNewState].nFrame + iBoneNum].Translation.z - m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.z;
		b = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iSaveState].pFrame[m_nBone*(UINT)m_pRootObject->m_pAnimationTime->m_fSaveLastFrame + iBoneNum].Translation.z;
		result.T.z = m * fTimeRate + b;
	}
}
void AnimationController::SetToParentTransforms()
{
	float fTime = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - m_pRootObject->m_pAnimationTime->m_chronoStart).count() / 1000.0f;
	bool bStop = false;
	
	if (m_pRootObject->m_pAnimationTime->m_iState != CHANG_INDEX)
	{
		if (m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].fTime < fTime)
		{
			if (m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nType == ALL ||
				m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nType == PLAY_NOW)
			{
				ChangeAnimation(NONE);
			}
			else if (m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nType == CONTINUOUS_PLAYBACK)
			{
				ChangeAnimation(m_pRootObject->m_pAnimationTime->m_iState + 1);
				fTime = 0;
			}
			else if (m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nType == NOT_CYCLE)
			{
				bStop = true;
				fTime = 0;
			}
			else if (m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nType == CYCLE_NEED_TIME)
			{
				m_pRootObject->m_pAnimationTime->m_fSaveLastFrame = m_pAnimation[m_pRootObject->m_pAnimationTime->m_iState].nFrame - 1;
				m_pRootObject->m_pAnimationTime->m_iNewState = m_pRootObject->m_pAnimationTime->m_iState;

				m_pRootObject->m_pAnimationTime->m_iSaveState = m_pRootObject->m_pAnimationTime->m_iState;
				m_pRootObject->m_pAnimationTime->m_iState = CHANG_INDEX;

				m_pRootObject->m_pAnimationTime->m_chronoStart = chrono::system_clock::now();
				m_pRootObject->m_pAnimationTime->m_fCurrentFrame = 0.0f;
				fTime = 0;
			}
		}
	}

	if (m_pRootObject->m_pAnimationTime->m_iState == CHANG_INDEX && (fTime > CHANGE_TIME))
	{
		ChangeAnimation(CHANG_INDEX);
	}

	if (m_pRootObject->m_pAnimationTime->m_iState != CHANG_INDEX)
		GetCurrentFrame(bStop); // ���� ������ ���

	m_Animation_Stack->push(m_pRootObject); // ��Ʈ ��� ���� ����
	for (int i = 0;; i++)
	{// ���ÿ� ���̻� ��尡 ������ ������ ���� ���´�.
		if (m_Animation_Stack->empty())
			break;
		// �ִϸ��̼� �����͸� ������ ��, ���̿켱 ������� �����߱�
		// �� ������� ��带 ���鼭 ����� ä�� �ִ´�.
		CAnimationObject* TargetFrame = m_Animation_Stack->top();

		if (TargetFrame != m_pRootObject)
		{
			SRT srt;
			float fRendererScale = TargetFrame->GetRndererScale();
			// �ִϸ��̼ǰ� �ִϸ��̼� ���̴� Ư���ð����� �����Ͽ�
			// �ִϸ��̼� ��ȯ�� �ڿ������� �Ѵ�.
			if (m_pRootObject->m_pAnimationTime->m_iState != CHANG_INDEX)
				Interpolate(i, fRendererScale, srt);
			else
				Interpolate(i, fTime, fRendererScale, srt);

			XMVECTOR S = XMLoadFloat3(&srt.S);
			XMVECTOR P = XMLoadFloat3(&srt.T);
			XMVECTOR Q = XMLoadFloat4(&srt.R);

			XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
			// �ִϸ��̼ǵ����ͷ� ���� ����� Position���� Quaternion���� ������ ����� �����, ToParent��Ŀ� ����.
			XMStoreFloat4x4(&TargetFrame->m_xmf4x4ToParentTransform, XMMatrixAffineTransformation(S, zero, Q, P));
		}

		if (TargetFrame->m_pSibling)
			m_Animation_Stack->push(TargetFrame->m_pSibling);
		if (TargetFrame->m_pChild)
			m_Animation_Stack->push(TargetFrame->m_pChild);
	}
}
void AnimationController::SetToRootTransforms()
{// ���� �켱 Ž���� �ǽ��Ѵ�.

	m_Animation_Stack->push(m_pRootObject->m_pChild);

	while (1)
	{
		if (m_Animation_Stack->empty())
			break;

		CAnimationObject* TargetFrame = m_Animation_Stack->top();

		// ��Ʈ�� �ڽ� ������ �����ؼ�, ������ �Ʒ��� ToParent����� ���ؼ� ������, ToRoot����� �����.
		if (TargetFrame->m_pParent != NULL && TargetFrame != m_pRootObject->m_pChild)
			TargetFrame->m_xmf4x4ToRootTransform =
			Matrix4x4::Multiply(TargetFrame->m_xmf4x4ToParentTransform, TargetFrame->m_pParent->m_xmf4x4ToRootTransform);
		else // ���� �θ� ��尡 ������ �ڽ��� ToParent����� �� ToRoot����� �ȴ�.
			TargetFrame->m_xmf4x4ToRootTransform = TargetFrame->m_xmf4x4ToParentTransform;

		if (TargetFrame->m_pSibling)
			m_Animation_Stack->push(TargetFrame->m_pSibling);
		if (TargetFrame->m_pChild)
			m_Animation_Stack->push(TargetFrame->m_pChild);
	}
}
void AnimationController::AdvanceAnimation(ID3D12GraphicsCommandList* pd3dCommandList, bool bOnce)
{
	if (bOnce)
	{
		SetToParentTransforms(); // �ð��� ���߾� m_xmf4x4ToParentTransform�� �´� ���� �־��ش�. 
		SetToRootTransforms();
	}
	else
	{
		for (UINT i = 0; i < m_pFactorObject->m_pAnimationFactors->m_nBindpos; i++)
		{
			// ���� ��ǥ�迡�� �ش� ����� ��ǥ��� �̵���Ű�� ���
			XMMATRIX offset = XMLoadFloat4x4(&m_pBindPoses[i]);
			// �ִϸ��̼� ������ ���, �ش� ����� ��ǥ�迡�� ���� ��ǥ��� �̵���Ű�� ���
			XMMATRIX toRoot = XMLoadFloat4x4(&m_ppBoneObject[i]->m_xmf4x4ToRootTransform);
			XMMATRIX finalTransform = XMMatrixMultiply(XMMatrixTranspose(offset), (toRoot));

			// ������۴� ������ �ֱ⶧���� �ѹ��� 32���� ��Ĺۿ� ���̴��� �Ѱ����� ���Ѵ�.
			// �Ͽ� ������� 3���� ����� ���� �����Ѵ�. 
			if (i < BONE_TRANSFORM_NUM)
				XMStoreFloat4x4(&m_pFactorObject->m_pAnimationFactors->m_BoneTransforms->m_xmf4x4BoneTransform[i], XMMatrixTranspose(finalTransform));
			else if (i >= BONE_TRANSFORM_NUM && i < (BONE_TRANSFORM_NUM2 + BONE_TRANSFORM_NUM))
				XMStoreFloat4x4(&m_pFactorObject->m_pAnimationFactors->m_BoneTransforms2->m_xmf4x4BoneTransform[i - BONE_TRANSFORM_NUM], XMMatrixTranspose(finalTransform));
			else
				XMStoreFloat4x4(&m_pFactorObject->m_pAnimationFactors->m_BoneTransforms3->m_xmf4x4BoneTransform[i - (BONE_TRANSFORM_NUM2 + BONE_TRANSFORM_NUM)], XMMatrixTranspose(finalTransform));
		}
	}
}
