#include "TraceFilters.h"
#include "../../SDK.h"

bool CTraceFilterHitscan::ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask)
{
	if (!pServerEntity || pServerEntity == pSkip)
		return false;

	auto pEntity = static_cast<IClientEntity*>(pServerEntity)->As<CBaseEntity>();
	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();

	const int iTargetTeam = pEntity->m_iTeamNum(), iLocalTeam = pLocal ? pLocal->m_iTeamNum() : iTargetTeam;
	bool bSniperRifle = false;
	if (pLocal && pLocal == pSkip && pWeapon)
	{
		switch (pWeapon->m_iWeaponID())
		{
		case TF_WEAPON_SNIPERRIFLE:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		case TF_WEAPON_SNIPERRIFLE_DECAP:
			bSniperRifle = true;
		}
	}

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFAmmoPack:
	case ETFClassID::CFuncAreaPortalWindow:
	case ETFClassID::CFuncRespawnRoomVisualizer:
	case ETFClassID::CSniperDot:
	case ETFClassID::CTFReviveMarker: return false;
	case ETFClassID::CTFMedigunShield:
		if (iTargetTeam == iLocalTeam)
			return false;
		break;
	case ETFClassID::CTFPlayer:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
		if (bSniperRifle && iTargetTeam == iLocalTeam)
			return false;
	}

	return true;
}
TraceType_t CTraceFilterHitscan::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

bool CTraceFilterProjectile::ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask)
{
	if (!pServerEntity || pServerEntity == pSkip)
		return false;

	auto pEntity = static_cast<IClientEntity*>(pServerEntity)->As<CBaseEntity>();
	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();

	const int iTargetTeam = pEntity->m_iTeamNum(), iLocalTeam = pLocal ? pLocal->m_iTeamNum() : iTargetTeam;
	const bool bCrossbow = (pLocal && pLocal == pSkip && pWeapon) ? pWeapon->m_iWeaponID() == TF_WEAPON_CROSSBOW : false;

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CBaseEntity:
	case ETFClassID::CBaseDoor:
	case ETFClassID::CDynamicProp:
	case ETFClassID::CPhysicsProp:
	case ETFClassID::CObjectCartDispenser:
	case ETFClassID::CFuncTrackTrain:
	case ETFClassID::CFuncConveyor:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter: return true;
	case ETFClassID::CTFPlayer: return bCrossbow ? true : iTargetTeam != iLocalTeam;
	}

	return false;
}
TraceType_t CTraceFilterProjectile::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

bool CTraceFilterWorldAndPropsOnly::ShouldHitEntity(IHandleEntity* pServerEntity, int nContentsMask)
{
	if (!pServerEntity || pServerEntity == pSkip)
		return false;

	auto pEntity = static_cast<IClientEntity*>(pServerEntity)->As<CBaseEntity>();
	switch (pEntity->GetClassID())
	{
	case ETFClassID::CBaseEntity:
	case ETFClassID::CBaseDoor:
	case ETFClassID::CDynamicProp:
	case ETFClassID::CPhysicsProp:
	case ETFClassID::CObjectCartDispenser:
	case ETFClassID::CFuncTrackTrain:
	case ETFClassID::CFuncConveyor: return true;
	}

	return false;
}
TraceType_t CTraceFilterWorldAndPropsOnly::GetTraceType() const
{
	return TRACE_EVERYTHING;
}