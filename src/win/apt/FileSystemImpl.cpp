#include <apt/FileSystem.h>

#include <apt/log.h>
#include <apt/memory.h>
#include <apt/platform.h>
#include <apt/win.h>
#include <apt/Pool.h>
#include <apt/String.h>
#include <apt/StringHash.h>
#include <apt/TextParser.h>

#include <Shlwapi.h>
#include <commdlg.h>
#include <cstring>

#include <EASTL/vector.h>
#include <EASTL/vector_map.h>

#pragma comment(lib, "shlwapi")

using namespace apt;

// Windows expects a list of string pairs to pass to GetOpenFileName() (display name, filter).
static void BuildFilterString(std::initializer_list<const char*> _filterList, StringBase& ret_)
{
	for (auto& filter : _filterList) {
		ret_.appendf("%s?%s?", filter, filter);
	}
	ret_.replace('?', '\0'); // \hack
}

static DateTime FileTimeToDateTime(const FILETIME& _fileTime)
{
	LARGE_INTEGER li;
	li.LowPart  = _fileTime.dwLowDateTime;
	li.HighPart = _fileTime.dwHighDateTime;
	return DateTime(li.QuadPart);
}

static bool GetFileDateTime(const char* _fullPath, DateTime& created_, DateTime& modified_)
{
#if 0
	const char* err = nullptr;
	HANDLE h = CreateFile(
		_fullPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		);
	if (h == INVALID_HANDLE_VALUE) {
		err = GetPlatformErrorString(GetLastError());
		goto GetFileDateTime_End;
	}
	FILETIME created, modified;
	if (GetFileTime(h, &created, NULL, &modified) == 0) {
		err = GetPlatformErrorString(GetLastError());
		goto GetFileDateTime_End;
	}	
	created_ = FileTimeToDateTime(created);
	modified_ = FileTimeToDateTime(modified);
	
GetFileDateTime_End:
	if (err) {
		APT_LOG_ERR("GetFileDateTime: %s", err);
		APT_ASSERT(false);
	}
	APT_PLATFORM_VERIFY(CloseHandle(h));
	return err == nullptr;
#else
	const char* err = nullptr;
	WIN32_FILE_ATTRIBUTE_DATA attr;
	if (GetFileAttributesEx(_fullPath, GetFileExInfoStandard, &attr) == 0) {
		err = (const char*)GetPlatformErrorString(GetLastError());
		goto GetFileDateTime_End;
	}
	created_ = FileTimeToDateTime(attr.ftCreationTime);
	modified_ = FileTimeToDateTime(attr.ftLastWriteTime);
GetFileDateTime_End:
	if (err) {
		APT_LOG_ERR("GetFileDateTime: %s", err);
		APT_ASSERT(false);
	}
	return err == nullptr;
#endif
}

static void GetAppPath(TCHAR ret_[MAX_PATH], const char* _append = nullptr)
{
	TCHAR tmp[MAX_PATH];

	APT_PLATFORM_VERIFY(GetModuleFileName(0, tmp, MAX_PATH));
	APT_PLATFORM_VERIFY(GetFullPathName(tmp, MAX_PATH, ret_, NULL)); // GetModuleFileName can return a relative path (e.g. when launching from the ide)

	if (_append && *_append != '\0') {
		APT_PLATFORM_VERIFY(GetFullPathName(_append, MAX_PATH, tmp, NULL));
		char* pathEnd = strrchr(ret_, (int)'\\');
		++pathEnd;
		strcpy(pathEnd, _append);
	} else {
		char* pathEnd = strrchr(ret_, (int)'\\');
		++pathEnd;
		*pathEnd = '\0';
	}
}

// PUBLIC

bool FileSystem::Delete(const char* _path)
{
	if (DeleteFile(_path) == 0) {
		DWORD err = GetLastError();
		if (err != ERROR_FILE_NOT_FOUND) {
			APT_LOG_ERR("DeleteFile(%s): %s", _path, GetPlatformErrorString(err));
		}
		return false;
	}
	return true;
}

DateTime FileSystem::GetTimeCreated(const char* _path, RootType _rootHint)
{
	PathStr fullPath;
	if (!FindExisting(fullPath, _path, _rootHint)) {
		return DateTime(); // \todo return invalid sentinel
	}
	DateTime created, modified;
	GetFileDateTime((const char*)fullPath, created, modified);
	return created;
}

