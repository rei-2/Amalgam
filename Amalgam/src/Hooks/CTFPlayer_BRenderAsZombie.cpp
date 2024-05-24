#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_BRenderAsZombie, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 B9 ? ? ? ? E8 ? ? ? ? 84 C0", 0x0);
MAKE_SIGNATURE(CTFRagdoll_CreateTFRagdoll_Call, "client.dll", "E8 ? ? ? ? 84 C0 74 ? C6 87", 0x5);

MAKE_HOOK(CTFPlayer_BRenderAsZombie, S::CTFPlayer_BRenderAsZombie(), bool, __fastcall,
	void* ecx, bool bWeaponsCheck)
{
	static auto dwDesired = S::CTFRagdoll_CreateTFRagdoll_Call();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	if (Vars::Visuals::Ragdolls::NoGib.Value && dwRetAddr == dwDesired)
		return true;

	return CALL_ORIGINAL(ecx, bWeaponsCheck);
}