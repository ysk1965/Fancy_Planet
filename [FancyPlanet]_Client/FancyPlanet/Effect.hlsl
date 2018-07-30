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

struct INSTANCED_EFFECT_INFO
{
	matrix		gmtxWorld;
	float4 		m_xmf4Color;
	float4			m_xmf4Emissive;
	float			m_fAlpha;
};

SamplerState gWrapSamplerState : register(s0);

#include "Light.hlsl"

Texture2D gtxtEffect1 : register(t0);
Texture2D gtxtEffect2 : register(t1);
StructuredBuffer<INSTANCED_EFFECT_INFO> gEffectInfos : register(t2);

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float alpha : ALPHA;
	float4 emissive : EMISSIVE;
};

VS_OUTPUT VS_EFFECT(VS_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gEffectInfos[nInstanceID].gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = normalize(mul(input.normal, (float3x3)gEffectInfos[nInstanceID].gmtxWorld));
	output.uv = input.uv;
	output.color = gEffectInfos[nInstanceID].m_xmf4Color;
	output.alpha = gEffectInfos[nInstanceID].m_fAlpha;
	output.emissive = gEffectInfos[nInstanceID].m_xmf4Emissive;
	return output;
}

float4 PS_EFFECT(VS_OUTPUT input) : SV_TARGET
{
	float3 diffuse = gtxtEffect1.Sample(gWrapSamplerState, input.uv).rgb;
	float4 result = float4(diffuse, diffuse.r);

	clip(diffuse.r - 0.3f);
	result += input.emissive;
	result *= input.color;
	result.a *= input.alpha;
		
	return result;
}

struct VS_MESH_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
};
struct VS_MESH_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float alpha : ALPHA;
	float4 emissive : EMISSIVE;
};
VS_MESH_OUTPUT VS_MESH_EFFECT(VS_MESH_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_MESH_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gEffectInfos[nInstanceID].gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.uv = input.uv;
	output.color = gEffectInfos[nInstanceID].m_xmf4Color;
	output.alpha = gEffectInfos[nInstanceID].m_fAlpha;
	output.emissive = gEffectInfos[nInstanceID].m_xmf4Emissive;
	return output;
}
float4 PS_MESH_EFFECT(VS_MESH_OUTPUT input) : SV_TARGET
{
	float4 result = float4(input.color.xyz, 1.0f);
	result *= input.color;
	result.a *= input.alpha;

	return result;
}