#include "../SDK/SDK.h"

#include "../Features/Misc/Misc.h"

MAKE_SIGNATURE(ClientModeTFNormal_UpdateSteamRichPresence, "client.dll", "4C 8B DC 41 54 41 55 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 45 33 ED 49 89 5B ? 48 8B D9", 0x0);

MAKE_HOOK(ClientModeTFNormal_UpdateSteamRichPresence, S::ClientModeTFNormal_UpdateSteamRichPresence(), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::ClientModeTFNormal_UpdateSteamRichPresence[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	if (!F::Misc.SteamRPC())
		CALL_ORIGINAL(rcx);
}