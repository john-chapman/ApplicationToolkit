#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <EASTL/vector.h>

TEST_CASE("adhoc")
{
	eastl::vector<int> vi;
	for (int i = 0; i < 47; ++i) {
		vi.push_back(i);
	}

	int x = 4;
}