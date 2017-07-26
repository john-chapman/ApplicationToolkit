#pragma once
#ifndef apt_platform_h
#define apt_platform_h

#include <apt/apt.h>
#ifndef APT_PLATFORM_WIN
	#error apt: APT_PLATFORM_WIN was not defined, probably the build system was configured incorrectly
#endif

// ASSERT/VERIFY with platform-specific error string (use to wrap OS calls)
#define APT_PLATFORM_ASSERT(_err) APT_ASSERT_MSG(_err, apt::GetPlatformErrorString((uint64)::GetLastError()))
#define APT_PLATFORM_VERIFY(_err) APT_VERIFY_MSG(_err, apt::GetPlatformErrorString((uint64)::GetLastError()))

namespace apt {

/// Format a system error code as a string.
/// \note This returns a ptr to an internal static char buffer. The buffer is
///	   thread-local but calling code **must** copy the returned string immediately
///	   and not store the returned ptr.
const char* GetPlatformErrorString(uint64 _err);

} // namespace apt

#endif // apt_platform_h
