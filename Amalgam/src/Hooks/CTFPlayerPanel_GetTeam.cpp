#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerPanel_GetTeam, "client.dll", "8B 91 ? ? ? ? 83 FA ? 74 ? 48 8B 05", 0x0);
MAKE_SIGNATURE(CTFTeamStatusPlayerPanel_GetTeam_Call, "client.dll", "8B 9F ? ? ? ? 40 32 F6", 0x0);

MAKE_HOOK(CTFPlayerPanel_GetTeam, S::CTFPlayerPanel_GetTeam(), int, __fastcall,
	void* rcx)
{
	static auto dwDesired = S::CTFTeamStatusPlayerPanel_GetTeam_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (Vars::Visuals::UI::RevealScoreboard.Value && dwRetAddr == dwDesired)
	{
		if (auto pResource = H::Entities.GetPR())
			return pResource->GetTeam(I::EngineClient->GetLocalPlayer());
	}

	return CALL_ORIGINAL(rcx);
}