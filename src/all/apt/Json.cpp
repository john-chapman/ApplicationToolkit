#include <apt/Json.h>

#include <apt/log.h>
#include <apt/math.h>
#include <apt/memory.h>
#include <apt/FileSystem.h>
#include <apt/String.h>
#include <apt/Time.h>

#include <EASTL/vector.h>

#define RAPIDJSON_ASSERT(x) APT_ASSERT(x)
#define RAPIDJSON_PARSE_DEFAULT_FLAGS (rapidjson::kParseFullPrecisionFlag | rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag)
#include <rapidjson/error/en.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#define JSON_ERR_TYPE(_call, _name, _type, _expected, _onFail) \
	if (_type != _expected) { \
		APT_LOG_ERR("Json: (%s) %s has type %s, expected %s", _call, _name, GetValueTypeString(_type), GetValueTypeString(_expected)); \
		_onFail; \
	}
#define JSON_ERR_SIZE(_call, _name, _size, _expected, _onFail) \
	if (_size != _expected) { \
		APT_LOG_ERR("Json: (%s) %s has size %d, expected %d", _call, _name, _size, _expected); \
		_onFail; \
	}
#define JSON_ERR_ARRAY_SIZE(_call, _name, _index, _arraySize, _onFail) \
	if (_index >= _arraySize) { \
		APT_LOG_ERR("Json: (%s) %s index out of bounds, %d/%d", _call, _name, _index, _arraySize - 1); \
		_onFail; \
	}

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

static const char* GetValueTypeString(Json::ValueType _type)
{
	switch (_type) {
		case Json::ValueType_Null:    return "Null";
		case Json::ValueType_Object:  return "Object";
		case Json::ValueType_Array:   return "Array";
		case Json::ValueType_Bool:    return "Bool";
		case Json::ValueType_Number:  return "Number";
		case Json::ValueType_String:  return "String";
		default:                      return "Unknown";
	};
}

/*******************************************************************************

                                   Json

*******************************************************************************/

/*	Traversal of the dom:
		- The top of the stack is the current object/array.
		- Also store an index on the stack which points to the current position within
		  the object for the next() operation to work.

	\todo
	- m_impl is hard to reason about, need to distil the ideas a bit:
		- Stack is for container objects only, maintain a separate Value instance for the
		  position *within* the current container. Need to support json whose root isn't an
		  object?
		- goto() moves within the current container - takes either a name or an index. ONLY
		  after calling goto() can you call enter/leave, get/set etc.
	- Only access rapidjson via the Impl class?
*/
struct Json::Impl
{
	rapidjson::Document m_dom;

	struct Value
	{
		rapidjson::Value* m_value  = nullptr; // current container (array or object)
		const char*       m_name   = "";      // name of current element (if in an object)
		int               m_index  = -1;      // index of the current element within the container
	};
	eastl::vector<Value> m_stack;

	rapidjson::Value* current()
	{
		auto& top = m_stack.back();
		if (top.m_index < 0) {
			return top.m_value;
		}
		auto topType = GetValueType(top.m_value->GetType());
		if (topType == ValueType_Array) {
			JSON_ERR_ARRAY_SIZE("current()", top.m_name, top.m_index, (int)top.m_value->GetArray().Size(), return nullptr);
			return top.m_value->Begin() + top.m_index;
		} else if (topType == ValueType_Object) {
			JSON_ERR_ARRAY_SIZE("current()", top.m_name, top.m_index, (int)top.m_value->GetObject().MemberCount(), return nullptr);
			return &(top.m_value->MemberBegin() + top.m_index)->value;
		}
		return nullptr;
	}

	const char* currentName()
	{
		return m_stack.back().m_name;
	}

	ValueType currentType()
	{
		auto top = current();
		if (!top) {
			return ValueType_Count;
		}
		return GetValueType(top->GetType());
	}

	void enter()
	{
		auto topType = currentType();
		APT_ASSERT(topType == ValueType_Object || topType == ValueType_Array);
		m_stack.push_back({ current(), currentName(), -1 });
	}