DateTime FileSystem::GetTimeModified(const char* _path, RootType _rootHint)
{
	PathStr fullPath;
	if (!FindExisting(fullPath, _path, _rootHint)) {
		return DateTime(); // \todo return invalid sentinel
	}
	DateTime created, modified;
	GetFileDateTime((const char*)fullPath, created, modified);
	return modified;
}

bool FileSystem::CreateDir(const char* _path)
{
	TextParser tp(_path);
	while (tp.advanceToNext("\\/") != 0) {
		String<64> mkdir;
		mkdir.set(_path, tp.getCharCount());
		if (CreateDirectory((const char*)mkdir, NULL) == 0) {
			DWORD err = GetLastError();
			if (err != ERROR_ALREADY_EXISTS) {
				APT_LOG_ERR("CreateDirectory(%s): %s", _path, GetPlatformErrorString(err));
				return false;
			}
		}
		tp.advance(); // skip the delimiter
	}
	return true;
}

PathStr FileSystem::MakeRelative(const char* _path, RootType _root)
{
	TCHAR root[MAX_PATH] = {};
	if (IsAbsolute((const char*)s_roots[_root])) {
		APT_PLATFORM_VERIFY(GetFullPathName((const char*)s_roots[_root], MAX_PATH, root, NULL));
	} else {
		GetAppPath(root, (const char*)s_roots[_root]);
	}

 // construct the full path
	TCHAR path[MAX_PATH] = {};
	APT_PLATFORM_VERIFY(GetFullPathName(_path, MAX_PATH, path, NULL));

	char tmpbuf[MAX_PATH];
	char* tmp = tmpbuf;
 // PathRelativePathTo will fail if tmp and root don't share a common prefix
	APT_VERIFY(PathRelativePathTo(tmp, root, FILE_ATTRIBUTE_DIRECTORY, path, PathIsDirectory(path) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL));
	if (strncmp(tmp, ".\\", 2) == 0) { // remove "./" from the path start - not strictly necessary but it makes nicer looking relative paths for the most common case
		tmp += 2;
	}	
	PathStr ret(tmp);
	ret.replace('\\', s_separator);
	return ret;
}

bool FileSystem::IsAbsolute(const char* _path)
{
	return PathIsRelative(_path) == FALSE;
}

PathStr FileSystem::StripRoot(const char* _path)
{
	TCHAR path[MAX_PATH] = {};
	APT_PLATFORM_VERIFY(GetFullPathName(_path, MAX_PATH, path, NULL));

	for (int r = 0; r < RootType_Count; ++r) {
		if (s_roots[r].isEmpty()) {
			continue;
		}
		TCHAR root[MAX_PATH];
		if (IsAbsolute((const char*)s_roots[r])) {
			APT_PLATFORM_VERIFY(GetFullPathName((const char*)s_roots[r], MAX_PATH, root, NULL));
		} else {
			GetAppPath(root, (const char*)s_roots[r]);
		}
		const char* rootBeg = strstr(path, root);
		if (rootBeg != nullptr) {
			PathStr ret(path + strlen(root) + 1);
			ret.replace('\\', s_separator);
			return ret;
		}
	}
 // no root found, strip the whole path if not absolute
	if (!IsAbsolute(_path)) {
		return StripPath(_path);
	}
	return _path;
}

bool FileSystem::PlatformSelect(PathStr& ret_, std::initializer_list<const char*> _filterList)
{
	static DWORD       s_filterIndex = 0;
	static const DWORD kMaxOutputLength = MAX_PATH;
	static char        s_output[kMaxOutputLength];

	PathStr filters;
	BuildFilterString(_filterList, filters);

	OPENFILENAMEA ofn   = {};
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = (LPSTR)filters;
	ofn.nFilterIndex    = s_filterIndex;
	ofn.lpstrInitialDir = (LPSTR)s_roots[RootType_Application];
	ofn.lpstrFile       = s_output;
	ofn.nMaxFile        = kMaxOutputLength;
	ofn.lpstrTitle      = "File";
	ofn.Flags           = OFN_DONTADDTORECENT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	if (GetOpenFileName(&ofn) != 0) {
		s_filterIndex = ofn.nFilterIndex;
	 // parse s_output into results_
		ret_.set(s_output);
		ret_.replace('\\', '/'); // sanitize path for display
		return true;
	} else {
		DWORD err = CommDlgExtendedError();
		if (err != 0) {
			APT_LOG_ERR("GetOpenFileName (0x%x)", err);
			APT_ASSERT(false);
		}		
	}
	return false;
}

