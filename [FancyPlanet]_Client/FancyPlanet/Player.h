#pragma once

#define DIR_FORWARD				0x01
#define DIR_BACKWARD			0x02
#define DIR_LEFT				0x04
#define DIR_RIGHT				0x08
#define DIR_UP					0x10
#define DIR_DOWN				0x20

#define FLAGE_NUM			3

#define JUMP 0
#define LAUNCH 1
#define ETC 2

#include "Object.h"
#include "Camera.h"

struct CB_PLAYER_INFO
{
	XMFLOAT4X4					m_xmf4x4World;
};

class CPlayer : public CGameObject
{
protected:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float           			m_fPitch = 0.0f;
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	LPVOID						m_pPlayerUpdatedContext = NULL;
	LPVOID						m_pCameraUpdatedContext = NULL;

	CCamera						*m_pCamera = NULL;

	float						m_fSpeed = 1.0f;
	float						m_fTimeDelta = 0.0f;

	bool						m_bJumpState = false;
private:
	XMFLOAT2				m_xmf2MousPos;
	bool						m_bFalgs[FLAGE_NUM];
	DWORD                m_dwStart[FLAGE_NUM];

	CGameObject* pSphereMesh; // 충돌박스 입히기
	CAnimationObject* m_pRenderObject;
	bool m_bAttackedState = false;
	bool m_bKeySwitch = true;
public:
	void SetMousPos(const float& fX, const float& fY)
	{
		m_xmf2MousPos.x = fX;
		m_xmf2MousPos.y = fY;
	}
	XMFLOAT2& GetMousPos()
	{
		return m_xmf2MousPos;
	}
	bool GetFlag(int index)
	{
		return m_bFalgs[index];
	}
	void SetFlag(int Index, bool bflag)
	{
		m_bFalgs[Index] = bflag;
	}
	DWORD& GetStartTime(int Index)
	{
		return m_dwStart[Index];
	}
	void SetStartTime(const DWORD& dwTime, int Index)
	{
		m_dwStart[Index] = dwTime;
	}
	bool GetJumpState()
	{
		return m_bJumpState;
	}
	void SetJumpState(bool bJumpState)
	{
		m_bJumpState = bJumpState;
	}
	void SetKeySwitch(bool bKeySwitch)
	{
		m_bKeySwitch = bKeySwitch;
	}
	bool GetKeySwitch()
	{
		return m_bKeySwitch;
	}
	void SetRenderObject(CAnimationObject* pRenderObject)
	{
		m_pRenderObject = pRenderObject;
	}
	CAnimationObject* GetRenderObject()
	{
		return m_pRenderObject;
	}
	XMFLOAT4X4& GetPlayerWorldTransform()
	{
		return m_pRenderObject->m_xmf4x4World;
	}
	bool GetBeAttackedState()
	{
		return m_bAttackedState;
	}
	void SetBeAttackedStateToFalse()
	{
		m_bAttackedState = false;
	}
	void SetWorldPosition(const XMFLOAT4X4& xmf3Pos)
	{
		m_pCamera->Move(m_xmf3Position.x - xmf3Pos._41, m_xmf3Position.y - xmf3Pos._42, m_xmf3Position.z - xmf3Pos._43);
		m_xmf4x4World = xmf3Pos;
		m_xmf3Position.x= xmf3Pos._41;
		m_xmf3Position.y= xmf3Pos._42;
		m_xmf3Position.z= xmf3Pos._43;
	}
public:
	CPlayer(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature
		, int nMeshes = 1);
	virtual ~CPlayer();
	void	BuildObject();
	XMFLOAT3 GetPosition()
	{
		return(m_xmf3Position);
	}
	XMFLOAT3 GetLookVector()
	{
		return(m_xmf3Look);
	}
	XMFLOAT3 GetUpVector()
	{
		return(m_xmf3Up);
	}
	XMFLOAT3 GetRightVector()
	{
		return(m_xmf3Right);
	}
	void SetPosition(const XMFLOAT3& xmf3Position)
	{
		Move(XMFLOAT3(xmf3Position.x - m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false);
	}

	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	CCamera *GetCamera() { return(m_pCamera); }
	void SetCamera(CCamera *pCamera) { m_pCamera = pCamera; }

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
	void Rotate(float x, float y, float z);

	void Update(float fTimeElapsed);

	virtual void OnPlayerUpdateCallback(float fTimeElapsed);
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	virtual void ChangeAnimation() {};
	virtual void UpdateTransform(XMFLOAT4X4 *pxmf4x4Parent = NULL) {};
	CCamera *OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera *ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);
	virtual void Animate(float fTimeElapsed);

protected:
	ID3D12Resource * m_pd3dcbPlayer = NULL;
	CB_PLAYER_INFO					*m_pcbMappedPlayer = NULL;
};