	void leave()
	{
		m_stack.pop_back();
	}
	
	rapidjson::Value* find(const char* _name)
	{
		auto top = m_stack.back().m_value;
		if (!top->IsObject()) {
			return nullptr;
		}
		auto it = top->FindMember(_name);
		if (it != top->MemberEnd()) {
			m_stack.back().m_index = (int)(it - top->MemberBegin());
			m_stack.back().m_name  = it->name.GetString();
			return &it->value;
		}
		return nullptr;
	}

	rapidjson::Value* get(Json::ValueType _expectedType, int _i = -1)
	{
		rapidjson::Value* ret = current();
		if (_i >= 0 && GetValueType(ret->GetType()) == ValueType_Array) {
			int n = (int)ret->GetArray().Size();
			JSON_ERR_ARRAY_SIZE("get", currentName(), _i, n, return ret);
			ret = &ret->GetArray()[_i];
		}
		return ret;
	}

	bool getBool(int _i)
	{
		return get(ValueType_Bool, _i)->GetBool();
	}

	const char* getString(int _i)
	{
		return get(ValueType_String, _i)->GetString();
	}

	template <typename T>
	T getNumber(int _i)
	{
		switch (APT_DATA_TYPE_TO_ENUM(T)) {
			default:
			case DataType_Uint8:
			case DataType_Uint8N:
			case DataType_Uint16:
			case DataType_Uint16N:
			case DataType_Uint32:
			case DataType_Uint32N:
				return (T)get(ValueType_Number, _i)->GetUint();
			case DataType_Uint64:
			case DataType_Uint64N:
				return (T)get(ValueType_Number, _i)->GetUint64();
			case DataType_Sint8:
			case DataType_Sint8N:
			case DataType_Sint16:
			case DataType_Sint16N:
			case DataType_Sint32:
			case DataType_Sint32N:
				return (T)get(ValueType_Number, _i)->GetInt();
			case DataType_Sint64:
			case DataType_Sint64N:
				return (T)get(ValueType_Number, _i)->GetUint();
			case DataType_Float16: 
				return (T)PackFloat16(get(ValueType_Number, _i)->GetFloat());
			case DataType_Float32:
				return (T)get(ValueType_Number, _i)->GetFloat();
			case DataType_Float64:
				return (T)get(ValueType_Number, _i)->GetDouble();
		};
	}

