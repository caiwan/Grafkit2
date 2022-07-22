#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <grafkit/core/input.h>
#include <grafkit/core/window.h>
#include <windows.h>

namespace Grafkit
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

		// LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) override;

	protected:
		void InitializeWindows(
		    int screenWidth = 800, int screenHeight = 600, int isFullscreen = 0, int resizeable = 0, const char * title = "Grafkit2 Application");
		void ShutdownWindows();

		Window mWindow;
		Input mInput;

		// shortcut for esc
		[[nodiscard]] bool IsEscPressed() const { return mInput.IsKeyDown(VK_ESCAPE); }
	};

} // namespace Grafkit
