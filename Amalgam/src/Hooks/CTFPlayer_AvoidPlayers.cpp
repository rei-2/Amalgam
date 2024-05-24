#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_AvoidPlayers, "client.dll", "48 89 54 24 ? 55 41 56", 0x0);

MAKE_HOOK(CTFPlayer_AvoidPlayers, S::CTFPlayer_AvoidPlayers(), void, __fastcall,
	void* ecx, CUserCmd* pCmd)
{
	if (Vars::Misc::Movement::NoPush.Value)
		return;

	CALL_ORIGINAL(ecx, pCmd);
}