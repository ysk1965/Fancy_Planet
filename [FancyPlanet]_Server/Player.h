#pragma once
#include "stdafx.h"
#include "DynamicActor.h"

extern array<Client, MAX_USER> g_clients;
extern HANDLE gh_iocp;
extern BASE base[3];
extern TEAM Team[Team_NUMBER];

class CPlayer :
	public PxUserControllerHitReport,
	public PxControllerBehaviorCallback
{
protected:
	PxController * m_pPxCharacterController;
	PxScene*      m_pScene;
	PxVec3         m_Translate;
	FLOAT         m_fFallvelocity;
	FLOAT         m_fFallAcceleration;
	
	XMFLOAT3      m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3        m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3      m_xmf3RotationX = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3      m_xmf3RotationY = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3      m_xmf3RotationZ = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3      m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3      m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3      m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT4X4      m_xmf4x4World;
	XMFLOAT4		m_xmf4Quaternion;
	float           m_fPitch = 0.0f;
	float           m_fYaw = 0.0f;
	float           m_fRoll = 0.0f;
	int            m_isMoveState;//서버
	int            m_ID;
	float         m_fTimeElapsed = 0.f;
	float         m_fInterpolationTime = 0.f;
	int            m_iHP;
	int            m_iTeam;
	int            m_iEnemyTeam;
	int            m_iSceneState;
	int            m_iCharacterType;
	bool         m_isAttackState = false;
	bool         m_isJumpState = false;
	bool         m_isDoubleJumpState = false;
	XMFLOAT3      m_xmf3CameraLookAt;
	bool         m_isRapidMoveState = false; // 원시인의 스킬.
	bool		 m_isFlashState = false; //드론 스킬
	bool		 m_isAttackSkillState = false; // 공격 스킬 플래그
	float         m_fMoveSkillCoolTime = 10.f;
	float         m_fMoveSkillTimeCheck = 0.f;

	float m_fRazerbuf = 0.f;
	float m_fRazerTickbuf = 0.f;
	float m_fRazerTick = 0.2f;
	float m_fRazerTime = 2.0f;
	DynamicActor   *TestBullet;
	CHeightMapTerrain*      m_pTerrain;
public:
	CPlayer();
	void BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial, PxControllerManager *pPxControllerManager, void* pContext, int id);
	void SetPosition(XMFLOAT3 vPosition);
	void Update(float fTimeElapsed);
	void UsingRazer(float timeelapse);
	
	void UsingAttackSkill();
	XMFLOAT4X4 GetWorldMatrix() const { return m_xmf4x4World; };
	bool IsOnGround(void);
	void PxMove(float speed, float fTimeElapsed);
	void Rotate();
	PxController* getPxController() {
		return m_pPxCharacterController;
	}

	void setTerrain(CHeightMapTerrain* pTerrain);

	void onShapeHit(const PxControllerShapeHit & hit);

	void AddForceAtLocalPos(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup);

	void AddForceAtPosInternal(PxRigidBody & body, const PxVec3 & force, const PxVec3 & pos, PxForceMode::Enum mode, bool wakeup);


	virtual void onControllerHit(const PxControllersHit& hit) {}
	virtual void onObstacleHit(const PxControllerObstacleHit& hit) {}

	virtual PxControllerBehaviorFlags      getBehaviorFlags(const PxShape& shape, const PxActor& actor);
	virtual PxControllerBehaviorFlags      getBehaviorFlags(const PxController& controller);
	virtual PxControllerBehaviorFlags      getBehaviorFlags(const PxObstacle& obstacle);
	int GetMoveState() { return m_isMoveState; }//서버
	void SetMoveState(int state) { m_isMoveState = state; }//서버
	void SetRotatePosition(XMFLOAT3 Look, XMFLOAT3 Up, XMFLOAT3 Right, const array <Client, MAX_USER> &arr);
	XMFLOAT3 GetPosition() {
		return XMFLOAT3(m_pPxCharacterController->getFootPosition().x,
			m_pPxCharacterController->getFootPosition().y,
			m_pPxCharacterController->getFootPosition().z);
	}

	void Jump();
	void Fire();
	void SetAttackSkillState(bool state) { m_isAttackSkillState = state; }
	void SetAttackState(bool state, XMFLOAT3 CameraLookAt) { m_isAttackState = state; m_xmf3CameraLookAt = CameraLookAt; }
	void SetLookVector(XMFLOAT3 xmf3Look) { m_xmf3Look = xmf3Look; }
	void SetUpVector(XMFLOAT3 xmf3Look) { m_xmf3Up = xmf3Look; }
	void SetRightVector(XMFLOAT3 xmf3Look) { m_xmf3Right = xmf3Look; }
	int GetPlayerTeam() { return m_iTeam; }
	void SetPlayerTeam(int team) { m_iTeam = team; }
	void SetCharacterType(int type) { m_iCharacterType = type; }
	void UsingRapidMove() { m_isRapidMoveState = true; }//원시인 스킬 사용
	void UsingTeleport() { m_isFlashState = true; }
	chrono::system_clock::time_point m_MoveSkillCoolTimer;
	void UsingDoubleJump();
	void DroneFlash();
	XMFLOAT4 GetQuaternion() { return m_xmf4Quaternion; }

	float m_fFrameAdvanceTime = 0.f;
	float m_fRespawnTime = 0.f;
};