#include <apt/Serializer.h>

#include <apt/log.h>

#include <cstdarg>

using namespace apt;

// PUBLIC

void Serializer::setError(const char* _msg, ...)
{
	va_list args;
	va_start(args, _msg);
	m_errStr.setfv(_msg, args);
	va_end(args);
}

template <typename T, uint Tlen>
static bool ValueImpl(Serializer& _serializer_, T& _value_, const char* _name)
{
	uint len = Tlen; \
	if (_serializer_.beginArray(len, _name)) { \
		bool ret = true;
		if (len != Tlen) {
			_serializer_.setError("Error serializing vec/mat %s: array length was %d, expected %d", _name ? _name : "", (int)len, (int)Tlen);
			ret = false;
		} else {
			for (int i = 0; i < (int)Tlen; ++i) { 
				ret &= _serializer_.value(_value_[i]);
			}
		}
		_serializer_.endArray();
		return ret;
	}
	return false;
}
bool Serializer::value(vec2& _value_, const char* _name) { return ValueImpl<vec2,   2>(*this, _value_, _name); }
bool Serializer::value(vec3& _value_, const char* _name) { return ValueImpl<vec3,   3>(*this, _value_, _name); }
bool Serializer::value(vec4& _value_, const char* _name) { return ValueImpl<vec4,   4>(*this, _value_, _name); }
bool Serializer::value(mat2& _value_, const char* _name) { return ValueImpl<mat2, 2*2>(*this, _value_, _name); }
bool Serializer::value(mat3& _value_, const char* _name) { return ValueImpl<mat3, 3*3>(*this, _value_, _name); }
bool Serializer::value(mat4& _value_, const char* _name) { return ValueImpl<mat4, 4*4>(*this, _value_, _name); }

template <typename T>
static bool SerializeImpl(Serializer& _serializer_, T& _value_, const char* _name)
{
	if (!_serializer_.value(_value_, _name)) {
		APT_LOG_ERR(_serializer_.getError());
		return false;
	}
	return true;
}
bool apt::Serialize(Serializer& _serializer_, bool&       _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, sint8&      _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, uint8&      _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, sint16&     _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, uint16&     _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, sint32&     _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, uint32&     _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, sint64&     _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, uint64&     _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, float32&    _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, float64&    _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, StringBase& _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, vec2&       _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, vec3&       _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, vec4&       _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, mat2&       _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, mat3&       _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }
bool apt::Serialize(Serializer& _serializer_, mat4&       _value_, const char* _name) { return SerializeImpl(_serializer_, _value_, _name); }