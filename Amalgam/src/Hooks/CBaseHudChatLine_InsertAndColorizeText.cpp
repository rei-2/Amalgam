#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CBaseHudChatLine_InsertAndColorizeText, "client.dll", "44 89 44 24 ? 55 53 56 57", 0x0);

MAKE_HOOK(CBaseHudChatLine_InsertAndColorizeText, S::CBaseHudChatLine_InsertAndColorizeText(), void,
	void* rcx, wchar_t* buf, int clientIndex)
{
	PlayerInfo_t pi{};
	if (!clientIndex || !I::EngineClient->GetPlayerInfo(clientIndex, &pi))
		return CALL_ORIGINAL(rcx, buf, clientIndex);

	std::string sMessage = SDK::ConvertWideToUTF8(buf);
	std::string sName = pi.name;
	auto iFind = sMessage.find(pi.name);
	bool bFound = iFind != std::string::npos;

	int iType = 0;
	if (const char* sReplace = F::PlayerUtils.GetPlayerName(clientIndex, nullptr, &iType))
	{
		if (bFound)
			sMessage = sMessage.replace(std::max(iFind - 1, 0ui64), sName.length() + 1, std::format("\x3{}\x1", sReplace));
		if (iType == 1)
			return CALL_ORIGINAL(rcx, const_cast<wchar_t*>(SDK::ConvertUtf8ToWide(sMessage).c_str()), clientIndex);
		else
			sName = sReplace;
	}

	if (Vars::Visuals::UI::ChatTags.Value)
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
			if (bFound)
				sMessage.insert(iFind + sName.length(), "\x1");
			sMessage.insert(0, std::format("{}[{}] \x3", cColor, sTag));
		}
	}

	CALL_ORIGINAL(rcx, const_cast<wchar_t*>(SDK::ConvertUtf8ToWide(sMessage).c_str()), clientIndex);
}