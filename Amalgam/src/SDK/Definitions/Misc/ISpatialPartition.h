#pragma once
#include "../Definitions.h"
#include "../Types.h"

struct Ray_t;
class IHandleEntity;

#define PARTITION_ALL_CLIENT_EDICTS	(PARTITION_CLIENT_NON_STATIC_EDICTS | PARTITION_CLIENT_STATIC_PROPS | PARTITION_CLIENT_RESPONSIVE_EDICTS |	PARTITION_CLIENT_SOLID_EDICTS)

#define PARTITION_CLIENT_GAME_EDICTS (PARTITION_ALL_CLIENT_EDICTS & ~PARTITION_CLIENT_STATIC_PROPS)
#define PARTITION_SERVER_GAME_EDICTS (PARTITION_ENGINE_SOLID_EDICTS|PARTITION_ENGINE_TRIGGER_EDICTS|PARTITION_ENGINE_NON_STATIC_EDICTS)

enum IterationRetval_t
{
	ITERATION_CONTINUE = 0,
	ITERATION_STOP
};

typedef unsigned short SpatialPartitionHandle_t;
typedef int SpatialPartitionListMask_t;
typedef int SpatialTempHandle_t;

class IPartitionEnumerator
{
public:
	virtual IterationRetval_t EnumElement(IHandleEntity* pHandleEntity) = 0;
};

class IPartitionQueryCallback
{
public:
	virtual void OnPreQuery_V1() = 0;
	virtual void OnPreQuery(SpatialPartitionListMask_t listMask) = 0;
	virtual void OnPostQuery(SpatialPartitionListMask_t listMask) = 0;
};

enum
{
	PARTITION_INVALID_HANDLE = (SpatialPartitionHandle_t)~0
};

class ISpatialPartition
{
public:
	virtual ~ISpatialPartition() {}
	virtual SpatialPartitionHandle_t CreateHandle(IHandleEntity* pHandleEntity) = 0;
	virtual SpatialPartitionHandle_t CreateHandle(IHandleEntity* pHandleEntity, SpatialPartitionListMask_t listMask, const Vector& mins, const Vector& maxs) = 0;
	virtual void DestroyHandle(SpatialPartitionHandle_t handle) = 0;
	virtual void Insert(SpatialPartitionListMask_t listMask, SpatialPartitionHandle_t handle) = 0;
	virtual void Remove(SpatialPartitionListMask_t listMask, SpatialPartitionHandle_t handle) = 0;
	virtual void RemoveAndInsert(SpatialPartitionListMask_t removeMask, SpatialPartitionListMask_t insertMask, SpatialPartitionHandle_t handle) = 0;
	virtual void Remove(SpatialPartitionHandle_t handle) = 0;
	virtual void ElementMoved(SpatialPartitionHandle_t handle, const Vector& mins, const Vector& maxs) = 0;
	virtual SpatialTempHandle_t HideElement(SpatialPartitionHandle_t handle) = 0;
	virtual void UnhideElement(SpatialPartitionHandle_t handle, SpatialTempHandle_t tempHandle) = 0;
	virtual void InstallQueryCallback_V1(IPartitionQueryCallback* pCallback) = 0;
	virtual void RemoveQueryCallback(IPartitionQueryCallback* pCallback) = 0;

	virtual void EnumerateElementsInBox(
		SpatialPartitionListMask_t listMask,
		const Vector& mins,
		const Vector& maxs,
		bool coarseTest,
		IPartitionEnumerator* pIterator
	) = 0;

	virtual void EnumerateElementsInSphere(
		SpatialPartitionListMask_t listMask,
		const Vector& origin,
		float radius,
		bool coarseTest,
		IPartitionEnumerator* pIterator
	) = 0;

	virtual void EnumerateElementsAlongRay(
		SpatialPartitionListMask_t listMask,
		const Ray_t& ray,
		bool coarseTest,
		IPartitionEnumerator* pIterator
	) = 0;

	virtual void EnumerateElementsAtPoint(
		SpatialPartitionListMask_t listMask,
		const Vector& pt,
		bool coarseTest,
		IPartitionEnumerator* pIterator
	) = 0;

	virtual void SuppressLists(SpatialPartitionListMask_t nListMask, bool bSuppress) = 0;
	virtual SpatialPartitionListMask_t GetSuppressedLists() = 0;
	virtual void RenderAllObjectsInTree(float flTime) = 0;
	virtual void RenderObjectsInPlayerLeafs(const Vector& vecPlayerMin, const Vector& vecPlayerMax, float flTime) = 0;
	virtual void RenderLeafsForRayTraceStart(float flTime) = 0;
	virtual void RenderLeafsForRayTraceEnd(void) = 0;
	virtual void RenderLeafsForHullTraceStart(float flTime) = 0;
	virtual void RenderLeafsForHullTraceEnd(void) = 0;
	virtual void RenderLeafsForBoxStart(float flTime) = 0;
	virtual void RenderLeafsForBoxEnd(void) = 0;
	virtual void RenderLeafsForSphereStart(float flTime) = 0;
	virtual void RenderLeafsForSphereEnd(void) = 0;
	virtual void RenderObjectsInBox(const Vector& vecMin, const Vector& vecMax, float flTime) = 0;
	virtual void RenderObjectsInSphere(const Vector& vecCenter, float flRadius, float flTime) = 0;
	virtual void RenderObjectsAlongRay(const Ray_t& ray, float flTime) = 0;
	virtual void ReportStats(const char* pFileName) = 0;
	virtual void InstallQueryCallback(IPartitionQueryCallback* pCallback) = 0;
};