SamplerState Sampler : register(s1);
Texture2D Texture:register(t0);
cbuffer Params:register(b0)
{
	float2 Repeat;
}

float4 main(float4 Position : SV_POSITION, float2 TexCoord : TEXCOORD0) : SV_TARGET0
{
	//return Texture.Sample(Sampler, TexCoord * Repeat * 255.0f);
	return Texture.Sample(Sampler, TexCoord * Repeat);
}
