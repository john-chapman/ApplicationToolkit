#include <apt/platform.h>

#include <apt/win.h>
#include <apt/memory.h>
#include <apt/String.h>

#include <intrin.h> // __cpuid

#pragma comment(lib, "version")

const char* apt::GetPlatformErrorString(uint64 _err)
{
	static thread_local String<1024> ret;
	ret.setf("(%llu) ", _err);
	APT_VERIFY(
		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
			NULL, 
			(DWORD)_err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)(ret.begin() + ret.getLength()),
			(DWORD)(ret.getCapacity() - ret.getLength()),
			NULL
		) != 0
	);
	return (const char*)ret;
}

const char* apt::GetPlatformInfoString()
{
	static thread_local String<1024> ret;

 // OS version
	ret.appendf("\tOS:     ");
	#if 0
	 // GetVersionEx is deprecated
		OSVERSIONINFOEX osinf;
		osinf.dwOSVersionInfoSize = sizeof(osinf);
		if (!GetVersionEx((LPOSVERSIONINFOA)&osinf)) {
			ret.append((const char*)GetPlatformErrorString(GetLastError()));
		} else {
			ret.appendf("Windows %u.%u.%u", osinf.dwMajorVersion, osinf.dwMinorVersion, osinf.dwBuildNumber);
		}
	#else
		void* verinf;
		VS_FIXEDFILEINFO* osinf;
		UINT osinfsz;
		DWORD sz = GetFileVersionInfoSize("kernel32.dll", NULL);
		if (sz == 0) {
			goto osver_failure;
		}
		verinf = APT_MALLOC(sz);
		if (!GetFileVersionInfo("kernel32.dll", (DWORD)0, sz, verinf)) {
			goto osver_failure;
		}
		if (!VerQueryValue(verinf, "\\", (LPVOID*)&osinf, &osinfsz)) {
			goto osver_failure;
		}
		ret.appendf("Windows %u.%u.%u", HIWORD(osinf->dwProductVersionMS), LOWORD(osinf->dwProductVersionMS), HIWORD(osinf->dwProductVersionLS));
		goto osver_end;
	osver_failure:
		ret.appendf("Windows %u.%u.%u", HIWORD(osinf->dwProductVersionMS), LOWORD(osinf->dwProductVersionMS), HIWORD(osinf->dwProductVersionLS));
	osver_end:
		APT_FREE(verinf);
		
	#endif
	
 // cpu vendor/brand
	int cpuinf[4] = { -1 };
	char cpustr[64];
	__cpuid(cpuinf, 0x80000002);
	memcpy(cpustr + 0,  cpuinf, sizeof(cpuinf));
	__cpuid(cpuinf, 0x80000003);
	memcpy(cpustr + 16, cpuinf, sizeof(cpuinf));
	__cpuid(cpuinf, 0x80000004);
	memcpy(cpustr + 32, cpuinf, sizeof(cpuinf));
	ret.appendf("\n\tCPU:    %s", cpustr);

 // proccessor count
	SYSTEM_INFO sysinf;
	GetSystemInfo(&sysinf);
	ret.appendf(" (%u cores)", sysinf.dwNumberOfProcessors);
	
 // global memory status
	MEMORYSTATUSEX meminf;
	meminf.dwLength = sizeof(meminf);
	ret.append("\n\tMemory: ");
	if (!GlobalMemoryStatusEx(&meminf)) {
		ret.append((const char*)GetPlatformErrorString(GetLastError()));
	} else {
		ret.appendf("%lluMb", meminf.ullTotalPhys / 1024 / 1024);
	}

	return (const char*)ret;
}