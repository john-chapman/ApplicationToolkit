#pragma once
#ifndef apt_String_h
#define apt_String_h

#include <apt/def.h>

#include <cstdarg> // va_list
#include <utility> // std::move

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// StringBase
// Base for string class with an optional local buffer. If/when the local 
// buffer overflows it is replaced with a heap-allocated buffer. Once the 
// buffer is heap-allocated it never returns to using the local buffer.
// All `const char*` interfaces expect null-terminated strings.
////////////////////////////////////////////////////////////////////////////////
class StringBase
{
public:
	// Copy _count characters from _src. If _count == 0, all of _src is copied
	// If the end of _src is found  before _count characters have been copied,
	// the result is padded with zeros until _count characters have been written
	// (like strncpy). Unlike strncpy, an implicit null char is appended to the
	// end of the result. Return the new length of the string, excluding the null 
	// terminator.
	uint set(const char* _src, uint _count = 0);
	// Set formatted content. Return the new length of the string, excluding the 
	// null terminator.
	uint setf(const char* _fmt, ...);
	uint setfv(const char* _fmt, va_list _args);

	// Append _count characters from _src. If _count == 0, all of _src is
	// appended. If the end of _src is found  before _count characters have been 
	// appended, the result is padded with zeros until _count characters have been 
	// written (like strncpy). Unlike strncpy, an implicit null char is appended 
	// to the end of the result. Return the new length of the string, excluding 
	// the null terminator.
	uint append(const char* _src, uint _count = 0);
	// Append formatted content. Return the new length of the string, excluding the 
	// null terminator.
	uint appendf(const char* _fmt, ...);
	uint appendfv(const char* _fmt, va_list _args);

	// Find the first (or last) occurence of any character in _list (which is 
	// null-terminated). If no match is found return null.
	const char* findFirst(const char* _list) const;
	const char* findLast(const char* _list) const;

	// Find the first occurence of the substring _str, return null if not found.
	const char* find(const char* _str) const;

	// Replace all instanced of _find with _replace.
	void replace(char _find, char _replace);

	// Convert to upper/lower case.
	void toUpperCase();
	void toLowerCase();

	// String length, excluding the terminating null.
	// \note String length is not stored internally, hence getLength() is **not**
	//    a constant time operation.
	uint getLength() const;

	void clear()                                { m_buf[0] = '\0'; }

	bool isEmpty() const                        { return m_buf[0] == '\0'; }
	bool isLocal() const                        { return m_buf == getLocalBuf(); }
	uint getCapacity() const                    { return m_capacity; }
	void setCapacity(uint _capacity);

	bool operator==(const char* _rhs) const;
	operator const char*() const                { return m_buf; }
	operator char*()                            { return m_buf; }
	
	friend void swap(StringBase& _a, StringBase& _b);

protected:
	
	// String always heap-allocated.
	StringBase();
	// String has a local buffer of _localBufferSize chars.
	StringBase(uint _localBufferSize);
	// Move ctor. If _rhs is local it *must* have the same capacity as this (because the local buffer 
	// size isn't stored). This is enforced by the deriving String class move ctors.
	StringBase(StringBase&& _rhs);
	// Move assignment. As move ctor.
	StringBase& operator=(StringBase&& _rhs);

	~StringBase();

	char* getLocalBuf()             { return (char*)this + sizeof(StringBase); }
	const char* getLocalBuf() const { return (char*)this + sizeof(StringBase); }

private:
	char* m_buf;       // Ptr to local buffer, or heap allocated.
	uint  m_capacity;  // Local buffer/allocated size.

	// Resize m_buf to _capacity, discard contents.
	void alloc(uint _capacity);

	// Resize m_buf to _capaicty, maintain contents.
	void realloc(uint _capacity);

}; // class String


template <uint kCapacity>
class String: public StringBase
{
	char m_localBuf[kCapacity];

public:
	String():                              StringBase(kCapacity)       {}
	String(const String<kCapacity>& _rhs): StringBase(kCapacity)       { set(_rhs); }
	String<kCapacity>& operator=(const String<kCapacity>& _rhs)        { set(_rhs); return *this; }
	String(String<kCapacity>&& _rhs):      StringBase(std::move(_rhs)) {}
	String<kCapacity>& operator=(String<kCapacity>&& _rhs)             { StringBase::operator=(std::move(_rhs)); return *this; }
	String(const char* _fmt, ...):         StringBase(kCapacity)
	{
		if (_fmt) {
			va_list args;
			va_start(args, _fmt);
			setfv(_fmt, args);
			va_end(args);
		}
	}
};

template <>
class String<0>: public StringBase
{
public:
	String():                      StringBase()                {}
	String(const String<0>& _rhs): StringBase()                { set(_rhs); }
	String<0>& operator=(const String<0>& _rhs)                { set(_rhs); return *this; }
	String(String<0>&& _rhs):      StringBase(std::move(_rhs)) {}
	String<0>& operator=(String<0>&& _rhs)                     { (String<0>)StringBase::operator=(std::move(_rhs)); return *this; }
	String(const char* _fmt, ...): StringBase()
	{
		if (_fmt) {
			va_list args;
			va_start(args, _fmt);
			setfv(_fmt, args);
			va_end(args);
		}
	}
};

} // namespace apt

#endif // apt_String_h
