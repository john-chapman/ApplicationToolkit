#include <apt/FileSystem.h>

#include <apt/log.h>
#include <apt/File.h>
#include <apt/String.h>

#include <cstring>

using namespace apt;

// PUBLIC

const char* FileSystem::GetRoot(RootType _type)
{
	return (const char*)s_roots[_type];
}

void FileSystem::SetRoot(RootType _type, const char* _path)
{
	s_roots[_type].set(_path);
}

bool FileSystem::Read(File& file_, const char* _path, RootType _rootHint)
{
	PathStr fullPath;
	if (!FindExisting(fullPath, _path ? _path : file_.getPath(), _rootHint)) {
		APT_LOG_ERR("Error loading '%s':\n\tFile not found", _path);
		//APT_ASSERT(false);
		return false;
	}
	return File::Read(file_, (const char*)fullPath);
}

bool FileSystem::ReadIfExists(File& file_, const char* _path, RootType _rootHint)
{
	PathStr fullPath;
	if (!FindExisting(fullPath, _path ? _path : file_.getPath(), _rootHint)) {
		return false;
	}
	return File::Read(file_, (const char*)fullPath);
}

bool FileSystem::Write(const File& _file, const char* _path, RootType _root)
{
	PathStr fullPath = MakePath(_path ? _path : _file.getPath(), _root);
	return File::Write(_file, (const char*)fullPath);
}

bool FileSystem::Exists(const char* _path, RootType _rootHint)
{
	PathStr buf;
	return FindExisting(buf, _path, _rootHint);
}

bool FileSystem::Matches(const char* _pattern, const char* _str)
{
// based on https://research.swtch.com/glob
	const size_t plen = strlen(_pattern);
	const size_t nlen = strlen(_str);
	size_t px = 0;
	size_t nx = 0;
	size_t nextPx = 0;
	size_t nextNx = 0;
	while (px < plen || nx < nlen) {
		if (px < plen) {
			char c = _pattern[px];
			switch (c) {
				case '?': // match 1 char
					if (nx < nlen) {
						px++;
						nx++;
						continue;
					}
					break;
					
				case '*': // match 0 or more char
				// try to match at nx, else restart at nx+1 next.
					nextPx = px;
					nextNx = nx + 1;
					px++;
					continue;
				
				default: // non-wildcard
					if (nx < nlen && _str[nx] == c) {
						px++;
						nx++;
						continue;
					}
					break;
			};
		}

		if (0 < nextNx && nextNx <= nlen) {
			px = nextPx;
			nx = nextNx;
			continue;
		}
		return false;
    }

    return true;
}

bool FileSystem::MatchesMulti(std::initializer_list<const char*> _patternList, const char* _str)
{
	for (auto& pattern : _patternList) {
		if (Matches(pattern, _str)) {
			return true;
		}
	}
	return false;
}

PathStr FileSystem::MakePath(const char* _path, RootType _root)
{
	APT_ASSERT(_root < RootType_Count);
	bool useRoot = !s_roots[_root].isEmpty() && !IsAbsolute(_path);
	if (useRoot) {
	 // check if the root already exists in path as a directory
		const char* r = strstr((const char*)s_roots[_root], _path);
		if (!r || *(r + s_roots[_root].getLength()) != s_separator) {
			return PathStr("%s%c%s", (const char*)s_roots[_root], s_separator, _path);
		}
	}
	return PathStr(_path);
}

PathStr FileSystem::StripPath(const char* _path)
{
	const char* beg = _path;
	while (*_path) {
		if (*_path == '/' || *_path == '\\') {
			beg = _path + 1;
		}
		++_path;
	}
	return PathStr(beg);
}

PathStr FileSystem::GetPath(const char* _path)
{
	const char* beg = _path;
	const char* end = _path;
	while (*_path) {
		if (*_path == '/' || *_path == '\\') {
			end = _path + 1;
		}
		++_path;
	}
	PathStr ret;
	ret.set(beg, end - beg);
	return ret;
}

PathStr FileSystem::GetFileName(const char* _path)
{
	const char* beg = _path;
	while (*_path) {
		if (*_path == '/' || *_path == '\\') {
			beg = _path + 1;
		}
		++_path;
	}
	const char* end = beg;
	while (*end && *end != '.') {
		++end;
	}
	PathStr ret;
	ret.set(beg, end - beg);
	return ret;
}

const char* FileSystem::FindExtension(const char* _path)
{
	const char* ret = strrchr(_path, (int)'.');
	return ret == nullptr ? nullptr : ret + 1;
}

bool FileSystem::CompareExtension(const char* _ext, const char* _path)
{
	if (*_ext == '.') {
		++_ext;
	}
	const char* cmp = FindExtension(_path);
	if (cmp) {
		return _stricmp(_ext, cmp) == 0;
	}
	return false;
}

const char* FileSystem::FindFileNameAndExtension(const char* _path)
{
	const char* ret = _path;
	while (*_path) {
		if (*_path == '/' || *_path == '\\') {
			ret = _path + 1;
		}
		++_path;
	}
	return ret;
}

// PRIVATE

PathStr FileSystem::s_roots[RootType_Count];

bool FileSystem::FindExisting(PathStr& ret_, const char* _path, RootType _rootHint)
{
	for (int r = (int)_rootHint; r != -1; --r) {
		ret_ = MakePath(_path, (RootType)r);
		if (File::Exists((const char*)ret_)) {
			return true;
		}
	}
	return false;
}
