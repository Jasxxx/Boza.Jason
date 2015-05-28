#pragma pack_matrix(row_major)

struct DS_OUTPUT
{
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float3 normalOut : NORMAL;
	float4 projectedCoordinate : SV_POSITION;
	float3 position : POSITION;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float2 textureCoords : TEXTURE;
	float3 normalOut : NORMAL;
	float3 position : POSITION;
};

cbuffer	constantBuffer : register (b0)
{
	float4x4 world;
	float4x4 view;
	float4x4 proj;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	Output.position = float3(
		patch[0].position*domain.x + patch[1].position*domain.y + patch[2].position*domain.z);
	Output.projectedCoordinate = float4(
		patch[0].position*domain.x + patch[1].position*domain.y + patch[2].position*domain.z, 1);
	Output.projectedCoordinate = mul(Output.projectedCoordinate, world);
	Output.projectedCoordinate = mul(Output.projectedCoordinate, view);
	Output.projectedCoordinate = mul(Output.projectedCoordinate, proj);
	Output.normalOut = float3(
		patch[0].normalOut*domain.x + patch[1].normalOut*domain.y + patch[2].normalOut*domain.z);
	Output.textureCoords = float2(patch[0].textureCoords*domain.x + patch[1].textureCoords*domain.y + patch[2].textureCoords*domain.z);
	Output.colorOut = float4(
		1,1,1, 1);

	return Output;
}
