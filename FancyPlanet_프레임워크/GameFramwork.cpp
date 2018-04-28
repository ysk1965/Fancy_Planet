//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"
#include <process.h>
CGameFramework* CGameFramework::m_pGFforMultiThreads = NULL;

CGameFramework::CGameFramework()
{
	m_pdxgiFactory = NULL;
	m_pdxgiSwapChain = NULL;
	m_pd3dDevice = NULL;

	for (int i = 0; i < m_nRenderTargetBuffers; i++)
		m_ppd3dRenderTargetBuffers[i] = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++)
		m_ppd3dSwapChainBackBuffers[i] = NULL;
	m_nSwapChainBufferIndex = 0;

	for (int i = 0; i < NUM_COMMANDLIST; i++)
	{
		m_ppd3dCommandAllocators = new ID3D12CommandAllocator*[NUM_COMMANDLIST];
		m_ppd3dCommandLists = new ID3D12GraphicsCommandList*[NUM_COMMANDLIST];
	}
	m_pd3dScreenCommandList = NULL;
	m_pd3dScreenCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;

	m_pd3dRtvDescriptorHeap = NULL;
	m_pd3dDsvDescriptorHeap = NULL;

	m_nRtvDescriptorIncrementSize = 0;
	m_nDsvDescriptorIncrementSize = 0;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_ppScenes = NULL;
	m_pPlayer = NULL;

	_tcscpy_s(m_pszFrameRate, _T("LabProject ("));

	m_pGFforMultiThreads = this;

	BuildThreadsAndEvents();
}

CGameFramework::~CGameFramework()
{
}
void CGameFramework::BuildThreadsAndEvents()
{

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		m_workerBeginRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_workerFinishedRenderFrame[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL);

		m_threadParameters[i].threadIndex = i;

		struct threadwrapper
		{
			static unsigned int WINAPI thunk(LPVOID lpParameter)
			{
				ThreadParameter* parameter = reinterpret_cast<ThreadParameter*>(lpParameter);
				CGameFramework::Get()->RenderSubset(parameter->threadIndex);
				return 0;
			}
		};

		m_threadHandles[i] = reinterpret_cast<HANDLE>(_beginthreadex(
			nullptr,
			0,
			threadwrapper::thunk,
			reinterpret_cast<LPVOID>(&m_threadParameters[i]),
			0,
			nullptr));

		assert(m_workerBeginRenderFrame[i] != NULL);
		assert(m_workerFinishedRenderFrame[i] != NULL);
		assert(m_threadHandles[i] != NULL);
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
	//Physx SDK Engine Init=========================
	InitializePhysxEngine();

	BuildObjects();

	return(true);
}

//#define _WITH_SWAPCHAIN

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
	for (UINT i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 1;
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
	}
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void **)&m_pd3dScreenCommandAllocator);
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dScreenCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void **)&m_pd3dScreenCommandList);
	hResult = m_pd3dScreenCommandList->Close();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + m_nRenderTargetBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	d3dDescriptorHeapDesc.NumDescriptors = 4;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void **)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV); //??
}

