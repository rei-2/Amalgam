#pragma once

#include "../../SDK/Definitions/Types.h"

#include <cmath>
#include <cfloat>
#include <algorithm>
#include <array>

#undef min
#undef max

#define PI 3.14159265358979323846
#define M_RADPI 57.295779513082
#define DEG2RAD(x) ((float)(x) * (float)((float)(PI) / 180.f))
#define RAD2DEG(x) ((float)(x) * (float)(180.f / (float)(PI)))

#pragma warning (push)
#pragma warning (disable : 26451)
#pragma warning (disable : 4244)

#define floatCompare(x, y) (fabsf(x - y) <= FLT_EPSILON * fmaxf(1.f, fmaxf(fabsf(x), fabsf(y))))

namespace Math
{
	inline double FastSqrt(double n)
	{
		return std::sqrt(n);
	}

	inline void SinCos(float flRadians, float* pSin, float* pCos)
	{
		*pSin = std::sin(flRadians);
		*pCos = std::cos(flRadians);
	}

	inline float NormalizeAngle(float flAngle)
	{
		return std::isfinite(flAngle) ? std::remainder(flAngle, 360.f) : 0.f;
	}

	inline float NormalizeRad(float flAngle) noexcept
	{
		return std::isfinite(flAngle) ? std::remainder(flAngle, PI * 2) : 0.f;
	}

	inline void ClampAngles(Vec3& v)
	{
		v.x = std::clamp(NormalizeAngle(v.x), -89.f, 89.f);
		v.y = NormalizeAngle(v.y);
		v.z = 0.f;
	}

	inline float AngleDiffRad(float flAngle1, float flAngle2) noexcept
	{
		double delta = NormalizeRad(flAngle1 - flAngle2);
		if (flAngle1 > flAngle2)
		{
			if (delta >= PI) { delta -= PI * 2; }
		}
		else
		{
			if (delta <= -PI) { delta += PI * 2; }
		}
		return static_cast<float>(delta);
	}

	inline void VectorAngles(const Vec3& vForward, Vec3& vAngles)
	{
		float yaw, pitch;

		if (vForward.y == 0 && vForward.x == 0)
		{
			yaw = 0;
			pitch = vForward.z > 0 ? 270 : 90;
		}
		else
		{
			yaw = RAD2DEG(atan2f(vForward.y, vForward.x));
			if (yaw < 0)
				yaw += 360;

			pitch = RAD2DEG(atan2f(-vForward.z, vForward.Length2D()));
			if (pitch < 0)
				pitch += 360;
		}

		vAngles.x = pitch;
		vAngles.y = yaw;
		vAngles.z = 0;
	}

