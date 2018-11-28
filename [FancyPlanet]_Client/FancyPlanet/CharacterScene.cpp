#include "stdafx.h"
#include "Scene.h"
#include <stack>

CharacterScene::CharacterScene()
{
	m_ppAnimationController = new AnimationController*[MESH_NUM];
	m_ppSampleAnimationObjects = new CAnimationObject*[MESH_NUM];
	m_pnAnimationFactor = new UINT[MESH_NUM];
}
CharacterScene::~CharacterScene()
{
	ReleaseObjects();
	ReleaseShaderVariables();
}
CAnimationObject* CharacterScene::FindAnimationFactorObject(CAnimationObject* pRootObject, int nIndex)
{
	stack<CAnimationObject*> FrameStack;

	FrameStack.push(pRootObject);

	while (!FrameStack.empty())
	{
		CAnimationObject* pFindObject = FrameStack.top();
		FrameStack.pop();

		if (pFindObject->m_pAnimationFactors != NULL && pFindObject->GetSkinnedMeshRendererNum() == nIndex)
			return pFindObject;

		if (pFindObject->m_pChild)
			FrameStack.push(pFindObject->m_pChild);
		if (pFindObject->m_pSibling)
			FrameStack.push(pFindObject->m_pSibling);
	}
	return NULL;
}
void CharacterScene::SelectCharacter(int nCharaterType)
{
	m_ppSoldierObjects[0]->SetPlayer(NULL);
	m_ppDroneObjects[0]->SetPlayer(NULL);
	m_ppAliens[0]->SetPlayer(NULL);

	m_ppSoldierObjects[0]->SetPosition(-500, -500, -500);
	m_ppDroneObjects[0]->SetPosition(-500, -500, -500);
	m_ppAliens[0]->SetPosition(-500, -500, -500);

	if (nCharaterType == SOLDIER)
	{
		m_ppSoldierObjects[0]->SetPosition(0, 0, 0);
		m_ppSoldierObjects[0]->SetPlayer(m_pPlayer);
		m_pPlayer->SetRenderObject(m_ppSoldierObjects[0]);
	}
	else if (nCharaterType == DRONE)
	{
		m_ppDroneObjects[0]->SetPosition(0, 0, 0);
		m_ppDroneObjects[0]->SetPlayer(m_pPlayer);
		m_pPlayer->SetRenderObject(m_ppDroneObjects[0]);
	}
	else
	{
		m_ppAliens[0]->SetPosition(0, 0, 0);
		m_ppAliens[0]->SetPlayer(m_pPlayer);
		m_pPlayer->SetRenderObject(m_ppAliens[0]);
	}
}
void CharacterScene::ResetCharacter(UINT nIndex, UINT nCharacterType)
{
	if (nCharacterType == SOLDIER)
	{
		m_ppSoldierObjects[nIndex]->m_pAnimationTime->Reset();
	}
	else if (nCharacterType == DRONE)
	{
		m_ppDroneObjects[nIndex]->m_pAnimationTime->Reset();
	}
	else
	{
		m_ppAliens[nIndex]->m_pAnimationTime->Reset();
	}
}

