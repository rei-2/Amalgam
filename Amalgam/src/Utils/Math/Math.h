#pragma once
#include "../../SDK/Definitions/Types.h"
#include <cmath>
#include <cfloat>
#include <algorithm>

#undef min
#undef max

#pragma warning (push)
#pragma warning (disable : 26451)
#pragma warning (disable : 4244)

#define FLT_COMPARE(x, y) (fabsf(x - y) <= FLT_EPSILON * fmaxf(1.f, fmaxf(fabsf(x), fabsf(y))))

namespace Math
{
	inline float Lerp(float a, float b, float t)
	{
		return a + (b - a) * t;
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

	inline double FastSqrt(double n)
	{
		return std::sqrt(n);
	}

	inline void SinCos(float flRadians, float* pSin, float* pCos)
	{
		*pSin = std::sin(flRadians);
		*pCos = std::cos(flRadians);
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

	inline void ClampAngles(Vec3& v)
	{
		v.x = std::clamp(NormalizeAngle(v.x), -89.f, 89.f);
		v.y = NormalizeAngle(v.y);
		v.z = 0.f;
	}

	inline void ClampAngles(Vec2& v)
	{
		v.x = std::clamp(NormalizeAngle(v.x), -89.f, 89.f);
		v.y = NormalizeAngle(v.y);
	}

	inline void VectorAngles(const Vec3& vForward, Vec3& vAngles)
	{
		float flYaw, flPitch;

		if (vForward.y == 0 && vForward.x == 0)
		{
			flYaw = 0;
			flPitch = vForward.z > 0 ? 270 : 90;
		}
		else
		{
			flYaw = Rad2Deg(atan2f(vForward.y, vForward.x));
			if (flYaw < 0)
				flYaw += 360;

			flPitch = Rad2Deg(atan2f(-vForward.z, vForward.Length2D()));
			if (flPitch < 0)
				flPitch += 360;
		}

		vAngles.x = flPitch;
		vAngles.y = flYaw;
		vAngles.z = 0;
	}

	inline Vec3 VectorAngles(const Vec3& vForward)
	{
		Vec3 vResult;
		VectorAngles(vForward, vResult);
		return vResult;
	}

	inline void AngleVectors(const Vec3& vAngles, Vec3* pForward = nullptr, Vec3* pRight = nullptr, Vec3* pUp = nullptr)
	{
		float sp, sy, sr, cp, cy, cr;
		SinCos(Deg2Rad(vAngles.x), &sp, &cp);
		SinCos(Deg2Rad(vAngles.y), &sy, &cy);

		if (pForward)
		{
			pForward->x = cp * cy;
			pForward->y = cp * sy;
			pForward->z = -sp;
		}

		if (pRight || pUp)
		{
			SinCos(Deg2Rad(vAngles.z), &sr, &cr);

			if (pRight)
			{
				pRight->x = (-1 * sr * sp * cy + -1 * cr * -sy);
				pRight->y = (-1 * sr * sp * sy + -1 * cr * cy);
				pRight->z = -1 * sr * cp;
			}

			if (pUp)
			{
				pUp->x = (cr * sp * cy + -sr * -sy);
				pUp->y = (cr * sp * sy + -sr * cy);
				pUp->z = cr * cp;
			}
		}
	}

	inline Vec3 CalcAngle(const Vec3& vFrom, const Vec3& vTo, bool bClamp = true)
	{
		Vec3 vDelta = vFrom - vTo;
		float flHyp = std::sqrtf((vDelta.x * vDelta.x) + (vDelta.y * vDelta.y));

		Vec3 vAngles = {
			atanf(vDelta.z / flHyp) * RAD,
			atanf(vDelta.y / vDelta.x) * RAD,
			0.f
		};

		if (vDelta.x >= 0.f)
			vAngles.y += 180.f;

		if (bClamp)
			ClampAngles(vAngles);

		return vAngles;
	}

	inline float CalcFov(const Vec3& vFromAng, const Vec3& vToAng)
	{
		Vec3 vFromForward = Vec3();
		AngleVectors(vFromAng, &vFromForward);

		Vec3 vToForward = Vec3();
		AngleVectors(vToAng, &vToForward);

		float flResult = Rad2Deg(acos(vFromForward.Dot(vToForward)));
		if (!isfinite(flResult) || isinf(flResult) || isnan(flResult))
			flResult = 0.f;

		return flResult;
	}

	inline Vec3 RotatePoint(const Vec3& vPoint, const Vec3& vOrigin, const Vec3& vAngles)
	{
		Vec3 vOffset = vPoint - vOrigin;

		float sp, sy, sr, cp, cy, cr;
		SinCos(Deg2Rad(vAngles.x), &sp, &cp);
		SinCos(Deg2Rad(vAngles.y), &sy, &cy);
		SinCos(Deg2Rad(vAngles.z), &sr, &cr);

		Vec3 vX = {
			cy * cp,
			cy * sp * sr - sy * cr,
			cy * sp * cr + sy * sr
		};
		Vec3 vY = {
			sy * cp,
			sy * sp * sr + cy * cr,
			sy * sp * cr - cy * sr
		};
		Vec3 vZ = {
			-sp,
			cp * sr,
			cp * cr
		};

		return Vec3(vX.Dot(vOffset), vY.Dot(vOffset), vZ.Dot(vOffset)) + vOrigin;
	}

	inline bool IsBoxIntersectingBox(const Vec3& vMins1, const Vec3& vMaxs1, const Vec3& vMins2, const Vec3& vMaxs2)
	{
		if ((vMins1.x > vMaxs2.x) || (vMaxs1.x < vMins2.x))
			return false;
		if ((vMins1.y > vMaxs2.y) || (vMaxs1.y < vMins2.y))
			return false;
		if ((vMins1.z > vMaxs2.z) || (vMaxs1.z < vMins2.z))
			return false;
		return true;
	}

	inline bool IsPointIntersectingBox(const Vec3& vPoint, const Vec3& vMins, const Vec3& vMaxs)
	{
		return IsBoxIntersectingBox(vPoint, vPoint, vMins, vMaxs);
	}

	inline void VectorTransform(const Vec3& vIn, const matrix3x4& mMatrix, Vec3& vOut)
	{
		for (auto i = 0; i < 3; i++)
			vOut[i] = vIn.Dot(mMatrix[i]) + mMatrix[i][3];
	}

	inline Vec3 VectorTransform(const Vec3& vIn, const matrix3x4& mMatrix)
	{
		Vec3 vOut;
		VectorTransform(vIn, mMatrix, vOut);
		return vOut;
	}

	inline void VectorITransform(const Vec3& vIn, const matrix3x4& mMatrix, Vec3& vOut)
	{
		Vec3 vMatrixT;
		for (auto i = 0; i < 3; i++)
			vMatrixT[i] = vIn[i] - mMatrix[i][3];
		for (auto i = 0; i < 3; i++)
			vOut[i] = vMatrixT[0] * mMatrix[0][i] + vMatrixT[1] * mMatrix[1][i] + vMatrixT[2] * mMatrix[2][i];
	}

	inline Vec3 VectorITransform(const Vec3& vIn, const matrix3x4& mMatrix)
	{
		Vec3 vOut;
		VectorITransform(vIn, mMatrix, vOut);
		return vOut;
	}

	inline void MatrixSetColumn(const Vec3& vIn, int iColumn, matrix3x4& mOut)
	{
		mOut[0][iColumn] = vIn.x;
		mOut[1][iColumn] = vIn.y;
		mOut[2][iColumn] = vIn.z;
	}

	inline void AngleMatrix(const Vec3& vAngles, matrix3x4& mMatrix, bool bClearOrigin = true)
	{
		float sp, sy, sr, cp, cy, cr;
		SinCos(Deg2Rad(vAngles.x), &sp, &cp);
		SinCos(Deg2Rad(vAngles.y), &sy, &cy);
		SinCos(Deg2Rad(vAngles.z), &sr, &cr);

		mMatrix[0][0] = cp * cy;
		mMatrix[1][0] = cp * sy;
		mMatrix[2][0] = -sp;

		float crcy = cr * cy;
		float crsy = cr * sy;
		float srcy = sr * cy;
		float srsy = sr * sy;

		mMatrix[0][1] = sp * srcy - crsy;
		mMatrix[1][1] = sp * srsy + crcy;
		mMatrix[2][1] = sr * cp;

		mMatrix[0][2] = (sp * crcy + srsy);
		mMatrix[1][2] = (sp * crsy - srcy);
		mMatrix[2][2] = cr * cp;

		if (bClearOrigin)
		{
			mMatrix[0][3] = 0.f;
			mMatrix[1][3] = 0.f;
			mMatrix[2][3] = 0.f;
		}
	}

	inline void MatrixAngles(const matrix3x4& mMatrix, Vec3& vAngles)
	{
		const Vec3 vForward = { mMatrix[0][0], mMatrix[1][0], mMatrix[2][0] };
		const Vec3 vLeft = { mMatrix[0][1], mMatrix[1][1], mMatrix[2][1] };
		const Vec3 vUp = { 0.f, 0.f, mMatrix[2][2] };

		float flLen = vForward.Length2D();
		if (flLen > 0.001f)
		{
			vAngles.x = Rad2Deg(std::atan2(-vForward.z, flLen));
			vAngles.y = Rad2Deg(std::atan2(vForward.y, vForward.x));
			vAngles.z = Rad2Deg(std::atan2(vLeft.z, vUp.z));
		}
		else
		{
			vAngles.x = Rad2Deg(std::atan2(-vForward.z, flLen));
			vAngles.y = Rad2Deg(std::atan2(-vLeft.x, vLeft.y));
			vAngles.z = 0;
		}
	}

	inline Vec3 MatrixAngles(const matrix3x4& mMatrix)
	{
		Vec3 vOut;
		MatrixAngles(mMatrix, vOut);
		return vOut;
	}

	inline bool RayToOBB(const Vec3& vOrigin, const Vec3& vDirection, const Vec3& vMins, const Vec3& vMaxs, const matrix3x4& mMatrix, float flScale = 1.f)
	{
		if (!flScale)
			return false;

		Vec3 vDelta = Vec3(mMatrix[0][3], mMatrix[1][3], mMatrix[2][3]) - vOrigin;

		Vec3 X = Vec3(mMatrix[0][0], mMatrix[1][0], mMatrix[2][0]) / flScale;
		Vec3 Y = Vec3(mMatrix[0][1], mMatrix[1][1], mMatrix[2][1]) / flScale;
		Vec3 Z = Vec3(mMatrix[0][2], mMatrix[1][2], mMatrix[2][2]) / flScale;
		if (flScale != 1.f)
			X /= flScale, Y /= flScale, Z /= flScale;

		Vec3 f = Vec3(X.Dot(vDirection), Y.Dot(vDirection), Z.Dot(vDirection));
		Vec3 e = Vec3(X.Dot(vDelta), Y.Dot(vDelta), Z.Dot(vDelta));

		float t[6] = { 0, 0, 0, 0, 0, 0 };

		for (int i = 0; i < 3; ++i)
		{
			if (FLT_COMPARE(f[i], 0.f))
			{
				if (-e[i] + vMins[i] * flScale > 0.f || -e[i] + vMaxs[i] * flScale < 0.f)
					return false;

				f[i] = 0.00001f;
			}

			t[i * 2 + 0] = (e[i] + vMaxs[i]) / f[i];
			t[i * 2 + 1] = (e[i] + vMins[i]) / f[i];
		}

		float flTMin = fmaxf(fmaxf(fminf(t[0], t[1]), fminf(t[2], t[3])), fminf(t[4], t[5]));
		float flTMax = fminf(fminf(fmaxf(t[0], t[1]), fmaxf(t[2], t[3])), fmaxf(t[4], t[5]));

		if (flTMax < 0.f || flTMin > flTMax)
			return false;

		return true;
	}

	inline void VectorRotate(const Vec3& vIn, const matrix3x4& mMatrix, Vec3& vOut)
	{
		vOut.x = vIn.Dot(mMatrix[0]);
		vOut.y = vIn.Dot(mMatrix[1]);
		vOut.z = vIn.Dot(mMatrix[2]);
	}

	inline Vec3 VectorRotate(const Vec3& vIn, const matrix3x4& mMatrix)
	{
		Vec3 vOut;
		VectorRotate(vIn, mMatrix, vOut);
		return vOut;
	}

	inline void MatrixCopy(const matrix3x4& mIn, matrix3x4& mOut)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
				mOut[i][j] = mIn[i][j];
		}
	}

