#include "ESP.h"

#include "../Groups/Groups.h"
#include "../../Players/PlayerUtils.h"
#include "../../Spectate/Spectate.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include <unordered_map>

MAKE_SIGNATURE(CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot, "client.dll", "48 89 6C 24 ? 56 41 54 41 55 41 56 41 57 48 83 EC", 0x0);
MAKE_SIGNATURE(CEconItemView_GetItemName, "client.dll", "40 53 48 83 EC ? 48 8B D9 C6 81 ? ? ? ? ? E8 ? ? ? ? 48 8B 8B", 0x0);


inline float Lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

// Stores previous health values for smoothing per player
static std::unordered_map<int, float> g_SmoothHealth;



static inline void StorePlayer(CTFPlayer* pPlayer, CTFPlayer* pLocal, Group_t* pGroup, std::unordered_map<CBaseEntity*, PlayerCache_t>& mCache)
{
	int iIndex = pPlayer->entindex();

	if (int iObserverMode = pLocal->m_iObserverMode();
		iObserverMode == OBS_MODE_FIRSTPERSON || iObserverMode == OBS_MODE_THIRDPERSON
		? !I::Input->CAM_IsThirdPerson() && iIndex == I::EngineClient->GetLocalPlayer()
		: iObserverMode == OBS_MODE_FIRSTPERSON && pLocal->m_hObserverTarget().GetEntryIndex() == iIndex)
		return;

	auto pWeapon = pPlayer->m_hActiveWeapon()->As<CTFWeaponBase>();
	auto pResource = H::Entities.GetResource();
	bool bLocal = pPlayer->entindex() == I::EngineClient->GetLocalPlayer();
	int iClassNum = pPlayer->m_iClass();

	PlayerCache_t& tCache = mCache[pPlayer];
	tCache.m_flAlpha = pGroup->m_tColor.a / 255.f;
	tCache.m_tColor = F::Groups.GetColor(pPlayer, pGroup).Alpha(255);
	tCache.m_bBox = pGroup->m_iESP & ESPEnum::Box;
	tCache.m_bBones = pGroup->m_iESP & ESPEnum::Bones;
	if (pGroup->m_iESP & ESPEnum::Distance && !bLocal)
	{
		Vec3 vDelta = pPlayer->m_vecOrigin() - pLocal->m_vecOrigin();
		tCache.m_vText.emplace_back(ALIGN_BOTTOM, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pResource)
	{
		if (pGroup->m_iESP & ESPEnum::Name)
			tCache.m_vText.emplace_back(
				ALIGN_TOP,
				F::PlayerUtils.GetPlayerName(iIndex, pResource->GetName(iIndex)),
				(pGroup->m_iESPOptions & ESPOptionsEnum::NameUseGroupColor) ? tCache.m_tColor : Vars::Colors::ElementsColor.Value,
				Vars::Colors::ElementsOutline.Value);

		if (pGroup->m_iESP & (ESPEnum::Labels | ESPEnum::Priority) && !pResource->IsFakePlayer(iIndex))
		{
			uint32_t uAccountID = pResource->m_iAccountID(iIndex);

			if (pGroup->m_iESP & ESPEnum::Priority)
			{
				if (auto pTag = F::PlayerUtils.GetSignificantTag(uAccountID, 1))
				{
					int mode = (pGroup->m_iESPOptions & ESPOptionsEnum::TagsDisplayBottom) ? ALIGN_PLAYER_TAG_BOTTOM : ALIGN_PLAYER_TAG_TOP;
					tCache.m_vText.emplace_back(mode, pTag->m_sName, pTag->m_tColor,
						pTag->m_tColor.IsColorDark() ? Color_t(255, 255, 255) : Color_t(0, 0, 0));
				}
			}

			if (pGroup->m_iESP & ESPEnum::Labels)
			{
				std::vector<std::tuple<std::string, Color_t, int>> vTags = {};
				for (auto& iID : F::PlayerUtils.GetPlayerTags(uAccountID))
				{
					auto pTag = F::PlayerUtils.GetTag(iID);
					if (pTag && pTag->m_bLabel)
						vTags.emplace_back(pTag->m_sName, pTag->m_tColor, pTag->m_iPriority);
				}
				if (H::Entities.IsFriend(uAccountID))
				{
					auto pTag = &F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(FRIEND_TAG)];
					if (pTag->m_bLabel)
						vTags.emplace_back(pTag->m_sName, pTag->m_tColor, pTag->m_iPriority);
				}

				if (!vTags.empty())
				{
					std::sort(vTags.begin(), vTags.end(), [&](const auto a, const auto b) -> bool
						{
							// sort by priority if unequal
							if (std::get<2>(a) != std::get<2>(b))
								return std::get<2>(a) > std::get<2>(b);

							return std::get<0>(a) < std::get<0>(b);
						});

					for (auto& [sName, tColor, _] : vTags)
						tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, sName, tColor, tColor.IsColorDark() ? Color_t(255, 255, 255) : Color_t(0, 0, 0));
				}
			}
		}
	}


	float flHealth = pPlayer->m_iHealth(), flMaxHealth = pPlayer->GetMaxHealth();

	if (pGroup->m_iESP & ESPEnum::HealthBar)
	{
		float realHealth = flHealth > flMaxHealth
			? 1.f + std::clamp((flHealth - flMaxHealth) / (floorf(flMaxHealth / 10.f) * 5), 0.f, 1.f)
			: std::clamp(flHealth / flMaxHealth, 0.f, 1.f);

		int idx = pPlayer->entindex();
		if (!g_SmoothHealth.count(idx))
			g_SmoothHealth[idx] = realHealth;

		float lerpT = (realHealth < g_SmoothHealth[idx]) ? Vars::ESP::DownwardBarLerp.Value : Vars::ESP::UpwardBarLerp.Value;

		float nigger = g_SmoothHealth[idx] = Lerp(g_SmoothHealth[idx], realHealth, lerpT);

		tCache.m_flHealth = realHealth;
		tCache.m_flSmoothHealth = nigger;

		nigger = std::clamp(nigger, 0.0f, 2.0f);

		float factor;
		Color_t tColor;

		if (nigger > 1.0f) // stupid overheal fix
		{	
			factor = std::clamp(nigger - 1.0f, 0.0f, 1.0f);
			tColor = Vars::Colors::FullHPColor.Value;
		}
		else
			if (nigger > 0.5f)
			{
				factor = (nigger - 0.5f) / 0.5f;
				tColor = Vars::Colors::MidHPColor.Value.Lerp(Vars::Colors::FullHPColor.Value, factor);
			}
			else
			{
				factor = std::clamp(nigger / 0.5f, 0.0f, 1.0f);
				tColor = Vars::Colors::LowHPColor.Value.Lerp(Vars::Colors::MidHPColor.Value, factor);
			}

		if (pPlayer->InCond(TF_COND_INVULNERABLE) ||
			pPlayer->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
			pPlayer->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
			pPlayer->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
		{
			tColor = Vars::Colors::UberColor.Value;
			tCache.m_vBars.emplace_back(ALIGN_LEFT, nigger, tColor, Vars::Colors::UberColor.Value, true);
		}
		else
		{
			tCache.m_vBars.emplace_back(ALIGN_LEFT, (nigger > 0.99f && nigger < 1.0f) ? 1.0f : nigger, tColor, Vars::Colors::OverhealColor.Value, true);

		}
	}

	if (pGroup->m_iESP & ESPEnum::HealthText)
		tCache.m_vText.emplace_back((pGroup->m_iESPOptions & ESPOptionsEnum::CenteredHealthText) ? ALIGN_CENTER : ALIGN_LEFT, std::format("{}", (int)flHealth), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	tCache.m_flHealth = flHealth > flMaxHealth ? 1.f + std::clamp((flHealth - flMaxHealth) / (floorf(flMaxHealth / 10.f) * 5), 0.f, 1.f) : std::clamp(flHealth / flMaxHealth, 0.f, 1.f);

	if (pGroup->m_iESP & (ESPEnum::UberBar | ESPEnum::UberText) && iClassNum == TF_CLASS_MEDIC)
	{
		auto pMediGun = pPlayer->GetWeaponFromSlot(SLOT_SECONDARY);
		if (pMediGun && pMediGun->GetClassID() == ETFClassID::CWeaponMedigun)
		{
			Color_t uberColor = Vars::Colors::UberBarLow.Value.Lerp(Vars::Colors::UberBarHigh.Value, std::clamp(pMediGun->As<CWeaponMedigun>()->m_flChargeLevel(), 0.f, 1.f));
			float flUber = std::clamp(pMediGun->As<CWeaponMedigun>()->m_flChargeLevel(), 0.f, 1.f);
			if (pGroup->m_iESP & ESPEnum::UberBar)
				tCache.m_vBars.emplace_back(ALIGN_BOTTOM, flUber, uberColor, Color_t(), true);
			if (pGroup->m_iESP & ESPEnum::UberText)
				tCache.m_vText.emplace_back(ALIGN_BOTTOMRIGHT, std::format("{:.0f}%", flUber * 100), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
		}
	}

	if (pGroup->m_iESP & ESPEnum::ClassIcon)
		tCache.m_iClassIcon = iClassNum;
	if (pGroup->m_iESP & ESPEnum::ClassText)
		tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, SDK::GetClassByIndex(iClassNum, false), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);

	if (pGroup->m_iESP & ESPEnum::WeaponIcon && pWeapon)
		tCache.m_pWeaponIcon = pWeapon->GetWeaponIcon();
	if (pGroup->m_iESP & ESPEnum::WeaponText && pWeapon)
	{
		auto pAttributeManager = U::Memory.CallVirtual<1, void*>(uintptr_t(pWeapon) + 3096);
		auto pCurItemData = reinterpret_cast<void*>(uintptr_t(pAttributeManager) + 144);
		tCache.m_vText.emplace_back(ALIGN_BOTTOM, SDK::ConvertWideToUTF8(S::CEconItemView_GetItemName.Call<const wchar_t*>(pCurItemData)), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pGroup->m_iESP & ESPEnum::LagCompensation && !pPlayer->IsDormant() && !bLocal)
	{
		if (H::Entities.GetLagCompensation(iIndex))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Lagcomp", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pGroup->m_iESP & ESPEnum::Ping && pResource && !bLocal)
	{
		int iPing = pResource->m_iPing(iIndex);
		if (iPing && (iPing >= 200 || iPing <= 5))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("{}MS", iPing), Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pGroup->m_iESP & ESPEnum::KDR && pResource && !bLocal)
	{
		int iKills = pResource->m_iScore(iIndex), iDeaths = pResource->m_iDeaths(iIndex);
		if (iKills >= 20)
		{
			int iKDR = iKills / std::max(iDeaths, 1);
			if (iKDR >= 10)
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("High KD [{} / {}]", iKills, iDeaths), Vars::Colors::IndicatorTextMid.Value, Vars::Colors::ElementsOutline.Value);
		}
	}

	// Buffs
	if (pGroup->m_iESP & ESPEnum::Buffs)
	{
		if (pPlayer->InCond(TF_COND_INVULNERABLE) ||
			pPlayer->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
			pPlayer->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
			pPlayer->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Uber", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		else if (pPlayer->InCond(TF_COND_MEGAHEAL))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Megaheal", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		else if (pPlayer->InCond(TF_COND_PHASE))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Bonk", Vars::Colors::IndicatorTextMid.Value, Vars::Colors::ElementsOutline.Value);

		bool bCrits = pPlayer->IsCritBoosted(), bMiniCrits = pPlayer->IsMiniCritBoosted();
		if (pWeapon)
		{
			if (bMiniCrits && SDK::AttribHookValue(0, "minicrits_become_crits", pWeapon)
				|| SDK::AttribHookValue(0, "crit_while_airborne", pWeapon) && pPlayer->InCond(TF_COND_BLASTJUMPING))
				bCrits = true, bMiniCrits = false;
			if (bCrits && SDK::AttribHookValue(0, "crits_become_minicrits", pWeapon))
				bCrits = false, bMiniCrits = true;
		}
		if (bCrits)
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Crits", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		else if (bMiniCrits)
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Mini-crits", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);

		/* vaccinator effects */
		if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) || pPlayer->InCond(TF_COND_BULLET_IMMUNE))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Bullet+", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_BULLET_RESIST))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Bullet", Color_t(246, 151, 255, 255), Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST) || pPlayer->InCond(TF_COND_BLAST_IMMUNE))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Blast+", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_BLAST_RESIST))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Blast", Color_t(246, 151, 255, 255), Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST) || pPlayer->InCond(TF_COND_FIRE_IMMUNE))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Fire+", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_FIRE_RESIST))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Fire", Color_t(246, 151, 255, 255), Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_OFFENSEBUFF))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Banner", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_DEFENSEBUFF))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Battalions", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_REGENONDAMAGEBUFF))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Conch", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_RUNE_STRENGTH))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Strength", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_HASTE))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Haste", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_REGEN))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Regen", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_RESIST))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Resistance", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_VAMPIRE))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Vampire", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_REFLECT))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Reflect", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_PRECISION))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Precision", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_AGILITY))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Agility", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_KNOCKOUT))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Knockout", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_KING))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "King", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_PLAGUE))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Plague", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_RUNE_SUPERNOVA))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Supernova", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
		if (pPlayer->InCond(TF_COND_POWERUPMODE_DOMINANT))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Dominant", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);

		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			auto pWeapon = pPlayer->GetWeaponFromSlot(i)->As<CTFSpellBook>();
			if (!pWeapon || pWeapon->GetWeaponID() != TF_WEAPON_SPELLBOOK || !pWeapon->m_iSpellCharges())
				continue;

			switch (pWeapon->m_iSelectedSpellIndex())
			{
			case 0: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Fireball", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 1: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Bats", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 2: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Heal", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 3: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Pumpkins", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 4: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Jump", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 5: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Stealth", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 6: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Teleport", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 7: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Lightning", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 8: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Minify", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 9: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Meteors", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 10: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Monoculus", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 11: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Skeletons", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 12: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Glove", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 13: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Parachute", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 14: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Heal", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			case 15: tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Bomb", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value); break;
			}
		}

		if (pPlayer->InCond(TF_COND_RADIUSHEAL) ||
			pPlayer->InCond(TF_COND_HEALTH_BUFF) ||
			pPlayer->InCond(TF_COND_RADIUSHEAL_ON_DAMAGE) ||
			pPlayer->InCond(TF_COND_HALLOWEEN_QUICK_HEAL) ||
			pPlayer->InCond(TF_COND_HALLOWEEN_HELL_HEAL) ||
			pPlayer->InCond(TF_COND_KING_BUFFED))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Heal", Vars::Colors::IndicatorTextGood.Value, Vars::Colors::ElementsOutline.Value);
		else if (pPlayer->InCond(TF_COND_HEALTH_OVERHEALED))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "HP+", Vars::Colors::IndicatorTextGood.Value, Vars::Colors::ElementsOutline.Value);

		//if (pPlayer->InCond(TF_COND_BLASTJUMPING))
		//	tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Blastjump", Vars::Colors::IndicatorTextMid.Value, Vars::Colors::ElementsOutline.Value);
	}

	// Debuffs
	if (pGroup->m_iESP & ESPEnum::Debuffs)
	{
		if (pPlayer->InCond(TF_COND_MARKEDFORDEATH)
			|| pPlayer->InCond(TF_COND_MARKEDFORDEATH_SILENT)
			|| pPlayer->InCond(TF_COND_PASSTIME_PENALTY_DEBUFF))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Marked", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_URINE))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Jarate", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_MAD_MILK))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Milk", Vars::Menu::Theme::Inactive.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_STUNNED))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Stun", Vars::Colors::IndicatorMisc.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_BURNING))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Burn", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_BLEEDING))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Bleed", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
	}

	// Misc
	if (pGroup->m_iESP & ESPEnum::Flags)
	{
		if (pPlayer->m_bFeignDeathReady())
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "DR", Vars::Colors::IndicatorMisc.Value, Vars::Colors::ElementsOutline.Value);
		else if (pPlayer->InCond(TF_COND_FEIGN_DEATH))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Feign", Vars::Colors::IndicatorMisc.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->m_flInvisibility())
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("{:.0f}%", pPlayer->m_flInvisibility() * 100), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_DISGUISED))
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Disguise", Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);

		if (pPlayer->InCond(TF_COND_AIMING) || pPlayer->InCond(TF_COND_ZOOMED))
		{
			switch (pWeapon ? pWeapon->GetWeaponID() : -1)
			{
			case TF_WEAPON_MINIGUN:
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Rev", Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
				break;
			case TF_WEAPON_SNIPERRIFLE:
			case TF_WEAPON_SNIPERRIFLE_CLASSIC:
			case TF_WEAPON_SNIPERRIFLE_DECAP:
			{
				if (bLocal)
				{
					tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("Zoom {:.0f}%", Math::RemapVal(pWeapon->As<CTFSniperRifle>()->m_flChargedDamage(), 0.f, 150.f, 0.f, 100.f)), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
					break;
				}
				else
				{
					auto GetSniperDot = [](CBaseEntity* pEntity) -> CSniperDot*
						{
							for (auto pDot : H::Entities.GetGroup(EntityEnum::SniperDots))
							{
								if (pDot->m_hOwnerEntity().Get() == pEntity)
									return pDot->As<CSniperDot>();
							}
							return nullptr;
						};
					if (CSniperDot* pPlayerDot = GetSniperDot(pPlayer))
					{
						float flChargeTime = std::max(SDK::AttribHookValue(3.f, "mult_sniper_charge_per_sec", pWeapon), 1.5f);
						tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("{:.0f}%", Math::RemapVal(TICKS_TO_TIME(I::ClientState->m_ClockDriftMgr.m_nServerTick) - pPlayerDot->m_flChargeStartTime() - 0.3f, 0.f, flChargeTime, 0.f, 100.f)), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
						break;
					}
				}
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Charging", Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
				break;
			}
			case TF_WEAPON_COMPOUND_BOW:
				if (bLocal)
				{
					tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("Charging {:.0f}%", Math::RemapVal(TICKS_TO_TIME(I::ClientState->m_ClockDriftMgr.m_nServerTick) - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime(), 0.f, 1.f, 0.f, 100.f)), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
					break;
				}
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Charging", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
				break;
			default:
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Charging", Vars::Colors::IndicatorMid.Value, Vars::Colors::ElementsOutline.Value);
			}
		}
	}
}

