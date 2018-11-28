#include "stdafx.h"
#include "GameFramework.h"
#include <process.h>


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


void CGameFramework::BuildLobby()
{
	// 로비를 그리기 위한 셋팅
	HRESULT hResult = m_pd3dScreenCommandAllocator->Reset();
	hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);

	m_pPlayer = new CPlayer(m_pd3dDevice, m_pd3dScreenCommandList, 0);
	m_pPlayer->BuildObject();
	m_pCamera = m_pPlayer->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	m_pUIShader = new UIShader(m_pPlayer, m_nCharacterType);
	m_pUIShader->CreateGraphicsRootSignature(m_pd3dDevice);
	m_pUIShader->CreateShader(m_pd3dDevice, 1);
	m_pUIShader->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList);

	m_pShadowShader = new CShadowShader(m_pd3dDevice, m_pd3dScreenCommandList, m_nWndClientWidth, m_nWndClientHeight);
	m_pShadowShader->SetPlayer(m_pPlayer);

	CharacterScene *pCScene = new CharacterScene();
	m_ppScenes[CHARACTER] = pCScene;

	BuildLightsAndMaterials();
	CreateLightsAndMaterialsShaderVariables(m_pd3dDevice, m_pd3dScreenCommandList);

	m_ppScenes[CHARACTER]->m_pPlayer = m_pPlayer;
	m_ppScenes[CHARACTER]->BuildObjects(m_pd3dDevice, m_pd3dScreenCommandList, m_pShadowShader->GetShadowMap(), 0);
	m_ppScenes[CHARACTER]->SetLightsAndMaterialsShaderVariables(m_pd3dcbLights, m_pd3dcbMaterials, m_pShadowShader->GetShadowMap());

	hResult = m_pd3dScreenCommandList->Close();

	ID3D12CommandList *ppd3dScreenCommandLists[] = { m_pd3dScreenCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dScreenCommandLists);
	WaitForGpuComplete();
}

