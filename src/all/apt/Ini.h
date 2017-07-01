#pragma once
#ifndef apt_Ini_h
#define apt_Ini_h

#include <apt/def.h>
#include <apt/String.h>

#include <EASTL/vector.h>

namespace apt {

class File;

////////////////////////////////////////////////////////////////////////////////
// Ini
// Read/write INI files; e.g.
// \code
//  ; comment
//    [section] ; sections are optional
//    int    = 12
//    double = 12.0
//    string = "twelve"  ; strings are enclosed in double quotes
//    array  = 12, 13, 14
//    array2 =  ; all whitespace is ignored, including line breaks
//       "twelve",
//       "thirteen",
//       "fourteen"
// \endcode
//
// \todo String escape characters
// \todo Line breaks within strings
////////////////////////////////////////////////////////////////////////////////
class Ini
{
public:

	enum ValueType
	{
		ValueType_Bool, 
		ValueType_Int,
		ValueType_Double,
		ValueType_String,

		ValueType_Count
	};

	union Value
	{
		bool    m_bool;
		sint64  m_int;
		double  m_double;
		char*   m_string;
	};

	class Property
	{
		friend class Ini;

		ValueType m_type;
		int       m_count;
		Value*    m_first;

		Property(ValueType _type, int _count, Value* _first)
			: m_type(_type)
			, m_count(_count)
			, m_first(_first)
		{
		}

	public:
		Property()
			: m_type(ValueType_Count)
			, m_count(0)
			, m_first(0)
		{
		}
		
		bool        isNull() const                { return m_first == 0; }
		ValueType   getType() const               { return m_type; }
		int         getCount() const              { return m_count; }
		bool        asBool(int _i = 0) const      { APT_ASSERT(m_type == ValueType_Bool);   APT_ASSERT(_i < m_count); return m_first[_i].m_bool;   }
		sint64      asInt(int _i = 0) const       { APT_ASSERT(m_type == ValueType_Int);    APT_ASSERT(_i < m_count); return m_first[_i].m_int;    }
		double      asDouble(int _i = 0) const    { APT_ASSERT(m_type == ValueType_Double); APT_ASSERT(_i < m_count); return m_first[_i].m_double; }
		const char* asString(int _i = 0) const    { APT_ASSERT(m_type == ValueType_String); APT_ASSERT(_i < m_count); return m_first[_i].m_string; }	
	};
	
	// Read from a file and parse. Invalidates any existing Property instances returned by getProperty();
	// Return false if an error occurred.
	static bool Read(Ini& iniFile_, const File& _file);
	static bool Read(Ini& iniFile_, const char* _path);

	// Write to a file.
	static bool Write(const Ini& _iniFile, File& file_);
	static bool Write(const Ini& _iniFile, const char* _path);

	Ini();
	~Ini();
	
	// Retrieve a named property, optionally specifying the section to search.
	// Note that the returned Property instance is only valid during the lifetime
	// of the Ini to which it belongs. Calling load() will invalidate all
	// previously returned Property instances.
	// Return property instance. If no matching property was found, isNull() will return true.
	Property getProperty(const char* _name, const char* _section = 0) const;

	void pushSection(const char* _name);
	template <typename tType>
	void pushValueArray(const char* _name, const tType _value[], int _count);
	void pushValueArray(const char* _name, const char* _value[], int _count);
	template <typename tType>
	void pushValue(const char* _name, const tType& _value) { pushValueArray<tType>(_name, &_value, 1); }
	void pushValue(const char* _name, const char* _value)  { pushValueArray(_name, &_value, 1); }

private:
	typedef String<32> NameStr;

	struct Key
	{
		NameStr    m_name;
		ValueType  m_type;
		int        m_valueCount;
		int        m_valueOffset;
	};
	struct Section
	{
		NameStr    m_name;
		int        m_propertyCount;
		int        m_keyOffset;      
	};

	eastl::vector<Section> m_sections;
	eastl::vector<Key>     m_keys;
	eastl::vector<Value>   m_values;

	bool parse(const char* _str);

	const Section* findSection(const char* _name) const;
	const Key* findKey(const char* _name, const Section* _section) const;
	
}; // class Ini

} // namespace apt

#endif // apt_Ini_h
