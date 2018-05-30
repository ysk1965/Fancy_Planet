#pragma once

#define FRAME_BUFFER_WIDTH      1024
#define FRAME_BUFFER_HEIGHT      768

#include "Timer.h"
#include "Player.h"
#include "Scene.h"
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
	PHYSX,
	CHARACTER,
	OBJECT
};
using namespace std;

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChainRenderTargetViews();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();
	void CreateUIShader();

	void OnResizeBackBuffers();

	void BuildThreadsAndEvents();
	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void SpaceDivision();
	void RenderSubset(int iIndex);
	void RenderUI();
	void NotifyIdleState();
	void CollisionCheckByBullet();
	void SendBulletPacket(UINT nShell);

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void CalculatePickRay(const XMFLOAT2& xmf2MousePos);

	XMFLOAT4X4& GetPlayerMatrix()
	{
		return m_pPlayer->GetPlayerWorldTransform();
	};
	void PrepareFrame();
private:
	ID3D12GraphicsCommandList * m_pd3dScreenCommandList;

	CComputShader* m_pComputeShader = NULL;

	CTextureToFullScreenShader *m_pScreenShader = NULL;

	UIShader* m_pUIShader = NULL;

	ID3D12CommandAllocator      *m_pd3dScreenCommandAllocator;

	HINSTANCE               m_hInstance;
	HWND                  m_hWnd;

	int                     m_nWndClientWidth;
	int                     m_nWndClientHeight;

	XMFLOAT3			xmf3PickDirection;

	IDXGIFactory4            *m_pdxgiFactory = NULL;
	IDXGISwapChain3            *m_pdxgiSwapChain = NULL;
	ID3D12Device            *m_pd3dDevice = NULL;

	int                     m_nDivision = 0;
	XMFLOAT3				 pickRayDirection;

	bool                  **m_ppbDivision = NULL;
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
	ID3D12CommandQueue         *m_pd3dCommandQueue = NULL;

	ID3D12Fence               *m_pd3dFence = NULL;
	UINT64                  m_nFenceValues[m_nSwapChainBuffers];
	HANDLE                  m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug               *m_pd3dDebugController;
#endif

	static const UINT            m_nRenderTargetBuffers = 4;
	ID3D12Resource               *m_ppd3dRenderTargetBuffers[m_nRenderTargetBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE      m_pd3dRtvRenderTargetBufferCPUHandles[m_nRenderTargetBuffers];

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
	HANDLE m_threadHandles[NUM_SUBSETS];

	static CGameFramework* m_pGFforMultiThreads;

	//Physx SDK Member Variables =========================
	PxPhysics*                  m_pPxPhysicsSDK;
	PxScene*                  m_pPxScene;
	PxMaterial*                  m_pPxMaterial;
	PxControllerManager*         m_pPxControllerManager;
	PxFoundation*               m_pPxFoundation;
	PxDefaultErrorCallback         m_PxDefaultErrorCallback;
	PxDefaultAllocator            m_PxDefaultAllocatorCallback;
	PxVisualDebuggerConnection*     m_pPVDConnection;
	PxCooking*                  m_pCooking;
	//====================================================
public:
	/////////////// Physx SDK Member Function ///////////////
	void InitializePhysxEngine();
	void ReleasePhysxEngine();

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
	std::chrono::system_clock::time_point bulletstart;

	bool keystate = false;
	std::chrono::system_clock::time_point idlestatepoint;
	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point mousestart;
	bool istime = false;

	float g_fgametime;// 게임시간.
	float g_fgravity; //중력 세기.
public:
	void ProcessPacket(char *ptr);
	void ReadPacket(SOCKET sock);
	void InitNetwork(HWND main_window);

};