static inline void StoreBuilding(CBaseObject* pBuilding, CTFPlayer* pLocal, Group_t* pGroup, std::unordered_map<CBaseEntity*, BuildingCache_t>& mCache)
{
	auto pOwner = pBuilding->m_hBuilder().Get();
	int iIndex = pOwner ? pOwner->entindex() : -1;

	bool bIsMini = pBuilding->m_bMiniBuilding();

	BuildingCache_t& tCache = mCache[pBuilding];
	tCache.m_flAlpha = pGroup->m_tColor.a / 255.f;
	tCache.m_tColor = F::Groups.GetColor(pOwner ? pOwner : pBuilding, pGroup).Alpha(255);
	tCache.m_bBox = pGroup->m_iESP & ESPEnum::Box;

	if (pGroup->m_iESP & ESPEnum::Distance)
	{
		Vec3 vDelta = pBuilding->m_vecOrigin() - pLocal->m_vecOrigin();
		tCache.m_vText.emplace_back(ALIGN_BOTTOM, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pGroup->m_iESP & ESPEnum::Name)
	{
		const char* sName = "Building";
		switch (pBuilding->GetClassID())
		{
		case ETFClassID::CObjectSentrygun: sName = bIsMini ? "Mini Sentry" : "Sentry"; break;
		case ETFClassID::CObjectDispenser: sName = "Dispenser"; break;
		case ETFClassID::CObjectTeleporter: sName = pBuilding->m_iObjectMode() ? "Teleporter Exit" : "Teleporter";
		}
		tCache.m_vText.emplace_back(ALIGN_TOP, sName, (pGroup->m_iESPOptions & ESPOptionsEnum::NameUseGroupColor) ? tCache.m_tColor : Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	float flHealth = pBuilding->m_iHealth(), flMaxHealth = pBuilding->m_iMaxHealth();



	if (pGroup->m_iESP & ESPEnum::HealthBar)
	{
		float realHealth = flHealth > flMaxHealth
			? 1.f + std::clamp((flHealth - flMaxHealth) / (floorf(flMaxHealth / 10.f) * 5), 0.f, 1.f)
			: std::clamp(flHealth / flMaxHealth, 0.f, 1.f);

		int idx = pBuilding->entindex();
		if (!g_SmoothHealth.count(idx))
			g_SmoothHealth[idx] = realHealth;


		float lerpT = (realHealth < g_SmoothHealth[idx]) ? Vars::ESP::DownwardBarLerp.Value : Vars::ESP::UpwardBarLerp.Value;

		float smoothHealth = g_SmoothHealth[idx] = Lerp(g_SmoothHealth[idx], realHealth, lerpT);


		tCache.m_flHealth = realHealth;
		tCache.m_flSmoothHealth = smoothHealth;

		smoothHealth = std::clamp(smoothHealth, 0.0f, 2.0f);

		float factor;
		Color_t tColor;

		if (smoothHealth > 1.0f) // stupid overheal fix
		{
			factor = std::clamp(smoothHealth - 1.0f, 0.0f, 1.0f);
			tColor = Vars::Colors::FullHPColor.Value;
		}
		else if (smoothHealth > 0.5f)
		{
			factor = (smoothHealth - 0.5f) / 0.5f;
			tColor = Vars::Colors::MidHPColor.Value.Lerp(Vars::Colors::FullHPColor.Value, factor);
		}
		else
		{
			factor = std::clamp(smoothHealth / 0.5f, 0.0f, 1.0f);
			tColor = Vars::Colors::LowHPColor.Value.Lerp(Vars::Colors::MidHPColor.Value, factor);
		}
		tCache.m_vBars.emplace_back(ALIGN_LEFT, (smoothHealth > 0.99f && smoothHealth < 1.0f) ? 1.0f : smoothHealth, tColor, Vars::Colors::OverhealColor.Value, true);
	}


	if (pGroup->m_iESP & ESPEnum::HealthText)
		tCache.m_vText.emplace_back((pGroup->m_iESPOptions & ESPOptionsEnum::CenteredHealthText) ? ALIGN_CENTER : ALIGN_LEFT, std::format("{}", (int)flHealth), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);

	if (pGroup->m_iESP & (ESPEnum::AmmoBars | ESPEnum::AmmoText) && pBuilding->IsSentrygun() && !pBuilding->m_bBuilding())
	{
		int iShells, iMaxShells, iRockets, iMaxRockets; pBuilding->As<CObjectSentrygun>()->GetAmmoCount(iShells, iMaxShells, iRockets, iMaxRockets);

		if (pGroup->m_iESP & ESPEnum::AmmoBars)
		{
			tCache.m_vBars.emplace_back(ALIGN_BOTTOM, float(iShells) / iMaxShells, Vars::Menu::Theme::Inactive.Value, Color_t(), true);
			if (iMaxRockets)
				tCache.m_vBars.emplace_back(ALIGN_BOTTOM, float(iRockets) / iMaxRockets, Vars::Menu::Theme::Inactive.Value, Color_t(), true);
		}
		if (pGroup->m_iESP & ESPEnum::AmmoText)
		{
			tCache.m_vText.emplace_back(ALIGN_BOTTOMRIGHT, std::format("{}", iShells), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
			if (iMaxRockets)
				tCache.m_vText.back().m_sText += std::format(", {}", iRockets);
		}
	}

	if (pGroup->m_iESP & ESPEnum::Owner && !pBuilding->m_bWasMapPlaced() && pOwner)
	{
		if (auto pResource = H::Entities.GetResource(); pResource)
			tCache.m_vText.emplace_back(ALIGN_TOP, F::PlayerUtils.GetPlayerName(iIndex, pResource->GetName(iIndex)), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pGroup->m_iESP & ESPEnum::Level && !bIsMini)
		tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("Level {}", pBuilding->m_iUpgradeLevel()), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);

	if (pGroup->m_iESP & ESPEnum::Flags)
	{
		if (!pBuilding->IsDormant() && pBuilding->m_bBuilding())
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("{:.0f}%", pBuilding->m_flPercentageConstructed() * 100), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);

		if (pBuilding->IsSentrygun() && pBuilding->As<CObjectSentrygun>()->m_bPlayerControlled())
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Wrangled", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);

		if (pBuilding->m_bHasSapper())
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Sapped", Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
		else if (pBuilding->m_bDisabled())
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Disabled", Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}
}

static inline const char* GetProjectileName(CBaseEntity* pProjectile)
{
	const char* sReturn = "Projectile";
	switch (pProjectile->GetClassID())
	{
	case ETFClassID::CTFWeaponBaseMerasmusGrenade: sReturn = "Bomb"; break;
	case ETFClassID::CTFGrenadePipebombProjectile: sReturn = pProjectile->As<CTFGrenadePipebombProjectile>()->HasStickyEffects() ? "Sticky" : "Pipe"; break;
	case ETFClassID::CTFStunBall: sReturn = "Baseball"; break;
	case ETFClassID::CTFBall_Ornament: sReturn = "Bauble"; break;
	case ETFClassID::CTFProjectile_Jar: sReturn = "Jarate"; break;
	case ETFClassID::CTFProjectile_Cleaver: sReturn = "Cleaver"; break;
	case ETFClassID::CTFProjectile_JarGas: sReturn = "Gas"; break;
	case ETFClassID::CTFProjectile_JarMilk:
	case ETFClassID::CTFProjectile_ThrowableBreadMonster: sReturn = "Milk"; break;
	case ETFClassID::CTFProjectile_SpellBats:
	case ETFClassID::CTFProjectile_SpellKartBats: sReturn = "Bats"; break;
	case ETFClassID::CTFProjectile_SpellMeteorShower: sReturn = "Meteor shower"; break;
	case ETFClassID::CTFProjectile_SpellMirv:
	case ETFClassID::CTFProjectile_SpellPumpkin: sReturn = "Pumpkin"; break;
	case ETFClassID::CTFProjectile_SpellSpawnBoss: sReturn = "Monoculus"; break;
	case ETFClassID::CTFProjectile_SpellSpawnHorde:
	case ETFClassID::CTFProjectile_SpellSpawnZombie: sReturn = "Skeleton"; break;
	case ETFClassID::CTFProjectile_SpellTransposeTeleport: sReturn = "Teleport"; break;
	case ETFClassID::CTFProjectile_Arrow: sReturn = pProjectile->As<CTFProjectile_Arrow>()->m_iProjectileType() == TF_PROJECTILE_BUILDING_REPAIR_BOLT ? "Repair" : "Arrow"; break;
	case ETFClassID::CTFProjectile_GrapplingHook: sReturn = "Grapple"; break;
	case ETFClassID::CTFProjectile_HealingBolt: sReturn = "Heal"; break;
	case ETFClassID::CTFProjectile_Rocket:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_SentryRocket: sReturn = "Rocket"; break;
	case ETFClassID::CTFProjectile_BallOfFire: sReturn = "Fire"; break;
	case ETFClassID::CTFProjectile_MechanicalArmOrb: sReturn = "Short circuit"; break;
	case ETFClassID::CTFProjectile_SpellFireball: sReturn = "Fireball"; break;
	case ETFClassID::CTFProjectile_SpellLightningOrb: sReturn = "Lightning"; break;
	case ETFClassID::CTFProjectile_SpellKartOrb: sReturn = "Fist"; break;
	case ETFClassID::CTFProjectile_Flare: sReturn = "Flare"; break;
	case ETFClassID::CTFProjectile_EnergyRing: sReturn = "Energy"; break;
	}
	return sReturn;
}
static inline void StoreProjectile(CBaseEntity* pProjectile, CTFPlayer* pLocal, Group_t* pGroup, std::unordered_map<CBaseEntity*, EntityCache_t>& mCache)
{
	auto pOwner = F::ProjSim.GetEntities(pProjectile).second;
	int iIndex = pOwner ? pOwner->entindex() : -1;

	EntityCache_t& tCache = mCache[pProjectile];
	tCache.m_flAlpha = pGroup->m_tColor.a / 255.f;
	tCache.m_tColor = F::Groups.GetColor(pOwner ? pOwner : pProjectile, pGroup);
	tCache.m_bBox = pGroup->m_iESP & ESPEnum::Box;

	if (pGroup->m_iESP & ESPEnum::Distance)
	{
		Vec3 vDelta = pProjectile->m_vecOrigin() - pLocal->m_vecOrigin();
		tCache.m_vText.emplace_back(ALIGN_BOTTOM, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pGroup->m_iESP & ESPEnum::Name)
		tCache.m_vText.emplace_back(ALIGN_TOP, GetProjectileName(pProjectile), (pGroup->m_iESPOptions & ESPOptionsEnum::NameUseGroupColor) ? tCache.m_tColor : Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);

	if (pGroup->m_iESP & ESPEnum::Owner && pOwner)
	{
		if (auto pResource = H::Entities.GetResource(); pResource)
			tCache.m_vText.emplace_back(ALIGN_TOP, F::PlayerUtils.GetPlayerName(iIndex, pResource->GetName(iIndex)), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pGroup->m_iESP & ESPEnum::Flags)
	{
		switch (pProjectile->GetClassID())
		{
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
			if (pProjectile->As<CTFWeaponBaseGrenadeProj>()->m_bCritical())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Crit", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			if (pProjectile->As<CTFWeaponBaseGrenadeProj>()->m_iDeflected() && (pProjectile->GetClassID() != ETFClassID::CTFGrenadePipebombProjectile || !pProjectile->GetAbsVelocity().IsZero()))
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Reflected", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			break;
		case ETFClassID::CTFProjectile_Arrow:
		case ETFClassID::CTFProjectile_GrapplingHook:
		case ETFClassID::CTFProjectile_HealingBolt:
			if (pProjectile->As<CTFProjectile_Arrow>()->m_bCritical())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Crit", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			if (pProjectile->As<CTFBaseRocket>()->m_iDeflected())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Reflected", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			if (pProjectile->As<CTFProjectile_Arrow>()->m_bArrowAlight())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Alight", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			break;
		case ETFClassID::CTFProjectile_Rocket:
		case ETFClassID::CTFProjectile_BallOfFire:
		case ETFClassID::CTFProjectile_MechanicalArmOrb:
		case ETFClassID::CTFProjectile_SentryRocket:
		case ETFClassID::CTFProjectile_SpellFireball:
		case ETFClassID::CTFProjectile_SpellLightningOrb:
		case ETFClassID::CTFProjectile_SpellKartOrb:
			if (pProjectile->As<CTFProjectile_Rocket>()->m_bCritical())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Crit", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			if (pProjectile->As<CTFBaseRocket>()->m_iDeflected())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Reflected", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			break;
		case ETFClassID::CTFProjectile_EnergyBall:
			if (pProjectile->As<CTFProjectile_EnergyBall>()->m_bChargedShot())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Charge", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			if (pProjectile->As<CTFBaseRocket>()->m_iDeflected())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Reflected", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			break;
		case ETFClassID::CTFProjectile_Flare:
			if (pProjectile->As<CTFProjectile_Flare>()->m_bCritical())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Crit", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			if (pProjectile->As<CTFBaseRocket>()->m_iDeflected())
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Reflected", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			break;
		}
	}
}

static inline void StoreObjective(CBaseEntity* pObjective, CTFPlayer* pLocal, Group_t* pGroup, std::unordered_map<CBaseEntity*, EntityCache_t>& mCache)
{
	auto pOwner = pObjective->m_hOwnerEntity()->As<CTFPlayer>();
	if (pOwner == pLocal)
		return;

	EntityCache_t& tCache = mCache[pObjective];
	tCache.m_flAlpha = pGroup->m_tColor.a / 255.f;
	tCache.m_tColor = F::Groups.GetColor(pObjective, pGroup);
	tCache.m_bBox = pGroup->m_iESP & ESPEnum::Box;

	if (pGroup->m_iESP & ESPEnum::Distance)
	{
		Vec3 vDelta = pObjective->m_vecOrigin() - pLocal->m_vecOrigin();
		tCache.m_vText.emplace_back(ALIGN_BOTTOM, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	switch (pObjective->GetClassID())
	{
	case ETFClassID::CCaptureFlag:
	{
		auto pIntel = pObjective->As<CCaptureFlag>();

		if (pGroup->m_iESP & ESPEnum::Name)
			tCache.m_vText.emplace_back(ALIGN_TOP, "Objective", Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);

		if (pGroup->m_iESP & ESPEnum::Flags)
		{
			switch (pIntel->m_nFlagStatus())
			{
			case TF_FLAGINFO_HOME:
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Home", Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
				break;
			case TF_FLAGINFO_DROPPED:
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Dropped", Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
				break;
			default:
				tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, "Stolen", Vars::Colors::IndicatorTextBad.Value, Vars::Colors::ElementsOutline.Value);
			}
		}

		if (pGroup->m_iESP & ESPEnum::IntelReturnTime && pIntel->m_nFlagStatus() == TF_FLAGINFO_DROPPED)
		{
			float flReturnTime = std::max(pIntel->m_flResetTime() - TICKS_TO_TIME(I::ClientState->m_ClockDriftMgr.m_nServerTick), 0.f);
			tCache.m_vText.emplace_back(ALIGN_TOPRIGHT, std::format("Return {:.1f}s", pIntel->m_flResetTime() - TICKS_TO_TIME(I::ClientState->m_ClockDriftMgr.m_nServerTick)).c_str(), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
		}

		break;
	}
	}
}

static inline void StoreMisc(CBaseEntity* pEntity, CTFPlayer* pLocal, Group_t* pGroup, std::unordered_map<CBaseEntity*, EntityCache_t>& mCache)
{
	EntityCache_t& tCache = mCache[pEntity];
	tCache.m_flAlpha = pGroup->m_tColor.a / 255.f;
	tCache.m_tColor = F::Groups.GetColor(pEntity, pGroup);
	tCache.m_bBox = pGroup->m_iESP & ESPEnum::Box;

	if (pGroup->m_iESP & ESPEnum::Distance)
	{
		Vec3 vDelta = pEntity->m_vecOrigin() - pLocal->m_vecOrigin();
		tCache.m_vText.emplace_back(ALIGN_BOTTOM, std::format("[{:.0f}M]", vDelta.Length2D() / 41), Vars::Colors::ElementsColor.Value, Vars::Colors::ElementsOutline.Value);
	}

	if (pGroup->m_iESP & ESPEnum::Name)
	{
		const char* sName = "Unknown";
		switch (pEntity->GetClassID())
		{
		case ETFClassID::CTFBaseBoss: sName = "Boss"; break;
		case ETFClassID::CTFTankBoss: sName = "Tank"; break;
		case ETFClassID::CMerasmus: sName = "Merasmus"; break;
		case ETFClassID::CEyeballBoss: sName = "Monoculus"; break;
		case ETFClassID::CHeadlessHatman: sName = "Horsemann"; break;
		case ETFClassID::CZombie: sName = "Skeleton"; break;
		case ETFClassID::CBaseAnimating:
		{
			auto uHash = H::Entities.GetModel(pEntity->entindex());
			if (H::Entities.IsHealth(uHash))
				sName = "Health";
			else if (H::Entities.IsAmmo(uHash))
				sName = "Ammo";
			else if (H::Entities.IsSpellbook(uHash))
				sName = "Spells";
			else if (H::Entities.IsPowerup(uHash))
			{
				sName = "Powerup";
				switch (uHash)
				{
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_agility.mdl"): sName = "Agility"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_crit.mdl"): sName = "Revenge"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_defense.mdl"): sName = "Resistance"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_haste.mdl"): sName = "Haste"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_king.mdl"): sName = "King"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_knockout.mdl"): sName = "Knockout"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_plague.mdl"): sName = "Plague"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_precision.mdl"): sName = "Precision"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_reflect.mdl"): sName = "Reflect"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_regen.mdl"): sName = "Regeneration"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_strength.mdl"): sName = "Strength"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_supernova.mdl"): sName = "Supernova"; break;
				case FNV1A::Hash32Const("models/pickups/pickup_powerup_vampire.mdl"): sName = "Vampire";
				}
			}
			break;
		}
		case ETFClassID::CTFAmmoPack: sName = "Ammo"; break;
		case ETFClassID::CCurrencyPack: sName = "Money"; break;
		case ETFClassID::CTFGenericBomb:
		case ETFClassID::CTFPumpkinBomb: sName = "Bomb"; break;
		case ETFClassID::CHalloweenGiftPickup: sName = "Gargoyle"; break;
		}

		tCache.m_vText.emplace_back(ALIGN_TOP, sName, pGroup->m_tColor, Vars::Colors::ElementsOutline.Value);
	}
}

void CESP::Store(CTFPlayer* pLocal)
{
	m_mPlayerCache.clear();
	m_mBuildingCache.clear();
	m_mEntityCache.clear();
	if (!pLocal || !F::Groups.GroupsActive())
		return;

	for (auto& [pEntity, pGroup] : F::Groups.GetGroup(false))
	{
		if (!pGroup->m_iESP)
			continue;

		if (pEntity->IsPlayer())
			StorePlayer(pEntity->As<CTFPlayer>(), pLocal, pGroup, m_mPlayerCache);
		else if (pEntity->IsBuilding())
			StoreBuilding(pEntity->As<CBaseObject>(), pLocal, pGroup, m_mBuildingCache);
		else if (pEntity->IsProjectile())
			StoreProjectile(pEntity, pLocal, pGroup, m_mEntityCache);
		else if (pEntity->GetClassID() == ETFClassID::CCaptureFlag)
			StoreObjective(pEntity, pLocal, pGroup, m_mEntityCache);
		else
			StoreMisc(pEntity, pLocal, pGroup, m_mEntityCache);
	}
}

void CESP::Draw()
{
	DrawWorld();
	DrawBuildings();
	DrawPlayers();
}

void CESP::DrawPlayers()
{
	if (m_mPlayerCache.empty())
		return;

	const auto& fFont = H::Fonts.GetFont(FONT_ESP);
	const auto& fFont2 = H::Fonts.GetFont(FONT_FLAGS);
	const auto& fFont3 = H::Fonts.GetFont(FONT_TAGS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(2);
	for (auto& [pEntity, tCache] : m_mPlayerCache)
	{
		float x, y, w, h;
		if (!GetDrawBounds(pEntity, x, y, w, h))
			continue;

		int l = x - H::Draw.Scale(6), r = x + w + H::Draw.Scale(8), m = x + w / 2;
		int t = y - H::Draw.Scale(5), b = y + h + H::Draw.Scale(5);
		int lOffset = 0, rOffset = 0, bOffset = 0, tOffset = 0;
		I::MatSystemSurface->DrawSetAlphaMultiplier(tCache.m_flAlpha);

		if (tCache.m_bBox)
			H::Draw.LineRectOutline(x, y, w, h, tCache.m_tColor, { 0, 0, 0, 255 });

		if (tCache.m_bBones)
		{
			auto pPlayer = pEntity->As<CTFPlayer>();

			Color_t boneColor;
			float BonesHealth = std::clamp(tCache.m_flHealth, 0.f, 1.f);

			// Health gradient coloring
			if (BonesHealth > 0.5f)
			{
				// Interpolate between Yellow (Mid) and Green (High)
				float factor = (BonesHealth - 0.5f) / 0.5f;
				boneColor = Vars::Colors::MidHPColor.Value.Lerp(Vars::Colors::FullHPColor.Value, factor);
			}
			else
			{
				// Interpolate between Red (Low) and Yellow (Mid)
				float factor = BonesHealth / 0.5f;
				boneColor = Vars::Colors::LowHPColor.Value.Lerp(Vars::Colors::MidHPColor.Value, factor);
			}

			// Override if Ubered
			if (pPlayer->InCond(TF_COND_INVULNERABLE) ||
				pPlayer->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
				pPlayer->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
				pPlayer->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
			{
				boneColor = Vars::Colors::UberColor.Value;
			}
			// Override if overhealed
			else if (pPlayer->m_iHealth() > pPlayer->GetMaxHealth())
			{
				boneColor = tCache.m_tColor;
			}

			matrix3x4 aBones[MAXSTUDIOBONES];
			if (pPlayer->SetupBones(aBones, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, I::GlobalVars->curtime))
			{
				int iHead = pPlayer->GetBaseToHitbox(HITBOX_HEAD);
				int iSpine2 = pPlayer->GetBaseToHitbox(HITBOX_SPINE2);
				int iPelvis = pPlayer->GetBaseToHitbox(HITBOX_PELVIS);
				int iLeftUpperarm = pPlayer->GetBaseToHitbox(HITBOX_LEFT_UPPERARM);
				int iLeftForearm = pPlayer->GetBaseToHitbox(HITBOX_LEFT_FOREARM);
				int iLeftHand = pPlayer->GetBaseToHitbox(HITBOX_LEFT_HAND);
				int iRightUpperarm = pPlayer->GetBaseToHitbox(HITBOX_RIGHT_UPPERARM);
				int iRightForearm = pPlayer->GetBaseToHitbox(HITBOX_RIGHT_FOREARM);
				int iRightHand = pPlayer->GetBaseToHitbox(HITBOX_RIGHT_HAND);
				int iLeftThigh = pPlayer->GetBaseToHitbox(HITBOX_LEFT_THIGH);
				int iLeftCalf = pPlayer->GetBaseToHitbox(HITBOX_LEFT_CALF);
				int iLeftFoot = pPlayer->GetBaseToHitbox(HITBOX_LEFT_FOOT);
				int iRightThigh = pPlayer->GetBaseToHitbox(HITBOX_RIGHT_THIGH);
				int iRightCalf = pPlayer->GetBaseToHitbox(HITBOX_RIGHT_CALF);
				int iRightFoot = pPlayer->GetBaseToHitbox(HITBOX_RIGHT_FOOT);

				DrawBones(pPlayer, aBones, { iHead, iSpine2, iPelvis }, boneColor);
				DrawBones(pPlayer, aBones, { iSpine2, iLeftUpperarm, iLeftForearm, iLeftHand }, boneColor);
				DrawBones(pPlayer, aBones, { iSpine2, iRightUpperarm, iRightForearm, iRightHand }, boneColor);
				DrawBones(pPlayer, aBones, { iPelvis, iLeftThigh, iLeftCalf, iLeftFoot }, boneColor);
				DrawBones(pPlayer, aBones, { iPelvis, iRightThigh, iRightCalf, iRightFoot }, boneColor);
			}
		}
		for (auto& [iMode, flPercent, tColor, tOverfill, bAdjust] : tCache.m_vBars)
		{
			auto drawBar = [&](int x, int y, int w, int h, EAlign eAlign = ALIGN_LEFT)
				{
					Color_t outline = { 0, 0, 0, 255 };

					int nw = w, nh = h;
					int fx = x, fy = y;
					float clampedPercent = std::min(flPercent, 1.f);

					switch (eAlign)
					{
					case ALIGN_LEFT:   nw = w * clampedPercent; break;
					case ALIGN_RIGHT:  nw = w * clampedPercent; fx += w - nw; break;
					case ALIGN_TOP:    nh = h * clampedPercent; break;
					case ALIGN_BOTTOM: nh = h * clampedPercent; fy += h - nh; break;
					}

					H::Draw.FillRectOutline(x, y, w, h, Vars::Colors::BarFill.Value, outline);
					H::Draw.FillRect(fx, fy, nw, nh, flPercent > 1.f ? tOverfill : tColor);
				};

			int iSpace = H::Draw.Scale(4);
			int iThickness = H::Draw.Scale(2, Scale_Round);
			switch (iMode)
			{
			case ALIGN_LEFT:
				drawBar(l - lOffset, y, iThickness, h, ALIGN_BOTTOM);
				lOffset += iSpace + iThickness;
				break;
			case ALIGN_BOTTOM:
				drawBar(x, b + bOffset, w, iThickness);
				bOffset += iSpace + iThickness;
				break;
			}
		}


		for (auto& [iMode, sText, tColor, tOutline] : tCache.m_vText)
		{
			switch (iMode)
			{
			case ALIGN_TOP:
				H::Draw.StringOutlined(fFont, m, t - tOffset, tColor, tOutline, ALIGN_BOTTOM, sText.c_str());
				tOffset += nTall;
				break;
			case ALIGN_BOTTOM:
				H::Draw.StringOutlined(fFont, m, b + bOffset, tColor, tOutline, ALIGN_TOP, sText.c_str());
				bOffset += nTall;
				break;
			case ALIGN_PLAYER_TAG_TOP:
				H::Draw.StringOutlined(fFont3, m, t - tOffset, tColor, tOutline, ALIGN_BOTTOM, sText.c_str());
				tOffset += nTall;
				break;
			case ALIGN_PLAYER_TAG_BOTTOM:
				H::Draw.StringOutlined(fFont3, m, b + bOffset, tColor, tOutline, ALIGN_TOP, sText.c_str());
				bOffset += nTall;
				break;
			case ALIGN_LEFT:
			{
				float flDrawPercent = std::min(tCache.m_flSmoothHealth, 1.0f);
				float flRelativeY = y - H::Draw.Scale(2) + h - (h * flDrawPercent);
				H::Draw.StringOutlined(fFont2, l - lOffset, flRelativeY, tColor, tOutline, ALIGN_TOPRIGHT, sText.c_str());
			}
			break;
			case ALIGN_CENTER:
			{
				int iBarSpace = H::Draw.Scale(4);
				int iBarThickness = H::Draw.Scale(2, Scale_Round);
				int barX = x - iBarSpace - (iBarThickness / 2);

				float flDrawPercent = std::clamp(tCache.m_flSmoothHealth, 0.f, 1.f);
				float flRelativeY = y - H::Draw.Scale(2) + h - (h * flDrawPercent);
				H::Draw.StringOutlined(fFont2, barX, flRelativeY, tColor, tOutline, ALIGN_CENTER, sText.c_str());
			}
			break;

			case ALIGN_TOPRIGHT:
				H::Draw.StringOutlined(fFont2, r, y - H::Draw.Scale(2) + rOffset, tColor, tOutline, ALIGN_TOPLEFT, sText.c_str());
				rOffset += nTall;
				break;
			case ALIGN_BOTTOMRIGHT:
				H::Draw.StringOutlined(fFont2, r, y + h, tColor, tOutline, ALIGN_TOPLEFT, sText.c_str());
				break;
			}
		}
		if (tCache.m_iClassIcon)
		{
			const char* sTexture = "vgui/glyph_multiplayer.vtf";
			switch (tCache.m_iClassIcon)
			{
			case TF_CLASS_SCOUT: sTexture = "hud/leaderboard_class_scout.vtf"; break;
			case TF_CLASS_SOLDIER: sTexture = "hud/leaderboard_class_soldier.vtf"; break;
			case TF_CLASS_PYRO: sTexture = "hud/leaderboard_class_pyro.vtf"; break;
			case TF_CLASS_DEMOMAN: sTexture = "hud/leaderboard_class_demo.vtf"; break;
			case TF_CLASS_HEAVY: sTexture = "hud/leaderboard_class_heavy.vtf"; break;
			case TF_CLASS_ENGINEER: sTexture = "hud/leaderboard_class_engineer.vtf"; break;
			case TF_CLASS_MEDIC: sTexture = "hud/leaderboard_class_medic.vtf"; break;
			case TF_CLASS_SNIPER: sTexture = "hud/leaderboard_class_sniper.vtf"; break;
			case TF_CLASS_SPY: sTexture = "hud/leaderboard_class_spy.vtf"; break;
			}
			int iSize = H::Draw.Scale(18, Scale_Round);
			H::Draw.Texture(sTexture, m, t - tOffset, iSize, iSize, ALIGN_BOTTOM);
		}

		if (tCache.m_pWeaponIcon)
		{
			float flW = tCache.m_pWeaponIcon->Width(), flH = tCache.m_pWeaponIcon->Height();
			float flScale = H::Draw.Scale(std::min((w + 40) / 2.f, 80.f) / std::max(flW, flH * 2));
			H::Draw.DrawHudTexture(m - flW / 2.f * flScale, b + bOffset, flScale, tCache.m_pWeaponIcon, Vars::Colors::ElementsColor.Value);
		}
	}
	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

void CESP::DrawBuildings()
{
	if (m_mBuildingCache.empty())
		return;

	const auto& fFont = H::Fonts.GetFont(FONT_ESP);
	const auto& fFont2 = H::Fonts.GetFont(FONT_FLAGS);
	const auto& fFont3 = H::Fonts.GetFont(FONT_TAGS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(2);
	for (auto& [pEntity, tCache] : m_mBuildingCache)
	{
		float x, y, w, h;
		if (!GetDrawBounds(pEntity, x, y, w, h))
			continue;

		int l = x - H::Draw.Scale(6), r = x + w + H::Draw.Scale(6), m = x + w / 2;
		int t = y - H::Draw.Scale(5), b = y + h + H::Draw.Scale(5);
		int lOffset = 0, rOffset = 0, bOffset = 0, tOffset = 0;
		I::MatSystemSurface->DrawSetAlphaMultiplier(tCache.m_flAlpha);

		if (tCache.m_bBox)
			H::Draw.LineRectOutline(x, y, w, h, tCache.m_tColor, { 0, 0, 0, 255 });
		for (auto& [iMode, flPercent, tColor, tOverfill, bAdjust] : tCache.m_vBars)
		{
			auto drawBar = [&](int x, int y, int w, int h, EAlign eAlign = ALIGN_LEFT)
				{
					Color_t outline = { 0, 0, 0, 255 };

					int nw = w, nh = h;
					int fx = x, fy = y;
					float clampedPercent = std::min(flPercent, 1.f);

					switch (eAlign)
					{
					case ALIGN_LEFT:   nw = w * clampedPercent; break;
					case ALIGN_RIGHT:  nw = w * clampedPercent; fx += w - nw; break;
					case ALIGN_TOP:    nh = h * clampedPercent; break;
					case ALIGN_BOTTOM: nh = h * clampedPercent; fy += h - nh; break;
					}

					H::Draw.FillRectOutline(x, y, w, h, Vars::Colors::BarFill.Value, outline);
					H::Draw.FillRect(fx, fy, nw, nh, flPercent > 1.f ? tOverfill : tColor);
				};

			int iSpace = H::Draw.Scale(4);
			int iThickness = H::Draw.Scale(2, Scale_Round);
			switch (iMode)
			{
			case ALIGN_LEFT:
				drawBar(x - iSpace - iThickness - lOffset, y, iThickness, h, ALIGN_BOTTOM);
				lOffset += iSpace + iThickness;
				break;
			case ALIGN_BOTTOM:
				drawBar(x, y + h + iSpace + bOffset, w, iThickness);
				bOffset += iSpace + iThickness;
				break;
			}
		}

		for (auto& [iMode, sText, tColor, tOutline] : tCache.m_vText)
		{
			switch (iMode)
			{
			case ALIGN_TOP:
				H::Draw.StringOutlined(fFont, m, t - tOffset, tColor, tOutline, ALIGN_BOTTOM, sText.c_str());
				tOffset += nTall;
				break;

			case ALIGN_BOTTOM:
				H::Draw.StringOutlined(fFont, m, b + bOffset, tColor, tOutline, ALIGN_TOP, sText.c_str());
				bOffset += nTall;
				break;
			case ALIGN_LEFT:
			{
				float flDrawPercent = std::min(tCache.m_flSmoothHealth, 1.0f);
				float flRelativeY = y - H::Draw.Scale(2) + h - (h * flDrawPercent);
				H::Draw.StringOutlined(fFont2, l - lOffset, flRelativeY, tColor, tOutline, ALIGN_TOPRIGHT, sText.c_str());
			}
			break;
			case ALIGN_CENTER:
			{
				int iBarSpace = H::Draw.Scale(4);
				int iBarThickness = H::Draw.Scale(2, Scale_Round);
				int barX = x - iBarSpace - (iBarThickness / 2);

				float flDrawPercent = std::clamp(tCache.m_flSmoothHealth, 0.f, 1.f);
				float flRelativeY = y - H::Draw.Scale(2) + h - (h * flDrawPercent);
				H::Draw.StringOutlined(fFont2, barX, flRelativeY, tColor, tOutline, ALIGN_CENTER, sText.c_str());
			}
			break;
			case ALIGN_TOPRIGHT:
				H::Draw.StringOutlined(fFont2, r, y - H::Draw.Scale(2) + rOffset, tColor, tOutline, ALIGN_TOPLEFT, sText.c_str());
				rOffset += nTall;
				break;
			case ALIGN_BOTTOMRIGHT:
				H::Draw.StringOutlined(fFont2, r, y + h, tColor, tOutline, ALIGN_TOPLEFT, sText.c_str());
				break;
			}
		}
	}

	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

void CESP::DrawWorld()
{
	if (m_mEntityCache.empty())
		return;

	const auto& fFont = H::Fonts.GetFont(FONT_ESP);
	const auto& fFont2 = H::Fonts.GetFont(FONT_FLAGS);
	const auto& fFont3 = H::Fonts.GetFont(FONT_TAGS);
	const int nTall = fFont.m_nTall + H::Draw.Scale(2);
	for (auto& [pEntity, tCache] : m_mEntityCache)
	{
		float x, y, w, h;
		if (!GetDrawBounds(pEntity, x, y, w, h))
			continue;

		int l = x - H::Draw.Scale(6), r = x + w + H::Draw.Scale(6), m = x + w / 2;
		int t = y - H::Draw.Scale(5), b = y + h + H::Draw.Scale(5);
		int lOffset = 0, rOffset = 0, bOffset = 0, tOffset = 0;
		I::MatSystemSurface->DrawSetAlphaMultiplier(tCache.m_flAlpha);

		if (tCache.m_bBox)
			H::Draw.LineRectOutline(x, y, w, h, tCache.m_tColor, { 0, 0, 0, 255 });


		for (auto& [iMode, sText, tColor, tOutline] : tCache.m_vText)
		{
			switch (iMode)
			{
			case ALIGN_TOP:
				H::Draw.StringOutlined(fFont, m, t - tOffset, tColor, tOutline, ALIGN_BOTTOM, sText.c_str());
				tOffset += nTall;
				break;
			case ALIGN_BOTTOM:
				H::Draw.StringOutlined(fFont2, m, b + bOffset, tColor, tOutline, ALIGN_TOP, sText.c_str());
				bOffset += nTall;
				break;
			case ALIGN_TOPRIGHT:
				H::Draw.StringOutlined(fFont2, r, y - H::Draw.Scale(2) + rOffset, tColor, tOutline, ALIGN_TOPLEFT, sText.c_str());
				rOffset += nTall;
				break;
			}
		}
	}

	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

bool CESP::GetDrawBounds(CBaseEntity* pEntity, float& x, float& y, float& w, float& h)
{
	Vec3 vOrigin = pEntity->GetAbsOrigin();
	matrix3x4 mTransform = { { 1, 0, 0, vOrigin.x }, { 0, 1, 0, vOrigin.y }, { 0, 0, 1, vOrigin.z } };
	//if (pEntity->entindex() == I::EngineClient->GetLocalPlayer())
	Math::AngleMatrix({ 0.f, I::EngineClient->GetViewAngles().y, 0.f }, mTransform, false);

	float flLeft, flRight, flTop, flBottom;
	if (!SDK::IsOnScreen(pEntity, mTransform, &flLeft, &flRight, &flTop, &flBottom, true))
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

void CESP::DrawBones(CTFPlayer* pPlayer, matrix3x4* aBones, std::vector<int> vecBones, Color_t clr)
{
	for (size_t n = 1; n < vecBones.size(); n++)
	{
		auto vBone1 = pPlayer->GetHitboxCenter(aBones, vecBones[n]);
		auto vBone2 = pPlayer->GetHitboxCenter(aBones, vecBones[n - 1]);

		Vec3 vScreen1, vScreen2;
		if (SDK::W2S(vBone1, vScreen1) && SDK::W2S(vBone2, vScreen2))
			H::Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, clr);
	}
}