#include "Color.h"

#include "../../Vars.h"
#include "../../../Features/Players/PlayerUtils.h"

Color_t CColor::GetTeamColor(int iLocalTeam, int iTargetTeam, bool bOther)
{
	if (bOther)
		return iLocalTeam == iTargetTeam ? Vars::Colors::Team.Value : Vars::Colors::Enemy.Value;
	else
	{
		switch (iTargetTeam)
		{
		case 2: return Vars::Colors::TeamRed.Value;
		case 3: return Vars::Colors::TeamBlu.Value;
		}
	}

	return { 255, 255, 255, 255 };
}

Color_t CColor::GetEntityDrawColor(CTFPlayer* pLocal, CBaseEntity* pEntity, bool enableOtherColors)
{
	Color_t out = GetTeamColor(pLocal->m_iTeamNum(), pEntity->m_iTeamNum(), enableOtherColors);

	if (pEntity->IsPlayer())
	{
		auto pPlayer = pEntity->As<CTFPlayer>();

		if (pLocal == pPlayer)
			out = Vars::Colors::Local.Value;
		else if (H::Entities.IsFriend(pPlayer->entindex()))
			out = F::PlayerUtils.mTags["Friend"].Color;

		PlayerInfo_t pi{}; bool bTagColor = false; Color_t cTagColor;
		if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi))
		{
			std::string _; PriorityLabel_t plTag;
			if (bTagColor = F::PlayerUtils.GetSignificantTag(pi.friendsID, &_, &plTag))
				cTagColor = plTag.Color;
		}

		if (bTagColor)
			out = cTagColor;
		else if (pPlayer->IsCloaked())
			out = Vars::Colors::Cloak.Value;
		else if (pPlayer->IsInvulnerable())
			out = Vars::Colors::Invulnerable.Value;
	}

	if (pEntity->entindex() == G::Target.first && abs(G::Target.second - I::GlobalVars->tickcount) < 32)
		out = Vars::Colors::Target.Value;

	return out;
}