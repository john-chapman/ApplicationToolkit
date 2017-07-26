#pragma once
#ifndef apt_types_h
#define apt_types_h

#ifndef apt_h
	#error apt: include apt/apt.h, not apt/types.h
#endif

#include <cfloat>
#include <cstddef>
#include <cstdint>

#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable: 4244) // possible loss of data
	#pragma warning(disable: 4723) // possible DBZ
	#pragma warning(disable: 4146) // unary minus applied to unsigned
#endif

namespace apt {

enum DataType
{
	DataType_Invalid,

 // integer types
	DataType_Sint8,
	DataType_Uint8,
	DataType_Sint16,
	DataType_Uint16,
	DataType_Sint32,
	DataType_Uint32,
	DataType_Sint64,		
	DataType_Uint64,

 // normalized integer types
	DataType_Sint8N,
	DataType_Uint8N,
	DataType_Sint16N,
	DataType_Uint16N,
	DataType_Sint32N,
	DataType_Uint32N,
	DataType_Sint64N,
	DataType_Uint64N,

 // float types
	DataType_Float16,
	DataType_Float32,
	DataType_Float64,

	DataType_Count,
	DataType_Sint  = DataType_Sint64,
	DataType_Uint  = DataType_Uint64,
	DataType_Float = DataType_Float32,
};

inline constexpr bool DataTypeIsInt(DataType _type)
{
	return _type >= DataType_Sint8 && _type <= DataType_Uint64N;
}
inline constexpr bool DataTypeIsFloat(DataType _type)
{
	return _type >= DataType_Float16 && _type <= DataType_Float64;
}
inline constexpr bool DataTypeIsSigned(DataType _type)
{
	return (_type % 2) != 0 || _type >= DataType_Float16;
}
inline constexpr bool DataTypeIsNormalized(DataType _type)
{
	return _type >= DataType_Sint8N && _type <= DataType_Uint64N;
}


namespace internal {

template <typename tBase, DataType kEnum>
struct DataTypeBase
{
	typedef tBase BaseType;
	static const DataType Enum = kEnum;

	DataTypeBase() = default;

	template <typename tValue>
	DataTypeBase(tValue _value): m_val(_value) {}

