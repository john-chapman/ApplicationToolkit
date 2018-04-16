#pragma once

#include <apt/apt.h>
#include <apt/File.h>
#include <apt/String.h>
#include <apt/Time.h>

#include <initializer_list>

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// FileSystem
// Manage file access. 
// A fixed number of directory 'roots' may be set, which are searched in reverse 
// order when resolving a relative path (e.g. RootType_Application is checked 
// before RootType_Common).
////////////////////////////////////////////////////////////////////////////////
class FileSystem
{
public:
	enum RootType_
	{
		RootType_Root,         // Default is the working directory of the executable.
		RootType_Common,       // Shared path for common files.
		RootType_Application,  // Application-specific files.

		RootType_Count,
		RootType_Default = RootType_Application
	};
	typedef int RootType;

	// Get/set a path for the specified root type.
	static const char* GetRoot(RootType _type);
	static void        SetRoot(RootType _type, const char* _path);

 // File operations

	// Read a file into memory. Each root is searched, beginning at _rootHint. If _path is nullptr, file_.getPath() is used.
	// Return false if an error occurred, in which case file_ remains unchanged. On success, any resources already associated 
	// with file_ are released. _rootHint is ignored if _path is absolute.
	static bool        Read(File& file_, const char* _path = nullptr, RootType _rootHint = RootType_Default);

	// As Read() but first checks if the file exists. Return false if the file does not exist or if an error occurred.
	static bool        ReadIfExists(File& file_, const char* _path = nullptr, RootType _rootHint = RootType_Default);

	// Write _file's data to _path. If _path is 0, _file.getPath() is used. Return false if an error occurred, in which case 
	// any existing file at _path may or may not have been overwritten. _root is ignored if _path is absolute.
	static bool        Write(const File& _file, const char* _path = nullptr, RootType _root = RootType_Default);

	// Return true if _path exists. Each root is searched, beginning at _rootHint.
	static bool        Exists(const char* _path, RootType _rootHint = RootType_Default);

	// Delete a file.
	static bool        Delete(const char* _path);

	// Get the creation/last modified time for a file. The path is constructed as per Read(). 
	static DateTime    GetTimeCreated(const char* _path, RootType _rootHint = RootType_Default);
	static DateTime    GetTimeModified(const char* _path, RootType _rootHint = RootType_Default);

	// Create the directory specified by _path, plus all parent directories if they do not exist. Return false if an error occurred.
	// If _path contains only directory names, it must end in a path separator (e.g. "dir0/dir1/").
	static bool        CreateDir(const char* _path);

 // Path manipulation

	// Concatenate _path + s_separator + s_root[_root]. _root is ignored if _path is absolute.
	static PathStr     MakePath(const char* _path, RootType _root);

	// Match _str against _pattern with wildcard characters: '?' matches a single character, '*' matches zero or more characters.
	static bool        Matches(const char* _pattern, const char* _str);
	// Call Matches() for each of a list of patterns e.g. { "*.txt", "*.png" }.	
	static bool        MatchesMulti(std::initializer_list<const char*> _patternList, const char* _str);

	// Make _path relative to _root.
	static PathStr     MakeRelative(const char* _path, RootType _root = RootType_Root);
	// Return true if _path is absolute.
	static bool        IsAbsolute(const char* _path);

	// Strip any root from _path, or the whole path if _path is absolute.
	static PathStr     StripRoot(const char* _path);
	// Strip path from _path.
	static PathStr     StripPath(const char* _path);

	// Extract path from _path (remove file name + extension).
	static PathStr     GetPath(const char* _path);
	// Extract file name from _path (remove path + extension).
	static PathStr     GetFileName(const char* _path);
	// Extract extension from _path (remove path + file name).
	static PathStr     GetExtension(const char* _path) { return PathStr(FindExtension(_path)); }

	// Return ptr to the character following the last occurrence of '.' in _path.
	static const char* FindExtension(const char* _path);
	// Return ptr to the character following the last occurrence of '\' or '/' in _path.
	static const char* FindFileNameAndExtension(const char* _path);
	// Compare _ext with the extension from _path (case insensitive).
	static bool        CompareExtension(const char* _ext, const char* _path);
	
	// Select a file/files via the platform UI.
	static bool        PlatformSelect(PathStr& ret_, std::initializer_list<const char*> _filterList = { "*" });
	static int         PlatformSelectMulti(PathStr retList_[], int _maxResults, std::initializer_list<const char*> _filterList = { "*" });

 // Inspection

	// List up to _maxResults files in _path, with optional recursion. Return the number of files which would be found if not limited by _maxResults.
	static int         ListFiles(PathStr retList_[], int _maxResults, const char* _path, std::initializer_list<const char*> _filterList = { "*" }, bool _recursive = false);
	// List up to _maxResults dirs in _path, with optional recursion. _filters is a null-separated list of filter strings. Return the number of dirs which would be found if not limited by _maxResults.
	static int         ListDirs(PathStr retList_[], int _maxResults, const char* _path, std::initializer_list<const char*> _filterList= { "*" }, bool _recursive = false);


 // File action notifications

	enum FileAction_
	{
		FileAction_Created,
		FileAction_Deleted,
		FileAction_Modified,

		FileAction_Count
	};
	typedef int FileAction;

	typedef void (FileActionCallback)(const char* _path, FileAction _action);

	// Begin receiving notifications for changes to _dir (and its subtree). _callback will be called once for each event. See DispatchNotifcations().
	static void        BeginNotifications(const char* _dir, FileActionCallback* _callback);
	// Stop receiving notifications for changes to _dir.
	static void        EndNotifications(const char* _dir);
	// Dispatch file action notifications. If _dir is 0, dispatch to all registered callbacks. This should be called frequently.
	static void        DispatchNotifications(const char* _dir = nullptr);

private:
	static PathStr    s_roots[RootType_Count];
	static const char s_separator; // per-platform default separator
	
	// Get a path to an existing file based on _path and _rootHint. Return false if no existing file was found.
	static bool FindExisting(PathStr& ret_, const char* _path, RootType _rootHint);

};

} // namespace apt
