cbuffer PerObject : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
};

struct VOut
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
};

VOut main(float4 Position : POSITION, float3 Normal : NORMAL, float2 TexCoord : TEXCOORD, float3 Tangent : TANGENT, float3 Bitangent : BITANGENT)
{
	VOut Output;
	matrix WorldViewProj = mul(mul(World, View), Projection);
	Output.Position = mul(Position, WorldViewProj);
	Output.Normal = mul(Normal, (float3x3)World);
	Output.Tangent = mul(Tangent, (float3x3)World);
	Output.TexCoord = TexCoord;
	Output.Bitangent = Bitangent;
	return Output;
}
