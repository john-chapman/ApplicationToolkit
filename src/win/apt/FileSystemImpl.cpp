#include <apt/FileSystem.h>

#include <apt/log.h>
#include <apt/platform.h>
#include <apt/win.h>
#include <apt/String.h>
#include <apt/TextParser.h>

#include <Shlwapi.h>
#include <commdlg.h>
#include <cstring>

#include <EASTL/vector.h>

#pragma comment(lib, "shlwapi")

using namespace apt;

// Windows expects a list of string pairs to pass to GetOpenFileName().
static void BuildFilterString(const char* _filters, StringBase& ret_)
{
	TextParser tp = _filters;
	while (*tp != '\0') {
		ret_.appendf("%s#%s#", (const char*)tp, (const char*)tp);
		tp.advanceToNext('\0');
		tp.advance();
	}
	ret_.replace('#', '\0'); // \hack
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
	const char* err = nullptr;
	HANDLE h = CreateFile(
		_fullPath,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
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
	GetFileDateTime(fullPath, created, modified);
	return created;
}

DateTime FileSystem::GetTimeModified(const char* _path, RootType _rootHint)
{
	PathStr fullPath;
	if (!FindExisting(fullPath, _path, _rootHint)) {
		return DateTime(); // \todo return invalid sentinel
	}
	DateTime created, modified;
	GetFileDateTime(fullPath, created, modified);
	return modified;
}

void FileSystem::MakeRelative(StringBase& ret_, const char* _path, RootType _root)
{
 // construct the full root
	TCHAR root[MAX_PATH] = {};
	{
		TCHAR tmp[MAX_PATH] = {};
		APT_PLATFORM_VERIFY(GetModuleFileName(0, tmp, MAX_PATH));
		APT_PLATFORM_VERIFY(GetFullPathName(tmp, MAX_PATH, root, NULL)); // required to resolve a relative path (e.g. when launching from the ide)
		char* pathEnd = strrchr(root, (int)'\\');
		++pathEnd;
		strcpy(pathEnd, s_roots[_root]);
	}
 // construct the full path
	TCHAR path[MAX_PATH] = {};
	APT_PLATFORM_VERIFY(GetFullPathName(_path, MAX_PATH, path, NULL));

	char tmpbuf[MAX_PATH];
	char* tmp = tmpbuf;
	APT_ASSERT(PathRelativePathTo(tmp, root, FILE_ATTRIBUTE_DIRECTORY, path, PathIsDirectory(path) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL));
	if (strncmp(tmp, ".\\", 2) == 0) { // remove "./" from the path start - not strictly necessary but it makes nicer looking relative paths for the most common case
		tmp += 2;
	}	
	ret_.set(tmp);
	ret_.replace('\\', s_separator);
}

bool FileSystem::IsAbsolute(const char* _path)
{
	return PathIsRelative(_path) == FALSE;
}

bool FileSystem::PlatformSelect(PathStr& ret_, const char* _filters)
{
	static DWORD       s_filterIndex = 0;
	static const DWORD kMaxOutputLength = MAX_PATH;
	static char        s_output[kMaxOutputLength];

	PathStr filters;
	BuildFilterString(_filters, filters);

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

int FileSystem::PlatformSelectMulti(PathStr retList_[], int _maxResults, const char* _filters)
{
	static DWORD       s_filterIndex = 0;
	static const DWORD kMaxOutputLength = 1024 * 4;
	static char        s_output[kMaxOutputLength];

	PathStr filters;
	BuildFilterString(_filters, filters);

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

int FileSystem::ListFiles(PathStr retList_[], int _maxResults, const char* _path, const char* _filter, bool _recursive)
{
	eastl::vector<PathStr> dirs;
	dirs.push_back(_path);
	int ret = 0;
	while (ret < _maxResults && !dirs.empty()) {
		PathStr root = (PathStr&&)dirs.back();
		dirs.pop_back();
		root.replace('/', '\\');
		PathStr search = root;
		search.appendf("\\%s", _filter); // \todo check if / or \\ already at the end, same for the dir code below

		WIN32_FIND_DATA ffd;
		HANDLE h = FindFirstFile((const char*)search, &ffd);
		if (h == INVALID_HANDLE_VALUE) {
			APT_LOG_ERR("ListFiles (FindFirstFile): %s", GetPlatformErrorString(GetLastError()));
			continue;
		} 

		do {
			if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0) {
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (_recursive) {
						dirs.push_back(root);
						dirs.back().appendf("\\%s", ffd.cFileName, _filter);
					}
				} else {
					retList_[ret++].setf("%s\\%s", (const char*)root, ffd.cFileName);
				}
			}
	
        } while (ret < _maxResults && FindNextFile(h, &ffd) != 0);

		DWORD err = GetLastError();
		if (err != ERROR_NO_MORE_FILES) {
			APT_LOG_ERR("ListFiles (FindNextFile): %s", GetPlatformErrorString(err));
		}

		FindClose(h);
    }

	return ret;
}

bool FileSystem::CreateDir(const char* _path)
{
	TextParser tp(_path);
	while (tp.advanceToNext("\\/") != 0) {
		String<64> mkdir;
		mkdir.set(_path, tp.getCharCount());
		if (CreateDirectory(mkdir, NULL) == 0) {
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

// PROTECTED

const char FileSystem::s_separator = '/';
