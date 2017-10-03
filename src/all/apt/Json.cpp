#include <apt/Json.h>

#include <apt/log.h>
#include <apt/math.h>
#include <apt/FileSystem.h>
#include <apt/String.h>
#include <apt/Time.h>

#include <cstring>

#define RAPIDJSON_ASSERT(x) APT_ASSERT(x)
#define RAPIDJSON_PARSE_DEFAULT_FLAGS (kParseFullPrecisionFlag | kParseCommentsFlag | kParseTrailingCommasFlag)
#include <rapidjson/error/en.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

using namespace apt;

static Json::ValueType GetValueType(rapidjson::Type _type)
{
	switch (_type) {
		case rapidjson::kNullType:   return Json::ValueType_Null;
		case rapidjson::kObjectType: return Json::ValueType_Object;
		case rapidjson::kArrayType:  return Json::ValueType_Array;
		case rapidjson::kFalseType:
		case rapidjson::kTrueType:   return Json::ValueType_Bool;
		case rapidjson::kNumberType: return Json::ValueType_Number;
		case rapidjson::kStringType: return Json::ValueType_String;
		default: APT_ASSERT(false); break;
	};

	return Json::ValueType_Count;
}

/*******************************************************************************

                                   Json

*******************************************************************************/

struct Json::Impl
{
	static const int kMaxStackDepth = 8;

	rapidjson::Document m_dom;

 // current value set after find()
	rapidjson::Value* m_value;

 // value stack for objects/arrays
	rapidjson::Value* m_stack[kMaxStackDepth];
	int m_iter[kMaxStackDepth];
	int m_stackTop;

	void push(rapidjson::Value* _val = 0)
	{
		APT_ASSERT(m_stackTop < kMaxStackDepth);
		APT_ASSERT(top() != _val); // probably a mistake, called push() twice?
		m_stack[++m_stackTop] = _val ? _val : m_value;
		m_iter[m_stackTop] = 0;
	}
	void pop()
	{
		--m_stackTop; 
	}
	rapidjson::Value* top() 
	{
		return m_stack[m_stackTop]; 
	}
	int& topIter()
	{
		return m_iter[m_stackTop];
	}
	

	Impl(): m_stackTop(-1) {}
};


// PUBLIC

bool Json::Read(Json& json_, const File& _file)
{
	json_.m_impl->m_dom.Parse(_file.getData());
	if (json_.m_impl->m_dom.HasParseError()) {
		APT_LOG_ERR("Json error: %s\n\t'%s'", _file.getPath(), rapidjson::GetParseError_En(json_.m_impl->m_dom.GetParseError()));
		return false;
	}
	return true;
}

bool Json::Read(Json& json_, const char* _path, FileSystem::RootType _rootHint)
{
	APT_AUTOTIMER("Json::Read(%s)", _path);
	File f;
	if (!FileSystem::ReadIfExists(f, _path, _rootHint)) {
		return false;
	}
	return Read(json_, f);
}

bool Json::Write(const Json& _json, File& file_)
{
	rapidjson::StringBuffer buf;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> wr(buf);
	wr.SetIndent('\t', 1);
	wr.SetFormatOptions(rapidjson::kFormatSingleLineArray);
	_json.m_impl->m_dom.Accept(wr);
	file_.setData(buf.GetString(), buf.GetSize());
	return true;
}

bool Json::Write(const Json& _json, const char* _path, FileSystem::RootType _rootHint)
{
	APT_AUTOTIMER("Json::Write(%s)", _path);
	File f;
	if (Write(_json, f)) {
		return FileSystem::Write(f, _path, _rootHint);
	}
	return false;
}

Json::Json(const char* _path, FileSystem::RootType _rootHint)
	: m_impl(nullptr)
{
	m_impl = new Impl;
	m_impl->m_dom.SetObject();
	m_impl->push(&m_impl->m_dom);

	if (_path) {
		Json::Read(*this, _path, _rootHint);
	}
}

Json::~Json()
{
	if (m_impl) {
		delete m_impl;
	}
}

bool Json::find(const char* _name)
{
	if (!m_impl->top()->IsObject()) {
		return false;
	}
	auto it = m_impl->top()->FindMember(_name);
	if (it != m_impl->top()->MemberEnd()) {
		m_impl->m_value = &it->value;
		return true;
	}
	return false;
}

