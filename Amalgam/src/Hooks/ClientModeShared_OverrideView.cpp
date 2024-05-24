#include "../SDK/SDK.h"

#include "../Features/Visuals/Visuals.h"

MAKE_HOOK(ClientModeShared_OverrideView, U::Memory.GetVFunc(I::ClientModeShared, 16), void, __fastcall,
	void* ecx, CViewSetup* pView)
{
	CALL_ORIGINAL(ecx, pView);
	if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return;

	auto pLocal = H::Entities.GetLocal();
	if (pLocal && pView)
	{
		F::Visuals.FOV(pLocal, pView);
		F::Visuals.ThirdPerson(pLocal, pView);
	}
}