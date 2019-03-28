Texture2D resource;
SamplerState ss;

struct PSInput
{
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float4 normal: NORMAL;
};

float4 main(PSInput input) : SV_TARGET
{
	return resource.Sample(ss, input.texcoord);
}