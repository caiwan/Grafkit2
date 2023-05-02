cbuffer Params:register(b0)
{
	float4 Data;
	float Chamfer;
	float Falloff;
};

float4 main(float4 Position : SV_POSITION, float2 TexCoord : TEXCOORD0) : SV_TARGET0
{
	 float dc = (1 + Chamfer) / 2;
	 float2 c = Data.xy - TexCoord + 0.5 / 255.0f;
	 float2 d = abs(c / Data.zw / dc);
	 float l = 1 - saturate(sqrt(pow(d.x,2 / Chamfer) + pow(d.y,2 / Chamfer)));
	 return pow(abs(l + 0.5),1 / Falloff * 10);
}
