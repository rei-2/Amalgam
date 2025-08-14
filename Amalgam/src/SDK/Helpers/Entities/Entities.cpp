#include "Entities.h"

#include "../../SDK.h"
#include "../../../Utils/Hash/FNV1A.h"
#include "../../../Features/Players/PlayerUtils.h"
#include "../../../Features/Backtrack/Backtrack.h"
#include "../../../Features/CheaterDetection/CheaterDetection.h"
#include "../../../Features/Resolver/Resolver.h"

void CEntities::Store()
{
	auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal)
		return;

	m_pLocal = pLocal->As<CTFPlayer>();
	m_pLocalWeapon = m_pLocal->m_hActiveWeapon()->As<CTFWeaponBase>();

	int iLag;
	{
		static int iStaticTickcout = I::GlobalVars->tickcount;
		iLag = I::GlobalVars->tickcount - iStaticTickcout - 1;
		iStaticTickcout = I::GlobalVars->tickcount;
	}

	for (int n = I::EngineClient->GetMaxClients() + 1; n <= I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pEntity = I::ClientEntityList->GetClientEntity(n)->As<CBaseEntity>();
		if (!pEntity || ManageDormancy(pEntity))
			continue;

		auto nClassID = pEntity->GetClassID();
		switch (nClassID)
		{
		case ETFClassID::CTFPlayerResource:
			m_pPlayerResource = pEntity->As<CTFPlayerResource>();
			break;
		case ETFClassID::CObjectSentrygun:
		case ETFClassID::CObjectDispenser:
		case ETFClassID::CObjectTeleporter:
			m_mModels[n] = FNV1A::Hash32(I::ModelInfoClient->GetModelName(pEntity->GetModel()));
			m_mGroups[EGroupType::BUILDINGS_ALL].push_back(pEntity);
			m_mGroups[pEntity->m_iTeamNum() != m_pLocal->m_iTeamNum() ? EGroupType::BUILDINGS_ENEMIES : EGroupType::BUILDINGS_TEAMMATES].push_back(pEntity);
			break;
		case ETFClassID::CBaseAnimating:
		{
			m_mModels[n] = FNV1A::Hash32(I::ModelInfoClient->GetModelName(pEntity->GetModel()));
			if (IsHealth(GetModel(n)))
				m_mGroups[EGroupType::PICKUPS_HEALTH].push_back(pEntity);
			else if (IsAmmo(GetModel(n)))
				m_mGroups[EGroupType::PICKUPS_AMMO].push_back(pEntity);
			else if (IsPowerup(GetModel(n)))
				m_mGroups[EGroupType::PICKUPS_POWERUP].push_back(pEntity);
			else if (IsSpellbook(GetModel(n)))
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
			if ((nClassID == ETFClassID::CTFProjectile_Cleaver || nClassID == ETFClassID::CTFStunBall) && pEntity->As<CTFGrenadePipebombProjectile>()->m_bTouched()
				|| (nClassID == ETFClassID::CTFProjectile_Arrow || nClassID == ETFClassID::CTFProjectile_GrapplingHook) && !pEntity->m_MoveType())
				break;

			m_mGroups[EGroupType::WORLD_PROJECTILES].push_back(pEntity);

			if (nClassID == ETFClassID::CTFGrenadePipebombProjectile)
			{
				auto pPipebomb = pEntity->As<CTFGrenadePipebombProjectile>();
				if (pPipebomb->m_hThrower().Get() == pLocal && pPipebomb->m_iType() == TF_GL_MODE_REMOTE_DETONATE /*pPipebomb->HasStickyEffects()*/)
					m_mGroups[EGroupType::MISC_LOCAL_STICKIES].push_back(pEntity);
			}

			if (nClassID == ETFClassID::CTFProjectile_Flare)
			{
				auto pLauncher = pEntity->As<CTFProjectile_Flare>()->m_hLauncher()->As<CTFWeaponBase>();
				if (pEntity->m_hOwnerEntity().Get() == m_pLocal && pLauncher && pLauncher->As<CTFFlareGun>()->GetFlareGunType() == FLAREGUN_DETONATE)
					m_mGroups[EGroupType::MISC_LOCAL_FLARES].push_back(pEntity);
			}

			break;
		}
		case ETFClassID::CCaptureFlag:
			m_mGroups[EGroupType::WORLD_OBJECTIVE].push_back(pEntity);
			break;
		case ETFClassID::CTFBaseBoss:
		case ETFClassID::CTFTankBoss:
		case ETFClassID::CMerasmus:
		case ETFClassID::CEyeballBoss:
		case ETFClassID::CHeadlessHatman:
		case ETFClassID::CZombie:
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

	static Timer tTimer = {};
	bool bUpdateInfo = tTimer.Run(1.f);
	std::unordered_map<uint32_t, uint64_t> mParties = {};
	std::unordered_map<uint32_t, bool> mF2P = {};
	std::unordered_map<uint32_t, int> mLevels = {};
	if (bUpdateInfo)
	{
		m_mIPriorities.clear();
		m_mUPriorities.clear();
		m_mIFriends.clear();
		m_mUFriends.clear();
		m_mIParty.clear();
		m_mUParty.clear();
		m_mIF2P.clear();
		m_mUF2P.clear();
		m_mILevels.clear();
		m_mULevels.clear();

		if (auto pLobby = I::TFGCClientSystem->GetLobby())
		{
			int iMembers = pLobby->GetNumMembers();
			for (int i = 0; i < iMembers; i++)
			{
				auto cSteamID = CSteamID(); pLobby->GetMember(&cSteamID, i);
				auto uAccountID = cSteamID.GetAccountID();

				ConstTFLobbyPlayer pDetails;
				pLobby->GetMemberDetails(&pDetails, i);

				auto pProto = pDetails.Proto();
				mF2P[uAccountID] = pProto->chat_suspension;
				mLevels[uAccountID] = pProto->rank;
				mParties[uAccountID] = pProto->original_party_id;
			}
		}
		if (auto pParty = I::TFGCClientSystem->GetParty())
		{
			int iMembers = pParty->GetNumMembers();
			for (int i = 0; i < iMembers; i++)
			{
				auto cSteamID = CSteamID(); pParty->GetMember(&cSteamID, i);
				auto uAccountID = cSteamID.GetAccountID();
				mParties[uAccountID] = 1;
			}
		}
	}
	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		if (bUpdateInfo)
		{
			auto pResource = GetResource();
			if (pResource && pResource->m_bValid(n))
			{
				bool bLocal = n == I::EngineClient->GetLocalPlayer();
				uint32_t uAccountID = pResource->m_iAccountID(n);
				if (bLocal) m_uAccountID = uAccountID;

				m_mIPriorities[n] = m_mUPriorities[uAccountID] = !bLocal ? F::PlayerUtils.GetPriority(uAccountID, false) : 0;
				m_mIFriends[n] = m_mUFriends[uAccountID] = !pResource->IsFakePlayer(n) ? I::SteamFriends->HasFriend({ uAccountID, 1, k_EUniversePublic, k_EAccountTypeIndividual }, k_EFriendFlagImmediate) : false;
				m_mIParty[n] = m_mUParty[uAccountID] = mParties.contains(uAccountID) ? mParties[uAccountID] : 0;
				m_mIF2P[n] = m_mUF2P[uAccountID] = mF2P.contains(uAccountID) ? mF2P[uAccountID] : false;
				m_mILevels[n] = m_mULevels[uAccountID] = mLevels.contains(uAccountID) ? mLevels[uAccountID] : -2;
			}
		}

		auto pPlayer = I::ClientEntityList->GetClientEntity(n)->As<CTFPlayer>();
		if (!pPlayer || !pPlayer->IsPlayer() || ManageDormancy(pPlayer))
			continue;

		m_mModels[n] = FNV1A::Hash32(I::ModelInfoClient->GetModelName(pPlayer->GetModel()));
		m_mGroups[EGroupType::PLAYERS_ALL].push_back(pPlayer);
		m_mGroups[pPlayer->m_iTeamNum() != m_pLocal->m_iTeamNum() ? EGroupType::PLAYERS_ENEMIES : EGroupType::PLAYERS_TEAMMATES].push_back(pPlayer);
		
		if (n != I::EngineClient->GetLocalPlayer())
		{
			bool bDormant = pPlayer->IsDormant();
			float flSimTime = pPlayer->m_flSimulationTime(), flOldSimTime = pPlayer->m_flOldSimulationTime();
			if (float flDeltaTime = m_mDeltaTimes[n] = TICKS_TO_TIME(std::clamp(TIME_TO_TICKS(flSimTime - flOldSimTime) - iLag, 0, 24)))
			{
				m_mLagTimes[n] = flDeltaTime;
				m_mSetTicks[n] = I::GlobalVars->tickcount;
				if (!bDormant)
				{
					m_mOrigins[n].emplace_front(pPlayer->m_vecOrigin() + Vec3(0, 0, pPlayer->GetSize().z), flSimTime);
					if (m_mOrigins[n].size() > Vars::Aimbot::Projectile::VelocityAverageCount.Value)
						m_mOrigins[n].pop_back();

					if (pPlayer->IsAlive())
						F::CheaterDetection.ReportChoke(pPlayer, m_mChokes[n]);
				}
				else
					m_mOrigins[n].clear();

				m_mOldAngles[n] = m_mEyeAngles[n];
				m_mEyeAngles[n] = pPlayer->As<CTFPlayer>()->GetEyeAngles();
			}
			m_mChokes[n] = I::GlobalVars->tickcount - m_mSetTicks[n];
		}
	}
	F::Resolver.FrameStageNotify();
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (!pPlayer->IsAlive() || pPlayer->entindex() == I::EngineClient->GetLocalPlayer() && !I::EngineClient->IsPlayingDemo()) // local player managed in CreateMove
			continue;

		bool bResolver = F::Resolver.GetAngles(pPlayer);
		if (!Vars::Visuals::Removals::Interpolation.Value && !bResolver)
			continue;

		int iDeltaTicks = TIME_TO_TICKS(H::Entities.GetDeltaTime(pPlayer->entindex()));
		if (!iDeltaTicks)
			continue;

		float flOldFrameTime = I::GlobalVars->frametime;
		I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;
		for (int i = 0; i < iDeltaTicks; i++)
		{
			G::UpdatingAnims = true;

			if (bResolver)
			{
				float flYaw, flPitch;
				F::Resolver.GetAngles(pPlayer, &flYaw, &flPitch, nullptr, i + 1 == iDeltaTicks);

				float flOriginalYaw = pPlayer->m_angEyeAnglesY(), flOriginalPitch = pPlayer->m_angEyeAnglesX();
				pPlayer->m_angEyeAnglesY() = flYaw, pPlayer->m_angEyeAnglesX() = flPitch;
				pPlayer->UpdateClientSideAnimation();
				pPlayer->m_angEyeAnglesY() = flOriginalYaw, pPlayer->m_angEyeAnglesX() = flOriginalPitch;
			}
			else
				pPlayer->UpdateClientSideAnimation();

			G::UpdatingAnims = false;
		}
		I::GlobalVars->frametime = flOldFrameTime;
	}
}