	inline void AngleVectors(const Vec3& vAngles, Vec3* pForward = nullptr, Vec3* pRight = nullptr, Vec3* pUp = nullptr)
	{
		float sp, sy, sr, cp, cy, cr;
		SinCos(DEG2RAD(vAngles.x), &sp, &cp);
		SinCos(DEG2RAD(vAngles.y), &sy, &cy);

		if (pForward)
		{
			pForward->x = cp * cy;
			pForward->y = cp * sy;
			pForward->z = -sp;
		}

		if (pRight || pUp)
		{
			SinCos(DEG2RAD(vAngles.z), &sr, &cr);

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
			atanf(vDelta.z / flHyp) * float(M_RADPI),
			atanf(vDelta.y / vDelta.x) * float(M_RADPI),
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

		float flResult = RAD2DEG(acos(vFromForward.Dot(vToForward)));
		if (!isfinite(flResult) || isinf(flResult) || isnan(flResult))
			flResult = 0.f;

		return flResult;
	}

	inline void CreateVector(const Vec3& angle, Vec3& vector)
	{
		float p, y, tmp;

		p = float(angle[0] * PI / 180);
		y = float(angle[1] * PI / 180);
		tmp = float(cos(p));

		vector[0] = float(-tmp * -cos(y));
		vector[1] = float(sin(y) * tmp);
		vector[2] = float(-sin(p));
	}

	inline float GetFov(const Vec3& vAngle, const Vec3& vFrom, const Vec3& vTo)
	{
		Vec3 vAim, vAng;
		CreateVector(vAngle, vAim);
		CreateVector(CalcAngle(vFrom, vTo), vAng);

		float mag = sqrtf(pow(vAim.x, 2) + pow(vAim.y, 2) + pow(vAim.z, 2));
		float u_dot_v = vAim.Dot(vAng);

		return RAD2DEG(acos(u_dot_v / (pow(mag, 2))));
	}

	inline void VectorTransform(const Vec3& vIn, const matrix3x4& mMatrix, Vec3& vOut)
	{
		for (auto i = 0; i < 3; i++)
			vOut[i] = vIn.Dot(mMatrix[i]) + mMatrix[i][3];
	}

	inline float RemapValClamped(float val, float A, float B, float C, float D)
	{
		if (A == B)
			return val >= B ? D : C;

		float cVal = (val - A) / (B - A);
		cVal = std::clamp(cVal, 0.f, 1.f);

		return C + (D - C) * cVal;
	}

	inline Vec3 VelocityToAngles(const Vec3& vDirection)
	{
		auto Magnitude = [&](const Vec3& v) -> float {
			return sqrtf(v.Dot(v));
		};

		float yaw, pitch;

		if (vDirection.y == 0.f && vDirection.x == 0.f)
		{
			yaw = 0.f;
			pitch = vDirection.z > 0.f ? 270.f : 90.f;
		}
		else
		{
			yaw = RAD2DEG(atan2f(vDirection.y, vDirection.x));
			if (yaw < 0.f)
				yaw += 360.f;

			pitch = RAD2DEG(atan2f(-vDirection.z, Magnitude(vDirection)));
			if (pitch < 0.f)
				pitch += 360.f;
		}

		return { pitch, yaw, 0.f };
	}

	inline void MatrixSetColumn(const Vec3& vIn, int iColumn, matrix3x4& mOut)
	{
		mOut[0][iColumn] = vIn.x;
		mOut[1][iColumn] = vIn.y;
		mOut[2][iColumn] = vIn.z;
	}

	inline void AngleMatrix(const Vec3& vAngles, matrix3x4& mMatrix)
	{
		float sp, sy, sr, cp, cy, cr;
		SinCos(DEG2RAD(vAngles.x), &sp, &cp);
		SinCos(DEG2RAD(vAngles.y), &sy, &cy);
		SinCos(DEG2RAD(vAngles.z), &sr, &cr);

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

		mMatrix[0][3] = 0.f;
		mMatrix[1][3] = 0.f;
		mMatrix[2][3] = 0.f;
	}

	inline void MatrixAngles(const matrix3x4& mMatrix, Vec3& vAngles)
	{
		const Vec3 vForward = { mMatrix[0][0], mMatrix[1][0], mMatrix[2][0] };
		const Vec3 vLeft = { mMatrix[0][1], mMatrix[1][1], mMatrix[2][1] };
		const Vec3 vUp = { 0.f, 0.f, mMatrix[2][2] };

		float flLen = vForward.Length2D();
		if (flLen > 0.001f)
		{
			vAngles.x = RAD2DEG(std::atan2(-vForward.z, flLen));
			vAngles.y = RAD2DEG(std::atan2(vForward.y, vForward.x));
			vAngles.z = RAD2DEG(std::atan2(vLeft.z, vUp.z));
		}
		else
		{
			vAngles.x = RAD2DEG(std::atan2(-vForward.z, flLen));
			vAngles.y = RAD2DEG(std::atan2(-vLeft.x, vLeft.y));
			vAngles.z = 0.f;
		}
	}

	inline void RotateTriangle(std::array<Vec2, 3>& aPoints, float flRotation)
	{
		Vec2 vCenter = (aPoints[0] + aPoints[1] + aPoints[2]) / 3;

		for (auto& vPoint : aPoints)
		{
			vPoint -= vCenter;
			float flX = vPoint.x;
			float flY = vPoint.y;
			float flT = DEG2RAD(flRotation);
			float c = cosf(flT);
			float s = sinf(flT);
			vPoint.x = flX * c - flY * s;
			vPoint.y = flX * s + flY * c;
			vPoint += vCenter;
		}
	}

	inline bool RayToOBB(const Vec3& vOrigin, const Vec3& vDirection, const Vec3& vMins, const Vec3& vMaxs, const matrix3x4& mMatrix)
	{
		Vec3 vDelta = Vec3(mMatrix[0][3], mMatrix[1][3], mMatrix[2][3]) - vOrigin;

		Vec3 X = { mMatrix[0][0], mMatrix[1][0], mMatrix[2][0] };
		Vec3 Y = { mMatrix[0][1], mMatrix[1][1], mMatrix[2][1] };
		Vec3 Z = { mMatrix[0][2], mMatrix[1][2], mMatrix[2][2] };

		Vec3 f = { X.Dot(vDirection), Y.Dot(vDirection), Z.Dot(vDirection) };
		Vec3 e = { X.Dot(vDelta), Y.Dot(vDelta), Z.Dot(vDelta) };

		float t[6] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };

		for (int i = 0; i < 3; ++i)
		{
			if (floatCompare(f[i], 0.f))
			{
				if (-e[i] + vMins[i] > 0.f || -e[i] + vMaxs[i] < 0.f)
					return false;

				f[i] = 0.00001f;
			}

			t[i * 2 + 0] = (e[i] + vMaxs[i]) / f[i];
			t[i * 2 + 1] = (e[i] + vMins[i]) / f[i];
		}

		float tmin = fmaxf(fmaxf(fminf(t[0], t[1]), fminf(t[2], t[3])), fminf(t[4], t[5]));
		float tmax = fminf(fminf(fmaxf(t[0], t[1]), fmaxf(t[2], t[3])), fmaxf(t[4], t[5]));

		if (tmax < 0.f || tmin > tmax)
			return false;

		return true;
	}

