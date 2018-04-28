cbuffer cbPlayerInfo : register(b0)
{
	matrix		gmtxPlayerWorld : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
	float3		gvCameraPosition : packoffset(c8);
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

struct INSTANCEDGAMEOBJECTINFO2
{
	float4x4 gmtxBoneTransforms[32];
};

struct INSTANCEDGAMEOBJECTINFO3
{
	float4x4 gmtxBoneTransforms[26];
	matrix	gmtxRendererMeshWorld[6];
};

Texture2D<float4> gtxtDiffuse : register(t0);
Texture2D<float4> gtxtNormal : register(t1);
Texture2D<float4> gtxtDepth : register(t2);
Texture2D<float4> gtxtSpecular : register(t3);

Texture2D gtxtModel_Diffuse : register(t10);
Texture2D gtxtModel_Normal : register(t11);
Texture2D gtxtModel_Specular : register(t12);

StructuredBuffer<INSTANCEDGAMEOBJECTINFO> gAnimationObjectInfos : register(t0);
StructuredBuffer<INSTANCEDGAMEOBJECTINFO2> gAnimationObjectInfos2 : register(t1);
StructuredBuffer<INSTANCEDGAMEOBJECTINFO3> gAnimationObjectInfos3 : register(t2);

SamplerState gWrapSamplerState : register(s0);

#include "Light.hlsl"

struct PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT
{
	float4 diffuse : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 depth : SV_TARGET2;
	float4 specular : SV_TARGET3;
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

VS_TEXTURED_OUTPUT VSTexturedAnimation(VS_ANIMATION_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_TEXTURED_OUTPUT output;

	float4 fWeight = { 0.0f, 0.0f, 0.0f, 0.0f };
	fWeight[0] = input.boneWeights.x;
	fWeight[1] = input.boneWeights.y;
	fWeight[2] = input.boneWeights.z;
	fWeight[3] = 1.0f - fWeight[0] - fWeight[1] - fWeight[2];

	float3 position = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; i++)
	{
		if (input.boneIndices[i] < 31)
		{
			position += fWeight[i] * mul(float4(input.position, 1.0f), gAnimationObjectInfos[nInstanceID].gmtxBoneTransforms[input.boneIndices[i]]).xyz;
		}
		else if (input.boneIndices[i] >= 31 && input.boneIndices[i] < 63)
		{
			position += fWeight[i] * mul(float4(input.position, 1.0f), gAnimationObjectInfos2[nInstanceID].gmtxBoneTransforms[input.boneIndices[i] - 31]).xyz;
		}
		else
		{
			position += fWeight[i] * mul(float4(input.position, 1.0f), gAnimationObjectInfos3[nInstanceID].gmtxBoneTransforms[input.boneIndices[i] - 63]).xyz;
		}
	}

	output.uv = input.uv;
	output.positionW = mul(float4(position, 1.0f), gAnimationObjectInfos[nInstanceID].gmtxWorld).xyz;
	matrix mtxWorldViewProjection = mul(gAnimationObjectInfos[nInstanceID].gmtxWorld, gmtxView);
	mtxWorldViewProjection = mul(mtxWorldViewProjection, gmtxProjection);
	output.position = mul(float4(position, 1.0f), mtxWorldViewProjection);

	return output;
}
[earlydepthstencil]
PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT PSTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
	PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT output;
	float2 uv1;
	uv1.x = input.uv.x;
	uv1.y = 1.0f - input.uv.y;
	
	float3 diffuse = gtxtModel_Diffuse.Sample(gWrapSamplerState, uv1).rgb;
	
	output.depth = float4(input.positionW, 1.0f);
	output.diffuse = float4(diffuse, 1.0f);
	output.normal = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	return output;
}

struct A_VS_INPUT
{
	float3 position:POSITION;
	float3 normal:NORMAL;
	float2 uv:TEXCOORD;
	float3 tangent : TANGENT;
	float3 boneWeights:WEIGHTS;
	uint4 boneIndices : BONEINDICES;
};

