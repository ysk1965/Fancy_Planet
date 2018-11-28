//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include <process.h>

CGameFramework* CGameFramework::m_pGFforMultiThreads = NULL;

CGameFramework::CGameFramework()
{
	// FMOD
	string strBGFileName[] = {
		"../Assets/Audio/BACKGROUND_ROBBY.mp3",
		"../Assets/Audio/BACKGROUND_INGAME.mp3",
		"../Assets/Audio/BACKGROUND_END.mp3"
	};

	string strEffectFileName[] = {
		"../Assets/Audio/ROBBY_CLICK.wav",
		"../Assets/Audio/ROBBY_CHANGECHARACTER.mp3",
		"../Assets/Audio/HUMAN_SHOT.wav",
		"../Assets/Audio/HUMAN_DIED.wav",
		"../Assets/Audio/HUMAN_JUMP.wav",
		"../Assets/Audio/HUMANSKILL_1.wav",
		"../Assets/Audio/HUMANSKILL_2.wav",
		"../Assets/Audio/DRONE_SHOT.wav",
		"../Assets/Audio/DRONE_DIED.wav",
		"../Assets/Audio/DRONE_JUMP.wav",
		"../Assets/Audio/DRONESKILL_1.wav",
		"../Assets/Audio/DRONESKILL_2.wav",
		"../Assets/Audio/CREATURE_SHOT.wav",
		"../Assets/Audio/CREATURE_DIED.wav",
		"../Assets/Audio/CREATURE_JUMP.wav",
		"../Assets/Audio/CREATURESKILL_1.wav",
		"../Assets/Audio/CREATURESKILL_2.wav",
		"../Assets/Audio/INGAME_CHARGE.wav",
		"../Assets/Audio/INGAME_COMPLETE.wav",
	};


	m_FmodSound.CreateBGSound(3, strBGFileName);
	m_FmodSound.CreateEffectSound(19, strEffectFileName);


	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++)
		m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	m_ppd3dCommandAllocators = new ID3D12CommandAllocator*[NUM_COMMANDLIST];
	m_ppd3dCommandLists = new ID3D12GraphicsCommandList*[NUM_COMMANDLIST];

	m_ppd3dShadowCommandAllocators = new ID3D12CommandAllocator*[NUM_COMMANDLIST];
	m_ppd3dShadowCommandLists = new ID3D12GraphicsCommandList*[NUM_COMMANDLIST];

	m_pd3dScreenCommandList = NULL;
	m_pd3dScreenCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++)
		m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_ptOldCursorPos.x = m_nWndClientWidth / 2;
	m_ptOldCursorPos.y = m_nWndClientHeight / 2;

	m_ppScenes = new CScene*[NUM_SUBSETS];
	m_pPlayer = NULL;

	_tcscpy_s(m_pszFrameRate, _T("FancyPlanet ("));

	m_pGFforMultiThreads = this;

	m_nIsLobby = 0;

	BuildThreadsAndEvents();
}

