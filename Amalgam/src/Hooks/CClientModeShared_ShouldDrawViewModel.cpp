#include "../SDK/SDK.h"

#include "../Features/Visuals/Visuals.h"

MAKE_HOOK(CClientModeShared_ShouldDrawViewModel, U::Memory.GetVFunc(I::ClientModeShared, 24), bool,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CClientModeShared_ShouldDrawViewModel[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	auto pLocal = H::Entities.GetLocal();
	if (pLocal && pLocal->InCond(TF_COND_ZOOMED) && Vars::Visuals::Removals::Scope.Value && Vars::Visuals::UI::ZoomFieldOfView.Value > 20 && !I::Input->CAM_IsThirdPerson())
		return true;

	return CALL_ORIGINAL(rcx);
}