#pragma once
#include "BaseMath.h"
#include "../../SDK/Definitions/Types.h"
#include "../Hash/FNV1A.h"

#undef min
#undef max

#define FLT_COMPARE(x, y) (fabsf(x - y) <= FLT_EPSILON * fmaxf(1.f, fmaxf(fabsf(x), fabsf(y))))

#define DIST_EPSILON 0.03125f
#define CALC_EPSILON 0.001f

namespace Math
{
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

	inline bool IsBoxIntersectingBox(const Vec3& vMins1, const Vec3& vMaxs1, const Vec3& vMins2, const Vec3& vMaxs2)
	{
		if (vMins1.x > vMaxs2.x || vMaxs1.x < vMins2.x)
			return false;
		if (vMins1.y > vMaxs2.y || vMaxs1.y < vMins2.y)
			return false;
		if (vMins1.z > vMaxs2.z || vMaxs1.z < vMins2.z)
			return false;
		return true;
	}

	inline bool IsPointIntersectingBox(const Vec3& vPoint, const Vec3& vMins, const Vec3& vMaxs)
	{
		return IsBoxIntersectingBox(vPoint, vPoint, vMins, vMaxs);
	}

	inline void VectorAngles(const Vec3& vForward, Vec3& vAngles)
	{
		if (vForward.x || vForward.y)
		{
			vAngles.x = Rad2Deg(atan2f(-vForward.z, vForward.Length2D()));
			vAngles.y = Rad2Deg(atan2f(vForward.y, vForward.x));
		}
		else
		{
			vAngles.x = vForward.z > 0 ? 270 : 90;
			vAngles.y = 0;
		}
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
		float sp, sy, cp, cy;
		SinCos(Deg2Rad(vAngles.x), sp, cp);
		SinCos(Deg2Rad(vAngles.y), sy, cy);

		if (pForward)
		{
			pForward->x = cp * cy;
			pForward->y = cp * sy;
			pForward->z = -sp;
		}

		if (pRight || pUp)
		{
			float sr, cr;
			SinCos(Deg2Rad(vAngles.z), sr, cr);

			if (pRight)
			{
				pRight->x = -1 * sr * sp * cy + -1 * cr * -sy;
				pRight->y = -1 * sr * sp * sy + -1 * cr * cy;
				pRight->z = -1 * sr * cp;
			}

			if (pUp)
			{
				pUp->x = cr * sp * cy + -sr * -sy;
				pUp->y = cr * sp * sy + -sr * cy;
				pUp->z = cr * cp;
			}
		}
	}

	inline Vec3 CalcAngle(const Vec3& vFrom, const Vec3& vTo, bool bClamp = true)
	{
		Vec3 vDelta = vFrom - vTo;
		float flHyp = std::sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y);

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
		if (!isfinite(flResult))
			flResult = 0.f;

