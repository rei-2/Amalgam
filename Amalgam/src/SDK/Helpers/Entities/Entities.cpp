#include "Entities.h"

#include "../../SDK.h"
#include "../../../Utils/Hash/FNV1A.h"
#include "../../../Features/Players/PlayerUtils.h"
#include "../../../Features/Backtrack/Backtrack.h"
#include "../../../Features/CheaterDetection/CheaterDetection.h"

void CEntities::Store()
{
	auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal)
		return;

	m_pLocal = pLocal->As<CTFPlayer>();
	if (auto pEntity = m_pLocal->m_hActiveWeapon().Get())
		m_pLocalWeapon = pEntity->As<CTFWeaponBase>();

	switch (m_pLocal->m_iObserverMode())
	{
	case OBS_MODE_FIRSTPERSON:
	case OBS_MODE_THIRDPERSON:
	{
		if (auto pObservedTarget = m_pLocal->m_hObserverTarget().Get())
			m_pObservedTarget = pObservedTarget->As<CTFPlayer>();
		break;
	}
	default: break;
	}
	
	for (int n = 1; n <= I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pClientEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pClientEntity)
			continue;

		auto pEntity = pClientEntity->As<CBaseEntity>();

		if (pEntity->IsDormant())
		{
			if (!pEntity->IsPlayer() || !m_mDormancy.contains(pEntity))
				continue;

			auto pPlayer = pEntity->As<CTFPlayer>();
			auto& dormantData = m_mDormancy[pEntity];
			float lastUpdate = dormantData.LastUpdate;

			if (I::EngineClient->Time() - lastUpdate > Vars::ESP::DormantTime.Value)
				continue;

			pPlayer->SetAbsOrigin(dormantData.Location);
			pPlayer->m_vecOrigin() = dormantData.Location;
			pPlayer->m_lifeState() = LIFE_ALIVE;

			auto playerResource = GetPR();
			if (playerResource && playerResource->GetValid(n))
				pPlayer->m_iHealth() = playerResource->GetHealth(n);
		}
		else if (pEntity->IsPlayer())
			m_mDormancy[pEntity] = { pEntity->m_vecOrigin(), I::EngineClient->Time() };

		auto nClassID = pEntity->GetClassID();
		switch (nClassID)
		{
		case ETFClassID::CTFPlayerResource:
			m_pPlayerResource = pEntity->As<CTFPlayerResource>();
			break;
		case ETFClassID::CTFPlayer:
		{
			m_mGroups[EGroupType::PLAYERS_ALL].push_back(pEntity);
			m_mGroups[pEntity->m_iTeamNum() != m_pLocal->m_iTeamNum() ? EGroupType::PLAYERS_ENEMIES : EGroupType::PLAYERS_TEAMMATES].push_back(pEntity);

			PlayerInfo_t pi{};
			if (I::EngineClient->GetPlayerInfo(n, &pi) && !pi.fakeplayer)
				m_mIFriends[n] = m_mUFriends[pi.friendsID] = I::SteamFriends->HasFriend({ pi.friendsID, 1, k_EUniversePublic, k_EAccountTypeIndividual }, k_EFriendFlagImmediate);
			break;
		}
		case ETFClassID::CObjectSentrygun:
		case ETFClassID::CObjectDispenser:
		case ETFClassID::CObjectTeleporter:
			m_mGroups[EGroupType::BUILDINGS_ALL].push_back(pEntity);
			m_mGroups[pEntity->m_iTeamNum() != m_pLocal->m_iTeamNum() ? EGroupType::BUILDINGS_ENEMIES : EGroupType::BUILDINGS_TEAMMATES].push_back(pEntity);
			break;
		case ETFClassID::CBaseAnimating:
		{
			if (IsHealth(pEntity))
				m_mGroups[EGroupType::PICKUPS_HEALTH].push_back(pEntity);
			else if (IsAmmo(pEntity))
				m_mGroups[EGroupType::PICKUPS_AMMO].push_back(pEntity);
			else if (IsPowerup(pEntity))
				m_mGroups[EGroupType::PICKUPS_POWERUP].push_back(pEntity);
			else if (IsSpellbook(pEntity))
				m_mGroups[EGroupType::PICKUPS_SPELLBOOK].push_back(pEntity);
			break;
		}
		case ETFClassID::CTFAmmoPack:
			m_mGroups[EGroupType::PICKUPS_AMMO].push_back(pEntity);
			break;
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
			m_mGroups[EGroupType::WORLD_PROJECTILES].push_back(pEntity);

			if (nClassID == ETFClassID::CTFGrenadePipebombProjectile)
			{
				auto pPipebomb = pEntity->As<CTFGrenadePipebombProjectile>();
				if (pPipebomb->m_hThrower().Get() == pLocal && pPipebomb->HasStickyEffects())
					m_mGroups[EGroupType::MISC_LOCAL_STICKIES].push_back(pEntity);
			}

			if (nClassID == ETFClassID::CTFProjectile_Flare)
			{
				auto pLauncher = pEntity->As<CTFProjectile_Flare>()->m_hLauncher().Get()->As<CTFWeaponBase>();
				if (pEntity->m_hOwnerEntity().Get() == m_pLocal && pLauncher && pLauncher->m_iItemDefinitionIndex() == ETFWeapons::Pyro_s_TheDetonator)
					m_mGroups[EGroupType::MISC_LOCAL_FLARES].push_back(pEntity);
			}

			break;
		}
		case ETFClassID::CCaptureFlag:
			m_mGroups[EGroupType::WORLD_OBJECTIVE].push_back(pEntity);
			break;
		case ETFClassID::CHeadlessHatman:
		case ETFClassID::CTFTankBoss:
		case ETFClassID::CMerasmus:
		case ETFClassID::CZombie:
		case ETFClassID::CEyeballBoss:
			m_mGroups[EGroupType::WORLD_NPC].push_back(pEntity);
			break;
		case ETFClassID::CTFPumpkinBomb:
		case ETFClassID::CTFGenericBomb:
			m_mGroups[EGroupType::WORLD_BOMBS].push_back(pEntity);
			break;
		case ETFClassID::CCurrencyPack:
			m_mGroups[EGroupType::PICKUPS_MONEY].push_back(pEntity);
			break;
		case ETFClassID::CHalloweenGiftPickup:
			if (pEntity->As<CHalloweenGiftPickup>()->m_hTargetPlayer().Get() == m_pLocal)
				m_mGroups[EGroupType::WORLD_GARGOYLE].push_back(pEntity);
			break;
		case ETFClassID::CSniperDot:
			m_mGroups[EGroupType::MISC_DOTS].push_back(pEntity);
			break;
		}
	}

	int iLag;
	{
		static int iStaticTickcout = I::GlobalVars->tickcount;
		iLag = I::GlobalVars->tickcount - iStaticTickcout - 1;
		iStaticTickcout = I::GlobalVars->tickcount;
	}

	static auto sv_maxusrcmdprocessticks = U::ConVars.FindVar("sv_maxusrcmdprocessticks");
	int iMaxShift = sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24;
	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		auto pPlayer = I::ClientEntityList->GetClientEntity(n)->As<CTFPlayer>();
		if (!pPlayer || n == I::EngineClient->GetLocalPlayer())
			continue;

		bool bDormant = pPlayer->IsDormant();

		float flOldSimTime = m_mOldSimTimes[pPlayer] = m_mSimTimes.contains(pPlayer) ? m_mSimTimes[pPlayer] : pPlayer->m_flOldSimulationTime();
		float flSimTime = m_mSimTimes[pPlayer] = (bDormant ? m_pLocal : pPlayer)->m_flSimulationTime(); // lol
		float flDeltaTime = m_mDeltaTimes[pPlayer] = TICKS_TO_TIME(std::clamp(TIME_TO_TICKS(flSimTime - flOldSimTime) - iLag, 0, iMaxShift));
		if (flDeltaTime)
		{
			if (pPlayer->IsAlive() && !bDormant)
				F::CheaterDetection.ReportChoke(pPlayer, m_mChokes[pPlayer]);
			m_mSetTicks[pPlayer] = I::GlobalVars->tickcount;
		}
		m_mChokes[pPlayer] = I::GlobalVars->tickcount - m_mSetTicks[pPlayer];

		if (!flDeltaTime)
			continue;
		else if (bDormant)
		{
			m_mBones[pPlayer].first = false;
			continue;
		}

		m_bSettingUpBones = true;
		m_mBones[pPlayer].first = pPlayer->SetupBones(m_mBones[pPlayer].second, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, flSimTime);
		m_bSettingUpBones = false;

		Vec3 vOldAngles = m_mEyeAngles[pPlayer], vNewAngles = pPlayer->As<CTFPlayer>()->GetEyeAngles();
		m_mEyeAngles[pPlayer] = vNewAngles;
		m_mPingAngles[pPlayer] = (vNewAngles - vOldAngles) / (flSimTime - flOldSimTime) * (F::Backtrack.GetReal() + TICKS_TO_TIME(G::AnticipatedChoke));
	}

	for (auto pEntity : GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();

		PlayerInfo_t pi{};
		if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi))
			m_mIPriorities[pPlayer->entindex()] = m_mUPriorities[pi.friendsID] = F::PlayerUtils.GetPriority(pi.friendsID, false);
	}
}

