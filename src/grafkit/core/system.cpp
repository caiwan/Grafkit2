#include <grafkit/core/input.h>
#include <grafkit/core/system.h>
#include <grafkit/core/window.h>

//#include <spdlog/spdlog.h>

using namespace Grafkit;

System::System() : /*WindowHandler(), */ mWindow()
{
	// SPDLOG_INFO("---- APPSTART ----");

	mWindow.SubscribeMessage([&](HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> bool {
		switch (umsg)
		{
			case WM_KEYDOWN: mInput.KeyDown(static_cast<unsigned int>(wparam)); break;
			case WM_KEYUP: mInput.KeyUp(static_cast<unsigned int>(wparam)); break;
		}
		return false;
	});
}

System::~System() {}

int System::Execute()
{
	int result = 0;
	MSG msg;

	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// ================================================================================================================================
	// --- RegisterWindow

	try
	{
		result = Initialize();
		if (result != 0)
		{
			Release();
			return 1;
		}
	}
	catch (std::exception & ex)
	{
		///@todo handle exceptions here
		MessageBoxA(nullptr, ex.what(), "Exception", 0);
		// SPDLOG_ERROR(ex.what());

		Release();

		return 0;
	}

	// ================================================================================================================================
	// --- Mainloop

	try
	{
		int done = 0;
		while (!done)
		{
			// Handle the windows messages.
			while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// If windows signals to end the application then exit out.
			if (msg.message == WM_QUIT) { done = 1; }
			else
			{
				// Otherwise do the frame processing.
				result = Mainloop();
				if (result != 0) { done = 1; }
			}
		}
	}
	catch (std::exception & ex)
	{
#ifdef _DEBUG
		DebugBreak();
#endif
		MessageBoxA(nullptr, ex.what(), "Exception", 0);
		// SPDLOG_ERROR(ex.what());
	}

	// ================================================================================================================================
	// --- teardown
	{
		Release();
	}

	ShutdownWindows();

	return 0;
}

// LRESULT System::MessageHandler

void System::InitializeWindows(int screenWidth, int screenHeight, int fullscreen, int resizeable, const char * pTitle)
{
	mWindow.InitWindow(screenWidth, screenHeight, fullscreen);
	mWindow.SetVisible(true);
	mWindow.SetTitle(pTitle);
}

void System::ShutdownWindows() { mWindow.Shutdown(); }