bool Json::next()
{
	if (GetValueType(m_impl->top()->GetType()) == ValueType_Array) {
		auto it = m_impl->top()->Begin() + (m_impl->topIter()++);
		m_impl->m_value = it;
		return it != m_impl->top()->End();
	}

	if (GetValueType(m_impl->top()->GetType()) == ValueType_Object) {
		auto it = m_impl->top()->MemberBegin() + (m_impl->topIter()++);
		m_impl->m_value = &it->value;
		return it != m_impl->top()->MemberEnd();
	}

	APT_ASSERT(false); // not an object or an array
	return false;
}

Json::ValueType Json::getType() const
{
	return GetValueType(m_impl->m_value->GetType()); 
}

template <> bool Json::getValue<bool>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Bool, "Json::getValue: value was not a boolean");
	return m_impl->m_value->GetBool();
}
template <> sint64 Json::getValue<sint64>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return m_impl->m_value->GetInt64();
}
template <> sint32 Json::getValue<sint32>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return m_impl->m_value->GetInt();
}
template <> sint16 Json::getValue<sint16>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return (sint16)m_impl->m_value->GetInt();
}
template <> sint8 Json::getValue<sint8>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return (sint8)m_impl->m_value->GetInt();
}
template <> uint64 Json::getValue<uint64>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return m_impl->m_value->GetUint64();
}
template <> uint32 Json::getValue<uint32>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return m_impl->m_value->GetUint();
}
template <> uint16 Json::getValue<uint16>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return (uint16)m_impl->m_value->GetUint();
}
template <> uint8 Json::getValue<uint8>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return (uint8)m_impl->m_value->GetUint();
}
template <> float32 Json::getValue<float32>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return m_impl->m_value->GetFloat();
}
template <> float64 Json::getValue<float64>() const
{
	APT_ASSERT_MSG(getType() == ValueType_Number, "Json::getValue: value was not a number");
	return m_impl->m_value->GetDouble();
}
template <> const char* Json::getValue<const char*>() const
{
	APT_ASSERT_MSG(getType() == ValueType_String, "Json::getValue: value was not a string");
	return m_impl->m_value->GetString();
}
template <> vec2 Json::getValue<vec2>() const
{
	vec2 ret;
	Json* json = const_cast<Json*>(this);
	if (json->enterArray()) {
		APT_ASSERT_MSG(m_impl->top()->Size() == 2, "Json::getValue: invalid vec2, size = %d", m_impl->top()->Size());
		int i = 0;
		while (json->next()) {
			ret[i++] = getValue<float>();
		}
		json->leaveArray();
	}
	return ret;
}
template <> vec3 Json::getValue<vec3>() const
{
	vec3 ret;
	Json* json = const_cast<Json*>(this);
	if (json->enterArray()) {
		APT_ASSERT_MSG(m_impl->top()->Size() == 3, "Json::getValue: invalid vec3, size = %d", m_impl->top()->Size());
		int i = 0;
		while (json->next()) {
			ret[i++] = getValue<float>();
		}
		json->leaveArray();
	}
	return ret;
}
template <> vec4 Json::getValue<vec4>() const
{
	vec4 ret;
	Json* json = const_cast<Json*>(this);
	if (json->enterArray()) {
		APT_ASSERT_MSG(m_impl->top()->Size() == 4, "Json::getValue: invalid vec4, size = %d", m_impl->top()->Size());
		int i = 0;
		while (json->next()) {
			ret[i++] = getValue<float>();
		}
		json->leaveArray();
	}
	return ret;
}
template <> mat2 Json::getValue<mat2>() const
{
	mat2 ret;
	Json* json = const_cast<Json*>(this);
	if (json->enterArray()) {
		APT_ASSERT_MSG(m_impl->top()->Size() == 2, "Json::getValue: invalid mat2, size = %d (should be 2* vec2)", m_impl->top()->Size());
		int i = 0;
		while (json->next()) {
			ret[i++] = getValue<vec2>();
		}
		json->leaveArray();
	}
	return ret;
}
template <> mat3 Json::getValue<mat3>() const
{
	mat3 ret;
	Json* json = const_cast<Json*>(this);
	if (json->enterArray()) {
		APT_ASSERT_MSG(m_impl->top()->Size() == 3, "Json::getValue: invalid mat3, size = %d (should be 3* vec3)", m_impl->top()->Size());
		int i = 0;
		while (json->next()) {
			ret[i++] = getValue<vec3>();
		}
		json->leaveArray();
	}
	return ret;
}
template <> mat4 Json::getValue<mat4>() const
{
	mat4 ret;
	Json* json = const_cast<Json*>(this);
	if (json->enterArray()) {
		APT_ASSERT_MSG(m_impl->top()->Size() == 4, "Json::getValue: invalid mat4, size = %d (should be 4* vec4)", m_impl->top()->Size());
		int i = 0;
		while (json->next()) {
			ret[i++] = getValue<vec4>();
		}
		json->leaveArray();
	}
	return ret;
}

