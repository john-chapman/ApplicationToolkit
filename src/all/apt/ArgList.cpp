#include <apt/ArgList.h>

#include <cstdlib>
#include <cstring>
#include <cctype>

using namespace apt;

bool Arg::Value::asBool() const
{
	return !(m_value[0] == 'f' || m_value[0] == 'F' || m_value[0] == '0');
}

sint64 Arg::Value::asInt() const
{
	return (sint64)atol(m_value);
}

double Arg::Value::asDouble() const
{
	return atof(m_value);
}

const char* Arg::Value::asString() const
{
	return m_value;
}

static bool IsArgName(const char* _cmp, const char* _argNames)
{
	if (_cmp[0] == '-' && !isdigit(_cmp[1])) {
	 // '-' indicates a new arg unless the following digit is a number (in which case it's a negative value)
		return true;
	}
	while (*_argNames != '\0') {
		if (strcmp(_cmp, _argNames) == 0) {
			return true;
		}
		_argNames = strchr(_argNames, 0);
		APT_ASSERT(_argNames);
		++_argNames;
	}
	return false;
}

ArgList::ArgList(int _argc, char* _argv[], const char* _argNames)
{
	int currentArg = -1;
	for (int i = 1; i < _argc; ++i) {
		if (IsArgName(_argv[i], _argNames)) {
		 // skip over '-'
			int skip = 0;
			while (_argv[i][skip] == '-') {
				++skip;
			}
			m_args.push_back(Arg(&_argv[i][skip]));
			++currentArg;
		} else {
			if (currentArg == -1) {
			 // no args, push a null argument
				m_args.push_back(Arg(0));
				++currentArg;
			}
		 // push value into the current argument
			m_args[currentArg].pushValue(_argv[i]);
		}
	}
}

const Arg* ArgList::find(const char* _name) const
{
	for (auto arg = m_args.begin(); arg != m_args.end(); ++arg) {
		if (strcmp(_name, arg->getName()) == 0) {
			return (Arg*)&(*arg);
		}
	}
	return nullptr;
}
