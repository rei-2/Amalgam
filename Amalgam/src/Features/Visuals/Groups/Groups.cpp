#include "Groups.h"

#include "../../Players/PlayerUtils.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"

static inline bool ShouldTargetTeam(Group_t& tGroup, int iBit, CBaseEntity* pEntity, CTFPlayer* pLocal)
{
	if (!(tGroup.m_iTargets & iBit))
		return false;

	if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum()
		? !(tGroup.m_iConditions & ConditionsEnum::Enemy)
		: !(tGroup.m_iConditions & ConditionsEnum::Team))
	{
		return false;
	}

	if (pEntity->m_iTeamNum() == TF_TEAM_BLUE && !(tGroup.m_iConditions & ConditionsEnum::BLU)
		|| pEntity->m_iTeamNum() == TF_TEAM_RED && !(tGroup.m_iConditions & ConditionsEnum::RED))
		return false;

	return true;
}

static inline bool ShouldTargetOwner(Group_t& tGroup, int iBit, CBaseEntity* pOwner, CBaseEntity* pEntity, CTFPlayer* pLocal)
{
	if (!(tGroup.m_iTargets & iBit))
		return false;

	if (tGroup.m_iConditions & ConditionsEnum::Local && pOwner == pLocal
		|| tGroup.m_iConditions & ConditionsEnum::Friends && H::Entities.IsFriend(pOwner->entindex())
		|| tGroup.m_iConditions & ConditionsEnum::Party && H::Entities.InParty(pOwner->entindex())
		|| tGroup.m_iConditions & ConditionsEnum::Priority && F::PlayerUtils.IsPrioritized(pOwner->entindex())
		|| tGroup.m_iConditions & ConditionsEnum::Target && pEntity->entindex() == G::AimTarget.m_iEntIndex)
		return true;

	return ShouldTargetTeam(tGroup, iBit, pEntity, pLocal);
}

static inline bool ShouldTargetPlayer(Group_t& tGroup, int iBit, CBaseEntity* pEntity, CTFPlayer* pLocal)
{
	if (!(tGroup.m_iTargets & iBit))
		return false;

	auto pPlayer = pEntity->As<CTFPlayer>();
	if (!pPlayer->IsAlive() || pPlayer->IsAGhost())
		return false;

	if (tGroup.m_iPlayers)
	{
		if (tGroup.m_iPlayers & PlayerEnum::Classes)
		{
			int iFlag = 0;
			switch (pPlayer->m_iClass())
			{
			case TF_CLASS_SCOUT: iFlag = PlayerEnum::Scout; break;
			case TF_CLASS_SOLDIER: iFlag = PlayerEnum::Soldier; break;
			case TF_CLASS_PYRO: iFlag = PlayerEnum::Pyro; break;
			case TF_CLASS_DEMOMAN: iFlag = PlayerEnum::Demoman; break;
			case TF_CLASS_HEAVY: iFlag = PlayerEnum::Heavy; break;
			case TF_CLASS_ENGINEER: iFlag = PlayerEnum::Engineer; break;
			case TF_CLASS_MEDIC: iFlag = PlayerEnum::Medic; break;
			case TF_CLASS_SNIPER: iFlag = PlayerEnum::Sniper; break;
			case TF_CLASS_SPY: iFlag = PlayerEnum::Spy; break;
			}
			if (!(tGroup.m_iPlayers & iFlag))
				return false;
		}
		if (tGroup.m_iPlayers & PlayerEnum::Conds)
		{
			if (tGroup.m_iPlayers & PlayerEnum::Invulnerable && !pPlayer->IsInvulnerable()
				|| tGroup.m_iPlayers & PlayerEnum::Crits && !pPlayer->IsCritBoosted()
				|| tGroup.m_iPlayers & PlayerEnum::Invisible && !pPlayer->IsInvisible()
				|| tGroup.m_iPlayers & PlayerEnum::Disguise && !pPlayer->InCond(TF_COND_DISGUISED))
				return false;
		}
	}

	return ShouldTargetOwner(tGroup, iBit, pEntity, pEntity, pLocal);
}

