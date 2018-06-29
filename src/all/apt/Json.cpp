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

#define JSON_LOG_ERR_TYPE(_call, _name, _type, _expected, _onFail) \
	if (_type != _expected) { \
		APT_LOG_ERR("Json: (%s) %s has type %s, expected %s", _call, _name, GetValueTypeString(_type), GetValueTypeString(_expected)); \
		_onFail; \
	}
#define JSON_LOG_ERR_SIZE(_call, _name, _size, _expected, _onFail) \
	if (_size != _expected) { \
		APT_LOG_ERR("Json: (%s) %s has size %d, expected %d", _call, _name, _size, _expected); \
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

/*	Consider the stack top as a read-head. Calling enter()/leave() pushes/pops
	the stack top, calling next() or find() overwrites the stack top. This could work
	but will break existing code:
	   enterArray()
	      while (next()) {
	      }
	   leaveArray();
	After entering the array, the first call to next() is expected to move the stack top to
	the 0th array element. This would mean that enterArray() needs to set the stack top to
	some sort of invalid value (index -1).

	\todo
	- Only access rapidjson via the Impl class
*/
struct Json::Impl
{
	rapidjson::Document m_dom;

	struct Value
	{
		rapidjson::Value* m_value;
		const char*       m_name;
		int               m_index;
	};
	eastl::vector<Value> m_stack;

	Impl()
	{
		m_stack.push_back({ nullptr, "", -1});
	}

	auto& top()
	{
		return m_stack.back();
	}
	auto& topIndex()
	{
		return top().m_index;
	}
	auto& topName()
	{
		return top().m_name;
	}
	auto& topValue()
	{
		return top().m_value;
	}
	auto topType()
	{
		return GetValueType(top().m_value->GetType());
	}
	auto& setTop(rapidjson::Value* _value, const char* _name = "", int _index = -1)
	{
		m_stack.back() = { _value, _name, _index };
		return m_stack.back();
	}

	auto& parent()
	{
		APT_STRICT_ASSERT(m_stack.size() > 1);
		return *(m_stack.end() - 2);
	}

	void push()
	{
		m_stack.push_back({ nullptr, "", -1 });
	}
	void pop()
	{
		m_stack.pop_back();
	}
	
	auto get(Json::ValueType _expectedType, int _i = -1)
	{
		rapidjson::Value* ret = m_stack.back().m_value;
		if (_i >= 0) {
			if (GetValueType(ret->GetType()) == ValueType_Array) {
				int n = (int)ret->GetArray().Size();
				APT_ASSERT_MSG(_i < n, "Json: array index out of bounds (%d/%d)", _i, n);
				ret = &ret->GetArray()[_i];
			}
		}
		JSON_LOG_ERR_TYPE("get", topName(), GetValueType(ret->GetType()), _expectedType, ;);
		return ret;
	}

	bool getBool(int _i)
	{
		return get(ValueType_Bool, _i)->GetBool();
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
			case DataType_Float16: APT_ASSERT(false); // \todo
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
		auto val = get(_i);
		auto valType = GetValueType(val->GetType());
		JSON_LOG_ERR_TYPE("getVector", topName(), valType, ValueType_Array, return ret);
		auto& arr = val->GetArray();
		JSON_LOG_ERR_SIZE("getVector", topName(), (int)arr.Size(), APT_TRAITS_COUNT(T), return ret);
		JSON_LOG_ERR_TYPE("getVector", topName(), GetValueType(arr[0].GetType()), ValueType_Number, return ret);

		if (DataTypeIsFloat(APT_DATA_TYPE_TO_ENUM(APT_TRAITS_BASE_TYPE(T)))) {
			for (int i = 0; i < APT_TRAITS_COUNT(T); ++i) {
				ret[i] = arr[i].GetFloat();
			}
		} else {
			if (DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(APT_TRAITS_BASE_TYPE(T)))) {
				for (int i = 0; i < APT_TRAITS_COUNT(T); ++i) {
					ret[i] = arr[i].GetInt();
				}
			} else {
				for (int i = 0; i < APT_TRAITS_COUNT(T); ++i) {
					ret[i] = arr[i].GetUint();
				}
			}
		}		
		return ret;
	}

	template <typename T, int kCount> // \hack APT_TRAITS_COUNT(T) returns the # of floats in the matrix, hence _count is required
	T getMatrix(int _i)
	{
	 // matrices are arrays of vectors
		auto val = get(_i);
		auto valType = GetValueType(val->GetType());
		JSON_LOG_ERR_TYPE("getMatrix", topName(), valType, ValueType_Array, return identity);
		auto& arrVecs = val->GetArray();
		JSON_LOG_ERR_SIZE("getMatrix", topName(), (int)arrVecs.Size(), kCount, return identity);
		JSON_LOG_ERR_TYPE("getMatrix", topName(), GetValueType(arr[0]->GetType()), ValueType_Array, return identity);

		T ret;
		for (int i = 0; i < kCount; ++i) {
			for (int j = 0; j < kCount; ++j) { // only square matrices supported
				auto& vec = arrVecs[i];
				JSON_LOG_ERR_SIZE("getMatrix", topName(), (int)vec.Size(), kCount, return identity);
				ret[i][j] = vec[j].GetFloat();
			}
		}		
		return ret;
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
	m_impl->setTop(&m_impl->m_dom);
	enterObject(); // automatically enter the root 
	
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
	auto top = m_impl->topValue();
	if (!top->IsObject()) {
		return false;
	}
	auto it = top->FindMember(_name);
	if (it != top->MemberEnd()) {
		m_impl->setTop(&it->value, it->name.GetString());
		return true;
	}
	return false;
}
	
bool Json::next()
{
	auto& parent = m_impl->parent().m_value;
	ValueType type = GetValueType(parent->GetType());
	if (type == ValueType_Array) {
		auto it = parent->Begin() + (m_impl->topIndex()++);
		m_impl->topValue() = it;
		return it != parent->End();

	} else if (type == ValueType_Object) {
		auto it = parent->MemberBegin() + (m_impl->topIndex()++);
		m_impl->topValue() = &it->value;
		m_impl->topName()  = it->name.GetString();
		return it != parent->MemberEnd();

	}
	return false;
}

bool Json::enterObject()
{
	APT_ASSERT(!m_impl->m_stack.empty());
	JSON_LOG_ERR_TYPE("enterObject", m_impl->topName(), getType(), ValueType_Object, return false);
	m_impl->push();
	return true;
}

void Json::leaveObject()
{
	m_impl->pop();
	APT_ASSERT(getType() == ValueType_Object);
}

bool Json::enterArray()
{
	APT_ASSERT(!m_impl->m_stack.empty());
	JSON_LOG_ERR_TYPE("enterArray", m_impl->topName(), getType(), ValueType_Array, return false);
	m_impl->push();
	return true;
}

void Json::leaveArray()
{
	m_impl->pop();
	APT_ASSERT(getType() == ValueType_Array);
}

Json::ValueType Json::getType() const
{
	return m_impl->topType();
}

const char* Json::getName() const
{
	return m_impl->topName();
}

int Json::getArrayLength() const
{
	if (getType() == ValueType_Array) {
		return (int)m_impl->topValue()->GetArray().Size();
	}
	return -1;
}

template <> bool Json::getValue<bool>(int _i) const
{
	return m_impl->getBool(_i);
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