#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <apt/log.h>
#include <apt/math.h>
#include <apt/platform.h>
#include <apt/Time.h>


	#include <apt/win.h>
	#include <apt/memory.h>
	#include <apt/Pool.h>

using namespace apt;

namespace {
/*
	Notes:
	- In Pasta the WatchData struct is passed to the callback as the 'overlapped' argument - it's the first member of the struct so the ptrs are the same.
	  Actually the hEvent member of the OVERLAPPED struct isn't used, so we could hide the WatchData struct in there.
	- We re-issue the overlapped IO call as quickly as possible from inside the completion routine instead of dispatching THEN reissuing. Beveridge says it's
	  not clear what happens to notifications between the callback being called and the reissue - a simple test shows that they ARE dispatched.
	- The completion routine is called via SleepEx(0, TRUE) - it is therefore on the application's main thread, no need to synchronize anything.
	- Multiple FILE_ACTION_MODIFIED commands may be received! The application needs to filter these (and potentially determine *what* changed, e.g. file 
	  attributes, size).

	API Design:
	- Internally, the completion routine should queue and filter notifications (basically to remove duplicates). It seems safe to rely on the fact that duplicate
	  modifications result in *consecutive* calls to the function, so you don't need to search the whole queue just check the back. 
	- FileSystem::BeginNotifications(const char* _dir, Callback* _cbk)/FileSystem::EndNotifications(const char* _dir) -- use a single watcher per dir.
	- FileSystem::DispatchNotifications(const char* _dir) -- calls SleepEx(0, TRUE) ONCE to allow the kernel to fill the queue, then dispatch the queue to the 
	  callback.

	Todo:
	- Avoid using asserts in favor of a more robust API which logs the errors.
*/
	struct WatchData
	{
		OVERLAPPED m_overlapped = {};
		HANDLE     m_hDir       = INVALID_HANDLE_VALUE;
		DWORD      m_filter     = 0;
		UINT       m_bufSize    = 1024 * 32; // 32kb
		BYTE*      m_buf        = NULL;
	};
	static apt::Pool<WatchData> s_WatchList(8);

	WatchData* BeginWatch(const char* _dir);
	void       UpdateWatch(WatchData* _watch);
	void       EndWatch(WatchData*& _watch_);


	void CALLBACK WatchCompletion(DWORD _err, DWORD _bytes, LPOVERLAPPED _overlapped)
	{
		if (_err == ERROR_OPERATION_ABORTED) { // CancellIo was called
			return;
		}
		APT_ASSERT(_err == ERROR_SUCCESS);
		APT_ASSERT(_bytes != 0); // overflow? notifications losts in this case?

		WatchData* watch = (WatchData*)_overlapped; // m_overlapped is the first member, so this works

		APT_LOG_DBG(" --- ");

		TCHAR fileName[MAX_PATH];
		for (DWORD off = 0;;) {
			PFILE_NOTIFY_INFORMATION info = (PFILE_NOTIFY_INFORMATION)(watch->m_buf + off);		
			off += info->NextEntryOffset;

		 // unicode -> utf8
			int count = WideCharToMultiByte(CP_UTF8, 0, info->FileName, info->FileNameLength / sizeof(WCHAR), fileName, MAX_PATH - 1, NULL, NULL);
			fileName[count] = '\0';

			const char* action = "--";
			#define CASE_ENUM(e) case e: action = #e; break
			switch(info->Action) {
				CASE_ENUM(FILE_ACTION_ADDED);
				CASE_ENUM(FILE_ACTION_REMOVED);
				CASE_ENUM(FILE_ACTION_MODIFIED);
				CASE_ENUM(FILE_ACTION_RENAMED_NEW_NAME);
				CASE_ENUM(FILE_ACTION_RENAMED_OLD_NAME);
				default : "?";
			};
			#undef CASE_ENUM
			APT_LOG("%s : %s", fileName, action);
			
			if (info->NextEntryOffset == 0) {
				break;
			}
		}

	 // reissue ReadDirectoryChangesW; it seems that we don't actually miss any notifications which happen between the start of the completion routine
	 // and the reissue, so it's safe to wait until the dispatch is done
		UpdateWatch(watch);
	}

	WatchData* BeginWatch(const char* _dir)
	{
		CreateDirectoryA(_dir, NULL); // create if it doesn't already exist
	
		WatchData* ret = s_WatchList.alloc();

		ret->m_hDir = CreateFileA(
			_dir,                                                    // path
			FILE_LIST_DIRECTORY,                                     // desired access
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,  // share mode
			NULL,                                                    // security attribs
			OPEN_EXISTING,                                           // create mode
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,       // file attribs
			NULL                                                     // template handle
			);
		APT_PLATFORM_ASSERT(ret->m_hDir != INVALID_HANDLE_VALUE);

		// not required, we're using a completion routine
		//m_overlapped->m_hEvent = CreateEvent(
		//	NULL,  // security attribs
		//	TRUE,  // manual reset
		//	FALSE, // initial state
		//	NULL   // name
		//	);
		//APT_PLATFORM_ASSERT(ret->m_hEvent != INVALID_HANDLE_VALUE);

		ret->m_buf = (BYTE*)malloc_aligned(ret->m_bufSize, sizeof(DWORD));

		ret->m_filter = 0
			| FILE_NOTIFY_CHANGE_CREATION
			| FILE_NOTIFY_CHANGE_SIZE
			| FILE_NOTIFY_CHANGE_ATTRIBUTES 
			| FILE_NOTIFY_CHANGE_FILE_NAME 
			| FILE_NOTIFY_CHANGE_DIR_NAME 
			;

		UpdateWatch(ret);

		return ret;
	}

	void UpdateWatch(WatchData* _watch)
	{
		DWORD retBytes;
		APT_PLATFORM_VERIFY(ReadDirectoryChangesW(
			_watch->m_hDir,
			_watch->m_buf,
			_watch->m_bufSize,
			TRUE, // watch subtree
			_watch->m_filter,
			&retBytes,
 			&_watch->m_overlapped, 
			WatchCompletion
			));
	}

	void EndWatch(WatchData*& _watch_)
	{
		APT_VERIFY(CancelIo(_watch_->m_hDir));
		SleepEx(0, TRUE); // flush any pending calls to the completion routine

		CloseHandle(_watch_->m_hDir);
		free_aligned(_watch_->m_buf);
		s_WatchList.free(_watch_);
		_watch_ = nullptr;
	}


}

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


	auto watch = BeginWatch("dirtest");
	APT_LOG("Press ESC to quit...");
	while ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) == 0) {

	 // this must be called
		SleepEx(0, TRUE);
	}
	EndWatch(watch);
}
