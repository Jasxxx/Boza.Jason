struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 color : COLOR;
	float2 uv : TEXTURE;
	float2 nrm : NORMALS;
};

struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float2 normals : NORMALS;
};

// TODO: PART 3 STEP 2a
cbuffer THIS_IS_VRAM : register( b0 )
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};



OUTPUT_VERTEX main( INPUT_VERTEX fromVertexBuffer )
{
	OUTPUT_VERTEX sendToRasterizer = (OUTPUT_VERTEX)0;
	float4 vertex =  fromVertexBuffer.coordinate;

	vertex = mul(vertex, worldMatrix);
	vertex = mul(vertex, viewMatrix);
	vertex = mul(vertex, projectionMatrix);

	sendToRasterizer.projectedCoordinate = vertex;

	//sendToRasterizer.colorOut = float4(0,0,0,1);

	return sendToRasterizer;
}