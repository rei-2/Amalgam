#include "../SDK/SDK.h"

#include "../Features/Players/PlayerUtils.h"

MAKE_SIGNATURE(CPlayerResource_GetPlayerName, "client.dll", "48 89 5C 24 ? 56 48 83 EC ? 48 63 F2", 0x0);

MAKE_HOOK(CPlayerResource_GetPlayerName, S::CPlayerResource_GetPlayerName(), const char*, __fastcall,
	void* rcx, int iIndex)
{
	return F::PlayerUtils.GetPlayerName(iIndex, CALL_ORIGINAL(rcx, iIndex));
}