void CharacterScene::BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12Resource* pShadowMap, int nCharaterType)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);
	m_nObjects = 2;

	m_ppSampleAnimationObjects[0] = new CAnimationObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0, L"../Assets/SpaceSoldier.bss", m_ppAnimationController[0], m_nObjects, m_pnAnimationFactor[0], pShadowMap);
	m_ppSampleAnimationObjects[1] = new CAnimationObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0, L"../Assets/SpaceDrone.bss", m_ppAnimationController[1], m_nObjects, m_pnAnimationFactor[1], pShadowMap);
	m_ppSampleAnimationObjects[2] = new CAnimationObject(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, 0, L"../Assets/Alien.bss", m_ppAnimationController[2], m_nObjects, m_pnAnimationFactor[2], pShadowMap);

	m_ppSoldierObjects = new CAnimationObject*[m_nObjects];
	m_ppDroneObjects = new CAnimationObject*[m_nObjects];
	m_ppAliens = new CAnimationObject*[m_nObjects];

	CopyObject(pd3dDevice, pd3dCommandList, m_ppSampleAnimationObjects[0], m_ppSoldierObjects, m_nObjects - 1, 0);
	CopyObject(pd3dDevice, pd3dCommandList, m_ppSampleAnimationObjects[1], m_ppDroneObjects, m_nObjects - 1, 1);
	CopyObject(pd3dDevice, pd3dCommandList, m_ppSampleAnimationObjects[2], m_ppAliens, m_nObjects - 1, 2);

	m_ppSoldierObjects[m_nObjects - 1] = m_ppSampleAnimationObjects[0];
	m_ppDroneObjects[m_nObjects - 1] = m_ppSampleAnimationObjects[1];
	m_ppAliens[m_nObjects - 1] = m_ppSampleAnimationObjects[2];

	SelectCharacter(nCharaterType);

	for (int i = 0; i < m_nObjects; i++)
	{
		for (int j = 0; j < m_pnAnimationFactor[0]; j++)
		{
			m_ppSoldierObjects[i]->m_ppAnimationFactorObjects[j]->m_pAnimationFactors->SetBoneObject(m_ppSoldierObjects[i]);
		}
		for (int j = 0; j < m_pnAnimationFactor[1]; j++)
		{
			m_ppDroneObjects[i]->m_ppAnimationFactorObjects[j]->m_pAnimationFactors->SetBoneObject(m_ppDroneObjects[i]);
		}
		for (int j = 0; j < m_pnAnimationFactor[1]; j++)
		{
			m_ppAliens[i]->m_ppAnimationFactorObjects[j]->m_pAnimationFactors->SetBoneObject(m_ppAliens[i]);
		}

		m_ppSoldierObjects[i]->SetPosition(10 * i + 100, 0, 10 * i + 100);
		m_ppDroneObjects[i]->SetPosition(10 * i + 100, 0, 10 * i + 100);
		m_ppAliens[i]->SetPosition(10 * i + 120, 0, 10 * i + 120);

		m_ppSoldierObjects[i]->SetScale(10.0f, 10.0f, 10.0f);
		m_ppDroneObjects[i]->SetScale(15.0f, 15.0f, 15.0f);
		m_ppAliens[i]->SetScale(15.0f, 15.0f, 15.0f);
		m_ppSoldierObjects[i]->SetPlayerScale(10);
		m_ppDroneObjects[i]->SetPlayerScale(15);
		m_ppAliens[i]->SetPlayerScale(15);
	}
}

void CharacterScene::CalcCharacterIndex(array <PLAYER_INFO, MAX_USER>& PlayerArray)
{
	int SoldierIdx = 0;
	int DroneIdx = 0;
	int CreatureIdx = 0;

	for (int i = 0; i < MAX_USER; i++)
	{
		if (PlayerArray[i].m_isconnected)
		{
			if (PlayerArray[i].m_iCharacterType == SOLDIER)
			{
				PlayerArray[i].Character_idx = SoldierIdx;
				printf("솔져 캐락터 인덱스 %d  \n", PlayerArray[i].Character_idx);
				SoldierIdx++;
			}
			else if (PlayerArray[i].m_iCharacterType == DRONE)
			{
				PlayerArray[i].Character_idx = DroneIdx;
				printf("드론 캐락터 인덱스 %d  \n", PlayerArray[i].Character_idx);
				DroneIdx++;
			}
			else {
				PlayerArray[i].Character_idx = CreatureIdx;
				printf("크리쳐 캐락터 인덱스 %d  \n", PlayerArray[i].Character_idx);
				CreatureIdx++;
			}
		}
	}
}
void CharacterScene::SetPlayer(array <PLAYER_INFO, MAX_USER>& PlayerArray, UINT myid)
{
	m_nCharacterIndex = PlayerArray[myid].Character_idx;

	if (PlayerArray[myid].m_iCharacterType == SOLDIER)
	{
		m_ppSoldierObjects[PlayerArray[myid].Character_idx]->SetPlayer(m_pPlayer);
		m_pPlayer->SetRenderObject(m_ppSoldierObjects[PlayerArray[myid].Character_idx]);
	}
	else if (PlayerArray[myid].m_iCharacterType == DRONE)
	{
		m_ppDroneObjects[PlayerArray[myid].Character_idx]->SetPlayer(m_pPlayer);
		m_pPlayer->SetRenderObject(m_ppDroneObjects[PlayerArray[myid].Character_idx]);
	}
	else
	{
		m_ppAliens[PlayerArray[myid].Character_idx]->SetPlayer(m_pPlayer);
		m_pPlayer->SetRenderObject(m_ppAliens[PlayerArray[myid].Character_idx]);
	}
}
void CharacterScene::StartGameSetting(array <PLAYER_INFO, MAX_USER>& PlayerArray, int myId)
{
	CalcCharacterIndex(PlayerArray);
	SetPlayer(PlayerArray, myId);
}

