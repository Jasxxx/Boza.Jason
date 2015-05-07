texture2D baseTexture : register (t0);
//SamplerState filter : register(s0);

struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float2 normals : NORMALS;
};

// HLSLI FILE IS LIKE HEADER FILE
float4 main( OUTPUT_VERTEX input ) : SV_TARGET
{
	//float4 rcolor;
	//float2 uv = input.textureCoords;
	float4 color = float4(0,0,0,1);

	/*rcolor.a = color.b;
	rcolor.r = color.g;
	rcolor.g = color.r;
	rcolor.b = color.a;*/

	return color;
}