#include "ESP.h"

#include "../../Players/PlayerUtils.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"

MAKE_SIGNATURE(CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot, "client.dll", "48 89 6C 24 ? 56 41 54 41 55 41 56 41 57 48 83 EC", 0x0);
MAKE_SIGNATURE(CEconItemView_GetItemName, "client.dll", "40 53 48 83 EC ? 48 8B D9 C6 81 ? ? ? ? ? E8 ? ? ? ? 48 8B 8B", 0x0);

void CESP::Store(CTFPlayer* pLocal)
{
	m_mPlayerCache.clear();
	m_mBuildingCache.clear();
	m_mWorldCache.clear();

	if (!Vars::ESP::Draw.Value || !pLocal)
		return;

	StorePlayers(pLocal);
	StoreBuildings(pLocal);
	StoreProjectiles(pLocal);
	StoreObjective(pLocal);
	StoreWorld();
}

void CESP::StorePlayers(CTFPlayer* pLocal)
{
	if (!(Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Players) || !Vars::ESP::Player.Value)
		return;

	auto pResource = H::Entities.GetPR();
	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		int iIndex = pPlayer->entindex();

		if (iIndex == I::EngineClient->GetLocalPlayer())
		{
			if (!(Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Local) || !I::Input->CAM_IsThirdPerson())
				continue;
		}
		else
		{
			if (!pPlayer->IsAlive() || pPlayer->IsAGhost())
				continue;

			if (pPlayer->IsDormant())
			{
				if (!Vars::ESP::DormantAlpha.Value || Vars::ESP::DormantPriority.Value && F::PlayerUtils.GetPriority(iIndex) <= F::PlayerUtils.m_vTags[DEFAULT_TAG].Priority)
					continue;
			}

			if (!(Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Friends && H::Entities.IsFriend(iIndex))
				&& !(Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Prioritized && F::PlayerUtils.GetPriority(iIndex) > F::PlayerUtils.m_vTags[DEFAULT_TAG].Priority)
				&& pPlayer->m_iTeamNum() == pLocal->m_iTeamNum() ? !(Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Team) : !(Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Enemy))
				continue;
		}

		int iClassNum = pPlayer->m_iClass();
		auto pWeapon = pPlayer->m_hActiveWeapon().Get()->As<CTFWeaponBase>();

		PlayerCache& tCache = m_mPlayerCache[pEntity];
		tCache.m_flAlpha = (pPlayer->IsDormant() ? Vars::ESP::DormantAlpha.Value : Vars::ESP::ActiveAlpha.Value) / 255.f;
		tCache.m_tColor = H::Color.GetTeamColor(pLocal->m_iTeamNum(), pPlayer->m_iTeamNum(), Vars::Colors::Relative.Value);
		tCache.m_bBox = Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Box;
		tCache.m_bBones = Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Bones;

		if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Distance && pPlayer != pLocal)
		{
			Vec3 vDelta = pPlayer->m_vecOrigin() - pLocal->m_vecOrigin();
			tCache.m_vText.push_back({ TextBottom, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Menu::Theme::Active.Value });
		}

		PlayerInfo_t pi{};
		if (I::EngineClient->GetPlayerInfo(iIndex, &pi))
		{
			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Name)
				tCache.m_vText.push_back({ TextTop, F::PlayerUtils.GetPlayerName(iIndex, pi.name), Vars::Menu::Theme::Active.Value });

			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Priority)
			{
				if (auto pTag = F::PlayerUtils.GetSignificantTag(pi.friendsID, 1))
					tCache.m_vText.push_back({ TextTop, pTag->Name, pTag->Color });
			}

			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Labels)
			{
				std::vector<PriorityLabel_t*> vTags = {};
				for (auto& iID : F::PlayerUtils.m_mPlayerTags[pi.friendsID])
				{
					auto pTag = F::PlayerUtils.GetTag(iID);
					if (pTag && pTag->Label)
						vTags.push_back(pTag);
				}
				if (H::Entities.IsFriend(iIndex))
				{
					auto pTag = &F::PlayerUtils.m_vTags[FRIEND_TAG];
					if (pTag->Label)
						vTags.push_back(pTag);
				}

				if (vTags.size())
				{
					std::sort(vTags.begin(), vTags.end(), [&](const auto a, const auto b) -> bool
						{
							// sort by priority if unequal
							if (a->Priority != b->Priority)
								return a->Priority > b->Priority;

							return a->Name < b->Name;
						});

					for (auto& pTag : vTags)
						tCache.m_vText.push_back({ TextRight, pTag->Name, pTag->Color });
				}
			}
		}

		float flHealth = pPlayer->m_iHealth(), flMaxHealth = pPlayer->GetMaxHealth();
		if (tCache.m_bHealthBar = Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::HealthBar)
		{
			if (flHealth > flMaxHealth)
			{
				float flMaxOverheal = floorf(flMaxHealth / 10.f) * 5;
				tCache.m_flHealth = 1.f + std::clamp((flHealth - flMaxHealth) / flMaxOverheal, 0.f, 1.f);
			}
			else
				tCache.m_flHealth = std::clamp(flHealth / flMaxHealth, 0.f, 1.f);
		}
		if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::HealthText)
			tCache.m_vText.push_back({ TextHealth, std::format("{}", flHealth), flHealth > flMaxHealth ? Vars::Colors::Overheal.Value : Vars::Menu::Theme::Active.Value});

		if (iClassNum == TF_CLASS_MEDIC)
		{
			if (auto pMediGun = pPlayer->GetWeaponFromSlot(SLOT_SECONDARY))
			{
				tCache.m_flUber = std::clamp(pMediGun->As<CWeaponMedigun>()->m_flChargeLevel(), 0.f, 1.f);
				tCache.m_bUberBar = Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::UberBar;
				if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::UberText)
					tCache.m_vText.push_back({ TextUber, std::format("{:.0f}%%", tCache.m_flUber * 100.f), Vars::Menu::Theme::Active.Value });
			}
		}

		if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::ClassIcon)
			tCache.m_iClassIcon = iClassNum - 1;
		if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::ClassText)
			tCache.m_vText.push_back({ TextRight, GetPlayerClass(iClassNum), Vars::Menu::Theme::Active.Value });

		if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::WeaponIcon && pWeapon)
			tCache.m_pWeaponIcon = pWeapon->GetWeaponIcon();
		if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::WeaponText && pWeapon)
		{
			static auto fnGetEconItemViewByLoadoutSlot = reinterpret_cast<void*(__fastcall*)(void*, int, void**)>(S::CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot());
			static auto fnGetItemName = reinterpret_cast<const wchar_t*(__fastcall*)(void*)>(S::CEconItemView_GetItemName());

			int iWeaponSlot = pWeapon->GetSlot();
			switch (pPlayer->m_iClass())
			{
			case TF_CLASS_SPY:
			{
				switch (iWeaponSlot)
				{
				case 0: iWeaponSlot = 1; break;
				case 1: iWeaponSlot = 4; break;
				case 3: iWeaponSlot = 5; break;
				}
				break;
			}
			case TF_CLASS_ENGINEER:
			{
				switch (iWeaponSlot)
				{
				case 3: iWeaponSlot = 5; break;
				case 4: iWeaponSlot = 6; break;
				}
				break;
			}
			}

			if (void* pCurItemData = fnGetEconItemViewByLoadoutSlot(pPlayer, iWeaponSlot, 0))
				tCache.m_vText.push_back({ TextBottom, SDK::ConvertWideToUTF8(fnGetItemName(pCurItemData)), Vars::Menu::Theme::Active.Value });
		}

		if (Vars::Debug::Info.Value && !pPlayer->IsDormant() && pPlayer->entindex() != I::EngineClient->GetLocalPlayer())
		{
			int iAverage = TIME_TO_TICKS(F::MoveSim.GetPredictedDelta(pPlayer));
			int iCurrent = H::Entities.GetChoke(pPlayer);
			tCache.m_vText.push_back({ TextRight, std::format("Lag {}, {}", iAverage, iCurrent), {} });
		}

		{
			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::LagCompensation && !pPlayer->IsDormant() && pPlayer != pLocal)
			{
				if (H::Entities.GetLagCompensation(pPlayer))
					tCache.m_vText.push_back({ TextRight, "LAGCOMP", { 255, 100, 100, 255 } });
			}

			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Ping && pResource && pPlayer != pLocal)
			{
				auto pNetChan = I::EngineClient->GetNetChannelInfo();
				if (pNetChan && !pNetChan->IsLoopback())
				{
					int iPing = pResource->GetPing(pPlayer->entindex());
					if (iPing && (iPing >= 200 || iPing <= 5))
						tCache.m_vText.push_back({ TextRight, std::format("{}MS", iPing), { 255, 100, 100, 255 } });
				}
			}

			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::KDR && pResource && pPlayer != pLocal)
			{
				int iKills = pResource->GetKills(pPlayer->entindex()), iDeaths = pResource->GetDeaths(pPlayer->entindex());
				if (iKills >= 20)
				{
					int iKDR = iKills / std::max(iDeaths, 1);
					if (iKDR >= 10)
						tCache.m_vText.push_back({ TextRight, std::format("High KD [{} / {}]", iKills, iDeaths), { 255, 100, 100, 255 } });
				}
			}

			// Buffs
			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Buffs)
			{
				bool bCrits = false, bMiniCrits = false;
				if (pPlayer->IsCritBoosted())
					pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_PARTICLE_CANNON ? bMiniCrits = true : bCrits = true;
				if (pPlayer->IsMiniCritBoosted())
					pWeapon && pWeapon->m_iItemDefinitionIndex() == Sniper_t_TheBushwacka ? bCrits = true : bMiniCrits = true;

				if (bCrits)
					tCache.m_vText.push_back({ TextRight, "CRITS", { 255, 100, 100, 255 } });
				else if (bMiniCrits)
					tCache.m_vText.push_back({ TextRight, "MINI-CRITS", { 255, 175, 0, 255 } });

				if (pPlayer->InCond(TF_COND_RADIUSHEAL) ||
					pPlayer->InCond(TF_COND_HEALTH_BUFF) ||
					pPlayer->InCond(TF_COND_RADIUSHEAL_ON_DAMAGE) ||
					pPlayer->InCond(TF_COND_MEGAHEAL) ||
					pPlayer->InCond(TF_COND_HALLOWEEN_QUICK_HEAL) ||
					pPlayer->InCond(TF_COND_HALLOWEEN_HELL_HEAL) ||
					pPlayer->IsBuffedByKing())
					tCache.m_vText.push_back({ TextRight, "HP+", Vars::Colors::Overheal.Value });
				else if (pPlayer->InCond(TF_COND_HEALTH_OVERHEALED))
					tCache.m_vText.push_back({ TextRight, "HP", Vars::Colors::Overheal.Value });

				if (pPlayer->InCond(TF_COND_INVULNERABLE) ||
					pPlayer->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
					pPlayer->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
					pPlayer->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
					tCache.m_vText.push_back({ TextRight, "UBER", Vars::Colors::UberBar.Value });
				else if (pPlayer->InCond(TF_COND_PHASE))
					tCache.m_vText.push_back({ TextRight, "BONK", { 255, 175, 0, 255 } });

				/* vaccinator effects */
				if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) || pPlayer->InCond(TF_COND_BULLET_IMMUNE))
					tCache.m_vText.push_back({ TextRight, "BULLET+", { 255, 100, 100, 255 } });
				else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_BULLET_RESIST))
					tCache.m_vText.push_back({ TextRight, "BULLET", { 255, 100, 100, 255 } });
				if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST) || pPlayer->InCond(TF_COND_BLAST_IMMUNE))
					tCache.m_vText.push_back({ TextRight, "BLAST+", { 255, 100, 100, 255 } });
				else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_BLAST_RESIST))
					tCache.m_vText.push_back({ TextRight, "BLAST", { 255, 100, 100, 255 } });
				if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST) || pPlayer->InCond(TF_COND_FIRE_IMMUNE))
					tCache.m_vText.push_back({ TextRight, "FIRE+", { 255, 100, 100, 255 } });
				else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_FIRE_RESIST))
					tCache.m_vText.push_back({ TextRight, "FIRE", { 255, 100, 100, 255 } });

				if (pPlayer->InCond(TF_COND_OFFENSEBUFF))
					tCache.m_vText.push_back({ TextRight, "BANNER", tCache.m_tColor });
				if (pPlayer->InCond(TF_COND_DEFENSEBUFF))
					tCache.m_vText.push_back({ TextRight, "BATTALIONS", tCache.m_tColor });
				if (pPlayer->InCond(TF_COND_REGENONDAMAGEBUFF))
					tCache.m_vText.push_back({ TextRight, "CONCH", tCache.m_tColor });

				if (pPlayer->InCond(TF_COND_BLASTJUMPING))
					tCache.m_vText.push_back({ TextRight, "BLASTJUMP", { 255, 175, 0, 255 } });
			}

			// Debuffs
			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Debuffs)
			{
				if (pPlayer->InCond(TF_COND_STUNNED))
					tCache.m_vText.push_back({ TextRight, "STUNNED", { 255, 175, 0, 255 } });

				if (pPlayer->InCond(TF_COND_URINE))
					tCache.m_vText.push_back({ TextRight, "JARATE", { 255, 175, 0, 255 } });

				if (pPlayer->InCond(TF_COND_MARKEDFORDEATH) || pPlayer->InCond(TF_COND_MARKEDFORDEATH_SILENT))
					tCache.m_vText.push_back({ TextRight, "MARKED", { 255, 175, 0, 255 } });

				if (pPlayer->InCond(TF_COND_BURNING))
					tCache.m_vText.push_back({ TextRight, "BURN", { 255, 175, 0, 255 } });

				if (pPlayer->InCond(TF_COND_MAD_MILK))
					tCache.m_vText.push_back({ TextRight, "MILK", { 255, 175, 0, 255 } });
			}

			// Misc
			if (Vars::ESP::Player.Value & Vars::ESP::PlayerEnum::Misc)
			{
				if (Vars::Visuals::Removals::Taunts.Value && pPlayer->InCond(TF_COND_TAUNTING))
					tCache.m_vText.push_back({ TextRight, "TAUNT", { 255, 100, 200, 255 } });

				if (pPlayer->m_bFeignDeathReady())
					tCache.m_vText.push_back({ TextRight, "DR", { 255, 175, 0, 255 } });

				if (pPlayer->InCond(TF_COND_AIMING) && pWeapon)
				{
					switch (pWeapon->GetWeaponID())
					{
					case TF_WEAPON_MINIGUN:
						tCache.m_vText.push_back({ TextRight, "REV", { 127, 127, 127, 255 } });
						break;
					case TF_WEAPON_COMPOUND_BOW:
						tCache.m_vText.push_back({ TextRight, I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime() >= 1.f ? "CHARGED" : "CHARGING", { 255, 175, 0, 255 } });
						break;
					case TF_WEAPON_PARTICLE_CANNON:
						tCache.m_vText.push_back({ TextRight, "CHARGING", { 255, 175, 0, 255 } });
					}
				}

				if (pPlayer->InCond(TF_COND_ZOOMED))
					tCache.m_vText.push_back({ TextRight, "ZOOM", { 255, 175, 0, 255 } });

				if (pPlayer->InCond(TF_COND_STEALTHED) || pPlayer->InCond(TF_COND_STEALTHED_BLINK) || pPlayer->InCond(TF_COND_STEALTHED_USER_BUFF) || pPlayer->InCond(TF_COND_STEALTHED_USER_BUFF_FADING))
					tCache.m_vText.push_back({ TextRight, std::format("INVIS {:.0f}%%", pPlayer->GetInvisPercentage()), Vars::Colors::Cloak.Value });

				if (pPlayer->InCond(TF_COND_DISGUISING) || pPlayer->InCond(TF_COND_DISGUISED))
					tCache.m_vText.push_back({ TextRight, "DISGUISE", { 255, 175, 0, 255 } });
			}
		}
	}
}

