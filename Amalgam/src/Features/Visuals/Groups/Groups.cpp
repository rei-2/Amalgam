#include "Groups.h"

#include "../../Players/PlayerUtils.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"

bool CGroups::ShouldTargetTeam(bool bType, Group_t& tGroup, CBaseEntity* pEntity, CTFPlayer* pLocal)
{
	if (!bType)
		return false;

	if (tGroup.m_iConditions & ConditionsEnum::Relative)
	{
		if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
			return tGroup.m_iConditions & ConditionsEnum::Enemy;
		else
			return tGroup.m_iConditions & ConditionsEnum::Team;
	}
	else
	{
		if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum()
			? !(tGroup.m_iConditions & ConditionsEnum::Enemy)
			: !(tGroup.m_iConditions & ConditionsEnum::Team))
		{
			return false;
		}

		switch (pEntity->m_iTeamNum())
		{
		case TF_TEAM_BLUE:
			return tGroup.m_iConditions & ConditionsEnum::BLU;
		case TF_TEAM_RED:
			return tGroup.m_iConditions & ConditionsEnum::RED;
		}
	}
	return false;
}

bool CGroups::ShouldTargetOwner(bool bType, Group_t& tGroup, CBaseEntity* pOwner, CBaseEntity* pEntity, CTFPlayer* pLocal)
{
	if (!bType)
		return false;

	if (tGroup.m_iConditions & ConditionsEnum::Local && pOwner == pLocal
		|| tGroup.m_iConditions & ConditionsEnum::Friends && H::Entities.IsFriend(pOwner->entindex())
		|| tGroup.m_iConditions & ConditionsEnum::Party && H::Entities.InParty(pOwner->entindex())
		|| tGroup.m_iConditions & ConditionsEnum::Priority && F::PlayerUtils.IsPrioritized(pOwner->entindex())
		|| tGroup.m_iConditions & ConditionsEnum::Target && pEntity->entindex() == G::AimTarget.m_iEntIndex)
	{
		return true;
	}

	return ShouldTargetTeam(bType, tGroup, pEntity, pLocal);
}

