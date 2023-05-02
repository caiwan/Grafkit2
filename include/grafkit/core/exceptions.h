#pragma once

#include <stdexcept>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//

namespace Grafkit::Core::Exceptions
{
	// extern std::string FormatDwordException(const std::string & message, DWORD dword);
	// extern

	class HrException : public std::runtime_error
	{
	public:
		HrException(const HRESULT hr, const std::string& message = {})
			: std::runtime_error(FormatHResultException(message, hr))
			, mHr(hr)
		{
		}
		HRESULT Error() const { return mHr; }

		static std::string FormatHResultException(const std::string& message, HRESULT hResult);

	private:
		const HRESULT mHr;
	};

	class DwException : public std::runtime_error
	{
	public:
		DwException(const DWORD dword, const std::string& message = {})
			: std::runtime_error(FormatDwordException(message, dword))
			, mDword(dword)
		{
		}

		static std::string FormatDwordException(const std::string& message, DWORD dword);

	private:
		DWORD mDword;
	};

	inline void ThrowIfFailed(const bool result, const std::string& message)
	{
		if (!result)
		{
			throw std::runtime_error(message);
		}
	}

	inline void ThrowIfFailedHr(const HRESULT result, const std::string& message = "")
	{
		if (FAILED(result))
		{
			// TODO: Unresolved external
			// throw std::runtime_error(message + FormatHResultException(message, result));
			throw HrException(result, message);
		}
	}

	inline void ThrowIfFailedDw(const DWORD result, const std::string& message = "")
	{
		if (result != 0) // TODO: There is an ERROR_OK or RESULT_OK somewhere
		{
			throw DwException(result, message);
		}
	}

} // namespace Grafkit::Core::Exceptions
