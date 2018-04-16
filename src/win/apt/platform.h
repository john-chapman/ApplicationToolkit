#pragma once

#include <apt/apt.h>

#if !(APT_PLATFORM_WIN)
	#error apt: APT_PLATFORM_WIN was not defined, probably the build system was configured incorrectly
#endif

// ASSERT/VERIFY with platform-specific error string (use to wrap OS calls).
#define APT_PLATFORM_ASSERT(_err) APT_ASSERT_MSG(_err, apt::GetPlatformErrorString((uint64)::GetLastError()))
#define APT_PLATFORM_VERIFY(_err) APT_VERIFY_MSG(_err, apt::GetPlatformErrorString((uint64)::GetLastError()))

namespace apt {

// Format a system error code as a string.
const char* GetPlatformErrorString(uint64 _err);

// Return a string containing OS, CPU and system memory info.
const char* GetPlatformInfoString(); 

} // namespace apt