CGameFramework::~CGameFramework()
{
	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		CloseHandle(m_workerBeginRenderFrame[i]);
		CloseHandle(m_workerFinishShadowPass[i]);
		CloseHandle(m_workerFinishedRenderFrame[i]);
		CloseHandle(m_threadHandles[i]);
	}
}
bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();

	BuildLobby();

	//m_FmodSound.PlaySoundBG(static_cast<int>(BACKSOUND::BACKGROUND_ROBBY));
	return(true);
}
void CGameFramework::FrameAdvanceInEnd()
{
	m_GameTimer.Tick(0.0f);

	static UCHAR pKeysBuffer[256];
	
	if (m_iSceneState == GAMERESULT)
	{
		if (GetKeyboardState(pKeysBuffer))
		{
			if (pKeysBuffer[VK_SPACE] & 0xF0)
			{
				exit(true);
			}
		}
	}

	HRESULT hResult = m_pd3dScreenCommandAllocator->Reset();
	hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);

	m_ppScenes[CHARACTER]->SelectCharacter(m_nCharacterType);

	m_pCamera->SetCharacterType(m_nCharacterType);

	float pfClearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_pd3dScreenCommandList->ClearRenderTargetView(m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], pfClearColor, 0, NULL);
	m_pd3dScreenCommandList->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dScreenCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);

	m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
	m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

	m_pUIShader->EndRender(m_pd3dScreenCommandList, m_pCamera, m_bIsVictory);
	m_ppScenes[CHARACTER]->Render(m_pd3dScreenCommandList, m_pCamera);

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_pd3dScreenCommandList->Close();

	ID3D12CommandList *ppd3dScreenCommandLists[] = { m_pd3dScreenCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);
	WaitForGpuComplete();

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}
void CGameFramework::FrameAdvanceInLobby()
{
	if (m_iSceneState == LOADING) //m_iSceneState == LOADING
	{
		m_nIsLobby = 1;

		BuildObjects();
		OnResizeBackBuffers();

		m_pCamera->SetCharacterType(m_nCharacterType);
		m_pPlayer->SetCharacterType(m_nCharacterType);

		m_pCamera = m_pPlayer->ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
		m_pCamera->SetLobbyFlag(false);
	}
	m_GameTimer.Tick(0.0f);
	HRESULT hResult = m_pd3dScreenCommandAllocator->Reset();
	hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);

	if (m_bReady)
	{
		if (m_nCharacterType == SOLDIER)
			m_ppScenes[CHARACTER]->ChangeAnimationInLobby(static_cast<int>(SOLDIER::ANIMATION_MOVE_FORWARD), SOLDIER);
		else if (m_nCharacterType == DRONE)
			m_ppScenes[CHARACTER]->ChangeAnimationInLobby(static_cast<int>(DRONE::ANIMATION_SKILL), DRONE);
		else
			m_ppScenes[CHARACTER]->ChangeAnimationInLobby(static_cast<int>(CREATURE::ANIMATION_MOVE_RUN1), CREATURE);
	}
	else
		m_ppScenes[CHARACTER]->ChangeAnimationInLobby(0, m_nCharacterType);

	m_ppScenes[CHARACTER]->SelectCharacter(m_nCharacterType);

	m_pCamera->SetCharacterType(m_nCharacterType);

	float pfClearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_pd3dScreenCommandList->ClearRenderTargetView(m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], pfClearColor, 0, NULL);
	m_pd3dScreenCommandList->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dScreenCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);

	m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
	m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

	m_pUIShader->SetSelect(m_nCharacterType);
	m_pUIShader->RobbyRender(m_pd3dScreenCommandList, m_pCamera);
	m_ppScenes[CHARACTER]->Render(m_pd3dScreenCommandList, m_pCamera);

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hResult = m_pd3dScreenCommandList->Close();

	ID3D12CommandList *ppd3dScreenCommandLists[] = { m_pd3dScreenCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);
	WaitForGpuComplete();

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}
void CGameFramework::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

#ifdef _WITH_CREATE_SWAPCHAIN_FOR_HWND
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	HRESULT hResult = m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1 **)&m_pdxgiSwapChain);
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	dxgiSwapChainDesc.Flags = 0;
#else
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
#endif

	HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain **)&m_pdxgiSwapChain);
#endif

	if (!m_pdxgiSwapChain)
	{
		MessageBox(NULL, L"Swap Chain Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	hResult = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

#if defined(_DEBUG)
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void **)&m_pd3dDebugController);
	m_pd3dDebugController->EnableDebugLayer();
