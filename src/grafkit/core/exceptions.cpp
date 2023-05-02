#include <comdef.h>
#include <sstream>
//
#include <grafkit/core/exceptions.h>

using namespace Grafkit::Core::Exceptions;

std::string HrException::FormatHResultException(const std::string& message, HRESULT hResult)
{
	std::string result;
	const _com_error err(hResult);
	const LPCTSTR details = err.ErrorMessage();
	result.append(message);
	result.append(" ");
	result.append(details);
	return result;
}

std::string DwException::FormatDwordException(const std::string& message, DWORD dword)
{
	std::string result;
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dword,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&lpMsgBuf),
		0,
		nullptr);
	result.append(message);
	result.append(" ");
	result.append(static_cast<const char*>(lpMsgBuf));
	return result;
}
