#pragma once
#ifndef apt_Serializer_h
#define apt_Serializer_h

#include <apt/apt.h>
#include <apt/types.h>
#include <apt/math.h>
#include <apt/String.h>

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// Serializer
////////////////////////////////////////////////////////////////////////////////
class Serializer
{
public:
	enum Mode
	{
		Mode_Read,
		Mode_Write
	};

	Mode        getMode() const                       { return m_mode; }
	const char* getError() const                      { return (const char*)m_errStr; }
	void        setError(const char* _msg, ...);

	virtual bool beginObject(const char* _name = nullptr) = 0;
	virtual void endObject() = 0;

	virtual bool beginArray(uint& _length_, const char* _name = nullptr) = 0;
	virtual void endArray() = 0;

	virtual bool value(bool&       _value_, const char* _name = nullptr) = 0;
	virtual bool value(sint8&      _value_, const char* _name = nullptr) = 0;
	virtual bool value(uint8&      _value_, const char* _name = nullptr) = 0;
	virtual bool value(sint16&     _value_, const char* _name = nullptr) = 0;
	virtual bool value(uint16&     _value_, const char* _name = nullptr) = 0;
	virtual bool value(sint32&     _value_, const char* _name = nullptr) = 0;
	virtual bool value(uint32&     _value_, const char* _name = nullptr) = 0;
	virtual bool value(sint64&     _value_, const char* _name = nullptr) = 0;
	virtual bool value(uint64&     _value_, const char* _name = nullptr) = 0;
	virtual bool value(float32&    _value_, const char* _name = nullptr) = 0;
	virtual bool value(float64&    _value_, const char* _name = nullptr) = 0;
	virtual bool value(StringBase& _value_, const char* _name = nullptr) = 0;
	
	template <uint kCapacity>
	bool value(String<kCapacity>& _value_, const char* _name = nullptr)
	{
		return value((StringBase&)_value_, _name); 
	}

	bool value(vec2& _value_, const char* _name = nullptr);
	bool value(vec3& _value_, const char* _name = nullptr);
	bool value(vec4& _value_, const char* _name = nullptr);
	bool value(mat2& _value_, const char* _name = nullptr);
	bool value(mat3& _value_, const char* _name = nullptr);
	bool value(mat4& _value_, const char* _name = nullptr);

protected:
	Mode m_mode;

	Serializer(Mode _mode)
		: m_mode(_mode)
	{
	}
	virtual ~Serializer()
	{
	}

private:
	String<64> m_errStr;

}; // class Serializer


bool Serialize(Serializer& _serializer_, bool&        _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, sint8&       _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, uint8&       _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, sint16&      _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, uint16&      _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, sint32&      _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, uint32&      _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, sint64&      _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, uint64&      _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, float32&     _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, float64&     _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, StringBase&  _value_, const char* _name = nullptr);

template <uint kCapacity>
bool Serialize(Serializer& _serializer_, String<kCapacity>& _value_, const char* _name = nullptr)
{
	return Serialize((StringBase&)_value_, _name); 
}

bool Serialize(Serializer& _serializer_, vec2& _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, vec3& _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, vec4& _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, mat2& _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, mat3& _value_, const char* _name = nullptr);
bool Serialize(Serializer& _serializer_, mat4& _value_, const char* _name = nullptr);

} // namespace apt

#endif // apt_Serializer_h
