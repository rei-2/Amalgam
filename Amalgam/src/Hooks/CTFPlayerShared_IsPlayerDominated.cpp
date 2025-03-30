#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerShared_IsPlayerDominated, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 63 F2 48 8B D9 E8", 0x0);
MAKE_SIGNATURE(CTFClientScoreBoardDialog_UpdatePlayerList_IsPlayerDominated_Call, "client.dll", "84 C0 74 ? 45 84 FF 74", 0x0);
MAKE_SIGNATURE(CTFClientScoreBoardDialog_UpdatePlayerList_Jump, "client.dll", "8B E8 E8 ? ? ? ? 3B C7", 0x0);

MAKE_HOOK(CTFPlayerShared_IsPlayerDominated, S::CTFPlayerShared_IsPlayerDominated(), bool,
	void* rcx, int index)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayerShared_IsPlayerDominated[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, index);
#endif

	static const auto dwDesired = S::CTFClientScoreBoardDialog_UpdatePlayerList_IsPlayerDominated_Call();
	static const auto dwJump = S::CTFClientScoreBoardDialog_UpdatePlayerList_Jump();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	const bool bResult = CALL_ORIGINAL(rcx, index);

	if (Vars::Visuals::UI::RevealScoreboard.Value && dwRetAddr == dwDesired && !bResult)
		*static_cast<uintptr_t*>(_AddressOfReturnAddress()) = dwJump;

	return bResult;
}