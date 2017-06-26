#pragma once
#ifndef apt_FileSystem_h
#define apt_FileSystem_h

#include <apt/def.h>
#include <apt/File.h>
#include <apt/String.h>

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// FileSystem
// Manage file access. A fixed number of directory 'roots' may be set, which 
// are searched in reverse order when resolving a relative path (e.g. 
// RootType_Application is checked before RootType_Common).
////////////////////////////////////////////////////////////////////////////////
class FileSystem : private non_instantiable<FileSystem>
{
public:
	typedef File::PathStr PathStr;

	enum RootType
	{
		RootType_Root,         // By default the working directory of the executable.
		RootType_Common,       // Shared path for common files.
		RootType_Application,  // Application-specific files.

		RootType_Count,
		RootType_Default = RootType_Application
	};

	// Get/set a path for the specified root type.
	static const char* GetRoot(RootType _type);
	static void        SetRoot(RootType _type, const char* _path);


	// Read a file into memory. Each root is searched, beginning at _rootHint. If _path is nullptr, the 
	// path from file_ is used. Return false if an error occurred, in which case file_ remains unchanged. 
	// On success, any resources already associated with file_ are released. _rootHint is ignored if _path is absolute.
	static bool        Read(File& file_, const char* _path = nullptr, RootType _rootHint = RootType_Default);

	// As Read() but first checks if the file exists. Return false if the file does not exist or if an error occurred.
	static bool        ReadIfExists(File& file_, const char* _path = nullptr, RootType _rootHint = RootType_Default);

	// Write _file's data to _path. If _path is nullptr, the path from _file is used. Return false 
	// if an error occurred, in which case any existing file at _path may or may not have been overwritten.
	// _root is ignored if _path is absolute.
	static bool        Write(const File& _file, const char* _path = nullptr, RootType _root = RootType_Default);

	// Return true if _path exists. Each root is searched, beginning at _rootHint.
	static bool        Exists(const char* _path, RootType _rootHint = RootType_Default);

	// Delete a file.
	static bool        Delete(const char* _path);

	// Concatenates _path + s_separator + s_root[_root]. If _path is absolute the root is ignored.
	static void        MakePath(StringBase& ret_, const char* _path, RootType _root);

	// Make _path relative to _root. It is safe for _path to point to the string buffer in ret_.
	static void        MakeRelative(StringBase& ret_, const char* _path, RootType _root = RootType_Root);
	static void        MakeRelative(StringBase& ret_, RootType _root = RootType_Root) { MakeRelative(ret_, ret_, _root); }
	
	// Return true if _path is absolute.
	static bool        IsAbsolute(const char* _path);

	// Strip path from _path up to and including any root directory. It is safe for _path to point to 
	// the string buffer in ret_.
	static void        StripRoot(StringBase& ret_, const char* _path);
	static void        StripRoot(StringBase& ret_) { StripRoot(ret_, ret_); }
	// Strip path from _path. It is safe for _path to point to the string buffer in ret_.
	static void        StripPath(StringBase& ret_, const char* _path);
	static void        StripPath(StringBase& ret_) { StripPath(ret_, ret_); }

	// Extract path from _path (remove file name + extension). It is safe for _path to point
	// the string buffer in ret_.
	static void        GetPath(StringBase& ret_, const char* _path);
	static void        GetPath(StringBase& ret_) { GetFileName(ret_, ret_); }
	// Extract file name from _path. It is safe for _path to point to the string buffer in ret_.
	static void        GetFileName(StringBase& ret_, const char* _path);
	static void        GetFileName(StringBase& ret_) { GetFileName(ret_, ret_); }
	// Extract extension from _path. It is safe for _path to point to the string buffer in ret_.
	static void        GetExtension(StringBase& ret_, const char* _path) { ret_.set(FindExtension(_path)); }
	static void        GetExtension(StringBase& ret_) { GetExtension(ret_, ret_); }

	// Return ptr to the character following the last occurrence of '.' in _path.
	static const char* FindExtension(const char* _path);
	// Compare _ext with the extension from _path (case insensitive).
	static bool        CompareExtension(const char* _ext, const char* _path);
	
	// Select a file/files via the platform UI. _filters is a null-separated list of filter strings.
	static bool        PlatformSelect(PathStr& ret_, const char* _filters = "");
	static int         PlatformSelectMulti(PathStr retList_[], int _maxResults, const char* _filters = "");

	// List up to _maxResults files in _path with option recursion.
	static int         ListFiles(PathStr retList_[], int _maxResults, const char* _path, const char* _filter = "*.*", bool _recursive = false);

	// Create the directory specified by _path, plus all parent directories if they do not exist. Return false if an error occurred.
	// \note If _path contains only directory names, it must end in a path delimiter (e.g. "dir0/dir1/").
	static bool        CreateDir(const char* _path);

private:
	static PathStr    s_roots[RootType_Count];
	static int        s_rootLengths[RootType_Count];
	static const char s_separator; // per-platform default separator
	
	// Get a path to an existing file based on _path and _rootHint. Return false if no existing file was found.
	static bool FindExisting(PathStr& ret_, const char* _path, RootType _rootHint);

}; // class FileSystem

} // namespace apt

#endif // apt_FileSystem_h
