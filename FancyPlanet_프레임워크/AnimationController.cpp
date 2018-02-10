#include "stdafx.h"
#include "AnimationController.h"


AnimationController::AnimationController(UINT nAnimation) : m_nAnimation(nAnimation)
{
	m_pAnimation = new ANIMATION[m_nAnimation];
}


AnimationController::~AnimationController()
{
}

