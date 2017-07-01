#include <catch.hpp>

#include <apt/def.h>
#include <apt/types.h>
#include <apt/math.h>

#include <apt/hash.h>

using namespace apt;

template <typename tType>
static void FloatToIntN()
{
	REQUIRE(DataTypeConvert<tType>((float32)0.0f)  == (tType)0);
	REQUIRE(DataTypeConvert<tType>((float32)1.0f)  == tType::kMax);
	REQUIRE(DataTypeConvert<tType>((float32)-1.0f) == tType::kMin);
	REQUIRE(DataTypeConvert<tType>((float32)0.5f)  == tType::kMax / 2);
	REQUIRE(DataTypeConvert<tType>((float32)-0.5f) == tType::kMin / 2);
}
template <typename tType>
static void IntNToFloat()
{
	float err = 2.0f / powf(2.0f, sizeof(tType) * 8); // (kMax - kMin) fails for 32/64 bit types?
	float mn = DataTypeIsSigned(tType::Enum) ? -1.0f : 0.0f;
	REQUIRE(fabs(DataTypeConvert<float32>(tType::kMax) - 1.0f) < err);
	REQUIRE(fabs(DataTypeConvert<float32>(tType::kMin) - mn) < err);
	REQUIRE(fabs(DataTypeConvert<float32>((tType)(tType::kMax / 2)) - 0.5f) < err);
}

TEST_CASE("Validate type sizes", "[types]")
{
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

TEST_CASE("Validate conversion functions", "[types]")
{
	FloatToIntN<sint8N>();
	FloatToIntN<uint8N>();
	FloatToIntN<sint16N>();
	FloatToIntN<uint16N>();
	// The following all fail due to floating point precision
	//FloatToIntN<sint32N>();
	//FloatToIntN<uint32N>();
	//FloatToIntN<sint64N>();
	//FloatToIntN<uint64N>();
	
	IntNToFloat<sint8N>();
	IntNToFloat<uint8N>();
	IntNToFloat<sint16N>();
	IntNToFloat<uint16N>();
	IntNToFloat<sint32N>();
	IntNToFloat<uint32N>();
	IntNToFloat<sint64N>();
	IntNToFloat<uint64N>();
}

TEST_CASE("Validate metadata functions", "[types]")
{
	REQUIRE(DataTypeIsNormalized(DataType_Sint8)   == false);
	REQUIRE(DataTypeIsNormalized(DataType_Uint8)   == false);
	REQUIRE(DataTypeIsNormalized(DataType_Sint8N)  == true);
	REQUIRE(DataTypeIsNormalized(DataType_Uint8N)  == true);
	REQUIRE(DataTypeIsNormalized(DataType_Sint16)  == false);
	REQUIRE(DataTypeIsNormalized(DataType_Uint16)  == false);
	REQUIRE(DataTypeIsNormalized(DataType_Sint16N) == true);
	REQUIRE(DataTypeIsNormalized(DataType_Uint16N) == true);
	REQUIRE(DataTypeIsNormalized(DataType_Sint32)  == false);
	REQUIRE(DataTypeIsNormalized(DataType_Uint32)  == false);
	REQUIRE(DataTypeIsNormalized(DataType_Sint32N) == true);
	REQUIRE(DataTypeIsNormalized(DataType_Uint32N) == true);
	REQUIRE(DataTypeIsNormalized(DataType_Sint64)  == false);
	REQUIRE(DataTypeIsNormalized(DataType_Uint64)  == false);
	REQUIRE(DataTypeIsNormalized(DataType_Sint64N) == true);
	REQUIRE(DataTypeIsNormalized(DataType_Uint64N) == true);
	REQUIRE(DataTypeIsNormalized(DataType_Float16) == false);
	REQUIRE(DataTypeIsNormalized(DataType_Float32) == false);
	REQUIRE(DataTypeIsNormalized(DataType_Float64) == false);

	REQUIRE(DataTypeIsFloat(DataType_Sint8)   == false);
	REQUIRE(DataTypeIsFloat(DataType_Uint8)   == false);
	REQUIRE(DataTypeIsFloat(DataType_Sint8N)  == false);
	REQUIRE(DataTypeIsFloat(DataType_Uint8N)  == false);
	REQUIRE(DataTypeIsFloat(DataType_Sint16)  == false);
	REQUIRE(DataTypeIsFloat(DataType_Uint16)  == false);
	REQUIRE(DataTypeIsFloat(DataType_Sint16N) == false);
	REQUIRE(DataTypeIsFloat(DataType_Uint16N) == false);
	REQUIRE(DataTypeIsFloat(DataType_Sint32)  == false);
	REQUIRE(DataTypeIsFloat(DataType_Uint32)  == false);
	REQUIRE(DataTypeIsFloat(DataType_Sint32N) == false);
	REQUIRE(DataTypeIsFloat(DataType_Uint32N) == false);
	REQUIRE(DataTypeIsFloat(DataType_Sint64)  == false);
	REQUIRE(DataTypeIsFloat(DataType_Uint64)  == false);
	REQUIRE(DataTypeIsFloat(DataType_Sint64N) == false);
	REQUIRE(DataTypeIsFloat(DataType_Uint64N) == false);
	REQUIRE(DataTypeIsFloat(DataType_Float16) == true);
	REQUIRE(DataTypeIsFloat(DataType_Float32) == true);
	REQUIRE(DataTypeIsFloat(DataType_Float64) == true);

	REQUIRE(DataTypeIsSigned(DataType_Sint8)   == true);
	REQUIRE(DataTypeIsSigned(DataType_Uint8)   == false);
	REQUIRE(DataTypeIsSigned(DataType_Sint8N)  == true);
	REQUIRE(DataTypeIsSigned(DataType_Uint8N)  == false);
	REQUIRE(DataTypeIsSigned(DataType_Sint16)  == true);
	REQUIRE(DataTypeIsSigned(DataType_Uint16)  == false);
	REQUIRE(DataTypeIsSigned(DataType_Sint16N) == true);
	REQUIRE(DataTypeIsSigned(DataType_Uint16N) == false);
	REQUIRE(DataTypeIsSigned(DataType_Sint32)  == true);
	REQUIRE(DataTypeIsSigned(DataType_Uint32)  == false);
	REQUIRE(DataTypeIsSigned(DataType_Sint32N) == true);
	REQUIRE(DataTypeIsSigned(DataType_Uint32N) == false);
	REQUIRE(DataTypeIsSigned(DataType_Sint64)  == true);
	REQUIRE(DataTypeIsSigned(DataType_Uint64)  == false);
	REQUIRE(DataTypeIsSigned(DataType_Sint64N) == true);
	REQUIRE(DataTypeIsSigned(DataType_Uint64N) == false);
	REQUIRE(DataTypeIsSigned(DataType_Float16) == true);
	REQUIRE(DataTypeIsSigned(DataType_Float32) == true);
	REQUIRE(DataTypeIsSigned(DataType_Float64) == true);
}
