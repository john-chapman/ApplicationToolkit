#include <apt/math.h>

using namespace apt;

mat4 apt::TransformationMatrix(const vec3& _translation, const mat3& _rotationScale)
{
	return mat4(
		vec4(_rotationScale[0], 0.0f),
		vec4(_rotationScale[1], 0.0f),
		vec4(_rotationScale[2], 0.0f),
		vec4(_translation,      1.0f)
		);
}

mat4 apt::TransformationMatrix(const vec3& _translation, const quat& _rotation, const vec3& _scale)
{
	mat4 ret = linalg::rotation_matrix(_rotation);
	ret[0].x *= _scale.x;
	ret[1].y *= _scale.y;
	ret[2].z *= _scale.z;
	ret[3] = vec4(_translation, 1.0f);
	return ret;
}

mat3 apt::TransformationMatrix(const vec2& _translation, const mat2& _rotationScale)
{
	return mat3(
		vec3(_rotationScale[0], 0.0f),
		vec3(_rotationScale[1], 0.0f),
		vec3(_translation,      1.0f)
		);
}

mat4 apt::TranslationMatrix(const vec3& _translation)
{
	return linalg::translation_matrix(_translation);
}

mat4 apt::RotationMatrix(const vec3& _axis, float _radians)
{
	//return rotation_matrix(rotation_quat(_axis, _radians));
 // the following has better precision
	float c   = cosf(_radians);
	float s   = sinf(_radians);
	vec3  rca = (1.0f - c) * _axis;
	return mat4(
		vec4(c + rca[0] * _axis[0],              rca[0] * _axis[1] + s * _axis[2],   rca[0] * _axis[2] - s * _axis[1],   0.0f),
		vec4(rca[1] * _axis[0] - s * _axis[2],   c + rca[1] * _axis[1],              rca[1] * _axis[2] + s * _axis[0],   0.0f),
		vec4(rca[2] * _axis[0] + s * _axis[1],   rca[2] * _axis[1] - s * _axis[0],   c + rca[2] * _axis[2],              0.0f),
		vec4(0.0f, 0.0f, 0.0f, 1.0f)
		);
}

mat4 apt::RotationMatrix(const quat& _q)
{
	return linalg::rotation_matrix(_q);
}

quat apt::RotationQuaternion(const vec3& _axis, float _radians)
{
	return linalg::rotation_quat(_axis, _radians);
}

quat apt::RotationQuaternion(const mat3& _rotation)
{
	return linalg::rotation_quat(_rotation);
}

mat4 apt::ScaleMatrix(const vec3& _scale)
{
	return linalg::scaling_matrix(_scale);
}

vec3 apt::GetTranslation(const mat4& _m)
{
	return _m[3].xyz();
}

vec2 apt::GetTranslation(const mat3& _m)
{
	return _m[2].xy();
}

mat3 apt::GetRotation(const mat4& _m)
{
	mat3 ret = mat3(_m);
	ret[0] = Normalize(ret[0]);
	ret[1] = Normalize(ret[1]);
	ret[2] = Normalize(ret[2]);
	return ret;
}

mat2 apt::GetRotation(const mat3& _m)
{
	mat2 ret = mat2(_m);
	ret[0] = Normalize(ret[0]);
	ret[1] = Normalize(ret[1]);
	return ret;
}

vec3 apt::GetScale(const mat4& _m)
{
	vec3 ret;
	ret.x = Length(_m[0].xyz());
	ret.y = Length(_m[1].xyz());
	ret.z = Length(_m[2].xyz());
	return ret;
}

vec3 apt::ToEulerXYZ(const mat3& _m)
{
 // http://www.staff.city.ac.uk/~sbbh653/publications/euler.pdf
	vec3 ret;
	if_likely (fabs(_m[0][2]) < 1.0f) {
		ret.y = -asinf(_m[0][2]);
		float c = 1.0f / cosf(ret.y);
		ret.x = atan2f(_m[1][2] * c, _m[2][2] * c);
		ret.z = atan2f(_m[0][1] * c, _m[0][0] * c);
	} else {
		ret.z = 0.0f;
		if (!(_m[0][2] > -1.0f)) {
			ret.x = ret.z + atan2f(_m[1][0], _m[2][0]);
			ret.y = kHalfPi;
		} else {
			ret.x = -ret.z + atan2f(-_m[1][0], -_m[2][0]);			
			ret.y = -kHalfPi;
		}
	}
	return ret;
}

mat3 apt::FromEulerXYZ(const vec3& _euler)
{
// https://www.geometrictools.com/Documentation/EulerAngles.pdf
	float cx = cosf(_euler.x);
	float sx = sinf(_euler.x);
	float cy = cosf(_euler.y);
	float sy = sinf(_euler.y);
	float cz = cosf(_euler.z);
	float sz = sinf(_euler.z);
	return mat3(
		vec3( cy * cz,     cz * sx * sy + cx * sz,   -cx * cz * sy + sx * sz),
		vec3(-cy * sz,     cx * cz - sx * sy * sz,    cz * sx + cx * sy * sz),
		vec3( sy,         -cy * sx,                   cx * cy)
		);
}

