#include <catch.hpp>

#include <apt/memory.h>
#include <apt/Json.h>

using namespace apt;

TEST_CASE("ArrayAccess", "[Json]")
{
	const int arr[] = { 3, 5, 7, 6 };

	Json json;
	json.beginArray("ArrayInt");
		for (auto v : arr) {
			json.pushValue(v);
		}
	json.endArray();

	json.beginArray("ArrayInt");
	{
		int n = APT_ARRAY_COUNT(arr);
		REQUIRE(json.getArrayLength() == n);
		for (int i = 0; i < n; ++i) {
			auto v = json.getValue<int>(i);
			REQUIRE(json.getValue<int>(i) == arr[i]);
		}
	}
	json.endArray();



	json.beginArray("ArrayVec2");
		for (auto v : arr) {
			json.pushValue(vec2((float)v));
		}
	json.endArray();
	json.beginArray("ArrayVec2");
	{
		int n = APT_ARRAY_COUNT(arr);
		REQUIRE(json.getArrayLength() == n);
		for (int i = 0; i < n; ++i) {
			auto v = json.getValue<vec2>(i);
			REQUIRE(json.getValue<vec2>(i) == vec2((float)arr[i]));
		}
	}
	json.endArray();
}

TEST_CASE("ArrayOfArray", "[SerializerJson]")
{
	Json json;

	{	SerializerJson js(json, SerializerJson::Mode_Write);
		uint in = 4;
		js.beginArray(in, "ArrayOfArrays");
		for (int i = 0; i < in; ++i) {
			uint jn = 3;
			js.beginArray(jn);
				for (int j = 0; j < jn; ++j) {
					int v = i + j;
					js.value(v);
				}
			js.endArray();
		}
		js.endArray();
	}
	{	SerializerJson js(json, SerializerJson::Mode_Read);
		uint in = 4;
		js.beginArray(in, "ArrayOfArrays");
		REQUIRE(in == 4);
		for (int i = 0; i < in; ++i) {
			uint jn = 3;
			js.beginArray(jn);
			REQUIRE(jn == 3);
				for (int j = 0; j < jn; ++j) {
					int v;
					js.value(v);
					REQUIRE(v == i + j);
				}
			js.endArray();
		}
		js.endArray();
	}
}

TEST_CASE("Binary", "[SerializerJson]")
{
	const char* kSrcData = 
		"Man is distinguished, not only by his reason, but by this singular passion from "
		"other animals, which is a lust of the mind, that by a perseverance of delight "
		"in the continued and indefatigable generation of knowledge, exceeds the short "
		"vehemence of any carnal pleasure."
		;
	const uint kSrcDataSize = strlen(kSrcData);

	
	Json json;
	SerializerJson js(json, SerializerJson::Mode_Write);
	
	void* data = (void*)kSrcData;
	uint dataSize = kSrcDataSize;
	js.binary(data, dataSize, "BinaryTest");

	data = nullptr;
	js.setMode(SerializerJson::Mode_Read);
	js.binary(data, dataSize, "BinaryTest");
	
	REQUIRE(dataSize == kSrcDataSize);
	REQUIRE(memcmp(data, kSrcData, dataSize) == 0);
}

TEST_CASE("Enum", "[SerializerJson]")
{
	enum Fruit 
	{
		Fruit_Apples,
		Fruit_Pears,

		Fruit_Count
	};
	const char* FruitStr[Fruit_Count + 1] =
	{
		"Fruit_Apples",
		"Fruit_Pears",

		"Fruit_Count"
	};
	
	
	Json json;
	SerializerJson jsWrite(json, SerializerJson::Mode_Write);

	Fruit apples = Fruit_Apples;
	Fruit pears  = Fruit_Pears;
	Fruit count  = Fruit_Count;
	((Serializer&)jsWrite).beginArray("fruits");
		SerializeEnum(jsWrite, apples, FruitStr);
		SerializeEnum(jsWrite, pears,  FruitStr);
		SerializeEnum(jsWrite, count,  FruitStr);
	jsWrite.endArray();

	SerializerJson jsRead(json, SerializerJson::Mode_Read);
	((Serializer&)jsRead).beginArray("fruits");
		SerializeEnum(jsWrite, apples, FruitStr);
		SerializeEnum(jsWrite, pears,  FruitStr);
		SerializeEnum(jsWrite, count,  FruitStr);
	jsWrite.endArray();

	REQUIRE(apples == Fruit_Apples);
	REQUIRE(pears  == Fruit_Pears);
	REQUIRE(count  == Fruit_Count);
}