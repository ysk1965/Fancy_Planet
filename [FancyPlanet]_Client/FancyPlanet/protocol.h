#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <iostream>
using namespace std;
using namespace DirectX;


enum  PLAYER_STATE
{
	STAND,
	DIED,
};
#define   BUF_SIZE            1024
#define   WM_SOCKET            WM_USER + 1

#define MAX_BUFF_SIZE   1024
#define MAX_PACKET_SIZE  255



#define MAX_USER 10



#define MY_SERVER_PORT  4000

#define MAX_STR_SIZE  100

#define CS_POS    1
#define CS_READY  2
#define CS_LEFT  3
#define CS_RIGHT    4
#define CS_CHAT      5
#define CS_PLAYER_STATE_CHANGE 6
#define CS_GRAVITY_TEST 7
#define CS_ATTACKED 9
#define CS_SHOT      10

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_READY      4
#define SC_SCENE_CHANGE 5
#define SC_PLAYER_STATE_CHANGE 6
#define SC_GRAVITY_CHANGE 7
#define SC_TIME 11
#define SC_ATTACKED 9
#define SC_SHOT   10

#pragma pack (push, 1)

struct cs_packet_pos {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	char animstate = 0;

	XMFLOAT4X3 m_pos;
};

struct cs_packet_ready {
	unsigned char size;
	unsigned char type;
	int roomnumb = 0;
	bool state;
};

struct cs_packet_chat {
	unsigned char size;
	unsigned char type;
	wchar_t message[MAX_STR_SIZE];
};

struct sc_packet_pos {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	char animstate = 0;
	XMFLOAT4X3 m_pos;
};

struct sc_packet_put_player {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	char animstate;
	unsigned char roomnumb;
	float m_gravity;
	XMFLOAT4X3 m_pos;
};
struct sc_packet_remove_player {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	int roomnumb = 0;
};
struct sc_packet_ready {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	bool state;
};

struct sc_packet_chat {
	unsigned char size;
	unsigned char type;
	unsigned short id;

	wchar_t message[MAX_STR_SIZE];
};
struct cs_packet_shot {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char roomnumb;
	char animstate;
	pair<XMFLOAT3, XMFLOAT3> getprojectile;
};
struct sc_packet_shot {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	unsigned char roomnumb;
	char animstate;
	pair<XMFLOAT3, XMFLOAT3> getprojectile;
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
		pos = {};
		pos._11 = 10.f;
		pos._22 = 10.f;
		pos._33 = 10.f;
		pos._44 = 1.f;
	}
} PLAYER_INFO, *PLAYER_INFO_PTR;

#pragma pack (pop)