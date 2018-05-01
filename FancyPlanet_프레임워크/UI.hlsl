SamplerState gWrapSamplerState : register(s0);
Texture2D<float4> gtxUI : register(t14);
static float fMiniMapSize = -0.5f;
static float fArrowSize = 0.05f;

struct VS_UI_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};
cbuffer UI_INFO : register(b0)
{
	float2 uv1;
	float2 uv2;
	float2 xmf2Degree;
	float2 xmf2Arrow;
};

VS_UI_OUTPUT UIVS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	//사각형을 그린다.
	if (nVertexID == 0)
	{
		output.position = float4(-1.0f, fMiniMapSize, 0.0f, 1.0f);
		output.uv = float2(uv1.x, uv1.y);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(fMiniMapSize, fMiniMapSize, 0.0f, 1.0f);
		output.uv = float2(uv2.x, uv1.y);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(fMiniMapSize, -1.0f, 0.0f, 1.0f);
		output.uv = float2(uv2.x, uv2.y);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-1.0f, fMiniMapSize, 0.0f, 1.0f);
		output.uv = float2(uv1.x, uv1.y);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(fMiniMapSize, -1.0f, 0.0f, 1.0f);
		output.uv = float2(uv2.x, uv2.y);
	}
	else
	{
		output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(uv1.x, uv2.y);
	}
	
	return output;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


VS_UI_OUTPUT ARROW_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;
	float2x2 Rotation = float2x2(xmf2Degree.x, -xmf2Degree.y, xmf2Degree.y, xmf2Degree.x);
	float2 pos[4];
	pos[0] = float2(-fArrowSize, fArrowSize);
	pos[1] = float2(fArrowSize, fArrowSize);
	pos[2] = float2(fArrowSize, -fArrowSize);
	pos[3] = float2(-fArrowSize, -fArrowSize);

	for (int i = 0; i < 4; i++)
	{
		pos[i] = mul(pos[i], Rotation);
		pos[i].x += xmf2Arrow.x;
		pos[i].y += xmf2Arrow.y;
	}
	
	if (nVertexID == 0) 
	{
		output.position = float4(pos[0].x, pos[0].y, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(pos[1].x, pos[1].y, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(pos[2].x, pos[2].y, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(pos[0].x, pos[0].y, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(pos[2].x, pos[2].y, 0.0f, 1.0f);
		output.uv = float2(1, 1);
	}
	else
	{
		output.position = float4(pos[3].x, pos[3].y, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


float4 UIPS(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 color = gtxUI.Sample(gWrapSamplerState, input.uv);

	clip(color.a - 0.1f);

	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
	return color;
}