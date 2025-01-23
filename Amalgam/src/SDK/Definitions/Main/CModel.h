#pragma once
#include "CBaseTrace.h"
#include "CBaseHandle.h"
#include "../Misc/BSPFlags.h"
#include "../Misc/VCollide.h"

struct edict_t;
struct model_t;

#define	AREA_SOLID 1
#define	AREA_TRIGGERS 2

struct cmodel_t
{
	Vector mins, maxs;
	Vector origin;
	int headnode;
	vcollide_t	vcollisionData;
};

struct csurface_t
{
	const char* name;
	short surfaceProps;
	unsigned short	flags;
};

class __declspec(align(16))VectorAligned : public Vec3
{
public:
	inline VectorAligned(void) { };

	inline VectorAligned(float x, float y, float z) {
		Set(x, y, z);
	}

	explicit VectorAligned(const Vec3& othr) {
		Set(othr.x, othr.y, othr.z);
	}

	VectorAligned& operator=(const Vec3& othr) {
		Set(othr.x, othr.y, othr.z);
		return *this;
	}

	float w = 0.0f;
};

struct Ray_t
{
	VectorAligned m_Start{};
	VectorAligned m_Delta{};
	VectorAligned m_StartOffset{};
	VectorAligned m_Extents{};
	bool m_IsRay{};
	bool m_IsSwept{};

	void Init(Vector const& start, Vector const& end)
	{
		m_Delta = end - start;
		m_IsSwept = m_Delta.LengthSqr() != 0;
		m_Extents.Set();
		m_IsRay = true;
		m_StartOffset.Set();
		m_Start = start;
	}

	void Init(Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs)
	{
		m_Delta = end - start;
		m_IsSwept = m_Delta.LengthSqr() != 0;
		m_Extents = maxs - mins;
		m_Extents *= 0.5f;
		m_IsRay = m_Extents.LengthSqr() == 0;
		m_StartOffset = mins + maxs;
		m_StartOffset *= 0.5f;
		m_Start = start + m_StartOffset;
		m_StartOffset *= -1.0f;
	}

	Vector InvDelta() const
	{
		Vector vecInvDelta;
		for (int iAxis = 0; iAxis < 3; ++iAxis)
		{
			if (m_Delta[iAxis] != 0.0f)
			{
				vecInvDelta[iAxis] = 1.0f / m_Delta[iAxis];
			}
			else
			{
				vecInvDelta[iAxis] = 3.402823e+38f;
			}
		}
		return vecInvDelta;
	}
};