vec3 apt::SphericalToCartesian(float _radius, float _azimuth, float _elevation)
{
	return vec3(
		_radius * cos(_azimuth) * sin(_elevation),
		_radius * cos(_elevation),
		_radius * sin(_azimuth) * sin(_elevation)
		);
}

vec3 apt::CartesianToSpherical(const vec3& _cartesian)
{
	float radius = Max(FLT_EPSILON, Length(_cartesian));
	return vec3(
		radius,
		atan2(_cartesian.z, _cartesian.x),
		acos(_cartesian.y / radius)
		);
}

mat4 apt::Transpose(const mat4& _m)
{
	return linalg::transpose(_m);
}
mat3 apt::Transpose(const mat3& _m)
{
	return linalg::transpose(_m);
}
mat2 apt::Transpose(const mat2& _m)
{
	return linalg::transpose(_m);
}

mat4 apt::Inverse(const mat4& _m)
{
	return linalg::inverse(_m);
}
mat3 apt::Inverse(const mat3& _m)
{
	return linalg::inverse(_m);
}
mat2 apt::Inverse(const mat2& _m)
{
	return linalg::inverse(_m);
}
quat apt::Inverse(const quat& _q)
{
	return linalg::qinv(_q);
}
quat apt::Conjugate(const quat& _q)
{
	return linalg::qconj(_q);
}

mat4 apt::AffineInverse(const mat4& _m)
{
	mat3 rs = Transpose(mat3(_m));
	vec3 t  = rs * -_m[3].xyz();
	return TransformationMatrix(t, rs);
}
mat3 apt::AffineInverse(const mat3& _m)
{
	mat2 rs = Transpose(mat2(_m));
	vec2 t  = rs * -_m[2].xy();
	return TransformationMatrix(t, rs);
}

mat4 apt::AlignX(const vec3& _axis, const vec3& _up)
{
	vec3 y, z;
	y = _up - _axis * Dot(_up, _axis);
	float ylen = Length(y);
	if_unlikely (ylen < FLT_EPSILON) {
		vec3 k = vec3(1.0f, 0.0f, 0.0f);
		y = k - _axis * Dot(k, _axis);
		ylen = Length(y);
		if_unlikely (ylen < FLT_EPSILON) {
			k = vec3(0.0f, 0.0f, 1.0f);
			y = k - _axis * Dot(k, _axis);
			ylen = Length(y);
		}
	}
	y = y / ylen;
	z = Cross(_axis, y);

	return mat4(
		vec4(_axis.x, _axis.y, _axis.z, 0.0f),
		vec4(y.x,     y.y,     y.z,     0.0f),
		vec4(z.x,     z.y,     z.z,     0.0f),
		vec4(0.0f,    0.0f,    0.0f,    1.0f)
		);
}

mat4 apt::AlignY(const vec3& _axis, const vec3& _up)
{
	vec3 x, z;
	z = _up - _axis * Dot(_up, _axis);
	float zlen = Length(z);
	if_unlikely (zlen < FLT_EPSILON) {
		vec3 k = vec3(1.0f, 0.0f, 0.0f);
		z = k - _axis * Dot(k, _axis);
		zlen = Length(z);
		if_unlikely (zlen < FLT_EPSILON) {
			k = vec3(0.0f, 0.0f, 1.0f);
			z = k - _axis * Dot(k, _axis);
			zlen = Length(z);
		}
	}
	z = z / zlen;
	x = Cross(z, _axis);

	return mat4(
		vec4(x.x,     x.y,     x.z,     0.0f),
		vec4(_axis.x, _axis.y, _axis.z, 0.0f),
		vec4(z.x,     z.y,     z.z,     0.0f),
		vec4(0.0f,    0.0f,    0.0f,    1.0f)
		);
}

mat4 apt::AlignZ(const vec3& _axis, const vec3& _up)
{
	vec3 x, y;
	y = _up - _axis * Dot(_up, _axis);
	float ylen = Length(y);
	if_unlikely (ylen < FLT_EPSILON) {
		vec3 k = vec3(1.0f, 0.0f, 0.0f);
		y = k - _axis * Dot(k, _axis);
		ylen = Length(y);
		if_unlikely (ylen < FLT_EPSILON) {
			k = vec3(0.0f, 0.0f, 1.0f);
			y = k - _axis * Dot(k, _axis);
			ylen = Length(y);
		}
	}
	y = y / ylen;
	x = Cross(y, _axis);

	return mat4(
		vec4(x.x,     x.y,     x.z,     0.0f),
		vec4(y.x,     y.y,     y.z,     0.0f),
		vec4(_axis.x, _axis.y, _axis.z, 0.0f),
		vec4(0.0f,    0.0f,    0.0f,    1.0f)
		);
}
	
mat4 apt::LookAt(const vec3& _from, const vec3& _to, const vec3& _up)
{
	mat4 ret = AlignZ(Normalize(_to - _from), _up);
	ret[3] = vec4(_from, 1.0f);
	return ret;
}
