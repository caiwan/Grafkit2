#include <grafkit/render/Light.hpp>

using namespace Grafkit::Core;
using namespace Grafkit::Render;

Light::Light(Renderer& Renderer)
	: m_InternalRenderer(Renderer)
{
}

void Light::Initialize() noexcept
{
	m_lightConstantBuffer.lightCount = 2;

	m_lightConstantBuffer.lights[0].direction = { -0.5f, -0.094f, -0.5f };
	m_lightConstantBuffer.lights[0].color = { 0.9f, 0.9f, 0.521f, 1.0f };
	m_lightConstantBuffer.lights[0].lightIntensity = 1.5f;

	m_lightConstantBuffer.lights[1].direction = { -0.500f, -0.094f, 0.714f };
	m_lightConstantBuffer.lights[1].color = { 0.493f, 0.679f, 1.000f, 1.000f };
	m_lightConstantBuffer.lights[1].lightIntensity = 5.0f;
	m_InternalRenderer.CreateConstantBufferWithData(m_lightConstantBuffer, m_constantBuffer);
}

void Light::OnRender() noexcept
{
	m_InternalRenderer.SetConstantBuffer(m_constantBuffer, ShaderStage::PIXEL, 1);
	m_InternalRenderer.UpdateSubResource(m_constantBuffer, &m_lightConstantBuffer, sizeof(LightConstantBuffer));
}

void Light::OnUpdate(const float Time) noexcept { }
