texture2D baseTexture : register (t0);
SamplerState filter : register(s0);

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float4 projectedCoordinate : SV_POSITION;
};

float4 main(OUTPUT_VERTEX output) : SV_TARGET
{
	float4 color = baseTexture.Sample(filter, output.textureCoords.xy);
	return color;
}