	operator BaseType&()                       { return m_val; }
	operator const BaseType&() const           { return m_val; }
private:
	BaseType m_val;
};

} // namespace internal


// Sized integer types.
typedef std::int8_t                                             sint8;
typedef std::uint8_t                                            uint8;
typedef std::int16_t                                            sint16;
typedef std::uint16_t                                           uint16;
typedef std::int32_t                                            sint32;
typedef std::uint32_t                                           uint32;
typedef std::int64_t                                            sint64;
typedef std::uint64_t                                           uint64;

// Sized normalized integer types (use DataTypeConvert() for conversion to/from floating point types).
typedef internal::DataTypeBase<std::int8_t,   DataType_Sint8N>  sint8N;
typedef internal::DataTypeBase<std::uint8_t,  DataType_Uint8N>  uint8N;
typedef internal::DataTypeBase<std::int16_t,  DataType_Sint16N> sint16N;
typedef internal::DataTypeBase<std::uint16_t, DataType_Uint16N> uint16N;
typedef internal::DataTypeBase<std::int32_t,  DataType_Sint32N> sint32N;
typedef internal::DataTypeBase<std::uint32_t, DataType_Uint32N> uint32N;
typedef internal::DataTypeBase<std::int64_t,  DataType_Sint64N> sint64N;
typedef internal::DataTypeBase<std::uint64_t, DataType_Uint64N> uint64N;

// Sized floating point types.
// \note float16 is a storage type only and has no arithmetic operators.
typedef internal::DataTypeBase<std::uint16_t, DataType_Float16> float16;
typedef float                                                   float32;
typedef double                                                  float64;

typedef std::ptrdiff_t sint;
typedef std::size_t    uint;


namespace internal {

// Instantiate _macro for type/datatype pairs
#define APT_DataType_decl(_macro) \
	_macro(apt::sint8,   apt::DataType_Sint8  ) \
	_macro(apt::uint8,   apt::DataType_Uint8  ) \
	_macro(apt::sint16,  apt::DataType_Sint16 ) \
	_macro(apt::uint16,  apt::DataType_Uint16 ) \
	_macro(apt::sint32,  apt::DataType_Sint32 ) \
	_macro(apt::uint32,  apt::DataType_Uint32 ) \
	_macro(apt::sint64,  apt::DataType_Sint64 ) \
	_macro(apt::uint64,  apt::DataType_Uint64 ) \
	_macro(apt::sint8N,  apt::DataType_Sint8N ) \
	_macro(apt::uint8N,  apt::DataType_Uint8N ) \
	_macro(apt::sint16N, apt::DataType_Sint16N) \
	_macro(apt::uint16N, apt::DataType_Uint16N) \
	_macro(apt::sint32N, apt::DataType_Sint32N) \
	_macro(apt::uint32N, apt::DataType_Uint32N) \
	_macro(apt::sint64N, apt::DataType_Sint64N) \
	_macro(apt::uint64N, apt::DataType_Uint64N) \
	_macro(apt::float16, apt::DataType_Float16) \
	_macro(apt::float32, apt::DataType_Float32) \
	_macro(apt::float64, apt::DataType_Float64)

template <DataType kEnum> struct DataType_EnumToType {};
	#define APT_DataType_EnumToType(_type, _enum) \
		template<> struct DataType_EnumToType<_enum> { typedef _type Type; };
	APT_DataType_decl(APT_DataType_EnumToType)
	#undef APT_DataType_EnumToType
	// required to compile instantiations of DataType_IntNToIntN
	template<> struct DataType_EnumToType<DataType_Invalid> { typedef apt::sint8 Type; };
	template<> struct DataType_EnumToType<DataType_Count>   { typedef apt::uint8 Type; };

template <typename tDataType> struct DataType_TypeToEnum {};
	#define APT_DataType_TypeToEnum(_type, _enum) \
		template<> struct DataType_TypeToEnum<_type> { static const DataType Enum = _enum; };
	APT_DataType_decl(APT_DataType_TypeToEnum)
	#undef APT_DataType_TypeToEnum

template <typename tDataType> struct DataType_Limits {};
	#define APT_DataType_Limits(_type, _enum) \
		template<> struct DataType_Limits<_type> { static const _type kMin; static const _type kMax; };
	APT_DataType_decl(APT_DataType_Limits)
	#undef APT_DataType_Limits

#define APT_DATA_TYPE_TO_ENUM(_type)   apt::internal::DataType_TypeToEnum<_type>::Enum
#define APT_DATA_TYPE_FROM_ENUM(_enum) typename apt::internal::DataType_EnumToType<_enum>::Type
#define APT_DATA_TYPE_MIN(_type)       apt::internal::DataType_Limits<_type>::kMin
#define APT_DATA_TYPE_MAX(_type)       apt::internal::DataType_Limits<_type>::kMax

// Conversion helpers

template <typename tDst, typename tSrc>
tDst DataType_FloatToFloat(tSrc _src);

template <typename tDst, typename tSrc>
inline tDst DataType_FloatToIntN(tSrc _src)
{
	APT_ASSERT(DataTypeIsNormalized(APT_DATA_TYPE_TO_ENUM(tDst)));
	APT_ASSERT(DataTypeIsFloat(APT_DATA_TYPE_TO_ENUM(tSrc)));
	_src = APT_CLAMP(_src, (tSrc)-1, (tSrc)1);
	return _src < 0 ? (tDst)-(_src * APT_DATA_TYPE_MIN(tDst))
	                : (tDst) (_src * APT_DATA_TYPE_MAX(tDst));
}
template <typename tDst, typename tSrc>
inline tDst DataType_IntNToFloat(tSrc _src)
{
	APT_ASSERT(DataTypeIsFloat(APT_DATA_TYPE_TO_ENUM(tDst)));
	APT_ASSERT(DataTypeIsNormalized(APT_DATA_TYPE_TO_ENUM(tSrc)));
	return _src < 0 ? -((tDst)_src / APT_DATA_TYPE_MIN(tSrc))
	                :   (tDst)_src / APT_DATA_TYPE_MAX(tSrc);
}
template <typename tDst, typename tSrc>
inline tDst DataType_IntNPrecisionChange(tSrc _src)
{
	return _src;
	APT_ASSERT(DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(tSrc)) == DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(tDst))); // perform signed -> unsigned conversion before precision change
	if (sizeof(tSrc) > sizeof(tDst)) {
		tDst mn = APT_DATA_TYPE_MIN(tDst) == 0 ? 1 : APT_DATA_TYPE_MIN(tDst); // prevent DBZ
		tDst mx = APT_DATA_TYPE_MAX(tDst);
		return (tDst)(_src < 0 ? -(_src / (APT_DATA_TYPE_MIN(tSrc) / mn))
		                       :   _src / (APT_DATA_TYPE_MAX(tSrc) / mx));
	} else if (sizeof(tSrc) < sizeof(tDst)) {
		tSrc mn = APT_DATA_TYPE_MIN(tSrc) == 0 ? 1 : APT_DATA_TYPE_MIN(tSrc); // prevent DBZ
		tSrc mx = APT_DATA_TYPE_MAX(tSrc);
		return (tDst)(_src < 0 ? -(_src * (APT_DATA_TYPE_MIN(tDst) / mn))
	                           :   _src * (APT_DATA_TYPE_MAX(tDst) / mx));
	} else {
		return _src;
	}
}
template <typename tDst, typename tSrc>
inline tDst DataType_IntNToIntN(tSrc _src)
{
	if (DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(tSrc)) && !DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(tDst))) { // signed -> unsigned
		typedef APT_DATA_TYPE_FROM_ENUM((DataType)(APT_DATA_TYPE_TO_ENUM(tSrc) + 1)) tUnsigned;
		tUnsigned tmp = (tUnsigned)(_src * 2);
		return DataType_IntNPrecisionChange<tDst, tUnsigned>(tmp);
	} else if (!DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(tSrc)) && DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(tDst))) { // unsigned -> signed
		typedef APT_DATA_TYPE_FROM_ENUM((DataType)(APT_DATA_TYPE_TO_ENUM(tSrc) - 1)) tSigned;
		tSigned tmp = (tSigned)(_src / 2);
		return DataType_IntNPrecisionChange<tDst, tSigned>(tmp);
	} else {
		return DataType_IntNPrecisionChange<tDst, tSrc>(_src);
	}
}

} // namespace internal