	inline void GetMatrixOrigin(const matrix3x4& mMatrix, Vec3& vOrigin)
	{
		vOrigin.x = mMatrix[0][3];
		vOrigin.y = mMatrix[1][3];
		vOrigin.z = mMatrix[2][3];
	}

	inline Vec3 GetMatrixOrigin(const matrix3x4& mMatrix)
	{
		Vec3 vOut;
		GetMatrixOrigin(mMatrix, vOut);
		return vOut;
	}

	inline void ConcatTransforms(const matrix3x4& mIn1, const matrix3x4& mIn2, matrix3x4& mOut)
	{
		if (&mIn1 == &mOut)
		{
			matrix3x4 mIn1b;
			MatrixCopy(mIn1, mIn1b);
			ConcatTransforms(mIn1b, mIn2, mOut);
			return;
		}

		if (&mIn2 == &mOut)
		{
			matrix3x4 mIn2b;
			MatrixCopy(mIn2, mIn2b);
			ConcatTransforms(mIn1, mIn2b, mOut);
			return;
		}

		mOut[0][0] = mIn1[0][0] * mIn2[0][0] + mIn1[0][1] * mIn2[1][0] + mIn1[0][2] * mIn2[2][0];
		mOut[0][1] = mIn1[0][0] * mIn2[0][1] + mIn1[0][1] * mIn2[1][1] + mIn1[0][2] * mIn2[2][1];
		mOut[0][2] = mIn1[0][0] * mIn2[0][2] + mIn1[0][1] * mIn2[1][2] + mIn1[0][2] * mIn2[2][2];
		mOut[0][3] = mIn1[0][0] * mIn2[0][3] + mIn1[0][1] * mIn2[1][3] + mIn1[0][2] * mIn2[2][3] + mIn1[0][3];
		mOut[1][0] = mIn1[1][0] * mIn2[0][0] + mIn1[1][1] * mIn2[1][0] + mIn1[1][2] * mIn2[2][0];
		mOut[1][1] = mIn1[1][0] * mIn2[0][1] + mIn1[1][1] * mIn2[1][1] + mIn1[1][2] * mIn2[2][1];
		mOut[1][2] = mIn1[1][0] * mIn2[0][2] + mIn1[1][1] * mIn2[1][2] + mIn1[1][2] * mIn2[2][2];
		mOut[1][3] = mIn1[1][0] * mIn2[0][3] + mIn1[1][1] * mIn2[1][3] + mIn1[1][2] * mIn2[2][3] + mIn1[1][3];
		mOut[2][0] = mIn1[2][0] * mIn2[0][0] + mIn1[2][1] * mIn2[1][0] + mIn1[2][2] * mIn2[2][0];
		mOut[2][1] = mIn1[2][0] * mIn2[0][1] + mIn1[2][1] * mIn2[1][1] + mIn1[2][2] * mIn2[2][1];
		mOut[2][2] = mIn1[2][0] * mIn2[0][2] + mIn1[2][1] * mIn2[1][2] + mIn1[2][2] * mIn2[2][2];
		mOut[2][3] = mIn1[2][0] * mIn2[0][3] + mIn1[2][1] * mIn2[1][3] + mIn1[2][2] * mIn2[2][3] + mIn1[2][3];
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

		float sqrt_2m = sqrt(2 * m);
		if (q == 0.f)
		{
			if (-m - p > 0.f)
			{
				float flDelta = sqrt(2 * (-m - p));
				vRoots.push_back(-b / 4 + (sqrt_2m - flDelta) / 2);
				vRoots.push_back(-b / 4 - (sqrt_2m - flDelta) / 2);
				vRoots.push_back(-b / 4 + (sqrt_2m + flDelta) / 2);
				vRoots.push_back(-b / 4 - (sqrt_2m + flDelta) / 2);
			}
			if (-m - p == 0.f)
			{
				vRoots.push_back(-b / 4 - sqrt_2m / 2);
				vRoots.push_back(-b / 4 + sqrt_2m / 2);
			}
		}
		else
		{
			if (-m - p + q / sqrt_2m >= 0.f)
			{
				float flDelta = sqrt(2 * (-m - p + q / sqrt_2m));
				vRoots.push_back((-sqrt_2m + flDelta) / 2 - b / 4);
				vRoots.push_back((-sqrt_2m - flDelta) / 2 - b / 4);
			}
			if (-m - p - q / sqrt_2m >= 0.f)
			{
				float flDelta = sqrt(2 * (-m - p - q / sqrt_2m));
				vRoots.push_back((sqrt_2m + flDelta) / 2 - b / 4);
				vRoots.push_back((sqrt_2m - flDelta) / 2 - b / 4);
			}
		}
		return vRoots;
	}
}

#pragma warning (pop)