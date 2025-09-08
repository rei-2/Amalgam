#include "TraceFilters.h"

#include "../../SDK.h"

bool CTraceFilterHitscan::ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask)
{
	if (!pServerEntity || pServerEntity == pSkip)
		return false;

	auto pEntity = reinterpret_cast<CBaseEntity*>(pServerEntity);
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
	case ETFClassID::CTFMedigunShield: return pEntity->m_iTeamNum() != iTeam;
	case ETFClassID::CTFPlayer:
	case ETFClassID::CBaseObject:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter: 
	{
		if (iType != SKIP_CHECK && (iWeapon == WEAPON_INCLUDE ? bWeapon : !bWeapon))
			return iType == FORCE_HIT ? true : false;
		return pEntity->m_iTeamNum() != iTeam;
	}
	}

	return true;
}
TraceType_t CTraceFilterHitscan::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

bool CTraceFilterCollideable::ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask)
{
	if (!pServerEntity || pServerEntity == pSkip)
		return false;

	auto pEntity = reinterpret_cast<CBaseEntity*>(pServerEntity);
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
	case ETFClassID::CBaseEntity:
	case ETFClassID::CBaseDoor:
	case ETFClassID::CDynamicProp:
	case ETFClassID::CPhysicsProp:
	case ETFClassID::CPhysicsPropMultiplayer:
	case ETFClassID::CFunc_LOD:
	case ETFClassID::CObjectCartDispenser:
	case ETFClassID::CFuncTrackTrain:
	case ETFClassID::CFuncConveyor:
	case ETFClassID::CTFGenericBomb:
	case ETFClassID::CTFPumpkinBomb: return true;
	case ETFClassID::CFuncRespawnRoomVisualizer:
		if (nContentsMask & CONTENTS_PLAYERCLIP)
			return pEntity->m_iTeamNum() != iTeam;
		break;
	case ETFClassID::CTFMedigunShield:
		if (!(nContentsMask & CONTENTS_PLAYERCLIP))
			return pEntity->m_iTeamNum() != iTeam;
		break;
	case ETFClassID::CTFPlayer:
	{
		if (iPlayer == PLAYER_ALL)
			return true;
		if (iPlayer == PLAYER_NONE)
			return false;
		if (iType != SKIP_CHECK && (iWeapon == WEAPON_INCLUDE ? bWeapon : !bWeapon))
			return iType == FORCE_HIT ? true : false;
		return pEntity->m_iTeamNum() != iTeam;
	}
	case ETFClassID::CBaseObject:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser: return iObject == OBJECT_ALL ? true : iObject == OBJECT_NONE ? false : pEntity->m_iTeamNum() != iTeam;
	case ETFClassID::CObjectTeleporter: return true;
	//case ETFClassID::CTFBaseBoss:
	//case ETFClassID::CTFTankBoss:
	//case ETFClassID::CMerasmus:
	//case ETFClassID::CEyeballBoss:
	//case ETFClassID::CHeadlessHatman:
	//case ETFClassID::CZombie:
	case ETFClassID::CTFGrenadePipebombProjectile:
		return bMisc ? true : false;
	}

	return false;
}
TraceType_t CTraceFilterCollideable::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

bool CTraceFilterWorldAndPropsOnly::ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask)
{
	if (!pServerEntity || pServerEntity == pSkip)
		return false;
	if (pServerEntity->GetRefEHandle().GetSerialNumber() == (1 << 15))
		return I::ClientEntityList->GetClientEntity(0) != pSkip;

	auto pEntity = reinterpret_cast<CBaseEntity*>(pServerEntity);
	if (iTeam == -1) iTeam = pSkip ? pSkip->m_iTeamNum() : 0;

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CBaseEntity:
	case ETFClassID::CBaseDoor:
	case ETFClassID::CDynamicProp:
	case ETFClassID::CPhysicsProp:
	case ETFClassID::CPhysicsPropMultiplayer:
	case ETFClassID::CFunc_LOD:
	case ETFClassID::CObjectCartDispenser:
	case ETFClassID::CFuncTrackTrain:
	case ETFClassID::CFuncConveyor: return true;
	case ETFClassID::CFuncRespawnRoomVisualizer:
		if (nContentsMask & CONTENTS_PLAYERCLIP)
			return pEntity->m_iTeamNum() != iTeam;
	}

	return false;
}
TraceType_t CTraceFilterWorldAndPropsOnly::GetTraceType() const
{
	return TRACE_EVERYTHING_FILTER_PROPS;
}