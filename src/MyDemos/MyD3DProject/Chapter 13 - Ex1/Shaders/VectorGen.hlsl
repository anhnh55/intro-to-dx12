struct Data
{
	float3 vec;
};

StructuredBuffer<Data> gInput : register(t0);
RWStructuredBuffer<float> gOutput : register(u0);

#define N 64

[numthreads(N, 1, 1)]
void VectorGenCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
	float temp = length(gInput[dispatchThreadID.x].vec);
	gOutput[dispatchThreadID.x] = temp;
//	if (temp <= 10.0f)
//	{
//		gOutput[dispatchThreadID.x] = temp;
//		return;
//	}

//	gOutput[dispatchThreadID.x] = -1.0f;
	
}