int FileSystem::PlatformSelectMulti(PathStr retList_[], int _maxResults, std::initializer_list<const char*> _filterList)
{
	static DWORD       s_filterIndex = 0;
	static const DWORD kMaxOutputLength = 1024 * 4;
	static char        s_output[kMaxOutputLength];

	PathStr filters;
	BuildFilterString(_filterList, filters);

	OPENFILENAMEA ofn   = {};
	ofn.lStructSize     = sizeof(ofn);
	ofn.lpstrFilter     = (LPSTR)filters;
	ofn.nFilterIndex    = s_filterIndex;
	ofn.lpstrInitialDir = (LPSTR)s_roots[RootType_Application];
	ofn.lpstrFile       = s_output;
	ofn.nMaxFile        = kMaxOutputLength;	
	ofn.lpstrTitle      = "File";
	ofn.Flags           = OFN_ALLOWMULTISELECT | OFN_DONTADDTORECENT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
	if (GetOpenFileName(&ofn) != 0) {
		s_filterIndex = ofn.nFilterIndex;

	 // parse s_output into results_
		int ret = 0;
		TextParser tp(s_output);
		while (ret < _maxResults) {
			tp.advanceToNext('\0');
			tp.advance();
			if (*tp == '\0') {
				break;
			}
			retList_[ret].appendf("%s\\%s", s_output, (const char*)tp);
			retList_[ret].replace('\\', '/'); // sanitize path for display
			++ret;
		}
		return ret;

	} else {
		DWORD err = CommDlgExtendedError();
		if (err != 0) {
			APT_LOG_ERR("GetOpenFileName (0x%x)", err);
			APT_ASSERT(false);
		}		
	}
	return 0;
}

int FileSystem::ListFiles(PathStr retList_[], int _maxResults, const char* _path, std::initializer_list<const char*> _filterList, bool _recursive)
{
	eastl::vector<PathStr> dirs;
	dirs.push_back(_path);
	int ret = 0;
	while (!dirs.empty()) {
		PathStr root = (PathStr&&)dirs.back();
		dirs.pop_back();
		root.replace('/', '\\');
		PathStr search = root;
		search.appendf("\\*"); // ignore filter here, need to catch dirs for recursion

		WIN32_FIND_DATA ffd;
		HANDLE h = FindFirstFile((const char*)search, &ffd);
		if (h == INVALID_HANDLE_VALUE) {
			DWORD err = GetLastError();
			if (err != ERROR_FILE_NOT_FOUND) {
				APT_LOG_ERR("ListFiles (FindFirstFile): %s", GetPlatformErrorString(err));
			}
			continue;
		} 

		do {
			if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0) {
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (_recursive) {
						dirs.push_back(root);
						dirs.back().appendf("\\%s", ffd.cFileName);
					}
				} else {
					if (MatchesMulti(_filterList, (const char*)ffd.cFileName)) {
						if (ret < _maxResults) {
							retList_[ret].setf("%s\\%s", (const char*)root, ffd.cFileName);
						}
						++ret;
					}
				}
			}
	
        } while (FindNextFile(h, &ffd) != 0);

		DWORD err = GetLastError();
		if (err != ERROR_NO_MORE_FILES) {
			APT_LOG_ERR("ListFiles (FindNextFile): %s", GetPlatformErrorString(err));
		}

		FindClose(h);
    }

	return ret;
}

