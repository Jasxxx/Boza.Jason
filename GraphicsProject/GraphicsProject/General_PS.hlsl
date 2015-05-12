struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float3 normalOut : NORMAL;
	float4 projectedCoordinate : SV_POSITION;
	float3 ViewDir : VIEW;
};

float4 main(OUTPUT_VERTEX output) : SV_TARGET
{
	return output.colorOut;
}