void CharacterScene::ModelsSetPosition(array <PLAYER_INFO, MAX_USER>& PlayerArray, int myid, int Type)
{
	for (int i = 0; i < MAX_USER; i++)
	{
		if (PlayerArray[i].m_isconnected)
		{
			if (PlayerArray[i].m_iCharacterType == SOLDIER)
			{
				if (i != myid)
				{
					XMFLOAT3 Scale = XMFLOAT3(10.f, 10.f, 10.f);

					XMVECTOR S = XMLoadFloat3(&Scale); // 항상 곱해주는 스케읽밧
					XMVECTOR P = XMLoadFloat3(&XMFLOAT3(PlayerArray[i].pos._41, PlayerArray[i].pos._42 + 12.f, PlayerArray[i].pos._43)); // 서버에서 보내준 포지션값
					XMVECTOR Q = XMLoadFloat4(&PlayerArray[i].quat); // 서버에서 보내준 쿼터니언값

					XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 마지막 _x4자리 
					XMStoreFloat4x4(&m_ppSoldierObjects[PlayerArray[i].Character_idx]->m_xmf4x4ToParentTransform, XMMatrixAffineTransformation(S, zero, Q, P));
				}
			}
			else if (PlayerArray[i].m_iCharacterType == DRONE)
			{
				if (i != myid)
				{
					XMFLOAT3 Scale = XMFLOAT3(10.f, 10.f, 10.f);
					XMVECTOR S = XMLoadFloat3(&Scale); // 항상 곱해주는 스케읽밧
					XMVECTOR P = XMLoadFloat3(&XMFLOAT3(PlayerArray[i].pos._41, PlayerArray[i].pos._42, PlayerArray[i].pos._43)); // 서버에서 보내준 포지션값
					XMVECTOR Q = XMLoadFloat4(&PlayerArray[i].quat); // 서버에서 보내준 쿼터니언값

					XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 마지막 _x4자리 
					XMStoreFloat4x4(&m_ppDroneObjects[PlayerArray[i].Character_idx]->m_xmf4x4ToParentTransform, XMMatrixAffineTransformation(S, zero, Q, P));
				}

			}
			else
			{
				if (i != myid)
				{
					XMFLOAT3 Scale = XMFLOAT3(10.f, 10.f, 10.f);
					XMVECTOR S = XMLoadFloat3(&Scale); // 항상 곱해주는 스케읽밧
					XMVECTOR P = XMLoadFloat3(&XMFLOAT3(PlayerArray[i].pos._41, PlayerArray[i].pos._42, PlayerArray[i].pos._43)); // 서버에서 보내준 포지션값
					XMVECTOR Q = XMLoadFloat4(&PlayerArray[i].quat); // 서버에서 보내준 쿼터니언값

					XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 마지막 _x4자리 
					XMStoreFloat4x4(&m_ppAliens[PlayerArray[i].Character_idx]->m_xmf4x4ToParentTransform, XMMatrixAffineTransformation(S, zero, Q, P));
				}
			}
		}
	}
}
void CharacterScene::CopyObject(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, CAnimationObject* pSample, CAnimationObject** ppObjects, UINT nSize, UINT nAnimationFactorIndex)
{
	if (nSize <= 0)
		return;
	UINT nAnimationFactorObject = -1;
	stack<CAnimationObject*> FrameStack;
	stack<CAnimationObject*> ObjectsStack;

	FrameStack.push(pSample);
	CAnimationObject** ppSiblingObjects = new CAnimationObject*[nSize];
	CAnimationObject** ppChildObjects = new CAnimationObject*[nSize];
	CAnimationObject** ppEmptyObjects = new CAnimationObject*[nSize];
	CAnimationObject* TargetFrame = NULL;
	// -1 = 루트, 0 = 형제, 1 = 자식

	for (int i = 0; i < nSize; i++)
	{
		ppObjects[i] = new CAnimationObject(0);
		ppEmptyObjects[i] = ppObjects[i];
	}
	for (int i = 0; i < nSize; i++)
	{
		ObjectsStack.push(ppObjects[i]);
	}

	while (1)
	{
		if (FrameStack.empty())
			break;

		TargetFrame = FrameStack.top();
		FrameStack.pop();
		for (int i = 0; i < nSize; i++)
		{
			ppEmptyObjects[i] = ObjectsStack.top();
			ObjectsStack.pop();
		}
		for (int i = 0; i < nSize; i++)
		{
			*ppEmptyObjects[i] = *TargetFrame;
			if (TargetFrame->m_ppAnimationFactorObjects != NULL)
				ppEmptyObjects[i]->m_ppAnimationFactorObjects = new CAnimationObject*[m_pnAnimationFactor[nAnimationFactorIndex]];

			if (TargetFrame->m_pAnimationFactors)
			{
				if (nAnimationFactorObject == -1)
				{
					for (int k = 0; k < 4; k++)
					{
						if (pSample->m_ppAnimationFactorObjects[k]->m_iBoneIndex == TargetFrame->m_iBoneIndex)
						{
							nAnimationFactorObject = k;
							break;
						}
					}
				}
				ppEmptyObjects[i]->m_pAnimationFactors = new AnimationResourceFactors(pd3dDevice, pd3dCommandList, TargetFrame->m_pAnimationFactors->m_nBindpos, 0);
				memcpy(ppEmptyObjects[i]->m_pAnimationFactors->m_pBoneIndexList, TargetFrame->m_pAnimationFactors->m_pBoneIndexList, sizeof(UINT) * ppEmptyObjects[i]->m_pAnimationFactors->m_nBindpos);
				ppObjects[i]->m_ppAnimationFactorObjects[nAnimationFactorObject] = ppEmptyObjects[i];
			}
			if (TargetFrame->m_pAnimationTime)
			{
				ppEmptyObjects[i]->m_pAnimationTime = new AnimationTimeFactor();
			}
		}
		nAnimationFactorObject = -1;
		if (TargetFrame->m_pChild)
		{
			for (int i = 0; i < nSize; i++)
			{

				ppChildObjects[i] = new CAnimationObject(0);
				ppEmptyObjects[i]->m_pChild = ppChildObjects[i];
				ppChildObjects[i]->m_pParent = ppEmptyObjects[i];
			}
		}
		if (TargetFrame->m_pSibling)
		{
			for (int i = 0; i < nSize; i++)
			{
				ppChildObjects[i] = new CAnimationObject(0);
				ppEmptyObjects[i]->m_pSibling = ppChildObjects[i];
				ppChildObjects[i]->m_pParent = ppEmptyObjects[i]->m_pParent;
			}
		}

		if (TargetFrame->m_pChild)
		{
			for (int i = 0; i < nSize; i++)
			{
				ObjectsStack.push(ppEmptyObjects[i]->m_pChild);
			}
			FrameStack.push(TargetFrame->m_pChild);
		}
		if (TargetFrame->m_pSibling)
		{
			for (int i = 0; i < nSize; i++)
			{
				ObjectsStack.push(ppEmptyObjects[i]->m_pSibling);
			}
			FrameStack.push(TargetFrame->m_pSibling);
		}
	}
	delete[] ppEmptyObjects;
	delete[] ppSiblingObjects;
	delete[] ppChildObjects;
}
void CharacterScene::ReleaseObjects()
{
	if (m_ppSampleAnimationObjects)
	{
		for (int i = 0; i<MESH_NUM; i++)
			delete m_ppSampleAnimationObjects[i];

		delete[] m_ppSampleAnimationObjects;
	}

}
void CharacterScene::ReleaseUploadBuffers()
{
	for (int i = 0; i < MESH_NUM; i++)
	{
		m_ppSampleAnimationObjects[i]->ReleaseUploadBuffers();
	}
}
void CharacterScene::CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList)
{
}
void CharacterScene::UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList)
{
}