void CGameFramework::CreateRenderTargetViews()
{
	CTexture *pTexture = new CTexture(m_nRenderTargetBuffers, RESOURCE_TEXTURE2D_ARRAY, 0);

	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.0f, 0.0f, 0.0f, 1.0f } };
	for (UINT i = 0; i < m_nRenderTargetBuffers; i++)
	{
		//버퍼를 만든다.
		m_ppd3dRenderTargetBuffers[i] = pTexture->CreateTexture(m_pd3dDevice, m_pd3dScreenCommandList, m_nWndClientWidth, m_nWndClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, i);
		m_ppd3dRenderTargetBuffers[i]->AddRef();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBuffers * m_nRtvDescriptorIncrementSize);

	D3D12_RENDER_TARGET_VIEW_DESC d3dRenderTargetViewDesc;
	d3dRenderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dRenderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	d3dRenderTargetViewDesc.Texture2D.MipSlice = 0;
	d3dRenderTargetViewDesc.Texture2D.PlaneSlice = 0;

	for (UINT i = 0; i < m_nRenderTargetBuffers; i++)
	{
		m_pd3dRtvRenderTargetBufferCPUHandles[i] = d3dRtvCPUDescriptorHandle;
		//버퍼의 주소가 백버퍼 주소가 아니고, 방금 만든 주소를 사용한다.
		m_pd3dDevice->CreateRenderTargetView(pTexture->GetTexture(i), &d3dRenderTargetViewDesc, m_pd3dRtvRenderTargetBufferCPUHandles[i]);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
	m_pScreenShader = new CTextureToFullScreenShader();
	m_pScreenShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pScreenShader->CreateShader(m_pd3dDevice, m_pScreenShader->GetGraphicsRootSignature(), 4);
	m_pScreenShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList, m_pPlayer, pTexture);

	m_pUIShader = new UIShader(m_pPlayer);
	m_pUIShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pUIShader->CreateShader(m_pd3dDevice, m_pUIShader->GetGraphicsRootSignature(), 4);
	m_pUIShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);
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
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
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

	HRESULT hresult = m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void **)&m_pd3dDepthStencilBuffer);

	D3D12_DEPTH_STENCIL_VIEW_DESC d3dDepthStencilViewDesc;
	::ZeroMemory(&d3dDepthStencilViewDesc, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	d3dDepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dDepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	d3dDepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_d3dDsvDepthStencilBufferCPUHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, m_d3dDsvDepthStencilBufferCPUHandle);
}

void CGameFramework::OnResizeBackBuffers()
{
	WaitForGpuComplete();

	m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);

	for (int i = 0; i < m_nRenderTargetBuffers; i++)
		if (m_ppd3dRenderTargetBuffers[i])
			m_ppd3dRenderTargetBuffers[i]->Release();

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
	//m_pdxgiSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = 0;
#endif
	CreateSwapChainRenderTargetViews();
	CreateDepthStencilView();
	CreateRenderTargetViews();

	m_pd3dScreenCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dScreenCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	WaitForGpuComplete();
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_ppScenes)
		for (int i = 0; i<NUM_SUBSETS; i++)
			m_ppScenes[i]->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;

	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (m_ppScenes)
		for (int i = 0; i < NUM_SUBSETS; i++)
			m_ppScenes[i]->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			m_pCamera = m_pPlayer->ChangeCamera((DWORD)(wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_F9:
		{
			BOOL bFullScreenState = FALSE;
			m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
			m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);

			DXGI_MODE_DESC dxgiTargetParameters;
			dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			dxgiTargetParameters.Width = m_nWndClientWidth;
			dxgiTargetParameters.Height = m_nWndClientHeight;
			dxgiTargetParameters.RefreshRate.Numerator = 60;
			dxgiTargetParameters.RefreshRate.Denominator = 1;
			dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);

			OnResizeBackBuffers();

			break;
		}
		case VK_F10:
			break;
		default:
			break;
		case 'W':
		{
			is_fowardmove = false;
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->m_pos = GetPlayerMatrix();
			my_pos_packet->roomnumb = g_my_info.roomnumb;

			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			break;
		}
		case 'A':
		{
			is_fowardmove = false;
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->m_pos = GetPlayerMatrix();
			send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			break;
		}
		case 'S':
		{
			is_fowardmove = false;
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->m_pos = GetPlayerMatrix();
			send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			break;
		}
		case 'D':
		{
			is_fowardmove = false;
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->m_pos = GetPlayerMatrix();
			send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			break;
		}
		case 'R':
		{
			cs_packet_ready * my_packet = reinterpret_cast<cs_packet_ready *>(send_buffer);
			my_packet->size = sizeof(my_packet);
			send_wsabuf.len = sizeof(my_packet);
			my_packet->type = CS_READY;
			my_packet->state = g_my_info.m_isready;
			DWORD iobyte;
			printf("packet : READY  \n");
			if (g_my_info.m_scene == 0)//레디패킷은 로비일대만
			{
				int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

				if (ret) {
					int error_code = WSAGetLastError();
					printf("Error while sending packet [%d]", error_code);
				}
			}
			break;
		}
		}

	default:
		break;

	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
	{
		OnResizeBackBuffers();
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
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

	for (int i = 0; i < m_nRenderTargetBuffers; i++)
		if (m_ppd3dRenderTargetBuffers[i])
			m_ppd3dRenderTargetBuffers[i]->Release();
	for (int i = 0; i < m_nSwapChainBuffers; i++)
		if (m_ppd3dSwapChainBackBuffers[i])
			m_ppd3dSwapChainBackBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap)
		m_pd3dRtvDescriptorHeap->Release();

	if (m_ppd3dCommandAllocators)
	{
		for (int i = 0; i<NUM_COMMANDLIST; i++)
			m_ppd3dCommandAllocators[i]->Release();
	}
	if (m_pd3dCommandQueue)
		m_pd3dCommandQueue->Release();
	if (m_ppd3dCommandLists)
	{
		for (int i = 0; i<NUM_COMMANDLIST; i++)
			m_ppd3dCommandLists[i]->Release();
	}
	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiSwapChain)
		m_pdxgiSwapChain->Release();
	if (m_pd3dDevice)
		m_pd3dDevice->Release();
	if (m_pdxgiFactory)
		m_pdxgiFactory->Release();

	//Physx SDK Release=======================
	ReleasePhysxEngine();
	//========================================
}