bool Json::enterObject()
{
	if (getType() == ValueType_Object) {
		m_impl->push();
		return true;
	}
	APT_ASSERT(false); // not an object
	return false;
}

void Json::leaveObject()
{
	APT_ASSERT(GetValueType(m_impl->top()->GetType()) == ValueType_Object);
	m_impl->m_value = m_impl->top();
	m_impl->pop();
}

bool Json::enterArray()
{
	if (getType() == ValueType_Array) {
		m_impl->push();
		return true;
	}
	APT_ASSERT(false); // not an array
	return false;
}

void Json::leaveArray()
{
	APT_ASSERT(GetValueType(m_impl->top()->GetType()) == ValueType_Array);
	m_impl->m_value = m_impl->top();
	m_impl->pop();
}

int Json::getArrayLength() const
{
	if (GetValueType(m_impl->top()->GetType()) == ValueType_Array) {
		return (int)m_impl->top()->GetArray().Size();
	}
	return -1;
}

void Json::beginObject(const char* _name)
{
	if (_name && find(_name)) {
	  // object already existed, check the type
		APT_ASSERT(GetValueType(m_impl->m_value->GetType()) == ValueType_Object);
		return;
	} else {
		if (GetValueType(m_impl->top()->GetType()) == ValueType_Array) {
			if (_name) {
				APT_LOG("Json warning: calling beginObject() in an array, name '%s' will be ignored", _name);
			}
			m_impl->top()->PushBack(
				rapidjson::Value(rapidjson::kObjectType).Move(), 
				m_impl->m_dom.GetAllocator()
				);
			m_impl->m_value = m_impl->top()->End() - 1;

		} else {
			m_impl->top()->AddMember(
				rapidjson::StringRef(_name),
				rapidjson::Value(rapidjson::kObjectType).Move(), 
				m_impl->m_dom.GetAllocator()
				);
			m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
		}
	}
	APT_VERIFY(enterObject());
}

void Json::beginArray(const char* _name)
{
	if (_name && find(_name)) {
	  // object already existed, check the type
		APT_ASSERT(GetValueType(m_impl->m_value->GetType()) == ValueType_Array);
		return;
	} else {
		if (GetValueType(m_impl->top()->GetType()) == ValueType_Array) {
			if (_name) {
				APT_LOG("Json warning: calling beginArray() in an array, name '%s' will be ignored", _name);
			}
			m_impl->top()->PushBack(
				rapidjson::Value(rapidjson::kArrayType).Move(), 
				m_impl->m_dom.GetAllocator()
				);
			m_impl->m_value = m_impl->top()->End() - 1;

		} else {
			m_impl->top()->AddMember(
				rapidjson::StringRef(_name),
				rapidjson::Value(rapidjson::kArrayType).Move(), 
				m_impl->m_dom.GetAllocator()
				);
			m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
		}
	}
	APT_VERIFY(enterArray());
}