void CGameFramework::BuildObjects()
{
	XMFLOAT3 xmf3Target;
	ShowCursor(false);

	HRESULT hResult;

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		hResult = m_ppd3dCommandAllocators[i]->Reset();
		hResult = m_ppd3dCommandLists[i]->Reset(m_ppd3dCommandAllocators[i], NULL);
	}
	hResult = m_pd3dScreenCommandAllocator->Reset();
	hResult = m_pd3dScreenCommandList->Reset(m_pd3dScreenCommandAllocator, NULL);

	TerrainAndSkyBoxScene *pTScene = new TerrainAndSkyBoxScene();
	m_ppScenes[TERRAIN] = pTScene;

	EffectScene *pEffectScene = new EffectScene();
	m_ppScenes[EFFECT] = pEffectScene;

	ObjectScene *pOScene = new ObjectScene();
	m_ppScenes[OBJECT] = pOScene;

	m_ppScenes[TERRAIN]->m_pPlayer = m_pPlayer;

	m_pUIShader->SetGameStart(m_nCharacterType);

	m_pComputeShader = new CComputShader(m_pd3dDevice);
	m_pComputeShader->CreateComputeRootSignature(m_pd3dDevice);
	m_pComputeShader->BuildPSO(m_pd3dDevice);

	pTScene->BuildObjects(m_pd3dDevice, m_ppd3dCommandLists[TERRAIN], m_pShadowShader->GetShadowMap(), m_nCharacterType);
	pTScene->SetLightsAndMaterialsShaderVariables(m_pd3dcbLights, m_pd3dcbMaterials, m_pShadowShader->GetShadowMap());

	m_ppScenes[OBJECT]->m_pPlayer = m_pPlayer;
	m_ppScenes[OBJECT]->BuildObjects(m_pd3dDevice, m_ppd3dCommandLists[OBJECT], m_pShadowShader->GetShadowMap(), m_nCharacterType);
	m_ppScenes[OBJECT]->SetLightsAndMaterialsShaderVariables(m_pd3dcbLights, m_pd3dcbMaterials, m_pShadowShader->GetShadowMap());

	m_ppScenes[EFFECT]->m_pPlayer = m_pPlayer;
	m_ppScenes[EFFECT]->SetCharacter(m_ppScenes[CHARACTER]->GetCharatersNum(), m_ppScenes[CHARACTER]->GetSoldierObjects(), m_ppScenes[CHARACTER]->GetDroneObjects(), m_ppScenes[CHARACTER]->GetAlienObjects());
	m_ppScenes[EFFECT]->BuildObjects(m_pd3dDevice, m_ppd3dCommandLists[EFFECT], m_pShadowShader->GetShadowMap(), m_nCharacterType);
	m_ppScenes[EFFECT]->SetLightsAndMaterialsShaderVariables(m_pd3dcbLights, m_pd3dcbMaterials, m_pShadowShader->GetShadowMap());

	for (int i = 0; i < NUM_SUBSETS; i++)
	{
		hResult = m_ppd3dCommandLists[i]->Close();
	}

	hResult = m_pd3dScreenCommandList->Close();

	ID3D12CommandList *ppd3dCommandLists[] = { m_ppd3dCommandLists[0],  m_ppd3dCommandLists[1],  m_ppd3dCommandLists[2], m_ppd3dCommandLists[3], m_pd3dScreenCommandList };

	m_pd3dCommandQueue->ExecuteCommandLists(NUM_SUBSETS + 1, ppd3dCommandLists);

	WaitForGpuComplete();

	if (m_pPlayer)
		m_pPlayer->ReleaseUploadBuffers();

	if (m_ppScenes)
		for (int i = 0; i < NUM_SUBSETS; i++)
			m_ppScenes[i]->ReleaseUploadBuffers();

	m_GameTimer.Reset();

	m_ppScenes[CHARACTER]->StartGameSetting(g_player_info, g_myid);

	cs_loading_complete *p = reinterpret_cast<cs_loading_complete *>(send_buffer);
	p->size = sizeof(cs_loading_complete);
	send_wsabuf.len = sizeof(cs_loading_complete);
	p->id = g_myid;
	p->type = CS_LOADING_COMPLETE;
	send_wsabuf.buf = reinterpret_cast<char *>(p);
	DWORD iobyte;
	WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
}
void CGameFramework::BuildLightsAndMaterials()
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));

	m_pLights->m_xmf4GlobalAmbient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

	m_pLights->m_pLights[0].m_bEnable = false;
	m_pLights->m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights->m_pLights[0].m_fRange = 100.0f;
	m_pLights->m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_pLights->m_pLights[0].m_xmf4Specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Position = XMFLOAT3(30.0f, 200.0f, 30.0f);
	m_pLights->m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights->m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

	m_pLights->m_pLights[1].m_bEnable = false;
	m_pLights->m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[1].m_fRange = 100.0f;
	m_pLights->m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.35f, 0.35f, 0.35f, 1.0f);
	m_pLights->m_pLights[1].m_xmf4Specular = XMFLOAT4(0.58f, 0.58f, 0.58f, 0.0f);
	m_pLights->m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights->m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[1].m_fFalloff = 8.0f;
	m_pLights->m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));

	m_pLights->m_pLights[2].m_bEnable = true;
	m_pLights->m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.35f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[2].m_xmf4Specular = XMFLOAT4(0.00002f, 0.00002f, 0.00002f, 1.0f);
	m_pLights->m_pLights[2].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);

	m_pLights->m_pLights[3].m_bEnable = true;
	m_pLights->m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights->m_pLights[3].m_fRange = 60.0f;
	m_pLights->m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[3].m_xmf4Specular = XMFLOAT4(0.2f, 0.2f, 0.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Position = XMFLOAT3(0.0f, 300.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights->m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[3].m_fFalloff = 8.0f;
	m_pLights->m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights->m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(0.0f));

	m_pLights->m_pLights[4].m_bEnable = false;
	m_pLights->m_pLights[4].m_nType = DIRECTIONAL_LIGHT;
	m_pLights->m_pLights[4].m_xmf4Ambient = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Diffuse = XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f);
	m_pLights->m_pLights[4].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights->m_pLights[4].m_xmf3Direction = XMFLOAT3(1.0f, 1.0f, 1.0f);

	m_pMaterials = new MATERIALS;
	::ZeroMemory(m_pMaterials, sizeof(MATERIALS));

	m_pMaterials->m_pReflections[0] = { XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[1] = { XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 10.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[2] = { XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 15.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[3] = { XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 20.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[4] = { XMFLOAT4(0.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 25.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[5] = { XMFLOAT4(0.0f, 0.5f, 0.5f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 30.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[6] = { XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f), XMFLOAT4(0.5f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 35.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
	m_pMaterials->m_pReflections[7] = { XMFLOAT4(1.0f, 0.5f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 40.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };

	m_nMaterials = 8;
}
