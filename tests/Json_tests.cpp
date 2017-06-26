#include <catch.hpp>

#include <apt/Json.h>

using namespace apt;

TEST_CASE("ArrayOfArray", "[JsonSerializer]")
{
	Json json;

	{	JsonSerializer js(&json, JsonSerializer::Mode_Write);
		js.beginArray("ArrayOfArrays");
		for (int i = 0; i < 4; ++i) {
			js.beginArray();
				for (int j = 0; j < 3; ++j) {
					int v = i + j;
					js.value(v);
				}
			js.endArray();
		}
		js.endArray();
	}
	{	JsonSerializer js(&json, JsonSerializer::Mode_Read);
		js.beginArray("ArrayOfArrays");
		for (int i = 0; i < 4; ++i) {
			js.beginArray();
				for (int j = 0; j < 3; ++j) {
					int v;
					js.value(v);
					REQUIRE(v == i + j);
				}
			js.endArray();
		}
		js.endArray();
	}
}
