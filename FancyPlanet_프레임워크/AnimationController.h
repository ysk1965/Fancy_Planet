#pragma once

enum
{
	IDLE,
	WALK,
	RUN,
	ATTACK
};
struct ANIMATION
{
	UINT nSIze;
	float *TimePos;
	XMFLOAT3 *Translation;
	XMFLOAT3 *Scale;
	XMFLOAT4 *RotationQuat;
};
class AnimationController
{
private:
	UINT m_nAnimation = 0;
	UINT m_nState = IDLE;
	UINT m_nCurrentFrame;

	ANIMATION **m_pAnimation;
public:
	AnimationController(UINT nAnimation);
	~AnimationController();
	void ChangeAnimation(UINT nState);
	void AddAnimation(UINT nAnimation, UINT nKeyframe, XMFLOAT3 *Translation, XMFLOAT3 *Scale, XMFLOAT4 *RotationQuat, float* TimePos);
};

