
Texture2D gInput							 : register(t0);
Texture2D<uint2> EmissiveMap      : register(t1);
Texture2D gOriginSceneTex           : register(t2);

RWTexture2D<float4> gOutput : register(u0);
cbuffer cbSettings : register(b0)
{
	float gRadius;
	// Support up to 11 blur weights.
	float w0;
	float w1;
	float w2;
	float w3;
	float w4;
	float w5;
	float w6;
	float w7;
	float w8;
	float w9;
	float w10;
};
#define N 256

[numthreads(N, 1, 1)]
void Mul_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float4 fResult;

	gOutput[dispatchThreadID.xy] = EmissiveMap[dispatchThreadID.xy].g * gOriginSceneTex[dispatchThreadID.xy];
}
[numthreads(N, 1, 1)]
void Add_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float4 fResult;

	gOutput[dispatchThreadID.xy] = gInput[dispatchThreadID.xy] * 2.0f + gOriginSceneTex[dispatchThreadID.xy];
}

static const int gMaxBlurRadius = 5;
static const int  gBlurRadius = 4;
#define N 256
#define CacheSize (N + 2*gMaxBlurRadius)
groupshared float4 gCache[CacheSize];

[numthreads(N, 1, 1)]
void Horizontal_CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float weights[11] = { 0.0221905485, 0.0455890037, 0.0798114091, 0.119064637, 0.151360810, 0.163967222, 0.151360810, 0.119064637,
		0.0798114091, 0.0455890037, 0.0221905485 };

	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if (groupThreadID.x < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = max(dispatchThreadID.x - gBlurRadius, 0);
		gCache[groupThreadID.x] = gInput[int2(x, dispatchThreadID.y)];
	}
	if (groupThreadID.x >= N - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int x = min(dispatchThreadID.x + gBlurRadius, gInput.Length.x - 1);
		gCache[groupThreadID.x + 2 * gBlurRadius] = gInput[int2(x, dispatchThreadID.y)];
	}

	// Clamp out of bound samples that occur at image borders.
	gCache[groupThreadID.x + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];

	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0, 0, 0, 0);

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.x + gBlurRadius + i;

		blurColor += weights[i + gBlurRadius] * gCache[k];
	}

	gOutput[dispatchThreadID.xy] = blurColor;
}

[numthreads(1, N, 1)]
void Vertical_CS(int3 groupThreadID : SV_GroupThreadID,
	int3 dispatchThreadID : SV_DispatchThreadID)
{
	// Put in an array for each indexing.
	float weights[11] = { 0.0221905485, 0.0455890037, 0.0798114091, 0.119064637, 0.151360810, 0.163967222, 0.151360810, 0.119064637,
		0.0798114091, 0.0455890037, 0.0221905485 };

	//
	// Fill local thread storage to reduce bandwidth.  To blur 
	// N pixels, we will need to load N + 2*BlurRadius pixels
	// due to the blur radius.
	//

	// This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
	// have 2*BlurRadius threads sample an extra pixel.
	if (groupThreadID.y < gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = max(dispatchThreadID.y - gBlurRadius, 0);
		gCache[groupThreadID.y] = gInput[int2(dispatchThreadID.x, y)];
	}
	if (groupThreadID.y >= N - gBlurRadius)
	{
		// Clamp out of bound samples that occur at image borders.
		int y = min(dispatchThreadID.y + gBlurRadius, gInput.Length.y - 1);
		gCache[groupThreadID.y + 2 * gBlurRadius] = gInput[int2(dispatchThreadID.x, y)];
	}

	// Clamp out of bound samples that occur at image borders.
	gCache[groupThreadID.y + gBlurRadius] = gInput[min(dispatchThreadID.xy, gInput.Length.xy - 1)];


	// Wait for all threads to finish.
	GroupMemoryBarrierWithGroupSync();

	//
	// Now blur each pixel.
	//

	float4 blurColor = float4(0, 0, 0, 0);

	for (int i = -gBlurRadius; i <= gBlurRadius; ++i)
	{
		int k = groupThreadID.y + gBlurRadius + i;

		blurColor += weights[i + gBlurRadius] * gCache[k];
	}

	gOutput[dispatchThreadID.xy] = blurColor;
}