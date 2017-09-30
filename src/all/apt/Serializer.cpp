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

bool Serializer::value(vec2& _value_, const char* _name)
{
	uint len = 2;
	if (beginArray(len, _name)) {
		bool ret = true;
		for (int i = 0; i < (int)len; ++i) { 
			ret &= value(_value_[i]);
		}
		endArray();
		return ret;
	}
	return false;
}

bool Serializer::value(vec3& _value_, const char* _name)
{
	uint len = 3;
	if (beginArray(len, _name)) {
		bool ret = true;
		for (int i = 0; i < (int)len; ++i) { 
			ret &= value(_value_[i]);
		}
		endArray();
		return ret;
	}
	return false;
}

bool Serializer::value(vec4& _value_, const char* _name)
{
	uint len = 4;
	if (beginArray(len, _name)) {
		bool ret = true;
		for (int i = 0; i < (int)len; ++i) { 
			ret &= value(_value_[i]);
		}
		endArray();
		return ret;
	}
	return false;
}

bool Serializer::value(mat2& _value_, const char* _name)
{
	uint len = 2*2;
	if (beginArray(len, _name)) {
		bool ret = true;
		for (int i = 0; i < (int)len; ++i) { 
			ret &= value(_value_[i]);
		}
		endArray();
		return ret;
	}
	return false;
}

bool Serializer::value(mat3& _value_, const char* _name)
{
	uint len = 3*3;
	if (beginArray(len, _name)) {
		bool ret = true;
		for (int i = 0; i < (int)len; ++i) { 
			ret &= value(_value_[i]);
		}
		endArray();
		return ret;
	}
	return false;
}

bool Serializer::value(mat4& _value_, const char* _name)
{
	uint len = 4*4;
	if (beginArray(len, _name)) {
		bool ret = true;
		for (int i = 0; i < (int)len; ++i) { 
			ret &= value(_value_[i]);
		}
		endArray();
		return ret;
	}
	return false;
}



#define SerializeImpl(T) \
	bool apt::Serialize(Serializer& _serializer_, T& _value_, const char* _name) \
	{ \
		if (!_serializer_.value(_value_, _name)) { \
			APT_LOG_ERR(_serializer_.getError()); \
			return false; \
		} \
		return true; \
	}

SerializeImpl(bool)
SerializeImpl(sint8)
SerializeImpl(uint8)
SerializeImpl(sint16)
SerializeImpl(uint16)
SerializeImpl(sint32)
SerializeImpl(uint32)
SerializeImpl(sint64)
SerializeImpl(uint64)
SerializeImpl(float32)
SerializeImpl(float64)
SerializeImpl(StringBase)
SerializeImpl(vec2)
SerializeImpl(vec3)
SerializeImpl(vec4)
SerializeImpl(mat2)
SerializeImpl(mat3)
SerializeImpl(mat4)