template <> void Json::setValue<bool>(const char* _name, bool _val)
{
	if (find(_name)) {
		m_impl->m_value->SetBool(_val);
	} else {
		m_impl->top()->AddMember(
			rapidjson::StringRef(_name),
			rapidjson::Value(_val).Move(), 
			m_impl->m_dom.GetAllocator()
			);
		m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
	}
}
template <> void Json::setValue<sint32>(const char* _name, sint32 _val)
{
	if (find(_name)) {
		m_impl->m_value->SetInt(_val);
	} else {
		m_impl->top()->AddMember(
			rapidjson::StringRef(_name),
			rapidjson::Value(_val).Move(), 
			m_impl->m_dom.GetAllocator()
			);
		m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
	}
}
template <> void Json::setValue<sint64>(const char* _name, sint64 _val)
{
	if (find(_name)) {
		m_impl->m_value->SetInt64(_val);
	} else {
		m_impl->top()->AddMember(
			rapidjson::StringRef(_name),
			rapidjson::Value(_val).Move(), 
			m_impl->m_dom.GetAllocator()
			);
		m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
	}
}
template <> void Json::setValue<sint16>(const char* _name, sint16 _val)
{
	setValue<sint32>(_name, (sint32)_val);
}
template <> void Json::setValue<sint8>(const char* _name, sint8 _val)
{
	setValue<sint32>(_name, (sint32)_val);
}
template <> void Json::setValue<uint64>(const char* _name, uint64 _val)
{
	if (find(_name)) {
		m_impl->m_value->SetUint64(_val);
	} else {
		m_impl->top()->AddMember(
			rapidjson::StringRef(_name),
			rapidjson::Value(_val).Move(), 
			m_impl->m_dom.GetAllocator()
			);
		m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
	}
}
template <> void Json::setValue<uint32>(const char* _name, uint32 _val)
{
	if (find(_name)) {
		m_impl->m_value->SetUint(_val);
	} else {
		m_impl->top()->AddMember(
			rapidjson::StringRef(_name),
			rapidjson::Value(_val).Move(), 
			m_impl->m_dom.GetAllocator()
			);
		m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
	}
}
template <> void Json::setValue<uint16>(const char* _name, uint16 _val)
{
	setValue<uint32>(_name, (uint16)_val);
}
template <> void Json::setValue<uint8>(const char* _name, uint8 _val)
{
	setValue<uint32>(_name, (uint32)_val);
}
template <> void Json::setValue<float32>(const char* _name, float32 _val)
{
	if (find(_name)) {
		m_impl->m_value->SetFloat(_val);
	} else {
		m_impl->top()->AddMember(
			rapidjson::StringRef(_name),
			rapidjson::Value(_val).Move(), 
			m_impl->m_dom.GetAllocator()
			);
		m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
	}
}
template <> void Json::setValue<float64>(const char* _name, float64 _val)
{
	if (find(_name)) {
		m_impl->m_value->SetDouble(_val);
	} else {
		m_impl->top()->AddMember(
			rapidjson::StringRef(_name),
			rapidjson::Value(_val).Move(), 
			m_impl->m_dom.GetAllocator()
			);
		m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
	}
}
template <> void Json::setValue<const char*>(const char* _name, const char* _val)
{
	if (find(_name)) {
		m_impl->m_value->SetString(_val, m_impl->m_dom.GetAllocator());
	} else {
		m_impl->top()->AddMember(
			rapidjson::StringRef(_name),
			rapidjson::Value().SetString(_val, m_impl->m_dom.GetAllocator()).Move(), 
			m_impl->m_dom.GetAllocator()
			);
		m_impl->m_value = &(m_impl->top()->MemberEnd() - 1)->value;
	}
}

template <> void Json::pushValue<bool>(bool _val)
{
	m_impl->top()->PushBack(
		rapidjson::Value(_val).Move(), 
		m_impl->m_dom.GetAllocator()
		);
	m_impl->m_value = m_impl->top()->End() - 1;
}

template <> void Json::pushValue<sint64>(sint64 _val)
{
	m_impl->top()->PushBack(
		rapidjson::Value(_val).Move(), 
		m_impl->m_dom.GetAllocator()
		);
	m_impl->m_value = m_impl->top()->End() - 1;
}
template <> void Json::pushValue<sint32>(sint32 _val)
{
	m_impl->top()->PushBack(
		rapidjson::Value(_val).Move(), 
		m_impl->m_dom.GetAllocator()
		);
	m_impl->m_value = m_impl->top()->End() - 1;
}
template <> void Json::pushValue<sint16>(sint16 _val)
{
	pushValue<sint32>((sint32)_val);
}
template <> void Json::pushValue<sint8>(sint8 _val)
{
	pushValue<sint32>((sint32)_val);
}
template <> void Json::pushValue<uint64>(uint64 _val)
{
	m_impl->top()->PushBack(
		rapidjson::Value(_val).Move(), 
		m_impl->m_dom.GetAllocator()
		);
	m_impl->m_value = m_impl->top()->End() - 1;
}
template <> void Json::pushValue<uint32>(uint32 _val)
{
	m_impl->top()->PushBack(
		rapidjson::Value(_val).Move(), 
		m_impl->m_dom.GetAllocator()
		);
	m_impl->m_value = m_impl->top()->End() - 1;
}
template <> void Json::pushValue<uint16>(uint16 _val)
{
	pushValue((uint32)_val);
}
template <> void Json::pushValue<uint8>(uint8 _val)
{
	pushValue((uint32)_val);
}
template <> void Json::pushValue<float32>(float32 _val)
{
	m_impl->top()->PushBack(
		rapidjson::Value(_val).Move(), 
		m_impl->m_dom.GetAllocator()
		);
	m_impl->m_value = m_impl->top()->End() - 1;
}
template <> void Json::pushValue<float64>(float64 _val)
{
	m_impl->top()->PushBack(
		rapidjson::Value(_val).Move(), 
		m_impl->m_dom.GetAllocator()
		);
	m_impl->m_value = m_impl->top()->End() - 1;
}
template <> void Json::pushValue<const char*>(const char* _val)
{
	m_impl->top()->PushBack(
		rapidjson::Value().SetString(_val, m_impl->m_dom.GetAllocator()).Move(),
		m_impl->m_dom.GetAllocator()
		);
	m_impl->m_value = m_impl->top()->End() - 1;
}