int FileSystem::ListDirs(PathStr retList_[], int _maxResults, const char* _path, std::initializer_list<const char*> _filterList, bool _recursive)
{
	eastl::vector<PathStr> dirs;
	dirs.push_back(_path);
	int ret = 0;
	// There are 2 choices of behavior here: 'direct' recursion (where sub directories appear immediately after the parent in the list), or 'deferred' 
	// recursion, which is what is implemented. In theory, the latter is better because you can fill a small list of the first couple of levels of 
	// the hierarchy and then manually recurse into those directories as needed.
	while (!dirs.empty()) {
		PathStr root = (PathStr&&)dirs.back();
		dirs.pop_back();
		root.replace('/', '\\');
		PathStr search = root;
		search.appendf("\\*");
	
		WIN32_FIND_DATA ffd;
		HANDLE h = FindFirstFile((const char*)search, &ffd);
		if (h == INVALID_HANDLE_VALUE) {
			DWORD err = GetLastError();
			if (err != ERROR_FILE_NOT_FOUND) {
				APT_LOG_ERR("ListFiles (FindFirstFile): %s", GetPlatformErrorString(err));
			}
			continue;
		}

		do {
			if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0) {
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (_recursive) {
						dirs.push_back(root);
						dirs.back().appendf("\\%s", ffd.cFileName);
					}
					if (MatchesMulti(_filterList, (const char*)ffd.cFileName)) {
						if (ret < _maxResults) {
							retList_[ret].setf("%s\\%s", (const char*)root, ffd.cFileName);
						}
						++ret;
					}
				}
			}
        } while (FindNextFile(h, &ffd) != 0);

		DWORD err = GetLastError();
		if (err != ERROR_NO_MORE_FILES) {
			APT_LOG_ERR("ListFiles (FindNextFile): %s", GetPlatformErrorString(err));
		}

		FindClose(h);
    }

	return ret;
}


namespace {
/* Notes:
	- Changes within symbolic link subdirs don't generate events.
	- Deleting a subdir doesn't generate events for its subtree.
	- Duplicate 'modified' actions are received consecutively but may be split over 2 calls to DispatchNotifications(),
	  hence store the last received action inside the watch struct (the queue gets cleared, so can't use queue.back()).
*/
	struct Watch
	{
		OVERLAPPED m_overlapped = {};
		HANDLE     m_hDir       = INVALID_HANDLE_VALUE;
		DWORD      m_filter     = 0;
		UINT       m_bufSize    = 1024 * 32; // 32kb
		BYTE*      m_buf        = NULL;

		eastl::pair<PathStr, FileSystem::FileAction> m_prevAction;
		FileSystem::FileActionCallback* m_dispatchCallback;
		eastl::vector<eastl::pair<PathStr, FileSystem::FileAction> > m_dispatchQueue;
	};
	static Pool<Watch> s_WatchPool(8);
	static eastl::vector_map<StringHash, Watch*> s_WatchMap;

	void CALLBACK WatchCompletion(DWORD _err, DWORD _bytes, LPOVERLAPPED _overlapped);
	void          WatchUpdate(Watch* _watch);


	void CALLBACK WatchCompletion(DWORD _err, DWORD _bytes, LPOVERLAPPED _overlapped)
	{
		if (_err == ERROR_OPERATION_ABORTED) { // CancellIo was called
			return;
		}
		APT_ASSERT(_err == ERROR_SUCCESS);
		APT_ASSERT(_bytes != 0); // overflow? notifications lost in this case?

		Watch* watch = (Watch*)_overlapped; // m_overlapped is the first member, so this works

		TCHAR fileName[MAX_PATH];
		for (DWORD off = 0;;) {
			PFILE_NOTIFY_INFORMATION info = (PFILE_NOTIFY_INFORMATION)(watch->m_buf + off);		
			off += info->NextEntryOffset;

		 // unicode -> utf8
			int count = WideCharToMultiByte(CP_UTF8, 0, info->FileName, info->FileNameLength / sizeof(WCHAR), fileName, MAX_PATH - 1, NULL, NULL);
			fileName[count] = '\0';

			FileSystem::FileAction action = FileSystem::FileAction_Count;
			switch (info->Action) {
				case FILE_ACTION_ADDED: 
				case FILE_ACTION_RENAMED_NEW_NAME:
					action = FileSystem::FileAction_Created; 
					break;
				case FILE_ACTION_REMOVED:
				case FILE_ACTION_RENAMED_OLD_NAME:
					action = FileSystem::FileAction_Deleted; 
					break;
				case FILE_ACTION_MODIFIED:
				default:
					action = FileSystem::FileAction_Modified;
					break;
			};

		 // check to see if the action was duplicated - this happens often for FILE_ACTION_MODIFIED
			bool duplicate = false;
			auto& prev = watch->m_prevAction;
			if (prev.second == action) { // action matches
				if (prev.first.getLength() == count) { // path length matches
					if (memcmp(prev.first.c_str(), fileName, count) == 0) { // compare the strings
						duplicate = true;
					}
				}
			}
			if (!duplicate) {
				watch->m_prevAction = eastl::make_pair(PathStr(fileName), action);
				watch->m_prevAction.first.replace('\\', '/');
				watch->m_dispatchQueue.push_back(watch->m_prevAction);
			}
			
			if (info->NextEntryOffset == 0) {
				break;
			}
		}

	 // reissue ReadDirectoryChangesW; it seems that we don't actually miss any notifications which happen between the start of the completion routine
	 // and the reissue, so it's safe to wait until the dispatch is done
		WatchUpdate(watch);
	}

