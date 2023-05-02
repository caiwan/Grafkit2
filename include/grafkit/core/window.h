/**
  This file contains a base class to create and handle windows on the windows
  platform
*/

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <functional>
#include <string>
#include <vector>

namespace Grafkit::Core
{
	/**
	  Base window class
	*/
	class Window
	{
	public:
		explicit Window(const std::string& title);
		explicit Window(const std::string&& title);
		explicit Window();

		~Window();

		void InitWindow(int sx, int sy, int isFullscreen);
		void Shutdown();

		void RecreateWindow();

		void SetVisible(bool isVisible);
		[[nodiscard]] bool IsVisible() const { return mIsVisible; }

		bool PeekWindowMessage();

		void SetTitle(const std::string& txt);
		void SetTitle(const std::string&& txt);

		[[nodiscard]] int Width() const { return mWindowWidth; }
		[[nodiscard]] int Height() const { return mWindowHeight; }

		[[nodiscard]] size_t SurfaceWidth() const { return mSurfaceWidth; }
		[[nodiscard]] size_t SurfaceHeight() const { return mSurfaceHeight; }

		[[nodiscard]] bool IsFullscreen() const { return mIsFullscreen; }

		HWND NativeWindowHandler() const { return mWindowHandler; }
		HDC NativeDeviceContext() const { return mDeviceContext; }

		bool PublishMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void SubscribeMessage(const std::function<bool(HWND, UINT, WPARAM, LPARAM)> callback)
		{
			mSubscribers.push_back(std::move(callback));
		}

	private:
		/// Common constructor
		void RegisterWindow();

		std::vector<std::function<bool(HWND, UINT, WPARAM, LPARAM)>> mSubscribers;

		std::string mTitle;

		size_t mWindowHeight = 0;
		size_t mWindowWidth = 0;

		size_t mSurfaceHeight = 0;
		size_t mSurfaceWidth = 0;

		bool mIsFullscreen = false;

		bool mIsVisible = false;

		HWND mWindowHandler = nullptr;
		HDC mDeviceContext = nullptr;
	};
} // namespace Grafkit
