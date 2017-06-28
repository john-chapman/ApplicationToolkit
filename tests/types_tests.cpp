#include <catch.hpp>

#include <apt/def.h>
#include <apt/types.h>
#include <apt/math.h>

using namespace apt;

void StronglyTypedFunction(refactor::uint8N x)
{
}

TEST_CASE("Validate type sizes", "[types]")
{
	refactor::float32 f = -1.25f;
	refactor::sint8N x = refactor::DataTypeConvert<refactor::sint8N>(f);


	REQUIRE(sizeof(refactor::sint8)    == 1);
	REQUIRE(sizeof(refactor::uint8)    == 1);
	REQUIRE(sizeof(refactor::sint8N)   == 1);
	REQUIRE(sizeof(refactor::uint8N)   == 1);
	REQUIRE(sizeof(refactor::sint16)   == 2);
	REQUIRE(sizeof(refactor::uint16)   == 2);
	REQUIRE(sizeof(refactor::sint16N)  == 2);
	REQUIRE(sizeof(refactor::uint16N)  == 2);
	REQUIRE(sizeof(refactor::sint32)   == 4);
	REQUIRE(sizeof(refactor::uint32)   == 4);
	REQUIRE(sizeof(refactor::sint32N)  == 4);
	REQUIRE(sizeof(refactor::uint32N)  == 4);
	REQUIRE(sizeof(refactor::sint64)   == 8);
	REQUIRE(sizeof(refactor::uint64)   == 8);
	REQUIRE(sizeof(refactor::sint64N)  == 8);
	REQUIRE(sizeof(refactor::uint64N)  == 8);
	REQUIRE(sizeof(refactor::float16)  == 2);
	REQUIRE(sizeof(refactor::float32)  == 4);
	REQUIRE(sizeof(refactor::float64)  == 8);

	REQUIRE(sizeof(sint8)    == 1);
	REQUIRE(sizeof(uint8)    == 1);
	REQUIRE(sizeof(sint8N)   == 1);
	REQUIRE(sizeof(uint8N)   == 1);
	REQUIRE(sizeof(sint16)   == 2);
	REQUIRE(sizeof(uint16)   == 2);
	REQUIRE(sizeof(sint16N)  == 2);
	REQUIRE(sizeof(uint16N)  == 2);
	REQUIRE(sizeof(sint32)   == 4);
	REQUIRE(sizeof(uint32)   == 4);
	REQUIRE(sizeof(sint32N)  == 4);
	REQUIRE(sizeof(uint32N)  == 4);
	REQUIRE(sizeof(sint64)   == 8);
	REQUIRE(sizeof(uint64)   == 8);
	REQUIRE(sizeof(sint64N)  == 8);
	REQUIRE(sizeof(uint64N)  == 8);
	REQUIRE(sizeof(float16)  == 2);
	REQUIRE(sizeof(float32)  == 4);
	REQUIRE(sizeof(float64)  == 8);

	REQUIRE(sizeof(vec2) == sizeof(float32) * 2);
	REQUIRE(sizeof(vec3) == sizeof(float32) * 3);
	REQUIRE(sizeof(vec4) == sizeof(float32) * 4);
	REQUIRE(sizeof(quat) == sizeof(float32) * 4);
	REQUIRE(sizeof(mat2) == sizeof(float32) * 4);
	REQUIRE(sizeof(mat3) == sizeof(float32) * 9);
	REQUIRE(sizeof(mat4) == sizeof(float32) * 16);
}

TEST_CASE("Validate DataType::GetSizeInBytes() matches sizeof()", "[types]")
{
	REQUIRE(DataType::GetSizeBytes(DataType::Sint8)    == sizeof(sint8));
	REQUIRE(DataType::GetSizeBytes(DataType::Uint8)    == sizeof(uint8));
	REQUIRE(DataType::GetSizeBytes(DataType::Sint8N)   == sizeof(sint8));
	REQUIRE(DataType::GetSizeBytes(DataType::Uint8N)   == sizeof(uint8));
	REQUIRE(DataType::GetSizeBytes(DataType::Sint16)   == sizeof(sint16));
	REQUIRE(DataType::GetSizeBytes(DataType::Uint16)   == sizeof(uint16));
	REQUIRE(DataType::GetSizeBytes(DataType::Sint16N)  == sizeof(sint16));
	REQUIRE(DataType::GetSizeBytes(DataType::Uint16N)  == sizeof(uint16));
	REQUIRE(DataType::GetSizeBytes(DataType::Sint32)   == sizeof(sint32));
	REQUIRE(DataType::GetSizeBytes(DataType::Uint32)   == sizeof(uint32));
	REQUIRE(DataType::GetSizeBytes(DataType::Sint32N)  == sizeof(sint32));
	REQUIRE(DataType::GetSizeBytes(DataType::Uint32N)  == sizeof(uint32));
	REQUIRE(DataType::GetSizeBytes(DataType::Sint64)   == sizeof(sint64));
	REQUIRE(DataType::GetSizeBytes(DataType::Uint64)   == sizeof(uint64));
	REQUIRE(DataType::GetSizeBytes(DataType::Sint64N)  == sizeof(sint64));
	REQUIRE(DataType::GetSizeBytes(DataType::Uint64N)  == sizeof(uint64));
	REQUIRE(DataType::GetSizeBytes(DataType::Float16)  == sizeof(float16));
	REQUIRE(DataType::GetSizeBytes(DataType::Float32)  == sizeof(float32));
	REQUIRE(DataType::GetSizeBytes(DataType::Float64)  == sizeof(float64));
}

