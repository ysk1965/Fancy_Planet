#pragma once
#include"Header.h"
#include "Enum.h"
#include <d3d12.h>

struct HEADER //recv send 함수에게 어떤 종류의 데이터인지와 그 데이터의 크기를 알려주기 위한 패킷의 헤더.
{
	int type; 
	int size;
};

struct LOGIN_REQUEST_PACKET : public HEADER //가장 먼저 로그인을 하고 아이디가 세팅됨.
{
	LOGIN_REQUEST_PACKET()
	{
		type = LOGIN_REQUEST;
		size = sizeof(*this);
	}
	char ID[ID_LEN];
};

struct PLAYERINFO //로그인 후에 생성,저장 될 플레이어의 정보.
{
	char ID[ID_LEN];
	bool isReady = false;// 레디상태를 설정
	int gameState = 0; // enum형으로 클라이언트 씬 상태값 표현
	int Character = 0; // enum형으로 어떤 캐릭터를 선택했는지 표현
};

struct CHARACTER : public PLAYERINFO //인게임에서 사용할 플레이어의 캐릭터
{
	int CharacterState = 0; //enum형으로 캐릭터의 상태를 표현
	int xPos;
	int yPos; //다이렉트X12 벡터형(XMFLOAT3)으로 변환 필요
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