template <> void Json::setValue<vec2>(const char* _name, vec2 _val)
{
	beginArray(_name);
		pushValue(_val.x);
		pushValue(_val.y);
	leaveArray();
}
template <> void Json::setValue<vec3>(const char* _name, vec3 _val)
{
	beginArray(_name);
		pushValue(_val.x);
		pushValue(_val.y);		
		pushValue(_val.z);
	leaveArray();
}
template <> void Json::setValue<vec4>(const char* _name, vec4 _val)
{
	beginArray(_name);
		pushValue(_val.x);
		pushValue(_val.y);		
		pushValue(_val.z);				
		pushValue(_val.w);
	leaveArray();
}
template <> void Json::pushValue<vec2>(vec2 _val)
{
	beginArray();
		pushValue(_val.x);
		pushValue(_val.y);
	leaveArray();
}
template <> void Json::pushValue<vec3>(vec3 _val)
{
	beginArray();
		pushValue(_val.x);
		pushValue(_val.y);		
		pushValue(_val.z);
	leaveArray();
}
template <> void Json::pushValue<vec4>(vec4 _val)
{
	beginArray();
		pushValue(_val.x);
		pushValue(_val.y);		
		pushValue(_val.z);				
		pushValue(_val.w);
	leaveArray();
}

template <> void Json::setValue<mat2>(const char* _name, mat2 _val)
{
	beginArray(_name);
		for (int i = 0; i < 2; ++i) {
			pushValue<vec2>(_val[i]);
		}
	leaveArray();
}
template <> void Json::setValue<mat3>(const char* _name, mat3 _val)
{
	beginArray(_name);
		for (int i = 0; i < 3; ++i) {
			pushValue<vec3>(_val[i]);
		}
	leaveArray();
}
template <> void Json::setValue<mat4>(const char* _name, mat4 _val)
{
	beginArray(_name);
		for (int i = 0; i < 4; ++i) {
			pushValue<vec4>(_val[i]);
		}
	leaveArray();
}
template <> void Json::pushValue<mat2>(mat2 _val)
{
	beginArray();
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				pushValue(_val[i][j]);
			}
		}
	leaveArray();
}
template <> void Json::pushValue<mat3>(mat3 _val)
{
	beginArray();
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				pushValue(_val[i][j]);
			}
		}
	leaveArray();
}
template <> void Json::pushValue<mat4>(mat4 _val)
{
	beginArray();
		for (int i = 0; i < 4; ++i) {
			pushValue<vec4>(_val[i]);
		}
	leaveArray();
}

/*******************************************************************************

                              SerializerJson

*******************************************************************************/

// PUBLIC

SerializerJson::SerializerJson(Json* _json_, Mode _mode)
	: Serializer(_mode) 
	, m_json(_json_)
{
}

bool SerializerJson::beginObject(const char* _name)
{
	if (getMode() == Mode_Read) {
		if (m_json->getArrayLength() >= 0) { // inside array
			if (!m_json->next()) {
				return false;
			}
		} else {
			APT_ASSERT(_name);
			if (!m_json->find(_name)) {
				setError("SerializerJson::beginObject(); '%s' not found", _name);
				return false;
			}
		}
		if (m_json->getType() == Json::ValueType_Object) {
			m_json->enterObject();
			return true;
		} else {
			setError("SerializerJson::beginObject(); '%s' not an object", _name ? _name : "");
			return false;
		}
	} else {
		m_json->beginObject(_name);
		return true;
	}
}
void SerializerJson::endObject()
{
	if (m_mode == Mode_Read) {
		m_json->leaveObject();
	} else {
		m_json->endObject();
	}
}

