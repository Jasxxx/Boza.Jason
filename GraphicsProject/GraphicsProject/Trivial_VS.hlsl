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
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float3 normalOut : NORMAL;
	float4 projectedCoordinate : SV_POSITION;
	float3 ViewDir : VIEW;
};

// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register( b0 )
{
	float4x4 WORLDMATRIX;
	float4x4 VIEWMATRIX;
	float4x4 PROJECTIONMATRIX;
	float3 CamPos;
	float padding;
};

OUTPUT_VERTEX main( INPUT_VERTEX fromVertexBuffer )
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	sendToRasterizer.projectedCoordinate.w = 1;

	float4 vertex = fromVertexBuffer.coordinate;

	vertex = mul(vertex, WORLDMATRIX);
	sendToRasterizer.ViewDir = CamPos - vertex.xyz;
	vertex = mul(vertex, VIEWMATRIX);
	vertex = mul(vertex, PROJECTIONMATRIX);
	
	sendToRasterizer.projectedCoordinate = vertex;
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	sendToRasterizer.textureCoords = fromVertexBuffer.uv;

	sendToRasterizer.normalOut = mul(fromVertexBuffer.normal, (float3x3)WORLDMATRIX);
	sendToRasterizer.ViewDir = normalize(sendToRasterizer.ViewDir);

	return sendToRasterizer;
}