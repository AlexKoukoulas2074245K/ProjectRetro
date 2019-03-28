cbuffer MatrixBuffer
{
	float4x4 finalMatrix;
	float4x4 worldMatrix;
	float4x4 rotMatrix;
	float4 cameraPos;
};

struct VSInput
{
	float4 position: POSITION;
	float2 texcoord: TEXCOORD;
	float4 normal: NORMAL;
};

struct PSInput
{
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float4 normal: NORMAL; 
};

PSInput main(VSInput input)
{
	PSInput output;
	output.position = mul(finalMatrix, input.position);
	output.texcoord = input.texcoord;
	output.normal = mul(rotMatrix, input.normal);
	output.normal = normalize(output.normal);

	return output;
}