bool SerializerJson::beginArray(uint& _length_, const char* _name)
{
	if (m_mode == Mode_Read) {
		if (m_json->getArrayLength() >= 0) { // inside array
			if (!m_json->next()) {
				return false;
			}
		} else {
			APT_ASSERT(_name);
			if (!m_json->find(_name)) {
				setError("SerializerJson::beginArray(); '%s' not found", _name);
				return false;
			}
		}
		if (m_json->getType() == Json::ValueType_Array) {
			m_json->enterArray();
			_length_ = (uint)m_json->getArrayLength();
			return true;
		} else {
			setError("SerializerJson::beginArray(); '%s' not an array", _name ? _name : "");
			return false;
		}
	} else {
		m_json->beginArray(_name);
		return true;
	}
}
void SerializerJson::endArray()
{
	if (m_mode == Mode_Read) {
		m_json->leaveArray();
	} else {
		m_json->endArray();
	}
}

template <typename tType>
static bool ValueImpl(SerializerJson& _serializer_, tType& _value_, const char* _name)
{
	Json* json = _serializer_.getJson();
	if (!_name && json->getArrayLength() == -1) {
		_serializer_.setError("Error serializing %s; name must be specified if not in an array", Serializer::ValueTypeToStr<tType>());
		return false;
	}
	if (_serializer_.getMode() == SerializerJson::Mode_Read) {
		if (_name) {
			if (!json->find(_name)) {
				_serializer_.setError("Error serializing %s; '%s' not found", Serializer::ValueTypeToStr<tType>(), _name);
				return false;
			}
		} else {
			if (!json->next()) {
				return false;
			}
		}
		_value_ = json->getValue<tType>();
		return true;

	} else {
		if (_name) {
			json->setValue<tType>(_name, _value_);
		} else {
			json->pushValue<tType>(_value_);
		}
		return true; 
	}
}

bool SerializerJson::value(bool&    _value_, const char* _name) { return ValueImpl<bool>   (*this, _value_, _name); }
bool SerializerJson::value(sint8&   _value_, const char* _name) { return ValueImpl<sint8>  (*this, _value_, _name); }
bool SerializerJson::value(uint8&   _value_, const char* _name) { return ValueImpl<uint8>  (*this, _value_, _name); }
bool SerializerJson::value(sint16&  _value_, const char* _name) { return ValueImpl<sint16> (*this, _value_, _name); }
bool SerializerJson::value(uint16&  _value_, const char* _name) { return ValueImpl<uint16> (*this, _value_, _name); }
bool SerializerJson::value(sint32&  _value_, const char* _name) { return ValueImpl<sint32> (*this, _value_, _name); }
bool SerializerJson::value(uint32&  _value_, const char* _name) { return ValueImpl<uint32> (*this, _value_, _name); }
bool SerializerJson::value(sint64&  _value_, const char* _name) { return ValueImpl<sint64> (*this, _value_, _name); }
bool SerializerJson::value(uint64&  _value_, const char* _name) { return ValueImpl<uint64> (*this, _value_, _name); }
bool SerializerJson::value(float32& _value_, const char* _name) { return ValueImpl<float32>(*this, _value_, _name); }
bool SerializerJson::value(float64& _value_, const char* _name) { return ValueImpl<float64>(*this, _value_, _name); }

bool SerializerJson::value(StringBase& _value_, const char* _name) 
{ 
	if (!_name && m_json->getArrayLength() == -1) {
		setError("Error serializing StringBase; name must be specified if not in an array");
		return false;
	}
	if (getMode() == SerializerJson::Mode_Read) {
		if (_name) {
			if (!m_json->find(_name)) {
				setError("Error serializing StringBase; '%s' not found", _name);
				return false;
			}
		} else {
			if (!m_json->next()) {
				return false;
			}
		}

		if (m_json->getType() == Json::ValueType_String) {
			_value_.set(m_json->getValue<const char*>());
			return true;
		} else {
			setError("Error serializing StringBase; '%s' not a string", _name ? _name : "");
			return false;
		}

	} else {
		if (_name) {
			m_json->setValue<const char*>(_name, (const char*)_value_);
		} else {
			m_json->pushValue<const char*>((const char*)_value_);
		}
		return true; 
	}
}

