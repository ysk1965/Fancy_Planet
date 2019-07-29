// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#define MSG_BOX(_message)	MessageBox(NULL, _message, L"System Message", MB_OK)

#define FAILED(hr)      (((HRESULT)(hr)) < 0)
#define MAXPLAYER 4

#define   BUF_SIZE            2048
#define   WM_SOCKET            WM_USER + 1

#define MAX_BUFF_SIZE   4096
#define MAX_PACKET_SIZE  255


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
#define CS_LOGIN	99

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
#define SC_LOAD_OBJECT 20							//객체,건물
#define SC_MOVE_OBJECT 21

#define SC_LOAD_STATIC_OBJECT 22
#define SC_TOUCH_OBJECT 23
#define SC_STEP_BASE 30								//점령
#define SC_OCCUPATION_COMPLETE 31

#define SC_GAME_RESULT 40

#define SC_CHARACTER_CHANGE 50
#define SC_TEAM_CHANGE  51
#define SC_LOGIN 99


#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DX_PI    ((FLOAT)  3.141592654f)
#define D3DX_1BYPI ((FLOAT)  0.318309886f)
#include <WinSock2.h>


#pragma comment(lib, "ws2_32.lib")

#include <windows.h>
#include <windowsx.h>
#include <PxPhysicsAPI.h>
#include <string>
#include <iostream>
#include <tchar.h>
#include <d3dcompiler.h>

#include <math.h>
#include <thread>
#include <vector>
#include <array>
#include <iostream>
#include <chrono>
#include <mutex>
using namespace std;

#pragma comment(lib, "PhysX3ExtensionsDEBUG.lib")
using namespace physx;
//피직스================================================//

#include <DirectXMath.h>
using namespace DirectX;

#define EVT_RECV 0
#define EVT_SEND 1
#define EVT_MOVE 2
#define EVT_OCCUPATION_BASE1 3
#define EVT_OCCUPATION_BASE2 4
#define EVT_OCCUPATION_BASE3 5
#define EVT_ATTACKED 6
#define EVT_PLAYER_DIE 7
#define EVT_BLUETeam_WIN 8
#define EVT_REDTeam_WIN 9
#define EVT_GAME_START 10
#define EVT_MOVE_SKILL_ON 11
#define EVT_MOVE_SKILL_OFF 12
#define EVT_RESPAWN_PLAYER 13
#define EVT_LOADING_START 14
#define EVT_MOVE_OBJECT 15
#define EVT_JUMP_OFF 16
#define EVT_GREENTeam_WIN 17
#define EVT_OCCUPATION_COMPLETE 18
#define EVT_ATTACK_SKILL_ON 19
#define EVT_ATTACK_SKILL_OFF 20
#define EVT_RAY_HIT_DYNAMIC_OBJECT 21

enum CHARACTER_TYPE
{
	SOLDIER,
	DRONE,
	CREATURE,
};

enum  EMUM_PLAYER_STATE
{
	ALIVED,
	DIED,
};

enum ENUM_KEYSTATE
{
	NONE,//idle
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	JUMPING,
};

enum ENUM_Team
{
	BLUETeam,
	REDTeam,
	GREENTeam,
	NEUTRAL,

};


enum ENUM_SCENE
{
	INGAMEROOM,
	INGAME,
	GAMERESULT,
	LOADING
};

enum class  CREATURE
{
	ANIMATION_IDLE_STATE = 0,
	ANIMATION_MOVE_FORWARD = 1,
	ANIMATION_MOVE_LEFT = 2,
	ANIMATION_MOVE_RIGHT = 3,
	ANIMATION_MOVE_BACKWARD = 4,
	ANIMATION_MOVE_RUN1 = 5,
	ANIMATION_MOVE_RUN2 = 6,
	ANIMATION_SHOT = 7,
	ANIMATION_SHOT2 = 8,
	ANIMATION_SHOT3 = 9,
	ANIMATION_HIT = 10,
	ANIMATION_DIED = 11,
	ANIMATION_MOVE_JUMP = 12,
	ANIMATION_MOVE_FORWARD_SHOT = 13,
	ANIMATION_MOVE_LEFT_SHOT = 14,
	ANIMATION_MOVE_RIGHT_SHOT = 15,
	ANIMATION_MOVE_BACKWARD_SHOT = 16,
	ANIMATION_MOVE_JUMP_SHOT = 17
};