void CEntities::Clear(bool bShutdown)
{
	m_pLocal = nullptr;
	m_pLocalWeapon = nullptr;
	m_pObservedTarget = nullptr;
	m_pPlayerResource = nullptr;
	m_mGroups.clear();
	m_mIFriends.clear();
	m_mUFriends.clear();
	m_mIPriorities.clear();
	m_mUPriorities.clear();
	m_bSettingUpBones = false;

	if (bShutdown)
	{
		m_mSimTimes.clear();
		m_mOldSimTimes.clear();
		m_mDeltaTimes.clear();
		m_mChokes.clear();
		m_mDormancy.clear();
		m_mBones.clear();
		m_mEyeAngles.clear();
		m_mPingAngles.clear();
		m_mLagCompensation.clear();
	}
}

void CEntities::ManualNetwork(const StartSoundParams_t& params)
{
	if (params.soundsource <= 0 || params.soundsource == I::EngineClient->GetLocalPlayer())
		return;

	Vector vOrigin = params.origin;
	auto pEntity = I::ClientEntityList->GetClientEntity(params.soundsource)->As<CBaseEntity>();

	if (pEntity && pEntity->IsDormant() && pEntity->IsPlayer())
		m_mDormancy[pEntity] = { vOrigin, I::EngineClient->Time() };
}

