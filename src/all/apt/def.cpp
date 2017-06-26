#include <apt/def.h>
#include <apt/log.h>

#ifdef APT_COMPILER_MSVC
	#pragma warning(disable: 4996) // vsnprintf
#endif
#include <cstdarg> // va_list, va_start, va_end
#include <cstdio>  // vsnprintf

static const int kAssertMsgMax = 1024; // max size for message buffer in assert()
static apt::AssertCallback* g_AssertCallback = &apt::DefaultAssertCallback;

void apt::SetAssertCallback(AssertCallback* _callback) 
{
	g_AssertCallback = _callback;
}

apt::AssertCallback* apt::GetAssertCallback() {
	return g_AssertCallback;
}

apt::AssertBehavior apt::DefaultAssertCallback(const char* _e, const char* _msg, const char* _file, int _line) 
{
	APT_LOG_ERR("APT_ASSERT (%s, line %d)\n\t'%s' %s", _file, _line, _e ? _e : "", _msg ? _msg : "");
	return AssertBehavior_Break;
}

apt::AssertBehavior apt::internal::AssertAndCallback(const char* _e, const char* _file, int _line, const char* _msg, ...) 
{
	char buf[kAssertMsgMax];
	buf[0] = '\0';
	if (_msg != 0) {
		va_list args;
		va_start(args, _msg);
		APT_VERIFY(vsnprintf(buf, kAssertMsgMax, _msg, args) > 0);
		va_end(args);
	}

	if (g_AssertCallback) {
		return g_AssertCallback(_e, buf, apt::internal::StripPath(_file), _line);
	}
	return AssertBehavior_Break;
}

const char* apt::internal::StripPath(const char* _path) 
{
	int i = 0, last = 0;
	while (_path[i] != 0) {
		if (_path[i] == '\\' || _path[i] == '/')
			last = i + 1;
		++i;
	}
	return &_path[last];
}
