#include <apt/types.h>
#include <apt/def.h>

#include <cstring>

namespace apt { namespace refactor
{

	#define APT_DataType_Constants(_baseType) \
		const _baseType DataTypeBase<_baseType>::kMin = std::numeric_limits<_baseType>::min(); \
		const _baseType DataTypeBase<_baseType>::kMax = std::numeric_limits<_baseType>::max()

	APT_DataType_Constants(std::uint8_t);
	APT_DataType_Constants(float);

} }

using std::numeric_limits;

/* \todo This is a mess, could probably be cleaned up/simplified.

	We branch (tag distpach) on whether a normalized conversion is required 
	(if tSrc or tDst is normalized type). For non-normalized conversions this results 
	in a simple cast from tSrc to tDst. Normalized conversions must do a further
	dispatch to select the correct behaviour (see the table below). This is done
	via a single large tag (normalized_convertT) which is templated by all the
	relavent type information.

	kSrc   | kDst   | Behavior
	-------+--------+---------------------------------------------------------
	UN     | F      | src / UN_Max
	-------+--------+---------------------------------------------------------
	SN     | F      | src < 0 ? -(src / SN_Min) 
	       |        |         : src / SN_Max
	-------+--------+---------------------------------------------------------
	F      | UN     | src * UN_Max
	-------+--------+---------------------------------------------------------
	F      | SN     | src < 0 ? src * SN_Min
	       |        |         : src * SN_Max
	-------+--------+---------------------------------------------------------
	UN     | SN     | SN_Min + src (followed by precision conversion)
	-------+--------+---------------------------------------------------------
	SN     | UN     | abs(SN_Min) + src (followed by precision conversion)
	-------+--------+---------------------------------------------------------
	UN_hi  | UN_lo  | src / (UN_hi_Max / UN_lo_Max)
	-------+--------+---------------------------------------------------------
	UN_lo  | UN_hi  | src * (UN_hi_Max / UN_lo_Max)
	-------+--------+---------------------------------------------------------
	SN_hi  | SN_lo  | src < 0 ? src / (SN_hi_Min / SN_lo_Min)
	       |        |         : src / (SN_hi_Max / SN_lo_Max)
	-------+--------+---------------------------------------------------------
	SN_lo  | SN_hi  | src < 0 ? src * (SN_hi_Min / SN_lo_Min)
	       |        |         : src * (SN_hi_Max ? SN_lo_Max)
	-------+--------+---------------------------------------------------------
*/

