#pragma once
#ifndef apt_math_h
#define apt_math_h

// \todo ?
//#define APT_MATH_SIMD

#include <linalg/linalg.h>

namespace apt {
	typedef linalg::aliases::float2   vec2;
	typedef linalg::aliases::float3   vec3;
	typedef linalg::aliases::float4   vec4;
	typedef linalg::aliases::uint2    uvec2;
	typedef linalg::aliases::uint3    uvec3;
	typedef linalg::aliases::uint4    uvec4;
	typedef linalg::aliases::int2     ivec2;
	typedef linalg::aliases::int3     ivec3;
	typedef linalg::aliases::int4     ivec4;
	typedef linalg::aliases::bool2    bvec2;
	typedef linalg::aliases::bool3    bvec3;
	typedef linalg::aliases::bool4    bvec4;
	typedef linalg::aliases::float2x2 mat2;
	typedef linalg::aliases::float3x3 mat3;
	typedef linalg::aliases::float4x4 mat4;
	typedef linalg::aliases::float4   quat;

	using namespace linalg;

	constexpr float kPi      = 3.14159265359f;
	constexpr float kTwoPi   = 2.0f * kPi;
	constexpr float kHalfPi  = 0.5f * kPi;
}

#include <apt/apt.h>

namespace apt {

	// Transformation matrix helpers.
	mat4 TransformationMatrix(const vec3& _translation, const mat3& _rotationScale);
	mat4 TransformationMatrix(const vec3& _translation, const quat& _rotation, const vec3& _scale = vec3(1.0f));
	mat4 TranslationMatrix(const vec3& _translation);
	mat4 RotationMatrix(const vec3& _axis, float _radians);
	mat4 RotationMatrix(const quat& _q);
	quat RotationQuaternion(const vec3& _axis, float _radians);
	mat4 ScaleMatrix(const vec3& _scale);
	vec3 GetTranslation(const mat4& _m);
	mat3 GetRotation(const mat4& _m);
	vec3 GetScale(const mat4& _m);
	vec3 ToEulerXYZ(const mat3& _m);
	mat3 FromEulerXYZ(const vec3& _euler);
	mat4 Inverse(const mat4& _m);
	mat4 AffineInverse(const mat4& _m);

	inline vec3 TransformPosition(const mat4& _m, const vec3& _p)  { return mul(_m, vec4(_p, 1.0f)).xyz(); }
	inline vec2 TransformPosition(const mat3& _m, const vec2& _p)  { return mul(_m, vec3(_p, 1.0f)).xy();  }
	inline vec3 TransformDirection(const mat4& _m, const vec3& _p) { return mul(_m, vec4(_p, 0.0f)).xyz(); }
	inline vec2 TransformDirection(const mat3& _m, const vec2& _p) { return mul(_m, vec3(_p, 0.0f)).xy();  }

	// Get an orthonormal bases with X/Y/Z aligned with _axis.
	mat4 AlignX(const vec3& _axis, const vec3& _up = vec3(0.0f, 1.0f, 0.0f));
	mat4 AlignY(const vec3& _axis, const vec3& _up = vec3(0.0f, 1.0f, 0.0f));
	mat4 AlignZ(const vec3& _axis, const vec3& _up = vec3(0.0f, 1.0f, 0.0f));
	
	// Matrix with position = _from and +Z = (_to - _from).
	mat4 LookAt(const vec3& _from, const vec3& _to, const vec3& _up = vec3(0.0f, 1.0f, 0.0f));

	// Convert between radians and degrees.
	inline float Degrees(float _radians)      { return _radians * (180.0f / kPi); }
	inline float Radians(float _degrees)      { return _degrees * (kPi / 180.0f); }

	// Return the fractional part of _x (elementwise for vector/matrix types).
	template <typename tType>
	inline tType Fract(const tType& _x)       { return linalg::fract(_x); }
		inline float Fract(float _x)          { return _x - std::floor(_x); }

	// Return the -1 if _x < 0, else 1 (elementwise for vector/matrix types).
	template <typename tType>
	inline tType Sign(const tType& _x)        { return linalg::copysign(tType(1), _x); }
		inline float Sign(const float& _x)    { return std::copysign(1.0f, _x); }

	// \todo Rename, add a templated interface for integral types, plus interface for e.g. random rotations
	class LCG
	{
		static const uint kMultiplier  = 48271u;
		static const uint kIncrement   = 0u;
		static const uint kModulus     = 2147483647u;
		static const uint kDefaultSeed = 1u;
		
		uint m_seed;
		
	public:
		LCG(uint _seed = kDefaultSeed)
			: m_seed(_seed) 
		{
		}
	
		void  seed(uint _seed)               { m_seed = _seed; }
		void  discard(uint _count = 1u)      { while (_count > 0) { rand(); --_count; } }

		int   rand()                         { return (int)urand(); }
		int   rand(int _max)                 { return (int)urand() % _max; }
		int   rand(int _min, int _max)       { return _min + (int)urand() % (_max - _min); }

		uint  urand()                        { m_seed = (m_seed * kMultiplier + kIncrement) % kModulus; return m_seed; }
		uint  urand(uint _max)               { return urand() % _max; }
		uint  urand(uint _min, uint _max)    { return _min + urand() % (_max - _min); }

		float frand()                        { return (float)urand() / (float)0x7fffffffu; }
		float frand(float _max)              { return frand() * _max; }
		float frand(float _min, float _max)  { return _min + frand() * (_max - _min); }
	};
}

#endif // frm_math_h
