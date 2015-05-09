#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE;
};

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float4 projectedCoordinate : SV_POSITION;
};

// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register( b0 )
{
	float4x4 WORLDMATRIX;
	float4x4 VIEWMATRIX;
	float4x4 PROJECTIONMATRIX;
};

OUTPUT_VERTEX main( INPUT_VERTEX fromVertexBuffer )
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	sendToRasterizer.projectedCoordinate.w = 1;

	float4 vertex = fromVertexBuffer.coordinate;

	vertex = mul(vertex, WORLDMATRIX);
	vertex = mul(vertex, VIEWMATRIX);
	vertex = mul(vertex, PROJECTIONMATRIX);
	
	sendToRasterizer.projectedCoordinate = vertex;
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	sendToRasterizer.textureCoords = fromVertexBuffer.uv;

	return sendToRasterizer;
}