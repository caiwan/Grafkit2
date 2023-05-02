#pragma once
#include <grafkit/core/Renderer.h>

namespace Grafkit::Render
{
	class Light
	{
	public:
		explicit Light(Core::Renderer& Renderer);

		void Initialize() noexcept;

		void OnRender() noexcept;
		void OnUpdate(float Time) noexcept;

	private:
		static constexpr uint8_t MAX_LIGHTS = 8;

		struct LightData
		{
			DirectX::XMFLOAT4 color;
			DirectX::XMFLOAT3 direction;
			float lightIntensity;
		};

		struct LightConstantBuffer
		{
			LightData lights[MAX_LIGHTS];
			uint8_t lightCount;
		};

		LightConstantBuffer m_lightConstantBuffer {};

		Core::Buffer m_constantBuffer {};
		Core::Renderer& m_InternalRenderer;
	};
};