namespace apt {

#define APT_DataType_convert(_rule) \
	template <typename tSrc, typename tDst> \
	inline static tDst _rule(tSrc _src)
APT_DataType_convert(unsigned_to_float)
{
	return (tDst)_src / (tDst)numeric_limits<tSrc>::max();
}
APT_DataType_convert(signed_to_float)
{
	return _src < 0 ? -((tDst)_src / (tDst)numeric_limits<tSrc>::min()) 
	                : (tDst)_src / (tDst)numeric_limits<tSrc>::max();
}
APT_DataType_convert(float_to_unsigned)
{
	return (tDst)(_src * (tSrc)numeric_limits<tDst>::max());
}
APT_DataType_convert(float_to_signed)
{
	return (tDst)(_src < 0 ? -(_src * (tSrc)numeric_limits<tDst>::min())
	                       : _src * (tSrc)numeric_limits<tDst>::max());
}
APT_DataType_convert(unsigned_to_signed)
{
	return (tDst)(numeric_limits<tDst>::min() + _src);
}
APT_DataType_convert(signed_to_unsigned)
{
	return (tDst)(abs(numeric_limits<tSrc>::min()) + _src);
}
APT_DataType_convert(unsigned_change_precision)
{
	if (sizeof(tSrc) >= sizeof(tDst)) {
		return (tDst)(_src / (numeric_limits<tSrc>::max() / numeric_limits<tDst>::max()));
	} else {
		return (tDst)(_src * (numeric_limits<tDst>::max() / numeric_limits<tSrc>::max()));
	}
}
APT_DataType_convert(signed_change_precision)
{
	if (sizeof(tSrc) >= sizeof(tDst)) {
		return (tDst)(_src < 0 ? _src / (numeric_limits<tSrc>::min() / numeric_limits<tDst>::min())
		                       : _src / (numeric_limits<tSrc>::max() / numeric_limits<tDst>::max()));
	} else {
		return (tDst)(_src < 0 ? _src * (numeric_limits<tDst>::min() / numeric_limits<tSrc>::min())
	                           : _src * (numeric_limits<tDst>::max() / numeric_limits<tSrc>::max()));
	}
}
#undef APT_DataType_Convert

template <bool is_src_int, bool is_dst_int, bool is_src_signed, bool is_dst_signed, bool same_size>
struct normalized_convertT{};
#define APT_DataType_dispatch_convert(is_src_int, is_dst_int, is_src_signed, is_dst_signed, same_size) \
	template <typename tSrc, typename tDst> \
	inline static tDst normalized_convert(tSrc _src, normalized_convertT<is_src_int, is_dst_int, is_src_signed, is_dst_signed, same_size>)
APT_DataType_dispatch_convert(false, true, true, true, false)
{
	return float_to_signed<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(false, true, true, false, false)
{
	return float_to_unsigned<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(false, true, true, true, true) // float/int same size
{
	return float_to_signed<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(false, true, true, false, true) // float/int same size
{
	return float_to_unsigned<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, false, true, true, false)
{
	return signed_to_float<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, false, false, true, false)
{
	return signed_to_float<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, false, true, true, true) // float/int same size
{
	return signed_to_float<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, false, false, true, true) // float/int same size
{
	return signed_to_float<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, true, true, false, true)
{
	return signed_to_unsigned<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, true, false, true, true)
{
	return unsigned_to_signed<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, true, true, true, false)
{
	return signed_change_precision<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, true, false, false, false)
{
	return unsigned_change_precision<tSrc, tDst>(_src);
}
APT_DataType_dispatch_convert(true, true, true, false, false)
{
	typedef typename DataType::ToType<(DataType::Enum)(DataType::ToEnum<tSrc>::Enum + 1)>::Type tUnsigned;
	tUnsigned tmp = signed_to_unsigned<tSrc, tUnsigned>(_src);
	return unsigned_change_precision<tUnsigned, tDst>(tmp);
}
APT_DataType_dispatch_convert(true, true, false, true, false)
{
	typedef typename DataType::ToType<(DataType::Enum)(DataType::ToEnum<tSrc>::Enum - 1)>::Type tSigned;
	tSigned tmp = unsigned_to_signed<tSrc, tSigned>(_src);
	return signed_change_precision<tSigned, tDst>(tmp);
}
APT_DataType_dispatch_convert(true, true, true, true, true)
{
	return (tDst)_src;
}
APT_DataType_dispatch_convert(true, true, false, false, true)
{
	return (tDst)_src;
}
#undef APT_DataType_dispatch_convert

template <bool require_normalized>
struct convertT{};
#define APT_DataType_dispatch_convert(require_normalized) \
	template <typename tSrc, typename tDst> \
	inline tDst static convert(tSrc _src, convertT<require_normalized>)
APT_DataType_dispatch_convert(true)
{
	using std::numeric_limits;
	return normalized_convert<tSrc, tDst>(_src,
		normalized_convertT<
			numeric_limits<tSrc>::is_integer,
			numeric_limits<tDst>::is_integer,
			numeric_limits<tSrc>::is_signed,
			numeric_limits<tDst>::is_signed,
			sizeof(tSrc) == sizeof(tDst)>()
		);
}
APT_DataType_dispatch_convert(false)
{
	return (tDst)_src;
}
#undef APT_DataType_dispatch_convert

template <typename tType>
struct convert_traits {};
	template <> struct convert_traits<sint8>   { static const bool is_normalized = false; };
	template <> struct convert_traits<uint8>   { static const bool is_normalized = false; };
	template <> struct convert_traits<sint16>  { static const bool is_normalized = false; };
	template <> struct convert_traits<uint16>  { static const bool is_normalized = false; };
	template <> struct convert_traits<sint32>  { static const bool is_normalized = false; };
	template <> struct convert_traits<uint32>  { static const bool is_normalized = false; };
	template <> struct convert_traits<sint64>  { static const bool is_normalized = false; };
	template <> struct convert_traits<uint64>  { static const bool is_normalized = false; };

	template <> struct convert_traits<sint8N>  { static const bool is_normalized = true; };
	template <> struct convert_traits<uint8N>  { static const bool is_normalized = true; };
	template <> struct convert_traits<sint16N> { static const bool is_normalized = true; };
	template <> struct convert_traits<uint16N> { static const bool is_normalized = true; };
	template <> struct convert_traits<sint32N> { static const bool is_normalized = true; };
	template <> struct convert_traits<uint32N> { static const bool is_normalized = true; };
	template <> struct convert_traits<sint64N> { static const bool is_normalized = true; };
	template <> struct convert_traits<uint64N> { static const bool is_normalized = true; };

	template <> struct convert_traits<float16> { static const bool is_normalized = false; };
	template <> struct convert_traits<float32> { static const bool is_normalized = false; };
	template <> struct convert_traits<float64> { static const bool is_normalized = false; };

template <typename tSrc, typename tDst>
tDst DataType::Convert(tSrc _src)
{
	return convert<tSrc, tDst>(_src, convertT< (convert_traits<tSrc>::is_normalized || convert_traits<tDst>::is_normalized) >());
}

void DataType::Convert(DataType _srcType, DataType _dstType, const void* _src, void* dst_, uint _count)
{
	if (_srcType == _dstType) {
		memcpy(dst_, _src, DataType::GetSizeBytes(_srcType) * _count);

	}  else {
		#define case_decl(_srcEnum, _srcTypename) \
			case _srcEnum: \
				switch (_dstType) { \
					case Sint8:   *((sint8*  )dst_) = Convert<_srcTypename, sint8  >(*((const _srcTypename*)_src)); break; \
					case Uint8:   *((uint8*  )dst_) = Convert<_srcTypename, uint8  >(*((const _srcTypename*)_src)); break; \
					case Sint16:  *((sint16* )dst_) = Convert<_srcTypename, sint16 >(*((const _srcTypename*)_src)); break; \
					case Uint16:  *((uint16* )dst_) = Convert<_srcTypename, uint16 >(*((const _srcTypename*)_src)); break; \
					case Sint32:  *((sint32* )dst_) = Convert<_srcTypename, sint32 >(*((const _srcTypename*)_src)); break; \
					case Uint32:  *((uint32* )dst_) = Convert<_srcTypename, uint32 >(*((const _srcTypename*)_src)); break; \
					case Sint64:  *((sint64* )dst_) = Convert<_srcTypename, sint64 >(*((const _srcTypename*)_src)); break; \
					case Uint64:  *((uint64* )dst_) = Convert<_srcTypename, uint64 >(*((const _srcTypename*)_src)); break; \
					case Sint8N:  *((sint8N* )dst_) = Convert<_srcTypename, sint8N >(*((const _srcTypename*)_src)); break; \
					case Uint8N:  *((uint8N* )dst_) = Convert<_srcTypename, uint8N >(*((const _srcTypename*)_src)); break; \
					case Sint16N: *((sint16N*)dst_) = Convert<_srcTypename, sint16N>(*((const _srcTypename*)_src)); break; \
					case Uint16N: *((uint16N*)dst_) = Convert<_srcTypename, uint16N>(*((const _srcTypename*)_src)); break; \
					case Sint32N: *((sint32N*)dst_) = Convert<_srcTypename, sint32N>(*((const _srcTypename*)_src)); break; \
					case Uint32N: *((uint32N*)dst_) = Convert<_srcTypename, uint32N>(*((const _srcTypename*)_src)); break; \
					case Sint64N: *((sint64N*)dst_) = Convert<_srcTypename, sint64N>(*((const _srcTypename*)_src)); break; \
					case Uint64N: *((uint64N*)dst_) = Convert<_srcTypename, uint64N>(*((const _srcTypename*)_src)); break; \
					case Float32: *((float32*)dst_) = Convert<_srcTypename, float32>(*((const _srcTypename*)_src)); break; \
					case Float64: *((float64*)dst_) = Convert<_srcTypename, float64>(*((const _srcTypename*)_src)); break; \
				}; \
				break				

		for (uint i = 0; i < _count; ++i) {
			switch (_srcType) {
				case_decl(Sint8  , sint8  );
				case_decl(Uint8  , uint8  );
				case_decl(Sint16 , sint16 );
				case_decl(Uint16 , uint16 );
				case_decl(Sint32 , sint32 );
				case_decl(Uint32 , uint32 );
				case_decl(Sint64 , sint64 );
				case_decl(Uint64 , uint64 );
				case_decl(Sint8N , sint8N );
				case_decl(Uint8N , uint8N );
				case_decl(Sint16N, sint16N);
				case_decl(Uint16N, uint16N);
				case_decl(Sint32N, sint32N);
				case_decl(Uint32N, uint32N);
				case_decl(Sint64N, sint64N);
				case_decl(Uint64N, uint64N);
				//case_decl(kFloat16, float16);
				case_decl(Float32, float32);
				case_decl(Float64, float64);
				default: APT_ASSERT(false); break;
			};
			_src = (const char*)_src + GetSizeBytes(_srcType);
			dst_ = (char*)dst_ + GetSizeBytes(_dstType);
		}
	}
}

} // namespace apt
