SamplerState gWrapSamplerState : register(s0);
Texture2D<float4> gtxUI : register(t0);
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
	float4 xmf4TimeNumber[3];
	float4 xmf4HpNumber[3];
	float4 xmf4ScoreNumber[6];
	float2 xmf2SKill;
	float2 xmf2Point;
	float4 CoverColor;
	float4 xmf4Select;
	float	fGaze;
	float  fGravity;
};

VS_UI_OUTPUT MINIMAP_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	//사각형을 그린다.
	if (nVertexID == 0)
	{
		output.position = float4(-1.0f, fMiniMapSize + 0.1f, 0.0f, 1.0f);
		output.uv = float2(uv1.x, uv1.y);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(fMiniMapSize, fMiniMapSize + 0.1f, 0.0f, 1.0f);
		output.uv = float2(uv2.x, uv1.y);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(fMiniMapSize, -1.0f, 0.0f, 1.0f);
		output.uv = float2(uv2.x, uv2.y);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-1.0f, fMiniMapSize + 0.1f, 0.0f, 1.0f);
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

VS_UI_OUTPUT CROSS_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(-0.3f, 0.3f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.3f, 0.3f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(0.3f, -0.3f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-0.3f, 0.3f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(0.3f, -0.3f, 0.0f, 1.0f);
		output.uv = float2(1, 1);
	}
	else
	{
		output.position = float4(-0.3f, -0.3f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VS_UI_OUTPUT MINIUI_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0 || nVertexID == 3)
	{
		output.position = float4(-1.0f, -0.2f, 0.0f, 1.0f);
		output.uv = float2(0.0f, -0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.1f, -0.2f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2 || nVertexID == 4)
	{
		output.position = float4(0.1f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else
	{
		output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VS_UI_OUTPUT SCOREBOARD_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0 || nVertexID == 3)
	{
		output.position = float4(-0.5f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(0.0f, -0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.5f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2 || nVertexID == 4)
	{
		output.position = float4(0.5f, 0.75f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else
	{
		output.position = float4(-0.5f, 0.75f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VS_UI_OUTPUT GRAVITYBAR_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0 || nVertexID == 3)
	{
		output.position = float4(0.85f, 0.65f, 0.0f, 1.0f);
		output.uv = float2(0.0f, -0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.95f, 0.65f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2 || nVertexID == 4)
	{
		output.position = float4(0.95f, -0.55f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else
	{
		output.position = float4(0.85f, -0.55f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VS_UI_OUTPUT GRAVITYPOINTER_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;
	float2 pos[4];
	pos[0] = float2(0.705, 0.625 - 0.95f * xmf2Point.x);
	pos[1] = float2(0.96, 0.625 - 0.95f * xmf2Point.x);
	pos[2] = float2(0.96, 0.425 - 0.95f * xmf2Point.x);
	pos[3] = float2(0.705, 0.425 - 0.95f * xmf2Point.x);
	

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

VS_UI_OUTPUT TIME_NUMBER_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(0.0f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[0].x, xmf4TimeNumber[0].y);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.1f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[0].z, xmf4TimeNumber[0].y);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(0.1f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[0].z, xmf4TimeNumber[0].w);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(0.0f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[0].x, xmf4TimeNumber[0].y);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(0.1f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[0].z, xmf4TimeNumber[0].w);
	}
	else if (nVertexID == 5)
	{
		output.position = float4(0.0f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[0].x, xmf4TimeNumber[0].w);
	}///////////////////////////////////////////////////////////////////////////////////////////////////
	else if (nVertexID == 6)
	{
		output.position = float4(0.06f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[1].x, xmf4TimeNumber[1].y);
	}
	else if (nVertexID == 7)
	{
		output.position = float4(0.16f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[1].z, xmf4TimeNumber[1].y);
	}
	else if (nVertexID == 8)
	{
		output.position = float4(0.16f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[1].z, xmf4TimeNumber[1].w);
	}
	else if (nVertexID == 9)
	{
		output.position = float4(0.06f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[1].x, xmf4TimeNumber[1].y);
	}
	else if (nVertexID == 10)
	{
		output.position = float4(0.16f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[1].z, xmf4TimeNumber[1].w);
	}
	else if (nVertexID == 11)
	{
		output.position = float4(0.06f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[1].x, xmf4TimeNumber[1].w);
	}///////////////////////////////////////////////////////////////////////////////////////////////////
	else if (nVertexID == 12)
	{
		output.position = float4(0.13f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[2].x, xmf4TimeNumber[2].y);
	}
	else if (nVertexID == 13)
	{
		output.position = float4(0.23f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[2].z, xmf4TimeNumber[2].y);
	}
	else if (nVertexID == 14)
	{
		output.position = float4(0.23f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[2].z, xmf4TimeNumber[2].w);
	}
	else if (nVertexID == 15)
	{
		output.position = float4(0.13f, 0.84f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[2].x, xmf4TimeNumber[2].y);
	}
	else if (nVertexID == 16)
	{
		output.position = float4(0.23f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[2].z, xmf4TimeNumber[2].w);
	}
	else if (nVertexID == 17)
	{
		output.position = float4(0.13f, 0.74f, 0.0f, 1.0f);
		output.uv = float2(xmf4TimeNumber[2].x, xmf4TimeNumber[2].w);
	}

	return output;
}
VS_UI_OUTPUT SKILL1_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(0.55f, -0.7f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.7f, -0.7f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(0.7f, -0.9f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(0.55f, -0.7f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(0.7f, -0.9f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 5)
	{
		output.position = float4(0.55f, -0.9f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}
	return output;
}

VS_UI_OUTPUT SKILL2_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(0.75f, -0.7f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.9f, -0.7f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(0.9f, -0.9f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(0.75f, -0.7f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(0.9f, -0.9f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 5)
	{
		output.position = float4(0.75f, -0.9f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}
	return output;
}
VS_UI_OUTPUT SKILL1_COVER_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(0.56f, -0.89f + (0.177f) * xmf2SKill.x, 0.0f, 1.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.69f, -0.89f + (0.177f) * xmf2SKill.x, 0.0f, 1.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(0.69f, -0.89f, 0.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(0.56f, -0.89f + (0.177f) * xmf2SKill.x, 0.0f, 1.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(0.69f, -0.89f, 0.0f, 1.0f);
	}
	else if (nVertexID == 5)
	{
		output.position = float4(0.56f, -0.89f, 0.0f, 1.0f);
	}
	return output;
}

VS_UI_OUTPUT SKILL2_COVER_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(0.76f, -0.89f + (0.177f) * xmf2SKill.y, 0.0f, 1.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.89f, -0.89f + (0.177f) * xmf2SKill.y, 0.0f, 1.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(0.89f, -0.89f, 0.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(0.76f, -0.89f + (0.177f) * xmf2SKill.y, 0.0f, 1.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(0.89f, -0.89f, 0.0f, 1.0f);
	}
	else if (nVertexID == 5)
	{
		output.position = float4(0.76f, -0.89f, 0.0f, 1.0f);
	}
	return output;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VS_UI_OUTPUT HP_NUMBER_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(-0.495f, -0.795f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[0].x, xmf4HpNumber[0].y);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(-0.395f, -0.795f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[0].z, xmf4HpNumber[0].y);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(-0.395f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[0].z, xmf4HpNumber[0].w);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-0.495f, -0.8f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[0].x, xmf4HpNumber[0].y);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(-0.395f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[0].z, xmf4HpNumber[0].w);
	}
	else if (nVertexID == 5)
	{
		output.position = float4(-0.495f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[0].x, xmf4HpNumber[0].w);
	}
	else if (nVertexID == 6)
	{
		output.position = float4(-0.445f, -0.795f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[1].x, xmf4HpNumber[1].y);
	}
	else if (nVertexID == 7)
	{
		output.position = float4(-0.345f, -0.795f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[1].z, xmf4HpNumber[1].y);
	}
	else if (nVertexID == 8)
	{
		output.position = float4(-0.345f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[1].z, xmf4HpNumber[1].w);
	}
	else if (nVertexID == 9)
	{
		output.position = float4(-0.445f, -0.795f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[1].x, xmf4HpNumber[1].y);
	}
	else if (nVertexID == 10)
	{
		output.position = float4(-0.345f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[1].z, xmf4HpNumber[1].w);
	}
	else if (nVertexID == 11)
	{
		output.position = float4(-0.445f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[1].x, xmf4HpNumber[1].w);
	}
	else if (nVertexID == 12)
	{
		output.position = float4(-0.38f, -0.795f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[2].x, xmf4HpNumber[2].y);
	}
	else if (nVertexID == 13)
	{
		output.position = float4(-0.28f, -0.795f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[2].z, xmf4HpNumber[2].y);
	}
	else if (nVertexID == 14)
	{
		output.position = float4(-0.28f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[2].z, xmf4HpNumber[2].w);
	}
	else if (nVertexID == 15)
	{
		output.position = float4(-0.38f, -0.795f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[2].x, xmf4HpNumber[2].y);
	}
	else if (nVertexID == 16)
	{
		output.position = float4(-0.28f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[2].z, xmf4HpNumber[2].w);
	}
	else if (nVertexID == 17)
	{
		output.position = float4(-0.38f, -0.945f, 0.0f, 1.0f);
		output.uv = float2(xmf4HpNumber[2].x, xmf4HpNumber[2].w);
	}

	return output;
}

VS_UI_OUTPUT SCORE_NUMBER_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;
	//////////////////////////////////////////////////////////////////////////////////////
	if (nVertexID == 0)// 가로세로 0.125
	{
		output.position = float4(-0.25f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[0].x, xmf4ScoreNumber[0].y);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(-0.125f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[0].z, xmf4ScoreNumber[0].y);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(-0.125f, 0.875f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[0].z, xmf4ScoreNumber[0].w);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-0.25f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[0].x, xmf4ScoreNumber[0].y);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(-0.125f, 0.875f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[0].z, xmf4ScoreNumber[0].w);
	}
	else if (nVertexID == 5)
	{
		output.position = float4(-0.25f, 0.875f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[0].x, xmf4ScoreNumber[0].w);
	}//////////////////////////////////////////////////////////////////////////////////////
	else if (nVertexID == 6)
	{
		output.position = float4(-0.135f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[1].x, xmf4ScoreNumber[1].y);
	}
	else if (nVertexID == 7)
	{
		output.position = float4(-0.035f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[1].z, xmf4ScoreNumber[1].y);
	}
	else if (nVertexID == 8)
	{
		output.position = float4(-0.035f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[1].z, xmf4ScoreNumber[1].w);
	}
	else if (nVertexID == 9)
	{
		output.position = float4(-0.135f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[1].x, xmf4ScoreNumber[1].y);
	}
	else if (nVertexID == 10)
	{
		output.position = float4(-0.035f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[1].z, xmf4ScoreNumber[1].w);
	}
	else if (nVertexID == 11)
	{
		output.position = float4(-0.135f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[1].x, xmf4ScoreNumber[1].w);
	}//////////////////////////////////////////////////////////////////////////////////////
	else if (nVertexID == 12)
	{
		output.position = float4(-0.05f, 0.93f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[2].x, xmf4ScoreNumber[2].y);
	}
	else if (nVertexID == 13)
	{
		output.position = float4(0.075f, 0.93f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[2].z, xmf4ScoreNumber[2].y);
	}
	else if (nVertexID == 14)
	{
		output.position = float4(0.075f, 0.805f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[2].z, xmf4ScoreNumber[2].w);
	}
	else if (nVertexID == 15)
	{
		output.position = float4(-0.05f, 0.93f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[2].x, xmf4ScoreNumber[2].y);
	}
	else if (nVertexID == 16)
	{
		output.position = float4(0.075f, 0.805f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[2].z, xmf4ScoreNumber[2].w);
	}
	else if (nVertexID == 17)
	{
		output.position = float4(-0.05f, 0.805f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[2].x, xmf4ScoreNumber[2].w);
	}//////////////////////////////////////////////////////////////////////////////////////
	else if (nVertexID == 18)
	{
		output.position = float4(-0.035f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[3].x, xmf4ScoreNumber[3].y);
	}
	else if (nVertexID == 19)
	{
		output.position = float4(0.065f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[3].z, xmf4ScoreNumber[3].y);
	}
	else if (nVertexID == 20)
	{
		output.position = float4(0.065f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[3].z, xmf4ScoreNumber[3].w);
	}
	else if (nVertexID == 21)
	{
		output.position = float4(-0.035f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[3].x, xmf4ScoreNumber[3].y);
	}
	else if (nVertexID == 22)
	{
		output.position = float4(0.065f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[3].z, xmf4ScoreNumber[3].w);
	}
	else if (nVertexID == 23)
	{
		output.position = float4(-0.035f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[3].x, xmf4ScoreNumber[3].w);
	}//////////////////////////////////////////////////////////////////////////////////////
	else if (nVertexID == 24)
	{
		output.position = float4(0.155f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[4].x, xmf4ScoreNumber[4].y);
	}
	else if (nVertexID == 25)
	{
		output.position = float4(0.28f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[4].z, xmf4ScoreNumber[4].y);
	}
	else if (nVertexID == 26)
	{
		output.position = float4(0.28f, 0.875f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[4].z, xmf4ScoreNumber[4].w);
	}
	else if (nVertexID == 27)
	{
		output.position = float4(0.155f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[4].x, xmf4ScoreNumber[4].y);
	}
	else if (nVertexID == 28)
	{
		output.position = float4(0.28f, 0.875f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[4].z, xmf4ScoreNumber[4].w);
	}
	else if (nVertexID == 29)
	{
		output.position = float4(0.155f, 0.875f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[4].x, xmf4ScoreNumber[4].w);
	}//////////////////////////////////////////////////////////////////////////////////////
	else if (nVertexID == 30)
	{
		output.position = float4(0.06f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[5].x, xmf4ScoreNumber[5].y);
	}
	else if (nVertexID == 31)
	{
		output.position = float4(0.16f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[5].z, xmf4ScoreNumber[5].y);
	}
	else if (nVertexID == 32)
	{
		output.position = float4(0.16f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[5].z, xmf4ScoreNumber[5].w);
	}
	else if (nVertexID == 33)
	{
		output.position = float4(0.06f, 1.0f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[5].x, xmf4ScoreNumber[5].y);
	}
	else if (nVertexID == 34)
	{
		output.position = float4(0.16f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[5].z, xmf4ScoreNumber[5].w);
	}
	else
	{
		output.position = float4(0.06f, 0.9f, 0.0f, 1.0f);
		output.uv = float2(xmf4ScoreNumber[5].x, xmf4ScoreNumber[5].w);
	}

	return output;
}

VS_UI_OUTPUT LOBBY_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(-1.0f, 0.95f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(1.0f, 0.95f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(1.0f, -0.95f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-1.0f, 0.95f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(1.0f, -0.95f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else
	{
		output.position = float4(-1.0f, -0.95f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}
VS_UI_OUTPUT OCCUPATION_GAZE_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(-0.3f, 0.7f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.3f, 0.7f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(0.3f, 0.6f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-0.3f, 0.7f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(0.3f, 0.6f, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else
	{
		output.position = float4(-0.3f, 0.6f, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}
VS_UI_OUTPUT OCCUPATION_GAZE_COVER_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(-0.272f, 0.68f, 0.0f, 1.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(0.544f  * fGaze - 0.272f, 0.68f, 0.0f, 1.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(0.544f  * fGaze - 0.272f, 0.62f, 0.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(-0.272f, 0.68f, 0.0f, 1.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(0.544f  * fGaze - 0.272f, 0.62f, 0.0f, 1.0f);
	}
	else
	{
		output.position = float4(-0.272f, 0.62f, 0.0f, 1.0f);
	}

	return output;
}
VS_UI_OUTPUT SELECT_VS(uint nVertexID : SV_VertexID)
{
	VS_UI_OUTPUT output;

	if (nVertexID == 0)
	{
		output.position = float4(xmf4Select.x, xmf4Select.y, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 1)
	{
		output.position = float4(xmf4Select.z, xmf4Select.y, 0.0f, 1.0f);
		output.uv = float2(1.0f, 0.0f);
	}
	else if (nVertexID == 2)
	{
		output.position = float4(xmf4Select.z, xmf4Select.w, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else if (nVertexID == 3)
	{
		output.position = float4(xmf4Select.x, xmf4Select.y, 0.0f, 1.0f);
		output.uv = float2(0.0f, 0.0f);
	}
	else if (nVertexID == 4)
	{
		output.position = float4(xmf4Select.z, xmf4Select.w, 0.0f, 1.0f);
		output.uv = float2(1.0f, 1.0f);
	}
	else
	{
		output.position = float4(xmf4Select.x, xmf4Select.w, 0.0f, 1.0f);
		output.uv = float2(0.0f, 1.0f);
	}

	return output;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
float4 UIPS(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 color = gtxUI.Sample(gWrapSamplerState, input.uv);

	clip(color.a - 0.1f);

	return color;
}
float4 UI_COVER_PS(VS_UI_OUTPUT input) : SV_TARGET
{
	float4 color = CoverColor;

	return color;
}
float4 OCCUPATION_GAZE_COVER_PS(VS_UI_OUTPUT input) : SV_TARGET
{
	return float4(0.0f, 0.0f,0.0f,0.7f);
}