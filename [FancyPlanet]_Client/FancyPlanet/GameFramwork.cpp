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
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	m_pComputeShader = new CComputShader(m_nWndClientWidth, m_nWndClientHeight);

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
	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		CloseHandle(m_workerBeginRenderFrame[i]);
		CloseHandle(m_workerFinishShadowPass[i]);
		CloseHandle(m_workerFinishedRenderFrame[i]);
		CloseHandle(m_threadHandles[i]);
	}
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
		m_workerFinishShadowPass[i] = CreateEvent(
			NULL,
			FALSE,
			FALSE,
			NULL
		);
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

	m_pComputeShader->CreateComputeRootSignature(m_pd3dDevice);
	m_pComputeShader->BuildPSO(m_pd3dDevice);
	m_pComputeShader->BuildTextures(m_pd3dDevice);

	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();

	BuildObjects();

	return(true);
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
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers + m_nRenderTargetBuffers;
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

void CGameFramework::CreateRenderTargetViews()
{
	CTexture *pTexture = new CTexture(m_nRenderTargetBuffers, RESOURCE_TEXTURE2D_ARRAY, 0);

	D3D12_CLEAR_VALUE d3dClearValue = { DXGI_FORMAT_R8G8B8A8_UNORM,{ 0.0f, 0.0f, 0.0f, 1.0f } };
	for (UINT i = 0; i < m_nRenderTargetBuffers; i++)
	{
		//���۸� �����.
		m_ppd3dRenderTargetBuffers[i] = pTexture->CreateTexture(m_pd3dDevice, m_nWndClientWidth, m_nWndClientHeight
			, DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ, &d3dClearValue, i);
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
		m_pd3dDevice->CreateRenderTargetView(pTexture->GetTexture(i), &d3dRenderTargetViewDesc, m_pd3dRtvRenderTargetBufferCPUHandles[i]);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
	m_pShadowShader = new CShadowShader(m_pd3dDevice, m_pd3dPreShadowCommandList, m_nWndClientWidth, m_nWndClientHeight);
	m_pShadowShader->SetPlayer(m_pPlayer);

	m_pScreenShader = new CTextureToFullScreenShader(m_pShadowShader->GetShadowMap());
	m_pScreenShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pScreenShader->CreateShader(m_pd3dDevice, m_pScreenShader->GetGraphicsRootSignature(), 4);
	m_pScreenShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList, m_pPlayer, pTexture);

	m_pUIShader = new UIShader(m_pPlayer);
	m_pUIShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pUIShader->CreateShader(m_pd3dDevice, 4);
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
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	m_nSwapChainBufferIndex = 0;
#endif

	CreateSwapChainRenderTargetViews();
	CreateDepthStencilView();
	CreateRenderTargetViews();

	hResult = m_pd3dScreenCommandList->Close();
	m_pd3dPreShadowCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_pd3dScreenCommandList, m_pd3dPreShadowCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(2, ppd3dCommandLists);

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

		default:
		{

			break;

		}
		break;

		case 'R':
		{
			cs_packet_ready * my_packet = reinterpret_cast<cs_packet_ready *>(send_buffer);
			my_packet->size = sizeof(my_packet);
			send_wsabuf.len = sizeof(my_packet);
			my_packet->type = CS_READY;
			my_packet->state = g_my_info.m_isready;
			DWORD iobyte;
			printf("packet : READY  \n");
			if (g_my_info.m_scene == 0)//������Ŷ�� �κ��ϴ븸
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
		case 'W':
		{
			g_player_info[g_myid].anim_state = ANIMATION_IDLE_STATE;
			g_my_info.anim_state = ANIMATION_IDLE_STATE;

			cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
			p->size = sizeof(cs_packet_pos);
			p->type = CS_POS;
			p->id = g_myid;
			p->keyInputState = ANIMATION_IDLE_STATE;
			send_wsabuf.len = sizeof(cs_packet_pos);
			send_wsabuf.buf = reinterpret_cast<char *>(p);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			printf("Ű�� �� \n");
			break;
		}
		case 'A':
		{
			g_player_info[g_myid].anim_state = ANIMATION_IDLE_STATE;
			g_my_info.anim_state = ANIMATION_IDLE_STATE;

			cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
			p->size = sizeof(cs_packet_pos);
			p->type = CS_POS;
			p->id = g_myid;
			p->keyInputState = ANIMATION_IDLE_STATE;
			send_wsabuf.len = sizeof(cs_packet_pos);
			send_wsabuf.buf = reinterpret_cast<char *>(p);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			printf("Ű�� �� \n");
			break;
		}
		case 'S':
		{
			g_player_info[g_myid].anim_state = ANIMATION_IDLE_STATE;
			g_my_info.anim_state = ANIMATION_IDLE_STATE;

			cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
			p->size = sizeof(cs_packet_pos);
			p->type = CS_POS;
			p->id = g_myid;
			p->keyInputState = ANIMATION_IDLE_STATE;
			send_wsabuf.len = sizeof(cs_packet_pos);
			send_wsabuf.buf = reinterpret_cast<char *>(p);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			printf("Ű�� �� \n");
			break;
		}
		case 'D':
		{
			g_player_info[g_myid].anim_state = ANIMATION_IDLE_STATE;
			g_my_info.anim_state = ANIMATION_IDLE_STATE;

			cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
			p->size = sizeof(cs_packet_pos);
			p->type = CS_POS;
			p->id = g_myid;
			p->keyInputState = ANIMATION_IDLE_STATE;
			send_wsabuf.len = sizeof(cs_packet_pos);
			send_wsabuf.buf = reinterpret_cast<char *>(p);
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			printf("Ű�� �� \n");
			break;
		}
		}
	case WM_KEYDOWN:
	{

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

void CGameFramework::BuildObjects()
{
	ShowCursor(false);

	HRESULT hResult;

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		hResult = m_ppd3dCommandAllocators[i]->Reset();
		hResult = m_ppd3dCommandLists[i]->Reset(m_ppd3dCommandAllocators[i], NULL);
	}

	m_nDivision = 10;

	m_ppScenes = new CScene*[NUM_SUBSETS];

	int i = 0;

	TerrainAndSkyBoxScene *pTScene = new TerrainAndSkyBoxScene();
	pTScene->BuildObjects(m_pd3dDevice, m_ppd3dCommandLists[i]);
	m_ppScenes[i++] = pTScene;

	UIScene *pUScene = new UIScene();
	m_ppScenes[i++] = pUScene;

	CharacterScene *pCScene = new CharacterScene();
	m_ppScenes[i++] = pCScene;

	ObjectScene *pOScene = new ObjectScene();
	m_ppScenes[i++] = pOScene;

	m_ppScenes[TERRAIN]->m_pPlayer = m_pPlayer = new CPlayer(m_pd3dDevice, m_ppd3dCommandLists[2], m_ppScenes[0]->GetGraphicsRootSignature(), m_ppScenes[TERRAIN]->GetTerrain(), 0);
	m_pPlayer->BuildObject(m_ppScenes[TERRAIN]->GetTerrain());
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
		hResult = m_ppd3dCommandLists[i]->Close();
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

	if (m_pUIShader)
		m_pUIShader->ReleaseObjects();

	if (m_pUIShader)
		delete m_pUIShader;

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

	if (g_my_info.anim_state == 0 && ms.count() > 500 && g_my_info.state != DIED)
	{
		cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
		my_pos_packet->size = sizeof(cs_packet_pos);
		send_wsabuf.len = sizeof(cs_packet_pos);
		my_pos_packet->type = CS_POS;

		send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
		g_my_info.pos = GetPlayerMatrix();
		DWORD iobyte;
		WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		idlestatepoint = chrono::system_clock::now();
	}
}

void CGameFramework::SendBulletPacket(UINT nShell)
{



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
					m_pPlayer->GetRenderObject()->m_pAnimationTime->m_iState == 4)
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
			if (m_pPlayer->GetRenderObject()->m_pAnimationTime->m_iState != 4)
			{
				if (pKeysBuffer[87] & 0xF0)
				{

					dwDirection |= DIR_FORWARD;
					g_player_info[g_myid].anim_state = ANIMATION_MOVE_FORWARD;
					g_my_info.anim_state = ANIMATION_MOVE_FORWARD;

					cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
					p->size = sizeof(cs_packet_pos);
					p->type = CS_POS;
					p->id = g_myid;
					p->keyInputState = ANIMATION_MOVE_FORWARD;
					send_wsabuf.len = sizeof(cs_packet_pos);
					send_wsabuf.buf = reinterpret_cast<char *>(p);
					DWORD iobyte;
					WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
					/*if (!m_pPlayer->GetJumpState())
					{
					m_ppScenes[CHARACTER]->ChangeAnimation(ANIMATION_MOVE_FORWARD);
					IsInput = true;
					}*/
					printf("�̵� ��Ŷ ���� [%d] \n", p->id);
					keystate = true;
				}
				else if (pKeysBuffer[83] & 0xF0)
				{
					dwDirection |= DIR_BACKWARD;
					g_player_info[g_myid].anim_state = ANIMATION_MOVE_BACKWARD;
					g_my_info.anim_state = ANIMATION_MOVE_BACKWARD;

					cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
					p->size = sizeof(cs_packet_pos);
					p->type = CS_POS;
					p->id = g_myid;
					p->keyInputState = ANIMATION_MOVE_BACKWARD;
					send_wsabuf.len = sizeof(cs_packet_pos);
					send_wsabuf.buf = reinterpret_cast<char *>(p);
					DWORD iobyte;
					WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
					printf("�̵� ��Ŷ ���� \n");
					keystate = true;
				}
				else if (pKeysBuffer[65] & 0xF0)
				{
					dwDirection |= DIR_LEFT;
					g_player_info[g_myid].anim_state = ANIMATION_MOVE_LEFT;
					g_my_info.anim_state = ANIMATION_MOVE_LEFT;
					cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
					p->size = sizeof(cs_packet_pos);
					p->type = CS_POS;
					p->id = g_myid;
					p->keyInputState = ANIMATION_MOVE_LEFT;
					send_wsabuf.len = sizeof(cs_packet_pos);
					send_wsabuf.buf = reinterpret_cast<char *>(p);
					DWORD iobyte;
					WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
					printf("�̵� ��Ŷ ���� \n");
					keystate = true;
				}
				else if (pKeysBuffer[68] & 0xF0)
				{
					dwDirection |= DIR_RIGHT;
					g_player_info[g_myid].anim_state = ANIMATION_MOVE_RIGHT;
					g_my_info.anim_state = ANIMATION_MOVE_RIGHT;
					cs_packet_pos *p = reinterpret_cast<cs_packet_pos *>(send_buffer);
					p->size = sizeof(cs_packet_pos);
					p->type = CS_POS;
					p->id = g_myid;
					p->keyInputState = ANIMATION_MOVE_RIGHT;
					send_wsabuf.len = sizeof(cs_packet_pos);
					send_wsabuf.buf = reinterpret_cast<char *>(p);
					DWORD iobyte;
					WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
					printf("�̵� ��Ŷ ���� \n");
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

					IsInput = true;
					keystate = true;

				}
				if (pKeysBuffer[VK_LSHIFT] & 0xF0)
				{

					m_ppScenes[CHARACTER]->ChangeAnimation(ANIMATION_MOVE_DANCE);
					IsInput = true;
					keystate = true;
				}
			}
			float cxDelta = 0.0f, cyDelta = 0.0f;
			POINT ptCursorPos;
			if (GetCapture() == m_hWnd) // GetCapture() == m_hWnd [ī�޶� ����]
			{
				GetCursorPos(&ptCursorPos);
				if (GetCapture() == m_hWnd
					&& (m_pPlayer->GetRenderObject()->m_pAnimationTime->m_iState != 4))
				{
					m_ppScenes[CHARACTER]->ChangeAnimation(4);
					g_player_info[g_myid].anim_state = 4;
					g_my_info.anim_state = 4;
					cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
					my_pos_packet->size = sizeof(cs_packet_pos);
					send_wsabuf.len = sizeof(cs_packet_pos);
					my_pos_packet->type = CS_POS;

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

					cs_packet_rotate* p = reinterpret_cast<cs_packet_rotate *>(send_buffer);
					send_wsabuf.len = sizeof(cs_packet_rotate);
					send_wsabuf.buf = reinterpret_cast<char *>(p);
					p->size = sizeof(cs_packet_rotate);
					p->type = CS_ROTATE;
					p->id = g_myid;
					p->xmvUP = m_pPlayer->GetUpVector();
					p->xmvRight = m_pPlayer->GetRightVector();
					p->xmvLook = m_pPlayer->GetLookVector();
					DWORD iobyte;
					WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);


				}
			}
		}
	}
	if (!IsInput)//Ű�� ������ �ʾ�����?
	{
		if (g_my_info.state != DIED)
		{
			if (!m_pPlayer->GetJumpState())//Ű�� �ȴ����ְ� �������ʰ� �������µ��ƴϴ�.
			{
				g_player_info[g_myid].anim_state = ANIMATION_IDLE_STATE;
				g_my_info.anim_state = ANIMATION_IDLE_STATE;
				m_ppScenes[CHARACTER]->ChangeAnimation(ANIMATION_IDLE_STATE);
				NotifyIdleState();

			}
			else//Ű�� �ȴ����ְ� �������¸� �����ϴµ��� ��� ���������
			{
				g_player_info[g_myid].anim_state = ANIMATION_MOVE_JUMP;
				g_my_info.anim_state = ANIMATION_MOVE_JUMP;
				m_ppScenes[CHARACTER]->ChangeAnimation(ANIMATION_MOVE_JUMP);
			}
		}
		else//�׾����� �������� �ִϸ��̼�
		{
			g_player_info[g_myid].anim_state = ANIMATION_DIED;
			g_my_info.anim_state = ANIMATION_DIED;
			m_ppScenes[CHARACTER]->ChangeAnimation(ANIMATION_DIED);
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
			my_pos_packet->keyInputState = ANIMATION_MOVE_JUMP;
			send_wsabuf.buf = reinterpret_cast<char *>(my_pos_packet);
			g_my_info.pos = GetPlayerMatrix();
			DWORD iobyte;
			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
			keystate = false;
		}
	}
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
	//m_pPlayer->SetWorldPosition(XMFLOAT3(g_my_info.pos._41, g_my_info.pos._42, g_my_info.pos._43));

	m_ppScenes[CHARACTER]->ChangeAnimation(g_my_info.anim_state);
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

		m_pShadowShader->OnPrepareRender(m_ppd3dShadowCommandLists[iIndex]);

		m_ppScenes[iIndex]->ShadowRender(m_ppd3dShadowCommandLists[iIndex], m_pCamera, m_pShadowShader->GetShadowInfo());

		m_ppd3dShadowCommandLists[iIndex]->Close();

		SetEvent(m_workerFinishShadowPass[iIndex]);

		m_ppd3dCommandLists[iIndex]->RSSetViewports(1, m_pCamera->GetViewport());
		m_ppd3dCommandLists[iIndex]->RSSetScissorRects(1, m_pCamera->GetScissorRect());
		m_ppd3dCommandLists[iIndex]->OMSetRenderTargets(4, m_pd3dRtvRenderTargetBufferCPUHandles, TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);

		m_ppScenes[iIndex]->Render(m_ppd3dCommandLists[iIndex], m_pCamera);

		m_ppd3dCommandLists[iIndex]->Close();

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

	m_pShadowShader->UpdateTransform();
}

