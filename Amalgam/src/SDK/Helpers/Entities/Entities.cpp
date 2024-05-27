#include "Entities.h"

#include "../../SDK.h"
#include "../../../Utils/Hash/FNV1A.h"
#include "../../../Features/Players/PlayerUtils.h"

void CEntities::Fill()
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

	for (int n = 1; n < I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		auto pClientEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pClientEntity)
			continue;

		auto pEntity = pClientEntity->As<CBaseEntity>();

		if (pEntity->IsDormant())
		{
			if (!pEntity->IsPlayer())
				continue;

			auto pPlayer = pEntity->As<CTFPlayer>();

			// Is any dormant data available?
			if (!G::DormancyMap.count(n))
				continue;

			auto& dormantData = G::DormancyMap[n];
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
			{
				m_mIDIndex[pi.friendsID] = n;
				m_mFriends[n] = I::SteamFriends->HasFriend({ pi.friendsID, 1, k_EUniversePublic, k_EAccountTypeIndividual }, k_EFriendFlagImmediate);
			}
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
			if (IsAmmo(pEntity))
				m_mGroups[EGroupType::WORLD_AMMO].push_back(pEntity);
			else if (IsHealth(pEntity))
				m_mGroups[EGroupType::WORLD_HEALTH].push_back(pEntity);
			else if (IsSpellbook(pEntity))
				m_mGroups[EGroupType::WORLD_SPELLBOOK].push_back(pEntity);
			break;
		}
		case ETFClassID::CTFAmmoPack:
			m_mGroups[EGroupType::WORLD_AMMO].push_back(pEntity);
			break;
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFGrenadePipebombProjectile:
		case ETFClassID::CTFProjectile_Jar:
		case ETFClassID::CTFProjectile_JarGas:
		case ETFClassID::CTFProjectile_JarMilk:
		case ETFClassID::CTFProjectile_Arrow:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_Flare:
		case ETFClassID::CTFProjectile_GrapplingHook:
		case ETFClassID::CTFProjectile_Cleaver:
		case ETFClassID::CTFProjectile_EnergyBall:
		case ETFClassID::CTFProjectile_EnergyRing:
		case ETFClassID::CTFProjectile_HealingBolt:
		case ETFClassID::CTFProjectile_ThrowableBreadMonster:
		case ETFClassID::CTFStunBall:
		case ETFClassID::CTFBall_Ornament:
		{
			m_mGroups[EGroupType::WORLD_PROJECTILES].push_back(pEntity);

			if (nClassID == ETFClassID::CTFGrenadePipebombProjectile)
			{
				auto pPipebomb = pEntity->As<CTFGrenadePipebombProjectile>();

				if (pPipebomb->HasStickyEffects() && pPipebomb->As<CBaseGrenade>()->m_hThrower().Get() == pLocal)
					m_mGroups[EGroupType::MISC_LOCAL_STICKIES].push_back(pEntity);
			}

			if (nClassID == ETFClassID::CTFProjectile_Flare)
			{
				auto pSecondary = m_pLocal->GetWeaponFromSlot(EWeaponSlot::SLOT_SECONDARY);
				if (pSecondary && pSecondary->m_iItemDefinitionIndex() == ETFWeapons::Pyro_s_TheDetonator && pEntity->m_hOwnerEntity().Get() == m_pLocal)
					m_mGroups[EGroupType::MISC_LOCAL_FLARES].push_back(pEntity);

				break;
			}

			break;
		}
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
			m_mGroups[EGroupType::WORLD_MONEY].push_back(pEntity);
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

	m_mUPriorities.clear(); m_mIPriorities.clear();
	for (auto& pPlayer : m_mGroups[EGroupType::PLAYERS_ALL])
	{
		PlayerInfo_t pi{};
		if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi))
		{
			int iPriority = F::PlayerUtils.GetPriority(pi.friendsID, false);
			m_mUPriorities[pi.friendsID] = iPriority;
			m_mIPriorities[pPlayer->entindex()] = iPriority;
		}
	}
}