		return flResult;
	}

	inline Vec3 RotatePoint(const Vec3& vPoint, const Vec3& vOrigin, const Vec3& vAngles)
	{
		Vec3 vOffset = vPoint - vOrigin;

		float sp, sy, sr, cp, cy, cr;
		SinCos(Deg2Rad(vAngles.x), sp, cp);
		SinCos(Deg2Rad(vAngles.y), sy, cy);
		SinCos(Deg2Rad(vAngles.z), sr, cr);

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
	
	inline float AABBLine(Vec3 vMins, Vec3 vMaxs, Vec3 vStart, Vec3 vDir)
	{
		Vec3 a = {
			(vMins.x - vStart.x) / vDir.x,
			(vMins.y - vStart.y) / vDir.y,
			(vMins.z - vStart.z) / vDir.z
		};
		Vec3 b = {
			(vMaxs.x - vStart.x) / vDir.x,
			(vMaxs.y - vStart.y) / vDir.y,
			(vMaxs.z - vStart.z) / vDir.z
		};
		Vec3 c = {
			std::min(a.x, b.x),
			std::min(a.y, b.y),
			std::min(a.z, b.z)
		};
		return std::max(std::max(c.x, c.y), c.z);
	}

	inline Vec3 PullPoint(Vec3 vFrom, Vec3 vTo, Vec3 vOrigin, Vec3 vMins, Vec3 vMaxs)
	{
		return vTo.Lerp(vFrom, fabsf(AABBLine(vOrigin + vMins, vOrigin + vMaxs, vTo, vFrom - vTo)));
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
		SinCos(Deg2Rad(vAngles.x), sp, cp);
		SinCos(Deg2Rad(vAngles.y), sy, cy);
		SinCos(Deg2Rad(vAngles.z), sr, cr);

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

	inline void OffsetPolygon(std::vector<Vec3>& vVertices, const Vec3& vNormal, float flOffset)
	{
		for (auto& vVertex : vVertices)
			vVertex += vNormal * flOffset;
	}

	inline void ExpandPolygon(std::vector<Vec3>& vOldVertices, std::vector<Vec3>& vNewVertices, const Vec3& vNormal, float flOffset, const Vec3* pTarget = nullptr)
	{
		Vec3 vForward = (vNewVertices.front() - vNewVertices.back()).Normalized();
		Vec3 vRight = vForward.Cross(vNormal).Normalized();

		vOldVertices = vNewVertices;
		for (int i = 0, n = int(vNewVertices.size()); i < n; i++)
		{
			Vec3& vVertex1 = vOldVertices[i], &vVertex2 = vOldVertices[(i + 1) % n], &vVertex3 = vOldVertices[(i + 2) % n];

			Vec2 vPoint1 = { vVertex1.Dot(vForward), vVertex1.Dot(vRight) };
			Vec2 vPoint2 = { vVertex2.Dot(vForward), vVertex2.Dot(vRight) };
			Vec2 vPoint3 = { vVertex3.Dot(vForward), vVertex3.Dot(vRight) };

			Vec2 vDelta21 = vPoint2 - vPoint1;
			Vec2 vDelta32 = vPoint3 - vPoint2;

			Vec2 vNormal1 = Vec2(vDelta21.y, -vDelta21.x).Normalized();
			Vec2 vNormal2 = Vec2(vDelta32.y, -vDelta32.x).Normalized();

			Vec2 vBisect = Vec2(vNormal1.x + vNormal2.x, vNormal1.y + vNormal2.y).Normalized();
			float flDistance = flOffset / vNormal1.Dot(vBisect);
			vBisect *= flDistance;

			if (!pTarget)
				vNewVertices[(i + 1) % n] += vForward * vBisect.x + vRight * vBisect.y;
			else
			{
				Vec3 vToTarget = *pTarget - vVertex2;
				if (Vec3 vDisplacementX = vForward * vBisect.x; vDisplacementX.Dot(vToTarget) > 0.f)
					vNewVertices[(i + 1) % n] += vDisplacementX;
				if (Vec3 vDisplacementY = vRight * vBisect.y; vDisplacementY.Dot(vToTarget) > 0.f)
					vNewVertices[(i + 1) % n] += vDisplacementY;
			}
		}
	}
	inline void ExpandPolygon(std::vector<Vec3>& vVertices, const Vec3& vNormal, float flOffset, const Vec3* pTarget = nullptr)
	{
		std::vector<Vec3> vTemporary;
		ExpandPolygon(vTemporary, vVertices, vNormal, flOffset, pTarget);
	}

	inline Vec3 ClosestPointOnLine(const Vec3& vPoint, const Vec3& vPoint1, const Vec3& vPoint2)
	{
		Vec3 vDelta = vPoint2 - vPoint1;

		float flRatio = std::clamp((vPoint - vPoint1).Dot(vDelta) / vDelta.Dot(vDelta), 0.f, 1.f);

		return vPoint1 + vDelta * flRatio;
	}

	inline Vec3 ClosestPointOnTriangle(const Vec3& vPoint, const Vec3& vPoint1, const Vec3& vPoint2, const Vec3& vPoint3, bool* pInside = nullptr)
	{
		Vec3 vDelta21 = vPoint2 - vPoint1;
		Vec3 vDelta31 = vPoint3 - vPoint1;
		if (pInside) *pInside = false;

		Vec3 vDeltaPT = vPoint - vPoint1;
		float flDot1 = vDelta21.Dot(vDeltaPT);
		float flDot2 = vDelta31.Dot(vDeltaPT);
		if (flDot1 <= 0 && flDot2 <= 0)
			return vPoint1;

		vDeltaPT = vPoint - vPoint2;
		float flDot3 = vDelta21.Dot(vDeltaPT);
		float flDot4 = vDelta31.Dot(vDeltaPT);
		if (flDot3 >= 0 && flDot4 <= flDot3)
			return vPoint2;

		vDeltaPT = vPoint - vPoint3;
		float flDot5 = vDelta21.Dot(vDeltaPT);
		float flDot6 = vDelta31.Dot(vDeltaPT);
		if (flDot6 >= 0 && flDot5 <= flDot6)
			return vPoint3;

		float flSideC = flDot1 * flDot4 - flDot3 * flDot2;
		if (flSideC <= 0 && flDot1 >= 0 && flDot3 <= 0)
		{
			float flRatio = flDot1 / (flDot1 - flDot3);
			return vPoint1 + vDelta21 * flRatio;
		}

		float flSideB = flDot5 * flDot2 - flDot1 * flDot6;
		if (flSideB <= 0 && flDot2 >= 0 && flDot6 <= 0)
		{
			float flRatio = flDot2 / (flDot2 - flDot6);
			return vPoint1 + vDelta31 * flRatio;
		}

		float flSideA = flDot3 * flDot6 - flDot5 * flDot4;
		if (flSideA <= 0 && (flDot4 - flDot3) >= 0 && (flDot5 - flDot6) >= 0)
		{
			float flRatio = (flDot4 - flDot3) / (flDot4 - flDot3 + flDot5 - flDot6);
			return vPoint2 + (vPoint3 - vPoint2) * flRatio;
		}

		float flDenominator = 1 / (flSideA + flSideB + flSideC);
		float flV = flSideB * flDenominator;
		float flW = flSideC * flDenominator;
		if (pInside) *pInside = true;
		return vPoint1 + vDelta21 * flV + vDelta31 * flW;
	}

	inline Vec3 ClosestPointOnPolygon(const Vec3& vPoint, const std::vector<Vec3>& vVertices, const Vec3& vNormal, bool* pInside = nullptr)
	{
		float flDot = vNormal.Dot(vPoint - vVertices.front());
		Vec3 vProjection = vPoint - flDot * vNormal;

		for (int i = 0, n = int(vVertices.size()); i < n; i++)
		{
			const Vec3& vVertex1 = vVertices[i], &vVertex2 = vVertices[(i + 1) % n];

			Vec3 vEdge = vVertex2 - vVertex1;
			Vec3 vToProjection = vProjection - vVertex1;

			if (vToProjection.Cross(vEdge).Dot(vNormal) < 0)
				goto outside;
		}
		if (pInside) *pInside = true;
		return vProjection;

		outside:
		float flLowestDistance = std::numeric_limits<float>::max();
		for (int i = 0, n = int(vVertices.size()); i < n; i++)
		{
			const Vec3& vVertex1 = vVertices[i], &vVertex2 = vVertices[(i + 1) % n];

			Vec3 vLine = ClosestPointOnLine(vPoint, vVertex1, vVertex2);
			float flDistance = vLine.DistToSqr(vPoint);
			if (flDistance < flLowestDistance)
				vProjection = vLine, flLowestDistance = flDistance;
		}
		if (pInside) *pInside = false;
		return vProjection;
	}

	// note: not clamped
	inline float FullFraction(const Vec3& vDir, const CGameTrace& trace)
	{
		if (!trace.DidHit() || FNV1A::Hash32(trace.surface.name) == FNV1A::Hash32Const("**displacement**"))
			return trace.fraction;

		float flFractionEpsilon = DIST_EPSILON / vDir.Dot(trace.plane.normal);
		return trace.fraction - flFractionEpsilon;
	}
	inline float FullFraction(const Vec3& vStart, const Vec3& vEnd, const CGameTrace& trace)
	{
		return FullFraction(vEnd - vStart, trace);
	}
}