void CESP::StoreBuildings(CTFPlayer* pLocal)
{
	if (!(Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Buildings) || !Vars::ESP::Building.Value)
		return;

	for (auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ALL))
	{
		auto pBuilding = pEntity->As<CBaseObject>();
		auto pOwner = pBuilding->m_hBuilder().Get();
		int iIndex = pOwner ? pOwner->entindex() : 0;

		if (pOwner)
		{
			if (iIndex == I::EngineClient->GetLocalPlayer())
			{
				if (!(Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Local))
					continue;
			}
			else
			{
				if (!(Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Friends && H::Entities.IsFriend(iIndex))
					&& !(Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Prioritized && F::PlayerUtils.GetPriority(iIndex) > F::PlayerUtils.m_vTags[DEFAULT_TAG].Priority)
					&& pOwner->m_iTeamNum() == pLocal->m_iTeamNum() ? !(Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Team) : !(Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Enemy))
					continue;
			}
		}
		else if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum() ? !(Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Team) : !(Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Enemy))
			continue;

		bool bIsMini = pBuilding->m_bMiniBuilding();

		BuildingCache& tCache = m_mBuildingCache[pEntity];
		tCache.m_flAlpha = Vars::ESP::ActiveAlpha.Value / 255.f;
		tCache.m_tColor = H::Color.GetTeamColor(pLocal->m_iTeamNum(), (pOwner ? pOwner : pEntity)->m_iTeamNum(), Vars::Colors::Relative.Value);
		tCache.m_bBox = Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Box;

		if (Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Distance)
		{
			Vec3 vDelta = pEntity->m_vecOrigin() - pLocal->m_vecOrigin();
			tCache.m_vText.push_back({ TextBottom, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Menu::Theme::Active.Value });
		}

		if (Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Name)
		{
			const char* szName = "Building";
			switch (pEntity->GetClassID())
			{
			case ETFClassID::CObjectSentrygun: szName = bIsMini ? "Mini-Sentry" : "Sentry"; break;
			case ETFClassID::CObjectDispenser: szName = "Dispenser"; break;
			case ETFClassID::CObjectTeleporter: szName = pBuilding->m_iObjectMode() ? "Teleporter Exit" : "Teleporter Entrance";
			}
			tCache.m_vText.push_back({ TextTop, szName, Vars::Menu::Theme::Active.Value });
		}

		float flHealth = pBuilding->m_iHealth(), flMaxHealth = pBuilding->m_iMaxHealth();
		if (tCache.m_bHealthBar = Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::HealthBar)
			tCache.m_flHealth = flHealth / flMaxHealth;
		if (Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::HealthText)
			tCache.m_vText.push_back({ TextHealth, std::format("{}", flHealth), Vars::Menu::Theme::Active.Value });

		if (Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Owner && !pBuilding->m_bWasMapPlaced() && pOwner)
		{
			PlayerInfo_t pi{};
			if (I::EngineClient->GetPlayerInfo(iIndex, &pi))
				tCache.m_vText.push_back({ TextTop, F::PlayerUtils.GetPlayerName(iIndex, pi.name), Vars::Menu::Theme::Active.Value });
		}

		if (Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Level && !bIsMini)
			tCache.m_vText.push_back({ TextRight, std::format("{} / {}", pBuilding->m_iUpgradeLevel(), bIsMini ? 1 : 3), Vars::Menu::Theme::Active.Value});

		if (Vars::ESP::Building.Value & Vars::ESP::BuildingEnum::Flags)
		{
			float flConstructed = pBuilding->m_flPercentageConstructed();
			if (flConstructed < 1.f)
				tCache.m_vText.push_back({ TextRight, std::format("{:.0f}%%", flConstructed * 100.f), { 255, 175, 0, 255 } });

			if (pBuilding->IsSentrygun() && pBuilding->As<CObjectSentrygun>()->m_bPlayerControlled())
				tCache.m_vText.push_back({ TextRight, "WRANGLED", { 255, 100, 100, 255 } });

			if (pBuilding->m_bHasSapper())
				tCache.m_vText.push_back({ TextRight, "SAPPED", { 255, 175, 0, 255 } });
			else if (pBuilding->m_bDisabled())
				tCache.m_vText.push_back({ TextRight, "DISABLED", { 255, 175, 0, 255 } });

			if (pBuilding->IsSentrygun() && !pBuilding->m_bBuilding())
			{
				int iShells, iMaxShells, iRockets, iMaxRockets; pBuilding->As<CObjectSentrygun>()->GetAmmoCount(iShells, iMaxShells, iRockets, iMaxRockets);
				if (!iShells)
					tCache.m_vText.push_back({ TextRight, "NO AMMO", { 127, 127, 127, 255 } });
				if (!bIsMini && !iRockets)
					tCache.m_vText.push_back({ TextRight, "NO ROCKETS", { 127, 127, 127, 255 } });
			}
		}
	}
}

