RWStructuredBuffer<float2> results : register(u0);

struct GSData
{
	float changeValue;
	float yValue;
};

cbuffer wave : register(b0)
{
	float average;
	float time;
	float2 padding;
};

groupshared GSData data[1024 * 1 * 1];

[numthreads(32, 32, 1)]
void main(uint3 index : SV_DispatchThreadID, uint gIndex : SV_GroupIndex)
{
	uint temp = index.x * index.y;

	data[gIndex].changeValue = results[temp].x;
	data[gIndex].yValue = results[temp].y;
	//if (verts[i].Position.y >= fAverage + 0.01f || verts[i].Position.y <= fAverage - 0.01f)

	//float temptimer = time * data[gIndex].changeValue;

	data[gIndex].yValue += data[gIndex].changeValue;
	

	if (data[gIndex].yValue > /*average*/ + 0.1f || data[gIndex].yValue <= /*average*/ - 0.1f)
	{
		results[temp].x = -results[temp].x;
	}

	GroupMemoryBarrierWithGroupSync();

	results[temp].y = data[gIndex].yValue;


}