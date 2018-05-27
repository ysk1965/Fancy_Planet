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

	m_ppd3dCommandAllocators = new ID3D12CommandAllocator*[NUM_COMMANDLIST];
	m_ppd3dCommandLists = new ID3D12GraphicsCommandList*[NUM_COMMANDLIST];

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

	m_ptOldCursorPos.x = m_nWndClientWidth / 2;
	m_ptOldCursorPos.y = m_nWndClientHeight / 2;

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

	CreateUIShader();
}

void CGameFramework::CreateUIShader()
{
	m_pMiniUIShader = new MiniUIShader();
	m_pMiniUIShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pMiniUIShader->CreateShader(m_pd3dDevice, m_pMiniUIShader->GetGraphicsRootSignature(), 4);
	m_pMiniUIShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);

	m_pMiniMapShader = new MiniMapShader(m_pPlayer);
	m_pMiniMapShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pMiniMapShader->CreateShader(m_pd3dDevice, m_pMiniMapShader->GetGraphicsRootSignature(), 4);
	m_pMiniMapShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);

	m_pArrowShader = new ArrowShader(m_pPlayer);
	m_pArrowShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pArrowShader->CreateShader(m_pd3dDevice, m_pArrowShader->GetGraphicsRootSignature(), 4);
	m_pArrowShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);

	m_pCrossShader = new CrossShader();
	m_pCrossShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pCrossShader->CreateShader(m_pd3dDevice, m_pCrossShader->GetGraphicsRootSignature(), 4);
	m_pCrossShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);

	m_pScoreboardShader = new ScoreBoardShader();
	m_pScoreboardShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pScoreboardShader->CreateShader(m_pd3dDevice, m_pScoreboardShader->GetGraphicsRootSignature(), 4);
	m_pScoreboardShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);

	m_pTimeNumberShader = new TimeNumberShader();
	m_pTimeNumberShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pTimeNumberShader->CreateShader(m_pd3dDevice, m_pTimeNumberShader->GetGraphicsRootSignature(), 4);
	m_pTimeNumberShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);

	m_pGravityPointerShader = new GravityPointerShader(m_pPxScene);
	m_pGravityPointerShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pGravityPointerShader->CreateShader(m_pd3dDevice, m_pGravityPointerShader->GetGraphicsRootSignature(), 4);
	m_pGravityPointerShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);

	m_pHPNumberShader = new HPNumberShader();
	m_pHPNumberShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pHPNumberShader->CreateShader(m_pd3dDevice, m_pTimeNumberShader->GetGraphicsRootSignature(), 4);
	m_pHPNumberShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);
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
		for (int i = 0; i < NUM_SUBSETS; i++)
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
			keystate = false;
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->animstate = 0;
			my_pos_packet->type = CS_POS;
			my_pos_packet->m_pos._11 = GetPlayerMatrix()._11;
			my_pos_packet->m_pos._12 = GetPlayerMatrix()._12;
			my_pos_packet->m_pos._13 = GetPlayerMatrix()._13;
			my_pos_packet->m_pos._21 = GetPlayerMatrix()._21;
			my_pos_packet->m_pos._22 = GetPlayerMatrix()._22;
			my_pos_packet->m_pos._23 = GetPlayerMatrix()._23;
			my_pos_packet->m_pos._31 = GetPlayerMatrix()._31;
			my_pos_packet->m_pos._32 = GetPlayerMatrix()._32;
			my_pos_packet->m_pos._33 = GetPlayerMatrix()._33;
			my_pos_packet->m_pos._41 = GetPlayerMatrix()._41;
			my_pos_packet->m_pos._42 = GetPlayerMatrix()._42;
			my_pos_packet->m_pos._43 = GetPlayerMatrix()._43;


			g_my_info.pos = GetPlayerMatrix();
			printf("이동 캐릭터 포지션 %f,%f,%f\n", GetPlayerMatrix()._41, GetPlayerMatrix()._42, GetPlayerMatrix()._43);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			break;
		}
		case 'A':
		{
			keystate = false;
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->animstate = 0;
			my_pos_packet->m_pos._11 = GetPlayerMatrix()._11;
			my_pos_packet->m_pos._12 = GetPlayerMatrix()._12;
			my_pos_packet->m_pos._13 = GetPlayerMatrix()._13;
			my_pos_packet->m_pos._21 = GetPlayerMatrix()._21;
			my_pos_packet->m_pos._22 = GetPlayerMatrix()._22;
			my_pos_packet->m_pos._23 = GetPlayerMatrix()._23;
			my_pos_packet->m_pos._31 = GetPlayerMatrix()._31;
			my_pos_packet->m_pos._32 = GetPlayerMatrix()._32;
			my_pos_packet->m_pos._33 = GetPlayerMatrix()._33;
			my_pos_packet->m_pos._41 = GetPlayerMatrix()._41;
			my_pos_packet->m_pos._42 = GetPlayerMatrix()._42;
			my_pos_packet->m_pos._43 = GetPlayerMatrix()._43;
			send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			//printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			break;
		}
		case 'S':
		{
			keystate = false;
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->animstate = 0;
			my_pos_packet->m_pos._11 = GetPlayerMatrix()._11;
			my_pos_packet->m_pos._12 = GetPlayerMatrix()._12;
			my_pos_packet->m_pos._13 = GetPlayerMatrix()._13;
			my_pos_packet->m_pos._21 = GetPlayerMatrix()._21;
			my_pos_packet->m_pos._22 = GetPlayerMatrix()._22;
			my_pos_packet->m_pos._23 = GetPlayerMatrix()._23;
			my_pos_packet->m_pos._31 = GetPlayerMatrix()._31;
			my_pos_packet->m_pos._32 = GetPlayerMatrix()._32;
			my_pos_packet->m_pos._33 = GetPlayerMatrix()._33;
			my_pos_packet->m_pos._41 = GetPlayerMatrix()._41;
			my_pos_packet->m_pos._42 = GetPlayerMatrix()._42;
			my_pos_packet->m_pos._43 = GetPlayerMatrix()._43;
			send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			//printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			break;
		}
		case 'D':
		{
			keystate = false;
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->animstate = 0;
			my_pos_packet->m_pos._11 = GetPlayerMatrix()._11;
			my_pos_packet->m_pos._12 = GetPlayerMatrix()._12;
			my_pos_packet->m_pos._13 = GetPlayerMatrix()._13;
			my_pos_packet->m_pos._21 = GetPlayerMatrix()._21;
			my_pos_packet->m_pos._22 = GetPlayerMatrix()._22;
			my_pos_packet->m_pos._23 = GetPlayerMatrix()._23;
			my_pos_packet->m_pos._31 = GetPlayerMatrix()._31;
			my_pos_packet->m_pos._32 = GetPlayerMatrix()._32;
			my_pos_packet->m_pos._33 = GetPlayerMatrix()._33;
			my_pos_packet->m_pos._41 = GetPlayerMatrix()._41;
			my_pos_packet->m_pos._42 = GetPlayerMatrix()._42;
			my_pos_packet->m_pos._43 = GetPlayerMatrix()._43;
			send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			//printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
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
					//printf("Error while sending packet [%d]", error_code);
				}
			}
			break;
		}

		case 'Z':
		{
			cs_gravity_test * my_packet = reinterpret_cast<cs_gravity_test *>(send_buffer);
			my_packet->size = sizeof(my_packet);
			send_wsabuf.len = sizeof(my_packet);
			my_packet->type = CS_GRAVITY_TEST;

			DWORD iobyte;
			int ret = WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

			if (ret) {
				int error_code = WSAGetLastError();
				//printf("Error while sending packet [%d]", error_code);

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
		for (int i = 0; i < NUM_COMMANDLIST; i++)
			m_ppd3dCommandAllocators[i]->Release();
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

	//Physx SDK Release=======================
	ReleasePhysxEngine();
	//========================================
}

void CGameFramework::BuildObjects()
{
	ShowCursor(false);

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		m_ppd3dCommandAllocators[i]->Reset();
		m_ppd3dCommandLists[i]->Reset(m_ppd3dCommandAllocators[i], NULL);
	}
	m_nDivision = 10;

	m_ppScenes = new CScene*[NUM_SUBSETS];

	int i = 0;

	TerrainAndSkyBoxScene *pTScene = new TerrainAndSkyBoxScene(m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	pTScene->BuildObjects(m_pd3dDevice, m_ppd3dCommandLists[i]);
	m_ppScenes[i++] = pTScene;

	UIScene *pUScene = new UIScene();
	m_ppScenes[i++] = pUScene;

	CharacterScene *pCScene = new CharacterScene(m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	m_ppScenes[i++] = pCScene;

	ObjectScene *pOScene = new ObjectScene(m_pPxPhysicsSDK, m_pPxScene, m_pPxControllerManager, m_pCooking);
	m_ppScenes[i++] = pOScene;

	m_ppScenes[TERRAIN]->m_pPlayer = m_pPlayer = new CPlayer(m_pd3dDevice, m_ppd3dCommandLists[2], m_ppScenes[0]->GetGraphicsRootSignature(), m_ppScenes[TERRAIN]->GetTerrain(), 0);
	m_pPlayer->BuildObject(m_pPxPhysicsSDK, m_pPxScene, m_pPxMaterial, m_pPxControllerManager, m_ppScenes[TERRAIN]->GetTerrain());
	m_pPlayer->ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);

	for (int j = PHYSX; j < NUM_SUBSETS; j++)
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
		for (int i = 0; i < NUM_SUBSETS; i++)
			m_ppScenes[i]->ReleaseUploadBuffers();

	m_GameTimer.Reset();
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

	if (m_pScreenShader)
		m_pScreenShader->ReleaseObjects();

	if (m_pScreenShader)
		delete m_pScreenShader;

	if (m_pMiniUIShader)
		m_pMiniUIShader->ReleaseObjects();

	if (m_pMiniUIShader)
		delete m_pMiniUIShader;

	if (m_pMiniMapShader)
		m_pMiniMapShader->ReleaseObjects();

	if (m_pMiniMapShader)
		delete m_pMiniMapShader;

	if (m_pArrowShader)
		m_pArrowShader->ReleaseObjects();

	if (m_pArrowShader)
		delete m_pArrowShader;

	if (m_pCrossShader)
		m_pCrossShader->ReleaseObjects();

	if (m_pCrossShader)
		delete m_pCrossShader;

	if (m_pScoreboardShader)
		m_pScoreboardShader->ReleaseObjects();

	if (m_pScoreboardShader)
		delete m_pScoreboardShader;

	if (m_pTimeNumberShader)
		m_pTimeNumberShader->ReleaseObjects();

	if (m_pTimeNumberShader)
		delete m_pTimeNumberShader;

	if (m_pGravityPointerShader)
		m_pGravityPointerShader->ReleaseObjects();

	if (m_pGravityPointerShader)
		delete m_pGravityPointerShader;

	if (m_pHPNumberShader)
		m_pHPNumberShader->ReleaseObjects();

	if (m_pHPNumberShader)
		delete m_pHPNumberShader;
}
void CGameFramework::CollisionCheckByBullet()
{
	if (m_pPlayer->GetBeAttackedState())
	{
		cs_packet_attacked * my_attacked_packet = reinterpret_cast<cs_packet_attacked *>(send_buffer);
		my_attacked_packet->size = sizeof(cs_packet_attacked);
		send_wsabuf.len = sizeof(cs_packet_attacked);
		my_attacked_packet->type = CS_ATTACKED;

		send_wsabuf.buf = reinterpret_cast<char *>(my_attacked_packet);

		DWORD iobyte;
		WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		m_pPlayer->SetBeAttackedStateToFalse();
	}
	if (g_my_info.hp <= 0 && g_my_info.state != DIED)
	{
		cs_packet_character_state * my_state_packet = reinterpret_cast<cs_packet_character_state *>(send_buffer);
		my_state_packet->size = sizeof(cs_packet_character_state);
		send_wsabuf.len = sizeof(cs_packet_character_state);
		my_state_packet->type = CS_PLAYER_STATE_CHANGE;
		my_state_packet->state = DIED;
		g_my_info.state = DIED;
		my_state_packet->id = g_myid;
		g_player_info[g_myid].state = DIED;
		send_wsabuf.buf = reinterpret_cast<char *>(my_state_packet);

		DWORD iobyte;
		WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	}
}
void CGameFramework::NotifyIdleState()
{
	std::chrono::milliseconds ms;

	ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - idlestatepoint);

	if (g_my_info.anim_state == 0 && ms.count()>500 && g_my_info.state != DIED)
	{
		cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
		my_pos_packet->size = sizeof(cs_packet_pos);
		send_wsabuf.len = sizeof(cs_packet_pos);
		my_pos_packet->type = CS_POS;
		my_pos_packet->animstate = 0;
		my_pos_packet->m_pos._11 = GetPlayerMatrix()._11;
		my_pos_packet->m_pos._12 = GetPlayerMatrix()._12;
		my_pos_packet->m_pos._13 = GetPlayerMatrix()._13;
		my_pos_packet->m_pos._21 = GetPlayerMatrix()._21;
		my_pos_packet->m_pos._22 = GetPlayerMatrix()._22;
		my_pos_packet->m_pos._23 = GetPlayerMatrix()._23;
		my_pos_packet->m_pos._31 = GetPlayerMatrix()._31;
		my_pos_packet->m_pos._32 = GetPlayerMatrix()._32;
		my_pos_packet->m_pos._33 = GetPlayerMatrix()._33;
		my_pos_packet->m_pos._41 = GetPlayerMatrix()._41;
		my_pos_packet->m_pos._42 = GetPlayerMatrix()._42;
		my_pos_packet->m_pos._43 = GetPlayerMatrix()._43;
		send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
		g_my_info.pos = GetPlayerMatrix();
		DWORD iobyte;
		WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		idlestatepoint = chrono::system_clock::now();
	}
}

