#pragma once
#include"stdafx.h"

class CPhysXFramework;

#define   BUF_SIZE            1024
#define   WM_SOCKET            WM_USER + 1

#define MAX_BUFF_SIZE   4096
#define MAX_PACKET_SIZE  255



#define MAX_USER 10
#define MY_SERVER_PORT  4000
#define MAX_STR_SIZE  100
#define NAME_LEN 10 //로그인 ID 사이즈
#define CS_POS    1
#define CS_READY  2
#define CS_ROTATE  3
#define CS_RIGHT    4
#define CS_CHAT      5
#define CS_PLAYER_STATE_CHANGE 6
#define CS_GRAVITY_TEST 7
#define CS_ATTACKED 9
#define CS_SHOT      10

#define CS_REQUEST_OBJECT 20
#define CS_MOVE_OBJECT 21

#define CS_LOGIN	99

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_READY      4
#define SC_SCENE_CHANGE 5
#define SC_PLAYER_STATE_CHANGE 6
#define SC_GRAVITY_CHANGE 7
#define SC_ROTATE 8
#define SC_TIME 11
#define SC_ATTACKED 9
#define SC_SHOT   10

#define SC_LOAD_OBJECT 20
#define SC_MOVE_OBJECT 21

#define SC_LOGIN 99




#pragma pack (push, 1)

struct cs_packet_pos {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned short key;
	int keyInputState;
};

struct sc_packet_pos {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned short key;
	int keyInputState;
	XMFLOAT4X3 m_pos;
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
	XMFLOAT4X3 m_pos;
};
struct cs_packet_ready {
	unsigned char size;
	unsigned char type;
	int roomnumb = 0;
	bool state;
};




struct sc_packet_put_player {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	char name[10];
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
};
struct sc_packet_shot {
	unsigned char size;
	unsigned char type;
	unsigned short id;
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
	unsigned char hp;
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

///////////////////////오브젝트 패킷/////////////////////////
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
};

struct cs_packet_loadObject
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	//XMFLOAT3 m_xmf3ObjectPosition[OBJECTS_NUMBER];
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