void CGameFramework::BuildObjects()
{
	for (int i = 0; i<NUM_SUBSETS; i++)
		m_ppd3dCommandLists[i]->Reset(m_ppd3dCommandAllocators[i], NULL);

	m_nDivision = 10;

	m_ppScenes = new CScene*[NUM_SUBSETS];

	//for (int i = 0; i < NUM_SUBSETS; i++)
	//{
	//	TerrainAndSkyBoxScene *pScene = new TerrainAndSkyBoxScene();
	//	pScene->BuildObjects(m_pd3dDevice, m_ppd3dCommandLists[i]);
	//	m_ppScenes[i] = pScene;
	//}

	int i = 0;

	TerrainAndSkyBoxScene *pTScene = new TerrainAndSkyBoxScene(m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	pTScene->BuildObjects(m_pd3dDevice, m_ppd3dCommandLists[i]);
	m_ppScenes[i++] = pTScene;

	PhysXScene *pPScene = new PhysXScene(m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	m_ppScenes[i++] = pPScene;

	CharacterScene *pCScene = new CharacterScene(m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	m_ppScenes[i++] = pCScene;

	ObjectScene *pOScene = new ObjectScene(m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	m_ppScenes[i++] = pOScene;

	m_ppScenes[TERRAIN]->m_pPlayer = m_pPlayer = new CPlayer(m_pd3dDevice, m_ppd3dCommandLists[2], m_ppScenes[0]->GetGraphicsRootSignature(), m_ppScenes[TERRAIN]->GetTerrain(), 0);
	m_pPlayer->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, m_pPxControllerManager);
	m_pPlayer->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	for (int j = PHYSICS; j < NUM_SUBSETS; j++)
	{
		m_ppScenes[j]->m_pPlayer = m_pPlayer;
		m_ppScenes[j]->BuildObjects(m_pd3dDevice, m_ppd3dCommandLists[j]);
	}
	m_pCamera = m_pPlayer->GetCamera();


	m_ppbDivision = new bool*[m_nDivision];
	for (int i = 0; i < m_nDivision; i++)
	{
		bool * pbNew = new bool;
		*pbNew = false;
		m_ppbDivision[i] = pbNew;
	}

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		m_ppd3dCommandLists[i]->Close();
	}
	ID3D12CommandList *ppd3dCommandLists[] = { m_ppd3dCommandLists[0],  m_ppd3dCommandLists[1],  m_ppd3dCommandLists[2], m_ppd3dCommandLists[3] };
	m_pd3dCommandQueue->ExecuteCommandLists(NUM_SUBSETS, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pPlayer)
		m_pPlayer->ReleaseUploadBuffers();

	if (m_ppScenes)
		for (int i = 0; i<NUM_SUBSETS; i++)
			m_ppScenes[i]->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pPlayer)
		delete m_pPlayer;

	if (m_ppScenes)
		for (int i = 0; i<NUM_SUBSETS; i++)
			m_ppScenes[i]->ReleaseObjects();

	if (m_ppScenes)
		for (int i = 0; i<NUM_SUBSETS; i++)
			delete m_ppScenes[i];

	if (m_pScreenShader)
		m_pScreenShader->ReleaseObjects();

	if (m_pScreenShader)
		delete m_pScreenShader;

	if (m_pUIShader)
		m_pUIShader->ReleaseObjects();

	if (m_pUIShader)
		delete m_pUIShader;
}

void CGameFramework::ProcessInput()
{

	static UCHAR pKeysBuffer[256];
	bool bProcessedByScene = false;
	bool IsInput = false;

	if (GetKeyboardState(pKeysBuffer) && m_ppScenes[CHARACTER])
		bProcessedByScene = m_ppScenes[CHARACTER]->ProcessInput(pKeysBuffer);


	if (!bProcessedByScene)
	{
		DWORD dwDirection = 0;
		if (pKeysBuffer[87] & 0xF0)
		{

			dwDirection |= DIR_FORWARD;
			if (false == is_fowardmove)
			{
				cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
				my_pos_packet->size = sizeof(cs_packet_pos);
				send_wsabuf.len = sizeof(cs_packet_pos);
				my_pos_packet->m_pos = GetPlayerMatrix();
				my_pos_packet->type = CS_POS;
				my_pos_packet->roomnumb = g_my_info.roomnumb;
				send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
				g_my_info.pos = GetPlayerMatrix();
				DWORD iobyte;
				printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
				int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				if (ret) {
					int error_code = WSAGetLastError();
					printf("Error while sending packet [%d]", error_code);
				}
				is_fowardmove = true;
			}

			m_ppScenes[CHARACTER]->ChangeAnimation(2);
			IsInput = true;
		}
		if (pKeysBuffer[83] & 0xF0)
		{
			dwDirection |= DIR_BACKWARD;
			if (false == is_fowardmove)
			{
				cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
				my_pos_packet->size = sizeof(cs_packet_pos);
				send_wsabuf.len = sizeof(cs_packet_pos);
				my_pos_packet->type = CS_POS;
				my_pos_packet->m_pos = GetPlayerMatrix();
				send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
				g_my_info.pos = GetPlayerMatrix();
				DWORD iobyte;
				printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
				is_fowardmove = true;
			}
			m_ppScenes[CHARACTER]->ChangeAnimation(-2);
			IsInput = true;
		}
		if (pKeysBuffer[65] & 0xF0)
		{
			//dwDirection |= DIR_LEFT;
			m_pPlayer->Rotate(0.0f, -0.6f, 0.0f);
			if (false == is_fowardmove)
			{
				cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
				my_pos_packet->size = sizeof(cs_packet_pos);
				send_wsabuf.len = sizeof(cs_packet_pos);
				my_pos_packet->type = CS_POS;
				my_pos_packet->m_pos = GetPlayerMatrix();
				send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
				g_my_info.pos = GetPlayerMatrix();
				DWORD iobyte;
				printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			}
			//m_ppScenes[CHARACTER]->ChangeAnimation(1);
			//IsInput = true;
		}
		if (pKeysBuffer[68] & 0xF0)
		{
			//dwDirection |= DIR_RIGHT;
			m_pPlayer->Rotate(0.0f, 0.6f, 0.0f);
			if (false == is_fowardmove)
			{
				cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
				my_pos_packet->size = sizeof(cs_packet_pos);
				send_wsabuf.len = sizeof(cs_packet_pos);
				my_pos_packet->type = CS_POS;
				my_pos_packet->m_pos = GetPlayerMatrix();
				send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
				g_my_info.pos = GetPlayerMatrix();
				DWORD iobyte;
				printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
				WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			}
			//m_ppScenes[CHARACTER]->ChangeAnimation(1);
			//IsInput = true;
		}
		if (pKeysBuffer[VK_NEXT] & 0xF0)
		{
			dwDirection |= DIR_DOWN;
			IsInput = true;
		}
		if (pKeysBuffer[VK_SPACE] & 0xF0)
		{
			m_ppScenes[CHARACTER]->ChangeAnimation(3);
			IsInput = true;
		}
		float cxDelta = 0.0f, cyDelta = 0.0f;
		POINT ptCursorPos;
		if (GetCapture() == m_hWnd)
		{
			SetCursor(NULL);
			GetCursorPos(&ptCursorPos);
			cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
			cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
			SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		}

		if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (cxDelta || cyDelta)
			{
				if (pKeysBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
				else
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
			}
			if (dwDirection)
				m_pPlayer->Move(dwDirection, 100.0f * m_GameTimer.GetTimeElapsed(), true);
		}
	}

	if (!IsInput)
		m_ppScenes[CHARACTER]->ChangeAnimation(0);

	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();
	if (m_pPlayer)
		m_pPlayer->Animate(fTimeElapsed);
}

void CGameFramework::SpaceDivision()
{
	for (int i = 0; i < m_nDivision; i++)
	{
		*m_ppbDivision[i] = false;
	}
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
	}

	m_ppd3dCommandLists[0]->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	m_pd3dScreenCommandAllocator->Reset();
	m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);

	::SynchronizeResourceTransition(m_ppd3dCommandLists[0], m_ppd3dRenderTargetBuffers[0], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::SynchronizeResourceTransition(m_ppd3dCommandLists[0], m_ppd3dRenderTargetBuffers[1], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::SynchronizeResourceTransition(m_ppd3dCommandLists[0], m_ppd3dRenderTargetBuffers[2], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	::SynchronizeResourceTransition(m_ppd3dCommandLists[0], m_ppd3dRenderTargetBuffers[3], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);

	float pfClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_ppd3dCommandLists[0]->ClearRenderTargetView(m_pd3dRtvRenderTargetBufferCPUHandles[0], pfClearColor, 0, NULL); // 백버퍼가 아닌 랜더타켓
	m_ppd3dCommandLists[0]->ClearRenderTargetView(m_pd3dRtvRenderTargetBufferCPUHandles[1], pfClearColor, 0, NULL);
	m_ppd3dCommandLists[0]->ClearRenderTargetView(m_pd3dRtvRenderTargetBufferCPUHandles[2], pfClearColor, 0, NULL);
	m_ppd3dCommandLists[0]->ClearRenderTargetView(m_pd3dRtvRenderTargetBufferCPUHandles[3], pfClearColor, 0, NULL);
	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		m_ppd3dCommandLists[i]->OMSetRenderTargets(4, m_pd3dRtvRenderTargetBufferCPUHandles, TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);
	}
}

void CGameFramework::FrameAdvance()
{
	// PhysX Update
	if (m_pPxScene)
	{
		m_pPxScene->simulate(1 / 10.f);
		m_pPxScene->fetchResults(true);
	}

	m_GameTimer.Tick(0.0f);
	ProcessInput();
	AnimateObjects();
	PrepareFrame();
	SpaceDivision();

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		SetEvent(m_workerBeginRenderFrame[i]);
	}
	WaitForMultipleObjects(NUM_SUBSETS, m_workerFinishedRenderFrame, TRUE, INFINITE);

	ID3D12CommandList *ppd3dCommandLists[] = { m_ppd3dCommandLists[0],  m_ppd3dCommandLists[1],  m_ppd3dCommandLists[2], m_ppd3dCommandLists[3] };
	for (int i = 0; i<NUM_COMMANDLIST; i++)
		HRESULT hResult = m_ppd3dCommandLists[i]->Close();
	m_pd3dCommandQueue->ExecuteCommandLists(NUM_SUBSETS, ppd3dCommandLists);
	WaitForGpuComplete();

	HRESULT hResult = m_pd3dScreenCommandList->Close();
	for (int i = 0; i < NUM_COMMANDLIST; i++)
	{
		HRESULT hResult = m_ppd3dCommandAllocators[i]->Reset();
		hResult = m_ppd3dCommandLists[i]->Reset(m_ppd3dCommandAllocators[i], NULL);
	}
	hResult = m_pd3dScreenCommandAllocator->Reset();
	hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[0], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[1], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[2], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[3], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_pd3dScreenCommandList->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	m_pd3dScreenCommandList->ClearRenderTargetView(m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], Colors::Azure, 0, NULL);
	m_pd3dScreenCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);
	//원래대로 백버퍼에 그린다.
	m_pUIShader->Render(m_pd3dScreenCommandList, m_pCamera);
	m_pScreenShader->Render(m_pd3dScreenCommandList, m_pCamera);