enum class  DRONE
{
	ANIMATION_IDLE_STATE = 0,
	ANIMATION_MOVE_FORWARD = 1,
	ANIMATION_MOVE_LEFT = 2,
	ANIMATION_MOVE_RIGHT = 3,
	ANIMATION_MOVE_BACKWARD = 4,
	ANIMATION_HIT = 5,
	ANIMATION_SHOT = 6,
	ANIMATION_SHOT2 = 7,
	ANIMATION_DIED = 8,
	ANIMATION_SKILL = 9,
	ANIMATION_MOVE_FORWARD_SHOT = 10,
	ANIMATION_MOVE_LEFT_SHOT = 11,
	ANIMATION_MOVE_RIGHT_SHOT = 12,
	ANIMATION_MOVE_BACKWARD_SHOT = 13,
};
enum class SOLDIER {
	ANIMATION_IDLE_STATE = 0,
	ANIMATION_MOVE_FORWARD = 1,
	ANIMATION_MOVE_LEFT = 2,
	ANIMATION_MOVE_RIGHT = 3,
	ANIMATION_MOVE_BACKWARD = 4,
	ANIMATION_MOVE_RUN = 5,
	ANIMATION_USE = 6,
	ANIMATION_PICK_UP = 7,
	ANIMATION_DIED = 8,
	ANIMATION_SHOT = 9,
	ANIMATION_RELOAD = 10,
	ANIMATION_MOVE_FORWARD_SHOT = 11,
	ANIMATION_MOVE_LEFT_SHOT = 12,
	ANIMATION_MOVE_RIGHT_SHOT = 13,
	ANIMATION_MOVE_BACKWARD_SHOT = 14,
	ANIMATION_MOVE_RUN_SHOT = 15,
	ANIMATION_MOVE_JUMP = 15,
	ANIMATION_SKILL = 16,
	ANIMATION_MOVE_JUMP_SHOT = 17
};

struct EXOVER {
	WSAOVERLAPPED m_over;
	char m_iobuf[MAX_BUFF_SIZE];
	WSABUF m_wsabuf;
	int m_event;
	int m_object;
	XMFLOAT3 m_xmf3ExplosivePos;
};

class Client {
public:
	SOCKET m_s;
	bool m_isconnected;
	XMFLOAT4X4 m_pos;
	XMFLOAT4 m_quat;
	EXOVER m_rxover;
	int m_packet_size;  // 지금 조립하고 있는 패킷의 크기
	int	m_prev_packet_size; // 지난번 recv에서 완성되지 않아서 저장해 놓은 패킷의 앞부분의 크기

	char m_name[5];
	int m_iRoomNumb;
	int m_iTeam;
	int m_iCharacterType;
	int m_iHP;
	bool m_isAliveState = true;
	bool m_isReady;
	int m_scene_state;
	char m_packet[MAX_PACKET_SIZE];
	int m_animstate;
	float m_gravity;
	float m_fObjectInterpolationTime = 0.f;
	Client()
	{
		m_pos = {};
		m_gravity = 8.f;
		m_isReady = false;
		m_isconnected = false;
		m_iRoomNumb = 0;
		m_scene_state = INGAMEROOM;
		m_iHP = 100;
		

		ZeroMemory(&m_rxover.m_over, sizeof(WSAOVERLAPPED));
		m_rxover.m_wsabuf.buf = m_rxover.m_iobuf;
		m_rxover.m_wsabuf.len = sizeof(m_rxover.m_wsabuf.buf);
		m_rxover.m_event = EVT_RECV;
		m_prev_packet_size = 0;
	}
};
class ROOM
{
public:
	int m_iRoomNumber=0;
	int m_iUser=0;
	int m_iReadyUser=0;
	int m_iLoadingCompleteUser=0;
	int m_iState=0;

	void init()
	{
		m_iReadyUser=0;
		m_iLoadingCompleteUser=0;
		m_iState = INGAMEROOM;
	}


};
class BASE
{
public:
	char name[9];
	int m_iState = 0; // 중립,블루팀,레드팀 점령지.
	int m_iOccupiedState = NEUTRAL; //아직 점령중이라면 누가 점령중인지. 
	int m_iStepCnt = 0; // 밟고 있는 여부
	float m_fPriorStepTime = 0.f;
	float m_fStepTime = 0.f;
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Size;

	void Initialize()
	{ 
		m_iState = NEUTRAL; // 중립,블루팀,레드팀 점령지.
		m_iOccupiedState = NEUTRAL; //아직 점령중이라면 누가 점령중인지. 
		m_fStepTime = 0.f;
	}
};
class TEAM
{
public:
	int m_iTeam = 0;
	int m_iKill = 0;
	int m_iNumberOfterritories = 0;
	bool m_isWin = false;
	void Initialize()
	{
		m_iTeam = 0;
		m_iKill = 0;
		m_iNumberOfterritories = 0;
		m_isWin = false;
	}

};



#define ANIMATION_MOVE_BACKWARD 4
#define ANIMATION_IDLE_STATE 0
#define ANIMATION_MOVE_FORWARD 1
#define ANIMATION_MOVE_LEFT 2
#define ANIMATION_MOVE_RIGHT 3
#define ANIMATION_SHOT 9
#define ANIMATION_MOVE_JUMP 11
#define ANIMATION_DIED 8
#define ANIMATION_MOVE_DANCE 12


