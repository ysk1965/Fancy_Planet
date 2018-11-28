#pragma once
#include "Mesh.h"
#include "Camera.h"
class CPlayer;
class CAnimationObject;

#define AdjustmentValue 60.0f
#define DRONE_HIT_TIME 1.0f

struct EffectConstant
{
	EffectConstant()
	{
		m_xmf4x4World = Matrix4x4::Identity();
		m_xmf4Color = XMFLOAT4A(0.6f, 0.6f, 0.0f, 1.0f);
		m_fAlpha = 1.0f;
		m_xmf4Emissive = XMFLOAT4A(1.0f, 1.0f, 1.0f, 1.0f);
	}
	XMFLOAT4X4						m_xmf4x4World;
	XMFLOAT4							m_xmf4Color;
	XMFLOAT4							m_xmf4Emissive;
	float									m_fAlpha;
};
struct EffectInfo
{
	float fScale = 1;
	float fTotalTime;
	XMFLOAT3 xmf3Veclocity;
	XMFLOAT3 xmf3Acceleration;

	DWORD dwdTime;
};

class Effect
{
protected:

	CMesh * *m_ppMeshes = NULL;
	UINT								m_nMeshes;
	UINT								m_nObjects;
	float								m_fTime;

	EffectConstant* m_pEffects = NULL;
	EffectInfo *m_pEffectsInfo = NULL;
	ID3D12Resource *m_pd3dcbEffect = NULL;
	EffectConstant* m_pcbMappedEffect = NULL;

	XMFLOAT3 m_xmf3Pos;
public:
	Effect(UINT nMeshes, UINT nObjects);
	~Effect();

	void SetParticlePosition(UINT nIndex, const XMFLOAT3& xmf3Position);

	void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);
	void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex) = 0;
	void SetMesh(CMesh* pMesh, UINT nMeshIndex);
};

class BillboardEffect : public Effect
{
protected:
	CAnimationObject * m_pPlayer;

	bool m_bEffectFlag = false;
	DWORD m_dwdLength;
public:
	BillboardEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer);
	~BillboardEffect();

	virtual void SetParticle(UINT nType);
	virtual void Init(UINT nIndex, const DWORD& dwdTime, bool bflag);
	virtual void Animate(float fHeight, const DWORD& dwdTime, const unsigned long& nCurrentFrameRate);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex);
	void SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& right);
};

class MeshEffect : public Effect
{
protected:
	CAnimationObject * m_pPlayer;
	XMFLOAT3 m_xmf3Target;

	bool m_bEffectFlag = false;
	DWORD m_dwdLength;
public:

	void SetTarget(XMFLOAT3& xmf3Target);
	void Animate(const DWORD& dwdTime);

	MeshEffect(UINT nMeshes, UINT nObjects, CAnimationObject * pPlayer);
	~MeshEffect();

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex);
};

class SparkEffect : public BillboardEffect
{
	XMFLOAT3* m_xmf3OriginPos;
	XMFLOAT3 m_xmfDifferenc;
public:
	virtual void SetParticle();

	SparkEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer);
	~SparkEffect();
	virtual void Init(UINT nIndex, const DWORD& dwdTime, bool bflag);
	virtual void Animate(const DWORD& dwdTime, const unsigned long& nCurrentFrameRate);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex);

};

class SmokeEffect : public BillboardEffect
{
public:
	SmokeEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer);
	~SmokeEffect();

	virtual void Init(UINT nIndex, const DWORD& dwdTime);
	virtual void Animate(float fHeight, const DWORD& dwdTime, const unsigned long& nCurrentFrameRate);
};
class DroneEffect : public BillboardEffect
{
private:
	bool bIsHit;
	DWORD m_dwHitTime;
public:
	DroneEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer);
	~DroneEffect();

	void SetHit();
	virtual void Init(UINT nIndex, const DWORD& dwdTime, bool bflag);
	virtual void Animate(float fHeight, const DWORD& dwdTime, const unsigned long& nCurrentFrameRate);
};

class TeleportEffect : public BillboardEffect
{
public:
	TeleportEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer);
	~TeleportEffect();

	void SetParticle();
	virtual void Init(UINT nIndex, const DWORD& dwdTime, bool bflag);
	virtual void Animate(const DWORD& dwdTime);
};

class JumpEffect : public BillboardEffect
{
public:
	JumpEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer);
	~JumpEffect();

	void SetParticle();
	virtual void Init(UINT nIndex, const DWORD& dwdTime, bool bflag);
	virtual void Animate(const DWORD& dwdTime, const unsigned long& nCurrentFrameRate);
};

class SphereEffect : public MeshEffect
{

public:
	SphereEffect(UINT nMeshes, UINT nObjects, CAnimationObject * pPlayer);
	~SphereEffect();

	void Init(UINT nIndex);
	void Animate(const DWORD& dwdTime, const unsigned long& nCurrentFrameRate);
	void SetTarget(XMFLOAT3& xmf3Target);
	void SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& right);

	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex);
};
class BloodEffect : public BillboardEffect
{
private:
	XMFLOAT3 m_xmf3Target;
public:
	BloodEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer);
	~BloodEffect();

	void Init(UINT nIndex, const XMFLOAT3& xmf3Dir);
	void SetTarget(XMFLOAT3& xmf3Target);
	void Animate(const DWORD& dwdTime, const unsigned long& nCurrentFrameRate);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, UINT nMeshIndex);
};
class DroneSkillEffect : public MeshEffect
{
public:
	DroneSkillEffect(UINT nMeshes, UINT nObjects, CAnimationObject* pPlayer);
	~DroneSkillEffect();

	void SetTarget();
	void Animate(const DWORD& dwdTime, XMFLOAT3& xmf3Dir);
};