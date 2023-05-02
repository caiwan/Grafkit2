#include "application.h"

Application::Application(fwzSettings& settings)
	: System()
	//, mGraphicsContext()
	, mSettings(settings)
{
}

Application::~Application() = default;

int Application::Initialize()
{
	const auto result = InitializeRender(mSettings.scrWidth, mSettings.scrHeight);
	return result == ErrorCode::OK ? 0 : 1;
}

int Application::Mainloop()
{

	// TODO: m_render.Begin(m_screen)

	m_render.SetRenderTarget(m_screen);
	m_render.ClearRenderTarget(m_screen, DirectX::XMFLOAT4(0.0f, 0.2f, 0.4f, 1.0f));
	m_render.ClearDepthStencil(m_screen, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_render.SetViewport(m_screen.width, m_screen.height);

	// Do the thing here

	// TODO: m_render.End(m_backBuffer)
	m_render.SetRenderTarget(m_backBuffer);
	m_render.ClearRenderTarget(m_backBuffer, DirectX::XMFLOAT4(0.0f, 0.2f, 0.4f, 1.0f));

	const auto result = m_render.Present(0, 0);

	return IsEscPressed() || result != ErrorCode::OK;
}

void Application::Release() { }
