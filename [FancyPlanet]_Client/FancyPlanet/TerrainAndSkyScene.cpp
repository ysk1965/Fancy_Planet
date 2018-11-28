#include "stdafx.h"
#include "Scene.h"

TerrainAndSkyBoxScene::TerrainAndSkyBoxScene()
{
}
TerrainAndSkyBoxScene::~TerrainAndSkyBoxScene()
{
	ReleaseObjects();
}
void TerrainAndSkyBoxScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	XMFLOAT3 xmf3Scale(8.0f, 3.0f, 8.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.5f, 0.0f, 0.0f);

	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature,
		_T("../Assets/Image/Terrain/HeightMap2.raw", ), 257, 257, 257, 257, xmf3Scale, xmf4Color, pShadowMap);

	m_pStaticObjectShader = new StaticObjectShader();
	m_pStaticObjectShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	m_pStaticObjectShader->BuildObjects(pd3dDevice, pd3dCommandList, pShadowMap);
	m_ppStaticObjects = new StaticObject*[STATIC_OBJECT_NUM];

	m_ppStaticObjects[0] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/Toadstool01.bss_mesh", 3, 0);
	m_ppStaticObjects[1] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/Toadstool02.bss_mesh", 3, 0);
	m_ppStaticObjects[2] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/Crystal.bss_mesh", 7, 1);
	m_ppStaticObjects[3] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/Crystal.bss_mesh", 7, 1);
	m_ppStaticObjects[4] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/Crystal.bss_mesh", 7, 1);
	m_ppStaticObjects[5] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/dry_brunch.bss_mesh", 5, 0);
	m_ppStaticObjects[6] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/fallen_tree.bss_mesh", 5, 0);
	m_ppStaticObjects[7] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/stump_A.bss_mesh", 5, 0);
	m_ppStaticObjects[8] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/stump_B.bss_mesh", 5, 0);
	m_ppStaticObjects[9] = new StaticObject(pd3dDevice, pd3dCommandList, L"../Assets/StaticObjects/Mesh/Capsule.bss_mesh", 3, 1);
	//Capsule.bss_mesh
	ObjectsSetting(pd3dCommandList);
}
void TerrainAndSkyBoxScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature)
		m_pd3dGraphicsRootSignature->Release();

	if (m_pStaticObjectShader)
		delete m_pStaticObjectShader;

	if (m_pTerrain)
		delete m_pTerrain;

}
void TerrainAndSkyBoxScene::ReleaseUploadBuffers()
{
	if (m_pTerrain)
		m_pTerrain->ReleaseUploadBuffers();

}
void TerrainAndSkyBoxScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera, int Scene)
{
}
void TerrainAndSkyBoxScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	m_ppStaticObjects[2]->SetChangeColor(pd3dCommandList, GetTickCount());
	m_ppStaticObjects[3]->SetChangeColor(pd3dCommandList, GetTickCount());
	m_ppStaticObjects[4]->SetChangeColor(pd3dCommandList, GetTickCount());
	m_ppStaticObjects[9]->UpdateBase();
	m_ppStaticObjects[9]->UpdateShaderVariables(pd3dCommandList);
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(8, d3dcbLightsGpuVirtualAddress);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(7, d3dcbMaterialsGpuVirtualAddress);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	if (m_pTerrain)
	{
		if (m_pTerrain->m_pMaterial)
			m_pTerrain->m_pMaterial->m_pShader->SetDescriptorHeaps(pd3dCommandList);

		pd3dCommandList->SetGraphicsRootDescriptorTable(9, m_pTerrain->m_pMaterial->m_pShader->GetShadowGPUDescriptorHandle());

		if (m_pTerrain->m_pMaterial)
			m_pTerrain->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);
		m_pTerrain->Render(pd3dCommandList, 2, pCamera);
	}

	m_pStaticObjectShader->OnPrepareRender(pd3dCommandList, 0, m_ppStaticObjects[0]->GetType());
	m_ppStaticObjects[0]->Render(pd3dCommandList);
	m_pStaticObjectShader->OnPrepareRender(pd3dCommandList, 1, m_ppStaticObjects[1]->GetType());
	m_ppStaticObjects[1]->Render(pd3dCommandList);
	m_pStaticObjectShader->OnPrepareRender(pd3dCommandList, 2, m_ppStaticObjects[2]->GetType());
	m_ppStaticObjects[2]->Render(pd3dCommandList);
	m_ppStaticObjects[3]->Render(pd3dCommandList);
	m_ppStaticObjects[4]->Render(pd3dCommandList);

	m_pStaticObjectShader->OnPrepareRender(pd3dCommandList, 3, m_ppStaticObjects[5]->GetType());
	m_ppStaticObjects[5]->Render(pd3dCommandList);
	m_ppStaticObjects[6]->Render(pd3dCommandList);
	m_ppStaticObjects[7]->Render(pd3dCommandList);
	m_ppStaticObjects[8]->Render(pd3dCommandList);

	m_pStaticObjectShader->OnPrepareRender(pd3dCommandList, 4, 2);
	m_ppStaticObjects[9]->Render(pd3dCommandList);
}
void TerrainAndSkyBoxScene::BaseSetting(UINT Num, UINT nColor)
{
	m_ppStaticObjects[9]->SetBase(Num, nColor);
}
void TerrainAndSkyBoxScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	m_pTerrain->m_pMaterial->m_pShader->ShadowRender(pd3dCommandList);
	pCamera->ShadowUpdateShaderVariables(pd3dCommandList, pCameraInfo);
	m_pTerrain->ShadowRender(pd3dCommandList, 2);

	m_pStaticObjectShader->OnPrepareShadowRender(pd3dCommandList);
	for (int i = 0; i < STATIC_OBJECT_NUM; i++)
	{
		m_ppStaticObjects[i]->Render(pd3dCommandList);
	}
}
void TerrainAndSkyBoxScene::ObjectsSetting(ID3D12GraphicsCommandList *pd3dCommandList)
{
	m_ppStaticObjects[0]->SetScale(0, 1100, 1100, 1100);
	m_ppStaticObjects[0]->SetPosition(0, XMFLOAT3(1670.822266, 94.650200, 1558.749878));
	m_ppStaticObjects[0]->SetScale(1, 600, 600, 600);
	m_ppStaticObjects[0]->SetPosition(1, XMFLOAT3(1495.401245, 101.178879, 1435.619629));
	m_ppStaticObjects[0]->Rotate(1, 10.911f, 124.077f, 5.325f);
	m_ppStaticObjects[0]->SetScale(2, 900, 900, 900);
	m_ppStaticObjects[0]->SetPosition(2, XMFLOAT3(1714.393555, 99.482132, 1299.404663));
	m_ppStaticObjects[0]->Rotate(2, -12.911f, 24.077f, -5.325f);

	m_ppStaticObjects[1]->SetScale(0, 1000, 1000, 1000);
	m_ppStaticObjects[1]->SetPosition(0, XMFLOAT3(1630.588013, 92.810158, 1585.109619));
	m_ppStaticObjects[1]->Rotate(0, 0.911f, 154.077f, -15.325f);
	m_ppStaticObjects[1]->SetScale(1, 1000, 1000, 1000);
	m_ppStaticObjects[1]->SetPosition(1, XMFLOAT3(1698.136108, 94.818596, 1546.218506));
	m_ppStaticObjects[1]->Rotate(1, 0.911f, -154.077f, 15.325f);
	m_ppStaticObjects[1]->SetScale(2, 1500, 1500, 1500);
	m_ppStaticObjects[1]->SetPosition(2, XMFLOAT3(1486.307373, 118.221954, 1268.123291));
	m_ppStaticObjects[1]->Rotate(2, -2.911f, 100.077f, 7.325f);

	STO_E tmp;

	tmp.xmf4x4World = Matrix4x4::Identity();
	tmp.xmf4x4World._11 = 60.0f;
	tmp.xmf4x4World._22 = 60.0f;
	tmp.xmf4x4World._33 = 60.0f;
	tmp.xmf4Color = XMFLOAT4(0.3f, 0.8f, 0.4f, 1.0f);
	tmp.fAlpha = 0.4f;

	for (int i = 0; i < 7; i++)
	{
		m_ppStaticObjects[2]->SetShaderVariable(i, &tmp);
	}
	XMFLOAT3 pos = XMFLOAT3(932.477173f, 107.522667f, 348.899689f);
	float s = 20.0f;

	m_ppStaticObjects[2]->SetPosition(0, XMFLOAT3(0.0f * s + pos.x, 1.33f * s + pos.y, 0.0f * s + pos.z));
	m_ppStaticObjects[2]->Rotate(0, -2.868f, -8.079f, 12.806f);

	m_ppStaticObjects[2]->SetPosition(1, XMFLOAT3(0.53f * s + pos.x, 0.881f * s + pos.y, 1.2f * s + pos.z));
	m_ppStaticObjects[2]->Rotate(1, -28.911f, -154.077f, 9.325f);

	m_ppStaticObjects[2]->SetPosition(2, XMFLOAT3(-0.006f * s + pos.x, 0.333f * s + pos.y, -0.287f * s + pos.z));
	m_ppStaticObjects[2]->Rotate(2, -26.017f, -67.317f, 32.665f);

	m_ppStaticObjects[2]->SetPosition(3, XMFLOAT3(0.503f * s + pos.x, 0.333f * s + pos.y, 0.174f * s + pos.z));
	m_ppStaticObjects[2]->Rotate(3, -40.665f, -445.691f, 15.07f);

	m_ppStaticObjects[2]->SetPosition(4, XMFLOAT3(-0.667f * s + pos.x, 0.333f * s + pos.y, 0.583f * s + pos.z));
	m_ppStaticObjects[2]->Rotate(4, -44.672f, -600.968f, 10.02f);

	m_ppStaticObjects[2]->SetPosition(5, XMFLOAT3(-0.636f * s + pos.x, 0.663f * s + pos.y, -0.5f * s + pos.z));
	m_ppStaticObjects[2]->Rotate(5, -28.984f, -616.18f, -30.812f);

	m_ppStaticObjects[2]->SetPosition(6, XMFLOAT3(-0.097f * s + pos.x, 0.054f * s + pos.y, 1.136f * s + pos.z));
	m_ppStaticObjects[2]->Rotate(6, -45.269, -588.908, 33.884f);

	tmp.xmf4x4World = Matrix4x4::Identity();
	tmp.xmf4x4World._11 = 80.0f;
	tmp.xmf4x4World._22 = 80.0f;
	tmp.xmf4x4World._33 = 80.0f;
	tmp.xmf4Color = XMFLOAT4(0.3f, 0.6f, 0.6f, 1.0f);
	tmp.fAlpha = 0.3f;

	for (int i = 0; i < 7; i++)
	{
		m_ppStaticObjects[3]->SetShaderVariable(i, &tmp);
	}
	pos = XMFLOAT3(607.572754, 107.131805, 281.181152);

	m_ppStaticObjects[3]->SetPosition(0, XMFLOAT3(0.0f * s + pos.x, 1.33f * s + pos.y, 0.0f * s + pos.z));
	m_ppStaticObjects[3]->Rotate(0, -2.868f, -8.079f, 12.806f);

	m_ppStaticObjects[3]->SetPosition(1, XMFLOAT3(0.53f * s + pos.x, 0.881f * s + pos.y, 1.2f * s + pos.z));
	m_ppStaticObjects[3]->Rotate(1, -28.911f, -154.077f, 9.325f);

	m_ppStaticObjects[3]->SetPosition(2, XMFLOAT3(-0.006f * s + pos.x, 0.333f * s + pos.y, -0.287f * s + pos.z));
	m_ppStaticObjects[3]->Rotate(2, -26.017f, -67.317f, 32.665f);

	m_ppStaticObjects[3]->SetPosition(3, XMFLOAT3(0.503f * s + pos.x, 0.333f * s + pos.y, 0.174f * s + pos.z));
	m_ppStaticObjects[3]->Rotate(3, -40.665f, -445.691f, 15.07f);

	m_ppStaticObjects[3]->SetPosition(4, XMFLOAT3(-0.667f * s + pos.x, 0.333f * s + pos.y, 0.583f * s + pos.z));
	m_ppStaticObjects[3]->Rotate(4, -44.672f, -600.968f, 10.02f);

	m_ppStaticObjects[3]->SetPosition(5, XMFLOAT3(-0.636f * s + pos.x, 0.663f * s + pos.y, -0.5f * s + pos.z));
	m_ppStaticObjects[3]->Rotate(5, -28.984f, -616.18f, -30.812f);

	m_ppStaticObjects[3]->SetPosition(6, XMFLOAT3(-0.097f * s + pos.x, 0.054f * s + pos.y, 1.136f * s + pos.z));
	m_ppStaticObjects[3]->Rotate(6, -45.269, -588.908, 33.884f);

	tmp.xmf4x4World = Matrix4x4::Identity();
	tmp.xmf4x4World._11 = 70.0f;
	tmp.xmf4x4World._22 = 70.0f;
	tmp.xmf4x4World._33 = 70.0f;
	tmp.xmf4Color = XMFLOAT4(0.2f, 0.1f, 0.1f, 1.0f);
	tmp.fAlpha = 0.1f;

	for (int i = 0; i < 7; i++)
	{
		m_ppStaticObjects[4]->SetShaderVariable(i, &tmp);
	}
	pos = XMFLOAT3(781.886230, 86.008614, 565.859192);

	m_ppStaticObjects[4]->SetPosition(0, XMFLOAT3(0.0f * s + pos.x, 1.33f * s + pos.y, 0.0f * s + pos.z));
	m_ppStaticObjects[4]->Rotate(0, -2.868f, -8.079f, 12.806f);

	m_ppStaticObjects[4]->SetPosition(1, XMFLOAT3(0.53f * s + pos.x, 0.881f * s + pos.y, 1.2f * s + pos.z));
	m_ppStaticObjects[4]->Rotate(1, -28.911f, -154.077f, 9.325f);

	m_ppStaticObjects[4]->SetPosition(2, XMFLOAT3(-0.006f * s + pos.x, 0.333f * s + pos.y, -0.287f * s + pos.z));
	m_ppStaticObjects[4]->Rotate(2, -26.017f, -67.317f, 32.665f);

	m_ppStaticObjects[4]->SetPosition(3, XMFLOAT3(0.503f * s + pos.x, 0.333f * s + pos.y, 0.174f * s + pos.z));
	m_ppStaticObjects[4]->Rotate(3, -40.665f, -445.691f, 15.07f);

	m_ppStaticObjects[4]->SetPosition(4, XMFLOAT3(-0.667f * s + pos.x, 0.333f * s + pos.y, 0.583f * s + pos.z));
	m_ppStaticObjects[4]->Rotate(4, -44.672f, -600.968f, 10.02f);

	m_ppStaticObjects[4]->SetPosition(5, XMFLOAT3(-0.636f * s + pos.x, 0.663f * s + pos.y, -0.5f * s + pos.z));
	m_ppStaticObjects[4]->Rotate(5, -28.984f, -616.18f, -30.812f);

	m_ppStaticObjects[4]->SetPosition(6, XMFLOAT3(-0.097f * s + pos.x, 0.054f * s + pos.y, 1.136f * s + pos.z));
	m_ppStaticObjects[4]->Rotate(6, -45.269, -588.908, 33.884f);

	for (int i = 0; i < 5; i++)
	{
		m_ppStaticObjects[5]->SetScale(i, 1100, 1100, 1100);
		m_ppStaticObjects[6]->SetScale(i, 1100, 1100, 1100);
		m_ppStaticObjects[7]->SetScale(i, 1100, 1100, 1100);
		m_ppStaticObjects[8]->SetScale(i, 1100, 1100, 1100);
		
		m_ppStaticObjects[5]->Rotate(i, 180.0, 0.0, 180.0);
		m_ppStaticObjects[6]->Rotate(i, 180.0, 0.0, 180.0);
		m_ppStaticObjects[7]->Rotate(i, 180.0, 0.0, 180.0);
		m_ppStaticObjects[8]->Rotate(i, 180.0, 0.0, 180.0);
	}
	m_ppStaticObjects[5]->SetPosition(0, XMFLOAT3(216.518463, 94.860519, 1431.946167));
	m_ppStaticObjects[6]->SetPosition(0, XMFLOAT3(540.722534, 100.869858, 1431.592773));
	m_ppStaticObjects[7]->SetPosition(0, XMFLOAT3(480.020935, 92.711632, 1336.365845));
	m_ppStaticObjects[8]->SetPosition(0, XMFLOAT3(383.315643, 96.609077, 1317.209473));

	m_ppStaticObjects[5]->SetPosition(1, XMFLOAT3(435.259521, 92.106895, 1319.065308));
	m_ppStaticObjects[6]->SetPosition(1, XMFLOAT3(267.298950, 90.467430, 1385.926514));//
	m_ppStaticObjects[7]->SetPosition(1, XMFLOAT3(220.718872, 109.198067, 1535.012695));
	m_ppStaticObjects[8]->SetPosition(1, XMFLOAT3(266.725037, 98.263977, 1559.339844));

	m_ppStaticObjects[5]->SetPosition(2, XMFLOAT3(288.454590, 101.124023, 1606.226929));
	m_ppStaticObjects[6]->SetPosition(2, XMFLOAT3(301.801544, 103.388237, 1640.386597));
	m_ppStaticObjects[7]->SetPosition(2, XMFLOAT3(326.116028, 103.215439, 1686.152710));
	m_ppStaticObjects[8]->SetPosition(2, XMFLOAT3(360.315735, 99.238319, 1706.187134));

	m_ppStaticObjects[5]->SetPosition(3, XMFLOAT3(410.664917, 98.873093, 1717.138794));
	m_ppStaticObjects[6]->SetPosition(3, XMFLOAT3(441.359192, 100.421715, 1740.701172));
	m_ppStaticObjects[7]->SetPosition(3, XMFLOAT3(468.827881, 104.588554, 1766.410889));
	m_ppStaticObjects[8]->SetPosition(3, XMFLOAT3(529.147583, 96.430466, 1433.449341));

	m_ppStaticObjects[5]->SetPosition(4, XMFLOAT3(494.631866, 91.454956, 1393.026367));
	m_ppStaticObjects[6]->SetPosition(4, XMFLOAT3(513.432739, 90.485909, 1382.323364));
	m_ppStaticObjects[7]->SetPosition(4, XMFLOAT3(240.012741, 97.164391, 1400.278687));
	m_ppStaticObjects[8]->SetPosition(4, XMFLOAT3(318.064117, 95.336906, 1372.545166));

	tmp.xmf4x4World = Matrix4x4::Identity();
	tmp.xmf4x4World._11 = 100.0f;
	tmp.xmf4x4World._22 = 100.0f;
	tmp.xmf4x4World._33 = 100.0f;
	tmp.xmf4Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	tmp.fAlpha = 0.0f;

	for (int i = 0; i < 3; i++)
	{
		m_ppStaticObjects[9]->SetShaderVariable(i, &tmp);
	}

	m_ppStaticObjects[9]->SetPosition(0, XMFLOAT3(645.257019, 158.432705, 1016.105225));
	m_ppStaticObjects[9]->SetPosition(1, XMFLOAT3(1257.854858, 144.713272, 846.498474));
	m_ppStaticObjects[9]->SetPosition(2, XMFLOAT3(983.552856, 164.819122, 1397.244995));


	for (int i = 0; i < STATIC_OBJECT_NUM; i++)
	{
		m_ppStaticObjects[i]->UpdateShaderVariables(pd3dCommandList);
	}
}
ID3D12RootSignature *TerrainAndSkyBoxScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[6];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 4;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 5;
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 6;
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 9;
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 10;
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[11];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Player
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1; //Object
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1; //Terrain(Base)
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1; //Terrain(Detail)
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1; //Terrain(Normal)
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3];
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1; //skyBoxTextrue
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[7].Descriptor.ShaderRegister = 7; //Materials
	pd3dRootParameters[7].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[8].Descriptor.ShaderRegister = 8; //Lights
	pd3dRootParameters[8].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[9].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5];
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV; // ÀÎ½ºÅÏ½Ì
	pd3dRootParameters[10].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[10].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[3];

	pd3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	pd3dSamplerDescs[0].MipLODBias = 0;
	pd3dSamplerDescs[0].MaxAnisotropy = 1;
	pd3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[0].MinLOD = 0;
	pd3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[0].ShaderRegister = 0;
	pd3dSamplerDescs[0].RegisterSpace = 0;
	pd3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 1;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dSamplerDescs[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[2].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[2].MipLODBias = 0;
	pd3dSamplerDescs[2].MaxAnisotropy = 16;
	pd3dSamplerDescs[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[2].MinLOD = 0;
	pd3dSamplerDescs[2].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[2].ShaderRegister = 2;
	pd3dSamplerDescs[2].RegisterSpace = 0;
	pd3dSamplerDescs[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	pd3dSamplerDescs[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = _countof(pd3dSamplerDescs);
	d3dRootSignatureDesc.pStaticSamplers = pd3dSamplerDescs;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob *pd3dSignatureBlob = NULL;
	ID3DBlob *pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize()
		, __uuidof(ID3D12RootSignature), (void **)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob)
		pd3dSignatureBlob->Release();
	if (pd3dErrorBlob)
		pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}
