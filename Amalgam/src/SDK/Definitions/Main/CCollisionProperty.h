#pragma once
#include "ICollideable.h"
#include "../../../Utils/Signatures/Signatures.h"

MAKE_SIGNATURE(CCollisionProperty_SetCollisionBounds, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? F3 0F 10 0A", 0x0);
MAKE_SIGNATURE(CCollisionProperty_CalcNearestPoint, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 49 8B D8 48 8B F9 4C 8D 44 24", 0x0);

typedef unsigned short SpatialPartitionHandle_t;

class CCollisionProperty : public ICollideable
{
public:
	CBaseEntity* m_pOuter;
	Vec3 m_vecMinsPreScaled;
	Vec3 m_vecMaxsPreScaled;
	Vec3 m_vecMins;
	Vec3 m_vecMaxs;
	float m_flRadius;
	unsigned short m_usSolidFlags;
	SpatialPartitionHandle_t m_Partition;
	unsigned char m_nSurroundType;
	unsigned char m_nSolidType;
	unsigned char m_triggerBloat;
	bool m_bUniformTriggerBloat;
	Vec3 m_vecSpecifiedSurroundingMinsPreScaled;
	Vec3 m_vecSpecifiedSurroundingMaxsPreScaled;
	Vec3 m_vecSpecifiedSurroundingMins;
	Vec3 m_vecSpecifiedSurroundingMaxs;
	Vec3 m_vecSurroundingMins;
	Vec3 m_vecSurroundingMaxs;

public:
	SIGNATURE_ARGS(SetCollisionBounds, void, CCollisionProperty, (const Vec3& mins, const Vec3& maxs), this, std::ref(mins), std::ref(maxs));
	SIGNATURE_ARGS(CalcNearestPoint, void, CCollisionProperty, (const Vec3& vecWorldPt, Vec3* pVecNearestWorldPt), this, std::ref(vecWorldPt), pVecNearestWorldPt);
};