static inline bool ShouldTargetBuilding(Group_t& tGroup, int iBit, CBaseEntity* pOwner, CBaseEntity* pEntity, CTFPlayer* pLocal)
{
	if (!(tGroup.m_iTargets & iBit))
		return false;

	if (tGroup.m_iBuildings)
	{
		int iFlag = 0;
		switch (pEntity->GetClassID())
		{
		case ETFClassID::CObjectSentrygun: iFlag = BuildingEnum::Sentry; break;
		case ETFClassID::CObjectDispenser: iFlag = BuildingEnum::Dispenser; break;
		case ETFClassID::CObjectTeleporter: iFlag = BuildingEnum::Teleporter; break;
		}
		if (!(tGroup.m_iBuildings & iFlag))
			return false;
	}

	return ShouldTargetOwner(tGroup, iBit, pOwner, pEntity, pLocal);
}

static inline bool ShouldTargetProjectile(Group_t& tGroup, int iBit, CBaseEntity* pOwner, CBaseEntity* pEntity, CTFPlayer* pLocal)
{
	if (!(tGroup.m_iTargets & iBit))
		return false;

	if (tGroup.m_iProjectiles)
	{
		int iFlag = ProjectileEnum::Misc;
		switch (pEntity->GetClassID())
		{
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_EnergyBall:
		case ETFClassID::CTFProjectile_SentryRocket: iFlag = ProjectileEnum::Rocket; break;
		case ETFClassID::CTFGrenadePipebombProjectile: iFlag = pEntity->As<CTFGrenadePipebombProjectile>()->HasStickyEffects() ? ProjectileEnum::Sticky : ProjectileEnum::Pipe; break;
		case ETFClassID::CTFProjectile_Arrow: iFlag = pEntity->As<CTFProjectile_Arrow>()->m_iProjectileType() == TF_PROJECTILE_BUILDING_REPAIR_BOLT ? ProjectileEnum::Repair : ProjectileEnum::Arrow; break;
		case ETFClassID::CTFProjectile_HealingBolt: iFlag = ProjectileEnum::Heal; break;
		case ETFClassID::CTFProjectile_Flare: iFlag = ProjectileEnum::Flare; break;
		case ETFClassID::CTFProjectile_BallOfFire: iFlag = ProjectileEnum::Fire; break;
		case ETFClassID::CTFProjectile_Cleaver: iFlag = ProjectileEnum::Cleaver; break;
		case ETFClassID::CTFProjectile_JarMilk:
		case ETFClassID::CTFProjectile_ThrowableBreadMonster: iFlag = ProjectileEnum::Milk; break;
		case ETFClassID::CTFProjectile_Jar: iFlag = ProjectileEnum::Jarate; break;
		case ETFClassID::CTFProjectile_JarGas: iFlag = ProjectileEnum::Gas; break;
		case ETFClassID::CTFBall_Ornament: iFlag = ProjectileEnum::Bauble; break;
		case ETFClassID::CTFStunBall: iFlag = ProjectileEnum::Baseball; break;
		case ETFClassID::CTFProjectile_EnergyRing: iFlag = ProjectileEnum::Energy; break;
		case ETFClassID::CTFProjectile_MechanicalArmOrb: iFlag = ProjectileEnum::ShortCircuit; break;
		case ETFClassID::CTFProjectile_SpellMeteorShower: iFlag = ProjectileEnum::MeteorShower; break;
		case ETFClassID::CTFProjectile_SpellLightningOrb: iFlag = ProjectileEnum::Lightning; break;
		case ETFClassID::CTFProjectile_SpellFireball: iFlag = ProjectileEnum::Fireball; break;
		case ETFClassID::CTFWeaponBaseMerasmusGrenade: iFlag = ProjectileEnum::Bomb; break;
		case ETFClassID::CTFProjectile_SpellBats:
		case ETFClassID::CTFProjectile_SpellKartBats: iFlag = ProjectileEnum::Bats; break;
		case ETFClassID::CTFProjectile_SpellMirv:
		case ETFClassID::CTFProjectile_SpellPumpkin: iFlag = ProjectileEnum::Pumpkin; break;
		case ETFClassID::CTFProjectile_SpellSpawnBoss: iFlag = ProjectileEnum::Monoculus; break;
		case ETFClassID::CTFProjectile_SpellSpawnHorde:
		case ETFClassID::CTFProjectile_SpellSpawnZombie: iFlag = ProjectileEnum::Skeleton; break;
		}
		if (!(tGroup.m_iProjectiles & iFlag))
			return false;
	}

	return ShouldTargetOwner(tGroup, iBit, pOwner, pEntity, pLocal);
}

