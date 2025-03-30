#include "../SDK/SDK.h"

MAKE_SIGNATURE(KeyValues_SetInt, "client.dll", "40 53 48 83 EC ? 41 8B D8 41 B0", 0x0);
MAKE_SIGNATURE(CTFClientScoreBoardDialog_UpdatePlayerList_SetInt_Call, "client.dll", "48 8B 05 ? ? ? ? 83 78 ? ? 0F 84 ? ? ? ? 48 8B 0D ? ? ? ? 8B D7", 0x0);
MAKE_SIGNATURE(CTFClientScoreBoardDialog_UpdatePlayerList_Jump, "client.dll", "8B E8 E8 ? ? ? ? 3B C7", 0x0);

MAKE_HOOK(KeyValues_SetInt, S::KeyValues_SetInt(), void,
	void* rcx, const char* keyName, int value)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::KeyValues_SetInt[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, keyName, value);
#endif

	static const auto dwDesired = S::CTFClientScoreBoardDialog_UpdatePlayerList_SetInt_Call();
	static const auto dwJump = S::CTFClientScoreBoardDialog_UpdatePlayerList_Jump();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	CALL_ORIGINAL(rcx, keyName, value);

	if (Vars::Visuals::UI::RevealScoreboard.Value && dwRetAddr == dwDesired && keyName && FNV1A::Hash32(keyName) == FNV1A::Hash32Const("nemesis"))
		*static_cast<uintptr_t*>(_AddressOfReturnAddress()) = dwJump;
}