	void WatchUpdate(Watch* _watch)
	{
		APT_PLATFORM_VERIFY(ReadDirectoryChangesW(
			_watch->m_hDir,
			_watch->m_buf,
			_watch->m_bufSize,
			TRUE, // watch subtree
			_watch->m_filter,
			NULL,
 			&_watch->m_overlapped, 
			WatchCompletion
			));
	}
}

void FileSystem::BeginNotifications(const char* _dir, FileActionCallback* _callback)
{
	StringHash dirHash(_dir);
	if (s_WatchMap.find(dirHash) != s_WatchMap.end()) {
		APT_ASSERT(false);
		return;
	}

	CreateDirectoryA(_dir, NULL); // create if it doesn't already exist

	Watch* watch = s_WatchPool.alloc();
	watch->m_hDir = CreateFileA(
		_dir,                                                    // path
		FILE_LIST_DIRECTORY,                                     // desired access
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,  // share mode
		NULL,                                                    // security attribs
		OPEN_EXISTING,                                           // create mode
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,       // file attribs
		NULL                                                     // template handle
		);
	APT_PLATFORM_ASSERT(watch->m_hDir != INVALID_HANDLE_VALUE);

	s_WatchMap[dirHash] = watch;
	watch->m_buf = (BYTE*)APT_MALLOC_ALIGNED(watch->m_bufSize, sizeof(DWORD));
	watch->m_filter = 0
			| FILE_NOTIFY_CHANGE_CREATION
			| FILE_NOTIFY_CHANGE_SIZE
			| FILE_NOTIFY_CHANGE_ATTRIBUTES 
			| FILE_NOTIFY_CHANGE_FILE_NAME 
			| FILE_NOTIFY_CHANGE_DIR_NAME 
			;
	watch->m_dispatchCallback = _callback;
	WatchUpdate(watch);
}

void FileSystem::EndNotifications(const char* _dir)
{
	StringHash dirHash(_dir);
	auto it = s_WatchMap.find(dirHash);
	if (it == s_WatchMap.end()) {
		APT_ASSERT(false);
		return;
	}
	auto watch = it->second;
	APT_PLATFORM_VERIFY(CancelIo(watch->m_hDir));
	SleepEx(0, TRUE); // flush any pending calls to the completion routine
	APT_PLATFORM_VERIFY(CloseHandle(watch->m_hDir));
	APT_FREE_ALIGNED(watch->m_buf);
	s_WatchPool.free(watch);
	s_WatchMap.erase(it);
}

void FileSystem::DispatchNotifications(const char* _dir)
{
 // clear 'prevAction' - identical consecutive actions *between* calls to DispatchNotifications are allowed
	if (_dir) {
		auto it = s_WatchMap.find(StringHash(_dir));
		if (it == s_WatchMap.end()) {
			APT_ASSERT(false);
			return;
		}
		it->second->m_prevAction.second = FileAction_Count;

	} else {
		for (auto& it : s_WatchMap) {
			it.second->m_prevAction.second = FileAction_Count;
		}
	}

 // let the kernel call the completion routine and fill the dispatch queues
	SleepEx(0, TRUE);

 // dispatch
	if (_dir) {
		auto it = s_WatchMap.find(StringHash(_dir));
		Watch& watch = *it->second;
		for (auto& file : watch.m_dispatchQueue) {
			watch.m_dispatchCallback(file.first.c_str(), file.second);
		}
		watch.m_dispatchQueue.clear();

	} else {
		for (auto& it : s_WatchMap) {
			Watch& watch = *it.second;
			for (auto& file : watch.m_dispatchQueue) {
				watch.m_dispatchCallback(file.first.c_str(), file.second);
			}
			watch.m_dispatchQueue.clear();
		}
	}
}

// PROTECTED

const char FileSystem::s_separator = '/';
