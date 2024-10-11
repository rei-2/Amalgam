#include "SpectatorList.h"

#include "../../Players/PlayerUtils.h"

bool CSpectatorList::GetSpectators(CTFPlayer* pLocal)
{
	m_vSpectators.clear();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_TEAMMATES))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();

		if (!pPlayer->IsAlive() && pPlayer->m_hObserverTarget().Get() == pLocal)
		{
			int iIndex = pPlayer->entindex();

			std::string sMode;
			switch (pPlayer->m_iObserverMode())
			{
				case OBS_MODE_FIRSTPERSON:
					sMode = "1st";
					break;
				case OBS_MODE_THIRDPERSON:
					sMode = "3rd";
					break;
				default: 
					continue;
			}

			int respawnIn = 0; float respawnTime = 0;
			if (auto pResource = H::Entities.GetPR())
			{
				respawnTime = pResource->GetNextRespawnTime(iIndex);
				respawnIn = std::max(respawnTime - I::GlobalVars->curtime, 0.f);
			}
			bool respawnTimeIncreased = false; // theoretically the respawn times could be changed by the map but oh well
			if (!m_mRespawnCache.contains(iIndex))
				m_mRespawnCache[iIndex] = respawnTime;
			if (m_mRespawnCache[iIndex] + 0.9f < respawnTime)
			{
				respawnTimeIncreased = true;
				m_mRespawnCache[iIndex] = -1.f;
			}

			PlayerInfo_t pi{};
			if (I::EngineClient->GetPlayerInfo(iIndex, &pi))
			{
				std::string sName = F::PlayerUtils.GetPlayerName(iIndex, pi.name);
				m_vSpectators.push_back({ sName, sMode, respawnIn, respawnTimeIncreased, H::Entities.IsFriend(pPlayer->entindex()), pPlayer->m_iTeamNum(), pPlayer->entindex() });
			}
		}
		else
		{
			auto iter = m_mRespawnCache.find(pPlayer->entindex());
			if (iter != m_mRespawnCache.end())
				m_mRespawnCache.erase(iter);
		}
	}

	return !m_vSpectators.empty();
}

void CSpectatorList::Run(CTFPlayer* pLocal)
{
	if (!(Vars::Menu::Indicators.Value & (1 << 2)))
	{
		m_mRespawnCache.clear();
		return;
	}

	if (!pLocal->IsAlive() || !GetSpectators(pLocal))
		return;

	int x = Vars::Menu::SpectatorsDisplay.Value.x;
	int iconOffset = 0;
	int y = Vars::Menu::SpectatorsDisplay.Value.y + 8;

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
	//	iconOffset = 36;
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= H::Draw.m_nScreenW - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}
	//else
	//	iconOffset = 16;

	//if (!Vars::Menu::SpectatorAvatars.Value)
	//	iconOffset = 0;

	const auto& fFont = H::Fonts.GetFont(FONT_INDICATORS);

	H::Draw.String(fFont, x, y, Vars::Menu::Theme::Accent.Value, align, "Spectating You:");
	for (auto& Spectator : m_vSpectators)
	{
		y += fFont.m_nTall + 3;

		/*
		if (Vars::Visuals::SpectatorAvatars.Value)
		{
			int w, h;

			I::MatSystemSurface->GetTextSize(fFont.m_dwFont, SDK::ConvertUtf8ToWide(std::format("{} - {} (respawn {}s)", Spectator.m_sName, Spectator.m_sMode, Spectator.m_iRespawnIn)).c_str(), w, h);
			switch (align)
			{
			case ALIGN_TOPLEFT: w = 0; break;
			case ALIGN_TOP: w /= 2; break;
			}

			PlayerInfo_t pi{};
			if (!I::EngineClient->GetPlayerInfo(Spectator.m_iIndex, &pi))
				continue;

			H::Draw.Avatar(x - w - (36 - iconOffset), y, 24, 24, pi.friendsID);
			// center - half the width of the string
			y += 6;
		}
		*/

		Color_t color = Vars::Menu::Theme::Active.Value;
		if (Spectator.m_bIsFriend)
			color = { 200, 255, 200, 255 };
		else if (Spectator.m_bRespawnTimeIncreased)
			color = { 255, 100, 100, 255 };
		else if (FNV1A::Hash32(Spectator.m_sMode.c_str()) == FNV1A::Hash32Const("1st"))
			color = { 255, 200, 127, 255 };
		H::Draw.String(fFont, x + iconOffset, y, color, align, "%s - %s (respawn %ds)", Spectator.m_sName.c_str(), Spectator.m_sMode.c_str(), Spectator.m_iRespawnIn);
	}
}
