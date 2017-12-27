#pragma once

#define FRAME_BUFFER_WIDTH		640
#define FRAME_BUFFER_HEIGHT		480

#include "Timer.h"
#include "Player.h"

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
	void CreateDepthStencilView();
	void CreateCommandQueueAndList();

	void CreateRenderTargetViews();

	void OnResizeBackBuffers();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE						m_hInstance;
	HWND							m_hWnd;

	int								m_nWndClientWidth;
	int								m_nWndClientHeight;

	IDXGIFactory4					*m_pdxgiFactory = NULL;
	IDXGISwapChain3					*m_pdxgiSwapChain = NULL;
	ID3D12Device					*m_pd3dDevice = NULL;

	bool							m_bMsaa4xEnable = false;
	UINT							m_nMsaa4xQualityLevels = 0;

	static const UINT				m_nSwapChainBuffers = 2;
	UINT							m_nSwapChainBufferIndex;

	ID3D12Resource					*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap			*m_pd3dRtvDescriptorHeap = NULL;
	UINT							m_nRtvDescriptorIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBuffers];

	ID3D12Resource					*m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap			*m_pd3dDsvDescriptorHeap = NULL;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dDsvDepthStencilBufferCPUHandle;

	ID3D12CommandAllocator			*m_pd3dCommandAllocator = NULL;
	ID3D12CommandQueue				*m_pd3dCommandQueue = NULL;
	ID3D12GraphicsCommandList		*m_pd3dCommandList = NULL;

	ID3D12Fence						*m_pd3dFence = NULL;
	UINT64							m_nFenceValues[m_nSwapChainBuffers];
	HANDLE							m_hFenceEvent;
	UINT						m_nDsvDescriptorIncrementSize;
#if defined(_DEBUG)
	ID3D12Debug						*m_pd3dDebugController;
#endif

	CGameTimer						m_GameTimer;

	CCamera							*m_pCamera = NULL;

	static const UINT				m_nRenderTargetBuffers = 2; //랜더타겟이 추가로 두개가 더필요하다. 1.노말벡터, 2. 색백터
	ID3D12Resource					*m_ppd3dRenderTargetBuffers[m_nRenderTargetBuffers];
	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dRtvRenderTargetBufferCPUHandles[m_nRenderTargetBuffers];

	//	D3D12_CPU_DESCRIPTOR_HANDLE		m_pd3dSrvCPUHandles[m_nRenderTargetBuffers];
	//	D3D12_GPU_DESCRIPTOR_HANDLE		m_pd3dSrvGPUHandles[m_nRenderTargetBuffers];

	//2개의 텍스쳐를 읽어다가, Laplacian을 사용해 새로그려  화면에 출력하는 쉐이더

	POINT							m_ptOldCursorPos;

	_TCHAR							m_pszFrameRate[50];
};