void CESP::StoreProjectiles(CTFPlayer* pLocal)
{
	if (!(Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Projectiles) || !Vars::ESP::Projectile.Value)
		return;

	for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_PROJECTILES))
	{
		CBaseEntity* pOwner = nullptr;
		switch (pEntity->GetClassID())
		{
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
		{
			pOwner = pEntity->As<CBaseGrenade>()->m_hThrower().Get();
			break;
		}
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
		{
			auto pWeapon = pEntity->As<CTFBaseRocket>()->m_hLauncher().Get();
			pOwner = pWeapon ? pWeapon->As<CTFWeaponBase>()->m_hOwner().Get() : nullptr;
			break;
		}
		case ETFClassID::CTFBaseProjectile:
		case ETFClassID::CTFProjectile_EnergyRing:
		//case ETFClassID::CTFProjectile_Syringe:
		{
			auto pWeapon = pEntity->As<CTFBaseRocket>()->m_hLauncher().Get();
			pOwner = pWeapon ? pWeapon->As<CTFWeaponBase>()->m_hOwner().Get() : nullptr;
			break;
		}
		}

		if (pOwner)
		{
			int iIndex = pOwner->entindex();

			if (iIndex == I::EngineClient->GetLocalPlayer())
			{
				if (!(Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Local))
					continue;
			}
			else
			{
				if (!(Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Friends && H::Entities.IsFriend(iIndex))
					&& !(Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Prioritized && F::PlayerUtils.GetPriority(iIndex) > F::PlayerUtils.m_vTags[DEFAULT_TAG].Priority)
					&& pOwner->m_iTeamNum() == pLocal->m_iTeamNum() ? !(Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Team) : !(Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Enemy))
					continue;
			}
		}
		else if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum() ? !(Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Team) : !(Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Enemy))
			continue;

		WorldCache& tCache = m_mWorldCache[pEntity];
		tCache.m_flAlpha = Vars::ESP::ActiveAlpha.Value / 255.f;
		tCache.m_tColor = H::Color.GetTeamColor(pLocal->m_iTeamNum(), (pOwner ? pOwner : pEntity)->m_iTeamNum(), Vars::Colors::Relative.Value);
		tCache.m_bBox = Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Box;

		if (Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Distance)
		{
			Vec3 vDelta = pEntity->m_vecOrigin() - pLocal->m_vecOrigin();
			tCache.m_vText.push_back({ TextBottom, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Menu::Theme::Active.Value });
		}

		if (Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Name)
		{
			const char* szName = "Projectile";
			switch (pEntity->GetClassID())
			{
			//case ETFClassID::CBaseProjectile:
			//case ETFClassID::CBaseGrenade:
			//case ETFClassID::CTFWeaponBaseGrenadeProj:
			case ETFClassID::CTFWeaponBaseMerasmusGrenade: szName = "Bomb"; break;
			case ETFClassID::CTFGrenadePipebombProjectile: szName = pEntity->As<CTFGrenadePipebombProjectile>()->HasStickyEffects() ? "Sticky" : "Pipe"; break;
			case ETFClassID::CTFStunBall: szName = "Baseball"; break;
			case ETFClassID::CTFBall_Ornament: szName = "Bauble"; break;
			case ETFClassID::CTFProjectile_Jar: szName = "Jarate"; break;
			case ETFClassID::CTFProjectile_Cleaver: szName = "Cleaver"; break;
			case ETFClassID::CTFProjectile_JarGas: szName = "Gas"; break;
			case ETFClassID::CTFProjectile_JarMilk:
			case ETFClassID::CTFProjectile_ThrowableBreadMonster: szName = "Milk"; break;
			case ETFClassID::CTFProjectile_SpellBats:
			case ETFClassID::CTFProjectile_SpellKartBats: szName = "Bats"; break;
			case ETFClassID::CTFProjectile_SpellMeteorShower: szName = "Meteor shower"; break;
			case ETFClassID::CTFProjectile_SpellMirv:
			case ETFClassID::CTFProjectile_SpellPumpkin: szName = "Pumpkin"; break;
			case ETFClassID::CTFProjectile_SpellSpawnBoss: szName = "Monoculus"; break;
			case ETFClassID::CTFProjectile_SpellSpawnHorde:
			case ETFClassID::CTFProjectile_SpellSpawnZombie: szName = "Skeleton"; break;
			case ETFClassID::CTFProjectile_SpellTransposeTeleport: szName = "Teleport"; break;
			//case ETFClassID::CTFProjectile_Throwable:
			//case ETFClassID::CTFProjectile_ThrowableBrick:
			//case ETFClassID::CTFProjectile_ThrowableRepel:
			//case ETFClassID::CTFBaseRocket:
			//case ETFClassID::CTFFlameRocket:
			case ETFClassID::CTFProjectile_Arrow: szName = "Arrow"; break;
			case ETFClassID::CTFProjectile_GrapplingHook: szName = "Grapple"; break;
			case ETFClassID::CTFProjectile_HealingBolt: szName = "Heal"; break;
			case ETFClassID::CTFProjectile_Rocket:
			case ETFClassID::CTFProjectile_SentryRocket: szName = "Rocket"; break;
			case ETFClassID::CTFProjectile_BallOfFire: szName = "Fire"; break;
			case ETFClassID::CTFProjectile_MechanicalArmOrb: szName = "Short circuit"; break;
			case ETFClassID::CTFProjectile_SpellFireball: szName = "Fireball"; break;
			case ETFClassID::CTFProjectile_SpellLightningOrb: szName = "Lightning"; break;
			case ETFClassID::CTFProjectile_SpellKartOrb: szName = "Fist"; break;
			//case ETFClassID::CTFProjectile_EnergyBall:
			case ETFClassID::CTFProjectile_Flare: szName = "Flare"; break;
			//case ETFClassID::CTFBaseProjectile:
			case ETFClassID::CTFProjectile_EnergyRing: szName = "Energy"; break;
			//case ETFClassID::CTFProjectile_Syringe: szName = "Syringe";
			}
			tCache.m_vText.push_back({ TextTop, szName, Vars::Menu::Theme::Active.Value });
		}

		if (Vars::ESP::Projectile.Value & Vars::ESP::ProjectileEnum::Flags)
		{
			// do stuff like crit here
		}
	}
}

