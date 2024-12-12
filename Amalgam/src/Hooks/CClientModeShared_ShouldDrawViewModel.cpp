#include "../SDK/SDK.h"

#include "../Features/Visuals/Visuals.h"

MAKE_HOOK(CClientModeShared_ShouldDrawViewModel, U::Memory.GetVFunc(I::ClientModeShared, 24), bool,
	void* rcx)
{
	auto pLocal = H::Entities.GetLocal();
	if (pLocal && pLocal->IsScoped() && Vars::Visuals::Removals::Scope.Value && Vars::Visuals::UI::ZoomFieldOfView.Value > 20 && !I::Input->CAM_IsThirdPerson())
		return true;

	return CALL_ORIGINAL(rcx);
}