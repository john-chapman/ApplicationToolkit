#include <apt/Ini.h>

#include <apt/log.h>
#include <apt/File.h>
#include <apt/String.h>
#include <apt/TextParser.h>
#include <apt/Time.h>

using namespace apt;

static const char* kLineEnd = "\n";

#define INI_ERROR(line, msg) APT_LOG_ERR("Ini syntax error, line %d: '%s'", line, msg)

// PUBLIC

bool Ini::Read(Ini& iniFile_, const File& _file)
{
	return iniFile_.parse(_file.getData());
}

bool Ini::Read(Ini& iniFile_, const char* _path)
{
	APT_AUTOTIMER("Ini::Read(%s)", _path);
	File f;
	if (!File::Read(f, _path)) {
		return false;
	}
	return Read(iniFile_, f);
}

bool Ini::Write(const Ini& _iniFile, File& file_)
{
	String<0> buf;

	for (uint i = 0; i < _iniFile.m_sections.size(); ++i) {
		const Section& section = _iniFile.m_sections[i];
		if (section.m_propertyCount == 0) { // skip empty sections
			continue;
		}
		if (!section.m_name.isEmpty()) { // default empty section
			buf.appendf("[%s]\n", (const char*)section.m_name);
		}

		for (uint j = section.m_keyOffset, n = j + section.m_propertyCount; j < n; ++j) {
			const Key& key = _iniFile.m_keys[j];
			buf.appendf("%s = ", (const char*)key.m_name);
			for (uint16 k = key.m_valueOffset, m = k + key.m_valueCount; k < m; ++k) {
				const Value& val = _iniFile.m_values[k];
				switch (key.m_type) {
				case ValueType_Bool:   buf.append(val.m_bool ? "true" : "false"); break;
				case ValueType_Int:    buf.appendf("%d", val.m_int); break;
				case ValueType_Double: buf.appendf("%1.10f", val.m_double); break; // \todo better float representation for printing; want to print the smallest number of significant digits
				case ValueType_String: buf.appendf("\"%s\"", val.m_string); break;
				default:                 APT_ASSERT(false);
				};
				if ((k + 1) != m) {
					buf.append(", ");
				}
			}
			buf.append("\n");
		}

		buf.append("\n");
	}
	file_.setData((const char*)buf, buf.getLength());
	return true;
}

bool Ini::Write(const Ini& _iniFile, const char* _path)
{
	APT_AUTOTIMER("Ini::Write(%s)", _path);
	File f;
	if (Write(_iniFile, f)) {
		return File::Write(f, _path);
	}
	return false;
}

Ini::Ini()
{
 // push default section
	Section s = { "", 0, (uint16)m_keys.size() };
	m_sections.push_back(s);
}

Ini::~Ini()
{
	for (uint i = 0; i < m_keys.size(); ++i) {
		Key& k = m_keys[i];
		if (k.m_type != ValueType_String) {
			continue;
		}
		for (uint j = k.m_valueOffset; j < k.m_valueOffset + k.m_valueCount; ++j) {
			delete[] m_values[j].m_string;
		}
	}
}

Ini::Property Ini::getProperty(const char* _name, const char* _section) const
{
	Property ret(ValueType_Bool, 0, 0);

	const Section* section = _section ? findSection(_section) : 0;
	const Key* key = findKey(_name, section);
	if (key) {
		ret.m_type = (uint8)key->m_type;
		ret.m_count = key->m_valueCount;
		ret.m_first = &const_cast<Value&>(m_values[key->m_valueOffset]);
	}
	return ret;
}

void Ini::pushSection(const char* _name)
{
	APT_ASSERT_MSG(findSection(_name) == 0, "Ini::pushSection: '%s' already exists", _name);
	Section s = { NameStr(_name), 0, (uint16)m_keys.size() };
	m_sections.push_back(s);
}

