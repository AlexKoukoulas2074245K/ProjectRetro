Texture2D resource;
SamplerState ss;

cbuffer ColorBuffer
{
	int colorEnable;
	float3 padding;
	float4 color;
};

struct PSInput
{
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float4 normal: NORMAL;
};

float4 main(PSInput input) : SV_TARGET
{
	float4 samp = resource.Sample(ss, input.texcoord);
	if(!colorEnable || samp.a < 0.9f) return samp;
	return color;
}
