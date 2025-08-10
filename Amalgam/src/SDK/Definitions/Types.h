#pragma once
#include <algorithm>
#include <vector>
#include <deque>
#include <string>
#include <format>

#define PI 3.14159265358979323846
#define M_RADPI 57.295779513082
#define DEG2RAD(x) ((float)(x) * (float)((float)(PI) / 180.f))
#define RAD2DEG(x) ((float)(x) * (float)(180.f / (float)(PI)))

#pragma warning (disable : 26495)

class Vec2
{
public:
	float x = 0.f, y = 0.f;

public:
	inline void Zero()
	{
		x = y = 0.f;
	}

	inline Vec2(float X = 0.f, float Y = 0.f)
	{
		x = X; y = Y;
	}

	inline Vec2(float* v)
	{
		x = v[0]; y = v[1];
	}

	inline Vec2(const float* v)
	{
		x = v[0]; y = v[1];
	}

	inline Vec2(const Vec2& v)
	{
		x = v.x; y = v.y;
	}

	inline Vec2& operator=(const Vec2& v)
	{
		x = v.x; y = v.y; return *this;
	}

	inline float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	inline float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	inline bool operator==(const Vec2& v) const
	{
		return x == v.x && y == v.y;
	}

	inline bool operator!=(const Vec2& v) const
	{
		return x != v.x || y != v.y;
	}

	inline Vec2& operator+=(const Vec2& v)
	{
		x += v.x; y += v.y; return *this;
	}

	inline Vec2& operator-=(const Vec2& v)
	{
		x -= v.x; y -= v.y; return *this;
	}

	inline Vec2& operator*=(const Vec2& v)
	{
		x *= v.x; y *= v.y; return *this;
	}

	inline Vec2& operator/=(const Vec2& v)
	{
		x /= v.x; y /= v.y; return *this;
	}

	inline Vec2& operator+=(float v)
	{
		x += v; y += v; return *this;
	}

	inline Vec2& operator-=(float v)
	{
		x -= v; y -= v; return *this;
	}

	inline Vec2& operator*=(float v)
	{
		x *= v; y *= v; return *this;
	}

	inline Vec2& operator/=(float v)
	{
		x /= v; y /= v; return *this;
	}

	inline Vec2 operator+(const Vec2& v) const
	{
		return Vec2(x + v.x, y + v.y);
	}

	inline Vec2 operator-(const Vec2& v) const
	{
		return Vec2(x - v.x, y - v.y);
	}

	inline Vec2 operator*(const Vec2& v) const
	{
		return Vec2(x * v.x, y * v.y);
	}

	inline Vec2 operator/(const Vec2& v) const
	{
		return Vec2(x / v.x, y / v.y);
	}

	inline Vec2 operator+(float v) const
	{
		return Vec2(x + v, y + v);
	}

	inline Vec2 operator-(float v) const
	{
		return Vec2(x - v, y - v);
	}

	inline Vec2 operator*(float v) const
	{
		return Vec2(x * v, y * v);
	}

	inline Vec2 operator/(float v) const
	{
		return Vec2(x / v, y / v);
	}

	inline void Set(float X = 0.f, float Y = 0.f)
	{
		x = X; y = Y;
	}

	inline float Min() const
	{
		return std::min<float>(x, y);
	}

	inline float Max() const
	{
		return std::max<float>(x, y);
	}

	inline Vec2 Min(const Vec2& v) const
	{
		return Vec2(std::min<float>(x, v.x), std::min<float>(y, v.y));
	}

	inline Vec2 Max(const Vec2& v) const
	{
		return Vec2(std::max<float>(x, v.x), std::max<float>(y, v.y));
	}

	inline Vec2 Clamp(const Vec2& v1, const Vec2& v2) const
	{
		return Max(v1).Min(v2);
	}

	inline Vec2 Min(float v) const
	{
		return Vec2(std::min<float>(x, v), std::min<float>(y, v));
	}

	inline Vec2 Max(float v) const
	{
		return Vec2(std::max<float>(x, v), std::max<float>(y, v));
	}

	inline Vec2 Clamp(float v1, float v2) const
	{
		return Max(v1).Min(v2);
	}

	inline Vec2 Lerp(const Vec2& v, float t) const
	{
		return { x + (v.x - x) * t, y + (v.y - y) * t };
	}

	inline Vec2 Lerp(float v, float t) const
	{
		return { x + (v - x) * t, y + (v - y) * t };
	}