#ifdef _WITH_PLAYER_TOP
	m_pd3dCommandList->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	ID3D12CommandList *ppd3dScreenCommandLists[] = { m_pd3dScreenCommandList };

	hResult = m_pd3dScreenCommandList->Close();
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);
	WaitForGpuComplete();

	for (int i = 0; i<NUM_COMMANDLIST; i++)
		HRESULT hResult = m_ppd3dCommandLists[i]->Close();

#ifdef _WITH_PRESENT_PARAMETERS
	DXGI_PRESENT_PARAMETERS dxgiPresentParameters;
	dxgiPresentParameters.DirtyRectsCount = 0;
	dxgiPresentParameters.pDirtyRects = NULL;
	dxgiPresentParameters.pScrollRect = NULL;
	dxgiPresentParameters.pScrollOffset = NULL;
	m_pdxgiSwapChain->Present1(1, 0, &dxgiPresentParameters);
#else
#ifdef _WITH_SYNCH_SWAPCHAIN
	m_pdxgiSwapChain->Present(1, 0);
#else
	m_pdxgiSwapChain->Present(0, 0);
#endif
#endif

	//	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::InitializePhysxEngine()
{
	m_pPxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_PxDefaultAllocatorCallback, m_PxDefaultErrorCallback);
	// 파운데이션 클래스의 인스턴스를 작성

	PxTolerancesScale PxScale = PxTolerancesScale();
	// 시뮬레이션이 실행되는 스케일을 정의하는 클래스
	// 시뮬레이션 스케일을 변경하려면 장면의 기본 중력 값을 변경하려는 것이됨, 안정적인 시뮬레이션에서는 bounceThreshold를 변경해야할 수도 있음.

	if (FAILED(m_pPxPhysicsSDK == NULL))
	{
		MSG_BOX(L"PhysicsSDK Initialize Failed");
		// 실패메세지 필요하면 넣기
	}

	m_pPxPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pPxFoundation, PxScale, false);
	// PhysX SDK의 인스턴스 생성


	//Cooking Init
	PxCookingParams params(PxScale); // 메쉬 Cooking에 영향을 미치는 매개 변수 설명
	params.meshWeldTolerance = 0.001f; //메쉬 용접 공차
	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES |
		PxMeshPreprocessingFlag::eREMOVE_UNREFERENCED_VERTICES | PxMeshPreprocessingFlag::eREMOVE_DUPLICATED_TRIANGLES);
	// 메쉬 전처리 매개 변수, 옵션 제어하는데 사용
	m_pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_pPxFoundation, params);
	// Cooking 인터페이스의 인스턴스를 생성

	PxInitExtensions(*m_pPxPhysicsSDK); // PhysXExtensions 라이브러리 초기화
	PxSceneDesc sceneDesc(m_pPxPhysicsSDK->getTolerancesScale()); // Scene의 설명자 클래스
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f); // 중력 값

	if (!sceneDesc.cpuDispatcher)
	{
		PxDefaultCpuDispatcher* pCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		// CPU Task 디스패쳐의 기본 구현 (기본 디스패쳐 생성 그리고 초기화)
		sceneDesc.cpuDispatcher = pCpuDispatcher;
	}

	if (!sceneDesc.filterShader)
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	// 간단한 필터 셰이더 구현

	m_pPxScene = m_pPxPhysicsSDK->createScene(sceneDesc); // 씬 생성

	m_pPxControllerManager = PxCreateControllerManager(*m_pPxScene); // 컨트롤러 생성

																	 // Physx Visual Debugger
	if (NULL == m_pPxPhysicsSDK->getPvdConnectionManager())
	{
		return;
	}
	PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();
	m_pPxPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
	m_pPVDConnection = PxVisualDebuggerExt::createConnection(m_pPxPhysicsSDK->getPvdConnectionManager(), "127.0.0.1", 5425, 1000, connectionFlags);
	// PVD 셋팅
}

