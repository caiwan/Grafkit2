SamplerState LinearSampler : register(s0); // linear sampler
Texture2D Input:register(t0); // output of blur x
Texture2D Mask:register(t1); // output of blur x

cbuffer Parameters : register(b0)
{
	float Smooth = 0.5f;			// 0.0 -> box filter, > 0.0 for gaussian
	float Size = 0.5f;				// length of the blur (global)
	float SamplesX = 0.5f;			// number of samples to take
	float SamplesY = 0.5f;			// number of samples to take
	float DirectionX = 0.5f;		// direction of blur
	float DirectionY = 0.5f;		// direction of blur
	float PowerX = 0.5f;			// length of the blur
	float PowerY = 0.5f;			// length of the blur
}

float WeightFunction(float X) {
	return pow(2.71828, -(X * X) * (Smooth * Smooth) * 64.0);
}

float4 main(float4 PositionSS : SV_Position, float2 TexCoord : TEXCOORD) : SV_TARGET0
{
	float SamplesCount = SamplesX * 255 + 1;
	float Increment = 1.0 / SamplesCount;

	float Amount = Size * Size * PowerX;
	float Pi = 355.0 / 113.0;
	float2 Direction = float2(cos(DirectionX * Pi), sin(DirectionX * Pi));

	float WeightAccumulator = WeightFunction(0);
	float4 Accumulator = Input.Sample(LinearSampler, TexCoord) * WeightAccumulator;

	for (float X = Increment; X < 1.0; X += Increment)
	{
		float Weight = WeightFunction(X);
		Accumulator += Input.Sample(LinearSampler, TexCoord + X * Amount * Direction) * Weight;
		WeightAccumulator += Weight;
		Weight = WeightFunction(-X);
		Accumulator += Input.Sample(LinearSampler, TexCoord - X * Amount * Direction) * Weight;
		WeightAccumulator += Weight;
	}
	return lerp(Input.Sample(LinearSampler, TexCoord), Accumulator / WeightAccumulator, Mask.Sample(LinearSampler, TexCoord).r);
}