	inline Vec2 DeltaAngle(const Vec2& v) const
	{
		auto deltaAngle = [](const float flAngleA, const float flAngleB)
			{
				float flOut = fmodf((flAngleA - flAngleB) + 180.f, 360.f);
				return flOut += flOut < 0 ? 180.f : -180.f;
			};

		return { deltaAngle(x, v.x), deltaAngle(y, v.y) };
	}

	inline Vec2 DeltaAngle(float v) const
	{
		auto deltaAngle = [](const float flAngleA, const float flAngleB)
			{
				float flOut = fmodf((flAngleA - flAngleB) + 180.f, 360.f);
				return flOut += flOut < 0 ? 180.f : -180.f;
			};

		return { deltaAngle(x, v), deltaAngle(y, v) };
	}

	inline Vec2 LerpAngle(const Vec2& v, float t) const
	{
		auto shortDist = [](const float flAngleA, const float flAngleB)
			{
				const float flDelta = fmodf((flAngleA - flAngleB), 360.f);
				return fmodf(2 * flDelta, 360.f) - flDelta;
			};
		return { x - shortDist(x, v.x) * t, y - shortDist(y, v.y) * t };
	}

	inline Vec2 LerpAngle(float v, float t) const
	{
		auto shortDist = [](const float flAngleA, const float flAngleB)
			{
				const float flDelta = fmodf((flAngleA - flAngleB), 360.f);
				return fmodf(2 * flDelta, 360.f) - flDelta;
			};
		return { x - shortDist(x, v) * t, y - shortDist(y, v) * t };
	}

	inline float Length(void) const
	{
		return sqrtf(x * x + y * y);
	}

	inline float LengthSqr(void) const
	{
		return (x * x + y * y);
	}

	inline float DistTo(const Vec2& v) const
	{
		return (*this - v).Length();
	}

	inline float DistToSqr(const Vec2& v) const
	{
		return (*this - v).LengthSqr();
	}

	inline float Dot(const Vec2& v) const
	{
		return x * v.x + y * v.y;
	}

	inline bool IsZero(void) const
	{
		return fabsf(x) < 0.001f &&
			   fabsf(y) < 0.001f;
	}
};
using Vector2D = Vec2;

class Vec3
{
public:
	float x = 0.f, y = 0.f, z = 0.f;

public:
	inline void Zero()
	{
		x = y = z = 0.f;
	}

	inline Vec3(float X = 0.f, float Y = 0.f, float Z = 0.f)
	{
		x = X; y = Y; z = Z;
	}