namespace Vector3
{
	/*inline bool IsZero(XMFLOAT3& xmf3Vector)
	{
	if (::IsZero(xmf3Vector.x) && ::IsZero(xmf3Vector.y) && ::IsZero(xmf3Vector.z)) return(true);
	return(false);
	}*/

	inline XMFLOAT3 XMVectorToFloat3(XMVECTOR& xmvVector)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, xmvVector);
		return(xmf3Result);
	}

	inline XMFLOAT3 ScalarProduct(XMFLOAT3& xmf3Vector, float fScalar, bool bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)) * fScalar);
		else
			XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector) * fScalar);
		return(xmf3Result);
	}

	inline XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + XMLoadFloat3(&xmf3Vector2));
		return(xmf3Result);
	}

	inline XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) + (XMLoadFloat3(&xmf3Vector2) * fScalar));
		return(xmf3Result);
	}

	inline XMFLOAT3 Subtract(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMLoadFloat3(&xmf3Vector1) - XMLoadFloat3(&xmf3Vector2));
		return(xmf3Result);
	}

	inline float DotProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Dot(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
		return(xmf3Result.x);
	}

	inline XMFLOAT3 CrossProduct(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, bool bNormalize = true)
	{
		XMFLOAT3 xmf3Result;
		if (bNormalize)
			XMStoreFloat3(&xmf3Result, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2))));
		else
			XMStoreFloat3(&xmf3Result, XMVector3Cross(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
		return(xmf3Result);
	}

	inline XMFLOAT3 Normalize(XMFLOAT3& xmf3Vector)
	{
		XMFLOAT3 m_xmf3Normal;
		XMStoreFloat3(&m_xmf3Normal, XMVector3Normalize(XMLoadFloat3(&xmf3Vector)));
		return(m_xmf3Normal);
	}

	inline float Length(XMFLOAT3& xmf3Vector)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3Length(XMLoadFloat3(&xmf3Vector)));
		return(xmf3Result.x);
	}

	inline float Angle(XMVECTOR& xmvVector1, XMVECTOR& xmvVector2)
	{
		XMVECTOR xmvAngle = XMVector3AngleBetweenNormals(xmvVector1, xmvVector2);
		return(XMConvertToDegrees(acosf(XMVectorGetX(xmvAngle))));
	}

	inline float Angle(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2)
	{
		return(Angle(XMLoadFloat3(&xmf3Vector1), XMLoadFloat3(&xmf3Vector2)));
	}

	inline XMFLOAT3 TransformNormal(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3TransformNormal(XMLoadFloat3(&xmf3Vector), xmmtxTransform));
		return(xmf3Result);
	}

	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMMATRIX& xmmtxTransform)
	{
		XMFLOAT3 xmf3Result;
		XMStoreFloat3(&xmf3Result, XMVector3TransformCoord(XMLoadFloat3(&xmf3Vector), xmmtxTransform));
		return(xmf3Result);
	}

	inline XMFLOAT3 TransformCoord(XMFLOAT3& xmf3Vector, XMFLOAT4X4& xmmtx4x4Matrix)
	{
		return(TransformCoord(xmf3Vector, XMLoadFloat4x4(&xmmtx4x4Matrix)));
	}
}
namespace Matrix4x4
{
	inline XMFLOAT4X4 Identity()
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixIdentity());
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Multiply(XMFLOAT4X4& xmmtx4x4Matrix1, XMMATRIX& xmmtxMatrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMLoadFloat4x4(&xmmtx4x4Matrix1) * xmmtxMatrix2);
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Multiply(XMMATRIX& xmmtxMatrix1, XMFLOAT4X4& xmmtx4x4Matrix2)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, xmmtxMatrix1 * XMLoadFloat4x4(&xmmtx4x4Matrix2));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Inverse(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixInverse(NULL, XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 Transpose(XMFLOAT4X4& xmmtx4x4Matrix)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixTranspose(XMLoadFloat4x4(&xmmtx4x4Matrix)));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));
		return(xmmtx4x4Result);
	}

	inline XMFLOAT4X4 LookAtLH(XMFLOAT3& xmf3EyePosition, XMFLOAT3& xmf3LookAtPosition, XMFLOAT3& xmf3UpDirection)
	{
		XMFLOAT4X4 xmmtx4x4Result;
		XMStoreFloat4x4(&xmmtx4x4Result, XMMatrixLookAtLH(XMLoadFloat3(&xmf3EyePosition), XMLoadFloat3(&xmf3LookAtPosition), XMLoadFloat3(&xmf3UpDirection)));
		return(xmmtx4x4Result);
	}
}