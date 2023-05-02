#include <grafkit/core/input.h>
#include <grafkit/core/system.h>
#include <grafkit/core/window.h>

//#include <spdlog/spdlog.h>

using namespace Grafkit::Core;

System::System()
	: /*WindowHandler(), */ m_window()
{
	// SPDLOG_INFO("---- APPSTART ----");

	m_window.SubscribeMessage([&](HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> bool {
		switch (umsg)
		{
		case WM_KEYDOWN:
			m_input.KeyDown(static_cast<unsigned int>(wparam));
			break;
		case WM_KEYUP:
			m_input.KeyUp(static_cast<unsigned int>(wparam));
			break;
		}
		return false;
	});
}

System::~System() { }

int System::Execute()
{
	int result = 0;
	MSG msg;

	// InitializeRender the message structure.
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
	catch (std::exception& ex)
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
			if (msg.message == WM_QUIT)
			{
				done = 1;
			}
			else
			{
				// Otherwise do the frame processing.
				result = Mainloop();
				if (result != 0)
				{
					done = 1;
				}
			}
		}
	}
	catch (std::exception& ex)
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

	Shutdown();

	return 0;
}

// LRESULT System::MessageHandler

ErrorCode System::InitializeRender(int screenWidth, int screenHeight, int fullscreen, int resizeable, const char* pTitle)
{
	m_window.InitWindow(screenWidth, screenHeight, fullscreen);
	m_window.SetVisible(true);
	m_window.SetTitle(pTitle);

	// TODO: check returning result
	CHECK_RESULT(
		m_render.CreateDeviceAndSwapChainForHwnd(m_window.NativeWindowHandler(), m_window.Width(), m_window.Height(), m_backBuffer));

	CHECK_RESULT(m_render.CreateRenderTarget(m_window.Width(), m_window.Height(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, m_screen));
	CHECK_RESULT(m_render.CreateDepthStencil(m_window.Width(), m_window.Height(), DXGI_FORMAT_D24_UNORM_S8_UINT, m_screen));

	return ErrorCode::OK;
}

void System::Shutdown()
{
	m_render.DestroyRenderTarget(m_backBuffer);
	m_render.DestroyRenderTarget(m_screen);
	m_render.Shutdown();
	m_window.Shutdown();
}
