static const float PI = 3.14159265f;

cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	float4 CamLookAt;

};

struct vertexShader_OUTPUT
{
	float4 position : POSITION0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 OriginalPos : POSITION1;

};

struct geometryShader_OUTPUT
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
		
};


[maxvertexcount(3)]
void geometryShader(triangle vertexShader_OUTPUT input[3], inout TriangleStream<geometryShader_OUTPUT> OutputStream)
{
	geometryShader_OUTPUT output = (geometryShader_OUTPUT)0;

	//here we calculate backface culling and remove the faces out of view
	float3 faceNormal = {0, 0, 0};
	float test = 0;
	float3 LookVec;
	
	//Calculate normal of a triangleface:
	faceNormal = normalize(cross((input[1].OriginalPos - input[0].OriginalPos), (input[2].OriginalPos - input[0].OriginalPos)));	

	
	LookVec = (input[0].OriginalPos - CamLookAt);
	test = dot(normalize(LookVec), normalize(faceNormal));
	

	for(uint i = 0; i<3 ; i++)
	{
				
		
		//Pass data along
		output.position = input[i].position;
		output.tex = input[i].tex;
		output.normal = input[i].normal;
				
		//if the dot product within test is smaller than 0, the angle between the Face Normal and LookVector is greater than 90°.
		if(test < 0 )
		{
		OutputStream.Append(output);
		}
	}
	OutputStream.RestartStrip();
	
}
