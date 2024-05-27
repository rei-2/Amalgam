#include "ESP.h"

#include "../../Backtrack/Backtrack.h"
#include "../../Players/PlayerUtils.h"

MAKE_SIGNATURE(CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot, "client.dll", "48 89 6C 24 ? 56 41 54 41 55 41 56 41 57 48 83 EC", 0x0);
MAKE_SIGNATURE(CEconItemView_GetItemName, "client.dll", "40 53 48 83 EC ? 48 8B D9 C6 81 ? ? ? ? ? E8 ? ? ? ? 48 8B 8B", 0x0);

void CESP::Run(CTFPlayer* pLocal)
{
	if (!Vars::ESP::Draw.Value)
		return;

	DrawWorld();
	DrawBuildings(pLocal);
	DrawPlayers(pLocal);
}

bool CESP::GetDrawBounds(CBaseEntity* pEntity, int& x, int& y, int& w, int& h)
{
	Vec3 vMins = pEntity->m_vecMins(), vMaxs = pEntity->m_vecMaxs();

	auto& transform = const_cast<matrix3x4&>(pEntity->RenderableToWorldTransform());
	if (pEntity && pEntity->entindex() == I::EngineClient->GetLocalPlayer())
	{
		Vec3 vAngles = I::EngineClient->GetViewAngles();
		vAngles.x = vAngles.z = 0.f;
		Math::AngleMatrix(vAngles, transform);
		Math::MatrixSetColumn(pEntity->GetAbsOrigin(), 3, transform);
	}

	float flLeft, flRight, flTop, flBottom;
	if (!SDK::IsOnScreen(pEntity, transform, &flLeft, &flRight, &flTop, &flBottom))
		return false;

	x = flLeft + (flRight - flLeft) / 8.f;
	y = flBottom;
	w = flRight - flLeft - (flRight - flLeft) / 8.f * 2.f;
	h = flTop - flBottom;

	return !(x > H::Draw.m_nScreenW || x + w < 0 || y > H::Draw.m_nScreenH || y + h < 0);
}