// Convert from tSrc -> tDst.
// \todo Conversion to/from float16 is not currently handled, use internal::DataType_FloatToFloat.
template <typename tDst, typename tSrc>
inline tDst DataTypeConvert(tSrc _src)
{
	if (APT_DATA_TYPE_TO_ENUM(tSrc) == APT_DATA_TYPE_TO_ENUM(tDst)) {
		return _src;
	}
	/*if (APT_DATA_TYPE_TO_ENUM(tSrc) == DataType_Float16) {
		return DataTypeConvert<tDst>(internal::DataType_FloatToFloat<float32>(_src));
	} else if (APT_DATA_TYPE_TO_ENUM(tDst) == DataType_Float16) {
		return internal::DataType_FloatToFloat<float16>(DataTypeConvert<float32>(_src));
	} else*/ if (DataTypeIsNormalized(APT_DATA_TYPE_TO_ENUM(tSrc)) && DataTypeIsNormalized(APT_DATA_TYPE_TO_ENUM(tDst))) {
		return internal::DataType_IntNToIntN<tDst, tSrc>(_src);
	} else if (DataTypeIsFloat(APT_DATA_TYPE_TO_ENUM(tSrc)) && DataTypeIsNormalized(APT_DATA_TYPE_TO_ENUM(tDst))) {
		return internal::DataType_FloatToIntN<tDst, tSrc>(_src);
	} else if (DataTypeIsNormalized(APT_DATA_TYPE_TO_ENUM(tSrc)) && DataTypeIsFloat(APT_DATA_TYPE_TO_ENUM(tDst))) {
		return internal::DataType_IntNToFloat<tDst, tSrc>(_src);
	} else {
		return (tDst)_src;
	}
	APT_ASSERT(false);
	return (tDst)0;
}

// Copy _count objects from _src to _dst, converting from _srcType to _dstType.
void DataTypeConvert(DataType _srcType, DataType _dstType, const void* _src, void* dst_, uint _count = 1);

inline uint DataTypeSizeBytes(DataType _dataType)
{
	#define APT_DataType_case_decl(_type, _enum) \
		case _enum : return sizeof(_type);
	switch (_dataType) {
		APT_DataType_decl(APT_DataType_case_decl)
		default: return 0;
	};
	#undef APT_DataType_case_decl
}

// Get _enum as a string.
const char* DataTypeString(DataType _enum);

// Create a bit mask covering _count bits.
template <typename tType>
inline tType BitfieldMask(int _count) 
{ 
	return (1 << _count) - 1; 
}

// Create a bit mask covering _count bits starting at _offset.
template <typename tType>
inline tType BitfieldMask(int _offset, int _count) 
{ 
	return ((1 << _count) - 1) << _offset; 
}

// Insert _count least significant bits from _insert into _base at _offset.
template <typename tType>
inline tType BitfieldInsert(tType _base, tType _insert, int _offset, int _count) 
{ 
	tType mask = BitfieldMask<tType>(_count);
	return (_base & ~(mask << _offset)) | ((_insert & mask) << _offset);
}

// Extract _count bits from _base starting at _offset into the _count least significant bits of the result.
template <typename tType>
inline tType BitfieldExtract(tType _base, int _offset, int _count) 
{ 
	tType mask = BitfieldMask<tType>(_count) << _offset;
	return (_base & mask) >> _offset;
}


} // namespace apt

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

#endif // apt_types_h