void CGameFramework::SendBulletPacket(UINT nShell)
{

	cs_packet_shot * my_shot_packet = reinterpret_cast<cs_packet_shot *>(send_buffer);
	my_shot_packet->size = sizeof(cs_packet_shot);
	send_wsabuf.len = sizeof(cs_packet_shot);
	my_shot_packet->type = CS_SHOT;
	my_shot_packet->animstate = 6;
	my_shot_packet->id = g_myid;
	my_shot_packet->getprojectile = m_ppScenes[OBJECT]->GetProjectilePos(nShell);


	DWORD iobyte;
	WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
}



void CGameFramework::ProcessInput()
{
	static UCHAR pKeysBuffer[256];
	static bool bJumpState = false;
	bool bProcessedByScene = false;
	bool IsInput = false;
	bool bPlayerKeySwitch = m_pPlayer->GetKeySwitch();

	std::chrono::milliseconds ms;
	ms = std::chrono::duration_cast<std::chrono::milliseconds>(chrono::system_clock::now() - start);

	for (int i = 0; i < FLAGE_NUM; i++)
	{

		if (m_pPlayer->GetFlag(i) && g_my_info.state != DIED)
		{
			switch (i)
			{
			case JUMP:
			{
				break;
			}
			case LAUNCH:
			{
				if ((GetTickCount() - m_pPlayer->GetStartTime(LAUNCH)) > (DWORD)580 &&
					m_pPlayer->GetRenderObject()->m_pAnimationFactors->m_iState == 4)
				{
					POINT ptCursorPos;
 					GetCursorPos(&ptCursorPos);
					m_pPlayer->SetMousPos(ptCursorPos.x, ptCursorPos.y);

					CalculatePickRay(m_pPlayer->GetMousPos());

					UINT nShell = 0;
					nShell = m_ppScenes[OBJECT]->SetProjectile(xmf3PickDirection);
					m_pPlayer->SetFlag(LAUNCH, false);

					SendBulletPacket(nShell);
				}
				break;
			}
			case ETC:
			{
				break;
			}
			}
		}
	}

	if (bPlayerKeySwitch&&g_my_info.state != DIED)
	{
		if (GetKeyboardState(pKeysBuffer) && m_ppScenes[CHARACTER])
			bProcessedByScene = m_ppScenes[CHARACTER]->ProcessInput(pKeysBuffer);

		if (!bProcessedByScene)
		{
			DWORD dwDirection = 0;
			if (m_pPlayer->GetRenderObject()->m_pAnimationFactors->m_iState != 4)
			{
				if (pKeysBuffer[87] & 0xF0)
				{
					dwDirection |= DIR_FORWARD;
					g_player_info[g_myid].anim_state = 1;
					g_my_info.anim_state = 1;
					if (!m_pPlayer->GetJumpState())
					{
						m_ppScenes[CHARACTER]->ChangeAnimation(1);
						IsInput = true;
					}
					keystate = true;
				}
				else if (pKeysBuffer[83] & 0xF0)
				{
					dwDirection |= DIR_BACKWARD;
					g_player_info[g_myid].anim_state = -1;
					g_my_info.anim_state = -1;
					if (!m_pPlayer->GetJumpState())
					{
						m_ppScenes[CHARACTER]->ChangeAnimation(-1);
						IsInput = true;
					}
					keystate = true;
				}
				else if (pKeysBuffer[65] & 0xF0)
				{
					dwDirection |= DIR_LEFT;
					g_player_info[g_myid].anim_state = 2;
					g_my_info.anim_state = 2;
					if (!m_pPlayer->GetJumpState())
					{
						m_ppScenes[CHARACTER]->ChangeAnimation(2);
						IsInput = true;
					}
					keystate = true;
				}
				else if (pKeysBuffer[68] & 0xF0)
				{
					dwDirection |= DIR_RIGHT;
					g_player_info[g_myid].anim_state = 3;
					g_my_info.anim_state = 3;
					if (!m_pPlayer->GetJumpState())
					{
						m_ppScenes[CHARACTER]->ChangeAnimation(3);
						IsInput = true;
					}
					keystate = true;
				}
				if (pKeysBuffer[VK_NEXT] & 0xF0)
				{
					dwDirection |= DIR_DOWN;
					IsInput = true;
					keystate = true;
				}
				if (pKeysBuffer[VK_SPACE] & 0xF0) // jump
				{
					if (m_pPlayer->IsOnGround())
					{
						g_fgravity = -10.1f;
						m_pPxScene->setGravity(PxVec3(0, g_fgravity, 0));
						m_pPlayer->SetFallVelocity(100);
						m_pPlayer->SetJumpState(true);
						g_player_info[g_myid].anim_state = 1;
						g_my_info.anim_state = 1;
						m_ppScenes[CHARACTER]->ChangeAnimation(6);
						IsInput = true;
						keystate = true;
					}
				}
				if (pKeysBuffer[VK_LSHIFT] & 0xF0)
				{

					m_ppScenes[CHARACTER]->ChangeAnimation(9);
					IsInput = true;
					keystate = true;
				}
			}
			float cxDelta = 0.0f, cyDelta = 0.0f;
			POINT ptCursorPos;
			if (true) // GetCapture() == m_hWnd [카메라 고정]
			{
				GetCursorPos(&ptCursorPos);
				if (GetCapture() == m_hWnd 
					&& (m_pPlayer->GetRenderObject()->m_pAnimationFactors->m_iState != 4))
				{
					m_ppScenes[CHARACTER]->ChangeAnimation(4);
					g_player_info[g_myid].anim_state = 4;
					g_my_info.anim_state = 4;
					cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
					my_pos_packet->size = sizeof(cs_packet_pos);
					send_wsabuf.len = sizeof(cs_packet_pos);
					my_pos_packet->type = CS_POS;
					my_pos_packet->animstate = g_my_info.anim_state;
					my_pos_packet->m_pos._11 = GetPlayerMatrix()._11;
					my_pos_packet->m_pos._12 = GetPlayerMatrix()._12;
					my_pos_packet->m_pos._13 = GetPlayerMatrix()._13;
					my_pos_packet->m_pos._21 = GetPlayerMatrix()._21;
					my_pos_packet->m_pos._22 = GetPlayerMatrix()._22;
					my_pos_packet->m_pos._23 = GetPlayerMatrix()._23;
					my_pos_packet->m_pos._31 = GetPlayerMatrix()._31;
					my_pos_packet->m_pos._32 = GetPlayerMatrix()._32;
					my_pos_packet->m_pos._33 = GetPlayerMatrix()._33;
					my_pos_packet->m_pos._41 = GetPlayerMatrix()._41;
					my_pos_packet->m_pos._42 = GetPlayerMatrix()._42;
					my_pos_packet->m_pos._43 = GetPlayerMatrix()._43;
					send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
					g_my_info.pos = GetPlayerMatrix();
					DWORD iobyte;
					WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
					keystate = false;

					m_pPlayer->SetStartTime(GetTickCount(), LAUNCH);
					m_pPlayer->SetFlag(LAUNCH, true);
				}

				cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
				cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
				SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
			}

			if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
			{
				if (cxDelta || cyDelta)
				{
					m_pPlayer->Rotate(cyDelta / 4, 0.0f, -cxDelta / 4);
					m_pPlayer->Rotate(cyDelta / 4, cxDelta / 4, 0.0f);
				}
				if (dwDirection)
					m_pPlayer->Move(dwDirection, 60.0f * m_GameTimer.GetTimeElapsed(), true);
			}
		}
	}
	if (!IsInput)
	{
		if (g_my_info.state != DIED)
		{
			if (!m_pPlayer->GetJumpState())// 
			{
				g_player_info[g_myid].anim_state = 0;
				g_my_info.anim_state = 0;
				m_ppScenes[CHARACTER]->ChangeAnimation(0);
				NotifyIdleState();

			}
			else
			{
				g_player_info[g_myid].anim_state = 6;
				g_my_info.anim_state = 6;
				m_ppScenes[CHARACTER]->ChangeAnimation(6);
			}
		}
		else
		{
			g_player_info[g_myid].anim_state = 8;
			g_my_info.anim_state = 8;
			m_ppScenes[CHARACTER]->ChangeAnimation(8);
		}
	}

	//if(bJumpState && !m_pPlayer->GetJumpState())
	//   m_ppScenes[CHARACTER]->ChangeAnimation(7);

	bJumpState = m_pPlayer->GetJumpState();
	if (keystate == true || m_pPlayer->GetJumpState())
	{
		if (ms.count() > 33)
		{
			start = chrono::system_clock::now();
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->animstate = g_my_info.anim_state;
			my_pos_packet->m_pos._11 = GetPlayerMatrix()._11;
			my_pos_packet->m_pos._12 = GetPlayerMatrix()._12;
			my_pos_packet->m_pos._13 = GetPlayerMatrix()._13;
			my_pos_packet->m_pos._21 = GetPlayerMatrix()._21;
			my_pos_packet->m_pos._22 = GetPlayerMatrix()._22;
			my_pos_packet->m_pos._23 = GetPlayerMatrix()._23;
			my_pos_packet->m_pos._31 = GetPlayerMatrix()._31;
			my_pos_packet->m_pos._32 = GetPlayerMatrix()._32;
			my_pos_packet->m_pos._33 = GetPlayerMatrix()._33;
			my_pos_packet->m_pos._41 = GetPlayerMatrix()._41;
			my_pos_packet->m_pos._42 = GetPlayerMatrix()._42;
			my_pos_packet->m_pos._43 = GetPlayerMatrix()._43;
			send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			keystate = false;
		}
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}
void CGameFramework::CalculatePickRay(const XMFLOAT2& xmf2MousePos)
{
	XMFLOAT3 xmf3Vec;

	XMFLOAT4X4 projMatrix = m_pCamera->GetProjectionMatrix();
	xmf3Vec.x = ((2.0f * 512 / FRAME_BUFFER_WIDTH) - 1) / projMatrix._11;
	xmf3Vec.y = -((2.0f * 380 / FRAME_BUFFER_HEIGHT) - 1) / projMatrix._22;
	xmf3Vec.z = 1.0f;

	XMFLOAT4X4 invWorldView = m_pCamera->GetViewMatrix();
	invWorldView = Matrix4x4::Inverse(invWorldView);

	xmf3PickDirection.x = xmf3Vec.x * invWorldView._11 + xmf3Vec.y * invWorldView._21 + xmf3Vec.z * invWorldView._31;
	xmf3PickDirection.y = xmf3Vec.x * invWorldView._12 + xmf3Vec.y * invWorldView._22 + xmf3Vec.z * invWorldView._32;
	xmf3PickDirection.z = xmf3Vec.x * invWorldView._13 + xmf3Vec.y * invWorldView._23 + xmf3Vec.z * invWorldView._33;
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
		m_ppd3dCommandLists[i]->RSSetViewports(1, m_pCamera->GetViewport());
		m_ppd3dCommandLists[i]->RSSetScissorRects(1, m_pCamera->GetScissorRect());
	}
	m_pd3dScreenCommandAllocator->Reset();
	m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);
	m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
	m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

	float pfClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[i], D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		m_pd3dScreenCommandList->ClearRenderTargetView(m_pd3dRtvRenderTargetBufferCPUHandles[i], pfClearColor, 0, NULL);
	}
	m_pd3dScreenCommandList->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	m_ppd3dCommandLists[0]->OMSetRenderTargets(4, m_pd3dRtvRenderTargetBufferCPUHandles, TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);
	m_ppd3dCommandLists[1]->OMSetRenderTargets(4, m_pd3dRtvRenderTargetBufferCPUHandles, TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);
	m_ppd3dCommandLists[2]->OMSetRenderTargets(4, m_pd3dRtvRenderTargetBufferCPUHandles, TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);
	m_ppd3dCommandLists[3]->OMSetRenderTargets(4, m_pd3dRtvRenderTargetBufferCPUHandles, TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);
}