static inline bool ShouldTarget(Group_t& tGroup, CBaseEntity* pEntity, CTFPlayer* pLocal, bool bModels)
{
	if (tGroup.m_iConditions & ConditionsEnum::Dormant)
	{
		if (!pEntity->IsDormant())
			return false;

		switch (pEntity->GetClassID())
		{
		case ETFClassID::CTFPlayer:
		case ETFClassID::CObjectSentrygun:
		case ETFClassID::CObjectDispenser:
		case ETFClassID::CObjectTeleporter:
			if (!H::Entities.GetDormancy(pEntity->entindex()))
				return false;
		}
	}
	else if (pEntity->IsDormant())
		return false;

	switch (pEntity->GetClassID())
	{
	// players
	case ETFClassID::CTFPlayer:
		return ShouldTargetPlayer(tGroup, TargetsEnum::Players, pEntity, pLocal);
	// buildings
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
	{
		auto pOwner = pEntity->As<CBaseObject>()->m_hBuilder().Get();
		if (!pOwner) pOwner = pEntity;
		return ShouldTargetBuilding(tGroup, TargetsEnum::Buildings, pOwner, pEntity, pLocal);
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
		return ShouldTargetProjectile(tGroup, TargetsEnum::Projectiles, pOwner, pEntity, pLocal);
	}
	// ragdolls
	case ETFClassID::CTFRagdoll:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CRagdollPropAttached:
	{
		if (!bModels)
			return false;

		auto pOwner = pEntity->As<CTFRagdoll>()->m_hPlayer().Get();
		if (!pOwner) pOwner = pEntity;
		return ShouldTargetOwner(tGroup, TargetsEnum::Ragdolls, pOwner, pEntity, pLocal);
	}
	// objectives
	case ETFClassID::CCaptureFlag:
		return ShouldTargetTeam(tGroup, TargetsEnum::Objective, pEntity, pLocal);
	// npcs
	case ETFClassID::CTFBaseBoss:
	case ETFClassID::CTFTankBoss:
	case ETFClassID::CMerasmus:
	case ETFClassID::CEyeballBoss:
	case ETFClassID::CHeadlessHatman:
	case ETFClassID::CZombie:
		if (pEntity->IsInValidTeam())
		{
			if (auto pOwner = pEntity->m_hOwnerEntity().Get())
				return ShouldTargetOwner(tGroup, TargetsEnum::NPCs, pOwner, pEntity, pLocal);
			return ShouldTargetTeam(tGroup, TargetsEnum::NPCs, pEntity, pLocal);
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
	if (bModels)
	{
		auto pOwner = pEntity->m_hOwnerEntity().Get();
		if (pOwner && pOwner->IsPlayer())
			return ShouldTargetPlayer(tGroup, TargetsEnum::Players, pOwner, pLocal);
	}

	return false;
}



void CGroups::Store(CTFPlayer* pLocal)
{
	m_mEntities.clear();
	m_mModels.clear();
	if (!GroupsActive())
		return;

	Group_t* pGroup = nullptr;
	for (int n = 1; n <= I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(n)->As<CBaseEntity>();
		if (!pEntity)
			continue;

		if (GetGroup(pEntity, pLocal, pGroup, false))
			m_mEntities[pEntity] = pGroup;
		if (GetGroup(pEntity, pLocal, pGroup, true))
			m_mModels[pEntity] = pGroup;
	}
}

bool CGroups::GetGroup(CBaseEntity* pEntity, Group_t*& pGroup, bool bModels)
{
	if (!GroupsActive())
		return false;

	if (!bModels && m_mEntities.contains(pEntity))
	{
		pGroup = m_mEntities[pEntity];
		return true;
	}
	else if (bModels && m_mModels.contains(pEntity))
	{
		pGroup = m_mModels[pEntity];
		return true;
	}

	return false;
}

const std::unordered_map<CBaseEntity*, Group_t*>& CGroups::GetGroup(bool bModels)
{
	return !bModels ? m_mEntities : m_mModels;
}

bool CGroups::GetGroup(CBaseEntity* pEntity, CTFPlayer* pLocal, Group_t*& pGroup, bool bModels)
{
	if (!GroupsActive())
		return false;

	for (int i = int(m_vGroups.size() - 1); i >= 0; i--) // reverse so back groups have higher priority
	{
		auto& tGroup = m_vGroups[i];

		if (!(Vars::ESP::ActiveGroups.Value & 1 << i))
			continue;

		if (!ShouldTarget(tGroup, pEntity, pLocal, bModels))
			continue;

		pGroup = &tGroup;
		return true;
	}

	return false;
}

bool CGroups::GetGroup(int iType, Group_t*& pGroup, CBaseEntity* pEntity)
{
	if (!GroupsActive())
		return false;

	for (int i = int(m_vGroups.size() - 1); i >= 0; i--) // reverse so back groups have higher priority
	{
		auto& tGroup = m_vGroups[i];

		if (!(Vars::ESP::ActiveGroups.Value & 1 << i))
			continue;

		if (!(tGroup.m_iTargets & iType) || pEntity && (tGroup.m_iConditions & ConditionsEnum::Dormant ? !pEntity->IsDormant() : pEntity->IsDormant()))
			continue;

		pGroup = &tGroup;
		return true;
	}

	return false;
}

bool CGroups::GetGroup(int iType)
{
	if (!GroupsActive())
		return false;

	for (int i = int(m_vGroups.size() - 1); i >= 0; i--) // reverse so back groups have higher priority
	{
		auto& tGroup = m_vGroups[i];

		if (!(Vars::ESP::ActiveGroups.Value & 1 << i))
			continue;

		if (!(tGroup.m_iTargets & iType))
			continue;

		return true;
	}

	return false;
}

Color_t CGroups::GetColor(CBaseEntity* pEntity, Group_t* pGroup)
{
	if (!pGroup->m_bTagsOverrideColor)
		return pGroup->m_tColor;

	if (!pEntity->IsPlayer())
	{
		pEntity = pEntity->IsBuilding() ? pEntity->As<CBaseObject>()->m_hBuilder()
			: pEntity->IsProjectile() ? F::ProjSim.GetEntities(pEntity).second
			: pEntity->m_hOwnerEntity();
		if (!pEntity || !pEntity->IsPlayer())
			return pGroup->m_tColor;
	}

	auto pTag = F::PlayerUtils.GetSignificantTag(pEntity->entindex());
	if (!pTag)
		return pGroup->m_tColor;

	return pTag->m_tColor.Alpha(pGroup->m_tColor.a);
}

bool CGroups::GroupsActive()
{
	return Vars::ESP::ActiveGroups.Value && !m_vGroups.empty();
}

void CGroups::Move(int i1, int i2)
{
	m_vGroups.insert(std::next(m_vGroups.begin(), i2 + (i1 < i2 ? 1 : 0)), m_vGroups[i1]);
	m_vGroups.erase(std::next(m_vGroups.begin(), i1 + (i1 > i2 ? 1 : 0)));
}