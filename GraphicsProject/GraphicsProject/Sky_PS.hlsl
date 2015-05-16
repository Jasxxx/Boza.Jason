// Specular Light Reference http://www.rastertek.com/dx11tut10.html

TextureCube baseTexture : register (t0);
SamplerState filter : register(s0);

struct OUTPUT_VERTEX
{
	float4 Pos : SV_POSITION;
	float3 texCoord : TEXCOORD;
};

float4 main(OUTPUT_VERTEX input) : SV_TARGET
{
	return baseTexture.Sample(filter, input.texCoord);
}