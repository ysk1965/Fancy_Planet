#pragma once
#include"stdafx.h"
#include"protocol.h"
class BASE
{
public:
	
protected:
	char name[9];
	int m_iState = 0; // �߸�,�����,������ ������.
	int m_iOccupiedState = NEUTRAL; //���� �������̶�� ���� ����������. 
	float m_fStepTime;
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Size;
};

class INGAMEBASE : public BASE
{
public:
	SetName
};