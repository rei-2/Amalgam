#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseAnimating_UpdateClientSideAnimation, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B F8 48 85 C0 74 ? 48 8B 00 48 8B CF FF 90 ? ? ? ? 84 C0 75 ? 33 FF 48 3B DF", 0x0);

MAKE_HOOK(CBaseAnimating_UpdateClientSideAnimation, S::CBaseAnimating_UpdateClientSideAnimation(), void,
	void* rcx)
{
	auto pLocal = H::Entities.GetLocal();
	if (Vars::Visuals::Removals::Interpolation.Value && !G::UpdatingAnims || rcx == pLocal && !pLocal->InCond(TF_COND_HALLOWEEN_KART) && !I::EngineClient->IsPlayingDemo())
		return;

	CALL_ORIGINAL(rcx);
}