bool CGroups::ShouldTarget(Group_t& tGroup, CBaseEntity* pEntity, CTFPlayer* pLocal)
{
	switch (pEntity->GetClassID())
	{
	// players
	case ETFClassID::CTFPlayer:
		return ShouldTargetOwner(tGroup.m_iTargets & TargetsEnum::Players, tGroup, pEntity, pEntity, pLocal);
	// buildings
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	{
		auto pOwner = pEntity->As<CBaseObject>()->m_hBuilder().Get();
		if (!pOwner) pOwner = pEntity;
		return ShouldTargetOwner(tGroup.m_iTargets & TargetsEnum::Buildings, tGroup, pOwner, pEntity, pLocal);
	}
	// projectiles
	case ETFClassID::CBaseProjectile:
	case ETFClassID::CBaseGrenade:
	case ETFClassID::CTFWeaponBaseGrenadeProj:
	case ETFClassID::CTFWeaponBaseMerasmusGrenade:
	case ETFClassID::CTFGrenadePipebombProjectile:
	case ETFClassID::CTFStunBall:
	case ETFClassID::CTFBall_Ornament:
	case ETFClassID::CTFProjectile_Jar:
	case ETFClassID::CTFProjectile_Cleaver:
	case ETFClassID::CTFProjectile_JarGas:
	case ETFClassID::CTFProjectile_JarMilk:
	case ETFClassID::CTFProjectile_SpellBats:
	case ETFClassID::CTFProjectile_SpellKartBats:
	case ETFClassID::CTFProjectile_SpellMeteorShower:
	case ETFClassID::CTFProjectile_SpellMirv:
	case ETFClassID::CTFProjectile_SpellPumpkin:
	case ETFClassID::CTFProjectile_SpellSpawnBoss:
	case ETFClassID::CTFProjectile_SpellSpawnHorde:
	case ETFClassID::CTFProjectile_SpellSpawnZombie:
	case ETFClassID::CTFProjectile_SpellTransposeTeleport:
	case ETFClassID::CTFProjectile_Throwable:
	case ETFClassID::CTFProjectile_ThrowableBreadMonster:
	case ETFClassID::CTFProjectile_ThrowableBrick:
	case ETFClassID::CTFProjectile_ThrowableRepel:
	case ETFClassID::CTFBaseRocket:
	case ETFClassID::CTFFlameRocket:
	case ETFClassID::CTFProjectile_Arrow:
	case ETFClassID::CTFProjectile_GrapplingHook:
	case ETFClassID::CTFProjectile_HealingBolt:
	case ETFClassID::CTFProjectile_Rocket:
	case ETFClassID::CTFProjectile_BallOfFire:
	case ETFClassID::CTFProjectile_MechanicalArmOrb:
	case ETFClassID::CTFProjectile_SentryRocket:
	case ETFClassID::CTFProjectile_SpellFireball:
	case ETFClassID::CTFProjectile_SpellLightningOrb:
	case ETFClassID::CTFProjectile_SpellKartOrb:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_Flare:
	case ETFClassID::CTFBaseProjectile:
	case ETFClassID::CTFProjectile_EnergyRing:
	//case ETFClassID::CTFProjectile_Syringe:
	{
		auto pOwner = F::ProjSim.GetEntities(pEntity).second->As<CBaseEntity>();
		if (!pOwner) pOwner = pEntity;
		return ShouldTargetOwner(tGroup.m_iTargets & TargetsEnum::Projectiles, tGroup, pOwner, pEntity, pLocal);
	}
	// ragdolls
	case ETFClassID::CTFRagdoll:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CRagdollPropAttached:
	{
		auto pOwner = pEntity->As<CTFRagdoll>()->m_hPlayer().Get();
		if (!pOwner) pOwner = pEntity;
		return ShouldTargetOwner(tGroup.m_iTargets & TargetsEnum::Ragdolls, tGroup, pOwner, pEntity, pLocal);
	}
	// objectives
	case ETFClassID::CCaptureFlag:
		return ShouldTargetTeam(tGroup.m_iTargets & TargetsEnum::Objective, tGroup, pEntity, pLocal);
	// npcs
	case ETFClassID::CEyeballBoss:
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CMerasmus:
	case ETFClassID::CTFBaseBoss:
	case ETFClassID::CTFTankBoss:
	case ETFClassID::CZombie:
		if (pEntity->IsInValidTeam())
		{
			if (auto pOwner = pEntity->m_hOwnerEntity().Get())
				return ShouldTargetOwner(tGroup.m_iTargets & TargetsEnum::NPCs, tGroup, pOwner, pEntity, pLocal);
			return ShouldTargetTeam(tGroup.m_iTargets & TargetsEnum::NPCs, tGroup, pEntity, pLocal);
		}
		return tGroup.m_iTargets & TargetsEnum::NPCs;
	// pickups
	case ETFClassID::CBaseAnimating:
	{
		if (H::Entities.IsHealth(H::Entities.GetModel(pEntity->entindex())))
			return tGroup.m_iTargets & TargetsEnum::Health;
		else if (H::Entities.IsAmmo(H::Entities.GetModel(pEntity->entindex())))
			return tGroup.m_iTargets & TargetsEnum::Ammo;
		else if (H::Entities.IsPowerup(H::Entities.GetModel(pEntity->entindex())))
			return tGroup.m_iTargets & TargetsEnum::Powerups;
		else if (H::Entities.IsSpellbook(H::Entities.GetModel(pEntity->entindex())))
			return tGroup.m_iTargets & TargetsEnum::Spellbook;
		break;
	}
	case ETFClassID::CTFAmmoPack:
		return tGroup.m_iTargets & TargetsEnum::Ammo;
	case ETFClassID::CCurrencyPack:
		return tGroup.m_iTargets & TargetsEnum::Money;
	case ETFClassID::CHalloweenGiftPickup:
		return tGroup.m_iTargets & TargetsEnum::Gargoyle;
	// bombs
	case ETFClassID::CTFPumpkinBomb:
	case ETFClassID::CTFGenericBomb:
		return tGroup.m_iTargets & TargetsEnum::Bombs;
	case ETFClassID::CTFMedigunShield:
		return false;
	}

	// players
	auto pOwner = pEntity->m_hOwnerEntity().Get();
	if (pOwner && pOwner->IsPlayer())
		return ShouldTargetOwner(tGroup.m_iTargets & TargetsEnum::Players, tGroup, pOwner, pOwner, pLocal);

	return false;
}