#endif

	hResult = ::CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void **)&m_pdxgiFactory);

	IDXGIAdapter1 *pd3dAdapter = NULL;

	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice))) break;
	}

	if (!m_pd3dDevice)
	{
		hResult = m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void **)&pd3dAdapter);
		hResult = D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void **)&m_pd3dDevice);
	}

	if (!m_pd3dDevice)
	{
		MessageBox(NULL, L"Direct3D 12 Device Cannot be Created.", L"Error", MB_OK);
		::PostQuitMessage(0);
		return;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	hResult = m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void **)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
		m_nFenceValues[i] = 1;
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	::gnCbvSrvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, _uuidof(ID3D12CommandQueue), (void **)&m_pd3dCommandQueue);

	for (int i = 0; i < NUM_COMMANDLIST; i++)
	{
		hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_ppd3dCommandAllocators[i]);
		hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_ppd3dCommandAllocators[i], NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_ppd3dCommandLists[i]);
		hResult = m_ppd3dCommandLists[i]->Close();

		hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_ppd3dShadowCommandAllocators[i]);
		hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_ppd3dShadowCommandAllocators[i], NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_ppd3dShadowCommandLists[i]);
		hResult = m_ppd3dShadowCommandLists[i]->Close();
	}
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pd3dScreenCommandAllocator);
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dScreenCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dScreenCommandList);
	hResult = m_pd3dScreenCommandList->Close();

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pd3dPreShadowCommandAllocator);
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dPreShadowCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dPreShadowCommandList);
	hResult = m_pd3dPreShadowCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers; // 이미시브 컬러를 위한 버퍼
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateSwapChainRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pd3dRtvSwapChainBackBufferCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void **)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], &d3dRenderTargetViewDesc, m_pd3dRtvSwapChainBackBufferCPUHandles[i]);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}

void CGameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	HRESULT hresult = m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc
		, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_d3dDsvDepthStencilBufferCPUHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, &d3dDepthStencilViewDesc, m_d3dDsvDepthStencilBufferCPUHandle);

}

void CGameFramework::OnResizeBackBuffers()
{
	WaitForGpuComplete();
	HRESULT hResult = m_pd3dScreenCommandAllocator->Reset();
	hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);
	hResult = m_pd3dPreShadowCommandAllocator->Reset();
	hResult = m_pd3dPreShadowCommandList->Reset(m_pd3dPreShadowCommandAllocator, NULL);

	for (int i = 0; i < m_nSwapChainBuffers; i++)
		if (m_ppd3dSwapChainBackBuffers[i])
			m_ppd3dSwapChainBackBuffers[i]->Release();

	if (m_pd3dDepthStencilBuffer)
		m_pd3dDepthStencilBuffer->Release();
#ifdef _WITH_ONLY_RESIZE_BACKBUFFERS
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
	m_nSwapChainBufferIndex = 0;
#else
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = 0;
#endif

	CreateSwapChainRenderTargetViews();
	CreateDepthStencilView();
	m_pShadowShader->Release();
	m_pShadowShader->CreateResource(m_pd3dDevice, m_nWndClientWidth, m_nWndClientHeight);
	m_pShadowShader->CreateDepthStencilView(m_pd3dDevice);
	if (m_nIsLobby == 1)
		m_pComputeShader->BuildTextures(m_pd3dDevice, m_pd3dDepthStencilBuffer, m_nWndClientWidth, m_nWndClientHeight);
	hResult = m_pd3dScreenCommandList->Close();
	m_pd3dPreShadowCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dScreenCommandList, m_pd3dPreShadowCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(2, ppd3dCommandLists);

	WaitForGpuComplete();
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();
	::CloseHandle(m_hFenceEvent);

#if defined(_DEBUG)
	if (m_pd3dDebugController) m_pd3dDebugController->Release();
