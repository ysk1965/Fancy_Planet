
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

Texture2D gtxtTerrainBaseTexture : register(t4);
Texture2D gtxtTerrainDetailTexture : register(t5);
Texture2D gtxtTerrainNormalMap : register(t6);
Texture2D gtxtTexture_Stone : register(t7);
Texture2D gtxtTexture_StoneNormal : register(t8);
Texture2D gtxtModel_Diffuse_body : register(t10);
Texture2D gtxtModel_Normal_body : register(t11);
Texture2D gtxtModel_Specular_body : register(t12);
SamplerState gWrapSamplerState : register(s0);
SamplerState gClampSamplerState : register(s1);
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
	float3 positionW : POSITION;
	float3 tangentW : TANGENT;
	float3 bitangentW : BITANGENT;
};

struct PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT
{
	float4 diffuse : SV_TARGET0;
	float4 normal : SV_TARGET1;
	float4 depth : SV_TARGET2;
	float4 specular : SV_TARGET3;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
	VS_TERRAIN_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = normalize(mul(input.normal, (float3x3)gmtxWorld));
	output.tangentW = normalize(mul(input.tangent, (float3x3)gmtxWorld));
	output.bitangentW = normalize(cross(output.normalW, output.tangentW));
	output.uv1 = input.uv1;
	output.uv0 = input.uv0;

	return(output);
}

[earlydepthstencil]
PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET
{
	PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT output;

	float4 cBaseTexColor = gtxtTerrainBaseTexture.Sample(gWrapSamplerState, input.uv0);

	float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gWrapSamplerState, input.uv1);

	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	output.diffuse = cColor;

	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	// 노말맵으로 부터 법선벡터를 가져온다.
	float3 normal = gtxtTerrainNormalMap.Sample(gWrapSamplerState, input.uv0).rgb;
	// -1 와 1사이 값으로 변환한다.
	normal = 2.0f * normal - 1.0f;
	float3 normalW = mul(normal, TBN);
	output.normal = float4(normalW, 1.0f);

	output.depth = float4(input.positionW, 1.0f);

	output.specular = float4((float)gnMaterial, (float)gnMaterial, (float)gnMaterial, 1.0f / 255.0f);

	return(output);
}
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
};

VS_OUTPUT Test_VS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = normalize(mul(input.normal, (float3x3)gmtxWorld));
	output.tangentW = normalize(mul(input.tangent, (float3x3)gmtxWorld));
	output.bitangentW = normalize(cross(output.normalW, output.tangentW));
	output.uv = input.uv;

	return(output);
}

[earlydepthstencil]
PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT Test_PS (VS_OUTPUT input) : SV_TARGET
{
	PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT output;

	float3 diffuse = gtxtTexture_Stone.Sample(gWrapSamplerState, input.uv).rgb;

	output.diffuse = float4(diffuse,1.0f);

	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	// 노말맵으로 부터 법선벡터를 가져온다.
	float3 normal = gtxtTexture_StoneNormal.Sample(gWrapSamplerState, input.uv).rgb;
	// -1 와 1사이 값으로 변환한다.
	normal = 2.0f * normal - 1.0f;
	float3 normalW = mul(normal, TBN);
	output.normal = float4(normalW, 1.0f);

	output.depth = float4(input.positionW, 1.0f);

	output.specular = float4((float)gnMaterial, (float)gnMaterial, (float)gnMaterial, 64.0f / 255.0f);

	return output;
}


VS_OUTPUT VS_TDL(VS_INPUT input)
{
	VS_OUTPUT output;

	output.positionW = (float3)mul(float4(input.position, 1.0f), gmtxWorld);
	output.position = mul(mul(float4(output.positionW, 1.0f), gmtxView), gmtxProjection);
	output.normalW = normalize(mul(input.normal, (float3x3)gmtxWorld));
	output.tangentW = normalize(mul(input.tangent, (float3x3)gmtxWorld));
	output.bitangentW = normalize(cross(output.normalW, output.tangentW));
	output.uv = input.uv;

	return(output);
}
[earlydepthstencil]
PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT PS_TDL(VS_OUTPUT input) : SV_TARGET
{
	PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT output;
	float2 uv1, uv2;

	uv1.x = input.uv.x;
	uv1.y = 1.0f - input.uv.y;

	uv2.x = input.uv.x;
	uv2.x = input.uv.y;

	float3 diffuse = gtxtModel_Diffuse_body.Sample(gWrapSamplerState, uv1).rgb;

	output.diffuse = float4(diffuse, 1.0f);

	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	// 노말맵으로 부터 법선벡터를 가져온다.
	float3 normal = gtxtModel_Normal_body.Sample(gWrapSamplerState, uv1).rgb;
	// -1 와 1사이 값으로 변환한다.
	normal = 2.0f * normal - 1.0f;
	float3 normalW = mul(normal, TBN);
	output.normal = float4(normalW, 1.0f);

	output.depth = float4(input.positionW, 1.0f);

	output.specular = float4((float)gnMaterial, (float)gnMaterial, (float)gnMaterial, 15.0f / 255.0f);

	return output;
}

Texture2D<float4> gtxtDiffuse : register(t0);
Texture2D<float4> gtxtNormal : register(t1);
Texture2D<float4> gtxtDepth : register(t2);
Texture2D<float4> gtxtSpecular : register(t3);


float4 VSTextureToFullScreen(uint nVertexID : SV_VertexID) : SV_POSITION
{
	//사각형을 그린다.
	if (nVertexID == 0) 
		return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 1) 
		return(float4(+1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 2) 
		return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 3) 
		return(float4(-1.0f, +1.0f, 0.0f, 1.0f));
	if (nVertexID == 4) 
		return(float4(+1.0f, -1.0f, 0.0f, 1.0f));
	if (nVertexID == 5) 
		return(float4(-1.0f, -1.0f, 0.0f, 1.0f));
	
	return(float4(0, 0, 0, 0));
}

float4 PSTextureToFullScreen(float4 position : SV_POSITION) : SV_Target
{
	int3 uvm = int3(position.xy, 0);

	float3 normal = gtxtNormal.Load(uvm).xyz;
	float3 pos = gtxtDepth.Load(uvm).xyz;
	float3 diffuse = gtxtDiffuse.Load(uvm).xyz;
	float4 specular = gtxtSpecular.Load(uvm);

	float4 cllumination = Lighting(pos, normal, diffuse, specular.x, specular.a*255.0f);

	return (cllumination);
}

struct VS_TEXTURED_INPUT
{
	float3 position : POSITION;
	float2 uv : TEXCOORD;
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
[earlydepthstencil]
PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT PSTextured(VS_TEXTURED_OUTPUT input) : SV_Target
{
	PS_TEXTURED_DEFFERREDLIGHTING_OUTPUT output;
	float2 uv1;
	uv1.x = input.uv.x;
	uv1.y = 1.0f - input.uv.y;

	float3 diffuse = gtxtModel_Diffuse_body.Sample(gWrapSamplerState, uv1).rgb;

	output.depth = float4(input.positionW, 1.0f);
	output.diffuse = float4(diffuse, 1.0f);
	output.normal = float4(0.0f, 0.0f, 0.0f, 0.0f);
	output.specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	return output;
}

Texture2D<float4> gtxtSkyBox : register(t9);

float4 PSSkyBox(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float4 cColor = gtxtSkyBox.Sample(gClampSamplerState, input.uv);

	return(cColor);
}

