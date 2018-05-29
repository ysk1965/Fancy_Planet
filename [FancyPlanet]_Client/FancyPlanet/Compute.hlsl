
Texture2D gInput            : register(t0);
RWTexture2D<float4> gOutput : register(u0);

#define N 256

[numthreads(N, 1, 1)]
void CS(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID)
{
	float sum = gInput[dispatchThreadID.xy].r + gInput[dispatchThreadID.xy].g + gInput[dispatchThreadID.xy].b;
	sum /= 3.0f;

	gOutput[dispatchThreadID.xy] = sum + 0.1;
}