void CharacterScene::AnimateObjects(float fTimeElapsed, CCamera *pCamera, int Scene)
{
}
void CharacterScene::ChangeAnimationInLobby(int newState, UINT nCharacterType)
{
	if (nCharacterType == SOLDIER)
	{
		m_ppSoldierObjects[0]->ChangeAnimation(newState);
	}
	else if (nCharacterType == DRONE)
	{
		m_ppDroneObjects[0]->ChangeAnimation(newState);
	}
	else
	{
		m_ppAliens[0]->ChangeAnimation(newState);
	}
}
void CharacterScene::ChangeAnimation(int newState, array <PLAYER_INFO, MAX_USER>& PlayerArray, int id)
{
	if (PlayerArray[id].m_iCharacterType == SOLDIER)
	{
		m_ppSoldierObjects[PlayerArray[id].Character_idx]->m_pAnimationController->SetObject(m_ppSoldierObjects[PlayerArray[id].Character_idx], NULL);
		m_ppSoldierObjects[PlayerArray[id].Character_idx]->ChangeAnimation(newState);
	}
	else if (PlayerArray[id].m_iCharacterType == DRONE)
	{
		m_ppDroneObjects[PlayerArray[id].Character_idx]->m_pAnimationController->SetObject(m_ppDroneObjects[PlayerArray[id].Character_idx], NULL);
		m_ppDroneObjects[PlayerArray[id].Character_idx]->ChangeAnimation(newState);
	}
	else
	{
		m_ppAliens[PlayerArray[id].Character_idx]->m_pAnimationController->SetObject(m_ppAliens[PlayerArray[id].Character_idx], NULL);
		m_ppAliens[PlayerArray[id].Character_idx]->ChangeAnimation(newState);
	}
}
void CharacterScene::Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	if (m_ppSampleAnimationObjects[0]->m_ppAnimationFactorObjects[0]->m_pMaterial->m_pShader)
		m_ppSampleAnimationObjects[0]->m_ppAnimationFactorObjects[0]->m_pMaterial->m_pShader->SetPipelineState(pd3dCommandList);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(12, d3dcbLightsGpuVirtualAddress);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbMaterialsGpuVirtualAddress = m_pd3dcbMaterials->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(11, d3dcbMaterialsGpuVirtualAddress);

	m_pPlayer->OnPrepareRender();

	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppSoldierObjects[i]->m_pAnimationController->SetObject(m_ppSoldierObjects[i], NULL);
		m_ppSoldierObjects[i]->m_pAnimationController->AdvanceAnimation(pd3dCommandList, true);
		m_ppDroneObjects[i]->m_pAnimationController->SetObject(m_ppDroneObjects[i], NULL);
		m_ppDroneObjects[i]->m_pAnimationController->AdvanceAnimation(pd3dCommandList, true);
		m_ppAliens[i]->m_pAnimationController->SetObject(m_ppAliens[i], NULL);
		m_ppAliens[i]->m_pAnimationController->AdvanceAnimation(pd3dCommandList, true);


		m_ppSoldierObjects[i]->UpdateTransform(NULL);
		m_ppDroneObjects[i]->UpdateTransform(NULL);
		m_ppAliens[i]->UpdateTransform(NULL);
	}

	for (int k = MESH_NUM - 1; k >= 0; k--)
	{
		if (k == 0)
		{
			pd3dCommandList->OMSetStencilRef(0);
			m_ppSampleAnimationObjects[k]->Render(pd3dCommandList, pCamera, m_nObjects, m_ppSoldierObjects, m_pnAnimationFactor[k], m_pPlayer, m_nCharacterIndex);
		}
		else if (k == 2)
		{
			pd3dCommandList->OMSetStencilRef(0);
			m_ppSampleAnimationObjects[k]->Render(pd3dCommandList, pCamera, m_nObjects, m_ppAliens, m_pnAnimationFactor[k], m_pPlayer, m_nCharacterIndex);
		}
		else
		{
			pd3dCommandList->OMSetStencilRef(1);
			m_ppSampleAnimationObjects[k]->Render(pd3dCommandList, pCamera, m_nObjects, m_ppDroneObjects, m_pnAnimationFactor[k], m_pPlayer, m_nCharacterIndex);
		}

	}
}
void CharacterScene::ShadowRender(ID3D12GraphicsCommandList *pd3dCommandList, CCamera* pCamera, SHADOW_INFO* pCameraInfo)
{
}
void CharacterScene::ReleaseShaderVariables()
{
}
ID3D12RootSignature *CharacterScene::CreateGraphicsRootSignature(ID3D12Device *pd3dDevice)
{
	ID3D12RootSignature *pd3dGraphicsRootSignature = NULL;

	D3D12_DESCRIPTOR_RANGE pd3dDescriptorRanges[7];

	pd3dDescriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	pd3dDescriptorRanges[0].RegisterSpace = 0;
	pd3dDescriptorRanges[0].NumDescriptors = 1;
	pd3dDescriptorRanges[0].BaseShaderRegister = 2;
	pd3dDescriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[1].NumDescriptors = 1;
	pd3dDescriptorRanges[1].BaseShaderRegister = 7;
	pd3dDescriptorRanges[1].RegisterSpace = 0;
	pd3dDescriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[2].NumDescriptors = 1;
	pd3dDescriptorRanges[2].BaseShaderRegister = 8;
	pd3dDescriptorRanges[2].RegisterSpace = 0;
	pd3dDescriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[3].NumDescriptors = 1;
	pd3dDescriptorRanges[3].BaseShaderRegister = 10; // 모델 디퓨즈맵
	pd3dDescriptorRanges[3].RegisterSpace = 0;
	pd3dDescriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[4].NumDescriptors = 1;
	pd3dDescriptorRanges[4].BaseShaderRegister = 11; // 모델 노말맵
	pd3dDescriptorRanges[4].RegisterSpace = 0;
	pd3dDescriptorRanges[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[5].NumDescriptors = 1;
	pd3dDescriptorRanges[5].BaseShaderRegister = 12; // 모델 스펙큘러맵
	pd3dDescriptorRanges[5].RegisterSpace = 0;
	pd3dDescriptorRanges[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	pd3dDescriptorRanges[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	pd3dDescriptorRanges[6].NumDescriptors = 1;
	pd3dDescriptorRanges[6].BaseShaderRegister = 0; // 그림자 맵
	pd3dDescriptorRanges[6].RegisterSpace = 0;
	pd3dDescriptorRanges[6].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER pd3dRootParameters[14];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; //Player
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1; //Object
	pd3dRootParameters[2].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[0];
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[1];//Test
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[4].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[4].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[2];//Test
	pd3dRootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[5].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[3];
	pd3dRootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[6].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[4];
	pd3dRootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[7].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[5];
	pd3dRootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	pd3dRootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[8].Descriptor.ShaderRegister = 0;
	pd3dRootParameters[8].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[9].Descriptor.ShaderRegister = 1;
	pd3dRootParameters[9].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	pd3dRootParameters[10].Descriptor.ShaderRegister = 2;
	pd3dRootParameters[10].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	pd3dRootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[11].Descriptor.ShaderRegister = 7; //Materials
	pd3dRootParameters[11].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[12].Descriptor.ShaderRegister = 8; //Lights
	pd3dRootParameters[12].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[13].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[13].DescriptorTable.pDescriptorRanges = &pd3dDescriptorRanges[6];
	pd3dRootParameters[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_STATIC_SAMPLER_DESC pd3dSamplerDescs[2];

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

	pd3dSamplerDescs[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	pd3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	pd3dSamplerDescs[1].MipLODBias = 0;
	pd3dSamplerDescs[1].MaxAnisotropy = 16;
	pd3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	pd3dSamplerDescs[1].MinLOD = 0;
	pd3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	pd3dSamplerDescs[1].ShaderRegister = 1;
	pd3dSamplerDescs[1].RegisterSpace = 0;
	pd3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	pd3dSamplerDescs[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

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