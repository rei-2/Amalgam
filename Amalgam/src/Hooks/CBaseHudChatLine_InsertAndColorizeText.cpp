#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CBaseHudChatLine_InsertAndColorizeText, "client.dll", "44 89 44 24 ? 55 53 56 57", 0x0);

MAKE_HOOK(CBaseHudChatLine_InsertAndColorizeText, S::CBaseHudChatLine_InsertAndColorizeText(), void,
	void* rcx, wchar_t* buf, int clientIndex)
{
	std::string sMessage = SDK::ConvertWideToUTF8(buf);

	if (clientIndex)
	{
		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(clientIndex, &pi))
			return CALL_ORIGINAL(rcx, buf, clientIndex);

		std::string sName = pi.name;
		auto iFind = sMessage.find(sName);

		int iType = 0;
		if (const char* sReplace = F::PlayerUtils.GetPlayerName(clientIndex, nullptr, &iType))
		{
			if (iFind != std::string::npos)
				sMessage = sMessage.replace(std::max(iFind - 1, 0ui64), sName.length() + 1, std::format("\x3{}\x1", sReplace));
			sName = sReplace;
		}

		if (Vars::Visuals::UI::ChatTags.Value && iType != 1)
		{
			std::string sTag, cColor;
			if (clientIndex == I::EngineClient->GetLocalPlayer())
			{
				if (Vars::Visuals::UI::ChatTags.Value & Vars::Visuals::UI::ChatTagsEnum::Local)
					sTag = "You", cColor = Vars::Colors::Local.Value.ToHexA();
			}
			else if (Vars::Visuals::UI::ChatTags.Value & Vars::Visuals::UI::ChatTagsEnum::Friends && H::Entities.IsFriend(clientIndex))
				sTag = "Friend", cColor = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(FRIEND_TAG)].Color.ToHexA();
			else if (Vars::Visuals::UI::ChatTags.Value & Vars::Visuals::UI::ChatTagsEnum::Party && H::Entities.InParty(clientIndex))
				sTag = "Party", cColor = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(PARTY_TAG)].Color.ToHexA();
			else if (Vars::Visuals::UI::ChatTags.Value & Vars::Visuals::UI::ChatTagsEnum::Assigned)
			{
				if (auto pTag = F::PlayerUtils.GetSignificantTag(clientIndex, 0))
					sTag = pTag->Name, cColor = pTag->Color.ToHexA();
			}

			if (!sTag.empty())
			{
				if (iFind != std::string::npos)
					sMessage.insert(iFind + sName.length(), "\x1");
				sMessage.insert(0, std::format("{}[{}] \x3", cColor, sTag));
			}
		}
	}

	if (Vars::Visuals::UI::StreamerMode.Value)
	{
		std::vector<std::pair<std::string, std::string>> vReplace;
		for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
		{
			PlayerInfo_t pi{}; int iType = 0;
			const char* sReplace = F::PlayerUtils.GetPlayerName(pEntity->entindex(), nullptr, &iType);
			if (sReplace && iType == 1 && I::EngineClient->GetPlayerInfo(pEntity->entindex(), &pi))
				vReplace.push_back({ pi.name, sReplace });
		}
		for (auto& [sFind, sReplace] : vReplace)
		{
			{
				std::string sReplace2 = sReplace;
				std::transform(sFind.begin(), sFind.end(), sFind.begin(), ::tolower);
				std::transform(sReplace2.begin(), sReplace2.end(), sReplace2.begin(), ::tolower);
				if (FNV1A::Hash32(sFind.c_str()) == FNV1A::Hash32(sReplace2.c_str()))
					continue;
			}

			while (true)
			{
				std::string sMessage2 = sMessage;
				std::transform(sMessage2.begin(), sMessage2.end(), sMessage2.begin(), ::tolower);

				auto iFind = sMessage2.find(sFind);
				if (iFind == std::string::npos)
					break;

				sMessage = sMessage.replace(iFind, sFind.length(), sReplace);
			}
		}
	}

	CALL_ORIGINAL(rcx, const_cast<wchar_t*>(SDK::ConvertUtf8ToWide(sMessage).c_str()), clientIndex);
}