void CGameFramework::RenderUI()
{
	m_pMiniUIShader->Render(m_pd3dScreenCommandList, m_pCamera);
	m_pArrowShader->Render(m_pd3dScreenCommandList, m_pCamera);
	m_pMiniMapShader->Render(m_pd3dScreenCommandList, m_pCamera);
	m_pCrossShader->Render(m_pd3dScreenCommandList, m_pCamera);
	m_pTimeNumberShader->Render(m_pd3dScreenCommandList, m_pCamera);
	m_pScoreboardShader->Render(m_pd3dScreenCommandList, m_pCamera);
	m_pGravityPointerShader->Render(m_pd3dScreenCommandList, m_pCamera);
	m_pHPNumberShader->Render(m_pd3dScreenCommandList, m_pCamera);
}

void CGameFramework::FrameAdvance()
{
	if (m_pPxScene)
	{
		m_pPxScene->simulate(1 / 10.f);
		m_pPxScene->fetchResults(true);
	}

	m_ppScenes[PHYSX]->AnimateObjects(0, m_pCamera);
	m_ppScenes[OBJECT]->AnimateObjects(0, m_pCamera);

	m_GameTimer.Tick(0.0f);
	ProcessInput();
	AnimateObjects();
	PrepareFrame();
	SpaceDivision();

	m_pPxPhysicsSDK->getVisualDebugger()->updateCamera("PhysXCamera",
		PxVec3(m_pPlayer->GetCamera()->GetPosition().x, m_pPlayer->GetCamera()->GetPosition().y, m_pPlayer->GetCamera()->GetPosition().z),
		PxVec3(m_pPlayer->GetCamera()->GetUpVector().x, m_pPlayer->GetCamera()->GetUpVector().y, m_pPlayer->GetCamera()->GetUpVector().z),
		PxVec3(
			m_pPlayer->GetCamera()->GetPosition().x + m_pPlayer->GetCamera()->GetLookVector().x,
			m_pPlayer->GetCamera()->GetPosition().y + m_pPlayer->GetCamera()->GetLookVector().y,
			m_pPlayer->GetCamera()->GetPosition().z + m_pPlayer->GetCamera()->GetLookVector().z
		));

	m_pTimeNumberShader->SetAndCalculateScoreLocation(60 - g_fgametime);
	m_pHPNumberShader->SetAndCalculateHPLocation(g_my_info.hp);
	m_ppScenes[CHARACTER]->ModelsSetPosition(g_player_info, g_myid);
	CollisionCheckByBullet();//총알충돌체크
	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		SetEvent(m_workerBeginRenderFrame[i]);
	}
	WaitForMultipleObjects(NUM_SUBSETS, m_workerFinishedRenderFrame, TRUE, INFINITE);

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dScreenCommandList, m_ppd3dCommandLists[0],  m_ppd3dCommandLists[1],  m_ppd3dCommandLists[2], m_ppd3dCommandLists[3] };
	for (int i = 0; i < NUM_COMMANDLIST; i++)
		HRESULT hResult = m_ppd3dCommandLists[i]->Close();

	HRESULT hResult = m_pd3dScreenCommandList->Close();

	m_pd3dCommandQueue->ExecuteCommandLists(NUM_SUBSETS + 1, ppd3dCommandLists);

	WaitForGpuComplete();

	hResult = m_pd3dScreenCommandAllocator->Reset();
	hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);
	m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
	m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[0], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[1], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[2], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dRenderTargetBuffers[3], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_pd3dScreenCommandList->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	m_pd3dScreenCommandList->ClearRenderTargetView(m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], Colors::Azure, 0, NULL);
	m_pd3dScreenCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);
	//원래대로 백버퍼에 그린다.

	RenderUI();

	m_pScreenShader->Render(m_pd3dScreenCommandList, m_pCamera);

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
	sceneDesc.gravity = PxVec3(0.0f, 2 - rand() % 10, 0.0f); // 중력 값

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
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 재용
	//ServerAddr.sin_addr.s_addr = inet_addr("10.30.1.33");

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
			g_my_info.pos._11 = my_packet->m_pos._11; // 내 플레이어 구조체에 패킷의 좌표값을 저장합니다.
			g_my_info.pos._12 = my_packet->m_pos._12;
			g_my_info.pos._13 = my_packet->m_pos._13;
			g_my_info.pos._21 = my_packet->m_pos._21;
			g_my_info.pos._22 = my_packet->m_pos._22;
			g_my_info.pos._23 = my_packet->m_pos._23;
			g_my_info.pos._31 = my_packet->m_pos._31;
			g_my_info.pos._32 = my_packet->m_pos._32;
			g_my_info.pos._33 = my_packet->m_pos._33;
			g_my_info.pos._41 = my_packet->m_pos._41;
			g_my_info.pos._42 = my_packet->m_pos._42;
			g_my_info.pos._43 = my_packet->m_pos._43;
			g_my_info.pos._14 = 0.f;
			g_my_info.pos._24 = 0.f;
			g_my_info.pos._34 = 0.f;
			g_my_info.pos._44 = 1.f;
			g_my_info.roomnumb = 0;
			g_my_info.anim_state = my_packet->animstate;
			g_player_info[id].anim_state = my_packet->animstate;
			printf("animstate : %d\n", g_my_info.anim_state);

		}
		else {
			g_player_info[id].m_isconnected = true;
			g_player_info[id].pos._11 = my_packet->m_pos._11; // 내 플레이어 구조체에 패킷의 좌표값을 저장합니다.
			g_player_info[id].pos._12 = my_packet->m_pos._12;
			g_player_info[id].pos._13 = my_packet->m_pos._13;
			g_player_info[id].pos._21 = my_packet->m_pos._21;
			g_player_info[id].pos._22 = my_packet->m_pos._22;
			g_player_info[id].pos._23 = my_packet->m_pos._23;
			g_player_info[id].pos._31 = my_packet->m_pos._31;
			g_player_info[id].pos._32 = my_packet->m_pos._32;
			g_player_info[id].pos._33 = my_packet->m_pos._33;
			g_player_info[id].pos._41 = my_packet->m_pos._41;
			g_player_info[id].pos._42 = my_packet->m_pos._42;
			g_player_info[id].pos._43 = my_packet->m_pos._43;
			g_player_info[id].pos._14 = 0.f;
			g_player_info[id].pos._24 = 0.f;
			g_player_info[id].pos._34 = 0.f;
			g_player_info[id].pos._44 = 1.f;
			g_player_info[id].roomnumb = 0;
			g_player_info[id].anim_state = my_packet->animstate;
			printf("Other Client [%d] is Connect to Server", id);
			printf("animstate : %d\n", g_my_info.anim_state);

		}


		break;
	}
	case
		SC_POS://좌표값에대한 패킷.
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;

		g_player_info[id].pos._11 = my_packet->m_pos._11; // 내 플레이어 구조체에 패킷의 좌표값을 저장합니다.
		g_player_info[id].pos._12 = my_packet->m_pos._12;
		g_player_info[id].pos._13 = my_packet->m_pos._13;
		g_player_info[id].pos._21 = my_packet->m_pos._21;
		g_player_info[id].pos._22 = my_packet->m_pos._22;
		g_player_info[id].pos._23 = my_packet->m_pos._23;
		g_player_info[id].pos._31 = my_packet->m_pos._31;
		g_player_info[id].pos._32 = my_packet->m_pos._32;
		g_player_info[id].pos._33 = my_packet->m_pos._33;
		g_player_info[id].pos._41 = my_packet->m_pos._41;
		g_player_info[id].pos._42 = my_packet->m_pos._42;
		g_player_info[id].pos._43 = my_packet->m_pos._43;
		g_player_info[id].pos._14 = 0.f;
		g_player_info[id].pos._24 = 0.f;
		g_player_info[id].pos._34 = 0.f;
		g_player_info[id].pos._44 = 1.f;
		g_player_info[id].anim_state = my_packet->animstate;

		//printf("이동 캐릭터 [%d], 포지션 %f,%f,%f", id, g_player_info[id].pos._41, g_player_info[id].pos._42, g_player_info[id].pos._43);
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
		printf("씬 체인지! 씬 번호 : %d", my_packet->scenestate);
		if (my_packet->scenestate == 1)//게임시작 패킷일 때
		{
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;
			my_pos_packet->m_pos._11 = GetPlayerMatrix()._11;
			my_pos_packet->m_pos._12 = GetPlayerMatrix()._12;
			my_pos_packet->m_pos._13 = GetPlayerMatrix()._13;
			my_pos_packet->m_pos._21 = GetPlayerMatrix()._21;
			my_pos_packet->m_pos._22 = GetPlayerMatrix()._22;
			my_pos_packet->m_pos._23 = GetPlayerMatrix()._23;
			my_pos_packet->m_pos._31 = GetPlayerMatrix()._31;
			my_pos_packet->m_pos._32 = GetPlayerMatrix()._32;
			my_pos_packet->m_pos._33 = GetPlayerMatrix()._33;
			my_pos_packet->m_pos._41 = GetPlayerMatrix()._41;
			my_pos_packet->m_pos._42 = GetPlayerMatrix()._42;
			my_pos_packet->m_pos._43 = GetPlayerMatrix()._43;
			//my_pos_packet->roomnumb = g_my_info.roomnumb;
			my_pos_packet->animstate = g_my_info.anim_state;
			g_my_info.pos = GetPlayerMatrix();

			DWORD iobyte;
			printf("packet: Move Character! %f , %f , %f\n", my_pos_packet->m_pos._41, my_pos_packet->m_pos._42, my_pos_packet->m_pos._43);
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		}
		else if (my_packet->scenestate == 0)
		{
			g_my_info.m_scene = my_packet->scenestate;
			g_fgametime = 0.f;
			g_fgravity = 0.f;

		}
		break;
	}
	case SC_SHOT:
	{
		sc_packet_shot *my_packet = reinterpret_cast<sc_packet_shot *>(ptr);
		g_player_info[my_packet->id].anim_state = my_packet->animstate;
		printf("%d", my_packet->id);
		m_ppScenes[OBJECT]->SetVectorProjectile(my_packet->getprojectile.first, my_packet->getprojectile.second);
		break;
	}


	case SC_TIME:
	{
		sc_packet_time *my_packet = reinterpret_cast<sc_packet_time *>(ptr);
		g_fgametime = my_packet->m_ftime;
		printf("게임 시간 : %f", g_fgametime);

		break;
	}
	case SC_GRAVITY_CHANGE:
	{
		sc_gravity_change *my_packet = reinterpret_cast<sc_gravity_change*>(ptr);
		g_fgravity = my_packet->gravity_state;
		m_pPxScene->setGravity(PxVec3(0, g_fgravity, 0));
		printf("중력 변경 : %f\n", g_fgravity);
		break;
	}
	case SC_ATTACKED:
	{
		sc_packet_attacked *my_packet = reinterpret_cast<sc_packet_attacked*>(ptr);
		g_player_info[my_packet->id].hp = my_packet->hp;
		if (my_packet->id == g_myid && g_my_info.state != DIED)
		{
			g_my_info.hp = my_packet->hp;
		}
		else g_player_info[my_packet->id].hp = my_packet->hp;
		printf("%d 클라이언트가 공격받았습니다. hp : %d\n", my_packet->id, my_packet->hp);
		break;
	}
	case SC_PLAYER_STATE_CHANGE:
	{
		sc_packet_character_state *my_packet = reinterpret_cast<sc_packet_character_state*>(ptr);
		g_player_info[my_packet->id].state = my_packet->state;

		if (my_packet->state == DIED)
		{

			if (my_packet->id == g_myid)
			{
				printf("내 캐릭터가 죽었습니다\n");
				g_my_info.state = my_packet->state;
				m_ppScenes[CHARACTER]->ChangeAnimation(8); // 죽는 애니메이션
			}
			else
			{
				printf("플레이어 [%d]가 죽었습니다", my_packet->id);
				//다른 클라가 죽었을 때에 대한 처리
				g_player_info[my_packet->id].state = my_packet->state;
				g_player_info[my_packet->id].anim_state = 8;
			}
		}
		break;
	}
	default:
	{
		printf("Unknown PACKET type [%d]\n", ptr[1]);
		return;
	}
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
