struct VOut
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
};

#define PI 3.1415926535897932384626433832795f
#define MAX_LIGHT 8

SamplerState LinearSampler : register(s0);

Texture2D AlbedoTexture : register(t0);
Texture2D MetalnessTexture : register(t1);
Texture2D RoughnessTexture : register(t2);
Texture2D NormalTexture : register(t3);

struct DirectionalLight
{
	float4 Colour;
	float3 Direction;
	float LightIntensity;
};

cbuffer Camera : register(b0)
{
	float4 CameraPosition;
};

cbuffer LightConstants : register(b1)
{
	DirectionalLight Light[MAX_LIGHT];
	int LightCount;
}

cbuffer MaterialSettings : register(b2)
{
	int DiffuseOption;
}

struct Attributes
{
	float3 Position;
	float3 Normal;
	float2 TexCoord;
	float3 Tangent;
	float3 Bitangent;
};

struct Material
{
	float4 Albedo;
	float3 Normal;
	float Metalness;
	float Roughness;
};

float FresnelSchlick(float F0, float Fd90, float View)
{
	return F0 + (Fd90 - F0) * pow(max(1.0f - View, 0.1f), 5.0f);
}

float Disney(Attributes Attribs, DirectionalLight Light, Material Mat, float3 Eye)
{
	float3 HalfVector = normalize(Light.Direction + Eye);

	float NdotL = saturate(dot(Attribs.Normal, Light.Direction));
	float LdotH = saturate(dot(Light.Direction, HalfVector));
	float NdotV = saturate(dot(Attribs.Normal, Eye));

	float EnergyBias = lerp(0.0f, 0.5f, Mat.Roughness);
	float EnergyFactor = lerp(1.0f, 1.0f / 1.51f, Mat.Roughness);
	float Fd90 = EnergyBias + 2.0f * (LdotH * LdotH) * Mat.Roughness;
	float F0 = 1.0f;

	float LightScatter = FresnelSchlick(F0, Fd90, NdotL);
	float ViewScatter = FresnelSchlick(F0, Fd90, NdotV);

	return LightScatter * ViewScatter * EnergyFactor;
}

float CookTorrance(Attributes Attribs, DirectionalLight Light, Material Mat, float3 Eye)
{
	float VdotN = dot(Eye, Attribs.Normal);
	float LdotN = dot(Light.Direction, Attribs.Normal);
	float CosThetaI = LdotN;
	float ThetaR = acos(VdotN);
	float ThetaI = acos(CosThetaI);
	float CosPhiDiff = dot(normalize(Eye - Attribs.Normal * VdotN), normalize(Light.Direction - Attribs.Normal * LdotN));
	float Alpha = max(ThetaI, ThetaR);
	float Beta = min(ThetaI, ThetaR);
	float Sigma2 = Mat.Roughness * Mat.Roughness;
	float A = 1.0f - 0.5f * Sigma2 / (Sigma2 + 0.33f);
	float B = 0.45f * Sigma2 / (Sigma2 + 0.09f);

	if (CosPhiDiff >= 0.0f)
	{
		B *= sin(Alpha) * tan(Beta);
	}
	else
	{
		B *= 0;
	}
	return CosThetaI * (A + B);
}


float3 GGX(Attributes Attribs, DirectionalLight Light, Material Mat, float3 Eye)
{
	float3 H = normalize(normalize(Light.Direction) + Eye);
	float NdotH = saturate(dot(Attribs.Normal, H));

	float Rough2 = max(Mat.Roughness * Mat.Roughness, 2.0e-3f);
	float Rough4 = Rough2 * Rough2;

	float d = (NdotH * Rough4 - NdotH) * NdotH + 1.0f;
	float D = Rough4 / (PI * (d * d));

	float3 Reflectivity = Mat.Metalness;
	float Fresnel = 1.0;
	float NdotL = saturate(dot(Attribs.Normal, normalize(Light.Direction)));
	float LdotH = saturate(dot(normalize(Light.Direction), H));
	float NdotV = saturate(dot(Attribs.Normal, Eye));
	float3 F = Reflectivity + (Fresnel - Fresnel * Reflectivity) * exp2((-5.55473f * LdotH - 6.98316f) * LdotH);

	float K = Rough2 * 0.5f;
	float G_SmithL = NdotL * (1.0f - K) + K;
	float G_SmithV = NdotV * (1.0f - K) + K;
	float G = 0.25f / (G_SmithL * G_SmithV);

	return G * D * F;
}

float Diffuse(Attributes Attribs, DirectionalLight Light, Material Mat, float3 Eye)
{
	if (DiffuseOption == 0)
	{
		return Disney(Attribs, Light, Mat, Eye);
	}
	else if (DiffuseOption == 1)
	{
		return CookTorrance(Attribs, Light, Mat, Eye);
	}
	//Fall back to Disney if nothing is available
	return Disney(Attribs, Light, Mat, Eye);
}

float3 TransformNormals(Attributes Attribs)
{
	float3x3 ToWorld = float3x3(Attribs.Bitangent, Attribs.Tangent, Attribs.Normal);
	float3 NormalMap = NormalTexture.Sample(LinearSampler, Attribs.TexCoord).rgb * 2.0 - 1.0;
	NormalMap = mul(NormalMap.rgb, ToWorld);
	NormalMap = normalize(NormalMap);
	return NormalMap;
}

float4 main(VOut Input) : SV_TARGET
{
	Attributes Attribs;
	Attribs.Position = Input.Position;
	Attribs.TexCoord = Input.TexCoord;

	Attribs.Bitangent = normalize(Input.Bitangent);
	Attribs.Tangent = normalize(Input.Tangent);
	Attribs.Normal = normalize(Input.Normal);

	Material Mat;
	Mat.Albedo = AlbedoTexture.Sample(LinearSampler, Input.TexCoord);
	Mat.Roughness = RoughnessTexture.Sample(LinearSampler, Input.TexCoord).r;
	Mat.Metalness = MetalnessTexture.Sample(LinearSampler, Input.TexCoord).r;
	Attribs.Normal = TransformNormals(Attribs);

	float4 DiffuseColour = float4(0, 0, 0, 0);
	float3 SpecularColour = float3(0, 0, 0);
	float3 Eye = normalize(CameraPosition.xyz - Attribs.Position.xyz);

	for (int i = 0; i < LightCount; i++)
	{
		float NdotL = saturate(dot(Attribs.Normal, normalize(Light[i].Direction)));
		DiffuseColour += NdotL * Diffuse(Attribs, Light[i], Mat, Eye) * Light[i].Colour * Light[i].LightIntensity;
		SpecularColour += NdotL * GGX(Attribs, Light[i], Mat, Eye) * Light[i].Colour * Light[i].LightIntensity;
	}
	float3 FinalColor = Mat.Albedo.rgb * DiffuseColour.rgb + SpecularColour;
	return float4(FinalColor, Mat.Albedo.w);
}
