#pragma once
#ifndef apt_ArgList_h
#define apt_ArgList_h

#include <apt/apt.h>

#include <EASTL/vector.h>

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// Arg
// Arg associates a name with one or more values.
////////////////////////////////////////////////////////////////////////////////
class Arg
{
public:
	class Value
	{
		friend class Arg;
		const char* m_value;
		Value(const char* _value): m_value(_value) {}

	public:
		bool        asBool() const;
		sint64      asInt() const;
		double      asDouble() const;
		const char* asString() const;

		operator const char*() const { return m_value; }
	};

	Arg(char* _name = nullptr)
		: m_name(_name ? _name : "")
	{
	}

	const char* getName() const              { return m_name; }
	uint        getValueCount() const        { return (uint)m_values.size(); }
	void        pushValue(const char* _val)  { m_values.push_back(_val); }
	Value       getValue(int _i = 0) const   { APT_ASSERT(_i < m_values.size()); return Value(m_values[_i]); }
	Value       operator[](int _i) const     { APT_ASSERT(_i < m_values.size()); return Value(m_values[_i]); }
	
private:
	const char* m_name;
	eastl::vector<const char*> m_values;
};

////////////////////////////////////////////////////////////////////////////////
// ArgList
// Elements of _argv beginning with '-' or '--' are interpreted as arg names; 
// all other elements are interpreted as arg values (belonging to the preceeding
// arg).
//
// Special arg names which don't begin with '-' or '--' can be specified by
// passing a null-delimited set of strings to the ctor.
//
// If no arg names are specified, values are pushed to a default nameless arg.
////////////////////////////////////////////////////////////////////////////////
class ArgList
{
public:
	// Init from command line arguments. _argNames is a null-delimited set of strings which are to be
	// interpreted as args, e.g. "add\0mul\0div\0".
	ArgList(int _argc, char* _argv[], const char* _argNames = "");

	uint       getArgCount() const       { return (uint)m_args.size(); }
	const Arg& getArg(int _i)            { APT_ASSERT(_i < m_args.size()); return m_args[_i]; }
	const Arg& operator[](int _i) const  { APT_ASSERT(_i < m_args.size()); return m_args[_i]; }

	// Arg with matching _name, or nullptr if not present.
	const Arg* find(const char* _name) const;

private:
	eastl::vector<Arg> m_args;
};

} // namespace apt

#endif // apt_ArgList_h
