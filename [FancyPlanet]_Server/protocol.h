#pragma once
#include"stdafx.h"
#include "PhysXFramework.h"
class CPhysXFramework;




#pragma pack (push, 1)
struct cs_loading_complete
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
};
struct cs_packet_pos {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned short key;
	int keyInputState;
};
struct sc_packet_jump_off {
	unsigned char size;
	unsigned char type;
	unsigned short id;
};
struct cs_packet_ready {
	unsigned char size;
	unsigned char type;
	int roomnumb = 0;
	bool state;
};


struct sc_packet_pos {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned short key;
	int keyInputState;
	XMFLOAT3 m_xmf3Position;
	XMFLOAT4 m_xmf4Quaternion;

};

struct cs_packet_rotate {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	XMFLOAT3 xmvLook;
	XMFLOAT3 xmvRight;
	XMFLOAT3 xmvUP;
};
struct sc_packet_rotate {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	XMFLOAT4 m_xmf4Quaternion;
};
struct cs_packet_jump {
	unsigned char size;
	unsigned char type;
	unsigned short id;
};

struct sc_packet_put_player {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned short m_characterType;
	XMFLOAT4X4 m_pos;
	int m_iTeam;
	char name[10];
};

struct cs_packet_move_skill
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char charactertype;
};
struct sc_packet_move_skill
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char charactertype;
};
struct sc_packet_remove_player {
	unsigned char size;
	unsigned char type;
	unsigned short id;

};
struct sc_packet_ready {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	bool isReadyState;
};

struct cs_packet_shot {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	XMFLOAT3 m_xmf3CameraLookAt;
};
struct sc_packet_shot {
	unsigned char size;
	unsigned char type;
	unsigned short id;
};
struct cs_packet_attack_skill
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char charactertype;
};
struct sc_packet_attack_skill
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char charactertype;
	XMFLOAT3 m_xmf3ExplosivePos;
};
struct sc_packet_scene_change
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	int scenestate = 0;

};

struct sc_packet_time
{
	unsigned char size;
	unsigned char type;
	float m_ftime = 0;
};

struct sc_gravity_change
{
	unsigned char size;
	unsigned char type;
	float gravity_state;
};

struct cs_gravity_test
{
	unsigned char size;
	unsigned char type;
	float gravity_state;
};

struct cs_packet_attacked
{
	unsigned char size;
	unsigned char type;
	unsigned char id;
};
struct sc_packet_attacked
{
	unsigned char size;
	unsigned char type;
	unsigned char id;
	char hp;
	XMFLOAT3 m_xmfPos;
};
struct cs_packet_character_state
{
	unsigned char size;
	unsigned char type;
	unsigned char id;
	unsigned char state;
};
struct sc_packet_character_state
{
	unsigned char size;
	unsigned char type;
	unsigned char id;
	unsigned char slayer_id;
	unsigned char state;
};

///////////////////////////로그인 패킷////////////////////////
struct sc_packet_login {
	BYTE size;
	BYTE type;
	WORD id;
	long x;
	long y;
	wchar_t name[10];
};
struct cs_packet_login {
	BYTE size;
	BYTE type;
	WORD id;
	wchar_t name[10];
};


////////////////////////////오브젝트//////////////////////////////
struct cs_packet_requestObject
{
	unsigned char size;
	unsigned char type;
};

struct sc_packet_loadObject
{
	unsigned char size;
	unsigned char type;
	unsigned char m_ObjectID;
	XMFLOAT3 m_xmf3ObjectPosition;
	XMFLOAT4 m_xmf4ObjectQuaternion;
};



struct sc_packet_updateObject
{
	unsigned char size;
	unsigned char type;
	unsigned char m_ObjectID;
	XMFLOAT3 m_xmf3ObjectPosition;
	XMFLOAT4 m_xmf4ObjectQuaternion;
};
/////////////////게임 프로세스///////////
struct sc_packet_characterChange
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char m_iCharacterType;
};
struct cs_packet_characterChange
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char m_iCharacterType;
};
struct sc_packet_teamChange
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char m_iTeamToChange;
};
struct cs_packet_teamChange
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char m_iTeamToChange;
};
struct sc_packet_gameResult
{
	unsigned char size;
	unsigned char type;
	unsigned char m_iWinTeam;

};

//////////////////점령전/////////////////////
struct sc_packet_loadStaticObject
{
	unsigned char size;
	unsigned char type;
	unsigned short m_iMyID;
	unsigned char m_iTeam;
	unsigned char m_iCharacter;
	char m_ObjectID[9];
	unsigned char m_iOccupationState;
	XMFLOAT3 m_xmf3ObjectPosition;
};

struct sc_packet_stepBase
{
	unsigned char size;
	unsigned char type;
	unsigned char m_iTeam;
	unsigned char m_iOccupiedBase;
	float m_fPercentOfOccupation;
};
struct sc_occupation_complete
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char m_iOccupiedBase;

};

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;



// the blitter object structure BOB
typedef struct PLAYER_INFO
{
	char m_name[10];
	int state;          // 상태
	char anim_state;     // 애니메이션 상태
	int attr;           // 객체 속성(종류)
	XMFLOAT4X4 pos;          // 포지션값
	int xv, yv;        // 속도값
	float scale, rotation;

	WCHAR message[256]; //채팅 정보
	DWORD message_time;

	bool m_isconnected = false;
	bool m_isready = false;
	int roomnumb = 0;
	int m_scene = 0;
	int hp;
	PLAYER_INFO()
	{
		hp = 100;
		pos = {};
		pos._11 = 10.f;
		pos._22 = 10.f;
		pos._33 = 10.f;
		pos._44 = 1.f;
	}
} PLAYER_INFO, *PLAYER_INFO_PTR;

#pragma pack (pop)