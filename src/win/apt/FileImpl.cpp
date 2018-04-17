#include <apt/File.h>

#include <apt/log.h>
#include <apt/memory.h>
#include <apt/platform.h>
#include <apt/win.h>
#include <apt/FileSystem.h>
#include <apt/String.h>
#include <apt/TextParser.h>

#include <utility> // swap

using namespace apt;

File::File()
{
	ctorCommon();
	m_impl = INVALID_HANDLE_VALUE;
}

File::~File()
{
	dtorCommon();
	if ((HANDLE)m_impl != INVALID_HANDLE_VALUE) {
		APT_PLATFORM_VERIFY(CloseHandle((HANDLE)m_impl));
	}
}

bool File::Exists(const char* _path)
{
	return GetFileAttributes(_path) != INVALID_FILE_ATTRIBUTES;
}

bool File::Read(File& file_, const char* _path)
{
	if (!_path) {
		_path = file_.getPath();
	}
	APT_ASSERT(_path);

	bool  ret      = false;
	char* data     = nullptr;
	DWORD err      = 0;
	int   tryCount = 3; // avoid sharing violations, especially when loading a file after a file change notification

 	HANDLE h = INVALID_HANDLE_VALUE;
	do {
		h = CreateFile(
			_path,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
			);
		if (h == INVALID_HANDLE_VALUE) {
			err = GetLastError();
			if (err == ERROR_SHARING_VIOLATION && tryCount > 0) {
				APT_LOG_DBG("Sharing violation reading '%s', retrying...", _path);
				Sleep(1);
				--tryCount;
			} else {
				goto File_Read_end;
			}
		}
	} while (h == INVALID_HANDLE_VALUE);
	
	LARGE_INTEGER li;
	if (!GetFileSizeEx(h, &li)) {
		err = GetLastError();
		goto File_Read_end;
	}
	DWORD dataSize = (DWORD)li.QuadPart; // ReadFile can only read DWORD bytes

	data = (char*)APT_MALLOC(dataSize + 2); // +2 for null terminator
	APT_ASSERT(data);
	DWORD bytesRead;
	if (!ReadFile(h, data, dataSize, &bytesRead, 0)) {
		err = GetLastError();
		goto File_Read_end;
	}
	data[dataSize] = data[dataSize + 1] = 0;

	ret = true;
	
  // close existing handle/free existing data
	if ((HANDLE)file_.m_impl != INVALID_HANDLE_VALUE) {
		APT_PLATFORM_VERIFY(CloseHandle((HANDLE)file_.m_impl));
	}
	if (file_.m_data) {
		APT_FREE(file_.m_data);
	}
	
	file_.m_data     = data;
	file_.m_dataSize = dataSize;
	file_.setPath(_path);

File_Read_end:
	if (!ret) {
		if (data) {
			APT_FREE(data);
		}
		APT_LOG_ERR("Error reading '%s':\n\t%s", _path, GetPlatformErrorString((uint64)err));
		APT_ASSERT(false);
	}
	if (h != INVALID_HANDLE_VALUE) {
		APT_PLATFORM_VERIFY(CloseHandle(h));
	}
	return ret;
}

bool File::Write(const File& _file, const char* _path)
{
	if (!_path) {
		_path = _file.getPath();
	}
	APT_ASSERT(_path);

	bool  ret  = false;
	char* data = nullptr;
	DWORD err  = 0;
	
 	HANDLE h = CreateFile(
		_path,
		GENERIC_WRITE,
		0, // no sharing during write
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (h == INVALID_HANDLE_VALUE) {
		err = GetLastError();
		if (err == ERROR_PATH_NOT_FOUND) {
			if (FileSystem::CreateDir(_path)) {
				return Write(_file, _path);
			} else {
				return false;
			}
		} else {
			goto File_Write_end;
		}
	}

	DWORD bytesWritten;
	if (!WriteFile(h, _file.getData(), (DWORD)_file.getDataSize(), &bytesWritten, NULL)) {
		goto File_Write_end;
	}
	APT_ASSERT(bytesWritten == _file.getDataSize());

	ret = true;

File_Write_end:
	if (!ret) {
		APT_LOG_ERR("Error writing '%s':\n\t%s", _path, GetPlatformErrorString((uint64)err));
		APT_ASSERT(false);
	}
	if (h != INVALID_HANDLE_VALUE) {
		APT_PLATFORM_VERIFY(CloseHandle(h));
	}
	return ret;
}