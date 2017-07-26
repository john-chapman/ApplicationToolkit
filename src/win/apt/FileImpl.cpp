#include <apt/File.h>

#include <apt/log.h>
#include <apt/platform.h>
#include <apt/win.h>
#include <apt/FileSystem.h>
#include <apt/String.h>
#include <apt/TextParser.h>

#include <cstdlib> // malloc, free
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

	bool ret = false;
	const char* err = "";
	char* data = 0;

 	HANDLE h = CreateFile(
		_path,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (h == INVALID_HANDLE_VALUE) {
		err = GetPlatformErrorString(GetLastError());
		goto File_Read_end;
	}

	LARGE_INTEGER li;
	if (!GetFileSizeEx(h, &li)) {
		err = GetPlatformErrorString(GetLastError());
		goto File_Read_end;
	}
	DWORD dataSize = (DWORD)li.QuadPart; // ReadFile can only read DWORD bytes

	data = (char*)malloc(dataSize + 2); // +2 for null terminator
	APT_ASSERT(data);
	DWORD bytesRead;
	if (!ReadFile(h, data, dataSize, &bytesRead, 0)) {
		err = GetPlatformErrorString(GetLastError());
		goto File_Read_end;
	}
	data[dataSize] = data[dataSize + 1] = 0;

	ret = true;
	
  // close existing handle/free existing data
	if ((HANDLE)file_.m_impl != INVALID_HANDLE_VALUE) {
		APT_PLATFORM_VERIFY(CloseHandle((HANDLE)file_.m_impl));
	}
	if (file_.m_data) {
		free(file_.m_data);
	}
	
	file_.m_data = data;
	file_.m_dataSize = dataSize;
	file_.setPath(_path);

File_Read_end:
	if (!ret) {
		if (data) {
			free(data);
		}
		APT_LOG_ERR("Error reading '%s':\n\t%s", _path, err);
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

	bool ret = false;
	const char* errstr = "";
	char* data = 0;
	
 	HANDLE h = CreateFile(
		_path,
		GENERIC_WRITE,
		0, // prevent sharing while we write
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if (h == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (err == ERROR_PATH_NOT_FOUND) {
			if (FileSystem::CreateDir(_path)) {
				return Write(_file, _path);
			} else {
				return false;
			}
		} else {
			errstr = GetPlatformErrorString(err);
			goto File_Write_end;
		}
	}

	DWORD bytesWritten;
	if (!WriteFile(h, _file.getData(), (DWORD)_file.getDataSize(), &bytesWritten, NULL)) {
		errstr = GetPlatformErrorString(GetLastError());
		goto File_Write_end;
	}
	APT_ASSERT(bytesWritten == _file.getDataSize());

	ret = true;

File_Write_end:
	if (!ret) {
		APT_LOG_ERR("Error writing '%s':\n\t%s", _path, errstr);
		APT_ASSERT(false);
	}
	if (h != INVALID_HANDLE_VALUE) {
		APT_PLATFORM_VERIFY(CloseHandle(h));
	}
	return ret;
}