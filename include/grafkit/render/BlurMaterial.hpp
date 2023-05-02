#pragma once

#include <grafkit/core/Renderer.h>

namespace Grafkit::Render
{

	class BlurMaterial
	{
	private:
		struct BlurParams
		{
			float Smooth = 0.963f;	   // 0.0 -> box filter, > 0.0 for gaussian
			float Size = 0.643f;	   // length of the blur (global)
			float SamplesX = 0.352f;   // number of samples to take
			float SamplesY = 0.536f;   // number of samples to take
			float DirectionX = 0.488f; // direction of blur
			float DirectionY = 0.664f; // direction of blur
			float PowerX = 0.376f;	   // length of the blur
			float PowerY = 0.423f;	   // length of the blur
		};

	public:
		explicit BlurMaterial(Core::Renderer& Renderer);
		~BlurMaterial();

		ErrorCode Initialize(const uint32_t width, const uint32_t height) noexcept;
		void OnGui() noexcept;
		void OnRender(const Core::RenderTarget* RenderTargets, const size_t Count) noexcept;

		void SetMask(const Core::RenderTarget& Mask) noexcept;
		const Core::RenderTarget& GetResult() const noexcept;
		bool IsEnabled() const noexcept;

	private:
		Core::Renderer& m_InternalRenderer;
		BlurParams m_BlurParams {};

		Core::Shader m_BlurXShader {};
		Core::Shader m_BlurYShader {};

		Core::Buffer m_BlurConstantBuffer {};

		Core::RenderTarget m_RenderTarget {};
		Core::RenderTarget m_FinalRenderTarget {};

		Core::RenderTarget m_MaskTexture {};

		bool m_IsEnabled = true;
	};
}
