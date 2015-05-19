#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE;
	float3 normal : NORMAL;
	float3 instancePos : INSTANCEPOS;
};

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float3 normalOut : NORMAL;
	float4 projectedCoordinate : SV_POSITION;
	float3 position : POSITION;
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

	float4 vertex = float4(fromVertexBuffer.coordinate.xyz, 1);

	vertex += float4(fromVertexBuffer.instancePos, 0.0f);
	vertex = mul(vertex, WORLDMATRIX);
	sendToRasterizer.position = vertex.xyz;
	vertex = mul(vertex, VIEWMATRIX);
	vertex = mul(vertex, PROJECTIONMATRIX);
	
	sendToRasterizer.projectedCoordinate = vertex;
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	sendToRasterizer.textureCoords = fromVertexBuffer.uv;

	sendToRasterizer.normalOut = mul(float4(fromVertexBuffer.normal.xyz, 0), WORLDMATRIX).xyz;

	return sendToRasterizer;
}