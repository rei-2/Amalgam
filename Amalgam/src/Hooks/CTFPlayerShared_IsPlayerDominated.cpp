#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerShared_IsPlayerDominated, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 63 F2 48 8B D9 E8", 0x0);
MAKE_SIGNATURE(CTFClientScoreBoardDialog_UpdatePlayerList_IsPlayerDominated_Call, "client.dll", "84 C0 74 ? 45 84 FF 74", 0x0);
MAKE_SIGNATURE(KeyValues_SetInt, "client.dll", "40 53 48 83 EC ? 41 8B D8 41 B0", 0x0);
MAKE_SIGNATURE(CTFClientScoreBoardDialog_UpdatePlayerList_SetInt_Call, "client.dll", "48 8B 05 ? ? ? ? 83 78 ? ? 0F 84 ? ? ? ? 48 8B 0D ? ? ? ? 8B D7", 0x0);
MAKE_SIGNATURE(CTFClientScoreBoardDialog_UpdatePlayerList_Jump, "client.dll", "8B E8 E8 ? ? ? ? 3B C7", 0x0);
MAKE_SIGNATURE(CClientScoreBoardDialog_NeedsUpdate, "client.dll", "48 8B 05 ? ? ? ? F3 0F 10 41 ? 0F 2F 40 ? 0F 92 C0", 0x0);

MAKE_HOOK(CTFPlayerShared_IsPlayerDominated, S::CTFPlayerShared_IsPlayerDominated(), bool,
	void* rcx, int index)
{
	DEBUG_RETURN(CTFPlayerShared_IsPlayerDominated, rcx, index);

	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	const auto dwDesired = S::CTFClientScoreBoardDialog_UpdatePlayerList_IsPlayerDominated_Call();
	const auto dwJump = S::CTFClientScoreBoardDialog_UpdatePlayerList_Jump();

	bool bReturn = CALL_ORIGINAL(rcx, index);

	if (dwRetAddr == dwDesired && Vars::Visuals::UI::RevealScoreboard.Value && !SDK::CleanScreenshot() && !bReturn)
	{
		// both addresses live inside CClientScoreBoardDialog::UpdatePlayerList,
		// if they drift apart after a game update the signatures no longer point into the same function
		// and redirecting execution there would jump to an arbitrary location with a mismatched stack
		if (dwJump && (dwJump > dwDesired ? dwJump - dwDesired : dwDesired - dwJump) < 0x10000)
			*static_cast<uintptr_t*>(_AddressOfReturnAddress()) = dwJump;
	}

	return bReturn;
}

MAKE_HOOK(KeyValues_SetInt, S::KeyValues_SetInt(), void,
	void* rcx, const char* keyName, int value)
{
	DEBUG_RETURN(KeyValues_SetInt, rcx, keyName, value);

	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	const auto dwDesired = S::CTFClientScoreBoardDialog_UpdatePlayerList_SetInt_Call();
	const auto dwJump = S::CTFClientScoreBoardDialog_UpdatePlayerList_Jump();

	CALL_ORIGINAL(rcx, keyName, value);

	if (dwRetAddr == dwDesired && Vars::Visuals::UI::RevealScoreboard.Value && !SDK::CleanScreenshot() && keyName && FNV1A::Hash32(keyName) == FNV1A::Hash32Const("nemesis"))
	{
		if (dwJump && (dwJump > dwDesired ? dwJump - dwDesired : dwDesired - dwJump) < 0x10000)
			*static_cast<uintptr_t*>(_AddressOfReturnAddress()) = dwJump;
	}
}

MAKE_HOOK(CClientScoreBoardDialog_NeedsUpdate, S::CClientScoreBoardDialog_NeedsUpdate(), bool,
	void* rcx)
{
	DEBUG_RETURN(CClientScoreBoardDialog_NeedsUpdate, rcx);

	static bool bStaticMod = false;
	const bool bLastMod = bStaticMod;
	const bool bCurrMod = bStaticMod = (Vars::Visuals::UI::RevealScoreboard.Value || Vars::Visuals::UI::ScoreboardColors.Value) && !SDK::CleanScreenshot();

	if (bCurrMod != bLastMod)
		return true;

	return CALL_ORIGINAL(rcx);
}