void CGameFramework::FrameAdvance()
{
	m_ppScenes[PHYSX]->AnimateObjects(0, m_pCamera);
	m_ppScenes[OBJECT]->AnimateObjects(0, m_pCamera);

	m_GameTimer.Tick(0.0f);
	ProcessInput();
	AnimateObjects();
	PrepareFrame();
	SpaceDivision();

	m_pUIShader->SetAndCalculateScoreLocation(60 - g_fgametime);
	m_pUIShader->SetAndCalculateHPLocation(g_my_info.hp);

	m_ppScenes[CHARACTER]->ModelsSetPosition(g_player_info, g_myid);
	CollisionCheckByBullet();//�Ѿ��浹üũ

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		SetEvent(m_workerBeginRenderFrame[i]);
	}

	WaitForMultipleObjects(NUM_SUBSETS, m_workerFinishShadowPass, TRUE, INFINITE);

	ID3D12CommandList *ppd3dCommandLists1[] = { m_pd3dPreShadowCommandList
		, m_ppd3dShadowCommandLists[0],  m_ppd3dShadowCommandLists[1]
		,  m_ppd3dShadowCommandLists[2], m_ppd3dShadowCommandLists[3] };

	m_pd3dCommandQueue->ExecuteCommandLists(NUM_SUBSETS + 1, ppd3dCommandLists1);

	WaitForMultipleObjects(NUM_SUBSETS, m_workerFinishedRenderFrame, TRUE, INFINITE);

	ID3D12CommandList *ppd3dCommandLists2[] = { m_pd3dScreenCommandList
		, m_ppd3dCommandLists[0],  m_ppd3dCommandLists[1]
		,  m_ppd3dCommandLists[2], m_ppd3dCommandLists[3] };

	HRESULT hResult = m_pd3dScreenCommandList->Close();

	m_pd3dCommandQueue->ExecuteCommandLists(NUM_SUBSETS + 1, ppd3dCommandLists2);

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

	m_pScreenShader->UpdateShadowShaderVariables(m_pd3dScreenCommandList, m_pShadowShader->GetShadowInfo());
	m_pScreenShader->Render(m_pd3dScreenCommandList, m_pCamera);

	::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

	hResult = m_pd3dScreenCommandList->Close();

	ID3D12CommandList *ppd3dScreenCommandLists[] = { m_pd3dScreenCommandList };

	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);
	WaitForGpuComplete();

	//��꽦�̴� �κ�
	{
		hResult = m_pd3dScreenCommandAllocator->Reset();
		hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);
		m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
		m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

		m_pComputeShader->SetValue(1);
		m_pComputeShader->Compute(m_pd3dDevice, m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex]);

		hResult = m_pd3dScreenCommandList->Close();

		m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);

		WaitForGpuComplete();
	}

	//UI �׸���
	{
		hResult = m_pd3dScreenCommandAllocator->Reset();
		hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);
		m_pd3dScreenCommandList->RSSetViewports(1, m_pCamera->GetViewport());
		m_pd3dScreenCommandList->RSSetScissorRects(1, m_pCamera->GetScissorRect());

		m_pd3dScreenCommandList->ClearDepthStencilView(m_d3dDsvDepthStencilBufferCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
		m_pd3dScreenCommandList->OMSetRenderTargets(1, &m_pd3dRtvSwapChainBackBufferCPUHandles[m_nSwapChainBufferIndex], TRUE, &m_d3dDsvDepthStencilBufferCPUHandle);

		m_pUIShader->Render(m_pd3dScreenCommandList, m_pCamera);

		::SynchronizeResourceTransition(m_pd3dScreenCommandList, m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		hResult = m_pd3dScreenCommandList->Close();

		m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);
		WaitForGpuComplete();
	}

	m_pdxgiSwapChain->Present(0, 0);

	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}
