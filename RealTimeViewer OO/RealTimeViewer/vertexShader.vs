cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};


struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};


struct PixelInputType
{
	float4 position : POSITION0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 OriginalPos : POSITION1;

};


//Thusly the vertex shader arriveth:
PixelInputType vertexShader(VertexInputType input)
{
	PixelInputType output;

	//input.position.w = 1.0f;

	output.position = mul(input.position, worldMatrix);
	
	//Save pos in WS
	output.OriginalPos = output.position;
	
	//Continue transforming the output.position with the view and projection matrices
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);


	//send forth textures and normals as well, the normal transformed into world space and normalized
	output.tex = input.tex;
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	
	return output;

}