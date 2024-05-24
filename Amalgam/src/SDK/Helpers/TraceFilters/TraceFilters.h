#pragma once
#include "../../Definitions/Interfaces/IEngineTrace.h"

class CTraceFilterHitscan : public ITraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask) override;
	TraceType_t GetTraceType() const override;
	CBaseEntity* pSkip = nullptr;
};

class CTraceFilterProjectile : public ITraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask) override;
	TraceType_t GetTraceType() const override;
	CBaseEntity* pSkip = nullptr;
};

class CTraceFilterWorldAndPropsOnly : public ITraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask) override;
	TraceType_t GetTraceType() const override;
	CBaseEntity* pSkip = nullptr;
};