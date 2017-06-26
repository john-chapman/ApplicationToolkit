#pragma once
#ifndef apt_File_h
#define apt_File_h

#include <apt/def.h>
#include <apt/String.h>

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// File
// Noncopyable but movable.
// Files loaded into memory via Read() have an implicit null character appended
// to the internal data buffer, hence getData() can be interpreted directly as
// C string.
// \todo API should include some interface for either writing to the internal 
//   buffer directly, or setting the buffer ptr without copying all the data
//   (prefer the former, buffer ownership issues in the latter case).
// \todo Checksum/hash util.
////////////////////////////////////////////////////////////////////////////////
class File
	: private non_copyable<File>
{
public:
	typedef String<64> PathStr;

	File();
	~File();

	// Return true if _path exists.
	static bool Exists(const char* _path);

	// Read file into memory from _path, or file_.getPath() if _path is 0. Use getData() to
	// access the resulting buffer. Return false if an error occurred, in which case file_ 
	// remains unchanged. On success, any resources already associated with file_ are released.
	// \note An implicit null is appended to the data buffer, hence getData() can be 
	//   interpreted directly as a C string.
	static bool Read(File& file_, const char* _path = 0);

	// Write file to _path, or _file.getPath() if _path is 0. Return false if an error occurred, 
	// in which case any existing file at _path may or may not have been overwritten.
	static bool Write(const File& _file, const char* _path = 0);

	// Allocate _size bytes for the internal buffer and optionally copy from _data. If _data 
	// is 0 the buffer is allocated.
	void        setData(const char* _data, uint64 _size);

	// Append _size bytes from _data to the internal buffer. If _data is 0 the internal buffer is reallocated.
	void        appendData(const char* _data, uint64 _size);

	const char* getPath() const                                 { return m_path; }
	void        setPath(const char* _path)                      { m_path.set(_path); }
	const char* getData() const                                 { return m_data; }
	char*       getData()                                       { return m_data; }
	uint64      getDataSize() const                             { return m_dataSize; }
	void        setDataSize(uint64 _size)                       { setData(0, _size); }


private:
	PathStr m_path;
	char*   m_data;
	uint64  m_dataSize;
	void*   m_impl;

	void ctorCommon();
	void dtorCommon();

}; // class File

} // namespace apt

#endif // apt_File_h
