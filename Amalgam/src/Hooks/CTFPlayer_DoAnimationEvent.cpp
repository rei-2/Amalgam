#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_DoAnimationEvent, "client.dll", "48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B E8 8B FA", 0x0);

MAKE_HOOK(CTFPlayer_DoAnimationEvent, S::CTFPlayer_DoAnimationEvent(), void,
	void* rcx, PlayerAnimEvent_t event, int nData)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayer_DoAnimationEvent[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, event, nData);
#endif

	auto pPlayer = reinterpret_cast<CTFPlayer*>(rcx);
	if (pPlayer->entindex() != I::EngineClient->GetLocalPlayer())
		return;

	CALL_ORIGINAL(rcx, event, nData);
}