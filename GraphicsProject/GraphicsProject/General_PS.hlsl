texture2D baseTexture : register (t0);
SamplerState filter : register(s0);

struct OUTPUT_VERTEX
{
	float4 textureCoords : TEXTURE;
	float4 colorOut : COLOR;
	float4 normals : NORMALS;
	float4 projectedCoordinate : SV_POSITION;
};

// HLSLI FILE IS LIKE HEADER FILE
float4 main( OUTPUT_VERTEX input ) : SV_TARGET
{
	float4 rcolor;
	float2 uv = input.textureCoords.xy;
	float4 color = baseTexture.Sample(filter, uv);

	rcolor.a = color.b;
	rcolor.r = color.g;
	rcolor.g = color.r;
	rcolor.b = color.a;

	return rcolor;
}