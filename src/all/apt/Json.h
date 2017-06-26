#pragma once
#ifndef apt_Json_h
#define apt_Json_h

#include <apt/def.h>
#include <apt/FileSystem.h>

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// Json
// Traversal of a loaded document is a state machine:
// 
//  Json json("json.json") // use Json::Read() if you need to check the return code
//  
//  if (json.find("Value")) {                    // find a value in the DOM root
//     if (json.getType() == Json::ValueType_Number) {    // check it's the right type
//        int v = json.getValue<int>();          // retrieve/store the value
//     }
//  }
//  
//  if (json.find("Array")) {
//     if (json.enterArray()) {
//        while (json.next()) {                      // get the next value while one exists
//           if (json.getType() == Json::ValueType_Number) {  // check it's the right type
//              int v = json.getValue<int>();        // retrieve/store the value
//           }
//        }
//        json.leaveArray(); // must leave the array before proceeding
//     }
//  }
// 
//
// Creating and modifying a document works in a similar way:
// 
//  Json json; // create an empty document
//  
//  json.beginObject("Object");
//     json.setValue("Boolean", false);
//     json.setValue("Integer", 2);
//     json.beginObject("NestedObject");
//        json.setValue("Boolean", false);
//     json.endObject();
//  json.endObject();
//  
//  json.beginArray("Array");
//     for (int i = 0; i < 4; ++i) {
//        json.pushValue(i);
//    }
//  json.endArray();
//  
//  json.beginArray("ArrayOfArrays");
//     for (int i = 0; i < 4; ++i) {
//        json.beginArray(); // array members are anonymous
//           for (int j = 0; j < 4; ++j) {
//              json.pushValue(j);
//  		}
//        json.endArray();
//  json.endArray();
//
//  Json::Write(json, "json.json");
//
// \note String ptrs passed as the _name argument for setValue() are assumed to
//   have a lifetime at least as long as the Json object. String ptrs passed to
//   the _val argument are copied internally.
////////////////////////////////////////////////////////////////////////////////
class Json
{
	friend class JsonSerializer;
public:
	enum ValueType
	{
		ValueType_Null,
		ValueType_Object,
		ValueType_Array,
		ValueType_Bool,
		ValueType_Number,
		ValueType_String,

		ValueType_Count
	};

	static bool Read(Json& json_, const File& _file);
	static bool Read(Json& json_, const char* _path, FileSystem::RootType _rootHint = FileSystem::RootType_Default);
	static bool Write(const Json& _json, File& file_);
	static bool Write(const Json& _json, const char* _path, FileSystem::RootType _rootHint = FileSystem::RootType_Default);
		
	// Reads from _path if specified.
	Json(const char* _path = nullptr, FileSystem::RootType _rootHint = FileSystem::RootType_Default);
	~Json();

	// Find a named value in the current object. Return true if the value is found, in which case getValue() may be called.
	bool find(const char* _name);
	
	// Get the next value in the current object/array. Return true if not the end of the object/array, in which case getValue() may be called.
	bool next();

	// Get the type of the current value.
	ValueType getType() const;

	// Get the current value. tType is expected to match the type of the current value exactly (i.e. getValue<int>() must be called only 
	// if the value type is ValueType_Number).
	// \note Ptr returned by getValue<const char*> is only valid during the lifetime of the Json object.
	template <typename tType>
	tType getValue() const;

	// Enter the current object (call immediately after find() or next()). Return false if the current value is not an object.
	bool enterObject();
	// Leave the current object.
	void leaveObject();

	// Enter the array (call immediately after find() or next()). Return false if the current value is not an array.
	bool enterArray();
	// Leave the current array.
	void leaveArray();

	// Return length of the current array (or -1 if not in an array).
	int getArrayLength() const;

	// Create and enter an object. If the object already exists this has the same effect as a call to find() followed by
	// a call to enterObject(). If _name is nullptr the object is expected to be an array member.
	void beginObject(const char* _name = nullptr);
	// Leave the current object.
	void endObject() { leaveObject(); }
	
	// Create and set a named value. If the object already exists this modifies the type and value of the existing object.
	template <typename tType>
	void setValue(const char* _name, tType _value);

	// Create and enter an array. If the array already exists this has the effect as a call to find() followed by a 
	// call to enterArray(). If _name is nullptr the array is expected to be an array member.
	void beginArray(const char* _name = nullptr);
	// Leave the current array.
	void endArray() { leaveArray(); }

	// Create and set an array member.
	template <typename tType>
	void pushValue(tType _value);
	

private:
	struct Impl;
	Impl* m_impl;

}; // class Json

////////////////////////////////////////////////////////////////////////////////
// JsonSerializer
////////////////////////////////////////////////////////////////////////////////
class JsonSerializer
{
public:
	enum Mode
	{
		Mode_Read,
		Mode_Write
	};
	JsonSerializer(Json* _json_, Mode _mode);

	bool beginObject(const char* _name = nullptr);
	void endObject();

	bool beginArray(const char* _name = nullptr);
	void endArray();
	
	// Length of the current array (or -1 if not in an array).
	int getArrayLength() const { return m_json->getArrayLength(); }

	// Visit a named value in the current object/array.
	template <typename tType>
	bool value(const char* _name, tType& _value_);
	template <typename tType>
	bool value(tType& _value_);

	// Visit a named string in the current object/array. Pass nullptr as _string_ to get the string length.
	// Return the string length, excluding the terminating null.
	// \note This is the 'raw' api, prefer to use value(StringBase&).
	int string(const char* _name, char* _string_);
	int string(char* _string_);

	Mode getMode() const     { return m_mode; }
	void setMode(Mode _mode) { m_mode = _mode; }

private:
	Json* m_json;
	Mode  m_mode;

	// Return whether the read/write head is inside an array.
	bool insideArray();
	
};
class Serializer: public JsonSerializer {}; // \todo Absract base class?

} // namespace apt

#endif // apt_Json_h
