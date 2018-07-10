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

struct INSTANCEDGAMEOBJECTINFO
{
	matrix		gmtxWorld;
};

SamplerState gWrapSamplerState : register(s0);

#include "Light.hlsl"

struct PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT
{
	float4 diffuse : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 depth : SV_TARGET2;
	float4 shadow : SV_TARGET3;
};

Texture2D gtxtObject_Diffuse : register(t0);
Texture2D gtxtObject_Normal : register(t1);
Texture2D gtxtObject_Shadow : register(t2);

StructuredBuffer<INSTANCEDGAMEOBJECTINFO> gObjectInfos : register(t3);

struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
};
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normalW : NORMAL;
	float3 positionW : POSITION;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
	float4 ShadowPosH : SHADOW;
};

VS_OUTPUT Object_VS(VS_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gObjectInfos[nInstanceID].gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = normalize(mul(input.normal, (float3x3)gObjectInfos[nInstanceID].gmtxWorld));
	output.tangentW = normalize(mul(input.tangent, (float3x3)gObjectInfos[nInstanceID].gmtxWorld));
	output.bitangentW = normalize(cross(output.normalW, output.tangentW));
	output.uv = input.uv;
	output.ShadowPosH = mul(output.positionW, gmtxShadowTransform);
	return(output);
}
VS_OUTPUT Shadow_Object_VS(VS_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_OUTPUT output = (VS_OUTPUT)0.0f;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gObjectInfos[nInstanceID].gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxLightView), gmtxLightProjection);
	output.uv = input.uv;
	return(output);
}
[earlydepthstencil]
PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT Object_PS(VS_OUTPUT input) : SV_TARGET
{
	PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT output;
	float2 uv1;
	uv1.x = input.uv.x;
	uv1.y = 1.0f - input.uv.y;

	float3 diffuse = gtxtObject_Diffuse.Sample(gWrapSamplerState, uv1).rgb;
	
	output.diffuse = float4(diffuse, 1.0f);
	
	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	// 노말맵으로 부터 법선벡터를 가져온다.
	float3 normal = gtxtObject_Normal.Sample(gWrapSamplerState, uv1).rgb;
	// -1 와 1사이 값으로 변환한다.
	normal = 2.0f * normal - 1.0f;
	float3 normalW = mul(normal, TBN);
	output.normal = float4(normalW, 1.0f);
	
	output.depth = float4(input.positionW, 1.0f);

	output.shadow = input.ShadowPosH;
	
	return output;
}