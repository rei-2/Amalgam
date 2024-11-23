#include "AutoVote.h"

#include "../../Players/PlayerUtils.h"

void CAutoVote::UserMessage(bf_read& msgData)
{
	/*const int iTeam =*/ msgData.ReadByte();
	const int iVoteID = msgData.ReadLong();
	/*const int iCaller =*/ msgData.ReadByte();
	char sReason[256]; msgData.ReadString(sReason, sizeof(sReason));
	char sTarget[256]; msgData.ReadString(sTarget, sizeof(sTarget));
	const int iTarget = msgData.ReadByte() >> 1;
	msgData.Seek(0);

	if (Vars::Misc::Automation::AutoF2Ignored.Value && F::PlayerUtils.IsIgnored(iTarget))
		I::ClientState->SendStringCmd(std::format("vote {} option2", iVoteID).c_str());
	else if (Vars::Misc::Automation::AutoF1Priority.Value && F::PlayerUtils.GetPriority(iTarget) > F::PlayerUtils.m_vTags[DEFAULT_TAG].Priority && !H::Entities.IsFriend(iTarget))
		I::ClientState->SendStringCmd(std::format("vote {} option1", iVoteID).c_str());
}