void CEntities::Clear(bool bShutdown)
{
	m_pLocal = nullptr;
	m_pLocalWeapon = nullptr;
	m_pPlayerResource = nullptr;
	m_mGroups.clear();

	if (bShutdown)
	{
		m_mDeltaTimes.clear();
		m_mLagTimes.clear();
		m_mChokes.clear();
		m_mSetTicks.clear();
		m_mOldAngles.clear();
		m_mEyeAngles.clear();
		m_mLagCompensation.clear();
		m_mDormancy.clear();
		m_mAvgVelocities.clear();
		m_mModels.clear();
		m_mOrigins.clear();
	}
}

void CEntities::ManualNetwork(const StartSoundParams_t& params)
{
	if (params.soundsource <= 0 || params.soundsource == I::EngineClient->GetLocalPlayer())
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(params.soundsource)->As<CBaseEntity>();
	if (!pEntity || !pEntity->IsDormant() || !pEntity->IsPlayer() && !pEntity->IsBuilding())
		return;

	float flDuration = 0.f;
	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFPlayer: flDuration = 1.f; break;
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter: flDuration = 5.f; break;
	}
	if (flDuration)
		m_mDormancy[params.soundsource] = { params.origin, I::GlobalVars->curtime + flDuration };
}

bool CEntities::ManageDormancy(CBaseEntity* pEntity)
{
	bool bDormant = pEntity->IsDormant();

	float flDuration = 0.f;
	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFPlayer: flDuration = 1.f; break;
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter: flDuration = 5.f; break;
	}
	if (flDuration)
	{
		int n = pEntity->entindex();
		if (bDormant)
		{
			if (pEntity->IsPlayer())
			{
				if (auto pResource = GetResource(); pResource)
				{
					pEntity->As<CTFPlayer>()->m_lifeState() = pResource->m_bAlive(n) ? LIFE_ALIVE : LIFE_DEAD;
					pEntity->As<CTFPlayer>()->m_iHealth() = pResource->m_iHealth(n);
				}
			}
			if (m_mDormancy.contains(n))
			{
				auto& tDormancy = m_mDormancy[n];
				if (tDormancy.LastUpdate - I::GlobalVars->curtime > 0.f)
					pEntity->SetAbsOrigin(pEntity->m_vecOrigin() = tDormancy.Location);
				else
					m_mDormancy.erase(n);
			}
		}
		else if (!pEntity->IsPlayer() || pEntity->As<CTFPlayer>()->IsAlive())
			m_mDormancy[n] = { pEntity->m_vecOrigin(), I::GlobalVars->curtime + flDuration };
	}

	return bDormant;
}

