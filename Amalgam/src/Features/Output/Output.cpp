#include "Output.h"

#include "../Visuals/Notifications/Notifications.h"
#include "../Players/PlayerUtils.h"

static std::string sRed =		Color_t(255, 100, 100).ToHex();
static std::string sGreen =		Color_t(100, 255, 100).ToHex();
static std::string sYellow =	Color_t(200, 169, 0).ToHex();

static inline void OutputInfo(int flags, const char* sName, const char* sOutput, const char* sChat)
{
	SDK::Output(sName, sOutput, Vars::Menu::Theme::Accent.Value,
		flags & Vars::Logging::LogToEnum::Console,
		flags & Vars::Logging::LogToEnum::Debug,
		flags & Vars::Logging::LogToEnum::Toasts,
		flags & Vars::Logging::LogToEnum::Menu,
		false,
		flags & Vars::Logging::LogToEnum::Party
	);
	if (flags & Vars::Logging::LogToEnum::Chat)
		SDK::Output(Vars::Menu::CheatTag.Value.c_str(), sChat, Vars::Menu::Theme::Accent.Value, false, false, false, false, true, false, -1, "", "");
}

// Event info
void COutput::Event(IGameEvent* pEvent, uint32_t uHash, CTFPlayer* pLocal)
{
	if (uHash == FNV1A::Hash32Const("game_newmap"))
	{
		m_bInfoOnJoin = true;
		return;
	}

	if (!I::EngineClient->IsConnected() || !I::EngineClient->IsInGame() || !pLocal)
		return;

	switch (uHash)
	{
	case FNV1A::Hash32Const("vote_cast"): // Voting
	{
		if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::VoteCast))
			return;

		int iIndex = pEvent->GetInt("entityid");
		auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
		if (!pEntity || pEntity->GetClassID() != ETFClassID::CTFPlayer)
			return;

		bool bVotedYes = pEvent->GetInt("vote_option") == 0;
		bool bSameTeam = pEntity->As<CTFPlayer>()->m_iTeamNum() == pLocal->m_iTeamNum();

		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(iIndex, &pi))
			return;

		auto sName = F::PlayerUtils.GetPlayerName(iIndex, pi.name);
		OutputInfo(Vars::Logging::VoteCast::LogTo.Value, "Vote Cast",
			std::format("{}{} voted {}", (bSameTeam ? "" : "[Enemy] "), (sName), (bVotedYes ? "Yes" : "No")).c_str(),
			std::format("{}{}{}\x1 voted {}{}", (bSameTeam ? "" : "[Enemy] "), (sYellow), (sName), (bVotedYes ? sGreen : sRed), (bVotedYes ? "Yes" : "No")).c_str()
		);

		return;
	}
	case FNV1A::Hash32Const("player_changeclass"): // Class change
	{
		if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::ClassChanges))
			return;

		int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
		if (!pEntity || iIndex == pLocal->entindex())
			return;

		bool bSameTeam = pEntity->As<CTFPlayer>()->m_iTeamNum() == pLocal->m_iTeamNum();

		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(iIndex, &pi) || pi.fakeplayer)
			return; // dont spam chat by giving class changes for bots

		auto sName = F::PlayerUtils.GetPlayerName(iIndex, pi.name);
		OutputInfo(Vars::Logging::ClassChange::LogTo.Value, "Class Change",
			std::format("{}{} changed class to {}", (bSameTeam ? "" : "[Enemy] "), (sName), (SDK::GetClassByIndex(pEvent->GetInt("class")))).c_str(),
			std::format("{}{}{}\x1 changed class to {}{}", (bSameTeam ? "" : "[Enemy] "), (sYellow), (sName), (sYellow), (SDK::GetClassByIndex(pEvent->GetInt("class")))).c_str()
		);

		return;
	}
	case FNV1A::Hash32Const("player_hurt"): // Damage
	{
		if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Damage))
			return;

		int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
		if (!pEntity || iIndex == pLocal->entindex())
			return;

		int nAttacker = pEvent->GetInt("attacker");
		int nHealth = pEvent->GetInt("health");
		int nDamage = pEvent->GetInt("damageamount");
		bool bCrit = pEvent->GetBool("crit");
		bool bMinicrit = pEvent->GetBool("minicrit");
		int iMaxHealth = pEntity->As<CTFPlayer>()->GetMaxHealth();

		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(I::EngineClient->GetLocalPlayer(), &pi) || nAttacker != pi.userID ||
			!I::EngineClient->GetPlayerInfo(iIndex, &pi))
			return;

		auto sName = F::PlayerUtils.GetPlayerName(iIndex, pi.name);
		OutputInfo(Vars::Logging::Damage::LogTo.Value, "Damage",
			std::format("You hit {} for {} damage ({} / {}{})", (sName), (nDamage), (nHealth), (iMaxHealth), (bCrit ? ", crit" : bMinicrit ? ", minicrit" : "")).c_str(),
			std::format("You hit {}{}\x1 for {}{} damage{} ({} / {}{})", (sYellow), (sName), (sRed), (nDamage), (sYellow), (nHealth), (iMaxHealth), (bCrit ? ", crit" : bMinicrit ? ", minicrit" : "")).c_str()
		);

		return;
	}
	case FNV1A::Hash32Const("player_connect_client"): // tags/alias (player join)
	{
		if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Tags) && !(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Aliases) || m_bInfoOnJoin)
			return;

		std::string sID = pEvent->GetString("networkid");
		if (I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")) == pLocal->entindex()
			|| FNV1A::Hash32(sID.c_str()) == FNV1A::Hash32Const("BOT"))
			return;

		try
		{
			sID.replace(0, 5, "");
			sID.pop_back();
			uint32_t iID = std::stoi(sID);
			if (H::Entities.InParty(iID)) // ignore party
				return;

			auto sName = pEvent->GetString("name");
			TagsOnJoin(sName, iID);
			AliasOnJoin(sName, iID);
		}
		catch (...) {}

		return;
	}
	case FNV1A::Hash32Const("player_spawn"): // tags/alias (local join)
	{
		if (!(Vars::Logging::Logs.Value & (1 << 5)) && !(Vars::Logging::Logs.Value & (1 << 6)) || !m_bInfoOnJoin)
			return;

		if (I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")) != pLocal->entindex())
			return;

		m_bInfoOnJoin = false;
		for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
		{
			PlayerInfo_t pi{};
			if (n == pLocal->entindex() || !I::EngineClient->GetPlayerInfo(n, &pi) || pi.fakeplayer
				|| H::Entities.InParty(pi.friendsID)) // ignore party
				continue;

			TagsOnJoin(pi.name, pi.friendsID);
			AliasOnJoin(pi.name, pi.friendsID);
		}
	}
	}
}

