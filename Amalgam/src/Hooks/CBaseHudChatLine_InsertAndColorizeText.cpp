#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CBaseHudChatLine_InsertAndColorizeText, "client.dll", "44 89 44 24 ? 55 53 56 57", 0x0);

MAKE_HOOK(CBaseHudChatLine_InsertAndColorizeText, S::CBaseHudChatLine_InsertAndColorizeText(), void,
	void* rcx, wchar_t* buf, int clientIndex)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseHudChatLine_InsertAndColorizeText[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, buf, clientIndex);
#endif

	auto pResource = H::Entities.GetResource();
	if (!pResource || !pResource->IsValid(clientIndex))
		return CALL_ORIGINAL(rcx, buf, clientIndex);

	std::string sMessage = SDK::ConvertWideToUTF8(buf);
	const char* sName = pResource->GetName(clientIndex);
	auto iFind = sMessage.find(sName);

	int iType;
	if (const char* sReplace = F::PlayerUtils.GetPlayerName(clientIndex, nullptr, &iType))
	{
		if (iFind != std::string::npos)
			sMessage = sMessage.replace(std::max(iFind - 1, 0ui64), strlen(sName) + 1, std::format("\x3{}\x1", sReplace));
		sName = sReplace;
	}

	if (Vars::Visuals::UI::ChatTags.Value && !(iType & NameTypeEnum::Privacy))
	{
		std::string sTag, cColor;
		if (Vars::Visuals::UI::ChatTags.Value & Vars::Visuals::UI::ChatTagsEnum::Local && clientIndex == I::EngineClient->GetLocalPlayer())
			sTag = "You", cColor = Vars::Colors::Local.Value.ToHexA();
		else if (Vars::Visuals::UI::ChatTags.Value & Vars::Visuals::UI::ChatTagsEnum::Friends && H::Entities.IsFriend(clientIndex))
			sTag = "Friend", cColor = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(FRIEND_TAG)].m_tColor.ToHexA();
		else if (Vars::Visuals::UI::ChatTags.Value & Vars::Visuals::UI::ChatTagsEnum::Party && H::Entities.InParty(clientIndex))
			sTag = "Party", cColor = F::PlayerUtils.m_vTags[F::PlayerUtils.TagToIndex(PARTY_TAG)].m_tColor.ToHexA();
		else if (Vars::Visuals::UI::ChatTags.Value & Vars::Visuals::UI::ChatTagsEnum::Assigned)
		{
			if (auto pTag = F::PlayerUtils.GetSignificantTag(clientIndex, 0))
				sTag = pTag->m_sName, cColor = pTag->m_tColor.ToHexA();
		}

		if (!sTag.empty())
		{
			if (iFind != std::string::npos)
				sMessage.insert(iFind + strlen(sName), "\x1");
			sMessage.insert(0, std::format("{}[{}] \x3", cColor, sTag));
		}
	}

	if (Vars::Visuals::UI::StreamerMode.Value)
	{
		std::vector<std::pair<std::string, std::string>> vReplace;
		for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
		{
			if (!pResource->m_bValid(n))
				continue;

			int iType; const char* sReplace = F::PlayerUtils.GetPlayerName(n, nullptr, &iType);
			if (!sReplace || !(iType & NameTypeEnum::Privacy))
				continue;

			vReplace.emplace_back(pResource->GetName(n), sReplace);
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

			size_t iPos = 0;
			while (true)
			{
				std::string sMessage2 = sMessage;
				std::transform(sMessage2.begin(), sMessage2.end(), sMessage2.begin(), ::tolower);

				auto iFind = sMessage2.find(sFind, iPos);
				if (iFind == std::string::npos)
					break;

				iPos = iFind + sReplace.length();
				sMessage = sMessage.replace(iFind, sFind.length(), sReplace);
			}
		}
	}

	CALL_ORIGINAL(rcx, const_cast<wchar_t*>(SDK::ConvertUtf8ToWide(sMessage).c_str()), clientIndex);
}