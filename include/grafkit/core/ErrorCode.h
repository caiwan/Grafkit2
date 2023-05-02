#pragma once

enum class ErrorCode
{
	OK,
	FAIL,
	FILENOTFOUND,
	INVALIDCALL,
	NOTIMPLEMENTED
};

#define CHECK_RESULT(x)                                                                                                                    \
	{                                                                                                                                      \
		if (const auto result = (x); result != ErrorCode::OK)                                                                              \
		{                                                                                                                                  \
			return result;                                                                                                                 \
		}                                                                                                                                  \
	}
