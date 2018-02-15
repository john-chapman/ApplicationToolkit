#pragma once
#ifndef apt_Json_h
#define apt_Json_h

#include <apt/apt.h>
#include <apt/FileSystem.h>
#include <apt/Serializer.h>

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
	friend class SerializerJson; 
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

	// Get the current value. tType is expected to match the type of the current value exactly (i.e. getValue<int>() must be called only if the value type is ValueType_Number).
	// \note Ptr returned by getValue<const char*> is only valid during the lifetime of the Json object.
	template <typename tType>
	tType getValue(int _i = -1) const;
		
	// Create and set a named value. If the object already exists this modifies the type and value of the existing object.
	template <typename tType>
	void setValue(const char* _name, tType _value);

	// Enter the current object (call immediately after find() or next()). Return false if the current value is not an object.
	bool enterObject();
	// Leave the current object.
	void leaveObject();

	// Enter the array (call immediately after find() or next()). Return false if the current value is not an array.
	bool enterArray();
	// Leave the current array.
	void leaveArray();

	// Return number of elements in the current array (or -1 if not in an array).
	int getArrayLength() const;

	// Create and enter an object. If the object already exists this has the same effect as a call to find() followed by
	// a call to enterObject(). If _name is nullptr the object is expected to be an array member.
	void beginObject(const char* _name = nullptr);
	// Leave the current object.
	void endObject() { leaveObject(); }
	
	// Create and enter an array. If the array already exists this has the effect as a call to find() followed by a 
	// call to enterArray(). If _name is nullptr the array is expected to be an array member.
	void beginArray(const char* _name = nullptr);
	// Leave the current array.
	void endArray() { leaveArray(); }

	// Push _value into the current array.
	template <typename tType>
	void pushValue(tType _value);
	

private:
	struct Impl;
	Impl* m_impl;

};

////////////////////////////////////////////////////////////////////////////////
// SerializerJson
////////////////////////////////////////////////////////////////////////////////
class SerializerJson: public Serializer
{
public:
	SerializerJson(Json& _json_, Mode _mode);

	Json* getJson() { return m_json; }

	bool beginObject(const char* _name = nullptr) override;
	void endObject() override;

	bool beginArray(uint& _length_, const char* _name = nullptr) override;
	void endArray() override;

	bool value(bool&       _value_, const char* _name = nullptr) override;
	bool value(sint8&      _value_, const char* _name = nullptr) override;
	bool value(uint8&      _value_, const char* _name = nullptr) override;
	bool value(sint16&     _value_, const char* _name = nullptr) override;
	bool value(uint16&     _value_, const char* _name = nullptr) override;
	bool value(sint32&     _value_, const char* _name = nullptr) override;
	bool value(uint32&     _value_, const char* _name = nullptr) override;
	bool value(sint64&     _value_, const char* _name = nullptr) override;
	bool value(uint64&     _value_, const char* _name = nullptr) override;
	bool value(float32&    _value_, const char* _name = nullptr) override;
	bool value(float64&    _value_, const char* _name = nullptr) override;
	bool value(StringBase& _value_, const char* _name = nullptr) override;
	
	bool binary(void*& _data_, uint& _sizeBytes_, const char* _name = nullptr, CompressionFlags _compressionFlags = CompressionFlags_None) override;

private:
	Json* m_json;

	// 
	int string(const char* _value_, const char* _name);

}; // class SerializerJson


} // namespace apt

#endif // apt_Json_h
