SamplerState sm:register(s0);
Texture2D s:register(t0);
cbuffer Params:register(b0)
{
	float4 d;
}

float4 main(float4 Position : SV_POSITION, float2 t : TEXCOORD0) : SV_TARGET0
{
	const float Pi = 3.14159265;
	t += float2(cos(d.x * Pi * 2 * t.y) * d.y, cos(d.z * Pi * 2 * t.x) * d.w);

	return s.Sample(sm, t);
}
