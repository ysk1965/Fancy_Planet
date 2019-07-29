#include "Object.h"

//////////////////////////////////
//                        //
//         CGameObject         //
//                        //
//////////////////////////////////

CGameObject::CGameObject()
{
}

void CGameObject::BuildObject(PxPhysics * pPxPhysics, PxScene * pPxScene, PxMaterial * pPxMaterial,
	XMFLOAT3 vScale, XMFLOAT3 vPosition, PxCooking * pCooking, const char * name)
{
}

void CGameObject::SetPosition(XMFLOAT3 & xmf3Position)
{
}

XMFLOAT3 CGameObject::GetPosition()
{
	return XMFLOAT3(m_xmf3Position);
}

void CGameObject::SetRotation(XMFLOAT4& pxmf4Quaternion)
{
}

XMFLOAT4 CGameObject::GetRotation()
{
	return XMFLOAT4(m_xmf4Quaternion);
}

//////////////////////////////////
//                        //
//         TerrainMesh         //
//                        //
//////////////////////////////////

CMesh::CMesh(UINT nVertices, XMFLOAT3 *pxmf3Positions, UINT nIndices, UINT *pnIndices)
{
	m_nStride = sizeof(CVertex);
	m_nVertices = nVertices;

	//피직스 관련 Vertex, Index
	m_pPxVtx = new PxVec3[nVertices];
	ZeroMemory(m_pPxVtx, sizeof(PxVec3) * nVertices);

	m_pPxIndex = new PxU32[nIndices];
	ZeroMemory(m_pPxIndex, sizeof(PxU32) * nIndices);

	CVertex *pVertices = new CVertex[m_nVertices];
	for (UINT i = 0; i < m_nVertices; i++) {
		pVertices[i] = CVertex(pxmf3Positions[i]);
		m_pPxVtx->x = pxmf3Positions->x;
		m_pPxVtx->y = pxmf3Positions->y;
		m_pPxVtx->z = pxmf3Positions->z;
	}
}

CMesh::~CMesh()
{
}

//////////////////////////////////
//                        //
//         HeightMap         //
//                        //
//////////////////////////////////

CHeightMapTerrain::CHeightMapTerrain(LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color,
	PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking)
{
	// PxHeightField : x축을 따라 x축, y축을 따라... z축을 따라... 스케일 된 PxTriangleMesh로 모양을 만든다.
	// 먼저 만들어야 할 것들
	PxRigidActor* myActor = NULL;
	PxTriangleMesh* myTriMesh = NULL;
	PxMaterial* myMaterial = NULL;

	m_nWidth = nWidth;
	m_nLength = nLength;

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)
		m_ppMeshes[i] = NULL;

	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			m_pHeightMapGridMesh = new CHeightMapGridMesh(pPxPhysicsSDK, pPxScene, pPxControllerManager, pCooking, myActor, myTriMesh, myMaterial,
				xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			//SetMesh(x + (z*cxBlocks), pHeightMapGridMesh);
		}
	}

}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage)
		delete m_pHeightMapImage;
}

float CHeightMapTerrain::GetHeight(float x, float z)
{
	return 0.0f;
}

//////////////////////////////////////////////////////////////////////////////////////////

CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE *pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y)*m_nWidth)] = pHeightMapPixels[x + (y*m_nWidth)];
		}
	}

	if (pHeightMapPixels)
		delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels) delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z*m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z*m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1)*m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1)*m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

CHeightMapGridMesh::CHeightMapGridMesh(PxPhysics* pPxPhysicsSDK, PxScene* pPxScene, PxControllerManager* pPxControllerManager, PxCooking* pCooking, PxRigidActor* PhysXRigidActor, PxTriangleMesh* PhysXTriangleMesh, PxMaterial* PhysXMeterial,
	int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void *pContext)
{
	const PxReal heightScale = 0.01f;

	PxU32 hfNumVerts = nWidth * nLength;

	PxHeightFieldSample* samples = (PxHeightFieldSample*)malloc(sizeof(PxHeightFieldSample)*hfNumVerts);
	memset(samples, 0, hfNumVerts * sizeof(PxHeightFieldSample));



	//PhysXRigidActor = 
	//PhysXTriangleMesh = pCooking->createTriangleMesh(meshDesc, pPxPhysicsSDK->getPhysicsInsertionCallback());
	PhysXMeterial = pPxPhysicsSDK->createMaterial(0.5f, 0.5f, 0.2f);


	m_nVertices = nWidth * nLength;
	//   m_nStride = sizeof(CTexturedVertex);
	m_nStride = sizeof(CVertex);
	m_nOffset = 0;
	m_nSlot = 0;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	//   CTexturedVertex *pVertices = new CTexturedVertex[m_nVertices];
	CVertex *pVertices = new CVertex[m_nVertices];

	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = OnGetHeight(x, z, pContext);

			PxI32 h = fHeight * 0.335;
			PX_ASSERT(h <= 0xffff);
			samples[z + x * nLength].height = (PxI16)(h);
			samples[z + x * nLength].setTessFlag();
			samples[z + x * nLength].materialIndex0 = 1;
			samples[z + x * nLength].materialIndex1 = 1;

		}
	}
	PxHeightFieldDesc hfDesc;
	hfDesc.format = PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns = nWidth;
	hfDesc.nbRows = nLength;
	hfDesc.samples.data = samples;
	hfDesc.samples.stride = sizeof(PxHeightFieldSample);

	PxHeightField* heightField = pPxPhysicsSDK->createHeightField(hfDesc);

	PxTransform pose = PxTransform(PxIdentity);
	pose.p = PxVec3(0, 0, 0);

	PxRigidStatic* hfActor = pPxPhysicsSDK->createRigidStatic(pose);

	PxHeightFieldGeometry hfGeom(heightField, PxMeshGeometryFlags(), m_xmf3Scale.y, m_xmf3Scale.x, m_xmf3Scale.z);
	PxShape* hfShape = hfActor->createShape(hfGeom, *PhysXMeterial);

	hfActor->setName("HeightField");

	pPxScene->addActor(*hfActor);



	//delete[] pVertices;

	//m_nIndices = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	//UINT *pnIndices = new UINT[m_nIndices];

	//for (int j = 0, z = 0; z < nLength - 1; z++)
	//{
	//   if ((z % 2) == 0)
	//   {
	//      for (int x = 0; x < nWidth; x++)
	//      {
	//         if ((x == 0) && (z > 0)) pnIndices[j++] = (UINT)(x + (z * nWidth));
	//         pnIndices[j++] = (UINT)(x + (z * nWidth));
	//         pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
	//      }
	//   }
	//   else
	//   {
	//      for (int x = nWidth - 1; x >= 0; x--)
	//      {
	//         if (x == (nWidth - 1)) pnIndices[j++] = (UINT)(x + (z * nWidth));
	//         pnIndices[j++] = (UINT)(x + (z * nWidth));
	//         pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
	//      }
	//   }
	//}


	//delete[] pnIndices;
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void *pContext)
{
	CHeightMapImage *pHeightMapImage = (CHeightMapImage *)pContext;
	BYTE *pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z*nWidth)] * xmf3Scale.y;
	return(fHeight);
}