void CESP::StoreObjective(CTFPlayer* pLocal)
{
	if (!(Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Objective) || !Vars::ESP::Objective.Value)
		return;

	for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_OBJECTIVE))
	{
		if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum() ? !(Vars::ESP::Objective.Value & Vars::ESP::ObjectiveEnum::Team) : !(Vars::ESP::Objective.Value & Vars::ESP::ObjectiveEnum::Enemy))
			continue;

		WorldCache& tCache = m_mWorldCache[pEntity];
		tCache.m_flAlpha = Vars::ESP::ActiveAlpha.Value / 255.f;
		tCache.m_tColor = H::Color.GetTeamColor(pLocal->m_iTeamNum(), pEntity->m_iTeamNum(), Vars::Colors::Relative.Value);
		tCache.m_bBox = Vars::ESP::Objective.Value & Vars::ESP::ObjectiveEnum::Box;

		if (Vars::ESP::Objective.Value & Vars::ESP::ObjectiveEnum::Distance)
		{
			Vec3 vDelta = pEntity->m_vecOrigin() - pLocal->m_vecOrigin();
			tCache.m_vText.push_back({ TextBottom, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Menu::Theme::Active.Value });
		}

		switch (pEntity->GetClassID())
		{
		case ETFClassID::CCaptureFlag:
		{
			auto pIntel = pEntity->As<CCaptureFlag>();

			if (Vars::ESP::Objective.Value & Vars::ESP::ObjectiveEnum::Name)
				tCache.m_vText.push_back({ TextTop, "Intel", Vars::Menu::Theme::Active.Value });

			if (Vars::ESP::Objective.Value & Vars::ESP::ObjectiveEnum::Flags)
			{
				switch (pIntel->m_nFlagStatus())
				{
				case TF_FLAGINFO_HOME:
					tCache.m_vText.push_back({ TextRight, "HOME", { 255, 175, 0, 255 } });
					break;
				case TF_FLAGINFO_DROPPED:
					tCache.m_vText.push_back({ TextRight, "DROPPED", { 255, 175, 0, 255 } });
					break;
				default:
					tCache.m_vText.push_back({ TextRight, std::format("{}", pIntel->m_nFlagStatus()), { 255, 175, 0, 255 } });
				}
			}

			if (Vars::ESP::Objective.Value & Vars::ESP::ObjectiveEnum::IntelReturnTime && pIntel->m_nFlagStatus() == TF_FLAGINFO_DROPPED)
			{
				float flReturnTime = std::max(pIntel->m_flResetTime() - TICKS_TO_TIME(I::ClientState->m_ClockDriftMgr.m_nServerTick), 0.f);
				tCache.m_vText.push_back({ TextRight, std::format("RETURN {:.1f}S", pIntel->m_flResetTime() - TICKS_TO_TIME(I::ClientState->m_ClockDriftMgr.m_nServerTick)).c_str(), { 255, 175, 0, 255 } });
			}

			break;
		}
		}
	}
}

