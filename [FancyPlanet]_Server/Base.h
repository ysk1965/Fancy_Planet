#pragma once
#include"stdafx.h"
#include"protocol.h"
class BASE
{
public:
	
protected:
	char name[9];
	int m_iState = 0; // 중립,블루팀,레드팀 점령지.
	int m_iOccupiedState = NEUTRAL; //아직 점령중이라면 누가 점령중인지. 
	float m_fStepTime;
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Size;
};

class INGAMEBASE : public BASE
{
public:
	SetName
};