void CESP::DrawPlayers(CTFPlayer* pLocal)
{
	if (!Vars::ESP::Player.Value)
		return;

	auto pResource = H::Entities.GetPR();
	if (!pResource)
		return;

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();

		const int nIndex = pPlayer->entindex();
		if (!pPlayer->IsAlive() || pPlayer->IsAGhost())
			continue;

		if (pPlayer->IsDormant())
		{
			if (!Vars::ESP::DormantAlpha.Value || Vars::ESP::DormantPriority.Value && F::PlayerUtils.GetPriority(nIndex) <= F::PlayerUtils.mTags["Default"].Priority)
				continue;
		}

		if (nIndex != I::EngineClient->GetLocalPlayer())
		{
			if (!(Vars::ESP::Draw.Value & 1 << 3 && H::Entities.IsFriend(nIndex))
				&& !(Vars::ESP::Draw.Value & 1 << 4 && F::PlayerUtils.GetPriority(nIndex) > F::PlayerUtils.mTags["Default"].Priority))
			{
				if (!(Vars::ESP::Draw.Value & 1 << 0) && pPlayer->m_iTeamNum() != pLocal->m_iTeamNum())
					continue;
				if (!(Vars::ESP::Draw.Value & 1 << 1) && pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
					continue;
			}
		}
		else if (!(Vars::ESP::Draw.Value & 1 << 2) || !I::Input->CAM_IsThirdPerson())
			continue;

		I::MatSystemSurface->DrawSetAlphaMultiplier((pPlayer->IsDormant() ? Vars::ESP::DormantAlpha.Value : Vars::ESP::ActiveAlpha.Value) / 255.f);

		int x = 0, y = 0, w = 0, h = 0;
		if (GetDrawBounds(pPlayer, x, y, w, h))
		{
			int lOffset = 0, rOffset = 0, bOffset = 2, tOffset = 0;
			const auto& fFont = H::Fonts.GetFont(FONT_ESP);

			const Color_t drawColor = H::Color.GetEntityDrawColor(pLocal, pPlayer, Vars::Colors::Relative.Value); //GetTeamColor(pPlayer->m_iTeamNum(), Vars::Colors::Relative.Value);
			const int iMaxHealth = pPlayer->m_iMaxHealth(), iHealth = pPlayer->IsDormant() ? pResource->GetHealth(pPlayer->entindex()) : pPlayer->m_iHealth(), iClassNum = pPlayer->m_iClass();

			// Bones
			if (Vars::ESP::Player.Value & 1 << 11)
			{
				DrawBones(pPlayer, { 8, 7, 6, 4 }, drawColor);
				DrawBones(pPlayer, { 11, 10, 9, 4 }, drawColor);
				DrawBones(pPlayer, { 0, 4, 1 }, drawColor);
				DrawBones(pPlayer, { 14, 13, 1 }, drawColor);
				DrawBones(pPlayer, { 17, 16, 1 }, drawColor);
			}

			// Box
			if (Vars::ESP::Player.Value & 1 << 10)
				H::Draw.LineRectOutline(x, y, w, h, drawColor, { 0, 0, 0, 255 });

			// Health bar
			if (Vars::ESP::Player.Value & 1 << 1)
			{
				float flRatio = std::clamp(float(iHealth) / iMaxHealth, 0.f, 1.f);
				Color_t cColor = Vars::Colors::HealthBar.Value.StartColor.Lerp(Vars::Colors::HealthBar.Value.EndColor, flRatio);
				H::Draw.FillRectPercent(x - 6, y, 2, h, flRatio, cColor, { 0, 0, 0, 255 }, ALIGN_BOTTOM, true);

				if (iHealth > iMaxHealth)
				{
					const float flMaxOverheal = floorf(iMaxHealth / 10.f) * 5;
					flRatio = std::clamp((iHealth - iMaxHealth) / flMaxOverheal, 0.f, 1.f);
					cColor = Vars::Colors::Overheal.Value;
					H::Draw.FillRectPercent(x - 6, y, 2, h, flRatio, cColor, { 0, 0, 0, 0 }, ALIGN_BOTTOM, true);
				}

				lOffset += 5;
			}

			// Health text
			if (Vars::ESP::Player.Value & 1 << 2)
				H::Draw.String(fFont, x - 5 - lOffset, y + h - h * (float(std::min(iHealth, iMaxHealth)) / iMaxHealth) - 2, iHealth > iMaxHealth ? Vars::Colors::Overheal.Value : Vars::Menu::Theme::Active.Value, ALIGN_TOPRIGHT, "%d", iHealth);

			// Ubercharge bar/text
			if (iClassNum == TF_CLASS_MEDIC)
			{
				if (auto pMedGun = pPlayer->GetWeaponFromSlot(SLOT_SECONDARY))
				{
					if (Vars::ESP::Player.Value & 1 << 3)
					{
						const float flMaxUber = (pMedGun->m_iItemDefinitionIndex() == Medic_s_TheVaccinator ? 400.f : 100.f);
						const float flUber = std::min(pMedGun->As<CWeaponMedigun>()->m_flChargeLevel() * flMaxUber, flMaxUber);
						float ratio = flUber / flMaxUber;

						H::Draw.FillRectPercent(x, y + h + 4, w, 2, ratio, Vars::Colors::UberBar.Value);

						bOffset += 5;
					}

					if (Vars::ESP::Player.Value & 1 << 4)
						H::Draw.String(fFont, x + w + 4, y + h, Vars::Menu::Theme::Active.Value, ALIGN_TOPLEFT, L"%.f%%", std::clamp(pMedGun->As<CWeaponMedigun>()->m_flChargeLevel() * 100.f, 0.f, 100.f));
				}
			}

			// Name + priority text
			PlayerInfo_t pi{};
			if (I::EngineClient->GetPlayerInfo(nIndex, &pi))
			{
				const int middle = x + w / 2;
				if (Vars::ESP::Player.Value & 1 << 0)
				{
					tOffset += fFont.m_nTall + 2;
					H::Draw.String(fFont, middle, y - tOffset, Vars::Menu::Theme::Active.Value, ALIGN_TOP, SDK::ConvertUtf8ToWide(pi.name).data());
				}

				if (Vars::ESP::Player.Value & 1 << 12)
				{
					std::string sTag; PriorityLabel_t plTag;
					if (F::PlayerUtils.GetSignificantTag(pi.friendsID, &sTag, &plTag, 1))
					{
						tOffset += fFont.m_nTall + 2;
						H::Draw.String(fFont, middle, y - tOffset, plTag.Color, ALIGN_TOP, sTag.c_str());
					}
				}

				if (Vars::ESP::Player.Value & 1 << 13)
				{
					std::vector<std::pair<std::string, PriorityLabel_t>> vLabels = {};
					for (const auto& sTag : F::PlayerUtils.mPlayerTags[pi.friendsID])
					{
						PriorityLabel_t plTag;
						if (F::PlayerUtils.GetTag(sTag, &plTag) && plTag.Label)
							vLabels.push_back({ sTag, plTag });
					}
					if (H::Entities.IsFriend(nIndex))
					{
						const auto& plTag = F::PlayerUtils.mTags["Friend"];
						if (plTag.Label)
							vLabels.push_back({ "Friend", plTag });
					}

					if (vLabels.size())
					{
						std::sort(vLabels.begin(), vLabels.end(), [&](const auto& a, const auto& b) -> bool
							{
								// sort by priority if unequal
								if (a.second.Priority != b.second.Priority)
									return a.second.Priority > b.second.Priority;

								return a.first < b.first;
							});

						for (const auto& pLabel : vLabels)
						{
							H::Draw.String(fFont, x + w + 4, y + rOffset, pLabel.second.Color, ALIGN_TOPLEFT, pLabel.first.c_str());
							rOffset += fFont.m_nTall;
						}
					}
				}
			}

			// Class icon
			if (Vars::ESP::Player.Value & 1 << 5)
			{
				const int size = 18 * Vars::Menu::DPI.Value;
				H::Draw.Texture(x + w / 2, y - tOffset, size, size, iClassNum - 1, ALIGN_BOTTOM);
			}

			// Class text
			if (Vars::ESP::Player.Value & 1 << 6)
			{
				H::Draw.String(fFont, x + w + 4, y + rOffset, Vars::Menu::Theme::Active.Value, ALIGN_TOPLEFT, L"%ls", GetPlayerClass(iClassNum));
				rOffset += fFont.m_nTall;
			}

			// Distance
			if (Vars::ESP::Player.Value & 1 << 9)
			{
				if (pPlayer != pLocal)
				{
					const Vec3 vDelta = pPlayer->GetAbsOrigin() - pLocal->GetAbsOrigin();
					const int iDistance = round(vDelta.Length2D() / 41.f);
					
					H::Draw.String(fFont, x + w / 2, y + h + bOffset, Vars::Menu::Theme::Active.Value, ALIGN_TOP, L"%dM", iDistance);
					bOffset += fFont.m_nTall;
				}
			}

			auto pWeapon = pPlayer->m_hActiveWeapon().Get()->As<CTFWeaponBase>();
			if (pWeapon)
			{
				// Weapon text
				if (Vars::ESP::Player.Value & 1 << 8)
				{
					static auto getEconItemViewByLoadoutSlot = reinterpret_cast<void* (__cdecl*)(void*, int, void**)>(S::CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot());
					static auto getItemName = reinterpret_cast<const char* (__thiscall*)(void*)>(S::CEconItemView_GetItemName());

					int iWeaponSlot = pWeapon->m_iSlot();
					int iPlayerClass = pPlayer->m_iClass();

					const char* szItemName = "";

					switch (iPlayerClass)
					{
					case TF_CLASS_SPY:
					{
						switch (iWeaponSlot)
						{
						// Primary (gun)
						case 0: iWeaponSlot = 1; break;
						// Sapper ????
						case 1: iWeaponSlot = 4; break;
						// Knife
						//case 2:
						// Disguise kit
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

					void* pCurItemData = getEconItemViewByLoadoutSlot(pPlayer, iWeaponSlot, 0);
					if (pCurItemData)
					{
						szItemName = getItemName(pCurItemData);
						H::Draw.String(fFont, x + (w / 2), y + h + bOffset, Vars::Menu::Theme::Active.Value, ALIGN_TOP, "%ls", szItemName);
						bOffset += fFont.m_nTall;
					}
				}

				// Weapon icons
				if (Vars::ESP::Player.Value & 1 << 7)
				{
					if (CHudTexture* pIcon = pWeapon->GetWeaponIcon())
					{
						const float iw = pIcon->Width(), ih = pIcon->Height();
						const float scale = std::clamp(float(w) / std::max(iw, ih * 2), 0.25f, 0.75f) * Vars::Menu::DPI.Value;
						H::Draw.DrawHudTexture(x + float(w) / 2.f - iw / 2.f * scale, y + h + 1 + bOffset, scale, pIcon, Vars::Menu::Theme::Active.Value);
					}
				}
			}

			// Player conditions
			{
				// Lagcomp cond, idea from nitro
				if (Vars::ESP::Player.Value & 1 << 17 && !pPlayer->IsDormant() && pPlayer != pLocal)
				{
					if (F::Backtrack.mLagCompensation[pPlayer])
					{
						H::Draw.String(fFont, x + w + 4, y + rOffset, { 255, 95, 95, 255 }, ALIGN_TOPLEFT, "LAGCOMP");
						rOffset += fFont.m_nTall;
					}
				}

				// Ping warning, idea from nitro
				if (Vars::ESP::Player.Value & 1 << 18 && pPlayer != pLocal)
				{
					auto pNetChan = I::EngineClient->GetNetChannelInfo();
					if (pNetChan && !pNetChan->IsLoopback())
					{
						int iPing = pResource->GetPing(pPlayer->entindex());
						if (iPing && (iPing >= 200 || iPing <= 5))
						{
							H::Draw.String(fFont, x + w + 4, y + rOffset, { 255, 95, 95, 255 }, ALIGN_TOPLEFT, "%dMS", iPing);
							rOffset += fFont.m_nTall;
						}
					}
				}

				// Idea from rijin
				if (Vars::ESP::Player.Value & 1 << 19 && pPlayer != pLocal)
				{
					const int iKills = pResource->GetKills(pPlayer->entindex());
					const int iDeaths = pResource->GetDeaths(pPlayer->entindex());
					if (iKills >= 20)
					{
						const int iKDR = iKills / std::max(iDeaths, 1);
						if (iKDR >= 10)
						{
							H::Draw.String(fFont, x + w + 4, y + rOffset, { 255, 95, 95, 255 }, ALIGN_TOPLEFT, "HIGH K/D [%d/%d]", iKills, iDeaths);
							rOffset += fFont.m_nTall;
						}
					}
				}

				{
					auto drawCond = [&rOffset, &fFont](const char* text, Color_t color, int x, int y)
					{
						H::Draw.String(fFont, x, y + rOffset, color, ALIGN_TOPLEFT, text);
						rOffset += fFont.m_nTall;
					};

					// Buffs
					if (Vars::ESP::Player.Value & 1 << 14)
					{
						if (pPlayer->IsCritBoosted())
							drawCond("CRITS", { 255, 107, 108, 255 }, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_ENERGY_BUFF) ||
							pPlayer->InCond(TF_COND_NOHEALINGDAMAGEBUFF))
							drawCond("MINI-CRITS", { 254, 202, 87, 255 }, x + w + 4, y);

						if (pPlayer->m_iHealth() > pPlayer->m_iMaxHealth())
							drawCond("HP", Vars::Colors::Overheal.Value, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_HEALTH_BUFF) || pPlayer->InCond(TF_COND_MEGAHEAL) || pPlayer->IsBuffedByKing())
							drawCond("HP+", Vars::Colors::Overheal.Value, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_INVULNERABLE) ||
							pPlayer->InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED) ||
							pPlayer->InCond(TF_COND_INVULNERABLE_USER_BUFF) ||
							pPlayer->InCond(TF_COND_INVULNERABLE_CARD_EFFECT))
							drawCond("UBER", Vars::Colors::UberBar.Value, x + w + 4, y);
						else if (pPlayer->InCond(TF_COND_PHASE))
							drawCond("BONK", { 254, 202, 87, 255 }, x + w + 4, y);

						/* vaccinator effects */
						if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST) || pPlayer->InCond(TF_COND_BULLET_IMMUNE))
							drawCond("BULLET+", { 255, 107, 108, 255 }, x + w + 4, y);
						else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_BULLET_RESIST))
							drawCond("BULLET", { 255, 107, 108, 255 }, x + w + 4, y);
						if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST) || pPlayer->InCond(TF_COND_BLAST_IMMUNE))
							drawCond("BLAST+", { 255, 107, 108, 255 }, x + w + 4, y);
						else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_BLAST_RESIST))
							drawCond("BLAST", { 255, 107, 108, 255 }, x + w + 4, y);
						if (pPlayer->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST) || pPlayer->InCond(TF_COND_FIRE_IMMUNE))
							drawCond("FIRE+", { 255, 107, 108, 255 }, x + w + 4, y);
						else if (pPlayer->InCond(TF_COND_MEDIGUN_SMALL_FIRE_RESIST))
							drawCond("FIRE", { 255, 107, 108, 255 }, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_OFFENSEBUFF))
							drawCond("BANNER", drawColor, x + w + 4, y);
						if (pPlayer->InCond(TF_COND_DEFENSEBUFF))
							drawCond("BATTALIONS", drawColor, x + w + 4, y);
						if (pPlayer->InCond(TF_COND_REGENONDAMAGEBUFF))
							drawCond("CONCH", drawColor, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_BLASTJUMPING))
							drawCond("BLASTJUMP", { 254, 202, 87, 255 }, x + w + 4, y);
					}
					
					// Debuffs
					if (Vars::ESP::Player.Value & 1 << 15)
					{
						if (pPlayer->InCond(TF_COND_URINE))
							drawCond("JARATE", { 254, 202, 87, 255 }, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_MARKEDFORDEATH) || pPlayer->InCond(TF_COND_MARKEDFORDEATH_SILENT))
							drawCond("MARKED", { 254, 202, 87, 255 }, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_BURNING))
							drawCond("BURN", { 254, 202, 87, 255 }, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_MAD_MILK))
							drawCond("MILK", { 254, 202, 87, 255 }, x + w + 4, y);
					}
					
					// Misc
					if (Vars::ESP::Player.Value & 1 << 16)
					{
						if (Vars::Visuals::Removals::Taunts.Value && pPlayer->InCond(TF_COND_TAUNTING)) // i dont really see a need for this condition unless you have this enabled
							drawCond("TAUNT", { 255, 100, 200, 255 }, x + w + 4, y);

						if (pPlayer->m_bFeignDeathReady())
							drawCond("DR", { 254, 202, 87, 255 }, x + w + 4, y);
				
						if (pPlayer->InCond(TF_COND_AIMING))
						{
							if (pWeapon)
							{
								if (pWeapon->m_iWeaponID() == TF_WEAPON_MINIGUN)
									drawCond("REV", { 128, 128, 128, 255 }, x + w + 4, y);

								if (pWeapon->m_iWeaponID() == TF_WEAPON_COMPOUND_BOW)
								{
									if ((I::GlobalVars->curtime - pWeapon->As<CTFPipebombLauncher>()->m_flChargeBeginTime()) >= 1.f)
										drawCond("CHARGED", { 254, 202, 87, 255 }, x + w + 4, y);
									else
										drawCond("CHARGING", { 254, 202, 87, 255 }, x + w + 4, y);
								}

								if (pWeapon->m_iWeaponID() == TF_WEAPON_PARTICLE_CANNON)
									drawCond("CHARGING", { 254, 202, 87, 255 }, x + w + 4, y);
							}
						}

						if (pPlayer->InCond(TF_COND_ZOOMED))
							drawCond("ZOOM", { 254, 202, 87, 255 }, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_STEALTHED) || pPlayer->InCond(TF_COND_STEALTHED_BLINK) || pPlayer->InCond(TF_COND_STEALTHED_USER_BUFF) || pPlayer->InCond(TF_COND_STEALTHED_USER_BUFF_FADING))
							drawCond(std::format("CLOAK {:.0f}%%", pPlayer->GetInvisPercentage()).c_str(), Vars::Colors::Cloak.Value, x + w + 4, y);

						if (pPlayer->InCond(TF_COND_DISGUISING) || pPlayer->InCond(TF_COND_DISGUISE_WEARINGOFF) || pPlayer->InCond(TF_COND_DISGUISED))
							drawCond("DISGUISE", { 254, 202, 87, 255 }, x + w + 4, y);
					}																				
				}
			}
		}
	}

	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