bool CEntities::IsHealth(CBaseEntity* pEntity)
{
	switch (FNV1A::Hash32(I::ModelInfoClient->GetModelName(pEntity->GetModel())))
	{
	case FNV1A::Hash32Const("models/items/banana/plate_banana.mdl"):
	case FNV1A::Hash32Const("models/items/medkit_small.mdl"):
	case FNV1A::Hash32Const("models/items/medkit_medium.mdl"):
	case FNV1A::Hash32Const("models/items/medkit_large.mdl"):
	case FNV1A::Hash32Const("models/items/medkit_small_bday.mdl"):
	case FNV1A::Hash32Const("models/items/medkit_medium_bday.mdl"):
	case FNV1A::Hash32Const("models/items/medkit_large_bday.mdl"):
	case FNV1A::Hash32Const("models/items/plate.mdl"):
	case FNV1A::Hash32Const("models/items/plate_sandwich_xmas.mdl"):
	case FNV1A::Hash32Const("models/items/plate_robo_sandwich.mdl"):
	case FNV1A::Hash32Const("models/props_medieval/medieval_meat.mdl"):
	case FNV1A::Hash32Const("models/workshopweapons/c_models/c_chocolate/plate_chocolate.mdl"):
	case FNV1A::Hash32Const("models/workshopweapons/c_models/c_fishcake/plate_fishcake.mdl"):
	case FNV1A::Hash32Const("models/props_halloween/halloween_medkit_small.mdl"):
	case FNV1A::Hash32Const("models/props_halloween/halloween_medkit_medium.mdl"):
	case FNV1A::Hash32Const("models/props_halloween/halloween_medkit_large.mdl"):
	case FNV1A::Hash32Const("models/items/ld1/mushroom_large.mdl"):
	case FNV1A::Hash32Const("models/items/plate_steak.mdl"):
	case FNV1A::Hash32Const("models/props_brine/foodcan.mdl"):
		return true;
	}
	return false;
}

bool CEntities::IsAmmo(CBaseEntity* pEntity)
{
	switch (FNV1A::Hash32(I::ModelInfoClient->GetModelName(pEntity->GetModel())))
	{
	case FNV1A::Hash32Const("models/items/ammopack_small.mdl"):
	case FNV1A::Hash32Const("models/items/ammopack_medium.mdl"):
	case FNV1A::Hash32Const("models/items/ammopack_large.mdl"):
	case FNV1A::Hash32Const("models/items/ammopack_large_bday.mdl"):
	case FNV1A::Hash32Const("models/items/ammopack_medium_bday.mdl"):
	case FNV1A::Hash32Const("models/items/ammopack_small_bday.mdl"):
		return true;
	}
	return false;
}