bool CEntities::IsHealth(uint32_t uHash)
{
	switch (uHash)
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

bool CEntities::IsAmmo(uint32_t uHash)
{
	switch (uHash)
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

bool CEntities::IsPowerup(uint32_t uHash)
{
	switch (uHash)
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

bool CEntities::IsSpellbook(uint32_t uHash)
{
	switch (uHash)
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

CTFPlayer* CEntities::GetLocal()
{
	return I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer())->As<CTFPlayer>();
	//return m_pLocal;
}
CTFWeaponBase* CEntities::GetWeapon()
{
	auto pLocal = GetLocal();
	return pLocal ? pLocal->m_hActiveWeapon()->As<CTFWeaponBase>() : nullptr;
	//return m_pLocalWeapon;
}
CTFPlayerResource* CEntities::GetResource()
{
	return m_pPlayerResource;
}

const std::vector<CBaseEntity*>& CEntities::GetGroup(const EGroupType& Group) { return m_mGroups[Group]; }

float CEntities::GetDeltaTime(int iIndex) { return m_mDeltaTimes.contains(iIndex) ? m_mDeltaTimes[iIndex] : TICK_INTERVAL; }
float CEntities::GetLagTime(int iIndex) { return m_mLagTimes.contains(iIndex) ? m_mLagTimes[iIndex] : TICK_INTERVAL; }
int CEntities::GetChoke(int iIndex) { return m_mChokes.contains(iIndex) ? m_mChokes[iIndex] : 0; }
bool CEntities::GetDormancy(int iIndex) { return m_mDormancy.contains(iIndex); }
Vec3 CEntities::GetEyeAngles(int iIndex) { return m_mEyeAngles.contains(iIndex) ? m_mEyeAngles[iIndex] : Vec3(); }
Vec3 CEntities::GetPingAngles(int iIndex) { return m_mOldAngles.contains(iIndex) ? (m_mEyeAngles[iIndex] - m_mOldAngles[iIndex]) / GetLagTime(iIndex) * (F::Backtrack.GetReal() + TICKS_TO_TIME(F::Backtrack.GetAnticipatedChoke())) : Vec3(); }
bool CEntities::GetLagCompensation(int iIndex) { return m_mLagCompensation[iIndex]; }
void CEntities::SetLagCompensation(int iIndex, bool bLagComp) { m_mLagCompensation[iIndex] = bLagComp; }
Vec3* CEntities::GetAvgVelocity(int iIndex) { return iIndex != I::EngineClient->GetLocalPlayer() ? &m_mAvgVelocities[iIndex] : nullptr; }
void CEntities::SetAvgVelocity(int iIndex, Vec3 vAvgVelocity) { m_mAvgVelocities[iIndex] = vAvgVelocity; }
uint32_t CEntities::GetModel(int iIndex) { return m_mModels[iIndex]; }
std::deque<VelFixRecord>* CEntities::GetOrigins(int iIndex) { return m_mOrigins.contains(iIndex) ? &m_mOrigins[iIndex] : nullptr; }

int CEntities::GetPriority(int iIndex) { return m_mIPriorities[iIndex]; }
int CEntities::GetPriority(uint32_t uAccountID) { return m_mUPriorities[uAccountID]; }
bool CEntities::IsFriend(int iIndex) { return m_mIFriends[iIndex]; }
bool CEntities::IsFriend(uint32_t uAccountID) { return m_mUFriends[uAccountID]; }
bool CEntities::InParty(int iIndex) { return iIndex != I::EngineClient->GetLocalPlayer() && m_mIParty[iIndex] == 1; }
bool CEntities::InParty(uint32_t uAccountID) { return uAccountID != m_uAccountID && m_mUParty[uAccountID] == 1; }
bool CEntities::IsF2P(int iIndex) { return m_mIF2P[iIndex]; }
bool CEntities::IsF2P(uint32_t uAccountID) { return m_mUF2P[uAccountID]; }
int CEntities::GetLevel(int iIndex) { return m_mILevels.contains(iIndex) ? m_mILevels[iIndex] : -2; }
int CEntities::GetLevel(uint32_t uAccountID) { return m_mULevels.contains(uAccountID) ? m_mULevels[uAccountID] : -2; }
uint64_t CEntities::GetParty(int iIndex) { return m_mIParty.contains(iIndex) ? m_mIParty[iIndex] : 0; }
uint64_t CEntities::GetParty(uint32_t uAccountID) { return m_mUParty.contains(uAccountID) ? m_mUParty[uAccountID] : 0; }