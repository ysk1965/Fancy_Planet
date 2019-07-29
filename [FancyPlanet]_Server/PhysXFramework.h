#pragma once
#include "stdafx.h"
#include "Scene.h"
#include "Player.h"
#include "GameTimer.h"
#include "protocol.h"
extern array<Client, MAX_USER> g_clients;
extern XMFLOAT3 g_xmf3ObjectsPos[OBJECTS_NUMBER];
extern XMFLOAT4 g_xmf3ObjectsQuaternion[OBJECTS_NUMBER];
extern float g_time;
extern TEAM Team[Team_NUMBER];
extern BASE base[3];
class CPhysXFramework
{
public:
	CPhysXFramework();
	~CPhysXFramework();

	bool OnCreate();

	void OnDestroy();

	void BuildObjects();

	CPlayer* GetPlayersInfo() const { return m_pPlayer; }
	float GetTimeElapsed() { return m_GameTimer.GetTimeElapsed(); }
private:
	
	PxPhysics*						m_pPxPhysicsSDK;
	PxScene*						m_pPxScene;
	PxMaterial*						m_pPxMaterial;
	PxControllerManager*			m_pPxControllerManager;
	PxFoundation*					m_pPxFoundation;
	PxDefaultErrorCallback			m_PxDefaultErrorCallback;
	PxDefaultAllocator				m_PxDefaultAllocatorCallback;
	PxVisualDebuggerConnection*     m_pPVDConnection;
	PxCooking*						m_pCooking;
private:
	CPlayer* m_pPlayer;
	PhysXScene* m_pScene = NULL;
	CGameTimer m_GameTimer;
	float m_fUpdateInterpolationTime=0.f;
	float m_fFrameAdvanceTime = 0.f;
	PxVec3 m_pv3Gravity= PxVec3(0.0f, -9.81f, 0.0f);
	int m_iObjectUpdateCount = 0;
public:
	void SetGravity(float gravity) { m_pv3Gravity.y = gravity; }
	void FrameAdvance();
	void InitializePhysxEngine();
	void ReleasePhysxEngine();
	void SendPacket(int id, void *ptr);
	bool GravityChangeState = false;
};