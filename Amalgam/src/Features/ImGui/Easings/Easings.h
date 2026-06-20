#pragma once
#include "../../../Utils/Math/BaseMath.h"

namespace Ease
{
	template <typename T> inline T OutSine(T x)
	{
		return sin((x * Math::PI_V<T>) / T(2));
	}

	template <typename T> inline T InSine(T x)
	{
		return T(1) - cos((x * Math::PI_V<T>) / T(2));
	}

	template <typename T> inline T InOutSine(T x)
	{
		return -(cos(x * Math::PI_V<T>) - T(1)) / T(2);
	}

	template <typename T> inline T OutQuad(T x)
	{
		return T(1) - pow(T(1) - x, T(2));
	}

	template <typename T> inline T InQuad(T x)
	{
		return pow(x, T(2));
	}

	template <typename T> inline T InOutQuad(T x)
	{
		return x < T(0.5) ? T(2) * pow(x, T(2)) : T(1) - pow(-T(2) * x + T(2), T(2)) / T(2);
	}

	template <typename T> inline T OutCubic(T x)
	{
		return T(1) - pow(T(1) - x, T(3));
	}

	template <typename T> inline T InCubic(T x)
	{
		return pow(x, T(3));
	}

	template <typename T> inline T InOutCubic(T x)
	{
		return x < T(0.5) ? T(4) * pow(x, T(3)) : T(1) - pow(-T(2) * x + T(2), T(3)) / T(2);
	}

	template <typename T> inline T OutQuart(T x)
	{
		return T(1) - pow(T(1) - x, T(4));
	}

	template <typename T> inline T InQuart(T x)
	{
		return pow(x, T(4));
	}

	template <typename T> inline T InOutQuart(T x)
	{
		return x < T(0.5) ? T(8) * pow(x, T(4)) : T(1) - pow(-T(2) * x + T(2), T(4)) / T(2);
	}

	template <typename T> inline T OutQuint(T x)
	{
		return T(1) - pow(T(1) - x, T(5));
	}

	template <typename T> inline T InQuint(T x)
	{
		return pow(x, T(5));
	}

	template <typename T> inline T InOutQuint(T x)
	{
		return x < T(0.5) ? T(16) * pow(x, T(5)) : T(1) - pow(-T(2) * x + T(2), T(5)) / T(2);
	}

	template <typename T> inline T OutExpo(T x)
	{
		return x == T(1) ? T(1) : T(1) - pow(T(2), -T(10) * x);
	}

	template <typename T> inline T InExpo(T x)
	{
		return x == T(0) ? T(0) : pow(T(2), T(10) * x - T(10));
	}

	template <typename T> inline T InOutExpo(T x)
	{
		return x == T(0) ? T(0)
			: x == T(1) ? T(1)
			: x < T(0.5) ? pow(T(2), T(20) * x - T(10)) / T(2)
			: (T(2) - pow(T(2), -T(20) * x + T(10))) / T(2);
	}

	template <typename T> inline T OutCirc(T x)
	{
		return sqrt(T(1) - pow(x - T(1), T(2)));
	}

	template <typename T> inline T InCirc(T x)
	{
		return T(1) - sqrt(T(1) - pow(x, T(2)));
	}

	template <typename T> inline T InOutCirc(T x)
	{
		return x < T(0.5)
			? (T(1) - sqrt(T(1) - pow(T(2) * x, T(2)))) / T(2)
			: (sqrt(T(1) - pow(-T(2) * x + T(2), T(2))) + T(1)) / T(2);
	}

	template <typename T> inline T OutBack(T x)
	{
		constexpr T c1 = T(1.70158);
		constexpr T c3 = c1 + T(1);

		return T(1) + c3 * pow(x - T(1), T(3)) + c1 * pow(x - T(1), T(2));
	}

	template <typename T> inline T InBack(T x)
	{
		constexpr T c1 = T(1.70158);
		constexpr T c3 = c1 + T(1);

		return c3 * pow(x, T(3)) - c1 * pow(x, T(2));
	}

	template <typename T> inline T InOutBack(T x)
	{
		constexpr T c1 = T(1.70158);
		constexpr T c2 = c1 * T(1.525);

		return x < T(0.5)
		  ? (pow(T(2) * x, T(2)) * ((c2 + T(1)) * T(2) * x - c2)) / T(2)
		  : (pow(T(2) * x - T(2), T(2)) * ((c2 + T(1)) * (x * T(2) - T(2)) + c2) + T(2)) / T(2);
	}

	template <typename T> inline T OutElastic(T x)
	{
		constexpr T c4 = (T(2) * Math::PI_V<T>) / T(3);

		return x == T(0) ? T(0)
			: x == T(1) ? T(1)
			: pow(T(2), -T(10) * x) * sin((x * T(10) - T(0.75)) * c4) + T(1);
	}

	template <typename T> inline T InElastic(T x)
	{
		constexpr T c4 = (T(2) * Math::PI_V<T>) / T(3);

		return x == T(0) ? T(0)
			: x == T(1) ? T(1)
			: -pow(T(2), T(10) * x - T(10)) * sin((x * T(10) - T(10.75)) * c4);
	}

	template <typename T> inline T InOutElastic(T x)
	{
		constexpr T c5 = (T(2) * Math::PI_V<T>) / T(4.5);

		return x == T(0) ? T(0)
			: x == T(1) ? T(1)
			: x < T(0.5) ? -(pow(T(2), T(20) * x - T(10)) * sin((T(20) * x - T(11.125)) * c5)) / T(2)
			: (pow(T(2), -T(20) * x + T(10)) * sin((T(20) * x - T(11.125)) * c5)) / T(2) + T(1);
	}

	template <typename T> inline T OutBounce(T x)
	{
		constexpr T n1 = T(7.5625);
		constexpr T d1 = T(2.75);

		return x < T(1) / d1 ? n1 * x * x
			: x < T(2) / d1 ? n1 * (x -= T(1.5) / d1) * x + T(0.75)
			: x < T(2.5) / d1 ? n1 * (x -= T(2.25) / d1) * x + T(0.9375)
			: n1 * (x -= T(2.625) / d1) * x + T(0.984375);
	}

	template <typename T> inline T InBounce(T x)
	{
		return T(1) - OutBounce(T(1) - x);
	}
	
	template <typename T> inline T InOutBounce(T x)
	{
		return x < T(0.5)
		  ? (T(1) - OutBounce(T(1) - T(2) * x)) / T(2)
		  : (T(1) + OutBounce(T(2) * x - T(1))) / T(2);
	}
}