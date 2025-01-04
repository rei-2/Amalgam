#include "../SDK/SDK.h"

#include "../Features/Misc/Misc.h"
#include "../Features/Records/Records.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/Misc/AutoVote/AutoVote.h"
#include "../Features/Aimbot/AutoHeal/AutoHeal.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>

MAKE_HOOK(IBaseClientDLL_DispatchUserMessage, U::Memory.GetVFunc(I::BaseClientDLL, 36), bool,
	void* rcx, UserMessageType type, bf_read& msgData)
{
	auto bufData = reinterpret_cast<const char*>(msgData.m_pData);
	msgData.SetAssertOnOverflow(false);
	msgData.Seek(0);

	switch (type)
	{
	case VoteStart:
		F::Records.UserMessage(msgData);
		F::AutoVote.UserMessage(msgData);

		break;
	case VoiceSubtitle:
	{
		int iEntityID = msgData.ReadByte();
		int iVoiceMenu = msgData.ReadByte();
		int iCommandID = msgData.ReadByte();
		if (iVoiceMenu == 1 && iCommandID == 6)
			F::AutoHeal.m_mMedicCallers[iEntityID] = true;

		break;
	}
	case TextMsg:
		if (F::NoSpreadHitscan.ParsePlayerPerf(msgData))
			return true;

		if (Vars::Misc::Automation::AntiAutobalance.Value && bufData && msgData.GetNumBitsLeft() > 35)
		{
			std::string sMsg = bufData;
			if (!sMsg.empty())
			{
				sMsg.erase(0, 1);
				if (FNV1A::Hash32(sMsg.c_str()) == FNV1A::Hash32Const("#TF_Autobalance_TeamChangePending"))
					I::EngineClient->ClientCmd_Unrestricted("retry");
			}
		}
		break;
	case VGUIMenu:
		if (Vars::Visuals::Removals::MOTD.Value && bufData
			&& FNV1A::Hash32(bufData) == FNV1A::Hash32Const("info"))
		{
			I::EngineClient->ClientCmd_Unrestricted("closedwelcomemenu");
			return true;
		}
		break;
	case ForcePlayerViewAngles:
		return Vars::Visuals::Removals::AngleForcing.Value ? true : CALL_ORIGINAL(rcx, type, msgData);
	case SpawnFlyingBird:
	case PlayerGodRayEffect:
	case PlayerTauntSoundLoopStart:
	case PlayerTauntSoundLoopEnd:
		return Vars::Visuals::Removals::Taunts.Value ? true : CALL_ORIGINAL(rcx, type, msgData);
	case Shake:
	case Fade:
	case Rumble:
		return Vars::Visuals::Removals::ScreenEffects.Value ? true : CALL_ORIGINAL(rcx, type, msgData);
	}

	msgData.Seek(0);
	return CALL_ORIGINAL(rcx, type, msgData);
}