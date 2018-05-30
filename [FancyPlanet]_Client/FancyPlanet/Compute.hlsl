
Texture2D gInput            : register(t0);
RWTexture2D<float4> gOutput : register(u0);
cbuffer cbSettings : register(b0)
{
	float value;
};
#define N 256

[numthreads(N, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float4 fResult;

	fResult = gInput[int2(dispatchThreadID.x, dispatchThreadID.y)] + gInput[int2(dispatchThreadID.x - (int)value, dispatchThreadID.y - (int)value)] +
		gInput[int2(dispatchThreadID.x, dispatchThreadID.y - (int)value)] + gInput[int2(dispatchThreadID.x + (int)value, dispatchThreadID.y - (int)value)] +
			gInput[int2(dispatchThreadID.x + (int)value, dispatchThreadID.y)] + gInput[int2(dispatchThreadID.x + (int)value, dispatchThreadID.y + (int)value)]+
				gInput[int2(dispatchThreadID.x, dispatchThreadID.y + (int)value)] + gInput[int2(dispatchThreadID.x - (int)value, dispatchThreadID.y + (int)value)]+
					gInput[int2(dispatchThreadID.x - (int)value, dispatchThreadID.y)];

	fResult /= 9.0f;

	gOutput[dispatchThreadID.xy] = fResult;
}