void CESP::StoreWorld()
{
	if (Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::NPCs)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_NPC))
		{
			WorldCache& tCache = m_mWorldCache[pEntity];

			const char* szName = "NPC";
			switch (pEntity->GetClassID())
			{
			case ETFClassID::CHeadlessHatman: szName = "Horseless Headless Horsemann"; break;
			case ETFClassID::CTFTankBoss: szName = "Tank"; break;
			case ETFClassID::CMerasmus: szName = "Merasmus"; break;
			case ETFClassID::CZombie: szName = "Skeleton"; break;
			case ETFClassID::CEyeballBoss: szName = "Monoculus";
			}

			tCache.m_vText.push_back({ TextTop, szName, Vars::Colors::NPC.Value });
		}
	}

	if (Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Health)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PICKUPS_HEALTH))
		{
			WorldCache& tCache = m_mWorldCache[pEntity];

			tCache.m_vText.push_back({ TextTop, "Health", Vars::Colors::Health.Value });
		}
	}

	if (Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Ammo)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PICKUPS_AMMO))
		{
			WorldCache& tCache = m_mWorldCache[pEntity];

			tCache.m_vText.push_back({ TextTop, "Ammo", Vars::Colors::Ammo.Value });
		}
	}

	if (Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Money)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PICKUPS_MONEY))
		{
			WorldCache& tCache = m_mWorldCache[pEntity];

			tCache.m_vText.push_back({ TextTop, "Money", Vars::Colors::Money.Value });
		}
	}

	if (Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Powerups)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PICKUPS_POWERUP))
		{
			WorldCache& tCache = m_mWorldCache[pEntity];

			const char* szName = "Powerup";
			switch (FNV1A::Hash32(I::ModelInfoClient->GetModelName(pEntity->GetModel())))
			{
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_agility.mdl"): szName = "Agility"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_crit.mdl"): szName = "Revenge"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_defense.mdl"): szName = "Resistance"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_haste.mdl"): szName = "Haste"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_king.mdl"): szName = "King"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_knockout.mdl"): szName = "Knockout"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_plague.mdl"): szName = "Plague"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_precision.mdl"): szName = "Precision"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_reflect.mdl"): szName = "Reflect"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_regen.mdl"): szName = "Regeneration"; break;
			//case FNV1A::Hash32Const("models/pickups/pickup_powerup_resistance.mdl"): szName = "11"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_strength.mdl"): szName = "Strength"; break;
			//case FNV1A::Hash32Const("models/pickups/pickup_powerup_strength_arm.mdl"): szName = "13"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_supernova.mdl"): szName = "Supernova"; break;
			//case FNV1A::Hash32Const("models/pickups/pickup_powerup_thorns.mdl"): szName = "15"; break;
			//case FNV1A::Hash32Const("models/pickups/pickup_powerup_uber.mdl"): szName = "16"; break;
			case FNV1A::Hash32Const("models/pickups/pickup_powerup_vampire.mdl"): szName = "Vampire";
			}
			tCache.m_vText.push_back({ TextTop, szName, Vars::Colors::Powerup.Value });
		}
	}

	if (Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Bombs)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_BOMBS))
		{
			WorldCache& tCache = m_mWorldCache[pEntity];

			tCache.m_vText.push_back({ TextTop, pEntity->GetClassID() == ETFClassID::CTFPumpkinBomb ? "Pumpkin Bomb" : "Bomb", Vars::Colors::Halloween.Value });
		}
	}

	if (Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Spellbook)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::PICKUPS_SPELLBOOK))
		{
			WorldCache& tCache = m_mWorldCache[pEntity];

			tCache.m_vText.push_back({ TextTop, "Spellbook", Vars::Colors::Halloween.Value });
		}
	}

	if (Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Gargoyle)
	{
		for (auto pEntity : H::Entities.GetGroup(EGroupType::WORLD_GARGOYLE))
		{
			WorldCache& tCache = m_mWorldCache[pEntity];

			tCache.m_vText.push_back({ TextTop, "Gargoyle", Vars::Colors::Halloween.Value });
		}
	}
}

