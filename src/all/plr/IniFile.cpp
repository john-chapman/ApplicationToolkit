////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 John Chapman -- http://john-chapman.net
// This software is distributed freely under the terms of the MIT License.
// See http://opensource.org/licenses/MIT
////////////////////////////////////////////////////////////////////////////////
#include <plr/IniFile.h>

#include <plr/log.h>
#include <plr/TextParser.h>

#include <fstream>

using namespace plr;

static const char* kLineEnd = "\n";

#define INI_ERROR(line, msg) PLR_LOG_ERR("Ini syntax error, line %d: '%s'", line, msg)

const StringHash IniFile::kDefaultSection = StringHash::kInvalidHash;

IniFile::~IniFile()
{
	for (uint i = 0u; i < m_keys.size(); ++i) {
		Key& k = m_keys[i];
		if (k.m_type != ValueType::kString) {
			continue;
		}
		for (uint j = k.m_valueOffset; j < k.m_valueOffset + k.m_count; ++j) {
			delete[] m_values[j].m_string;
		}
	}
}

IniFile::Error IniFile::load(const char* _path)
{
	std::ifstream fin;
	fin.open(_path, std::ios::in | std::ios::binary);
	if (!fin.is_open()) {
		return Error::kFileNotFound;
	}

	Error ret = Error::kOk;
	char* fdata = 0;
	fin.seekg(0, std::ios::end);
	sint flen = fin.tellg();
	fin.seekg(0, std::ios::beg);
	
	if (flen <= 0) {
		ret = Error::kFileIo;
		goto IniFile_load_fail;
	}

	fdata = new char[flen + 1];
	PLR_ASSERT(fdata);
	PLR_VERIFY(fin.read(fdata, flen).tellg() == flen);
	if (fin.bad()) {
		ret = Error::kFileIo;
		goto IniFile_load_fail;
	}
	fdata[flen] = 0;
	ret = parse(fdata);

IniFile_load_fail:
	delete[] fdata;
	fin.close();
	return ret;
}

IniFile::Error IniFile::parse(const char* _str)
{
	PLR_ASSERT(_str);
	
	if (m_sections.empty()) {
		Section s = { kDefaultSection, 0u, m_keys.size() };
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
				return Error::kSyntax;
			}
			Section s = { StringHash(beg, tp - beg), 0u, m_keys.size() };
			m_sections.push_back(s);
			tp.advance(); // skip ']'
		} else if (*tp == '=' || *tp == ',') {
			if (m_keys.empty()) {
				INI_ERROR(tp.getLineCount(), "Unexpected '=' or ',' no property name was specified");
				return Error::kSyntax;
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
				k.m_type = ValueType::kString;
				tp.advance(); // skip '"'
				const char* beg = tp;
				if (!tp.advanceToNext('"')) {
					INI_ERROR(tp.getLineCount(beg), "Unterminated string");
					return Error::kSyntax;
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
				k.m_type = ValueType::kBool;
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
					k.m_type = ValueType::kInt;
					v.m_int = (sint64)l;
				} else if (l == 0 && d != 0.0) {
				 // value was a double
					k.m_type = ValueType::kDouble;
					v.m_double = d;
				} else {
				 // both were nonzero, guess if an int or a double was intended
					if (tp.containsAny(beg, ".eEnN")) { // n/N to catch INF/NAN
						k.m_type = ValueType::kDouble;
						v.m_double = d;
					} else {
						k.m_type = ValueType::kInt;
						v.m_int = (sint64)l;
					}
				}
				m_values.push_back(v);

			} else {
				INI_ERROR(tp.getLineCount(vbeg), "Invalid value");
				return Error::kSyntax;
			}

			if (k.m_count > 0u && k.m_type != t) {
				INI_ERROR(tp.getLineCount(vbeg), "Invalid array (arrays must be homogeneous)");
				return Error::kSyntax;
			}
			++k.m_count;
		} else if (!tp.isNull()) {
		 // new data
			if (tp.isNum()) {
				INI_ERROR(tp.getLineCount(), "Property names cannot begin with a number");
				return Error::kSyntax;
			}

			const char* beg = tp;
			if (!tp.advanceToNextNonAlphaNum()) {
				INI_ERROR(tp.getLineCount(), "Unexpected end of file");
				return Error::kSyntax;
			}

			Key k;
			k.m_key = StringHash(beg, tp - beg);
			k.m_valueOffset = m_values.size();
			k.m_count = 0;
			m_keys.push_back(k);

			++m_sections.back().m_count;
		}
	};
	return Error::kOk;
}

IniFile::Property IniFile::getProperty(const char* _key, const char* _section)
{
	StringHash k(_key);
	StringHash s = _section ? StringHash(_section) : StringHash::kInvalidHash;
	return getProperty(k, s);
}

IniFile::Property IniFile::getProperty(StringHash _key, StringHash _section)
{
	Property ret(ValueType::kBool, 0u, 0);

	uint koff = 0u;
	uint kcount = m_keys.size();
	if (_section != StringHash::kInvalidHash) {
		for (uint i = 0u; i < m_sections.size(); ++i) {
			if (m_sections[i].m_name == _section) {
				koff = m_sections[i].m_keyOffset;
				kcount = m_sections[i].m_count;
				break;
			}
		}
	}
	for (uint i = koff, n = koff + kcount; i < n; ++i) {
		if (m_keys[i].m_key == _key) {
			ret.m_type = (uint)m_keys[i].m_type;
			ret.m_count = m_keys[i].m_count;
			ret.m_first = &m_values[m_keys[i].m_valueOffset];
			break;
		}
	}
	return ret;
}