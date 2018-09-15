#pragma once

#include <apt/apt.h>
#include <apt/math.h>

namespace apt {

////////////////////////////////////////////////////////////////////////////////
// PRNG_CMWC
// Uniform PRNG via 'complimentary multiply-with-carry' (George Marsaglia's 
// 'Mother of All PRNGs'). Adapted from Agner Fog's implementation found here:
// http://www.agner.org/random/. Use as template parameter to Rand (see below).
////////////////////////////////////////////////////////////////////////////////
class PRNG_CMWC
{
public:
	PRNG_CMWC(uint32 _seed = 1) { seed(_seed); }

	void   seed(uint32 _seed);
	uint32 raw();

private:
	uint32 m_state[5];
};

////////////////////////////////////////////////////////////////////////////////
// Rand
// Uniform random number API, templated by generator type. Typical usage:
//    Rand<> rnd;                     // instantiate with default PRNG
//    rnd.get<bool>();                // return true/false
//    rnd.get<float>();               // in [0,1]
//    rnd.get<int>(-10,10);           // in [-10,10]
//    rnd.get<float>(-10.0f, 10.0f);  // in [-10,10]
////////////////////////////////////////////////////////////////////////////////
template <typename PRNG = PRNG_CMWC>
class Rand
{
public:
	Rand(uint32 _seed = 1): m_prng(_seed)   {}

	void   seed(uint32 _seed)               { m_prng.seed(_seed); }
	uint32 raw()                            { return m_prng.raw(); }

	template <typename tType>
	tType  get();
	template <typename tType>
	tType  get(const tType _min, const tType _max);

private:
	PRNG m_prng;
};

namespace internal {

template<typename tType> tType RandGet(uint32 _raw);
template<typename tType> tType RandGet(uint32 _raw, const tType _min, const tType _max);

template<> bool RandGet<bool>(uint32 _raw)
{
	return (_raw >> 31) != 0;
}
template<> float32 RandGet<float32>(uint32 _raw)
{
	internal::iee754_f32 x;
	x.u = _raw;
	x.u &= 0x007fffffu;
	x.u |= 0x3f800000u;
	return x.f - 1.0f;
}
 
template<> sint32 RandGet(uint32 _raw, const sint32 _min, const sint32 _max)
{
	uint64 i = (uint64)_raw * (_max - _min + 1);
	uint32 j = (uint32)(i >> 32);
	return (sint32)j + _min;
}
template<> float32 RandGet(uint32 _raw, const float32 _min, const float32 _max)
{
	float32 f = RandGet<float32>(_raw);
	return _min + f * (_max - _min);
}
 
template<> vec2 RandGet(uint32 _raw, const vec2 _min, const vec2 _max)
{
	return vec2(
		RandGet(_raw, _min.x, _max.x),
		RandGet(_raw, _min.y, _max.y)
		);
}
template<> vec3 RandGet(uint32 _raw, const vec3 _min, const vec3 _max)
{
	return vec3(
		RandGet(_raw, _min.x, _max.x),
		RandGet(_raw, _min.y, _max.y),
		RandGet(_raw, _min.z, _max.z)
		);
}
template<> vec4 RandGet(uint32 _raw, const vec4 _min, const vec4 _max)
{
	return vec4(
		RandGet(_raw, _min.x, _max.x),
		RandGet(_raw, _min.y, _max.y),
		RandGet(_raw, _min.z, _max.z),
		RandGet(_raw, _min.w, _max.w)
		);
}

} // namespace internal


template <typename PRNG>
template <typename tType> tType Rand<PRNG>::get() 
{
	return internal::RandGet<tType>(); 
}

template <typename PRNG>
template <typename tType> tType Rand<PRNG>::get(const tType _min, const tType _max) 
{
	return internal::RandGet<tType>(_min, _max);
}

} // namespace apt
