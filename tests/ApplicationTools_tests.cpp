#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <apt/log.h>
#include <apt/platform.h>
#include <apt/Time.h>

		#include <miniz.h>

using namespace apt;

TEST_CASE("adhoc")
{
	#ifdef APT_PLATFORM_WIN
	 // force the current working directoy to the exe location
		TCHAR buf[MAX_PATH] = {};
		DWORD buflen;
		APT_PLATFORM_VERIFY(buflen = GetModuleFileName(0, buf, MAX_PATH));
		char* pathend = strrchr(buf, (int)'\\');
		*(++pathend) = '\0';
		APT_PLATFORM_VERIFY(SetCurrentDirectory(buf));
		APT_LOG("Set current directory: '%s'", buf);
	#endif
/*	Compression todo:
	- See notes in miniz.h (line 100+). Make header-only, Strip unused APIs (need to add global macros)
	- Raw buffer compress/decompress via tdefl/tinfl (expose options?)
	- Look closer at the ZIP archive functions, inform redesign of Filesystem/File classes:
		- Keep files open, add seek interface.
		- How to manage archives? Filesystem class is all static functions, need a 'setimpl' or something? Look at other libs.
*/
}
