struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
};

float4 main(OUTPUT_VERTEX output) : SV_TARGET
{
	return output.colorOut;
}