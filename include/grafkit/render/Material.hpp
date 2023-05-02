#pragma once

#include <grafkit/core/Renderer.h>
#include <string>

namespace Grafkit::Render
{
	class Material
	{
	public:
		explicit Material(Core::Renderer& Renderer);

		~Material();

		void Initialize(const uint32_t Width, const uint32_t Height) noexcept;
		void OnRender(const Core::RenderTarget* RenderTargets = nullptr, const size_t Count = 0) noexcept;

	private:
		Core::Renderer& m_internalRenderer;

		Core::RenderTarget m_albedo {};
		Core::RenderTarget m_metalness {};
		Core::RenderTarget m_roughness {};
		Core::RenderTarget m_normal {};
		Core::Shader m_shader {};
		Core::Buffer m_constantBuffer {};

		enum class IlluminationModel : uint32_t
		{
			DISNEY = 0,
			COOKTORRANCE,
			COUNT
		};

		struct SMaterialConstantBuffer
		{
			IlluminationModel m_Illumination = IlluminationModel::DISNEY;
		};

		SMaterialConstantBuffer MaterialConstantBuffer {};

		bool bIsInitialized = false;
	};
}
