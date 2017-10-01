#include <catch.hpp>

#include <apt/Json.h>

using namespace apt;

TEST_CASE("ArrayOfArray", "[SerializerJson]")
{
	Json json;

	{	SerializerJson js(&json, SerializerJson::Mode_Write);
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
	{	SerializerJson js(&json, SerializerJson::Mode_Read);
		uint in = 4;
		js.beginArray(in, "ArrayOfArrays");
		REQUIRE(in== 4);
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
