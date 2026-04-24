#include "TraceFilters.h"

#include "../../SDK.h"

bool CTraceFilterHitscan::ShouldHitEntity(IHandleEntity* pHandleEntity, int nContentsMask)
{
	if (!pHandleEntity || pHandleEntity == pSkip)
		return false;

	auto pEntity = reinterpret_cast<CBaseEntity*>(pHandleEntity);
	if (iTeam == -1) iTeam = pSkip ? pSkip->m_iTeamNum() : 0;
	if (iType != SKIP_CHECK && !vWeapons.empty())
	{
		if (auto pWeapon = pSkip && pSkip->IsPlayer() ? pSkip->As<CTFPlayer>()->m_hActiveWeapon()->As<CTFWeaponBase>() : nullptr)
		{
			int iWeaponID = pWeapon->GetWeaponID();
			bWeapon = std::find(vWeapons.begin(), vWeapons.end(), iWeaponID) != vWeapons.end();
		}
		vWeapons.clear();
	}

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFAmmoPack:
	case ETFClassID::CFuncAreaPortalWindow:
	case ETFClassID::CFuncRespawnRoomVisualizer:
	case ETFClassID::CTFReviveMarker: return false;
	case ETFClassID::CTFMedigunShield: return !(nContentsMask & CONTENTS_PLAYERCLIP) && pEntity->m_iTeamNum() != iTeam;
	case ETFClassID::CTFPlayer:
	case ETFClassID::CBaseObject:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
		if (!(nContentsMask & CONTENTS_MOVEABLE)) return false;
		if (iType != SKIP_CHECK && (iWeapon == WEAPON_INCLUDE ? bWeapon : !bWeapon)) return iType == FORCE_HIT ? true : false;
		return pEntity->m_iTeamNum() != iTeam;
	}

	return nContentsMask & CONTENTS_SOLID;
}
TraceType_t CTraceFilterHitscan::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

bool CTraceFilterCollideable::ShouldHitEntity(IHandleEntity* pHandleEntity, int nContentsMask)
{
	if (!pHandleEntity || pHandleEntity == pSkip)
		return false;

	auto pEntity = reinterpret_cast<CBaseEntity*>(pHandleEntity);
	if (iTeam == -1) iTeam = pSkip ? pSkip->m_iTeamNum() : 0;
	if (iType != SKIP_CHECK && !vWeapons.empty())
	{
		if (auto pWeapon = pSkip && pSkip->IsPlayer() ? pSkip->As<CTFPlayer>()->m_hActiveWeapon()->As<CTFWeaponBase>() : nullptr)
		{
			int iWeaponID = pWeapon->GetWeaponID();
			bWeapon = std::find(vWeapons.begin(), vWeapons.end(), iWeaponID) != vWeapons.end();
		}
		vWeapons.clear();
	}

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CBaseEntity: return nContentsMask & CONTENTS_SOLID;
	case ETFClassID::CFunc_LOD:
	case ETFClassID::CBaseDoor:
	case ETFClassID::CDynamicProp:
	case ETFClassID::CPhysicsProp:
	case ETFClassID::CPhysicsPropMultiplayer:
	case ETFClassID::CObjectCartDispenser:
	case ETFClassID::CFuncTrackTrain:
	case ETFClassID::CFuncConveyor:
	case ETFClassID::CTFGenericBomb:
	case ETFClassID::CTFPumpkinBomb: return nContentsMask & CONTENTS_MOVEABLE;
	case ETFClassID::CFuncRespawnRoomVisualizer: return nContentsMask & CONTENTS_PLAYERCLIP && pEntity->m_iTeamNum() != iTeam;
	case ETFClassID::CTFMedigunShield: return !(nContentsMask & CONTENTS_PLAYERCLIP) && pEntity->m_iTeamNum() != iTeam;
	case ETFClassID::CTFPlayer:
		if (!(nContentsMask & CONTENTS_MONSTER)) return false;
		if (iPlayer == PLAYER_ALL) return true;
		if (iPlayer == PLAYER_NONE) return false;
		if (iType != SKIP_CHECK && (iWeapon == WEAPON_INCLUDE ? bWeapon : !bWeapon)) return iType == FORCE_HIT ? true : false;
		return pEntity->m_iTeamNum() != iTeam;
	case ETFClassID::CBaseObject:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser: return nContentsMask & CONTENTS_MOVEABLE && (iObject == OBJECT_ALL ? true : iObject == OBJECT_NONE ? false : pEntity->m_iTeamNum() != iTeam);
	case ETFClassID::CObjectTeleporter: return nContentsMask & CONTENTS_MOVEABLE;
	//case ETFClassID::CTFBaseBoss:
	//case ETFClassID::CTFTankBoss:
	//case ETFClassID::CMerasmus:
	//case ETFClassID::CEyeballBoss:
	//case ETFClassID::CHeadlessHatman:
	//case ETFClassID::CZombie: return nContentsMask & CONTENTS_MONSTER && bMisc;
	case ETFClassID::CTFGrenadePipebombProjectile: return nContentsMask & CONTENTS_MOVEABLE && bMisc && pEntity->As<CTFGrenadePipebombProjectile>()->m_iType() == TF_GL_MODE_REMOTE_DETONATE;
	}

	return false;
}
TraceType_t CTraceFilterCollideable::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

bool CTraceFilterWorldAndPropsOnly::ShouldHitEntity(IHandleEntity* pHandleEntity, int nContentsMask)
{
	if (!pHandleEntity || pHandleEntity == pSkip)
		return false;
	if (pHandleEntity->GetRefEHandle().GetSerialNumber() == (1 << 15))
		return nContentsMask & CONTENTS_SOLID && pHandleEntity->GetRefEHandle().GetEntryIndex() != iTeam; // team variable since cliententitylist can give nullptrs

	auto pEntity = reinterpret_cast<CBaseEntity*>(pHandleEntity);
	if (iTeam == -1) iTeam = pSkip ? pSkip->m_iTeamNum() : 0;

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CBaseEntity: return nContentsMask & CONTENTS_SOLID;
	case ETFClassID::CFunc_LOD:
	case ETFClassID::CBaseDoor:
	case ETFClassID::CDynamicProp:
	case ETFClassID::CPhysicsProp:
	case ETFClassID::CPhysicsPropMultiplayer:
	case ETFClassID::CObjectCartDispenser:
	case ETFClassID::CFuncTrackTrain:
	case ETFClassID::CFuncConveyor: return nContentsMask & CONTENTS_MOVEABLE;
	case ETFClassID::CFuncRespawnRoomVisualizer: return nContentsMask & CONTENTS_PLAYERCLIP && pEntity->m_iTeamNum() != iTeam;
	}

	return false;
}
TraceType_t CTraceFilterWorldAndPropsOnly::GetTraceType() const
{
	return TRACE_EVERYTHING_FILTER_PROPS;
}