bool CEntities::IsPowerup(CBaseEntity* pEntity)
{
	switch (FNV1A::Hash32(I::ModelInfoClient->GetModelName(pEntity->GetModel())))
	{
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_agility.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_crit.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_defense.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_haste.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_king.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_knockout.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_plague.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_precision.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_reflect.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_regen.mdl"):
	//case FNV1A::Hash32Const("models/pickups/pickup_powerup_resistance.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_strength.mdl"):
	//case FNV1A::Hash32Const("models/pickups/pickup_powerup_strength_arm.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_supernova.mdl"):
	//case FNV1A::Hash32Const("models/pickups/pickup_powerup_thorns.mdl"):
	//case FNV1A::Hash32Const("models/pickups/pickup_powerup_uber.mdl"):
	case FNV1A::Hash32Const("models/pickups/pickup_powerup_vampire.mdl"):
		return true;
	}
	return false;
}

bool CEntities::IsSpellbook(CBaseEntity* pEntity)
{
	switch (FNV1A::Hash32(I::ModelInfoClient->GetModelName(pEntity->GetModel())))
	{
	case FNV1A::Hash32Const("models/props_halloween/hwn_spellbook_flying.mdl"):
	case FNV1A::Hash32Const("models/props_halloween/hwn_spellbook_upright.mdl"):
	case FNV1A::Hash32Const("models/props_halloween/hwn_spellbook_upright_major.mdl"):
	case FNV1A::Hash32Const("models/items/crystal_ball_pickup.mdl"):
	case FNV1A::Hash32Const("models/items/crystal_ball_pickup_major.mdl"):
	case FNV1A::Hash32Const("models/props_monster_mash/flask_vial_green.mdl"):
	case FNV1A::Hash32Const("models/props_monster_mash/flask_vial_purple.mdl"): // prop_dynamic in the map, probably won't work
		return true;
	}
	return false;
}

CTFPlayer* CEntities::GetLocal() { return m_pLocal; }
CTFWeaponBase* CEntities::GetWeapon() { return m_pLocalWeapon; }
CTFPlayerResource* CEntities::GetPR() { return m_pPlayerResource; }
CTFPlayer* CEntities::GetObservedTarget() { return m_pObservedTarget; }

const std::vector<CBaseEntity*>& CEntities::GetGroup(const EGroupType& Group) { return m_mGroups[Group]; }

float CEntities::GetSimTime(CBaseEntity* pEntity) { return m_mSimTimes.contains(pEntity) ? m_mSimTimes[pEntity] : pEntity->m_flSimulationTime(); }
float CEntities::GetOldSimTime(CBaseEntity* pEntity) { return m_mOldSimTimes.contains(pEntity) ? m_mOldSimTimes[pEntity] : pEntity->m_flOldSimulationTime(); }
float CEntities::GetDeltaTime(CBaseEntity* pEntity) { return m_mDeltaTimes.contains(pEntity) ? m_mDeltaTimes[pEntity] : TICK_INTERVAL; }
int CEntities::GetChoke(CBaseEntity* pEntity) { return m_mChokes.contains(pEntity) ? m_mChokes[pEntity] : 0; }
matrix3x4* CEntities::GetBones(CBaseEntity* pEntity) { return m_mBones[pEntity].first ? m_mBones[pEntity].second : nullptr; }
Vec3 CEntities::GetEyeAngles(CBaseEntity* pEntity) { return m_mEyeAngles.contains(pEntity) ? m_mEyeAngles[pEntity] : Vec3(); }
Vec3 CEntities::GetPingAngles(CBaseEntity* pEntity) { return m_mPingAngles.contains(pEntity) ? m_mPingAngles[pEntity] : Vec3(); }
bool CEntities::GetLagCompensation(CBaseEntity* pEntity) { return m_mLagCompensation[pEntity]; }
void CEntities::SetLagCompensation(CBaseEntity* pEntity, bool bLagComp) { m_mLagCompensation[pEntity] = bLagComp; }

bool CEntities::IsFriend(int iIndex) { return m_mIFriends[iIndex]; }
bool CEntities::IsFriend(uint32_t friendsID) { return m_mUFriends[friendsID]; }
int CEntities::GetPriority(int iIndex) { return m_mIPriorities[iIndex]; }
int CEntities::GetPriority(uint32_t friendsID) { return m_mUPriorities[friendsID]; }

bool CEntities::IsSettingUpBones() { return m_bSettingUpBones; }