void CESP::Draw()
{
	if (!Vars::ESP::Draw.Value)
		return;

	DrawWorld();
	DrawBuildings();
	DrawPlayers();
}

void CESP::DrawPlayers()
{
	if (!(Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Players) || !Vars::ESP::Player.Value)
		return;

	const auto& fFont = H::Fonts.GetFont(FONT_ESP);
	for (auto& [pEntity, tCache] : m_mPlayerCache)
	{
		int x = 0, y = 0, w = 0, h = 0;
		if (!GetDrawBounds(pEntity, x, y, w, h))
			continue;

		int m = x + w / 2, r = x + w + 4, b = y + h + 4;
		int lOffset = 0, rOffset = 0, bOffset = 0, tOffset = 0;

		I::MatSystemSurface->DrawSetAlphaMultiplier(tCache.m_flAlpha);
		
		if (tCache.m_bBox)
			H::Draw.LineRectOutline(x, y, w, h, tCache.m_tColor, { 0, 0, 0, 255 });

		if (tCache.m_bBones)
		{
			auto pPlayer = pEntity->As<CTFPlayer>();
			DrawBones(pPlayer, { 8, 7, 6, 4 }, tCache.m_tColor);
			DrawBones(pPlayer, { 11, 10, 9, 4 }, tCache.m_tColor);
			DrawBones(pPlayer, { 0, 4, 1 }, tCache.m_tColor);
			DrawBones(pPlayer, { 14, 13, 1 }, tCache.m_tColor);
			DrawBones(pPlayer, { 17, 16, 1 }, tCache.m_tColor);
		}

		if (tCache.m_bHealthBar)
		{
			if (tCache.m_flHealth > 1.f)
			{
				Color_t cColor = Vars::Colors::HealthBar.Value.EndColor;
				H::Draw.FillRectPercent(x - 6, y, 2, h, 1.f, cColor, { 0, 0, 0, 255 }, ALIGN_BOTTOM, true);

				cColor = Vars::Colors::Overheal.Value;
				H::Draw.FillRectPercent(x - 6, y, 2, h, tCache.m_flHealth - 1.f, cColor, { 0, 0, 0, 0 }, ALIGN_BOTTOM, true);
			}
			else
			{
				Color_t cColor = Vars::Colors::HealthBar.Value.StartColor.Lerp(Vars::Colors::HealthBar.Value.EndColor, tCache.m_flHealth);
				H::Draw.FillRectPercent(x - 6, y, 2, h, tCache.m_flHealth, cColor, { 0, 0, 0, 255 }, ALIGN_BOTTOM, true);
			}
			lOffset += 5;
		}

		if (tCache.m_bUberBar)
		{
			H::Draw.FillRectPercent(x, y + h + 4, w, 2, tCache.m_flUber, Vars::Colors::UberBar.Value);
			bOffset += 5;
		}

		for (auto& [iMode, sText, tColor] : tCache.m_vText)
		{
			switch (iMode)
			{
			case TextTop:
				H::Draw.String(fFont, m, y - tOffset, tColor, ALIGN_BOTTOM, sText.c_str());
				tOffset += fFont.m_nTall + 2;
				break;
			case TextBottom:
				H::Draw.String(fFont, m, b + bOffset, tColor, ALIGN_TOP, sText.c_str());
				bOffset += fFont.m_nTall + 2;
				break;
			case TextRight:
				H::Draw.String(fFont, r, y + rOffset, tColor, ALIGN_TOPLEFT, sText.c_str());
				rOffset += fFont.m_nTall + 2;
				break;
			case TextHealth:
				H::Draw.String(fFont, x - 5 - lOffset, y + h - h * std::min(tCache.m_flHealth, 1.f) - 2, tColor, ALIGN_TOPRIGHT, sText.c_str());
				break;
			case TextUber:
				H::Draw.String(fFont, x + w + 4, y + h, tColor, ALIGN_TOPLEFT, sText.c_str());
			}
		}

		if (tCache.m_iClassIcon)
		{
			int size = 18 * Vars::Menu::DPI.Value;
			H::Draw.Texture(x + w / 2, y - tOffset, size, size, tCache.m_iClassIcon, ALIGN_BOTTOM);
		}

		if (tCache.m_pWeaponIcon)
		{
			const float iw = tCache.m_pWeaponIcon->Width(), ih = tCache.m_pWeaponIcon->Height();
			const float scale = std::clamp(float(w) / std::max(iw, ih * 2), 0.25f, 0.75f) * Vars::Menu::DPI.Value;
			H::Draw.DrawHudTexture(x + float(w) / 2.f - iw / 2.f * scale, y + h + 1 + bOffset, scale, tCache.m_pWeaponIcon, Vars::Menu::Theme::Active.Value);
		}
	}

	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

void CESP::DrawBuildings()
{
	if (!(Vars::ESP::Draw.Value & Vars::ESP::DrawEnum::Buildings) || !Vars::ESP::Building.Value)
		return;

	const auto& fFont = H::Fonts.GetFont(FONT_ESP);
	for (auto& [pEntity, tCache] : m_mBuildingCache)
	{
		int x = 0, y = 0, w = 0, h = 0;
		if (!GetDrawBounds(pEntity, x, y, w, h))
			continue;

		int m = x + w / 2, r = x + w + 4, b = y + h + 4;
		int lOffset = 0, rOffset = 0, bOffset = 0, tOffset = 0;

		I::MatSystemSurface->DrawSetAlphaMultiplier(tCache.m_flAlpha);

		if (tCache.m_bBox)
			H::Draw.LineRectOutline(x, y, w, h, tCache.m_tColor, { 0, 0, 0, 255 });

		if (tCache.m_bHealthBar)
		{
			Color_t cColor = Vars::Colors::HealthBar.Value.StartColor.Lerp(Vars::Colors::HealthBar.Value.EndColor, tCache.m_flHealth);
			H::Draw.FillRectPercent(x - 6, y, 2, h, tCache.m_flHealth, cColor, { 0, 0, 0, 255 }, ALIGN_BOTTOM, true);
			lOffset += 5;
		}

		for (auto& [iMode, sText, tColor] : tCache.m_vText)
		{
			switch (iMode)
			{
			case TextTop:
				H::Draw.String(fFont, m, y - tOffset, tColor, ALIGN_BOTTOM, sText.c_str());
				tOffset += fFont.m_nTall + 2;
				break;
			case TextBottom:
				H::Draw.String(fFont, m, b + bOffset, tColor, ALIGN_TOP, sText.c_str());
				bOffset += fFont.m_nTall + 2;
				break;
			case TextRight:
				H::Draw.String(fFont, r, y + rOffset, tColor, ALIGN_TOPLEFT, sText.c_str());
				rOffset += fFont.m_nTall + 2;
				break;
			case TextHealth:
				H::Draw.String(fFont, x - 5 - lOffset, y + h - h * std::min(tCache.m_flHealth, 1.f) - 2, tColor, ALIGN_TOPRIGHT, sText.c_str());
				break;
			}
		}
	}

	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

void CESP::DrawWorld()
{
	const auto& fFont = H::Fonts.GetFont(FONT_ESP);
	for (auto& [pEntity, tCache] : m_mWorldCache)
	{
		int x = 0, y = 0, w = 0, h = 0;
		if (!GetDrawBounds(pEntity, x, y, w, h))
			continue;

		int m = x + w / 2, r = x + w + 4, b = y + h + 4;
		int lOffset = 0, rOffset = 0, bOffset = 0, tOffset = 0;

		I::MatSystemSurface->DrawSetAlphaMultiplier(tCache.m_flAlpha);

		if (tCache.m_bBox)
			H::Draw.LineRectOutline(x, y, w, h, tCache.m_tColor, { 0, 0, 0, 255 });

		for (auto& [iMode, sText, tColor] : tCache.m_vText)
		{
			switch (iMode)
			{
			case TextTop:
				H::Draw.String(fFont, m, y - tOffset, tColor, ALIGN_BOTTOM, sText.c_str());
				tOffset += fFont.m_nTall + 2;
				break;
			case TextBottom:
				H::Draw.String(fFont, m, b + bOffset, tColor, ALIGN_TOP, sText.c_str());
				bOffset += fFont.m_nTall + 2;
				break;
			case TextRight:
				H::Draw.String(fFont, r, y + rOffset, tColor, ALIGN_TOPLEFT, sText.c_str());
				rOffset += fFont.m_nTall + 2;
			}
		}
	}

	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

bool CESP::GetDrawBounds(CBaseEntity* pEntity, int& x, int& y, int& w, int& h)
{
	auto& transform = const_cast<matrix3x4&>(pEntity->RenderableToWorldTransform());
	if (pEntity->entindex() == I::EngineClient->GetLocalPlayer())
	{
		Math::AngleMatrix({ 0.f, I::EngineClient->GetViewAngles().y, 0.f }, transform);
		Math::MatrixSetColumn(pEntity->GetAbsOrigin(), 3, transform);
	}

	float flLeft, flRight, flTop, flBottom;
	if (!SDK::IsOnScreen(pEntity, transform, &flLeft, &flRight, &flTop, &flBottom))
		return false;

	x = flLeft;
	y = flBottom;
	w = flRight - flLeft;
	h = flTop - flBottom;

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFPlayer:
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
		x += w * 0.125f;
		w *= 0.75f;
	}

	return !(x > H::Draw.m_nScreenW || x + w < 0 || y > H::Draw.m_nScreenH || y + h < 0);
}

const char* CESP::GetPlayerClass(int iClassNum)
{
	static const char* szClasses[] = {
		"Unknown", "Scout", "Sniper", "Soldier", "Demoman",
		"Medic", "Heavy", "Pyro", "Spy", "Engineer"
	};

	return iClassNum < 10 && iClassNum > 0 ? szClasses[iClassNum] : szClasses[0];
}

void CESP::DrawBones(CTFPlayer* pPlayer, std::vector<int> vecBones, Color_t clr)
{
	for (size_t n = 1; n < vecBones.size(); n++)
	{
		const auto vBone1 = pPlayer->GetHitboxCenter(vecBones[n]);
		const auto vBone2 = pPlayer->GetHitboxCenter(vecBones[n - 1]);

		Vec3 vScreenBone, vScreenParent;
		if (SDK::W2S(vBone1, vScreenBone) && SDK::W2S(vBone2, vScreenParent))
			H::Draw.Line(vScreenBone.x, vScreenBone.y, vScreenParent.x, vScreenParent.y, clr);
	}
}