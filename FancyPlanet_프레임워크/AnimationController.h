#pragma once

enum
{
	IDLE,
	WALK,
	RUN,
	ATTACK
};
struct FRAME
{
	XMFLOAT3 Translation;
	XMFLOAT4 RotationQuat;
};
struct ANIMATION
{
	UINT nTime;
	FRAME* pFrame;
};
class AnimationController
{
private:
	UINT m_nAnimation = 0;
	UINT m_nState = IDLE;
	UINT m_nCurrentFrame;

public:
	UINT GetAnimationCount()
	{
		return m_nAnimation;
	}
	AnimationController(UINT nAnimation);
	~AnimationController();
	//void ChangeAnimation(UINT nState);
	//void AddAnimation(UINT nAnimation, UINT nKeyframe, XMFLOAT3 *Translation, XMFLOAT3 *Scale, XMFLOAT4 *RotationQuat, float* TimePos);
	
	ANIMATION *m_pAnimation;
};

