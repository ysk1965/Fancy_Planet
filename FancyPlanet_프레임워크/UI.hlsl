SamplerState gWrapSamplerState : register(s0);
Texture2D<float4> gtxUI : register(t14);
static float fSize = -0.5f;

struct VS_UI_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
cbuffer UI_INFO : register(b0)
{
	float2 uv1;
	float2 uv2;
};

VS_UI_OUTPUT UIVS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	//사각형을 그린다.
	if (nVertexID == 0)
	{
		output.position = float4(-1.0f, fSize, 0.0f, 1.0f);
		output.uv = float2(uv1.x, uv1.y);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(fSize, fSize, 0.0f, 1.0f);
		output.uv = float2(uv2.x, uv1.y);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(fSize, -1.0f, 0.0f, 1.0f);
		output.uv = float2(uv2.x, uv2.y);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-1.0f, fSize, 0.0f, 1.0f);
		output.uv = float2(uv1.x, uv1.y);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(fSize, -1.0f, 0.0f, 1.0f);
		output.uv = float2(uv2.x, uv2.y);
	}
	else
	{
		output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(uv1.x, uv2.y);
	}


	return output;
}

float4 UIPS(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 color = gtxUI.Sample(gWrapSamplerState, input.uv);

	return color;
}