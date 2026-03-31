#pragma once
#include <cmath>
#include <algorithm>
#include <vector>

template <class T> inline int sign(T t)
{
	return (t > T(0)) - (t < T(0));
}

inline float fnmodf(float flX, float flY)
{	// silly fix for negative values
	return fmodf(flX, flY) + (flX < 0 ? flY : 0);
}

namespace Math
{
	template <class T = float>
	constexpr T PI_V = static_cast<T>(3.141592653589793);
	constexpr float PI = PI_V<>;

	template <class T = float>
	constexpr T RAD_V = static_cast<T>(57.29577951308232);
	constexpr float RAD = RAD_V<>;

	template <class T = float>
	constexpr T E_V = static_cast<T>(2.718281828459045);
	constexpr float E = E_V<>;

	template <class T = float>
	inline T Deg2Rad(T v)
	{
		return v * PI_V<T> / T(180);
	}

	template <class T = float>
	inline T Rad2Deg(T v)
	{
		return v * T(180) / PI_V<T>;
	}

	inline void SinCos(float flRadians, float& flSin, float& flCos)
	{
		flSin = std::sin(flRadians);
		flCos = std::cos(flRadians);
	}

	inline float DeltaAngle(float a, float b, float r = 360.f)
	{
		float flOut = fmodf(a - b + r / 2, r);
		return flOut += flOut < 0 ? r / 2 : -r / 2;
	}

	inline float ShortDist(float a, float b, float r = 360.f)
	{
		const float flDelta = fmodf(a - b, r);
		return fmodf(2 * flDelta, r) - flDelta;
	}

	inline float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
	}

	inline float LerpAngle(float a, float b, float t, float r = 360.f)
	{
		return a - Math::ShortDist(a, b, r) * t;
	}

	inline float NormalizeAngle(float flAngle, float flRange = 360.f)
	{
		return std::isfinite(flAngle) ? std::remainder(flAngle, flRange) : 0.f;
	}

	inline float NormalizeRad(float flAngle, float flRange = PI * 2)
	{
		return std::isfinite(flAngle) ? std::remainder(flAngle, flRange) : 0.f;
	}

	inline float ClampNormalizeAngle(float flAngle, float flRange = 180.f)
	{
		return std::isfinite(flAngle) ? (flAngle > flRange ? -flRange : flAngle < -flRange ? flRange : flAngle) : 0.f;
	}

	inline float ClampNormalizeRad(float flAngle, float flRange = PI)
	{
		return std::isfinite(flAngle) ? (flAngle > flRange ? -flRange : flAngle < -flRange ? flRange : flAngle) : 0.f;
	}

	inline float SimpleSpline(float val)
	{
		float flSquared = powf(val, 2);
		return 3 * flSquared - 2 * flSquared * val;
	}

	inline float RemapVal(float flVal, float a, float b, float c, float d, bool bClamp = true)
	{
		if (a == b)
			return flVal >= b ? d : c;

		float t = (flVal - a) / (b - a);
		if (bClamp)
			t = std::clamp(t, 0.f, 1.f);

		return Lerp(c, d, t);
	}

	inline float SimpleSplineRemapVal(float flVal, float a, float b, float c, float d, bool bClamp = true)
	{
		if (a == b)
			return flVal >= b ? d : c;

		float t = (flVal - a) / (b - a);
		if (bClamp)
			t = std::clamp(t, 0.f, 1.f);

		return Lerp(c, d, SimpleSpline(t));
	}

	inline std::vector<float> SolveQuadratic(float a, float b, float c)
	{
		float flRoot = powf(b, 2.f) - 4 * a * c;
		if (flRoot < 0)
			return {};

		a *= 2;
		b = -b;
		return { (b + sqrt(flRoot)) / a, (b - sqrt(flRoot)) / a };
	}

	inline float SolveCubic(float b, float c, float d)
	{
		float p = c - powf(b, 2) / 3;
		float q = 2 * powf(b, 3) / 27 - b * c / 3 + d;

		if (p == 0.f)
			return powf(q, 1.f / 3);
		if (q == 0.f)
			return 0.f;

		float t = sqrt(fabs(p) / 3);
		float g = q / (2.f / 3) / (p * t);
		if (p > 0.f)
			return -2 * t * sinh(asinh(g) / 3) - b / 3;

		if (4 * powf(p, 3) + 27 * powf(q, 2) < 0.f)
			return 2 * t * cos(acos(g) / 3) - b / 3;
		if (q > 0.f)
			return -2 * t * cosh(acosh(-g) / 3) - b / 3;
		return 2 * t * cosh(acosh(g) / 3) - b / 3;
	}

	inline std::vector<float> SolveQuartic(float a, float b, float c, float d, float e)
	{
		std::vector<float> vRoots = {};

		b /= a, c /= a, d /= a, e /= a;
		float p = c - powf(b, 2) / (8.f / 3);
		float q = powf(b, 3) / 8 - b * c / 2 + d;
		float m = SolveCubic(
			p,
			powf(p, 2) / 4 + powf(b, 4) / (256.f / 3) - e + b * d / 4 - powf(b, 2) * c / 16,
			-powf(q, 2) / 8
		);
		if (m < 0.f)
			return vRoots;

		float flSqrt2m = sqrt(2 * m);
		if (q == 0.f)
		{
			if (-m - p > 0.f)
			{
				float flDelta = sqrt(2 * (-m - p));
				vRoots.push_back(-b / 4 + (flSqrt2m - flDelta) / 2);
				vRoots.push_back(-b / 4 - (flSqrt2m - flDelta) / 2);
				vRoots.push_back(-b / 4 + (flSqrt2m + flDelta) / 2);
				vRoots.push_back(-b / 4 - (flSqrt2m + flDelta) / 2);
			}
			if (-m - p == 0.f)
			{
				vRoots.push_back(-b / 4 - flSqrt2m / 2);
				vRoots.push_back(-b / 4 + flSqrt2m / 2);
			}
		}
		else
		{
			if (-m - p + q / flSqrt2m >= 0.f)
			{
				float flDelta = sqrt(2 * (-m - p + q / flSqrt2m));
				vRoots.push_back((-flSqrt2m + flDelta) / 2 - b / 4);
				vRoots.push_back((-flSqrt2m - flDelta) / 2 - b / 4);
			}
			if (-m - p - q / flSqrt2m >= 0.f)
			{
				float flDelta = sqrt(2 * (-m - p - q / flSqrt2m));
				vRoots.push_back((flSqrt2m + flDelta) / 2 - b / 4);
				vRoots.push_back((flSqrt2m - flDelta) / 2 - b / 4);
			}
		}
		return vRoots;
	}
}