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
}

TEST_CASE("Vector push/pop", "[String]")
{
	VectorTest<0>();
	VectorTest<16>();
	VectorTest<64>();
}
