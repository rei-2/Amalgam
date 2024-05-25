#include "../SDK/SDK.h"

#include "../Features/Misc/Misc.h"
#include "../Features/Records/Records.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

MAKE_HOOK(BaseClientDLL_DispatchUserMessage, U::Memory.GetVFunc(I::BaseClientDLL, 36), bool, __fastcall,
	void* ecx, UserMessageType type, bf_read& msgData)
{
	const auto bufData = reinterpret_cast<const char*>(msgData.m_pData);
	msgData.SetAssertOnOverflow(false);
	msgData.Seek(0);

	switch (type)
	{
	case VoteStart:
		F::Records.UserMessage(msgData);

		break;
	/*
	case VoiceSubtitle:
	{
		int iEntityID = msgData.ReadByte();
		int iVoiceMenu = msgData.ReadByte();
		int iCommandID = msgData.ReadByte();

		if (iVoiceMenu == 1 && iCommandID == 6)
			F::...mMedicCallers.push_back(iEntityID);

		break;
	}
	*/
	case TextMsg:
		if (F::NoSpreadHitscan.ParsePlayerPerf(msgData))
			return true;

		if (Vars::Misc::Automation::AntiAutobalance.Value && msgData.GetNumBitsLeft() > 35)
		{
			static int anti_balance_attempts = 0;
			static std::string previous_name;

			auto pNetChan = I::EngineClient->GetNetChannelInfo();
			const std::string data(bufData);

			if (data.find("TeamChangeP") != std::string::npos && H::Entities.GetLocal())
			{
				const std::string serverName = pNetChan->GetAddress();
				if (serverName != previous_name)
				{
					previous_name = serverName;
					anti_balance_attempts = 0;
				}
				if (anti_balance_attempts < 2)
					I::EngineClient->ClientCmd_Unrestricted("retry");
				anti_balance_attempts++;
			}
		}
		break;
	case VGUIMenu:
		if (Vars::Visuals::Removals::MOTD.Value)
		{
			if (strcmp(reinterpret_cast<char*>(msgData.m_pData), "info") == 0)
			{
				I::EngineClient->ClientCmd_Unrestricted("closedwelcomemenu");
				return true;
			}
		}

		break;
	case ForcePlayerViewAngles:
		return Vars::Visuals::Removals::AngleForcing.Value ? true : CALL_ORIGINAL(ecx, type, msgData);
	case SpawnFlyingBird:
	case PlayerGodRayEffect:
	case PlayerTauntSoundLoopStart:
	case PlayerTauntSoundLoopEnd:
		return Vars::Visuals::Removals::Taunts.Value ? true : CALL_ORIGINAL(ecx, type, msgData);
	case Shake:
	case Fade:
	case Rumble:
		return Vars::Visuals::Removals::ScreenEffects.Value ? true : CALL_ORIGINAL(ecx, type, msgData);
	}

	msgData.Seek(0);
	return CALL_ORIGINAL(ecx, type, msgData);
}