void CESP::DrawBuildings(CTFPlayer* pLocal)
{
	if (!Vars::ESP::Building.Value)
		return;

	I::MatSystemSurface->DrawSetAlphaMultiplier(Vars::ESP::ActiveAlpha.Value);

	for (auto pEntity : H::Entities.GetGroup(EGroupType::BUILDINGS_ALL))
	{
		auto pBuilding = pEntity->As<CBaseObject>();

		auto pOwner = pBuilding->m_hBuilder().Get();
		if (pOwner)
		{
			int nIndex = pOwner->entindex();
			if (nIndex != I::EngineClient->GetLocalPlayer())
			{
				if (!(Vars::ESP::Draw.Value & 1 << 3 && H::Entities.IsFriend(nIndex))
					&& !(Vars::ESP::Draw.Value & 1 << 4 && F::PlayerUtils.GetPriority(nIndex) > F::PlayerUtils.mTags["Default"].Priority))
				{
					if (!(Vars::ESP::Draw.Value & 1 << 0) && pOwner->m_iTeamNum() != pLocal->m_iTeamNum())
						continue;
					if (!(Vars::ESP::Draw.Value & 1 << 1) && pOwner->m_iTeamNum() == pLocal->m_iTeamNum())
						continue;
				}
			}
			else if (!(Vars::ESP::Draw.Value & 1 << 2))
				continue;
		}
		else
		{
			if (!(Vars::ESP::Draw.Value & 1 << 0) && pBuilding->m_iTeamNum() != pLocal->m_iTeamNum())
				continue;
			if (!(Vars::ESP::Draw.Value & 1 << 1) && pBuilding->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;
		}

		int x = 0, y = 0, w = 0, h = 0;
		if (GetDrawBounds(pBuilding, x, y, w, h))
		{
			int lOffset = 0, rOffset = 0, /*bOffset = 0, */tOffset = 0;
			const auto& fFont = H::Fonts.GetFont(FONT_ESP);

			const Color_t drawColor = H::Color.GetEntityDrawColor(pLocal, pOwner ? pOwner : pBuilding, Vars::Colors::Relative.Value); //GetTeamColor(pBuilding->m_iTeamNum(), Vars::ESP::Main::Relative.Value);
			const int iMaxHealth = pBuilding->m_iMaxHealth(), iHealth = std::min(pBuilding->m_iHealth(), iMaxHealth);

			const bool bIsMini = pBuilding->m_bMiniBuilding();

			// Box
			if (Vars::ESP::Building.Value & 1 << 4)
				H::Draw.LineRectOutline(x, y, w, h, drawColor, { 0, 0, 0, 255 });

			// Health bar
			if (Vars::ESP::Building.Value & 1 << 1)
			{
				const float flRatio = float(iHealth) / iMaxHealth;
				Color_t cColor = Vars::Colors::HealthBar.Value.StartColor.Lerp(Vars::Colors::HealthBar.Value.EndColor, flRatio);
				H::Draw.FillRectPercent(x - 6, y, 2, h, flRatio, cColor, { 0, 0, 0, 255 }, ALIGN_BOTTOM, true);

				lOffset += 5;
			}

			// Health text
			if (Vars::ESP::Building.Value & 1 << 2)
				H::Draw.String(fFont, x - 5 - lOffset, y + h - h * (float(iHealth) / iMaxHealth) - 2, Vars::Menu::Theme::Active.Value, ALIGN_TOPRIGHT, "%d", iHealth);

			// Name
			if (Vars::ESP::Building.Value & 1 << 0)
			{
				const wchar_t* szName;

				switch (pBuilding->GetClassID())
				{
				case ETFClassID::CObjectSentrygun:
					szName = bIsMini ? L"Mini-Sentry" : L"Sentry";
					break;
				case ETFClassID::CObjectDispenser:
					szName = L"Dispenser";
					break;
				case ETFClassID::CObjectTeleporter:
					szName = pBuilding->m_iObjectMode() ? L"Teleporter Exit" : L"Teleporter Entrance";
					break;
				default: szName = L"Unknown"; break;
				}

				tOffset += fFont.m_nTall + 2;
				H::Draw.String(fFont, x + w / 2, y - tOffset, Vars::Menu::Theme::Active.Value, ALIGN_TOP, szName);
			}

			// Distance
			if (Vars::ESP::Building.Value & 1 << 3)
			{
				const Vec3 vDelta = pBuilding->GetAbsOrigin() - pLocal->GetAbsOrigin();
				const int iDistance = round(vDelta.Length2D() / 41.f);

				H::Draw.String(fFont, x + (w / 2), y + h, Vars::Menu::Theme::Active.Value, ALIGN_TOP, L"%dM", iDistance);
			}

			// Building owner
			if (Vars::ESP::Building.Value & 1 << 5 && !pBuilding->m_bWasMapPlaced() && pOwner)
			{
				PlayerInfo_t pi{};
				if (I::EngineClient->GetPlayerInfo(pOwner->entindex(), &pi))
				{
					tOffset += fFont.m_nTall + 2;
					H::Draw.String(fFont, x + w / 2, y - tOffset, { 254, 202, 87, 255 }, ALIGN_TOP, L"%ls", SDK::ConvertUtf8ToWide(pi.name).data());
				}
			}

			// Building level
			if (Vars::ESP::Building.Value & 1 << 6 && !bIsMini)
			{
				H::Draw.String(fFont, x + w + 4, y + rOffset, { 254, 202, 87, 255 }, ALIGN_TOPLEFT, L"%d/%d", pBuilding->m_iUpgradeLevel(), pBuilding->m_iHighestUpgradeLevel());
				rOffset += fFont.m_nTall;
			}

			// Building conditions
			if (Vars::ESP::Building.Value & 1 << 7)
			{
				std::vector<std::wstring> condStrings{};

				const float flConstructed = pBuilding->m_flPercentageConstructed() * 100.f;
				if (flConstructed < 100.f && static_cast<int>(flConstructed) != 0)
				{
					H::Draw.String(fFont, x + w + 4, y + rOffset, { 254, 202, 87, 255 }, ALIGN_TOPLEFT, L"BUILDING: %0.f%%", flConstructed);
					rOffset += fFont.m_nTall;
				}

				if (pBuilding->IsSentrygun() && pBuilding->As<CObjectSentrygun>()->m_bPlayerControlled())
					condStrings.emplace_back(L"WRANGLED");

				if (pBuilding->m_bHasSapper())
					condStrings.emplace_back(L"SAPPED");
				else if (pBuilding->m_bDisabled())
					condStrings.emplace_back(L"DISABLED");

				if (pBuilding->IsSentrygun() && !pBuilding->m_bBuilding())
				{
					int iShells;
					int iMaxShells;
					int iRockets;
					int iMaxRockets;

					pBuilding->As<CObjectSentrygun>()->GetAmmoCount(iShells, iMaxShells, iRockets, iMaxRockets);

					if (iShells == 0)
						condStrings.emplace_back(L"NO AMMO");
					if (!bIsMini && iRockets == 0)
						condStrings.emplace_back(L"NO ROCKETS");
				}

				if (!condStrings.empty())
				{
					for (auto& condString : condStrings)
					{
						H::Draw.String(fFont, x + y + 2, y + rOffset, { 254, 202, 87, 255 }, ALIGN_TOPLEFT, condString.data());
						rOffset += fFont.m_nTall;
					}
				}
			}
		}
	}

	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

void CESP::DrawWorld()
{
	Vec3 vScreen = {};
	const auto& fFont = H::Fonts.GetFont(FONT_ESP);
	const int nTextTopOffset = fFont.m_nTall * (5 / 4);

	I::MatSystemSurface->DrawSetAlphaMultiplier(Vars::ESP::ActiveAlpha.Value);

	if (Vars::ESP::Draw.Value & 1 << 5)
	{
		for (auto NPC : H::Entities.GetGroup(EGroupType::WORLD_NPC))
		{
			int x = 0, y = 0, w = 0, h = 0;
			if (GetDrawBounds(NPC, x, y, w, h))
			{
				const wchar_t* szName;
				switch (NPC->GetClassID())
				{
				case ETFClassID::CHeadlessHatman:
					szName = L"Horseless Headless Horsemann"; break;
				case ETFClassID::CTFTankBoss:
					szName = L"Tank"; break;
				case ETFClassID::CMerasmus:
					szName = L"Merasmus"; break;
				case ETFClassID::CZombie:
					szName = L"Skeleton"; break;
				case ETFClassID::CEyeballBoss:
					szName = L"Monoculus"; break;
				default:
					szName = L"Unknown"; break;
				}

				H::Draw.String(fFont, x + w / 2, y - nTextTopOffset, Vars::Colors::NPC.Value, ALIGN_TOP, szName);
			}
		}
	}

	if (Vars::ESP::Draw.Value & 1 << 6)
	{
		for (auto pHealth : H::Entities.GetGroup(EGroupType::WORLD_HEALTH))
		{
			int x = 0, y = 0, w = 0, h = 0;
			if (GetDrawBounds(pHealth, x, y, w, h))
				H::Draw.String(fFont, x + w / 2, y - nTextTopOffset, Vars::Colors::Health.Value, ALIGN_CENTER, L"Health");
		}
	}

	if (Vars::ESP::Draw.Value & 1 << 7)
	{
		for (auto pAmmo : H::Entities.GetGroup(EGroupType::WORLD_AMMO))
		{
			int x = 0, y = 0, w = 0, h = 0;
			if (GetDrawBounds(pAmmo, x, y, w, h))
				H::Draw.String(fFont, x + w / 2, y - nTextTopOffset, Vars::Colors::Ammo.Value, ALIGN_CENTER, L"Ammo");
		}
	}

	if (Vars::ESP::Draw.Value & 1 << 8)
	{
		for (auto pCash : H::Entities.GetGroup(EGroupType::WORLD_MONEY))
		{
			int x = 0, y = 0, w = 0, h = 0;
			if (GetDrawBounds(pCash, x, y, w, h))
			{
				H::Draw.String(fFont, x + w / 2, y - nTextTopOffset, Vars::Colors::Money.Value, ALIGN_TOP, L"Money");
			}
		}
	}

	if (Vars::ESP::Draw.Value & 1 << 9)
	{
		for (auto pBomb : H::Entities.GetGroup(EGroupType::WORLD_BOMBS))
		{
			int x = 0, y = 0, w = 0, h = 0;
			if (GetDrawBounds(pBomb, x, y, w, h))
			{
				const wchar_t* szName;
				switch (pBomb->GetClassID())
				{
				case ETFClassID::CTFPumpkinBomb:
					szName = L"Pumpkin Bomb"; break;
				case ETFClassID::CTFGenericBomb:
					szName = L"Bomb"; break;
				default:
					szName = L"Unknown"; break;
				}

				H::Draw.String(fFont, x + w / 2, y - nTextTopOffset, Vars::Colors::Bomb.Value, ALIGN_TOP, szName);
			}
		}
	}

	if (Vars::ESP::Draw.Value & 1 << 10)
	{
		for (auto pBook : H::Entities.GetGroup(EGroupType::WORLD_SPELLBOOK))
		{
			int x = 0, y = 0, w = 0, h = 0;
			if (GetDrawBounds(pBook, x, y, w, h))
			{
				H::Draw.String(fFont, x + w / 2, y - nTextTopOffset, Vars::Colors::Halloween.Value, ALIGN_TOP, L"Spellbook");
			}
		}
	}

	if (Vars::ESP::Draw.Value & 1 << 11)
	{
		for (auto pGargy : H::Entities.GetGroup(EGroupType::WORLD_GARGOYLE))
		{
			int x = 0, y = 0, w = 0, h = 0;
			if (GetDrawBounds(pGargy, x, y, w, h))
			{
				H::Draw.String(fFont, x + w / 2, y - nTextTopOffset, Vars::Colors::Halloween.Value, ALIGN_TOP, L"Gargoyle");
			}
		}
	}

	I::MatSystemSurface->DrawSetAlphaMultiplier(1.f);
}

const wchar_t* CESP::GetPlayerClass(int iClassNum)
{
	static const wchar_t* szClasses[] = {
		L"Unknown", L"Scout", L"Sniper", L"Soldier", L"Demoman",
		L"Medic", L"Heavy", L"Pyro", L"Spy", L"Engineer"
	};

	return iClassNum < 10 && iClassNum > 0 ? szClasses[iClassNum] : szClasses[0];
}

void CESP::DrawBones(CTFPlayer* pPlayer, std::vector<int> vecBones, Color_t clr)
{
	const size_t nMax = vecBones.size(), nLast = nMax - 1;
	for (size_t n = 0; n < nMax; n++)
	{
		if (n == nLast)
			continue;

		const auto vBone = pPlayer->GetHitboxPos(vecBones[n]);
		const auto vParent = pPlayer->GetHitboxPos(vecBones[n + 1]);

		Vec3 vScreenBone, vScreenParent;
		if (SDK::W2S(vBone, vScreenBone) && SDK::W2S(vParent, vScreenParent))
			H::Draw.Line(vScreenBone.x, vScreenBone.y, vScreenParent.x, vScreenParent.y, clr);
	}
}