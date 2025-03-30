#include "../SDK/SDK.h"

MAKE_SIGNATURE(CHudInspectPanel_UserCmd_InspectTarget_BIsFriendOrPartyMember_Call, "client.dll", "84 C0 75 ? 48 8B 0D ? ? ? ? 48 8D 15 ? ? ? ? 48 8B 01 FF 50 ? E9", 0x0);

MAKE_HOOK(ClientModeTFNormal_BIsFriendOrPartyMember, S::ClientModeTFNormal_BIsFriendOrPartyMember(), bool,
	void* rcx, CBaseEntity* pEntity)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::ClientModeTFNormal_BIsFriendOrPartyMember[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pEntity);
#endif

	static const auto dwDesired = S::CHudInspectPanel_UserCmd_InspectTarget_BIsFriendOrPartyMember_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (dwRetAddr == dwDesired && Vars::Misc::MannVsMachine::AllowInspect.Value)
		return true;
	return CALL_ORIGINAL(rcx, pEntity);
}