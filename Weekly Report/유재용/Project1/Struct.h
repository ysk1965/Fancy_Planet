#pragma once
#include"Header.h"
#include "Enum.h"
#include <d3d12.h>

struct HEADER //recv send �Լ����� � ������ ������������ �� �������� ũ�⸦ �˷��ֱ� ���� ��Ŷ�� ���.
{
	int type; 
	int size;
};

struct LOGIN_REQUEST_PACKET : public HEADER //���� ���� �α����� �ϰ� ���̵� ���õ�.
{
	LOGIN_REQUEST_PACKET()
	{
		type = LOGIN_REQUEST;
		size = sizeof(*this);
	}
	char ID[ID_LEN];
};

struct PLAYERINFO //�α��� �Ŀ� ����,���� �� �÷��̾��� ����.
{
	char ID[ID_LEN];
	bool isReady = false;// ������¸� ����
	int gameState = 0; // enum������ Ŭ���̾�Ʈ �� ���°� ǥ��
	int Character = 0; // enum������ � ĳ���͸� �����ߴ��� ǥ��
};

struct CHARACTER : public PLAYERINFO //�ΰ��ӿ��� ����� �÷��̾��� ĳ����
{
	int CharacterState = 0; //enum������ ĳ������ ���¸� ǥ��
	int xPos;
	int yPos; //���̷�ƮX12 ������(XMFLOAT3)���� ��ȯ �ʿ�
	int zPos;
};

struct CHARACTER_MOVE_PACKET :public HEADER
{	
	int xPos;
	int yPos;
	int zPos;
	CHARACTER_MOVE_PACKET(CHARACTER &a)
	{
		type = CHARACTER_MOVE;
		size = sizeof(*this);
		xPos = a.xPos;
		yPos = a.yPos;
		zPos = a.zPos;
	}
};