#endif

	if (m_pd3dDepthStencilBuffer)
		m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap)
		m_pd3dDsvDescriptorHeap->Release();

	for (int i = 0; i < m_nSwapChainBuffers; i++)
		if (m_ppd3dSwapChainBackBuffers[i])
			m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap)
		m_pd3dRtvDescriptorHeap->Release();

	if (m_ppd3dCommandAllocators)
	{
		for (int i = 0; i < NUM_COMMANDLIST; i++)
			m_ppd3dCommandAllocators[i]->Release();
	}
	if (m_ppd3dShadowCommandAllocators)
	{
		for (int i = 0; i < NUM_COMMANDLIST; i++)
			m_ppd3dShadowCommandAllocators[i]->Release();
	}
	if (m_pd3dCommandQueue)
		m_pd3dCommandQueue->Release();
	if (m_ppd3dCommandLists)
	{
		for (int i = 0; i < NUM_COMMANDLIST; i++)
			m_ppd3dCommandLists[i]->Release();
	}
	if (m_pd3dFence)
		m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain)
		m_pdxgiSwapChain->Release();
	if (m_pd3dDevice)
		m_pd3dDevice->Release();
	if (m_pdxgiFactory)
		m_pdxgiFactory->Release();
}

void CGameFramework::CreateLightsAndMaterialsShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void **)&m_pcbMappedLights);

	UINT ncbMaterialBytes = ((sizeof(MATERIALS) + 255) & ~255); //256의 배수
	m_pd3dcbMaterials = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbMaterialBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbMaterials->Map(0, NULL, (void **)&m_pcbMappedMaterials);
}

void CGameFramework::UpdateLightsAndMaterialsShaderVariables()
{
	::memcpy(m_pcbMappedLights, m_pLights, sizeof(LIGHTS));
	::memcpy(m_pcbMappedMaterials, m_pMaterials, sizeof(MATERIALS));
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer)
		delete m_pPlayer;

	if (m_ppScenes)
		for (int i = 0; i < NUM_SUBSETS; i++)
			m_ppScenes[i]->ReleaseObjects();

	if (m_ppScenes)
		for (int i = 0; i < NUM_SUBSETS; i++)
			delete m_ppScenes[i];

	if (m_pUIShader)
		m_pUIShader->ReleaseObjects();

	if (m_pUIShader)
		delete m_pUIShader;

	m_FmodSound.ReleaseSound();
}
void CGameFramework::NotifyIdleState()
{
	std::chrono::milliseconds ms;

	ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - idlestatepoint);

	if (g_my_info.anim_state == 0 && ms.count() > 500 && g_my_info.state != DIED)
	{
		cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
		my_pos_packet->size = sizeof(cs_packet_pos);
		send_wsabuf.len = sizeof(cs_packet_pos);
		my_pos_packet->id = g_myid;
		my_pos_packet->type = CS_POS;
		my_pos_packet->keyInputState = NONE;//idle 공통
		send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
		g_my_info.pos = GetPlayerMatrix();
		DWORD iobyte;
		WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		idlestatepoint = chrono::system_clock::now();				
	}
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	if (m_iSceneState == INGAME)
	{
		m_pCamera->Rotate(0, 0, 0);
		m_ppScenes[OBJECT]->SetSkyBoxPosition(m_pCamera->GetPosition());		
	}
	if (m_pPlayer)
	{
		//m_pCamera->Rotate(0, 0, 0);
		//m_ppScenes[OBJECT]->SetSkyBoxPosition(m_pCamera->GetPosition());
		m_pPlayer->CalcSkill();
	}
	m_ppScenes[OBJECT]->AnimateObjects(fTimeElapsed, NULL, m_iSceneState);
}

void CGameFramework::WaitForGpuComplete()
{
	const UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];

	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::RenderSubset(int iIndex)
{
	while (iIndex >= 0 && iIndex < NUM_SUBSETS)
	{
		WaitForSingleObject(m_workerBeginRenderFrame[iIndex], INFINITE);

		m_pShadowShader->OnPrepareRender(m_ppd3dShadowCommandLists[iIndex]);

		m_ppScenes[iIndex]->ShadowRender(m_ppd3dShadowCommandLists[iIndex], m_pCamera, m_pShadowShader->GetShadowInfo());

		m_ppd3dShadowCommandLists[iIndex]->Close();

		SetEvent(m_workerFinishShadowPass[iIndex]);

		m_ppd3dCommandLists[iIndex]->RSSetViewports(1, m_pCamera->GetViewport());
		m_ppd3dCommandLists[iIndex]->RSSetScissorRects(1, m_pCamera->GetScissorRect());
		m_ppd3dCommandLists[iIndex]->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);

		m_ppScenes[iIndex]->Render(m_ppd3dCommandLists[iIndex], m_pCamera);

		SetEvent(m_workerFinishedRenderFrame[iIndex]);
	}
}

