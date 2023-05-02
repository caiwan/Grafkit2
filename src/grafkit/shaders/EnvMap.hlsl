cbuffer Params:register(b0)
{
	float4 Radius;
}

float4 main(float4 Position : SV_POSITION, float2 t:TEXCOORD0) : SV_TARGET0
{
	float d = Radius.x - Radius.y;
	float2 scle = (t - 0.5) / (Radius.zw * Radius.x);
	float l = length(scle);
	return 1 + (Radius.y - l) / d;
}