	inline Vec3(float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	inline Vec3(const float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	inline Vec3(const Vec3& v)
	{
		x = v.x; y = v.y; z = v.z;
	}

	inline Vec3(const Vec2& v)
	{
		x = v.x; y = v.y; z = 0.f;
	}

	inline Vec3& operator=(const Vec3& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}

	inline float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	inline float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	inline bool operator==(const Vec3& v) const
	{
		return x == v.x && y == v.y && z == v.z;
	}

	inline bool operator!=(const Vec3& v) const
	{
		return x != v.x || y != v.y || z != v.z;
	}

	inline Vec3& operator+=(const Vec3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	inline Vec3& operator-=(const Vec3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	inline Vec3& operator*=(const Vec3& v)
	{
		x *= v.x; y *= v.y; z *= v.z; return *this;
	}

	inline Vec3& operator/=(const Vec3& v)
	{
		x /= v.x; y /= v.y; z /= v.z; return *this;
	}

	inline Vec3& operator+=(float v)
	{
		x += v; y += v; z += v; return *this;
	}

	inline Vec3& operator-=(float v)
	{
		x -= v; y -= v; z -= v; return *this;
	}

	inline Vec3& operator*=(float v)
	{
		x *= v; y *= v; z *= v; return *this;
	}

	inline Vec3& operator/=(float v)
	{
		x /= v; y /= v; z /= v; return *this;
	}

	inline Vec3 operator+(const Vec3& v) const
	{
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	inline Vec3 operator-(const Vec3& v) const
	{
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	inline Vec3 operator*(const Vec3& v) const
	{
		return Vec3(x * v.x, y * v.y, z * v.z);
	}

	inline Vec3 operator/(const Vec3& v) const
	{
		return Vec3(x / v.x, y / v.y, z / v.z);
	}

	inline Vec3 operator+(float v) const
	{
		return Vec3(x + v, y + v, z + v);
	}

	inline Vec3 operator-(float v) const
	{
		return Vec3(x - v, y - v, z - v);
	}

	inline Vec3 operator*(float v) const
	{
		return Vec3(x * v, y * v, z * v);
	}

	inline Vec3 operator/(float v) const
	{
		return Vec3(x / v, y / v, z / v);
	}

	inline void Set(float X = 0.f, float Y = 0.f, float Z = 0.f)
	{
		x = X; y = Y; z = Z;
	}

	inline Vec3 To2D() const
	{
		return { x, y };
	}

	inline Vec3 Min(const Vec3& v) const
	{
		return Vec3(std::min<float>(x, v.x), std::min<float>(y, v.y), std::min<float>(z, v.z));
	}

	inline Vec3 Max(const Vec3& v) const
	{
		return Vec3(std::max<float>(x, v.x), std::max<float>(y, v.y), std::max<float>(z, v.z));
	}

	inline Vec3 Clamp(const Vec3& v1, const Vec3& v2) const
	{
		return Max(v1).Min(v2);
	}

	inline float Min() const
	{
		return std::min<float>(x, std::min<float>(y, z));
	}

	inline float Max() const
	{
		return std::max<float>(x, std::max<float>(y, z));
	}

	inline Vec3 Min(float v) const
	{
		return Vec3(std::min<float>(x, v), std::min<float>(y, v), std::min<float>(z, v));
	}

	inline Vec3 Max(float v) const
	{
		return Vec3(std::max<float>(x, v), std::max<float>(y, v), std::max<float>(z, v));
	}

	inline Vec3 Clamp(float v1, float v2) const
	{
		return Max(v1).Min(v2);
	}

	inline Vec3 Lerp(const Vec3& v, float t) const
	{
		return { x + (v.x - x) * t, y + (v.y - y) * t, z + (v.z - z) * t };
	}

	inline Vec3 Lerp(float v, float t) const
	{
		return { x + (v - x) * t, y + (v - y) * t, z + (v - z) * t };
	}

	inline Vec3 DeltaAngle(const Vec3& v) const
	{
		auto deltaAngle = [](const float flAngleA, const float flAngleB)
			{
				float flOut = fmodf((flAngleA - flAngleB) + 180.f, 360.f);
				return flOut += flOut < 0 ? 180.f : -180.f;
			};

		return { deltaAngle(x, v.x), deltaAngle(y, v.y), deltaAngle(z, v.z) };
	}

	inline Vec3 DeltaAngle(float v) const
	{
		auto deltaAngle = [](const float flAngleA, const float flAngleB)
			{
				float flOut = fmodf((flAngleA - flAngleB) + 180.f, 360.f);
				return flOut += flOut < 0 ? 180.f : -180.f;
			};

		return { deltaAngle(x, v), deltaAngle(y, v), deltaAngle(z, v) };
	}

	inline Vec3 LerpAngle(const Vec3& v, float t) const
	{
		auto shortDist = [](const float flAngleA, const float flAngleB)
			{
				const float flDelta = fmodf((flAngleA - flAngleB), 360.f);
				return fmodf(2 * flDelta, 360.f) - flDelta;
			};
		return { x - shortDist(x, v.x) * t, y - shortDist(y, v.y) * t, z - shortDist(z, v.z) * t };
	}

	inline Vec3 LerpAngle(float v, float t) const
	{
		auto shortDist = [](const float flAngleA, const float flAngleB)
			{
				const float flDelta = fmodf((flAngleA - flAngleB), 360.f);
				return fmodf(2 * flDelta, 360.f) - flDelta;
			};
		return { x - shortDist(x, v) * t, y - shortDist(y, v) * t, z - shortDist(z, v) * t };
	}

	inline float Length(void) const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	inline float LengthSqr(void) const
	{
		return (x * x + y * y + z * z);
	}

	inline float Normalize()
	{
		float flLength = Length();
		float flLengthNormal = 1.f / (FLT_EPSILON + flLength);

		x *= flLengthNormal;
		y *= flLengthNormal;
		z *= flLengthNormal;

		return flLength;
	}

	inline float Normalize2D()
	{
		float flLength = Length2D();
		float flLengthNormal = 1.f / (FLT_EPSILON + flLength);

		x *= flLengthNormal;
		y *= flLengthNormal;
		z = 0;

		return flLength;
	}

	inline Vec3 Normalized()
	{
		float flLengthNormal = 1.f / (FLT_EPSILON + Length());
		return Vec3(x * flLengthNormal, y * flLengthNormal, z * flLengthNormal);
	}

	inline Vec3 Normalized2D()
	{
		float flLengthNormal = 1.f / (FLT_EPSILON + Length2D());
		return Vec3(x * flLengthNormal, y * flLengthNormal);
	}

	inline Vec3 Get2D()
	{
		return Vec3(x, y, 0);
	}

	inline float Length2D(void) const
	{
		return sqrtf(x * x + y * y);
	}

	inline float Length2DSqr(void) const
	{
		return (x * x + y * y);
	}

	inline float DistTo(const Vec3& v) const
	{
		return (*this - v).Length();
	}

	inline float DistTo2D(const Vec3& v) const
	{
		return (*this - v).Length2D();
	}

	inline float DistToSqr(const Vec3& v) const
	{
		return (*this - v).LengthSqr();
	}

	inline float DistTo2DSqr(const Vec3& v) const
	{
		return (*this - v).Length2DSqr();
	}

	inline float Dot(const Vec3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline Vec3 Cross(const Vec3& v) const
	{
		return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	inline bool IsZero(void) const
	{
		return fabsf(x) < 0.001f &&
			   fabsf(y) < 0.001f &&
			   fabsf(z) < 0.001f;
	}

	inline Vec3 ToAngle() const noexcept
	{
		return { RAD2DEG(atan2(-z, hypot(x, y))),
				 RAD2DEG(atan2(y, x)),
				 0.f };
	}
	inline Vec3 FromAngle() const noexcept
	{
		return { cos(DEG2RAD(x)) * cos(DEG2RAD(y)),
				 cos(DEG2RAD(x))* sin(DEG2RAD(y)),
				 -sin(DEG2RAD(x)) };
	}
};
using Vector = Vec3;
using QAngle = Vec3;
using RadianEuler = Vec3;

class Vector4D
{
public:
	float x, y, z, w;
};
using Quaternion = Vector4D;

using matrix3x4 = float[3][4];
class VMatrix
{
public:
	float m[4][4];

	inline const float* operator[](int i) const
	{
		return m[i];
	}

private:
	static inline void SinCos(float flRadians, float* pSin, float* pCos)
	{
		*pSin = std::sin(flRadians);
		*pCos = std::cos(flRadians);
	}

	static inline void Vector3DMultiplyPosition(const VMatrix& src1, const Vector src2, Vector& dst)
	{
		dst[0] = src1[0][0] * src2.x + src1[0][1] * src2.y + src1[0][2] * src2.z + src1[0][3];
		dst[1] = src1[1][0] * src2.x + src1[1][1] * src2.y + src1[1][2] * src2.z + src1[1][3];
		dst[2] = src1[2][0] * src2.x + src1[2][1] * src2.y + src1[2][2] * src2.z + src1[2][3];
	}

	static inline void SetupMatrixAnglesInternal(float m2[4][4], const QAngle& vAngles)
	{
		float sr, sp, sy, cr, cp, cy;

		SinCos(DEG2RAD(vAngles[1]), &sy, &cy);
		SinCos(DEG2RAD(vAngles[0]), &sp, &cp);
		SinCos(DEG2RAD(vAngles[2]), &sr, &cr);

		// matrix = (YAW * PITCH) * ROLL
		m2[0][0] = cp * cy;
		m2[1][0] = cp * sy;
		m2[2][0] = -sp;
		m2[0][1] = sr * sp * cy + cr * -sy;
		m2[1][1] = sr * sp * sy + cr * cy;
		m2[2][1] = sr * cp;
		m2[0][2] = (cr * sp * cy + -sr * -sy);
		m2[1][2] = (cr * sp * sy + -sr * cy);
		m2[2][2] = cr * cp;
		m2[0][3] = 0.f;
		m2[1][3] = 0.f;
		m2[2][3] = 0.f;
	}

public:
	inline const matrix3x4& As3x4() const
	{
		return *((const matrix3x4*)this);
	}

	inline void SetupMatrixOrgAngles(const Vector& origin, const QAngle& vAngles)
	{
		SetupMatrixAnglesInternal(m, vAngles);

		// Add translation
		m[0][3] = origin.x;
		m[1][3] = origin.y;
		m[2][3] = origin.z;
		m[3][0] = 0.0f;
		m[3][1] = 0.0f;
		m[3][2] = 0.0f;
		m[3][3] = 1.0f;
	}

	inline Vector VMul4x3(const Vector& vVec) const
	{
		Vector vResult;
		Vector3DMultiplyPosition(*this, vVec, vResult);
		return vResult;
	}

	inline Vector VMul4x3Transpose(const Vector& vVec) const
	{
		Vector tmp = vVec;
		tmp.x -= m[0][3];
		tmp.y -= m[1][3];
		tmp.z -= m[2][3];

		return Vector(
			m[0][0] * tmp.x + m[1][0] * tmp.y + m[2][0] * tmp.z,
			m[0][1] * tmp.x + m[1][1] * tmp.y + m[2][1] * tmp.z,
			m[0][2] * tmp.x + m[1][2] * tmp.y + m[2][2] * tmp.z
		);
	}

	inline Vector VMul3x3(const Vector& vVec) const
	{
		return Vector(
			m[0][0] * vVec.x + m[0][1] * vVec.y + m[0][2] * vVec.z,
			m[1][0] * vVec.x + m[1][1] * vVec.y + m[1][2] * vVec.z,
			m[2][0] * vVec.x + m[2][1] * vVec.y + m[2][2] * vVec.z
		);
	}

	inline Vector VMul3x3Transpose(const Vector& vVec) const
	{
		return Vector(
			m[0][0] * vVec.x + m[1][0] * vVec.y + m[2][0] * vVec.z,
			m[0][1] * vVec.x + m[1][1] * vVec.y + m[2][1] * vVec.z,
			m[0][2] * vVec.x + m[1][2] * vVec.y + m[2][2] * vVec.z
		);
	}

	inline Vector LocalToWorld(const Vector& vVec) const
	{
		return VMul4x3(vVec);
	}

	inline Vector WorldToLocal(const Vector& vVec) const
	{
		return VMul4x3Transpose(vVec);
	}

	inline Vector LocalToWorldRotation(const Vector& vVec) const
	{
		return VMul3x3(vVec);
	}

	inline Vector WorldToLocalRotation(const Vector& vVec) const
	{
		return VMul3x3Transpose(vVec);
	}
};

struct IntRange_t
{
	int Min = 0, Max = 0;

	inline bool operator==(const IntRange_t& t) const
	{
		return Min == t.Min && Max == t.Max;
	}

	inline bool operator!=(const IntRange_t& t) const
	{
		return Min != t.Min || Max != t.Max;
	}
};

struct FloatRange_t
{
	float Min = 0, Max = 0;

	inline bool operator==(const FloatRange_t& t) const
	{
		return Min == t.Min && Max == t.Max;
	}

	inline bool operator!=(const FloatRange_t& t) const
	{
		return Min != t.Min || Max != t.Max;
	}
};

namespace LerpEnum {
	enum LerpEnum {
		All, NoAlpha, Alpha
	};
};
using byte = unsigned char;
struct Color_t
{
	byte r = 255, g = 255, b = 255, a = 255;

	inline void SetRGB(float flR = 255.f, float flG = 255.f, float flB = 255.f, float flA = 255.f)
	{
		r = byte(std::clamp(flR, 0.f, 255.f));
		g = byte(std::clamp(flG, 0.f, 255.f));
		b = byte(std::clamp(flB, 0.f, 255.f));
		a = byte(std::clamp(flA, 0.f, 255.f));
	}

	inline void SetHSV(float flH, float flS = 100.f, float flV = 100.f, float flA = 255.f)
	{
		float flR, flG, flB;

		flH = std::clamp(flH, 0.f, 360.f);
		flS = std::clamp(flS, 0.f, 100.f);
		flV = std::clamp(flV, 0.f, 100.f);

		flS /= 100;
		flV /= 100;
		if (!flS)
			flR = flG = flB = flV;
		else
		{
			int i = int(floor(flH /= 60));
			float flF = flH - i;
			float flP = flV * (1 - flS);
			float flQ = flV * (1 - flF * flS);
			float flT = flV * (1 - (1 - flF) * flS);

			switch (i)
			{
			case 0: flR = flV; flG = flT; flB = flP; break;
			case 1: flR = flQ; flG = flV; flB = flP; break;
			case 2: flR = flP; flG = flV; flB = flT; break;
			case 3: flR = flP; flG = flQ; flB = flV; break;
			case 4: flR = flT; flG = flP; flB = flV; break;
			default: flR = flV; flG = flP; flB = flQ; break;
			}
		}

		r = byte(std::clamp(flR * 255, 0.f, 255.f));
		g = byte(std::clamp(flG * 255, 0.f, 255.f));
		b = byte(std::clamp(flB * 255, 0.f, 255.f));
		a = byte(std::clamp(flA, 0.f, 255.f));
	}

	inline void GetHSV(float& flH, float& flS, float& flV)
	{
		float flR = r / 255.f;
		float flG = g / 255.f;
		float flB = b / 255.f;

		float flK = 0.f;
		if (flG < flB)
		{
			float flTemp = flG;
			flG = flB;
			flB = flTemp;
			flK = -1.f;
		}
		if (flR < flG)
		{
			float flTemp = flR;
			flR = flG;
			flG = flTemp;
			flK = -2.f / 6.f - flK;
		}

		float flChroma = flR - (flG < flB ? flG : flB);
		flH = fabsf(flK + (flG - flB) / (6.f * flChroma + FLT_EPSILON)) * 360;
		flS = flChroma / (flR + FLT_EPSILON) * 100;
		flV = flR * 100;
	}

	inline Color_t HueShift(float flShift)
	{
		float flH, flS, flV; GetHSV(flH, flS, flV);
		Color_t tOut; tOut.SetHSV(fmodf(flH + flShift, 360.f), flS, flV, a);
		return tOut;
	}

	inline bool operator==(Color_t other) const
	{
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}

	inline bool operator!=(Color_t other) const
	{
		return r != other.r || g != other.g || b != other.b || a != other.a;
	}

	inline std::string ToHex() const
	{
		return std::format("\x7{:02x}{:02x}{:02x}", r, g, b);
	}

	inline std::string ToHexA() const
	{
		return std::format("\x8{:02x}{:02x}{:02x}{:02x}", r, g, b, a);
	}

	inline Color_t Lerp(Color_t to, float t, LerpEnum::LerpEnum eLerp = LerpEnum::All) const
	{
		//a + (b - a) * t
		switch (eLerp)
		{
		default:
			return {
				byte(r + (to.r - r) * t),
				byte(g + (to.g - g) * t),
				byte(b + (to.b - b) * t),
				byte(a + (to.a - a) * t)
			};
		case LerpEnum::NoAlpha:
			return {
				byte(r + (to.r - r) * t),
				byte(g + (to.g - g) * t),
				byte(b + (to.b - b) * t),
				byte(a)
			};
		case LerpEnum::Alpha:
			return {
				r,
				g,
				b,
				byte(a + (to.a - a) * t)
			};
		}
	}

	inline Color_t Alpha(byte to) const
	{
		return { r, g, b, to };
	}

	inline int Brightness() const
	{
		return r + g + b;
	}
};

struct Gradient_t
{
	Color_t StartColor = {};
	Color_t EndColor = {};

	inline bool operator==(Gradient_t other) const
	{
		return StartColor == other.StartColor && EndColor == other.EndColor;
	}

	inline bool operator!=(Gradient_t other) const
	{
		return StartColor != other.StartColor || EndColor != other.EndColor;
	}
};

struct Chams_t
{
	std::vector<std::pair<std::string, Color_t>> Visible = { { "Original", Color_t() } };
	std::vector<std::pair<std::string, Color_t>> Occluded = {};

	inline bool operator==(const Chams_t& t) const
	{
		return Visible == t.Visible && Occluded == t.Occluded;
	}

	inline bool operator!=(const Chams_t& t) const
	{
		return Visible != t.Visible || Occluded != t.Occluded;
	}

	inline bool operator()(bool bVisibleOnly = false) const
	{
		return bVisibleOnly ? !Visible.empty() : Visible != std::vector<std::pair<std::string, Color_t>>{ { "Original", Color_t() } } || !Occluded.empty();
	}
};

struct Glow_t
{
	int		Stencil = 0;
	float	Blur = 0;

	inline bool operator==(const Glow_t& t) const
	{
		return Stencil == t.Stencil && Blur == t.Blur;
	}

	inline bool operator!=(const Glow_t& t) const
	{
		return Stencil != t.Stencil || Blur != t.Blur;
	}

	inline bool operator()() const
	{
		return Stencil || Blur;
	}
};

struct DragBox_t
{
	int x = 150;
	int y = 100;

	inline bool operator==(const DragBox_t& t) const
	{
		return x == t.x && y == t.y;
	}

	inline bool operator!=(const DragBox_t& t) const
	{
		return x != t.x || y != t.y;
	}
};

struct WindowBox_t
{
	int x = 200;
	int y = 100;
	int w = 200;
	int h = 200;

	inline bool operator==(const WindowBox_t& t) const
	{
		return x == t.x && y == t.y && w == t.w && h == t.h;
	}

	inline bool operator!=(const WindowBox_t& t) const
	{
		return x != t.x || y != t.y || w != t.w || h != t.h;
	}
};