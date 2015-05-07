#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 color : COLOR;
	float4 uv : TEXTURE;
	float4 nrm : NORMALS;
};

struct OUTPUT_VERTEX
{
	float4 textureCoords : TEXTURE;
	float4 colorOut : COLOR;
	float4 normals : NORMALS;
	float4 projectedCoordinate : SV_POSITION;
};

// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register( b0 )
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};



OUTPUT_VERTEX main( INPUT_VERTEX fromVertexBuffer )
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	sendToRasterizer.projectedCoordinate.w = 1;
	float4 vertex =  fromVertexBuffer.coordinate;

	vertex = mul(vertex, worldMatrix);
	vertex = mul(vertex, viewMatrix);
	vertex = mul(vertex, projectionMatrix);

	//vertex.x /= vertex.w;
	//vertex.y /= vertex.w;
	//vertex.z /= vertex.w;
	//vec.w /= vec.w;
	//vertex.u /= vec.w;
	//vertex.v /= vec.w;

	//vertex.x = vec.x;
	//vertex.y = vec.y;
	//vertex.z = vec.z;
	//vertex.w = vec.w;
	
	sendToRasterizer.projectedCoordinate = vertex;

	// TODO : PART 3 STEP 7
	sendToRasterizer.colorOut = fromVertexBuffer.color;
	// END PART 3

	return sendToRasterizer;
}