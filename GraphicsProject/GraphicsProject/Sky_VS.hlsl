#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE;
	float3 normal : NORMAL;
};

struct OUTPUT_VERTEX
{
	float4 Pos : SV_POSITION;
	float3 textureCoords : TEXCOORD;
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
	sendToRasterizer.Pos.w = 1;

	float4 vertex = float4(fromVertexBuffer.coordinate.xyz,1);

	vertex = mul(vertex, WORLDMATRIX);
	vertex = mul(vertex, VIEWMATRIX);
	vertex = mul(vertex, PROJECTIONMATRIX);
	
	sendToRasterizer.Pos = vertex.xyww;
	sendToRasterizer.textureCoords = fromVertexBuffer.coordinate.xyz;
	return sendToRasterizer;
}