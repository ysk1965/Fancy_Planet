#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

#define	BUF_SIZE				1024
#define	WM_SOCKET				WM_USER + 1

#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255

#define BOARD_WIDTH   8
#define BOARD_HEIGHT  8

#define VIEW_RADIUS   3

#define MAX_USER 10

#define NPC_START  1000
#define NUM_OF_NPC  10000

#define MY_SERVER_PORT  4000

#define MAX_STR_SIZE  100

#define CS_POS    1
#define CS_READY  2
#define CS_LEFT  3
#define CS_RIGHT    4
#define CS_CHAT		5

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_READY		4
#define SC_SCENE_CHANGE 5

#pragma pack (push, 1)

struct cs_packet_pos {
	unsigned char size;
	unsigned char type;
	XMFLOAT4X4 m_pos;
	int roomnumb = 0;

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
	int roomnumb = 0;
	wchar_t message[MAX_STR_SIZE];
};

struct sc_packet_pos {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	int animstate = 0;
	int roomnumb = 0;
	XMFLOAT4X4 m_pos;
};

struct sc_packet_put_player {
	unsigned char size;
	unsigned char type;
	unsigned short id;

	int roomnumb;
	XMFLOAT4X4 m_pos;
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
	int roomnumb = 0;
	bool state;
};

struct sc_packet_chat {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	int roomnumb = 0;
	wchar_t message[MAX_STR_SIZE];
};
struct sc_packet_scene_change
{
	unsigned char size;
	unsigned char type;
	unsigned short id;
	int roomnumb = 0;
	int scenestate = 0;

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
	int anim_state;     // 애니메이션 상태
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

} PLAYER_INFO, *PLAYER_INFO_PTR;

#pragma pack (pop)