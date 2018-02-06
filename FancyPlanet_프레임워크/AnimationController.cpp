#include "stdafx.h"
#include "AnimationController.h"


AnimationController::AnimationController(UINT nAnimation) : m_nAnimation(nAnimation)
{
	m_pAnimation = new ANIMATION*[m_nAnimation];
}


AnimationController::~AnimationController()
{
}

void AnimationController::ChangeAnimation(UINT nState)
{
	m_nState = nState;
}

void AnimationController::AddAnimation(UINT nAnimation, UINT nKeyframe, XMFLOAT3 *Translation, XMFLOAT3 *Scale, XMFLOAT4 *RotationQuat, float* TimePos)
{
	m_pAnimation[nAnimation]->nSIze = nKeyframe;
	m_pAnimation[nAnimation]->Scale = Scale;
	m_pAnimation[nAnimation]->RotationQuat = RotationQuat;
	m_pAnimation[nAnimation]->Translation = Translation;
	m_pAnimation[nAnimation]->TimePos = TimePos;
}