#pragma once
#ifndef apt_log_h
#define apt_log_h

#include <apt/def.h>

#define APT_LOG(...)              do { apt::internal::Log(__VA_ARGS__); } while (0)
#define APT_LOG_ERR(...)          do { apt::internal::LogError(__VA_ARGS__); } while (0)
#ifdef APT_DEBUG
	#define APT_LOG_DBG(...)      do { apt::internal::LogDebug(__VA_ARGS__); } while (0)
#else
	#define APT_LOG_DBG(...)      do { } while(0)
#endif

namespace apt {

enum class LogType
{
	kLog,
	kError,
	kDebug
};

/// Typedef for log callbacks. Callbacks receive a message (as passed to the 
/// APT_LOG variant), plus an enum indicating whether the log was generated by
/// APT_LOG, APT_LOG_ERR or APT_LOG_DBG.
/// \ingroup APT_core
typedef void (LogCallback)(const char* _msg, LogType _type);

/// Set the current log callback. The default is 0.
/// \ingroup APT_core
void SetLogCallback(LogCallback* _callback);

/// \return Current log callback. The default is 0.
/// \ingroup APT_core
LogCallback* GetLogCallback();

} // namespace apt

namespace apt { namespace internal {

void Log     (const char* _fmt, ...);
void LogError(const char* _fmt, ...);
void LogDebug(const char* _fmt, ...);

} } // namespace apt::internal

#endif // apt_log_h
