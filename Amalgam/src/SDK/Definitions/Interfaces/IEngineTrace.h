#pragma once
#include "Interface.h"
#include "../Main/CModel.h"
#include "../Misc/CUtlVector.h"
#include "../Types.h"

class CGameTrace;
class IHandleEntity;
class ICollideable;
class CTraceListData;
class CPhysCollide;
typedef CGameTrace trace_t;

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,
	TRACE_ENTITIES_ONLY,
	TRACE_EVERYTHING_FILTER_PROPS
};

enum
{
	TRACE_STAT_COUNTER_TRACERAY = 0,
	TRACE_STAT_COUNTER_POINTCONTENTS,
	TRACE_STAT_COUNTER_ENUMERATE,
	NUM_TRACE_STAT_COUNTER
};

class ITraceFilter
{
public:
	virtual bool ShouldHitEntity(IHandleEntity* pEntity, int contentsMask) = 0;
	virtual TraceType_t	GetTraceType() const = 0;
};

class IEntityEnumerator
{
public:
	virtual bool EnumEntity(IHandleEntity* pHandleEntity) = 0;
};

class IEngineTrace
{
public:
	virtual int GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity = NULL) = 0;
	virtual int GetPointContents_Collideable(ICollideable* pCollide, const Vector& vecAbsPosition) = 0;
	virtual void ClipRayToEntity(const Ray_t& ray, unsigned int fMask, IHandleEntity* pEnt, trace_t* pTrace) = 0;
	virtual void ClipRayToCollideable(const Ray_t& ray, unsigned int fMask, ICollideable* pCollide, trace_t* pTrace) = 0;
	virtual void TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void SetupLeafAndEntityListRay(const Ray_t& ray, CTraceListData& traceData) = 0;
	virtual void SetupLeafAndEntityListBox(const Vector& vecBoxMin, const Vector& vecBoxMax, CTraceListData& traceData) = 0;
	virtual void TraceRayAgainstLeafAndEntityList(const Ray_t& ray, CTraceListData& traceData, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void SweepCollideable(ICollideable* pCollide, const Vector& vecAbsStart, const Vector& vecAbsEnd, const QAngle& vecAngles, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void EnumerateEntities(const Ray_t& ray, bool triggers, IEntityEnumerator* pEnumerator) = 0;
	virtual void EnumerateEntities(const Vector& vecAbsMins, const Vector& vecAbsMaxs, IEntityEnumerator* pEnumerator) = 0;
	virtual ICollideable* GetCollideable(IHandleEntity* pEntity) = 0;
	virtual int GetStatByIndex(int index, bool bClear) = 0;
	virtual void GetBrushesInAABB(const Vector& vMins, const Vector& vMaxs, CUtlVector<int>* pOutput, int iContentsMask = 0xFFFFFFFF) = 0;
	virtual CPhysCollide* GetCollidableFromDisplacementsInAABB(const Vector& vMins, const Vector& vMaxs) = 0;
	virtual bool GetBrushInfo(int iBrush, CUtlVector<Vector4D>* pPlanesOut, int* pContentsOut) = 0;
	virtual bool PointOutsideWorld(const Vector& ptTest) = 0;
	virtual int GetLeafContainingPoint(const Vector& ptTest) = 0;
};

class CEngineTrace : public IEngineTrace
{
public:
	virtual int GetPointContents(const Vector& vecAbsPosition, IHandleEntity** ppEntity = NULL) = 0;
	virtual int GetPointContents_Collideable(ICollideable* pCollide, const Vector& vecAbsPosition) = 0;
	virtual void ClipRayToEntity(const Ray_t& ray, unsigned int fMask, IHandleEntity* pEntity, trace_t* pTrace) = 0;
	virtual void TraceRay(const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void SetupLeafAndEntityListRay(const Ray_t& ray, CTraceListData& traceData) = 0;
	virtual void SetupLeafAndEntityListBox(const Vector& vecBoxMin, const Vector& vecBoxMax, CTraceListData& traceData) = 0;
	virtual void TraceRayAgainstLeafAndEntityList(const Ray_t& ray, CTraceListData& traceData, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void SweepCollideable(ICollideable* pCollide, const Vector& vecAbsStart, const Vector& vecAbsEnd, const QAngle& vecAngles, unsigned int fMask, ITraceFilter* pTraceFilter, trace_t* pTrace) = 0;
	virtual void EnumerateEntities(const Ray_t& ray, bool triggers, IEntityEnumerator* pEnumerator) = 0;
	virtual void EnumerateEntities(const Vector& vecAbsMins, const Vector& vecAbsMaxs, IEntityEnumerator* pEnumerator) = 0;
	virtual void HandleEntityToCollideable(IHandleEntity* pHandleEntity, ICollideable** ppCollide, const char** ppDebugName) = 0;
	virtual ICollideable* GetWorldCollideable() = 0;
	virtual void ClipRayToCollideable(const Ray_t& ray, unsigned int fMask, ICollideable* pEntity, trace_t* pTrace) = 0;
	virtual int GetStatByIndex(int index, bool bClear) = 0;
	virtual void GetBrushesInAABB(const Vector& vMins, const Vector& vMaxs, CUtlVector<int>* pOutput, int iContentsMask = 0xFFFFFFFF) = 0;
	virtual CPhysCollide* GetCollidableFromDisplacementsInAABB(const Vector& vMins, const Vector& vMaxs) = 0;
	virtual bool GetBrushInfo(int iBrush, CUtlVector<Vector4D>* pPlanesOut, int* pContentsOut) = 0;
	virtual bool PointOutsideWorld(const Vector& ptTest) = 0;
	virtual int GetLeafContainingPoint(const Vector& ptTest) = 0;
	virtual void SetTraceEntity(ICollideable* pCollideable, trace_t* pTrace) = 0;
	virtual ICollideable* GetCollideable(IHandleEntity* pEntity) = 0;
	virtual int SpatialPartitionMask() const = 0;
	virtual int SpatialPartitionTriggerMask() const = 0;

	int m_traceStatCounters[NUM_TRACE_STAT_COUNTER];
	const matrix3x4* m_pRootMoveParent;
};

MAKE_INTERFACE_VERSION(CEngineTrace, EngineTrace, "engine.dll", "EngineTraceClient003");