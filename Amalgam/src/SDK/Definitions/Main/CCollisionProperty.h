#pragma once
#include "CGameTrace.h"
#include "../Types.h"
#include "../../../Utils/Signatures/Signatures.h"

MAKE_SIGNATURE(CCollisionProperty_SetCollisionBounds, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 81 EC ? ? ? ? F3 0F 10 0A", 0x0);
MAKE_SIGNATURE(CCollisionProperty_CalcNearestPoint, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 49 8B D8 48 8B F9 4C 8D 44 24", 0x0);

struct model_t;

class ICollideable
{
public:
	virtual void* GetEntityHandle() = 0;
	virtual const Vec3& OBBMinsPreScaled() const = 0;
	virtual const Vec3& OBBMaxsPreScaled() const = 0;
	virtual const Vec3& OBBMins() const = 0;
	virtual const Vec3& OBBMaxs() const = 0;
	virtual void			WorldSpaceTriggerBounds(Vec3* pVecWorldMins, Vec3* pVecWorldMaxs) const = 0;
	virtual bool			TestCollision(const Ray_t& ray, unsigned int fContentsMask, CGameTrace& tr) = 0;
	virtual bool			TestHitboxes(const Ray_t& ray, unsigned int fContentsMask, CGameTrace& tr) = 0;
	virtual int				GetCollisionModelIndex() = 0;
	virtual const model_t* GetCollisionModel() = 0;
	virtual const Vec3& GetCollisionOrigin() const = 0;
	virtual const Vec3& GetCollisionAngles() const = 0;
	virtual const matrix3x4& CollisionToWorldTransform() const = 0;
	virtual SolidType_t		GetSolid() const = 0;
	virtual int				GetSolidFlags() const = 0;
	virtual void* GetIClientUnknown() = 0;
	virtual int				GetCollisionGroup() const = 0;
	virtual void			WorldSpaceSurroundingBounds(Vec3* pVecMins, Vec3* pVecMaxs) = 0;
	virtual bool			ShouldTouchTrigger(int triggerSolidFlags) const = 0;
	virtual const matrix3x4* GetRootParentToWorldTransform() const = 0;
};

class CCollisionProperty : public ICollideable
{
public:
	inline void SetCollisionBounds(const Vec3& mins, const Vec3& maxs)
	{
		S::CCollisionProperty_SetCollisionBounds.Call<void>(this, std::ref(mins), std::ref(maxs));
	}

	inline void CalcNearestPoint(const Vec3& vecWorldPt, Vec3* pVecNearestWorldPt)
	{
		S::CCollisionProperty_CalcNearestPoint.Call<void>(this, std::ref(vecWorldPt), pVecNearestWorldPt);
	}
};