	template <typename T>
	T getVector(int _i)
	{
	 // vectors are arrays of numbers
		T ret = T(APT_TRAITS_BASE_TYPE(T)(0));
		auto val = get(ValueType_Array, _i);
		auto& arr = val->GetArray();
		JSON_ERR_SIZE("getVector", currentName(), (int)arr.Size(), APT_TRAITS_COUNT(T), return ret);
		JSON_ERR_TYPE("getVector", currentName(), GetValueType(arr[0].GetType()), ValueType_Number, return ret);

		typedef APT_TRAITS_BASE_TYPE(T) BaseType;
		if (DataTypeIsFloat(APT_DATA_TYPE_TO_ENUM(BaseType))) {
			for (int i = 0; i < APT_TRAITS_COUNT(T); ++i) {
				ret[i] = (BaseType)arr[i].GetFloat();
			}
		} else {
			if (DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(BaseType))) {
				for (int i = 0; i < APT_TRAITS_COUNT(T); ++i) {
					ret[i] = (BaseType)arr[i].GetInt();
				}
			} else {
				for (int i = 0; i < APT_TRAITS_COUNT(T); ++i) {
					ret[i] = (BaseType)arr[i].GetUint();
				}
			}
		}		
		return ret;
	}

	template <typename T, int kCount> // \hack APT_TRAITS_COUNT(T) returns the # of floats in the matrix, hence kCount for the # of vectors
	T getMatrix(int _i)
	{
	 // matrices are arrays of vectors
		auto val = get(ValueType_Array, _i);
		auto& arrVecs = val->GetArray();
		JSON_ERR_SIZE("getMatrix", currentName(), (int)arrVecs.Size(), kCount, return identity);
		JSON_ERR_TYPE("getMatrix", currentName(), GetValueType(arrVecs[0].GetType()), ValueType_Array, return identity);

		T ret;
		for (int i = 0; i < kCount; ++i) {
			for (int j = 0; j < kCount; ++j) { // only square matrices supported
				auto& vec = arrVecs[i];
				JSON_ERR_SIZE("getMatrix", currentName(), (int)vec.Size(), kCount, return identity);
				ret[i][j] = vec[j].GetFloat();
			}
		}		
		return ret;
	}

	rapidjson::Value* findOrAdd(const char* _name)
	{
		auto top = m_stack.back().m_value;
		JSON_ERR_TYPE("findOrAdd", m_stack.back().m_name, GetValueType(top->GetType()), ValueType_Object, return nullptr);
		auto ret = find(_name);
		if (!ret) {
			m_stack.back().m_index = (int)top->MemberCount();
			top->AddMember(
				rapidjson::StringRef(_name),
				rapidjson::Value().Move(), 
				m_dom.GetAllocator()
				);
			ret = &(top->MemberEnd() - 1)->value;
			m_stack.back().m_name  = (top->MemberEnd() - 1)->name.GetString();
		}
		return ret;
	}
	rapidjson::Value* findOrAdd(int _i)
	{
		auto top = m_stack.back().m_value;
		JSON_ERR_TYPE("findOrAdd", m_stack.back().m_name, GetValueType(top->GetType()), ValueType_Array, return nullptr);
		int n = (int)top->GetArray().Size();
		JSON_ERR_ARRAY_SIZE("findOrAdd", m_stack.back().m_name, _i, n, return nullptr);
		return top->Begin() + _i;
	}
	rapidjson::Value* findOrAdd(const char* _name, int _i)
	{
		if (_name) {
			return findOrAdd(_name);
		} else if (_i >= 0) {
			return findOrAdd(_i);
		} else {
			return current();
		}
	}
	rapidjson::Value* pushNew()
	{
		auto top = m_stack.back().m_value;
		JSON_ERR_TYPE("pushNew",  m_stack.back().m_name, GetValueType(top->GetType()), ValueType_Array, return nullptr);
		top->PushBack(rapidjson::Value().Move(), m_dom.GetAllocator());
		auto ret = top->End() - 1;
		m_stack.back().m_index = (int)top->GetArray().Size() - 1;
		return ret;
	}

	void setBool(bool _value, const char* _name, int _i)
	{
		auto val = findOrAdd(_name, _i);
		if (val) {
			val->SetBool(_value);
		}
	}

	void setString(const char* _value, const char* _name, int _i)
	{
		auto val = findOrAdd(_name, _i);
		if (val) {
			val->SetString(_value, m_dom.GetAllocator());
		}
	}
	
	template <typename T>
	void setNumber(T _value, const char* _name, int _i)
	{
		auto val = findOrAdd(_name, _i);
		if (val) {
			switch (APT_DATA_TYPE_TO_ENUM(T)) {
				default:
				case DataType_Uint8:
				case DataType_Uint8N:
				case DataType_Uint16:
				case DataType_Uint16N:
				case DataType_Uint32:
				case DataType_Uint32N:
					val->SetUint((uint32)_value);
					break;
				case DataType_Uint64:
				case DataType_Uint64N:
					val->SetUint64((uint64)_value);
					break;
				case DataType_Sint8:
				case DataType_Sint8N:
				case DataType_Sint16:
				case DataType_Sint16N:
				case DataType_Sint32:
				case DataType_Sint32N:
					val->SetInt((sint32)_value);
					break;
				case DataType_Sint64:
				case DataType_Sint64N:
					val->SetInt64((sint64)_value);
					break;
				case DataType_Float16: 
					val->SetFloat(UnpackFloat16((uint16)_value));
					break;
				case DataType_Float32:
					val->SetFloat((float)_value);
					break;
				case DataType_Float64:
					val->SetDouble((double)_value);
					break;
			};
		}
	}

	template <typename T>
	void setVector(const T& _value, const char* _name, int _i)
	{
		auto val = findOrAdd(_name, _i);
		if (val) {
			int n = APT_TRAITS_COUNT(T);
			val->SetArray();
			for (int i = 0; i < n; ++i) {
				val->PushBack(_value[i], m_dom.GetAllocator());
			}
		}
	}

	template <typename T, int kCount> // \hack APT_TRAITS_COUNT(T) returns the # of floats in the matrix, hence kCount for the # of vectors
	void setMatrix(const T& _value, const char* _name, int _i)
	{
		auto arrVecs = findOrAdd(_name, _i);
		if (arrVecs) {
			arrVecs->SetArray();
			for (int i = 0; i < kCount; ++i) {
				arrVecs->PushBack(rapidjson::Value().SetArray().Move(), m_dom.GetAllocator());
				auto& vec = *(arrVecs->End() - 1);
				for (int j = 0; j < kCount; ++j) {
					vec.PushBack(_value[i][j], m_dom.GetAllocator());
				}
			}
		}
	}

	void begin(ValueType _type, const char* _name)
	{
		APT_STRICT_ASSERT(_type == ValueType_Object || _type == ValueType_Array);
		auto rapidJsonType = _type == ValueType_Object ? rapidjson::kObjectType : rapidjson::kArrayType;

		auto top = m_stack.back().m_value;
		if (_name) {
			auto ret = find(_name);
			if (ret) {
				APT_ASSERT(GetValueType(ret->GetType()) == _type);
				m_stack.push_back({ ret, m_stack.back().m_name, -1 });
				return;
			}
		} 
		rapidjson::Value* ret;
		int i = -1;
		if (top->GetType() == rapidjson::kArrayType) {
			if (_name) {
				APT_LOG("Json: calling begin() in an array, name '%s' will be ignored", _name);
			}
			i = (int)top->Size();
			top->PushBack(
				rapidjson::Value(rapidJsonType).Move(), 
				m_dom.GetAllocator()
				);
			ret = top->End() - 1;

		} else {
			i = (int)top->MemberCount();
			top->AddMember(
				rapidjson::StringRef(_name),
				rapidjson::Value(rapidJsonType).Move(), 
				m_dom.GetAllocator()
				);
			ret = &(top->MemberEnd() - 1)->value;
		}
	
		m_stack.back().m_index = i; // we're about to enter the new object, set the back index as required
		if (m_stack.back().m_value == nullptr) {
		 // stack top is invalid, this means we want to replace it below 
			m_stack.pop_back();
		}
		m_stack.push_back({ ret, _name ? _name : "", -1 });
	}

};


