#pragma once

#include <algorithm>
#include <assert.h>
#include <list>
#include <map>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//#pragma warning(push)
#pragma warning(disable : 4005) // Disable Overdefine a macro
#pragma warning(disable : 4244) // Disable Conversion from T1 to T2 possible loss of data
#pragma warning(disable : 4267) // Disable Conversion from 'size_t' to 'type',
                                // possible loss of data
#pragma warning(disable : 4290) // https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-3-c4290?view=vs-2019
                                // and
                                // https://docs.microsoft.com/en-us/cpp/cpp/exception-specifications-throw-cpp?view=vs-2019
#pragma warning(disable : 4305) // Disable Truncation from T1 to T2
#pragma warning(disable : 4838) // Disable conversion from T1 to T2 requires a
                                // narrowing conversion
#pragma warning(disable : 5040) // https://stackoverflow.com/questions/50399940/workaround-for-spectre-warning-msvc-c5040

#ifdef __cplusplus
#	include "common.h"
#	include "core/exceptions.h"
#	include "core/logger.h"
#	include "core/memory.h"
#	include "core/thread.h"
#	include "render/dxtypes.h"
#	include "render/renderer.h"
#	include "utils/archive.h"
#endif //__cplusplus

//#pragma warning(pop)
