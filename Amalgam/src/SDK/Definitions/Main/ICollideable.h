#pragma once
#include "CGameTrace.h"
#include "../Definitions.h"

class IHandleEntity;
class IClientUnknown;
struct model_t;

class ICollideable
{
public:
	virtual IHandleEntity* GetEntityHandle() = 0;
	virtual const Vector& OBBMinsPreScaled() const = 0;
	virtual const Vector& OBBMaxsPreScaled() const = 0;
	virtual const Vector& OBBMins() const = 0;
	virtual const Vector& OBBMaxs() const = 0;
	virtual void WorldSpaceTriggerBounds(Vector* pVecWorldMins, Vector* pVecWorldMaxs) const = 0;
	virtual bool TestCollision(const Ray_t& ray, unsigned int fContentsMask, trace_t& tr) = 0;
	virtual bool TestHitboxes(const Ray_t& ray, unsigned int fContentsMask, trace_t& tr) = 0;
	virtual int GetCollisionModelIndex() = 0;
	virtual const model_t* GetCollisionModel() = 0;
	virtual const Vector& GetCollisionOrigin() const = 0;
	virtual const QAngle& GetCollisionAngles() const = 0;
	virtual const matrix3x4& CollisionToWorldTransform() const = 0;
	virtual SolidType_t GetSolid() const = 0;
	virtual int GetSolidFlags() const = 0;
	virtual IClientUnknown* GetIClientUnknown() = 0;
	virtual int GetCollisionGroup() const = 0;
	virtual void WorldSpaceSurroundingBounds(Vector* pVecMins, Vector* pVecMaxs) = 0;
	virtual bool ShouldTouchTrigger(int triggerSolidFlags) const = 0;
	virtual const matrix3x4* GetRootParentToWorldTransform() const = 0;

	inline bool IsSolid()
	{
		return GetSolid() != SOLID_NONE && !(GetSolidFlags() & FSOLID_NOT_SOLID);
	}
};