void CGameFramework::ReleasePhysxEngine()
{
	if (m_pPVDConnection)
		m_pPVDConnection->release();
	if (m_pPxControllerManager)
		m_pPxControllerManager->release();
	if (m_pPxScene)
		m_pPxScene->release();
	if (m_pPxFoundation)
		m_pPxFoundation->release();
	if (m_pPxPhysicsSDK)
		m_pPxPhysicsSDK->release();
	if (m_pCooking)
		m_pCooking->release();
}
void CGameFramework::InitNetwork(HWND main_window)
{
	string s_ip;
	//원속함수들을 세팅합니다.
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	printf("Server IP INPUT : ");
	//scanf("%s", s_ip);

	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int Result = WSAConnect(g_mysocket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);

	int retval = WSAAsyncSelect(g_mysocket, main_window, WM_SOCKET, FD_CLOSE | FD_READ);
	if (retval) {
		int err_code = WSAGetLastError();
		printf("AsyncSelect Error [%d]\n", err_code);
	}
	printf("retval : %d\n", retval);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = 4000;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = 4000;
}

void CGameFramework::ProcessPacket(char *ptr)
{

	static bool first_time = true;

	switch (ptr[1])//패킷의 1번 인덱스에는 패킷의 종류에 대한 정보가 들어있습니다.
	{
	case SC_PUT_PLAYER://클라이언트 정보에 대한 패킷.
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;

		if (first_time) //패킷을 처음 주고 받았을 때 == 내가 접속했을 때
		{
			first_time = false; //이젠 처음이 아니므로 false로 전환
			g_myid = id; //그렇다면 내 아이디는 패킷의 아이디와 같겠죠.
		}
		if (id == g_myid) { // 내 아이디와 패킷의 아이디가 같다면? == 위의 조건문을 돌았다 == 내 정보
			g_my_info.pos = my_packet->m_pos; // 내 플레이어 구조체에 패킷의 좌표값을 저장합니다.
			g_my_info.roomnumb = my_packet->roomnumb;
			printf("My Roomnumber is [%d]\n", g_my_info.roomnumb);
		}
		else {
			g_player_info[id].m_isconnected = true;
			g_player_info[id].pos = my_packet->m_pos;
			g_player_info[id].roomnumb = my_packet->roomnumb;
			printf("Other Client [%d] is Connect to Server", id);
		}
		break;
	}
	case SC_POS://좌표값에대한 패킷.
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;
		if (id == g_myid) {//내 캐릭터에 대한 패킷이라면

		}
		else //다른 캐릭터의 패킷이라면
		{
			g_player_info[id].pos = my_packet->m_pos;
			printf("packet: Move Character! %f , %f , %f\n", my_packet->m_pos._41, my_packet->m_pos._42, my_packet->m_pos._43);
		}
		break;
	}

	case SC_REMOVE_PLAYER://접속 종료에 따른 패킷
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int id = my_packet->id;
		if (id == g_myid) {//내 캐릭터의 종료라면??

		}
		else {//다른 캐릭터의 종료라면??

			g_player_info[id].m_isconnected = false;

		}
		break;
	}
	case SC_READY://레디상태에 대한 체크
	{
		sc_packet_ready *my_packet = reinterpret_cast<sc_packet_ready *>(ptr);
		int id = my_packet->id;
		if (id == g_myid)
		{
			g_my_info.m_isready = my_packet->state;
			if (true == g_my_info.m_isready)
				printf("My Client is Ready\n");
			else
				printf("My Client is Not Ready\n");
		}
		else
		{//다른 캐릭터의 준비 패킷이라면?
			g_player_info[id].m_isready = my_packet->state;
			{
				if (true == g_player_info[id].m_isready)
					printf("Client [%d] is Ready\n", id);
				else
					printf("Client [%d] is Not Ready\n", id);
			}
		}
		break;
	}
	case SC_SCENE_CHANGE:
	{
		sc_packet_scene_change *my_packet = reinterpret_cast<sc_packet_scene_change *>(ptr);
		g_my_info.m_scene = my_packet->scenestate;
		printf("씬 체인지!\n");
		break;
	}

	/*case SC_CHAT:
	{
	sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(ptr);
	int other_id = my_packet->id;
	if (other_id == g_myid) {
	wcsncpy_s(player.message, my_packet->message, 256);
	player.message_time = GetTickCount();
	}
	else if (other_id < NPC_START) {
	wcsncpy_s(skelaton[other_id].message, my_packet->message, 256);
	skelaton[other_id].message_time = GetTickCount();
	}
	else {
	wcsncpy_s(npc[other_id - NPC_START].message, my_packet->message, 256);
	npc[other_id - NPC_START].message_time = GetTickCount();
	}
	break;

	}*/
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void CGameFramework::ReadPacket(SOCKET sock)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(sock, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}