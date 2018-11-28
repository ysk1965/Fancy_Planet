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
	float4			f4Color;
	float			fAlpha;
};

SamplerState gWrapSamplerState : register(s0);
SamplerComparisonState gShadowSamplerState : register(s1);

#include "Light.hlsl"

Texture2D gtxtObject_Diffuse : register(t4);
Texture2D gtxtObject_Normal : register(t5);
Texture2D ShadowMap : register(t10);

StructuredBuffer<INSTANCEDGAMEOBJECTINFO> gObjectInfos : register(t0);

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

struct VS_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};
struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normalW : NORMAL;
	float3 positionW : POSITION;
	float4 ShadowPosH : SHADOW;
	float4 color : COLOR;
	float alpha : ALPAH;
};

VS_OUTPUT Object_VS(VS_INPUT input, uint nInstanceID : SV_InstanceID)
{
	VS_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gObjectInfos[nInstanceID].gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = normalize(mul(input.normal, (float3x3)gObjectInfos[nInstanceID].gmtxWorld));
	output.uv = input.uv;
	output.ShadowPosH = mul(output.positionW, gmtxShadowTransform);
	output.color = gObjectInfos[nInstanceID].f4Color;
	output.alpha = gObjectInfos[nInstanceID].fAlpha;

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

float4 Object_PS(VS_OUTPUT input) : SV_TARGET
{
	float2 uv1;
	uv1.x = input.uv.x;
	uv1.y = 1.0f - input.uv.y;
	
	float4 diffuse = gtxtObject_Diffuse.Sample(gWrapSamplerState, uv1);
	
	diffuse *= input.color;

	float3 N = normalize(input.normalW);
	
	float3 normal = gtxtObject_Normal.Sample(gWrapSamplerState, uv1).rgb;
	
	float shadowFactor = CalcShadowFactor(input.ShadowPosH);
	float4 cllumination = Lighting(input.positionW, N, diffuse, 0, 30, shadowFactor);
	
	float4 result;
	result.rgb = lerp(diffuse.rgb, cllumination.rgb, 0.35f);
	result.a = input.alpha;

	return diffuse;
}

float4 Object_PS2(VS_OUTPUT input) : SV_TARGET
{
	float4 r = input.color;
	r.a = input.alpha;

	return r;
}