#define DEFINE_pushValueArray(_type, _typeEnum, _valueMember) \
	template <> void Ini::pushValueArray<_type>(const char* _name, const _type _value[], uint16 _count) { \
		APT_ASSERT_MSG(findKey(_name, &m_sections.back()) == 0, "Ini::pushValue: '%s' already exists in section '%s'", _name, m_sections.back().m_name.isEmpty() ? "default" : (const char*)m_sections.back().m_name); \
		Key key = { NameStr(_name), ValueType:: ## _typeEnum, _count, (uint16)m_values.size() }; \
		m_keys.push_back(key); \
		for (uint16 i = 0; i < _count; ++i) { \
			m_values.push_back(Value()); \
			m_values.back(). ## _valueMember = _value[i]; \
		} \
		++m_sections.back().m_propertyCount; \
	}
DEFINE_pushValueArray(bool,   ValueType_Bool,   m_bool)
DEFINE_pushValueArray(int,    ValueType_Int,    m_int)
DEFINE_pushValueArray(double, ValueType_Double, m_double)
DEFINE_pushValueArray(float,  ValueType_Double, m_double)
#undef DEFINE_pushValueArray

void Ini::pushValueArray(const char* _name, const char* _value[], uint16 _count)
{
	APT_ASSERT_MSG(findKey(_name, &m_sections.back()) == 0, "Ini::pushValue: '%s' already exists in section '%s'", _name, m_sections.back().m_name.isEmpty() ? "default" : (const char*)m_sections.back().m_name);
	Key key = { NameStr(_name), ValueType_String, _count, (uint16)m_values.size() };
	m_keys.push_back(key);
	for (uint16 i = 0; i < _count; ++i) {
		m_values.push_back(Value());
		m_values.back().m_string = new char[strlen(_value[i]) + 1];
		strcpy(m_values.back().m_string, _value[i]);
	}
	++m_sections.back().m_propertyCount;
}

// PRIVATE

bool Ini::parse(const char* _str)
{
	APT_ASSERT(_str);
	
	if (m_sections.empty()) {
		Section s = { "", 0, (uint16)m_keys.size() };
		m_sections.push_back(s);
	}

	TextParser tp(_str);
	while (!tp.isNull()) {
		tp.skipWhitespace();
		if (*tp == ';') {
		 // comment
			tp.skipLine();
		} else if (*tp == '[') {
		 // section
			tp.advance();
			const char* beg = tp;
			if (!tp.advanceToNext(']')) {
				INI_ERROR(tp.getLineCount(beg), "Unterminated section");
				return false;
			}
			Section s = { NameStr(), 0, (uint16)m_keys.size() };
			s.m_name.set(beg, tp - beg);
			m_sections.push_back(s);
			tp.advance(); // skip ']'
		} else if (*tp == '=' || *tp == ',') {
			if (m_keys.empty()) {
				INI_ERROR(tp.getLineCount(), "Unexpected '=' or ',' no property name was specified");
				return false;
			}
			Key& k = m_keys.back();
			ValueType t = k.m_type; // for sanity check below

			tp.advance(); // skip '='/','
			tp.skipWhitespace();
			while (*tp == ';') {
				tp.skipLine();
				tp.skipWhitespace();
			}
			const char* vbeg = tp; // for getLineCount if value was invalid
			if (*tp == '"') {
			 // value is a string
				k.m_type = ValueType_String;
				tp.advance(); // skip '"'
				const char* beg = tp;
				if (!tp.advanceToNext('"')) {
					INI_ERROR(tp.getLineCount(beg), "Unterminated string");
					return false;
				}

				Value v;
				sint n = tp - beg;
				tp.advance(); // skip '"'
				v.m_string = new char[n + 1];
				strncpy(v.m_string, beg, n);
				v.m_string[n] = '\0';
				m_values.push_back(v);

			} else if (*tp == 't' || *tp == 'f') {
			 // value is a bool
				k.m_type = ValueType_Bool;
				Value v;
				v.m_bool = *tp == 't' ? true : false;
				m_values.push_back(v);
				tp.advanceToNextWhitespaceOr(',');

			} else if (tp.isNum() || *tp == '-' || *tp == '+') {
			 // value is a number
				const char* beg = tp;
				tp.advanceToNextWhitespaceOr(',');
				long int l = strtol(beg, 0, 0);
				double d = strtod(beg, 0);
				Value v;
				if (d == 0.0 && l != 0) {
				 // value was an int
					k.m_type = ValueType_Int;
					v.m_int = (sint64)l;
				} else if (l == 0 && d != 0.0) {
				 // value was a double
					k.m_type = ValueType_Double;
					v.m_double = d;
				} else {
				 // both were nonzero, guess if an int or a double was intended
					if (tp.containsAny(beg, ".eEnN")) { // n/N to catch INF/NAN
						k.m_type = ValueType_Double;
						v.m_double = d;
					} else {
						k.m_type = ValueType_Int;
						v.m_int = (sint64)l;
					}
				}
				m_values.push_back(v);

			} else {
				INI_ERROR(tp.getLineCount(vbeg), "Invalid value");
				return false;
			}

			if (k.m_valueCount > 0 && k.m_type != t) {
				INI_ERROR(tp.getLineCount(vbeg), "Invalid array (arrays must be homogeneous)");
				return false;
			}
			++k.m_valueCount;
		} else if (!tp.isNull()) {
		 // new data
			if (tp.isNum()) {
				INI_ERROR(tp.getLineCount(), "Property names cannot begin with a number");
				return false;
			}

			const char* beg = tp;
			if (!tp.advanceToNextNonAlphaNum()) {
				INI_ERROR(tp.getLineCount(), "Unexpected end of file");
				return false;
			}

			Key k;
			k.m_name.set(beg, tp - beg);
			k.m_valueOffset = (uint16)m_values.size();
			k.m_valueCount = 0;
			m_keys.push_back(k);

			++m_sections.back().m_propertyCount;
		}
	};
	return true;
}

const Ini::Section* Ini::findSection(const char* _name) const
{
	for (uint i = 0; i < m_sections.size(); ++i) {
		if (m_sections[i].m_name == _name) {
			return &m_sections[i];
		}
	}
	return 0;
}
const Ini::Key* Ini::findKey(const char* _name, const Section* _section) const
{
	uint koff = 0;
	uint kcount = m_keys.size();
	if (_section) {
		koff = _section->m_keyOffset;
		kcount = _section->m_propertyCount;
	}
	for (uint i = koff, n = koff + kcount; i < n; ++i) {
		if (m_keys[i].m_name == _name) {
			return &m_keys[i];
		}
	}
	return 0;
}

