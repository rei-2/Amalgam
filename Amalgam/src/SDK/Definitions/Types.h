#pragma once
#include <string>
#include <vector>
#include <deque>
#include <format>

#pragma warning (disable : 26495)

class Vec2
{
public:
	float x = 0.f, y = 0.f;

public:
	Vec2(void)
	{
		x = y = 0.f;
	}

	Vec2(float X, float Y)
	{
		x = X; y = Y;
	}

	Vec2(float* v)
	{
		x = v[0]; y = v[1];
	}

	Vec2(const float* v)
	{
		x = v[0]; y = v[1];
	}

	Vec2(const Vec2& v)
	{
		x = v.x; y = v.y;
	}

	Vec2& operator=(const Vec2& v)
	{
		x = v.x; y = v.y; return *this;
	}

	float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	Vec2& operator+=(const Vec2& v)
	{
		x += v.x; y += v.y; return *this;
	}

	Vec2& operator-=(const Vec2& v)
	{
		x -= v.x; y -= v.y; return *this;
	}

	Vec2& operator*=(const Vec2& v)
	{
		x *= v.x; y *= v.y; return *this;
	}

	Vec2& operator/=(const Vec2& v)
	{
		x /= v.x; y /= v.y; return *this;
	}

	Vec2& operator+=(float v)
	{
		x += v; y += v; return *this;
	}

	Vec2& operator-=(float v)
	{
		x -= v; y -= v; return *this;
	}

	Vec2& operator*=(float v)
	{
		x *= v; y *= v; return *this;
	}

	Vec2& operator/=(float v)
	{
		x /= v; y /= v; return *this;
	}

	Vec2 operator+(const Vec2& v) const
	{
		return Vec2(x + v.x, y + v.y);
	}

	Vec2 operator-(const Vec2& v) const
	{
		return Vec2(x - v.x, y - v.y);
	}

	Vec2 operator*(const Vec2& v) const
	{
		return Vec2(x * v.x, y * v.y);
	}

	Vec2 operator/(const Vec2& v) const
	{
		return Vec2(x / v.x, y / v.y);
	}

	Vec2 operator+(float v) const
	{
		return Vec2(x + v, y + v);
	}

	Vec2 operator-(float v) const
	{
		return Vec2(x - v, y - v);
	}

	Vec2 operator*(float v) const
	{
		return Vec2(x * v, y * v);
	}

	Vec2 operator/(float v) const
	{
		return Vec2(x / v, y / v);
	}

	void Set(float X = 0.f, float Y = 0.f)
	{
		x = X; y = Y;
	}

	float Length(void) const
	{
		return sqrtf(x * x + y * y);
	}

	float LengthSqr(void) const
	{
		return (x * x + y * y);
	}

	float DistTo(const Vec2& v) const
	{
		return (*this - v).Length();
	}

	float DistToSqr(const Vec2& v) const
	{
		return (*this - v).LengthSqr();
	}

	float Dot(const Vec2& v) const
	{
		return x * v.x + y * v.y;
	}

	bool IsZero(void) const
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
	Vec3(void)
	{
		x = y = z = 0.f;
	}

	void Zero()
	{
		x = y = z = 0.f;
	}

	Vec3(float X, float Y, float Z)
	{
		x = X; y = Y; z = Z;
	}

