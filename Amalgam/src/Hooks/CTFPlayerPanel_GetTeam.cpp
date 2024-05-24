#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayerPanel_GetTeam, "client.dll", "8B 91 ? ? ? ? 83 FA ? 74 ? 48 8B 05", 0x0);
MAKE_SIGNATURE(CTFPlayerPanel_GetTeam_Desired, "client.dll", "8B 9F ? ? ? ? 40 32 F6", 0x0);

MAKE_HOOK(CTFPlayerPanel_GetTeam, S::CTFPlayerPanel_GetTeam(), int, __fastcall,
	void* ecx)
{
	static auto dwDesired = S::CTFPlayerPanel_GetTeam_Desired();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	if (Vars::Visuals::UI::RevealScoreboard.Value && dwRetAddr == dwDesired)
	{
		auto pResource = H::Entities.GetPR();
		if (pResource)
			return pResource->GetTeam(I::EngineClient->GetLocalPlayer());
	}

	return CALL_ORIGINAL(ecx);
}