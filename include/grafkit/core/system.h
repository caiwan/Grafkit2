#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <grafkit/core/Renderer.h>
#include <grafkit/core/input.h>
#include <grafkit/core/window.h>

namespace Grafkit::Core
{
	class System // : public Window::WindowHandler
	{
	public:
		System();
		virtual ~System();

		int Execute();

		virtual int Initialize() = 0;
		virtual int Mainloop() = 0;
		virtual void Release() = 0;

	protected:
		[[nodiscard]] ErrorCode InitializeRender(
			int screenWidth = 800,
			int screenHeight = 600,
			int isFullscreen = 0,
			int resizeable = 0,
			const char* title = "Grafkit2 Application");
		void Shutdown();

		Window m_window;
		Input m_input;

		Renderer m_render;
		RenderTarget m_screen;
		RenderTarget m_backBuffer;

		// shortcut for esc
		[[nodiscard]] bool IsEscPressed() const { return m_input.IsKeyDown(VK_ESCAPE); }
	};

} // namespace Grafkit