	inline void VectorRotate(Vec3& vIn, const matrix3x4& mIn, Vec3& vOut)
	{
		vOut.x = vIn.Dot(mIn[0]);
		vOut.y = vIn.Dot(mIn[1]);
		vOut.z = vIn.Dot(mIn[2]);
	}

	inline Vec3 GetRotatedPosition(Vec3 vStart, const float flRotation, const float flDistance)
	{
		const auto rad = DEG2RAD(flRotation);
		vStart.x += cosf(rad) * flDistance;
		vStart.y += sinf(rad) * flDistance;

		return vStart;
	}

	inline Vec3 RotatePoint(Vec3 vPoint, Vec3 vOrigin, Vec3 vAngles)
	{
		vPoint -= vOrigin;

		float sp, sy, sr, cp, cy, cr;
		SinCos(DEG2RAD(vAngles.x), &sp, &cp);
		SinCos(DEG2RAD(vAngles.y), &sy, &cy);
		SinCos(DEG2RAD(vAngles.z), &sr, &cr);

		Vec3 vX = {
			cy * cp,
			cy * sp * sr - sy * cr,
			cy * sp * cr + sy * sr
		};
		Vec3 vY = {
			sy * cp,
			sy * sp* sr + cy * cr,
			sy * sp* cr - cy * sr
		};
		Vec3 vZ = {
			-sp,
			cp * sr,
			cp * cr
		};

		return Vec3(vX.Dot(vPoint), vY.Dot(vPoint), vZ.Dot(vPoint)) + vOrigin;
	}

	inline void MatrixCopy(const matrix3x4& mIn, matrix3x4& mOut)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				mOut[i][j] = mIn[i][j];
			}
		}
	}

	inline void GetMatrixOrigin(const matrix3x4& mMatrix, Vec3& vOrigin)
	{
		vOrigin.x = mMatrix[0][3];
		vOrigin.y = mMatrix[1][3];
		vOrigin.z = mMatrix[2][3];
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
			return pow(q, 1.f / 3);
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