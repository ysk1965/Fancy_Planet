#pragma once
#include"stdafx.h"

class CPhysXFramework;

#define   BUF_SIZE            2048
#define   WM_SOCKET            WM_USER + 1

#define MAX_BUFF_SIZE   4096
#define MAX_PACKET_SIZE  255


#define MAX_BUFF_SIZE   2048
#define MAX_PACKET_SIZE  255
#define OBJECTS_NUMBER 30
#define STATION_NUMBER 3//점령지 수
#define Team_NUMBER 3//팀 수
#define MAX_USER 10
#define MY_SERVER_PORT  4000
#define MAX_STR_SIZE  100
#define NAME_LEN 10 //로그인 ID 사이즈
#define GAMETIME 180
#define CS_POS    1
#define CS_READY  2
#define CS_ROTATE  3
#define CS_RIGHT    4
#define CS_JUMP      5
#define CS_PLAYER_STATE_CHANGE 6
#define CS_GRAVITY_TEST 7
#define CS_ATTACKED 9
#define CS_SHOT      10
#define CS_MOVE_SKILL 12
#define CS_ATTACK_SKILL 13
#define CS_REQUEST_OBJECT 20
#define CS_CHARACTER_CHANGE 50
#define CS_TEAM_CHANGE  51
#define CS_LOADING_COMPLETE 98
#define CS_LOGIN   99

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_READY      4
#define SC_SCENE_CHANGE 5
#define SC_PLAYER_STATE_CHANGE 6
#define SC_GRAVITY_CHANGE 7
#define SC_ROTATE 8
#define SC_JUMP 9
#define SC_TIME 11
#define SC_ATTACKED 13
#define SC_ATTACK_SKILL 17
#define SC_SHOT   10
#define SC_MOVE_SKILL_ON 14
#define SC_MOVE_SKILL_OFF 15
#define SC_JUMP_OFF 16
#define SC_LOAD_OBJECT 20                     //객체,건물
#define SC_MOVE_OBJECT 21

#define SC_LOAD_STATIC_OBJECT 22
#define SC_TOUCH_OBJECT 23
#define SC_STEP_BASE 30                        //점령
#define SC_OCCUPATION_COMPLETE 31

#define SC_GAME_RESULT 40

#define SC_CHARACTER_CHANGE 50
#define SC_TEAM_CHANGE  51
#define SC_LOGIN 99





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



///////////////////////////캐릭터 저장 구조체////////////////////////////////////////////

// the blitter object structure BOB
typedef struct PLAYER_INFO
{
	int state;          // 상태
	char anim_state = 0;     // 애니메이션 상태
	char move_state = 0;
	int m_iCharacterType;           // 객체 속성(종류)
	XMFLOAT4X4 pos;          // 포지션값
	XMFLOAT4 quat;
	int xv, yv;        // 속도값
	float scale, rotation;

	WCHAR message[256]; //채팅 정보
	DWORD message_time;

	bool m_isconnected = false;
	bool m_isReady = false;
	int m_iRoomNumb = 0;
	int m_scene = 0;
	int m_iTeam;
	int m_iCharacterIndex = 0;
	int m_iHP;
	bool m_isMoveSKillState;
	bool m_isJumpState;
	bool m_isAttackState;
	int Character_idx = 0;
	PLAYER_INFO()
	{
		m_iHP = 100;
		m_iTeam = NEUTRAL;
		m_isReady = false;
		m_isMoveSKillState = false;
		m_isconnected = false;
		pos._11 = 10.f;
		pos._12 = 0.f;
		pos._13 = 0.f;
		pos._14 = 0.f;
		pos._21 = 0.f;
		pos._22 = 10.f;
		pos._23 = 0.f;
		pos._24 = 0.f;
		pos._31 = 0.f;
		pos._32 = 0.f;
		pos._33 = 10.f;
		pos._34 = 0.f;
		pos._44 = 1.f;
	}
} PLAYER_INFO, *PLAYER_INFO_PTR;

#pragma pack (pop)