// Vote start
void COutput::UserMessage(bf_read& msgData)
{
	if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::VoteStart))
		return;

	auto pLocal = H::Entities.GetLocal();
	if (!pLocal)
		return;

	int iTeam = msgData.ReadByte();
	/*int iVoteID =*/ msgData.ReadLong();
	int iCaller = msgData.ReadByte();
	char sReason[256]; msgData.ReadString(sReason, sizeof(sReason));
	char sTarget[256]; msgData.ReadString(sTarget, sizeof(sTarget));
	int iTarget = msgData.ReadByte() >> 1;
	msgData.Seek(0);
	bool bSameTeam = iTeam == pLocal->m_iTeamNum();

	PlayerInfo_t piTarget{}, piCaller{};
	if (!iCaller || !iTarget || !I::EngineClient->GetPlayerInfo(iCaller, &piCaller) || !I::EngineClient->GetPlayerInfo(iTarget, &piTarget))
		return;

	auto sTargetName = F::PlayerUtils.GetPlayerName(iTarget, piTarget.name);
	auto sCallerName = F::PlayerUtils.GetPlayerName(iCaller, piCaller.name);
	OutputInfo(Vars::Logging::VoteStart::LogTo.Value, "Vote Start",
		std::format("{}{} called a vote on {}", (bSameTeam ? "" : "[Enemy] "), (sCallerName), (sTargetName)).c_str(),
		std::format("{}{}{}\x1 called a vote on {}{}", (bSameTeam ? "" : "[Enemy] "), (sYellow), (sCallerName), (sYellow), (sTargetName)).c_str()
	);
}

// Cheat detection
void COutput::CheatDetection(const char* sName, const char* sAction, const char* sReason)
{
	if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::CheatDetection))
		return;

	OutputInfo(Vars::Logging::CheatDetection::LogTo.Value, "Cheat Detection",
		std::format("{} {} for {}", (sName), (sAction), (sReason)).c_str(),
		std::format("{}{}\x1 {} for {}{}", (sYellow), (sName), (sAction), (sYellow), (sReason)).c_str()
	);
}