	Vec3(float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vec3(const float* v)
	{
		x = v[0]; y = v[1]; z = v[2];
	}

	Vec3(const Vec3& v)
	{
		x = v.x; y = v.y; z = v.z;
	}

	Vec3(const Vec2& v)
	{
		x = v.x; y = v.y; z = 0.f;
	}

	Vec3& operator=(const Vec3& v)
	{
		x = v.x; y = v.y; z = v.z; return *this;
	}

	Vec3& operator=(const Vec2& v)
	{
		x = v.x; y = v.y; z = 0.f; return *this;
	}

	float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	float operator[](int i) const
	{
		return ((float*)this)[i];
	}

	Vec3& operator+=(const Vec3& v)
	{
		x += v.x; y += v.y; z += v.z; return *this;
	}

	Vec3& operator-=(const Vec3& v)
	{
		x -= v.x; y -= v.y; z -= v.z; return *this;
	}

	Vec3& operator*=(const Vec3& v)
	{
		x *= v.x; y *= v.y; z *= v.z; return *this;
	}

	Vec3& operator/=(const Vec3& v)
	{
		x /= v.x; y /= v.y; z /= v.z; return *this;
	}

	Vec3& operator+=(float v)
	{
		x += v; y += v; z += v; return *this;
	}

	Vec3& operator-=(float v)
	{
		x -= v; y -= v; z -= v; return *this;
	}

	Vec3& operator*=(float v)
	{
		x *= v; y *= v; z *= v; return *this;
	}

	Vec3& operator/=(float v)
	{
		x /= v; y /= v; z /= v; return *this;
	}

	Vec3 operator+(const Vec3& v) const
	{
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	Vec3 operator-(const Vec3& v) const
	{
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	Vec3 operator*(const Vec3& v) const
	{
		return Vec3(x * v.x, y * v.y, z * v.z);
	}

	Vec3 operator/(const Vec3& v) const
	{
		return Vec3(x / v.x, y / v.y, z / v.z);
	}

	Vec3 operator+(float v) const
	{
		return Vec3(x + v, y + v, z + v);
	}

	Vec3 operator-(float v) const
	{
		return Vec3(x - v, y - v, z - v);
	}

	Vec3 operator*(float v) const
	{
		return Vec3(x * v, y * v, z * v);
	}

	Vec3 operator/(float v) const
	{
		return Vec3(x / v, y / v, z / v);
	}

	bool operator==(const Vec3& v) const
	{
		return (x == v.x && y == v.y && z == v.z);
	}

	bool operator!=(const Vec3& v) const
	{
		return (x != v.x || y != v.y || z != v.z);
	}

	void Set(float X = 0.f, float Y = 0.f, float Z = 0.f)
	{
		x = X; y = Y; z = Z;
	}

	float Length(void) const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float LengthSqr(void) const
	{
		return (x * x + y * y + z * z);
	}

	float Normalize()
	{
		float fl_Length = Length();
		float fl_Length_normal = 1.f / (FLT_EPSILON + fl_Length);

		x = x * fl_Length_normal;
		y = y * fl_Length_normal;
		z = z * fl_Length_normal;

		return fl_Length;
	}

	float NormalizeInPlace()
	{
		return Normalize();
	}

	float Length2D(void) const
	{
		return sqrtf(x * x + y * y);
	}

	float Length2DSqr(void) const
	{
		return (x * x + y * y);
	}

	float DistTo(const Vec3& v) const
	{
		return (*this - v).Length();
	}

	float DistToSqr(const Vec3& v) const
	{
		return (*this - v).LengthSqr();
	}

	float Dot(const Vec3& v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vec3 Cross(const Vec3& v) const
	{
		return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}

	bool IsZero(void) const
	{
		return fabsf(x) < 0.001f &&
			   fabsf(y) < 0.001f &&
			   fabsf(z) < 0.001f;
	}

	Vec3 Scale(float fl)
	{
		return Vec3(x * fl, y * fl, z * fl);
	}

	void Init(float ix, float iy, float iz)
	{
		x = ix; y = iy; z = iz;
	}

	Vec3 toAngle() const noexcept
	{
		return { (atan2(-z, hypot(x, y))) * (180.f / 3.14159265358979323846f),
				 (atan2(y, x)) * (180.f / 3.14159265358979323846f),
				 0.f };
	}
	Vec3 fromAngle() const noexcept
	{
		return { cos(x * (3.14159265358979323846f / 180.f)) * cos(y * (3.14159265358979323846f / 180.f)),
				 cos(x * (3.14159265358979323846f / 180.f)) * sin(y * (3.14159265358979323846f / 180.f)),
				 -sin(x * (3.14159265358979323846f / 180.f)) };
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
	Vector m[4][4];

public:
	inline const matrix3x4& As3x4() const
	{
		return *((const matrix3x4*)this);
	}
};

struct IntRange_t
{
	int Min = 0, Max = 0;

	bool operator==(IntRange_t other) const
	{
		return Min == other.Min && Max == other.Max;
	}

	bool operator!=(IntRange_t other) const
	{
		return Min != other.Min || Max != other.Max;
	}
};

struct FloatRange_t
{
	float Min = 0, Max = 0;

	bool operator==(FloatRange_t other) const
	{
		return Min == other.Min && Max == other.Max;
	}

	bool operator!=(FloatRange_t other) const
	{
		return Min != other.Min || Max != other.Max;
	}
};

using byte = unsigned char;
struct Color_t
{
	byte r = 0, g = 0, b = 0, a = 0;

	bool operator==(Color_t other) const
	{
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}

	bool operator!=(Color_t other) const
	{
		return r != other.r || g != other.g || b != other.b || a != other.a;
	}

	std::string ToHex() const
	{
		return std::format("\x7{:02X}{:02X}{:02X}", r, g, b);
	}

	std::string ToHexA() const
	{
		return std::format("\x8{:02X}{:02X}{:02X}{:02X}", r, g, b, a);
	}

	Color_t Lerp(Color_t to, float t) const
	{
		//a + (b - a) * t
		return {
			byte(r + (to.r - r) * t),
			byte(g + (to.g - g) * t),
			byte(b + (to.b - b) * t),
			byte(a + (to.a - a) * t),
		};
	}
};

struct Gradient_t
{
	Color_t StartColor = { 0, 0, 0, 255 };
	Color_t EndColor = { 0, 0, 0, 255 };

	bool operator==(Gradient_t other) const
	{
		return StartColor == other.StartColor && EndColor == other.EndColor;
	}

	bool operator!=(Gradient_t other) const
	{
		return StartColor != other.StartColor || EndColor != other.EndColor;
	}
};

struct Chams_t
{
	std::vector<std::string>	VisibleMaterial = { "Original" };
	std::vector<std::string>	OccludedMaterial = {};
	Color_t						VisibleColor = { 255, 255, 255, 255 };
	Color_t						OccludedColor = { 255, 255, 255, 255 };
};

struct Glow_t
{
	bool	Stencil = false;
	bool	Blur = false;
	int		StencilScale = 1;
	int		BlurScale = 1;

	bool operator==(const Glow_t& other) const
	{
		return Stencil == other.Stencil && Blur == other.Blur && StencilScale == other.StencilScale && BlurScale == other.BlurScale;
	}
};

struct DragBox_t
{
	int x = 100;
	int y = 100;

	bool operator==(DragBox_t other) const
	{
		return x == other.x && y == other.y;
	}

	bool operator!=(DragBox_t other) const
	{
		return x != other.x || y != other.y;
	}
};

struct WindowBox_t
{
	int x = 100;
	int y = 100;
	int w = 200;
	int h = 150;

	bool operator==(WindowBox_t other) const
	{
		return x == other.x && y == other.y && w == other.w && h == other.h;
	}

	bool operator!=(WindowBox_t other) const
	{
		return x != other.x || y != other.y || w != other.w || h != other.h;
	}
};