void CGameFramework::InitNetwork(HWND main_window)
{
	string s_ip;
	//�����Լ����� �����մϴ�.
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	printf("Server IP INPUT : ");
	//scanf("%s", s_ip);

	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // ���
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
	sc_packet_pos *p = reinterpret_cast<sc_packet_pos*>(ptr);
	switch (p->type)//��Ŷ�� 1�� �ε������� ��Ŷ�� ������ ���� ������ ����ֽ��ϴ�.
	{
	case SC_PUT_PLAYER://Ŭ���̾�Ʈ ������ ���� ��Ŷ.
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(ptr);
		int id = my_packet->id;


		if (first_time) //��Ŷ�� ó�� �ְ� �޾��� �� == ���� �������� ��
		{
			first_time = false; //���� ó���� �ƴϹǷ� false�� ��ȯ
			g_myid = id; //�׷��ٸ� �� ���̵�� ��Ŷ�� ���̵�� ������.
		}
		if (id == g_myid) { // �� ���̵�� ��Ŷ�� ���̵� ���ٸ�? == ���� ���ǹ��� ���Ҵ� == �� ����
			g_player_info[id].m_isconnected = true;
			g_my_info.pos._14 = 0.f;
			g_my_info.pos._24 = 0.f;
			g_my_info.pos._34 = 0.f;
			g_my_info.pos._44 = 1.f;
			g_player_info[id].pos._14 = 0.f;
			g_player_info[id].pos._24 = 0.f;
			g_player_info[id].pos._34 = 0.f;
			g_player_info[id].pos._44 = 1.f;
			g_my_info.roomnumb = 0;

			cs_packet_requestObject * my_pos_packet = reinterpret_cast<cs_packet_requestObject *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_requestObject);
			send_wsabuf.len = sizeof(cs_packet_requestObject);
			my_pos_packet->type = CS_REQUEST_OBJECT;

			DWORD iobyte;

			WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

		}
		else {
			g_player_info[id].m_isconnected = true;
			g_player_info[id].pos._14 = 0.f;
			g_player_info[id].pos._24 = 0.f;
			g_player_info[id].pos._34 = 0.f;
			g_player_info[id].pos._44 = 1.f;
			g_player_info[id].roomnumb = 0;
			printf("Other Client [%d] is Connect to Server", id);

		}


		break;
	}
	case SC_POS://��ǥ�������� ��Ŷ.
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		int id = my_packet->id;

		if (g_myid == id)
		{
			g_my_info.pos._11 = my_packet->m_pos._11; // �� �÷��̾� ����ü�� ��Ŷ�� ��ǥ���� �����մϴ�.
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

		}
		g_player_info[id].pos._11 = my_packet->m_pos._11; // �� �÷��̾� ����ü�� ��Ŷ�� ��ǥ���� �����մϴ�.
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

		if (!m_pPlayer->GetJumpState())
		{

		}
		m_pPlayer->SetWorldPosition(g_my_info.pos);

		//printf("�̵� ĳ���� [%d], ������%f,%f,%f\n", id, g_player_info[id].pos._41, g_player_info[id].pos._42, g_player_info[id].pos._43);
		break;
	}
	case SC_ROTATE:
	{
		sc_packet_rotate *my_packet = reinterpret_cast<sc_packet_rotate *>(ptr);
		int id = my_packet->id;
		if (g_myid == id)
		{
			g_my_info.pos._11 = my_packet->m_pos._11;
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
		}

		g_player_info[id].pos._11 = my_packet->m_pos._11; // �� �÷��̾� ����ü�� ��Ŷ�� ��ǥ���� �����մϴ�.
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
		m_pPlayer->SetWorldPosition(g_my_info.pos);
		break;
	}
	case SC_REMOVE_PLAYER://���� ���ῡ ���� ��Ŷ
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(ptr);
		int id = my_packet->id;
		if (id == g_myid) {//�� ĳ������ ������??

		}
		else {//�ٸ� ĳ������ ������??

			g_player_info[id].m_isconnected = false;
			printf("[%d] Ŭ���̾�Ʈ ���� ����\n", id);
		}
		break;
	}
	case SC_READY://������¿� ���� üũ
	{
		sc_packet_ready *my_packet = reinterpret_cast<sc_packet_ready *>(ptr);
		int id = my_packet->id;
		if (id == g_myid)
		{

			if (true == g_my_info.m_isready)
				printf("My Client is Ready\n");
			else
				printf("My Client is Not Ready\n");
		}
		else
		{//�ٸ� ĳ������ �غ� ��Ŷ�̶��?

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
		printf("�� ü����! �� ��ȣ : %d", my_packet->scenestate);
		if (my_packet->scenestate == 1)//���ӽ��� ��Ŷ�� ��
		{
			cs_packet_pos * my_pos_packet = reinterpret_cast<cs_packet_pos *>(send_buffer);
			my_pos_packet->size = sizeof(cs_packet_pos);
			send_wsabuf.len = sizeof(cs_packet_pos);
			my_pos_packet->type = CS_POS;

			//my_pos_packet->roomnumb = g_my_info.roomnumb;

			g_my_info.pos = GetPlayerMatrix();

			DWORD iobyte;

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

		break;
	}


	case SC_TIME:
	{
		sc_packet_time *my_packet = reinterpret_cast<sc_packet_time *>(ptr);
		g_fgametime = my_packet->m_ftime;
		printf("���� �ð� : %f", g_fgametime);

		break;
	}
	case SC_GRAVITY_CHANGE:
	{

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
		printf("%d Ŭ���̾�Ʈ�� ���ݹ޾ҽ��ϴ�. hp : %d\n", my_packet->id, my_packet->hp);
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
				printf("�� ĳ���Ͱ� �׾����ϴ�\n");
				g_my_info.state = my_packet->state;
				m_ppScenes[CHARACTER]->ChangeAnimation(8); // �״� �ִϸ��̼�
			}
			else
			{
				printf("�÷��̾� [%d]�� �׾����ϴ�", my_packet->id);
				//�ٸ� Ŭ�� �׾��� ���� ���� ó��
				g_player_info[my_packet->id].state = my_packet->state;
				g_player_info[my_packet->id].anim_state = 8;
			}
		}
		break;
	}
	case SC_LOAD_OBJECT:
	{
		sc_packet_loadObject *my_packet = reinterpret_cast<sc_packet_loadObject*>(ptr);


		m_xmf3ObjectsPos[my_packet->m_ObjectID] = my_packet->m_xmf3ObjectPosition;
		//Sleep(10);
		m_ppScenes[OBJECT]->SetObjectsVectorFromPacket(my_packet->m_xmf3ObjectPosition, my_packet->m_ObjectID);
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
