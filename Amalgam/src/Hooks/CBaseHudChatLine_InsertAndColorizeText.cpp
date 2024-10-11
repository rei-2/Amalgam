#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CBaseHudChatLine_InsertAndColorizeText, "client.dll", "44 89 44 24 ? 55 53 56 57", 0x0);

MAKE_HOOK(CBaseHudChatLine_InsertAndColorizeText, S::CBaseHudChatLine_InsertAndColorizeText(), void, __fastcall,
	void* rcx, wchar_t* buf, int clientIndex)
{
	PlayerInfo_t pi{};
	if (!clientIndex || !I::EngineClient->GetPlayerInfo(clientIndex, &pi))
		return CALL_ORIGINAL(rcx, buf, clientIndex);

	bool bLocal = false, bFriend = false, bEnemy = false;
	PriorityLabel_t* pTag = nullptr;
	if (Vars::Visuals::UI::ChatTags.Value || Vars::Visuals::UI::StreamerMode.Value)
	{
		if (clientIndex == I::EngineClient->GetLocalPlayer())
			bLocal = true;
		else if (H::Entities.IsFriend(clientIndex))
			bFriend = true;
		else
		{
			auto pTag = F::PlayerUtils.GetSignificantTag(clientIndex, 0);
			if (!pTag)
			{
				auto pResource = H::Entities.GetPR();
				bEnemy = !pResource || pResource->GetTeam(I::EngineClient->GetLocalPlayer()) != pResource->GetTeam(clientIndex);
			}
		}
	}

	std::string sMessage = SDK::ConvertWideToUTF8(buf);
	std::string sName = pi.name;
	auto iFind = sMessage.find(pi.name);
	bool bFound = iFind != std::string::npos;

	if (bFound)
	{
		int iType = 0;
		if (const char* sReplace = F::PlayerUtils.GetPlayerName(clientIndex, nullptr, &iType))
		{
			sMessage = sMessage.replace(std::max(iFind - 1, 0ui64), sName.length() + 1, std::format("\x3{}\x1", sReplace));
			if (iType == 1)
				return CALL_ORIGINAL(rcx, const_cast<wchar_t*>(SDK::ConvertUtf8ToWide(sMessage).c_str()), clientIndex);
			else
				sName = sReplace;
		}
	}

	if (Vars::Visuals::UI::ChatTags.Value)
	{
		std::string sTag, cColor;
		if (bLocal)
		{
			if (Vars::Visuals::UI::ChatTags.Value & (1 << 0))
				sTag = "You", cColor = Vars::Colors::Local.Value.ToHexA();
		}
		else if (bFriend)
		{
			if (Vars::Visuals::UI::ChatTags.Value & (1 << 1))
				sTag = "Friend", cColor = F::PlayerUtils.m_vTags[FRIEND_TAG].Color.ToHexA();
		}
		else if (pTag)
		{
			if (Vars::Visuals::UI::ChatTags.Value & (1 << 2))
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