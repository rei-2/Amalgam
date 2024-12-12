#include "../SDK/SDK.h"

#include "../Features/Spectate/Spectate.h"

MAKE_SIGNATURE(CTFPlayer_ShouldDraw, "client.dll", "48 89 74 24 ? 57 48 83 EC ? 48 8D 71", 0x0);
MAKE_SIGNATURE(CBasePlayer_ShouldDrawThisPlayer, "client.dll", "48 83 EC ? E8 ? ? ? ? 84 C0 74 ? 48 8B 0D ? ? ? ? 48 85 C9", 0x0);
MAKE_SIGNATURE(CBasePlayer_BuildFirstPersonMeathookTransformations_ShouldDrawThisPlayer_Call, "client.dll", "84 C0 0F 84 ? ? ? ? 48 8B 94 24 ? ? ? ? 48 8B CE", 0x0);
MAKE_SIGNATURE(CViewRender_DrawViewModels, "client.dll", "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 8B FA", 0x0);

MAKE_HOOK(CTFPlayer_ShouldDraw, S::CTFPlayer_ShouldDraw(), bool,
	void* rcx)
{
	auto pLocal = H::Entities.GetLocal();
	if (pLocal && F::Spectate.m_iTarget != -1 && pLocal->m_hObserverTarget().Get())
		return Vars::Visuals::ThirdPerson::Enabled.Value ? true : rcx != pLocal->m_hObserverTarget().Get()->GetClientRenderable();

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(CBasePlayer_ShouldDrawThisPlayer, S::CBasePlayer_ShouldDrawThisPlayer(), bool,
	void* rcx)
{
	static const auto dwUndesired = S::CBasePlayer_BuildFirstPersonMeathookTransformations_ShouldDrawThisPlayer_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	auto pLocal = H::Entities.GetLocal();
	if (pLocal && F::Spectate.m_iTarget != -1)
	{
		if (dwRetAddr == dwUndesired)
			return false;
		return Vars::Visuals::ThirdPerson::Enabled.Value ? true : rcx != pLocal->m_hObserverTarget().Get();
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(CViewRender_DrawViewModels, S::CViewRender_DrawViewModels(), void,
	void* rcx, const CViewSetup& viewRender, bool drawViewmodel)
{
	CALL_ORIGINAL(rcx, viewRender, F::Spectate.m_iTarget != -1 ? false : drawViewmodel);
}