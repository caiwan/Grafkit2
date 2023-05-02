//#include <wingdi.h>
#include <cstdlib>
#include <tchar.h>
//
#include <grafkit/core/Exceptions.h>
#include <grafkit/core/Window.h>

#include <Windows.h>

//============================================================================================================
// Window handler and callback
//============================================================================================================
#define LRWAPI LRESULT WINAPI

using namespace Grafkit::Core;
using namespace Exceptions;

namespace
{
	LRWAPI window_callback_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

	Window* gWindow;

	LRWAPI window_callback_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		// Publish all messages
		if (gWindow && gWindow->PublishMessage(hwnd, msg, wp, lp))
			return true;

		// Maintain default functionality
		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		default:
			break;
		}

		return DefWindowProc(hwnd, msg, wp, lp);
	}
}

//============================================================================================================
// Window class
//============================================================================================================

Window::Window(/*WindowHandler * handler*/) //: mCallbackHandler(handler)
{
	ThrowIfFailed(!gWindow, "A global window already had been created.");
	gWindow = this;
	RegisterWindow();
}

Window::Window(/*WindowHandler * handler, */ const std::string& title) //: mCallbackHandler(handler)
{
	// assert(!gWindow);
	ThrowIfFailed(!gWindow, "A global window already had been created.");
	gWindow = this;
	mTitle.assign(title);
	RegisterWindow();
}

Window::Window(/*WindowHandler * handler, */ const std::string&& title) //: mCallbackHandler(handler)
{
	ThrowIfFailed(!gWindow, "A global window already had been created.");
	gWindow = this;
	mTitle.assign(title);
	RegisterWindow();
}

bool Window::PublishMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	for (const auto subscriber : mSubscribers)
	{
		if (subscriber(hWnd, msg, wParam, lParam))
			return true;
	}
	return false;
}

void Window::RegisterWindow()
{
	// Register A Window Class
	WNDCLASSEX windowClass;
	ZeroMemory(&windowClass, sizeof(WNDCLASSEX));
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = static_cast<WNDPROC>(&window_callback_proc);
	windowClass.hInstance = GetModuleHandle(nullptr);
	windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_APPWORKSPACE);
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = "Engine";
	if (RegisterClassEx(&windowClass) == 0)
	{
		MessageBox(HWND_DESKTOP, _T("RegisterClassEx Failed!"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		exit(1);
	}
}

Window::~Window()
{
	Shutdown();
	gWindow = nullptr;
}

void Window::InitWindow(int sx, int sy, int isfullscreen)
{
	mWindowWidth = sx;
	mWindowHeight = sy;
	mIsFullscreen = isfullscreen;

	DWORD windowFrame = 0;
	DWORD windowStyle = WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	DWORD windowExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	RECT windowRect = { 0, 0, static_cast<LONG>(mWindowWidth), static_cast<LONG>(mWindowHeight) };

	if (mIsFullscreen)
	{
		DEVMODE screenSettings;
		ZeroMemory(&screenSettings, sizeof(screenSettings));
		screenSettings.dmSize = sizeof(screenSettings);
		screenSettings.dmPelsWidth = mWindowWidth;
		screenSettings.dmPelsHeight = mWindowHeight;
		screenSettings.dmBitsPerPel = 32;
		screenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		const auto result = ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
		if (result != DISP_CHANGE_SUCCESSFUL)
		{
			MessageBox(nullptr, _T(""), _T("Could not initialize fullscreen window."), MB_OK);
			exit(1);
		}
		windowFrame = NULL;
		ShowCursor(false);
	}
	else
	{
		ShowCursor(true);
		windowFrame = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	}

	const auto windowTitle = mTitle.c_str();

	mWindowHandler = CreateWindowEx(
		windowExStyle,
		_T("Engine"),
		windowTitle,
		windowStyle | windowFrame,
		windowRect.top,
		windowRect.left,
		windowRect.right,
		windowRect.bottom,
		nullptr,
		nullptr,
		GetModuleHandle(nullptr),
		nullptr);

	if (mWindowHandler == nullptr)
	{
		MessageBox(HWND_DESKTOP, _T("Cannot create window"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		exit(1);
	}

	mDeviceContext = GetDC(mWindowHandler); // TODO: No resource is freed

	GetClientRect(mWindowHandler, &windowRect);
	mSurfaceHeight = windowRect.bottom - windowRect.top;
	mSurfaceWidth = windowRect.right - windowRect.left;

	SetVisible(false);
}

void Window::SetVisible(bool isVisible)
{
	if (isVisible)
	{
		SetForegroundWindow(mWindowHandler);
		ShowWindow(mWindowHandler, SW_SHOWDEFAULT);
		UpdateWindow(mWindowHandler);
		mIsVisible = true;
	}
	else
	{
		ShowWindow(mWindowHandler, SW_HIDE);
		UpdateWindow(mWindowHandler);
		mIsVisible = false;
	}
}

void Window::Shutdown()
{
	PeekWindowMessage();
	if (mWindowHandler != nullptr)
	{
		if (mDeviceContext != nullptr)
		{
			ReleaseDC(mWindowHandler, mDeviceContext);
			mDeviceContext = nullptr;
		}
		DestroyWindow(mWindowHandler);
		mWindowHandler = nullptr;
	}

	if (mIsFullscreen)
	{
		ChangeDisplaySettings(nullptr, 0);
		ShowCursor(TRUE);
	}
	PeekWindowMessage();

	UnregisterClass("Engine", nullptr);
}

void Window::RecreateWindow()
{
	bool show = mIsVisible;
	Shutdown();
	InitWindow(mWindowWidth, mWindowHeight, mIsFullscreen);
	SetVisible(show);
}

bool Window::PeekWindowMessage()
{
	MSG uMsg;
	bool quit = false;
	while (PeekMessage(&uMsg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&uMsg);
		DispatchMessage(&uMsg);
		if (uMsg.message == WM_QUIT)
			quit = true;
	}
	return quit;
}

void Window::SetTitle(const std::string& txt)
{
	mTitle.assign(txt);
	SetWindowText(mWindowHandler, mTitle.c_str());
}

void Window::SetTitle(const std::string&& txt)
{
	mTitle.assign(txt);
	SetWindowText(mWindowHandler, mTitle.c_str());
}
