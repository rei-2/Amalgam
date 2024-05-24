#pragma once
#include "CBaseHandle.h"
#include "IClientEntity.h"
#include "CCollisionProperty.h"
#include "../Main/UtlVector.h"
#include "../Definitions.h"
#include "../../../Utils/NetVars/NetVars.h"
#include "../../../Utils/Signatures/Signatures.h"
#include "../../../Utils/Memory/Memory.h"

MAKE_SIGNATURE(CBaseEntity_SetAbsOrigin, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 E8 ? ? ? ? F3 0F 10 83", 0x0);
MAKE_SIGNATURE(CBaseEntity_SetAbsAngles, "client.dll", "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 48 8B FA 48 8B D9 E8 ? ? ? ? F3 0F 10 83", 0x0);
MAKE_SIGNATURE(CBaseEntity_SetAbsVelocity, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? F3 0F 10 81 ? ? ? ? 48 8B DA 0F 2E 02", 0x0);
MAKE_SIGNATURE(CBaseEntity_EstimateAbsVelocity, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 E8 ? ? ? ? 48 3B D8", 0x0);
MAKE_SIGNATURE(CBaseEntity_CreateShadow, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B 41 ? 48 8B F9 48 83 C1 ? FF 90", 0x0);
MAKE_SIGNATURE(CBaseEntity_InvalidateBoneCache, "client.dll", "8B 05 ? ? ? ? FF C8 C7 81", 0x0);

enum CollideType_t
{
	ENTITY_SHOULD_NOT_COLLIDE = 0,
	ENTITY_SHOULD_COLLIDE,
	ENTITY_SHOULD_RESPOND
};

typedef CHandle<CBaseEntity> EHANDLE;

#define MULTIPLAYER_BACKUP 90

class IInterpolatedVar;

class VarMapEntry_t
{
public:
	unsigned short type;
	unsigned short m_bNeedsToInterpolate;
	void* data;
	IInterpolatedVar* watcher;
};

struct VarMapping_t
{
	CUtlVector<VarMapEntry_t> m_Entries;
	int m_nInterpolatedEntries;
	float m_lastInterpolationTime;
};

class CBaseEntity : public IClientEntity
{
public:
	NETVAR(m_flAnimTime, float, "CBaseEntity", "m_flAnimTime");
	NETVAR(m_flSimulationTime, float, "CBaseEntity", "m_flSimulationTime");
	NETVAR(m_ubInterpolationFrame, int, "CBaseEntity", "m_ubInterpolationFrame");
	NETVAR(m_vecOrigin, Vec3, "CBaseEntity", "m_vecOrigin");
	NETVAR(m_angRotation, Vec3, "CBaseEntity", "m_angRotation");
	NETVAR(m_nModelIndex, int, "CBaseEntity", "m_nModelIndex");
	NETVAR(m_fEffects, int, "CBaseEntity", "m_fEffects");
	NETVAR(m_nRenderMode, int, "CBaseEntity", "m_nRenderMode");
	NETVAR(m_nRenderFX, int, "CBaseEntity", "m_nRenderFX");
	NETVAR(m_clrRender, Color_t, "CBaseEntity", "m_clrRender");
	NETVAR(m_iTeamNum, int, "CBaseEntity", "m_iTeamNum");
	NETVAR(m_CollisionGroup, int, "CBaseEntity", "m_CollisionGroup");
	NETVAR(m_flElasticity, float, "CBaseEntity", "m_flElasticity");
	NETVAR(m_flShadowCastDistance, float, "CBaseEntity", "m_flShadowCastDistance");
	NETVAR(m_hOwnerEntity, EHANDLE, "CBaseEntity", "m_hOwnerEntity");
	NETVAR(m_hEffectEntity, EHANDLE, "CBaseEntity", "m_hEffectEntity");
	NETVAR(moveparent, int, "CBaseEntity", "moveparent");
	NETVAR(m_iParentAttachment, int, "CBaseEntity", "m_iParentAttachment");
	NETVAR(m_Collision, CCollisionProperty*, "CBaseEntity", "m_Collision");
	NETVAR(m_vecMinsPreScaled, Vec3, "CBaseEntity", "m_vecMinsPreScaled");
	NETVAR(m_vecMaxsPreScaled, Vec3, "CBaseEntity", "m_vecMaxsPreScaled");
	NETVAR(m_vecMins, Vec3, "CBaseEntity", "m_vecMins");
	NETVAR(m_vecMaxs, Vec3, "CBaseEntity", "m_vecMaxs");
	NETVAR(m_nSolidType, int, "CBaseEntity", "m_nSolidType");
	NETVAR(m_usSolidFlags, int, "CBaseEntity", "m_usSolidFlags");
	NETVAR(m_nSurroundType, int, "CBaseEntity", "m_nSurroundType");
	NETVAR(m_triggerBloat, int, "CBaseEntity", "m_triggerBloat");
	NETVAR(m_bUniformTriggerBloat, bool, "CBaseEntity", "m_bUniformTriggerBloat");
	NETVAR(m_vecSpecifiedSurroundingMinsPreScaled, Vec3, "CBaseEntity", "m_vecSpecifiedSurroundingMinsPreScaled");
	NETVAR(m_vecSpecifiedSurroundingMaxsPreScaled, Vec3, "CBaseEntity", "m_vecSpecifiedSurroundingMaxsPreScaled");
	NETVAR(m_vecSpecifiedSurroundingMins, Vec3, "CBaseEntity", "m_vecSpecifiedSurroundingMins");
	NETVAR(m_vecSpecifiedSurroundingMaxs, Vec3, "CBaseEntity", "m_vecSpecifiedSurroundingMaxs");
	NETVAR(m_iTextureFrameIndex, int, "CBaseEntity", "m_iTextureFrameIndex");
	NETVAR(m_PredictableID, int, "CBaseEntity", "m_PredictableID");
	NETVAR(m_bIsPlayerSimulated, bool, "CBaseEntity", "m_bIsPlayerSimulated");
	NETVAR(m_bSimulatedEveryTick, bool, "CBaseEntity", "m_bSimulatedEveryTick");
	NETVAR(m_bAnimatedEveryTick, bool, "CBaseEntity", "m_bAnimatedEveryTick");
	NETVAR(m_bAlternateSorting, bool, "CBaseEntity", "m_bAlternateSorting");
	NETVAR(m_nModelIndexOverrides, void*, "CBaseEntity", "m_nModelIndexOverrides");
	NETVAR(movetype, int, "CBaseEntity", "movetype");
	
	NETVAR_OFF(m_flOldSimulationTime, float, "CBaseEntity", "m_flSimulationTime", 4);

	VIRTUAL(UpdateVisibility, void, void(__thiscall*)(CBaseEntity*), this, 91);

	Vec3 GetCenter()
	{
		return m_vecOrigin() + Vec3(0, 0, (m_vecMins().z + m_vecMaxs().z) / 2);
	}

	Vec3 GetRenderCenter()
	{
		Vec3 vMin = {}, vMax = {};
		GetRenderBounds(vMin, vMax);
		return GetRenderOrigin() + Vec3(0.f, 0.f, (vMin.z + vMax.z) / 2);
	}

	bool IsInValidTeam(int* pTeamNumOut = nullptr)
	{
		int nTeamNum = m_iTeamNum();

		switch (nTeamNum)
		{
		case TF_TEAM_RED:
		case TF_TEAM_BLUE:
		{
			if (pTeamNumOut)
				*pTeamNumOut = nTeamNum;

			return true;
		}

		default: return false;
		}
	}

	CBaseEntity* GetMoveParent()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseEntity", "moveparent") - 8;
		auto m_pMoveParent = reinterpret_cast<EHANDLE*>(std::uintptr_t(this) + nOffset);

		if (!m_pMoveParent)
			return nullptr;

		return m_pMoveParent->Get();
	}

	CBaseEntity* NextMovePeer()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseEntity", "moveparent") - 16;
		auto m_pMovePeer = reinterpret_cast<EHANDLE*>(std::uintptr_t(this) + nOffset);

		if (!m_pMovePeer)
			return nullptr;

		return m_pMovePeer->Get();
	}

	CBaseEntity* FirstMoveChild()
	{
		static int nOffset = U::NetVars.GetNetVar("CBaseEntity", "moveparent") - 24;
		auto m_pMoveChild = reinterpret_cast<EHANDLE*>(std::uintptr_t(this) + nOffset);

		if (!m_pMoveChild)
			return nullptr;

		return m_pMoveChild->Get();
	}

	void SetAbsOrigin(const Vec3& absOrigin)
	{
		S::CBaseEntity_SetAbsOrigin.As<void(__fastcall*)(void*, const Vec3&)>()(this, absOrigin);
	}

	void SetAbsAngles(const Vec3& absAngles)
	{
		S::CBaseEntity_SetAbsAngles.As<void(__fastcall*)(void*, const Vec3&)>()(this, absAngles);
	}

	void SetAbsVelocity(const Vec3& vecAbsVelocity)
	{
		S::CBaseEntity_SetAbsVelocity.As<void(__thiscall*)(void*, const Vec3&)>()(this, vecAbsVelocity);
	}

	void EstimateAbsVelocity(Vec3& vel)
	{
		S::CBaseEntity_EstimateAbsVelocity.As<void(__thiscall*)(void*, Vec3&)>()(this, vel);
	}

	Vec3 GetAbsVelocity()
	{
		Vec3 vOut;
		EstimateAbsVelocity(vOut);
		return vOut;
	}

	void CreateShadow()
	{
		S::CBaseEntity_CreateShadow.As<void(__thiscall*)(void*)>()(this);
	}

	void InvalidateBoneCache()
	{
		S::CBaseEntity_InvalidateBoneCache.As<void(__fastcall*)(void*)>()(this);
	}
};