void CGameFramework::PrepareFrame()
{
	for (int i = 0; i < NUM_COMMANDLIST; i++)
	{
		HRESULT hResult = m_ppd3dCommandAllocators[i]->Reset();
		hResult = m_ppd3dCommandLists[i]->Reset(m_ppd3dCommandAllocators[i], NULL);
		hResult = m_ppd3dShadowCommandAllocators[i]->Reset();
		hResult = m_ppd3dShadowCommandLists[i]->Reset(m_ppd3dShadowCommandAllocators[i], NULL);
	}
	HRESULT hResult = m_pd3dScreenCommandAllocator->Reset();
	hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);
	hResult = m_pd3dPreShadowCommandAllocator->Reset();
	hResult = m_pd3dPreShadowCommandList->Reset(m_pd3dPreShadowCommandAllocator, NULL);

	m_pShadowShader->SwapResource(m_pd3dPreShadowCommandList, true);

	m_pShadowShader->ClearDepthStencilView(m_pd3dPreShadowCommandList);

	m_pd3dPreShadowCommandList->Close();

	m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
	m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

	float pfClearColor[4] = { 0.1f, 0.3f, 0.2f, 1.0f };

	m_pShadowShader->SwapResource(m_pd3dScreenCommandList, false);

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_pd3dScreenCommandList->ClearRenderTargetView(m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], pfClearColor, 0, NULL);

	m_pd3dScreenCommandList->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pShadowShader->UpdateTransform();
}

