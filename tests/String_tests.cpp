#include <catch.hpp>

#include <apt/log.h>
#include <apt/math.h>
#include <apt/String.h>

#include <EASTL/vector.h>

using namespace apt;

template <uint kCapacity>
static void VectorTest()
{
	eastl::vector<String<kCapacity>> strv;
	LCG lcg;
	for (int i = 0; i < 10000; ++i) {
		int strl = lcg.rand(1, APT_MAX((int)kCapacity * 2, 2));
		String<kCapacity> str;
		for (int j = 0; j < strl; ++j) {
			char c = (char)lcg.rand((int)'A', (int)'Z' + 1);
			str.append(&c, 1);
		}
		strv.push_back(str);
	}
	while (!strv.empty()) {
		strv.pop_back();
	}

	strv.clear();
	for (int i = 0; i < 100; ++i) {
		strv.push_back(String<kCapacity>("HELLO"));
		REQUIRE((strv.back() == "HELLO") == true);
	}
}

TEST_CASE("Vector", "[String]")
{
	VectorTest<0>();
	VectorTest<16>();
	VectorTest<64>();
}

TEST_CASE("toUpperCase, toLowerCase", "[String]")
{
	static const char* lower = "abcdefghijklmnopqrstuvwxyz1234567890";
	static const char* upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	String<0> str(lower);
	str.toUpperCase();
	REQUIRE((str == upper) == true);
	str.toLowerCase();
	REQUIRE((str == lower) == true);
}
