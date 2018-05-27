#pragma once

class CPhysXFramework
{
public:
	CPhysXFramework();
	~CPhysXFramework();

	bool OnCreate();

	void OnDestroy();

	void BuildObjects();

private:
	//Physx SDK Member Variables =========================
	PxPhysics*						m_pPxPhysicsSDK;
	PxScene*						m_pPxScene;
	PxMaterial*						m_pPxMaterial;
	PxControllerManager*			m_pPxControllerManager;
	PxFoundation*					m_pPxFoundation;
	PxDefaultErrorCallback			m_PxDefaultErrorCallback;
	PxDefaultAllocator				m_PxDefaultAllocatorCallback;
	PxVisualDebuggerConnection*     m_pPVDConnection;
	PxCooking*						m_pCooking;
	//====================================================
public:
	void FrameAdvance();
	/////////////// Physx SDK Member Function ///////////////
	void InitializePhysxEngine();
	void ReleasePhysxEngine();
};

