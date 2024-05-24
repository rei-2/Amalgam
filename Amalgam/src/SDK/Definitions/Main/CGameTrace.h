#pragma once
#include "CModel.h"
#include "UtlVector.h"
#include "IHandleEntity.h"
#include "../Misc/ISpatialPartition.h"

class CBaseEntity;

class CGameTrace : public CBaseTrace
{
public:
	bool DidHit() const;

	float fractionleftsolid{};
	csurface_t surface{};
	int hitgroup{};
	short physicsbone{};
	CBaseEntity* m_pEnt{};
	int hitbox{};
};

inline bool CGameTrace::DidHit() const
{
	return fraction < 1.f || allsolid || startsolid;
}

typedef CGameTrace trace_t;