struct A_VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 uv : TEXCOORD;
	float3 tangentW : TANGENT;
};
A_VS_OUTPUT ANIMATION_VS(A_VS_INPUT input, uint nInstanceID : SV_InstanceID)
{
	A_VS_OUTPUT output;

	float4 fWeight = { 0.0f, 0.0f, 0.0f, 0.0f };
	fWeight[0] = input.boneWeights.x;
	fWeight[1] = input.boneWeights.y;
	fWeight[2] = input.boneWeights.z;
	fWeight[3] = 1.0f - fWeight[0] - fWeight[1] - fWeight[2];

	float3 position = float3(0.0f, 0.0f, 0.0f);
	float3 normal = float3(0.0f, 0.0f, 0.0f);
	float3 tangent = float3(0.0f, 0.0f, 0.0f);
	//float4x4 qwe = float4x4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);


	for (int i = 0; i < 4; i++)
	{
		if (input.boneIndices[i] < 31)
		{
			position += fWeight[i] * mul(float4(input.position, 1.0f), gAnimationObjectInfos[nInstanceID].gmtxBoneTransforms[input.boneIndices[i]]).xyz;
			normal += fWeight[i] * mul(input.normal, (float3x3)gAnimationObjectInfos[nInstanceID].gmtxBoneTransforms[input.boneIndices[i]]);
			tangent += fWeight[i] * mul(input.tangent.xyz, (float3x3)gAnimationObjectInfos[nInstanceID].gmtxBoneTransforms[input.boneIndices[i]]);
		}
		else if (input.boneIndices[i] >= 31 && input.boneIndices[i] < 63)
		{
			position += fWeight[i] * mul(float4(input.position, 1.0f), gAnimationObjectInfos2[nInstanceID].gmtxBoneTransforms[input.boneIndices[i] - 31]).xyz;
			normal += fWeight[i] * mul(input.normal, (float3x3)gAnimationObjectInfos2[nInstanceID].gmtxBoneTransforms[input.boneIndices[i] - 31]);
			tangent += fWeight[i] * mul(input.tangent.xyz, (float3x3)gAnimationObjectInfos2[nInstanceID].gmtxBoneTransforms[input.boneIndices[i] - 31]);
		}
		else
		{
			position += fWeight[i] * mul(float4(input.position, 1.0f), gAnimationObjectInfos3[nInstanceID].gmtxBoneTransforms[input.boneIndices[i] - 63]).xyz;
			normal += fWeight[i] * mul(input.normal, (float3x3)gAnimationObjectInfos3[nInstanceID].gmtxBoneTransforms[input.boneIndices[i] - 63]);
			tangent += fWeight[i] * mul(input.tangent.xyz, (float3x3)gAnimationObjectInfos3[nInstanceID].gmtxBoneTransforms[input.boneIndices[i] - 63]);
		}
	}

	output.uv = input.uv;
	output.positionW = mul(float4(position, 1.0f), gAnimationObjectInfos[nInstanceID].gmtxWorld).xyz;
	output.normalW = mul(normal, (float3x3)gAnimationObjectInfos[nInstanceID].gmtxWorld);
	output.tangentW = mul(tangent, (float3x3)gAnimationObjectInfos[nInstanceID].gmtxWorld);
	matrix mtxWorldViewProjection = mul(gAnimationObjectInfos[nInstanceID].gmtxWorld, gmtxView);
	mtxWorldViewProjection = mul(mtxWorldViewProjection, gmtxProjection);
	output.position = mul(float4(position, 1.0f), mtxWorldViewProjection);

	return output;
}
[earlydepthstencil]
PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT PS_TDL(A_VS_OUTPUT input) : SV_TARGET
{
	PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT output;
	float2 uv1;
	
	uv1.x = input.uv.x;
	uv1.y = 1.0f - input.uv.y;
	
	float3 diffuse = gtxtModel_Diffuse.Sample(gWrapSamplerState, uv1).rgb;
	
	output.diffuse = float4(diffuse, 1.0f);
	
	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	// 노말맵으로 부터 법선벡터를 가져온다.
	float3 normal = gtxtModel_Normal.Sample(gWrapSamplerState, uv1).rgb;
	// -1 와 1사이 값으로 변환한다.
	normal = 2.0f * normal - 1.0f;
	float3 normalW = mul(normal, TBN);
	output.normal = float4(normalW, 1.0f);
	
	output.depth = float4(input.positionW, 1.0f);
	
	output.specular = float4((float)gnMaterial, (float)gnMaterial, (float)gnMaterial, 15.0f / 255.0f);
	
	return output;
}

struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	uint index : INDEX;
};
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normalW : NORMAL;
	float3 positionW : POSITION;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
};

VS_OUTPUT RendererMesh_VS(VS_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gAnimationObjectInfos3[nInstanceID].gmtxRendererMeshWorld[input.index]);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = normalize(mul(input.normal, (float3x3)gAnimationObjectInfos3[nInstanceID].gmtxRendererMeshWorld[input.index]));
	output.tangentW = normalize(mul(input.tangent, (float3x3)gAnimationObjectInfos3[nInstanceID].gmtxRendererMeshWorld[input.index]));
	output.bitangentW = normalize(cross(output.normalW, output.tangentW));
	output.uv = input.uv;

	return(output);
}

[earlydepthstencil]
PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT RendererMesh_PS(VS_OUTPUT input) : SV_TARGET
{
	PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT output;
	float2 uv1;
	uv1.x = input.uv.x;
	uv1.y = 1.0f - input.uv.y;

	float3 diffuse = gtxtModel_Diffuse.Sample(gWrapSamplerState, uv1).rgb;
	
	output.diffuse = float4(diffuse, 1.0f);
	
	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	// 노말맵으로 부터 법선벡터를 가져온다.
	float3 normal = gtxtModel_Normal.Sample(gWrapSamplerState, uv1).rgb;
	// -1 와 1사이 값으로 변환한다.
	normal = 2.0f * normal - 1.0f;
	float3 normalW = mul(normal, TBN);
	output.normal = float4(normalW, 1.0f);
	
	output.depth = float4(input.positionW, 1.0f);
	
	output.specular = float4(1.0f, 1.0f, 1.0f, 64.0f / 255.0f);
	
	return output;
}