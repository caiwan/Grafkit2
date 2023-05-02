#include <grafkit/render/BlurMaterial.hpp>

using namespace Grafkit::Core;
using namespace Grafkit::Render;

BlurMaterial::BlurMaterial(Renderer& Renderer)
	: m_InternalRenderer(Renderer)
{
	m_IsEnabled = false;
}

BlurMaterial::~BlurMaterial()
{
	m_InternalRenderer.DestroyShader(m_BlurXShader);
	m_InternalRenderer.DestroyShader(m_BlurYShader);
	;

	m_InternalRenderer.DestroyBuffer(m_BlurConstantBuffer);

	m_InternalRenderer.DestroyRenderTarget(m_RenderTarget);
	m_InternalRenderer.DestroyRenderTarget(m_FinalRenderTarget);

	m_InternalRenderer.DestroyTexture(m_MaskTexture);
}

ErrorCode BlurMaterial::Initialize(const uint32_t width, const uint32_t height) noexcept
{

	CHECK_RESULT(m_InternalRenderer.CreateVertexShader(L"FullScreenTriangleVS.hlsl", "main", nullptr, 0, m_BlurXShader));
	CHECK_RESULT(m_InternalRenderer.CreatePixelShader(L"BlurXPS.hlsl", "main", m_BlurXShader));
	CHECK_RESULT(m_InternalRenderer.CreateVertexShader(L"FullScreenTriangleVS.hlsl", "main", nullptr, 0, m_BlurYShader));
	CHECK_RESULT(m_InternalRenderer.CreatePixelShader(L"BlurYPS.hlsl", "main", m_BlurYShader));
	CHECK_RESULT(m_InternalRenderer.CreateConstantBufferWithData(m_BlurParams, m_BlurConstantBuffer));
	auto* maskBuffer = new uint8_t[width * height];
	for (size_t row = 0; row < height; ++row)
	{
		memset(maskBuffer + (row * width), 0, width);
		memset(maskBuffer + (row * width + static_cast<uint32_t>(width * 0.5f)), 255, static_cast<uint32_t>(width * 0.5f));
	}
	const auto result = m_InternalRenderer.CreateTextureFromMemory(maskBuffer, width, height, 1, DXGI_FORMAT_R8_UNORM, m_MaskTexture);
	delete[] maskBuffer;
	if (result != ErrorCode::OK)
	{
		return result;
	};

	CHECK_RESULT(m_InternalRenderer.CreateRenderTarget(width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, m_RenderTarget));
	CHECK_RESULT(m_InternalRenderer.CreateRenderTarget(width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, m_FinalRenderTarget));

	return ErrorCode::OK;
#undef CHECK_RESULT
}

void BlurMaterial::OnRender(const RenderTarget* RenderTargets, const size_t Count) noexcept
{
	assert(RenderTargets != nullptr);
	assert(Count == 1);

	if (m_IsEnabled)
	{
		// X Pass
		m_InternalRenderer.SetRenderTarget(m_RenderTarget);
		m_InternalRenderer.ClearRenderTarget(m_RenderTarget, DirectX::XMFLOAT4(0.0f, 0.2f, 0.4f, 1.0f));
		m_InternalRenderer.SetViewport(m_RenderTarget.width, m_RenderTarget.height);
		m_InternalRenderer.SetShader(m_BlurXShader);
		m_InternalRenderer.SetConstantBuffer(m_BlurConstantBuffer, ShaderStage::PIXEL);
		m_InternalRenderer.UpdateSubResource(m_BlurConstantBuffer, &m_BlurParams, sizeof(m_BlurParams));
		m_InternalRenderer.SetTexture(0, RenderTargets[0]);
		m_InternalRenderer.SetTexture(1, m_MaskTexture);
		m_InternalRenderer.SetConstantBuffer({ nullptr, 0 }, ShaderStage::VERTEX);
		m_InternalRenderer.SetVertexBuffer(0, { nullptr, 0 }, 0);
		m_InternalRenderer.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_InternalRenderer.Draw(3, 0);

		// Y Pass
		m_InternalRenderer.SetRenderTarget(m_FinalRenderTarget);
		m_InternalRenderer.ClearRenderTarget(m_FinalRenderTarget, DirectX::XMFLOAT4(0.0f, 0.2f, 0.4f, 1.0f));
		m_InternalRenderer.SetViewport(m_FinalRenderTarget.width, m_FinalRenderTarget.height);
		m_InternalRenderer.SetShader(m_BlurYShader);
		m_InternalRenderer.SetConstantBuffer(m_BlurConstantBuffer, ShaderStage::PIXEL);
		m_InternalRenderer.UpdateSubResource(m_BlurConstantBuffer, &m_BlurParams, sizeof(m_BlurParams));
		m_InternalRenderer.SetTexture(0, m_RenderTarget);
		m_InternalRenderer.SetTexture(1, m_MaskTexture);
		m_InternalRenderer.SetConstantBuffer({ nullptr, 0 }, ShaderStage::VERTEX);
		m_InternalRenderer.SetVertexBuffer(0, { nullptr, 0 }, 0);
		m_InternalRenderer.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_InternalRenderer.Draw(3, 0);

		m_InternalRenderer.UnbindRenderTargets();
	}
}

void BlurMaterial::SetMask(const RenderTarget& Mask) noexcept { m_MaskTexture = Mask; }

const RenderTarget& BlurMaterial::GetResult() const noexcept { return m_FinalRenderTarget; }

bool BlurMaterial::IsEnabled() const noexcept { return m_IsEnabled; }
