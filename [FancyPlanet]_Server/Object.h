#pragma once
#include "stdafx.h"

class CVertex
{
public:
	XMFLOAT3                  m_xmf3Position;

public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(float x, float y, float z) { m_xmf3Position = XMFLOAT3(x, y, z); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() { }
};

class CMesh
{
public:
	CMesh() {};
	CMesh(UINT nVertices, XMFLOAT3 *pxmf3Positions, UINT nIndices, UINT *pnIndices);
	UINT GetStartIndex()
	{
		return m_nStartIndex;
	};
	void SetStartIndex(UINT nStartIndex)
	{
		m_nStartIndex = nStartIndex;
	};
	UINT GetStride()
	{
		return m_nStride;
	};
	virtual ~CMesh();

private:
	int                        m_nReferences = 0;

public:
	void AddRef()
	{
		m_nReferences++;
	}
	void Release()
	{
		if (--m_nReferences <= 0)
			delete this;
	}

protected:
	UINT                     m_nSlot = 0;
	UINT                     m_nStride = 0;
	UINT                     m_nOffset = 0;

	UINT                     m_nIndices = 0;
	UINT                     m_nStartIndex = 0;
	int                        m_nBaseVertex = 0;

	//[PhysX]
	PxVec3*         m_pPxVtx;
	PxU32*         m_pPxIndex;

public:
	UINT                     m_nVertices = 0;
};

///////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////


class CGameObject {
protected:
	XMFLOAT3      m_xmf3Position;
	XMFLOAT4      m_xmf4Quaternion;
private:
	int                        m_nReferences = 0;
	UINT                     m_nObjects = 0;
public:
	void AddRef()
	{
		m_nReferences++;
	}
	void Release()
	{
		if (--m_nReferences <= 0)
			delete this;
	}

	CGameObject();

	virtual void BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial,
		XMFLOAT3 vScale, XMFLOAT3 vPosition, PxCooking* pCooking, const char* name);
	virtual void SetPosition(XMFLOAT3& xmf3Position);
	virtual XMFLOAT3 GetPosition();
	virtual void SetRotation(XMFLOAT4& pxmf4Quaternion);
	virtual XMFLOAT4 GetRotation();

	CMesh                     **m_ppMeshes;
	int                        m_nMeshes;
};

/////////////////////////////////////////////////////////////////////////////////////


class CHeightMapImage
{
private:
	BYTE * m_pHeightMapPixels;

	int                     m_nWidth;
	int                     m_nLength;
	XMFLOAT3               m_xmf3Scale;

public:
	CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~CHeightMapImage(void);

	float GetHeight(float fx, float fz, bool bReverseQuad);

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }

	BYTE *GetHeightMapPixels() { return(m_pHeightMapPixels); }
	int GetHeightMapWidth() { return(m_nWidth); }
	int GetHeightMapLength() { return(m_nLength); }
};

class CHeightMapGridMesh : public CMesh
{
protected:
	int                     m_nWidth;
	int                     m_nLength;
	XMFLOAT3               m_xmf3Scale;

public:
	CHeightMapGridMesh(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager,
		PxCooking* pCooking, PxRigidActor* PhysXRigidActor, PxTriangleMesh* PhysXTriangleMesh,
		PxMaterial* PhysXMeterial, int xStart, int zStart, int nWidth, int nLength,
		XMFLOAT3 xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f), void *pContext = NULL);
	virtual ~CHeightMapGridMesh();

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual float OnGetHeight(int x, int z, void *pContext);
};
/////////////////////////////////////////////////////////////////////////////////////////////////

class CHeightMapTerrain : CGameObject
{
public:
	CHeightMapTerrain(LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color,
		PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking);
	virtual ~CHeightMapTerrain();

private:
	CHeightMapImage * m_pHeightMapImage;

	int                     m_nWidth;
	int                     m_nLength;

	XMFLOAT3               m_xmf3Scale;
	CHeightMapGridMesh *      m_pHeightMapGridMesh;
public:
	//float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad) * m_xmf3Scale.y); } //World

	float GetHeight(float x, float z);

	int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
	int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

	CHeightMapGridMesh * GetGridMesh() { return(m_pHeightMapGridMesh); }
	CHeightMapImage * GetGridImage() { return(m_pHeightMapImage); }

	XMFLOAT3 GetScale() { return(m_xmf3Scale); }
	float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
	float GetLength() { return(m_nLength * m_xmf3Scale.z); }

};