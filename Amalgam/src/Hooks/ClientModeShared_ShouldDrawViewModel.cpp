#include "../SDK/SDK.h"

#include "../Features/Visuals/Visuals.h"

MAKE_HOOK(ClientModeShared_ShouldDrawViewModel, U::Memory.GetVFunc(I::ClientModeShared, 24), bool, __fastcall,
	void* ecx)
{
	if (auto pLocal = H::Entities.GetLocal())
	{
		if (pLocal->IsScoped() && Vars::Visuals::Removals::Scope.Value && Vars::Visuals::UI::ZoomFieldOfView.Value > 70 && !I::Input->CAM_IsThirdPerson())
			return true;
	}

	return CALL_ORIGINAL(ecx);
}