#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerShared_IsPlayerDominated, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 63 F2 48 8B D9 E8", 0x0);
MAKE_SIGNATURE(CTFPlayerShared_IsPlayerDominated_Desired, "client.dll", "84 C0 74 ? 40 84 ED 74 ? 45 8B 86", 0x0);
MAKE_SIGNATURE(CTFPlayerShared_IsPlayerDominated_Jump, "client.dll", "8B F0 E8 ? ? ? ? 3B C7", 0x0);

MAKE_HOOK(CTFPlayerShared_IsPlayerDominated, S::CTFPlayerShared_IsPlayerDominated(), bool, __fastcall,
	void* ecx, int index)
{
	static auto dwDesired = S::CTFPlayerShared_IsPlayerDominated_Desired();
	static auto dwJump = S::CTFPlayerShared_IsPlayerDominated_Jump();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	const bool bResult = CALL_ORIGINAL(ecx, index);

	if (dwRetAddr == dwDesired && Vars::Visuals::UI::RevealScoreboard.Value && !bResult)
		*static_cast<uintptr_t*>(_AddressOfReturnAddress()) = dwJump;

	return bResult;
}