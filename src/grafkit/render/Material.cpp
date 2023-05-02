#include <grafkit/render/Material.hpp>

using namespace Grafkit::Core;
using namespace Grafkit::Render;

Material::Material(Renderer& Renderer)
	: m_internalRenderer(Renderer)
{
}

Material::~Material()
{
	m_internalRenderer.DestroyTexture(m_albedo);
	m_internalRenderer.DestroyTexture(m_metalness);
	m_internalRenderer.DestroyTexture(m_roughness);
	m_internalRenderer.DestroyTexture(m_normal);

	m_internalRenderer.DestroyShader(m_shader);
	m_internalRenderer.DestroyBuffer(m_constantBuffer);
}

void Material::Initialize(const uint32_t Width, const uint32_t Height) noexcept
{
	if (bIsInitialized)
	{
		return;
	}
	const D3D11_INPUT_ELEMENT_DESC InputElementDescriptors[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	const uint8_t ShockingPink[4] = { 252, 15, 192, 255 };
	m_internalRenderer.CreateTextureFromMemory(ShockingPink, 1, 1, 4, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, m_albedo);
	m_internalRenderer.CreateTextureFromMemory(ShockingPink, 1, 1, 4, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, m_metalness);
	m_internalRenderer.CreateTextureFromMemory(ShockingPink, 1, 1, 4, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, m_roughness);
	m_internalRenderer.CreateTextureFromMemory(ShockingPink, 1, 1, 4, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, m_normal);
	m_internalRenderer.CreateVertexShader(L"DefaultVS.hlsl", "main", InputElementDescriptors, 5, m_shader);
	m_internalRenderer.CreatePixelShader(L"DefaultPS.hlsl", "main", m_shader);

	m_internalRenderer.CreateConstantBufferWithData(MaterialConstantBuffer, m_constantBuffer);

	bIsInitialized = true;
}

void Material::OnRender(const RenderTarget* RenderTargets, const size_t Count) noexcept
{
	m_internalRenderer.SetShader(m_shader);
	m_internalRenderer.SetConstantBuffer(m_constantBuffer, ShaderStage::PIXEL, 2);
	m_internalRenderer.UpdateSubResource(m_constantBuffer, &MaterialConstantBuffer, sizeof(struct SMaterialConstantBuffer));

	m_internalRenderer.SetTexture(0, m_albedo);
	m_internalRenderer.SetTexture(1, m_metalness);
	m_internalRenderer.SetTexture(2, m_roughness);
	m_internalRenderer.SetTexture(3, m_normal);
}