TEST_CASE("Validate metadata functions", "[types]")
{
	REQUIRE(DataType::IsNormalized(DataType::Sint8)    == false);
	REQUIRE(DataType::IsNormalized(DataType::Uint8)    == false);
	REQUIRE(DataType::IsNormalized(DataType::Sint8N)   == true);
	REQUIRE(DataType::IsNormalized(DataType::Uint8N)   == true);
	REQUIRE(DataType::IsNormalized(DataType::Sint16)   == false);
	REQUIRE(DataType::IsNormalized(DataType::Uint16)   == false);
	REQUIRE(DataType::IsNormalized(DataType::Sint16N)  == true);
	REQUIRE(DataType::IsNormalized(DataType::Uint16N)  == true);
	REQUIRE(DataType::IsNormalized(DataType::Sint32)   == false);
	REQUIRE(DataType::IsNormalized(DataType::Uint32)   == false);
	REQUIRE(DataType::IsNormalized(DataType::Sint32N)  == true);
	REQUIRE(DataType::IsNormalized(DataType::Uint32N)  == true);
	REQUIRE(DataType::IsNormalized(DataType::Sint64)   == false);
	REQUIRE(DataType::IsNormalized(DataType::Uint64)   == false);
	REQUIRE(DataType::IsNormalized(DataType::Sint64N)  == true);
	REQUIRE(DataType::IsNormalized(DataType::Uint64N)  == true);
	REQUIRE(DataType::IsNormalized(DataType::Float16)  == false);
	REQUIRE(DataType::IsNormalized(DataType::Float32)  == false);
	REQUIRE(DataType::IsNormalized(DataType::Float64)  == false);

	REQUIRE(DataType::IsFloat(DataType::Sint8)    == false);
	REQUIRE(DataType::IsFloat(DataType::Uint8)    == false);
	REQUIRE(DataType::IsFloat(DataType::Sint8N)   == false);
	REQUIRE(DataType::IsFloat(DataType::Uint8N)   == false);
	REQUIRE(DataType::IsFloat(DataType::Sint16)   == false);
	REQUIRE(DataType::IsFloat(DataType::Uint16)   == false);
	REQUIRE(DataType::IsFloat(DataType::Sint16N)  == false);
	REQUIRE(DataType::IsFloat(DataType::Uint16N)  == false);
	REQUIRE(DataType::IsFloat(DataType::Sint32)   == false);
	REQUIRE(DataType::IsFloat(DataType::Uint32)   == false);
	REQUIRE(DataType::IsFloat(DataType::Sint32N)  == false);
	REQUIRE(DataType::IsFloat(DataType::Uint32N)  == false);
	REQUIRE(DataType::IsFloat(DataType::Sint64)   == false);
	REQUIRE(DataType::IsFloat(DataType::Uint64)   == false);
	REQUIRE(DataType::IsFloat(DataType::Sint64N)  == false);
	REQUIRE(DataType::IsFloat(DataType::Uint64N)  == false);
	REQUIRE(DataType::IsFloat(DataType::Float16)  == true);
	REQUIRE(DataType::IsFloat(DataType::Float32)  == true);
	REQUIRE(DataType::IsFloat(DataType::Float64)  == true);

	REQUIRE(DataType::IsSigned(DataType::Sint8)    == true);
	REQUIRE(DataType::IsSigned(DataType::Uint8)    == false);
	REQUIRE(DataType::IsSigned(DataType::Sint8N)   == true);
	REQUIRE(DataType::IsSigned(DataType::Uint8N)   == false);
	REQUIRE(DataType::IsSigned(DataType::Sint16)   == true);
	REQUIRE(DataType::IsSigned(DataType::Uint16)   == false);
	REQUIRE(DataType::IsSigned(DataType::Sint16N)  == true);
	REQUIRE(DataType::IsSigned(DataType::Uint16N)  == false);
	REQUIRE(DataType::IsSigned(DataType::Sint32)   == true);
	REQUIRE(DataType::IsSigned(DataType::Uint32)   == false);
	REQUIRE(DataType::IsSigned(DataType::Sint32N)  == true);
	REQUIRE(DataType::IsSigned(DataType::Uint32N)  == false);
	REQUIRE(DataType::IsSigned(DataType::Sint64)   == true);
	REQUIRE(DataType::IsSigned(DataType::Uint64)   == false);
	REQUIRE(DataType::IsSigned(DataType::Sint64N)  == true);
	REQUIRE(DataType::IsSigned(DataType::Uint64N)  == false);
	REQUIRE(DataType::IsSigned(DataType::Float16)  == true);
	REQUIRE(DataType::IsSigned(DataType::Float32)  == true);
	REQUIRE(DataType::IsSigned(DataType::Float64)  == true);
}