bool Json::Read(Json& json_, const File& _file)
{
	json_.m_impl->m_dom.Parse(_file.getData());
	if (json_.m_impl->m_dom.HasParseError()) {
		APT_LOG_ERR("Json: %s\n\t'%s'", _file.getPath(), rapidjson::GetParseError_En(json_.m_impl->m_dom.GetParseError()));
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
	m_impl = APT_NEW(Impl);
	m_impl->m_dom.SetObject();	
	m_impl->m_stack.push_back({ &m_impl->m_dom, "", -1 });
	if (_path) {
		Json::Read(*this, _path, _rootHint);
	}
}

Json::~Json()
{
	if (m_impl) {
		APT_DELETE(m_impl);
	}
}

bool Json::find(const char* _name)
{
	return m_impl->find(_name);
}
	
bool Json::next()
{
	auto& current = m_impl->m_stack.back();
	++current.m_index;
	ValueType type = GetValueType(current.m_value->GetType());
	int n = -1;
	if (type == ValueType_Object) {
		n = current.m_value->MemberCount();
		if (current.m_index < n) {
			auto it = current.m_value->MemberBegin() + current.m_index;
			current.m_name = it->name.GetString();
		}
	} else {
		n = current.m_value->Size();
	}
	return current.m_index < n;
}

bool Json::enterObject()
{
	APT_ASSERT(!m_impl->m_stack.empty());
	JSON_ERR_TYPE("enterObject", getName(), getType(), ValueType_Object, return false);
	m_impl->enter();
	return true;
}

void Json::leaveObject()
{
	m_impl->leave();
	APT_ASSERT(getType() == ValueType_Object);
}

bool Json::enterArray()
{
	APT_ASSERT(!m_impl->m_stack.empty());
	JSON_ERR_TYPE("enterArray", getName(), getType(), ValueType_Array, return false);
	m_impl->enter();
	return true;
}

void Json::leaveArray()
{
	m_impl->leave();
}

Json::ValueType Json::getType() const
{
	return m_impl->currentType();
}

const char* Json::getName() const
{
	return m_impl->currentName();
}

int Json::getArrayLength() const
{
	if (getType() == ValueType_Array) {
		return (int)m_impl->current()->GetArray().Size();
	}
	return -1;
}

template <> bool Json::getValue<bool>(int _i) const
{
	return m_impl->getBool(_i);
}

template <> const char* Json::getValue<const char*>(int _i) const
{
	return m_impl->getString(_i);
}

// use the APT_DataType_decl macro to instantiate get<>() for all the number types
#define Json_getValue_Number(_type, _enum) \
	template <> _type Json::getValue<_type>(int _i) const { \
		return m_impl->getNumber<_type>(_i); \
	}
APT_DataType_decl(Json_getValue_Number)

// manually instantiate vector types
#define Json_getValue_Vector(_type) \
	template <> _type Json::getValue<_type>(int _i) const { \
		 return m_impl->getVector<_type>(_i); \
	}
Json_getValue_Vector(vec2)
Json_getValue_Vector(vec3)
Json_getValue_Vector(vec4)
Json_getValue_Vector(ivec2)
Json_getValue_Vector(ivec3)
Json_getValue_Vector(ivec4)
Json_getValue_Vector(uvec2)
Json_getValue_Vector(uvec3)
Json_getValue_Vector(uvec4)

// manually instantiate matrix types
#define Json_getValue_Matrix(_type, _count) \
	template <> _type Json::getValue<_type>(int _i) const { \
		 return m_impl->getMatrix<_type, _count>(_i); \
	}
Json_getValue_Matrix(mat2, 2)
Json_getValue_Matrix(mat3, 3)
Json_getValue_Matrix(mat4, 4)


template <> void Json::setValue<bool>(bool _value, int _i)
{
	m_impl->setBool(_value, nullptr, _i);
}
template <> void Json::setValue<bool>(bool _value, const char* _name)
{
	m_impl->setBool(_value, _name, -1);
}
template <> void Json::setValue<const char*>(const char* _value, int _i)
{
	m_impl->setString(_value, nullptr, _i);
}
template <> void Json::setValue<const char*>(const char* _value, const char* _name)
{
	m_impl->setString(_value, _name, -1);
}
#define Json_setValue_Number(_type, _enum) \
	template <> void Json::setValue<_type>(_type _value, int _i) { \
		m_impl->setNumber<_type>(_value, nullptr, _i); \
	} \
	template <> void Json::setValue<_type>(_type _value, const char* _name) { \
		m_impl->setNumber<_type>(_value, _name, -1); \
	}
APT_DataType_decl(Json_setValue_Number)

#define Json_setValue_Vector(_type) \
	template <> void Json::setValue<_type>(_type _value, int _i) { \
		m_impl->setVector<_type>(_value, nullptr, _i); \
	} \
	template <> void Json::setValue<_type>(_type _value, const char* _name) { \
		m_impl->setVector<_type>(_value, _name, -1); \
	}
Json_setValue_Vector(vec2)
Json_setValue_Vector(vec3)
Json_setValue_Vector(vec4)
Json_setValue_Vector(ivec2)
Json_setValue_Vector(ivec3)
Json_setValue_Vector(ivec4)
Json_setValue_Vector(uvec2)
Json_setValue_Vector(uvec3)
Json_setValue_Vector(uvec4)


#define Json_setValue_Matrix(_type, _count) \
	template <> void Json::setValue<_type>(_type _value, int _i) { \
		m_impl->setMatrix<_type, _count>(_value, nullptr, _i); \
	} \
	template <> void Json::setValue<_type>(_type _value, const char* _name) { \
		m_impl->setMatrix<_type, _count>(_value, _name, -1); \
	}
Json_setValue_Matrix(mat2, 2)
Json_setValue_Matrix(mat3, 3)
Json_setValue_Matrix(mat4, 4)

#define Json_pushValue(_type, _enum) \
	template <> void Json::pushValue<_type>(_type _value) { \
		m_impl->pushNew(); \
		setValue(_value); \
	}
Json_pushValue(bool, 0)
Json_pushValue(const char*, 0)
APT_DataType_decl(Json_pushValue)
Json_pushValue(vec2, 0)
Json_pushValue(vec3, 0)
Json_pushValue(vec4, 0)
Json_pushValue(ivec2, 0)
Json_pushValue(ivec3, 0)
Json_pushValue(ivec4, 0)
Json_pushValue(uvec2, 0)
Json_pushValue(uvec3, 0)
Json_pushValue(uvec4, 0)
Json_pushValue(mat2, 0)
Json_pushValue(mat3, 0)
Json_pushValue(mat4, 0)

void Json::beginObject(const char* _name)
{
	m_impl->begin(ValueType_Object, _name);
}

void Json::beginArray(const char* _name)
{
	m_impl->begin(ValueType_Array, _name);
}

/*******************************************************************************

                              SerializerJson

*******************************************************************************/

// PUBLIC

SerializerJson::SerializerJson(Json& _json_, Mode _mode)
	: Serializer(_mode) 
	, m_json(&_json_)
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
		if (_name) {
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
	}
	return true;
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
			json->setValue<tType>(_value_, _name);
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


// Base64 encode/decode of binary data, adapted from https://github.com/adamvr/arduino-base64
static const char kBase64Alphabet[] = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/"
		;
static inline void Base64A3ToA4(const unsigned char* _a3, unsigned char* a4_) 
{
	a4_[0] = (_a3[0] & 0xfc) >> 2;
	a4_[1] = ((_a3[0] & 0x03) << 4) + ((_a3[1] & 0xf0) >> 4);
	a4_[2] = ((_a3[1] & 0x0f) << 2) + ((_a3[2] & 0xc0) >> 6);
	a4_[3] = (_a3[2] & 0x3f);
}
static inline void Base64A4ToA3(const unsigned char* _a4, unsigned char* a3_) {
	a3_[0] = (_a4[0] << 2) + ((_a4[1] & 0x30) >> 4);
	a3_[1] = ((_a4[1] & 0xf) << 4) + ((_a4[2] & 0x3c) >> 2);
	a3_[2] = ((_a4[2] & 0x3) << 6) + _a4[3];
}
static inline unsigned char Base64Index(char _c)
{
	if (_c >= 'A' && _c <= 'Z') return _c - 'A';
	if (_c >= 'a' && _c <= 'z') return _c - 71;
	if (_c >= '0' && _c <= '9') return _c + 4;
	if (_c == '+') return 62;
	if (_c == '/') return 63;
	return -1;
}
static void Base64Encode(const char* _in, uint _inSizeBytes, char* out_, uint outSizeBytes_)
{
	uint i = 0;
	uint j = 0;
	uint k = 0;
	unsigned char a3[3];
	unsigned char a4[4];
	while (_inSizeBytes--) {
		a3[i++] = *(_in++);
		if (i == 3) {
			Base64A3ToA4(a3, a4);
			for (i = 0; i < 4; i++) {
				out_[k++] = kBase64Alphabet[a4[i]];
			}
			i = 0;
		}
	}
	if (i) {
		for (j = i; j < 3; j++) {
			a3[j] = '\0';
		}
		Base64A3ToA4(a3, a4);
		for (j = 0; j < i + 1; j++) {
			out_[k++] = kBase64Alphabet[a4[j]];
		}
		while ((i++ < 3)) {
			out_[k++] = '=';
		}
	}
	out_[k] = '\0';
	APT_ASSERT(outSizeBytes_ == k); // overflow
}
static void Base64Decode(const char* _in, uint _inSizeBytes, char* out_, uint outSizeBytes_) {
	uint i = 0;
	uint j = 0;
	uint k = 0;
	unsigned char a3[3];
	unsigned char a4[4];
	while (_inSizeBytes--) {
		if (*_in == '=') {
			break;
		}
		a4[i++] = *(_in++);
		if (i == 4) {
			for (i = 0; i < 4; i++) {
				a4[i] = Base64Index(a4[i]);
			}
			Base64A4ToA3(a4, a3);
			for (i = 0; i < 3; i++) {
				out_[k++] = a3[i];
			}
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++) {
			a4[j] = '\0';
		}
		for (j = 0; j < 4; j++) {
			a4[j] = Base64Index(a4[j]);
		}
		Base64A4ToA3(a4, a3);
		for (j = 0; j < i - 1; j++) {
			out_[k++] = a3[j];
		}
	}
	APT_ASSERT(outSizeBytes_ == k); // overflow
}
static uint Base64EncSizeBytes(uint _sizeBytes) 
{
	uint n = _sizeBytes;
	return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}
static uint Base64DecSizeBytes(char* _buf, uint _sizeBytes) 
{
	uint padCount = 0;
	for (uint i = _sizeBytes - 1; _buf[i] == '='; i--) {
		padCount++;
	}
	return ((6 * _sizeBytes) / 8) - padCount;
}

bool SerializerJson::binary(void*& _data_, uint& _sizeBytes_, const char* _name, CompressionFlags _compressionFlags)
{
	if (getMode() == Mode_Write) {
		APT_ASSERT(_data_);
		char* data = (char*)_data_;
		uint sizeBytes = _sizeBytes_;
		if (_compressionFlags != CompressionFlags_None) {
			data = nullptr;
			Compress(_data_, _sizeBytes_, (void*&)data, sizeBytes, _compressionFlags);
		}
		String<0> str;
		str.setLength(Base64EncSizeBytes(sizeBytes) + 1);
		str[0] = _compressionFlags == CompressionFlags_None ? '0' : '1'; // prepend 0, or 1 if compression
		Base64Encode(data, sizeBytes, (char*)str + 1, str.getLength() - 1);
		if (_compressionFlags != CompressionFlags_None) {
			free(data);
		}
		value((StringBase&)str, _name);

	} else {
		String<0> str;
		value((StringBase&)str, _name);
		bool compressed = str[0] == '1' ? true : false;
		uint binSizeBytes = Base64DecSizeBytes((char*)str + 1, str.getLength() - 1);
		char* bin = (char*)APT_MALLOC(binSizeBytes);
		Base64Decode((char*)str + 1, str.getLength() - 1, bin, binSizeBytes);

		char* ret = bin;
		uint retSizeBytes = binSizeBytes;
		if (compressed) {
			ret = nullptr; // Decompress to allocates the final buffer
			Decompress(bin, binSizeBytes, (void*&)ret, retSizeBytes);
		}
		if (_data_) {
			if (retSizeBytes != _sizeBytes_) {
				setError("Error serializing %s, buffer size was %llu (expected %llu)", _sizeBytes_, retSizeBytes);
				if (compressed) {
					APT_FREE(ret);
				}
				return false;
			}
			memcpy(_data_, ret, retSizeBytes);
			if (compressed) {
				APT_FREE(ret);
			}
		} else {
			_data_ = ret;
			_sizeBytes_ = retSizeBytes;
		}
	}
	return true;
}
