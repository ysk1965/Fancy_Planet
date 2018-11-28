#pragma once

#define FRAME_BUFFER_WIDTH      1024
#define FRAME_BUFFER_HEIGHT     768

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
#include "FmodSound.h"
#include "ComputShader.h"
#include <chrono>
enum
{
	NUM_SUBSETS = 4,
	NUM_COMMANDLIST = 4
};

enum
{
	TERRAIN,
	EFFECT,
	CHARACTER,
	OBJECT
};

enum class BACKSOUND
{
	BACKGROUND_ROBBY,
	BACKGROUND_INGAME,
	BACKGROUND_END,
};

enum class EFFECTSOUND
{
	ROBBY_CLICK,
	ROBBY_CHANGECHARACTER,
	HUMAN_SHOT,
	HUMAN_DIED,
	HUMAN_JUMP,
	HUMANSKILL_1,
	HUMANSKILL_2,
	DRONE_SHOT,
	DRONE_DIED,
	DRONE_JUMP,
	DRONESKILL_1,
	DRONESKILL_2,
	CREATURE_SHOT,
	CREATURE_DIED,
	CREATURE_JUMP,
	CREATURESKILL_1,
	CREATURESKILL_2,
	INGAME_CHARGE,
	INGAME_COMPLETE
};

using namespace std;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool GetLobbyState()
	{
		return m_nIsLobby;
	}
	void SendBulletPacket();

	void BuildLobby();
	void BuildLightsAndMaterials();
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChainRenderTargetViews();
	void CreateLightsAndMaterialsShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	void OnResizeBackBuffers();

	void BuildThreadsAndEvents();
	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();
	void FrameAdvanceInLobby();
	void FrameAdvanceInEnd();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void RenderSubset(int iIndex);
	void NotifyIdleState();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void UpdateLightsAndMaterialsShaderVariables();

	XMFLOAT4X4& GetPlayerMatrix()
	{
		return m_pPlayer->GetPlayerWorldTransform();
	};
	void PrepareFrame();
private:
	ID3D12GraphicsCommandList * m_pd3dScreenCommandList;
	ID3D12CommandAllocator      *m_pd3dScreenCommandAllocator;
	ID3D12GraphicsCommandList * m_pd3dPreShadowCommandList;
	ID3D12CommandAllocator      *m_pd3dPreShadowCommandAllocator;

	CComputShader* m_pComputeShader = NULL;

	UIShader* m_pUIShader = NULL;

	CShadowShader* m_pShadowShader = NULL;

	UINT m_nIsLobby;

	HINSTANCE               m_hInstance;
	HWND                  m_hWnd;

	int                     m_nWndClientWidth;
	int                     m_nWndClientHeight;

	int						m_nCharacterType = 0;
	int						m_iSceneState = INGAMEROOM;
	XMFLOAT3			xmf3PickDirection;

	IDXGIFactory4            *m_pdxgiFactory = NULL;
	IDXGISwapChain3            *m_pdxgiSwapChain = NULL;
	ID3D12Device            *m_pd3dDevice = NULL;

	bool                  m_bMsaa4xEnable = false;
	UINT                  m_nMsaa4xQualityLevels = 0;

	static const UINT            m_nSwapChainBuffers = 2;
	UINT                     m_nSwapChainBufferIndex;

	D3D12_CPU_DESCRIPTOR_HANDLE      m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE      m_d3dDsvDepthStencilBufferCPUHandle;

	ID3D12Resource            *m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap      *m_pd3dRtvDescriptorHeap = NULL;
	UINT                  m_nRtvDescriptorIncrementSize;

	ID3D12Resource            *m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap      *m_pd3dDsvDescriptorHeap = NULL;
	UINT                  m_nDsvDescriptorIncrementSize;

	ID3D12CommandAllocator      **m_ppd3dCommandAllocators;
	ID3D12GraphicsCommandList   **m_ppd3dCommandLists;
	ID3D12CommandAllocator      **m_ppd3dShadowCommandAllocators;
	ID3D12GraphicsCommandList   **m_ppd3dShadowCommandLists;
	ID3D12CommandQueue         *m_pd3dCommandQueue = NULL;

	ID3D12Fence               *m_pd3dFence = NULL;
	UINT64                  m_nFenceValues[m_nSwapChainBuffers];
	HANDLE                  m_hFenceEvent;
	bool m_bIsVictory = true;
	bool m_bReady = false;
#if defined(_DEBUG)
	ID3D12Debug               *m_pd3dDebugController;
#endif
	CGameTimer               m_GameTimer;

	CScene                  **m_ppScenes = NULL;
	CPlayer                  *m_pPlayer = NULL;
	CCamera                  *m_pCamera = NULL;

	POINT                  m_ptOldCursorPos;

	_TCHAR                  m_pszFrameRate[50];
	static CGameFramework* Get()
	{
		return  m_pGFforMultiThreads;
	}

	struct ThreadParameter
	{
		int threadIndex;
	};
	ThreadParameter m_threadParameters[NUM_SUBSETS];

	HANDLE m_workerBeginRenderFrame[NUM_SUBSETS];
	HANDLE m_workerFinishedRenderFrame[NUM_SUBSETS];
	HANDLE m_workerFinishShadowPass[NUM_SUBSETS];
	HANDLE m_threadHandles[NUM_SUBSETS];

	static CGameFramework* m_pGFforMultiThreads;

	// FMOD
	CFmodSound m_FmodSound;


	//서버
private:

	SOCKET g_mysocket;
	WSABUF   send_wsabuf;
	char    send_buffer[BUF_SIZE];
	WSABUF   recv_wsabuf;
	char   recv_buffer[BUF_SIZE];
	char   packet_buffer[BUF_SIZE];
	DWORD      in_packet_size = 0;
	int      saved_packet_size = 0;
	int      g_myid; // 내 아이디
	PLAYER_INFO g_my_info;
	array <PLAYER_INFO, MAX_USER> g_player_info;
	std::chrono::system_clock::time_point m_bulletstart;

	bool keystate = false;
	std::chrono::system_clock::time_point idlestatepoint;
	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point mousestart;
	bool istime = false;
	XMFLOAT3 m_xmf3PrePosition;
	float m_fgametime;// 게임시간.
	float m_fgravity; //중력 세기.
	XMFLOAT3 m_xmf3ObjectsPos[OBJECTS_NUMBER];
	XMFLOAT4 m_xmf4ObjectsQuaternion[OBJECTS_NUMBER];
	bool m_isMoveInput = false;
	bool m_isMovePacketRecv = false;
	ID3D12Resource				*m_pd3dcbLights = NULL;
	LIGHTS						*m_pcbMappedLights = NULL;

	ID3D12Resource				*m_pd3dcbMaterials = NULL;
	MATERIALS					*m_pcbMappedMaterials = NULL;

	LIGHTS * m_pLights = NULL;

	MATERIALS					*m_pMaterials = NULL;
	int							m_nMaterials = 0;
public:
	void ProcessPacket(char *ptr);
	void ReadPacket(SOCKET sock);
	void InitNetwork(HWND main_window);

	TEAM Team[Team_NUMBER];
	BASE Base[STATION_NUMBER];
};
