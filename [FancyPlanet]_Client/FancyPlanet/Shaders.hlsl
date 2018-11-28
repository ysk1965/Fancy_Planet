
cbuffer cbPlayerInfo : register(b0)
{
	matrix		gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView;
	matrix		gmtxProjection;
	matrix		gmtxLightView;
	matrix		gmtxLightProjection;
	matrix		gmtxShadowTransform;
	float3		    gvCameraPosition;
	float			gfPad1;
	float3			gvLightPosition;
};

cbuffer cbGameObjectInfo : register(b2)
{
	matrix		gmtxWorld : packoffset(c0);
	uint		gnMaterial : packoffset(c4);
};

struct INSTANCEDGAMEOBJECTINFO
{
	matrix		gmtxWorld;
	float4x4 gmtxBoneTransforms[31];
};

#define SHADOW_DEPTH_BIAS 0.00005f

Texture2D gtxtTerrainBaseTexture : register(t4);
Texture2D gtxtTerrainDetailTexture : register(t5);
Texture2D gtxtTerrainNormalMap : register(t6);

Texture2D ShadowMap : register(t10);

SamplerState gWrapSamplerState : register(s0);
SamplerState gClampSamplerState : register(s1);
SamplerComparisonState gShadowSamplerState : register(s2);

#include "Light.hlsl"

struct VS_TERRAIN_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR; //지형의 기본 색
	float2 uv0 : TEXCOORD0; // 베이스 텍스쳐
	float2 uv1 : TEXCOORD1; // 디테일 텍스쳐
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};

struct VS_TERRAIN_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv0 : TEXCOORD0;
	float2 uv1 : TEXCOORD1;
	float3 normalW : NORMAL;
	float4 positionW : POSITION;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float4 ShadowPosH : SHADOW;
};



float CalcShadowFactor(float4 shadowPosH)
{
	// Complete projection by doing division by w.
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC space.
	float depth = shadowPosH.z;

	uint width, height, numMips;
	ShadowMap.GetDimensions(0, width, height, numMips);

	// Texel size.
	float dx = 1.0f / (float)width;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	for (int i = 0; i < 9; ++i)
	{
		percentLit += ShadowMap.SampleCmpLevelZero(gShadowSamplerState, shadowPosH.xy, depth).r;
	}

	return percentLit / 9.0f;
}

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;
	
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld);
	output.position = mul(mul(float4(output.positionW.xyz, 1.0f), gmtxView), gmtxProjection);
	output.normalW = normalize(mul(input.normal, (float3x3)gmtxWorld));
	output.tangentW = normalize(mul(input.tangent, (float3x3)gmtxWorld));
	output.bitangentW = normalize(cross(output.normalW, output.tangentW));
	output.uv1 = input.uv1;
	output.uv0 = input.uv0;
	output.ShadowPosH = mul(output.positionW, gmtxShadowTransform);
	return(output);
}
VS_TERRAIN_OUTPUT VSShadowTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output = (VS_TERRAIN_OUTPUT)0.0f;

	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld);
	output.position = mul(mul(float4(output.positionW.xyz, 1.0f), gmtxLightView), gmtxLightProjection);

	return(output);
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gWrapSamplerState, input.uv0);

	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gWrapSamplerState, input.uv1);

	float4 cColor = saturate((cBaseTexColor * 0.2f) + (cDetailTexColor * 0.8f));
	//cColor = float4(0.0f, 0.8f, 0.0f, 1.0f);
	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	// 노말맵으로 부터 법선벡터를 가져온다.
	float3 normal = gtxtTerrainNormalMap.Sample(gWrapSamplerState, input.uv0).rgb;
	// -1 와 1사이 값으로 변환한다.
	normal = 2.0f * normal - 1.0f;
	float3 normalW = mul(normal, TBN);

	float shadowFactor = CalcShadowFactor(input.ShadowPosH);
	float4 cllumination = Lighting(input.positionW, normalW, cColor, 0, 30, shadowFactor);

	return cllumination;
}
Texture2D<float4> gtxtDiffuse : register(t0);
Texture2D<float4> gtxtNormal : register(t1);
Texture2D<float4> gtxtDepth : register(t2);
Texture2D<float4> gtxtShadow : register(t3);

struct A_VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
	float3 tangentW : TANGENT;
};

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};
struct VS_ANIMATION_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 boneWeights:WEIGHTS;
	uint4 boneIndices : BONEINDICES;
};
struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextured(VS_TEXTURED_INPUT input)
{
	VS_TEXTURED_OUTPUT output;

	float3 positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.positionW = positionW;
	output.position = mul(mul(float4(positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;

	return(output);
}

Texture2D gtxtSkyBox : register(t9);

float4 PSSkyBox(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 diffuse = float4(gtxtSkyBox.Sample(gClampSamplerState, input.uv).xyz, 1.0f);

	return(diffuse);
}