void CGameFramework::FrameAdvance()
{
	if (m_iSceneState == GAMERESULT)
	{
		FrameAdvanceInEnd();
	}
	else
	{
		m_GameTimer.Tick(0.0f);
		m_ppScenes[EFFECT]->UpdateEffect(m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37));
		AnimateObjects();
		ProcessInput();		
		UpdateLightsAndMaterialsShaderVariables();
		PrepareFrame();

		m_pPlayer->Update(m_GameTimer.GetTimeElapsed());

		if (m_iSceneState == INGAME)
		{
		for (int i = 0; i < OBJECTS_NUMBER; ++i)
			{
				m_ppScenes[OBJECT]->SetObjectsVectorFromPacket(m_xmf3ObjectsPos[i], m_xmf4ObjectsQuaternion[i], i);
			}

			for (int i = 0; i < MAX_USER; ++i)
			{
				if (g_player_info[i].m_isconnected&&g_player_info[i].m_isJumpState == true)
				{
					if (i != g_myid)
					{
						if (g_player_info[i].m_iCharacterType == SOLDIER)
							m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(SOLDIER::ANIMATION_MOVE_JUMP), g_player_info, i);
						else if (g_player_info[i].m_iCharacterType == CREATURE)
							m_ppScenes[CHARACTER]->ChangeAnimation(static_cast<int>(CREATURE::ANIMATION_MOVE_JUMP), g_player_info, i);
					}
				}

				if (g_player_info[i].m_isMoveSKillState)
				{
					if (g_player_info[i].m_iCharacterType == DRONE)
					{
						m_ppScenes[EFFECT]->SetParticle(1, DRONE, g_player_info[i].Character_idx, NULL);
						g_player_info[i].anim_state = static_cast<int>(DRONE::ANIMATION_SKILL);
						g_player_info[i].m_isMoveSKillState = false;
					}
					else if (g_player_info[i].m_iCharacterType == SOLDIER)
						m_ppScenes[EFFECT]->SetParticle(3, SOLDIER, g_player_info[i].m_iCharacterIndex, NULL);

				}
			}
		}
		UINT r[] = { Team[REDTeam].m_iKill,Team[REDTeam].m_iNumberOfterritories };
		UINT b[] = { Team[BLUETeam].m_iKill,Team[BLUETeam].m_iNumberOfterritories };
		UINT g[] = { Team[GREENTeam].m_iKill,Team[GREENTeam].m_iNumberOfterritories };

		m_pUIShader->SetAndCalculateScoreLocation(r, b, g);
		m_pUIShader->SetAndCalculateTimeLocation(GAMETIME - m_fgametime);
		m_pUIShader->SetAndCalculateHPLocation(g_my_info.m_iHP);
		m_pUIShader->SetSkillCover(m_pPlayer->GetSkill1Time(), m_pPlayer->GetSkill2Time());
		m_pUIShader->CalculatePointer(m_fgravity);

		if (m_iSceneState == INGAME)
		{
			m_ppScenes[CHARACTER]->ModelsSetPosition(g_player_info, g_myid, m_nCharacterType);
		}
		for (int i = 0; i < NUM_SUBSETS; i++)
		{
			SetEvent(m_workerBeginRenderFrame[i]);
		}

		WaitForMultipleObjects(NUM_SUBSETS, m_workerFinishShadowPass, TRUE, INFINITE);

		ID3D12CommandList *ppd3dCommandLists1[] = { m_pd3dPreShadowCommandList
			, m_ppd3dShadowCommandLists[0], m_ppd3dShadowCommandLists[1],  m_ppd3dShadowCommandLists[2], m_ppd3dShadowCommandLists[3] };

		m_pd3dCommandQueue->ExecuteCommandLists(NUM_SUBSETS + 1, ppd3dCommandLists1);
		WaitForGpuComplete();
		WaitForMultipleObjects(NUM_SUBSETS, m_workerFinishedRenderFrame, TRUE, INFINITE);

		::SynchronizeResourceTransition(m_ppd3dCommandLists[1], m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
		::SynchronizeResourceTransition(m_ppd3dCommandLists[1], m_pd3dDepthStencilBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);

		for (int i = 0; i < NUM_SUBSETS; i++)
		{
			m_ppd3dCommandLists[i]->Close();
		}

		ID3D12CommandList *ppd3dCommandLists2[] = { m_pd3dScreenCommandList
			, m_ppd3dCommandLists[3],  m_ppd3dCommandLists[2], m_ppd3dCommandLists[0],  m_ppd3dCommandLists[1] };

		HRESULT hResult = m_pd3dScreenCommandList->Close();

		m_pd3dCommandQueue->ExecuteCommandLists(NUM_SUBSETS + 1, ppd3dCommandLists2);

		WaitForGpuComplete();

		//계산쉐이더 부분
		{
			hResult = m_pd3dScreenCommandAllocator->Reset();
			hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);
			m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
			m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

			m_pComputeShader->Compute(m_pd3dDevice, m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex]);
			::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_pd3dDepthStencilBuffer, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			hResult = m_pd3dScreenCommandList->Close();

			ID3D12CommandList *ppd3dScreenCommandLists[] = { m_pd3dScreenCommandList };
			m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);

			WaitForGpuComplete();
		}

		//UI 그리기
		{
			hResult = m_pd3dScreenCommandAllocator->Reset();
			hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);
			m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
			m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

			m_pd3dScreenCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);

			m_pUIShader->Render(m_pd3dScreenCommandList, m_pCamera);

			::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			hResult = m_pd3dScreenCommandList->Close();

			ID3D12CommandList *ppd3dScreenCommandLists[] = { m_pd3dScreenCommandList };
			m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);
			WaitForGpuComplete();
		}

		m_pdxgiSwapChain->Present(0, 0);

		MoveToNextFrame();

		m_ppScenes[EFFECT]->Update(m_pCamera);
		::SetWindowText(m_hWnd, m_pszFrameRate);
	}
}