void CEntities::Clear()
{
	m_pLocal = nullptr;
	m_pLocalWeapon = nullptr;
	m_pObservedTarget = nullptr;
	m_pPlayerResource = nullptr;
	m_mGroups.clear();
	m_mIDIndex.clear();
	m_mFriends.clear();
}

bool CEntities::IsHealth(CBaseEntity* pEntity)
{
	switch (FNV1A::Hash(I::ModelInfoClient->GetModelName(pEntity->GetModel())))
	{
	case FNV1A::HashConst("models/items/banana/plate_banana.mdl"):
	case FNV1A::HashConst("models/items/medkit_small.mdl"):
	case FNV1A::HashConst("models/items/medkit_medium.mdl"):
	case FNV1A::HashConst("models/items/medkit_large.mdl"):
	case FNV1A::HashConst("models/items/medkit_small_bday.mdl"):
	case FNV1A::HashConst("models/items/medkit_medium_bday.mdl"):
	case FNV1A::HashConst("models/items/medkit_large_bday.mdl"):
	case FNV1A::HashConst("models/items/plate.mdl"):
	case FNV1A::HashConst("models/items/plate_sandwich_xmas.mdl"):
	case FNV1A::HashConst("models/items/plate_robo_sandwich.mdl"):
	case FNV1A::HashConst("models/props_medieval/medieval_meat.mdl"):
	case FNV1A::HashConst("models/workshopweapons/c_models/c_chocolate/plate_chocolate.mdl"):
	case FNV1A::HashConst("models/workshopweapons/c_models/c_fishcake/plate_fishcake.mdl"):
	case FNV1A::HashConst("models/props_halloween/halloween_medkit_small.mdl"):
	case FNV1A::HashConst("models/props_halloween/halloween_medkit_medium.mdl"):
	case FNV1A::HashConst("models/props_halloween/halloween_medkit_large.mdl"):
	case FNV1A::HashConst("models/items/ld1/mushroom_large.mdl"):
	case FNV1A::HashConst("models/items/plate_steak.mdl"):
	case FNV1A::HashConst("models/props_brine/foodcan.mdl"):
		return true;
	}
	return false;
}

bool CEntities::IsAmmo(CBaseEntity* pEntity)
{
	switch (FNV1A::Hash(I::ModelInfoClient->GetModelName(pEntity->GetModel())))
	{
	case FNV1A::HashConst("models/items/ammopack_small.mdl"):
	case FNV1A::HashConst("models/items/ammopack_medium.mdl"):
	case FNV1A::HashConst("models/items/ammopack_large.mdl"):
	case FNV1A::HashConst("models/items/ammopack_large_bday.mdl"):
	case FNV1A::HashConst("models/items/ammopack_medium_bday.mdl"):
	case FNV1A::HashConst("models/items/ammopack_small_bday.mdl"):
		return true;
	}
	return false;
}

bool CEntities::IsSpellbook(CBaseEntity* pEntity)
{
	switch (FNV1A::Hash(I::ModelInfoClient->GetModelName(pEntity->GetModel())))
	{
	case FNV1A::HashConst("models/props_halloween/hwn_spellbook_flying.mdl"):
	case FNV1A::HashConst("models/props_halloween/hwn_spellbook_upright.mdl"):
	case FNV1A::HashConst("models/props_halloween/hwn_spellbook_upright_major.mdl"):
	case FNV1A::HashConst("models/items/crystal_ball_pickup.mdl"):
	case FNV1A::HashConst("models/items/crystal_ball_pickup_major.mdl"):
	case FNV1A::HashConst("models/props_monster_mash/flask_vial_green.mdl"):
	case FNV1A::HashConst("models/props_monster_mash/flask_vial_purple.mdl"): // prop_dynamic in the map, probably won't work
		return true;
	}
	return false;
}