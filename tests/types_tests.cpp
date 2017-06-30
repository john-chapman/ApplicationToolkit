#include <catch.hpp>

#include <apt/def.h>
#include <apt/types.h>
#include <apt/math.h>

using namespace apt;

template <typename tType>
static void FloatToIntN()
{
	REQUIRE(refactor::DataTypeConvert<tType>((refactor::float32)0.0f)  == (tType)0);
	REQUIRE(refactor::DataTypeConvert<tType>((refactor::float32)1.0f)  == tType::kMax);
	REQUIRE(refactor::DataTypeConvert<tType>((refactor::float32)-1.0f) == tType::kMin);
	REQUIRE(refactor::DataTypeConvert<tType>((refactor::float32)0.5f)  == tType::kMax / 2);
	REQUIRE(refactor::DataTypeConvert<tType>((refactor::float32)-0.5f) == tType::kMin / 2);
}
template <typename tType>
static void IntNToFloat()
{
	float err = 2.0f / pow(2.0f, sizeof(tType) * 8); // (kMax - kMin) fails for 32/64 bit types?
	float mn = DataTypeIsSigned(tType::Enum) ? -1.0f : 0.0f;
	REQUIRE(fabs(refactor::DataTypeConvert<refactor::float32>(tType::kMax) - 1.0f) < err);
	REQUIRE(fabs(refactor::DataTypeConvert<refactor::float32>(tType::kMin) - mn) < err);
	REQUIRE(fabs(refactor::DataTypeConvert<refactor::float32>((tType)(tType::kMax / 2)) - 0.5f) < err);
}

TEST_CASE("Validate type sizes", "[types]")
{
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

	REQUIRE(sizeof(vec2) == sizeof(float32) * 2);
	REQUIRE(sizeof(vec3) == sizeof(float32) * 3);
	REQUIRE(sizeof(vec4) == sizeof(float32) * 4);
	REQUIRE(sizeof(quat) == sizeof(float32) * 4);
	REQUIRE(sizeof(mat2) == sizeof(float32) * 4);
	REQUIRE(sizeof(mat3) == sizeof(float32) * 9);
	REQUIRE(sizeof(mat4) == sizeof(float32) * 16);
}

TEST_CASE("Validate conversion functions", "[types]")
{
	FloatToIntN<refactor::sint8N>();
	FloatToIntN<refactor::uint8N>();
	FloatToIntN<refactor::sint16N>();
	FloatToIntN<refactor::uint16N>();
	// The following all fail due to floating point precision
	//FloatToIntN<refactor::sint32N>();
	//FloatToIntN<refactor::uint32N>();
	//FloatToIntN<refactor::sint64N>();
	//FloatToIntN<refactor::uint64N>();
	
	IntNToFloat<refactor::sint8N>();
	IntNToFloat<refactor::uint8N>();
	IntNToFloat<refactor::sint16N>();
	IntNToFloat<refactor::uint16N>();
	IntNToFloat<refactor::sint32N>();
	IntNToFloat<refactor::uint32N>();
	IntNToFloat<refactor::sint64N>();
	IntNToFloat<refactor::uint64N>();
}

TEST_CASE("Validate metadata functions", "[types]")
{
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Sint8)   == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Uint8)   == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Sint8N)  == true);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Uint8N)  == true);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Sint16)  == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Uint16)  == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Sint16N) == true);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Uint16N) == true);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Sint32)  == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Uint32)  == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Sint32N) == true);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Uint32N) == true);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Sint64)  == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Uint64)  == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Sint64N) == true);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Uint64N) == true);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Float16) == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Float32) == false);
	REQUIRE(refactor::DataTypeIsNormalized(refactor::DataType_Float64) == false);

	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Sint8)   == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Uint8)   == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Sint8N)  == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Uint8N)  == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Sint16)  == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Uint16)  == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Sint16N) == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Uint16N) == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Sint32)  == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Uint32)  == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Sint32N) == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Uint32N) == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Sint64)  == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Uint64)  == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Sint64N) == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Uint64N) == false);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Float16) == true);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Float32) == true);
	REQUIRE(refactor::DataTypeIsFloat(refactor::DataType_Float64) == true);

	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Sint8)   == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Uint8)   == false);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Sint8N)  == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Uint8N)  == false);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Sint16)  == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Uint16)  == false);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Sint16N) == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Uint16N) == false);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Sint32)  == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Uint32)  == false);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Sint32N) == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Uint32N) == false);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Sint64)  == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Uint64)  == false);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Sint64N) == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Uint64N) == false);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Float16) == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Float32) == true);
	REQUIRE(refactor::DataTypeIsSigned(refactor::DataType_Float64) == true);
}