// Tags
void COutput::TagsOnJoin(const char* sName, uint32_t uFriendsID)
{
	if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Tags))
		return;

	std::vector<std::pair<std::string, const char*>> vColorsTags = {};
	for (auto& iID : F::PlayerUtils.m_mPlayerTags[uFriendsID])
	{
		if (auto pTag = F::PlayerUtils.GetTag(iID))
			vColorsTags.emplace_back(pTag->m_tColor.ToHexA(), pTag->m_sName.c_str());
	}

	std::string sOutputText, sChatText;
	switch (vColorsTags.size())
	{
	case 0: return;
	case 1:
	{
		auto& pColorTag = *vColorsTags.begin();
		sOutputText = pColorTag.second;
		sChatText = std::format("{}{}", pColorTag.first, pColorTag.second);
		break;
	}
	case 2:
	{
		auto& pColorTag1 = *vColorsTags.begin(), &pColorTag2 = *(vColorsTags.begin() + 1);
		sOutputText = std::format("{} and {}", pColorTag1.second, pColorTag2.second);
		sChatText = std::format("{}{}\x1 and {}{}", pColorTag1.first, pColorTag1.second, pColorTag2.first, pColorTag2.second);
		break;
	}
	default:
	{
		for (auto it = vColorsTags.begin(); it != vColorsTags.end(); it++)
		{
			auto& pColorTag = *it;
			if (it + 1 != vColorsTags.end())
			{
				sOutputText += std::format("{}, ", pColorTag.second);
				sChatText += std::format("{}{}\x1, ", pColorTag.first, pColorTag.second);
			}
			else
			{
				sOutputText += std::format("and {}", pColorTag.second);
				sChatText += std::format("and {}{}", pColorTag.first, pColorTag.second);
			}
		}
	}
	}

	OutputInfo(Vars::Logging::Tags::LogTo.Value, "Tags",
		std::format("{} has the {} {}", (sName), (vColorsTags.size() == 1 ? "tag" : "tags"), (sOutputText)).c_str(),
		std::format("{}{}\x1 has the {} {}", (sYellow), (sName), (vColorsTags.size() == 1 ? "tag" : "tags"), (sChatText)).c_str()
	);
}
void COutput::TagsChanged(const char* sName, const char* sAction, const char* sColor, const char* sTag)
{
	if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Tags))
		return;

	auto uHash = FNV1A::Hash32(sAction);
	OutputInfo(Vars::Logging::Tags::LogTo.Value, "Tags",
		std::format("{} tag {} {} {}", (sAction), (sTag), (uHash == FNV1A::Hash32Const("Added") ? "to" : "from"), (sName)).c_str(),
		std::format("{} tag {}{}\x1 {} {}{}", (sAction), (sColor), (sTag), (uHash == FNV1A::Hash32Const("Added") ? "to" : "from"), (sYellow), (sName)).c_str()
	);
}

// Aliases
void COutput::AliasOnJoin(const char* sName, uint32_t uFriendsID)
{
	if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Aliases))
		return;

	if (!F::PlayerUtils.m_mPlayerAliases.contains(uFriendsID))
		return;

	auto& sAlias = F::PlayerUtils.m_mPlayerAliases[uFriendsID];

	OutputInfo(Vars::Logging::Tags::LogTo.Value, "Aliases",
		std::format("{} has the alias \"{}\"", (sName), (sAlias)).c_str(),
		std::format("{}{}\x1 has the alias \"{}{}\x1\"", (sYellow), (sName), (sYellow), (sAlias)).c_str()
	);
}
void COutput::AliasChanged(const char* sName, const char* sAction, const char* sAlias)
{
	if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Aliases))
		return;

	auto uHash = FNV1A::Hash32(sAction);
	OutputInfo(Vars::Logging::Tags::LogTo.Value, "Aliases",
		std::format("{} {}'s alias {} \"{}\"", (sAction), (sName), (uHash == FNV1A::Hash32Const("Changed") ? "to" : "of"), (sAlias)).c_str(),
		std::format("{} {}{}\x1's alias {} \"{}{}\x1\"", (sAction), (sYellow), (sName), (uHash == FNV1A::Hash32Const("Changed") ? "to" : "of"), (sYellow), (sAlias)).c_str()
	);
}

void COutput::ReportResolver(int iIndex, const char* sAction, const char* sAxis, float flValue)
{
	ReportResolver(iIndex, sAction, sAxis, std::format("{}", flValue).c_str());
}
void COutput::ReportResolver(int iIndex, const char* sAction, const char* sAxis, bool bValue)
{
	ReportResolver(iIndex, sAction, sAxis, std::format("{}", bValue).c_str());
}
void COutput::ReportResolver(int iIndex, const char* sAction, const char* sAxis, const char* sValue)
{
	if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Resolver))
		return;

	PlayerInfo_t pi{};
	if (!I::EngineClient->GetPlayerInfo(iIndex, &pi))
		return;

	auto sName = F::PlayerUtils.GetPlayerName(iIndex, pi.name);
	OutputInfo(Vars::Logging::Tags::LogTo.Value, "Resolver",
		std::format("{} {} of {} to {}", (sAction), (sAxis), (sName), (sValue)).c_str(),
		std::format("{} {}{}\x1 of {}{}\x1 to {}{}\x1", (sAction), (sYellow), (sAxis), (sYellow), (sName), (sYellow), (sValue)).c_str()
	);
}
void COutput::ReportResolver(const char* sMessage)
{
	if (!(Vars::Logging::Logs.Value & Vars::Logging::LogsEnum::Resolver))
		return;

	OutputInfo(Vars::Logging::Tags::LogTo.Value, "Resolver", sMessage, sMessage);
}