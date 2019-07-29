#include "CCloth.h"

CCloth::CCloth()
{
	m_pCloth = nullptr;
	m_Time = 0.0f;
	m_iVtxCount = 0;
	m_fWindCount = 0;
}

CCloth::~CCloth(void)
{
	if (m_pCloth)
		m_pCloth->release();
}

int CCloth::Update(const float& fTimeDelta)
{
	m_fWindCount += fTimeDelta;

	if (m_fWindCount >= 30.f)
	{
		m_WindDir.x *= -1.f;

		m_fWindCount = 0.0f;
	}

	//CGameObject::Update(fTimeDelta);

	UpdateWind(fTimeDelta);

	ClothPhysXUpdate(fTimeDelta);


	PxClothParticleData* data = m_pCloth->lockParticleData();

	PxClothParticle* pa = data->particles;

	return 0;
}

void CCloth::BuildObject(PxPhysics* pPxPhysics, PxScene* pPxScene, PxMaterial *pPxMaterial, XMFLOAT3 vScale, PxCooking* pCooking, const char* name)
{
	PxQuat q = PxQuat(PxIdentity);

	PxTransform m_tPxPos = PxTransform(PxVec3(0.f, 0.f, 0.f), q);

	PxReal height = 5.f;

	vector<PxVec4> vertices;
	vector<PxU32> primitives;
	vector<PxVec2>     mUVs;

	PxClothMeshDesc meshDesc = CreateMeshGrid(PxVec3(5, 0, 0), PxVec3(0, -5, 0), 5, 5, vertices, primitives, mUVs);

	for (PxU32 i = 0; i < meshDesc.points.count; i++)
	{
		PxReal u = mUVs[i].x, v = mUVs[i].y;
		bool constrain = ((u < 0.01) && (v < 0.01)) || ((u < 0.01) && (v > 0.99));
		vertices[i].w = constrain ? 0.0f : 1.0f;
	}

	PxClothFabric* clothFabric = PxClothFabricCreate(*pPxPhysics, meshDesc, PxVec3(0, -1, 0));
	PX_ASSERT(meshDesc.points.stride == sizeof(PxVec4));


	const PxClothParticle* particles = (const PxClothParticle*)meshDesc.points.data;
	m_pCloth = pPxPhysics->createCloth(m_tPxPos, *clothFabric, particles, PxClothFlags());
	PX_ASSERT(m_pCloth);

	m_pCloth->setName(name);
	pPxScene->addActor(*m_pCloth);

	m_pCloth->setSolverFrequency(240);
	m_pCloth->setDampingCoefficient(PxVec3(0.0f));

	m_pCloth->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(0.1f));
	m_pCloth->setTetherConfig(PxClothTetherConfig(1.0f, 1.0f));
}

PxClothMeshDesc CCloth::CreateMeshGrid(PxVec3 dirU, PxVec3 dirV, PxU32 numU, PxU32 numV, vector<PxVec4>& vertices, vector<PxU32>& indices, vector<PxVec2>& texcoords)
{
	int numVertices = numU * numV;
	int numQuads = (numU - 1) * (numV - 1); 

	vertices.resize(numVertices);
	indices.resize(numQuads * 4);
	texcoords.resize(numVertices);

	PxReal scaleU = 1 / PxReal(numU - 1); 
	PxReal scaleV = 1 / PxReal(numV - 1); 


	PxVec4* posIt = &(*vertices.begin());
	PxVec2* texIt = &(*texcoords.begin());

	for (PxU32 i = 0; i < numV; i++)
	{
		PxReal texV = i * scaleV;
		PxVec3 posV = (texV - 0.5f) * dirV;
		for (PxU32 j = 0; j < numU; j++)
		{
			PxReal texU = j * scaleU;
			PxVec3 posU = (texU - 0.5f) * dirU;

			PxVec4 pos = PxVec4(posU + posV, 1.0f);
			PxVec2 tex = PxVec2(texU, 1.0f - texV);

			*posIt++ = pos;
			*texIt++ = tex;
		}
	}

	PxU32* idxIt = &(*indices.begin());
	for (PxU32 i = 0; i < numV - 1; i++)
	{
		for (PxU32 j = 0; j < numU - 1; j++)
		{
			PxU32 i0 = i * numU + j;
			*idxIt++ = i0;
			*idxIt++ = i0 + 1;
			*idxIt++ = i0 + numU + 1;
			*idxIt++ = i0 + numU;
		}
	}

	PxClothMeshDesc meshDesc;

	meshDesc.points.data = &(*vertices.begin());
	meshDesc.points.count = static_cast<PxU32>(numVertices);
	meshDesc.points.stride = sizeof(PxVec4);

	meshDesc.invMasses.data = &(*vertices.begin()).w;
	meshDesc.invMasses.count = static_cast<PxU32>(numVertices);
	meshDesc.invMasses.stride = sizeof(PxVec4);

	meshDesc.quads.data = &(*indices.begin());
	meshDesc.quads.count = static_cast<PxU32>(numQuads);
	meshDesc.quads.stride = sizeof(PxU32) * 4;

	return meshDesc;


}

void CCloth::SetWind(const PxVec3 & dir, PxReal strength, const PxVec3 & range)
{
	m_WindStrength = strength;
	m_WindDir = dir;
	m_WindRange = range;
}

PxF32 CCloth::WindRand(PxF32 a, PxF32 b)
{
	return ((b - a)*((float)rand() / RAND_MAX)) + a;
}

void CCloth::UpdateWind(const float& fTimeDelta)
{
	m_Time += fTimeDelta * WindRand(0.0f, 1.0f);

	float st = 1.0f + (float)sin(m_Time);

	float windStrength = WindRand(1.0f, st) * m_WindStrength;
	float windRangeStrength = WindRand(0.0f, 0.5f);

	PxVec3 offset(PxReal(WindRand(-1, 1)), PxReal(WindRand(-1, 1)), PxReal(WindRand(-1, 1)));
	float ct = 1.0f + (float)cos(m_Time + 0.1);
	offset *= ct;
	PxVec3 windAcceleration = windStrength * m_WindDir + windRangeStrength * m_WindRange.multiply(offset);
	m_pCloth->setExternalAcceleration(windAcceleration);
}

void CCloth::ClothPhysXUpdate(const float & fTimeDelta)
{
	//PxTransform pT = m_pCloth->getGlobalPose();
	//PxMat44 m = PxMat44(pT);
	//PxVec3 RotatedOffset = m.rotate(PxVec3(0.0f, 0.0f, 0.0f));
	//m.setPosition(PxVec3(m.getPosition() + RotatedOffset));

	//XMStoreFloat4x4(&m_pTransform->m_matWorld, (XMMATRIX)m.front());
}

void CCloth::ClothSetPosition(XMFLOAT3 vPosition)
{
	PxTransform _PxTransform = m_pCloth->getGlobalPose();

	_PxTransform.p = PxVec3(vPosition.x, vPosition.y, vPosition.z);

	m_pCloth->setGlobalPose(_PxTransform);
}

void CCloth::ClothSetRotate(XMFLOAT3 vRot)
{
	PxTransform _PxTransform = m_pCloth->getGlobalPose();

	_PxTransform.q *= PxQuat(vRot.x, PxVec3(1, 0, 0));
	_PxTransform.q *= PxQuat(vRot.z, PxVec3(0, 1, 0));
	_PxTransform.q *= PxQuat(vRot.y, PxVec3(0, 0, 1));

	m_pCloth->setGlobalPose(_PxTransform);
}
