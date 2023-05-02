struct STriangleOutput
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
};

STriangleOutput main(uint VertexID : SV_VertexID)
{
	STriangleOutput Output;
	Output.TexCoord = float2((VertexID << 1) & 2, VertexID & 2);
	Output.Position = float4(Output.TexCoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);

	return Output;
}
