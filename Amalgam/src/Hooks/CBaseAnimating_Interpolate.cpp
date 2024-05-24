#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseAnimating_Interpolate, "client.dll", "48 8B C4 48 89 70 ? F3 0F 11 48", 0x0);

MAKE_HOOK(C_BaseAnimating_Interpolate, S::CBaseAnimating_Interpolate(), bool, __fastcall,
	void* ecx, float currentTime)
{
	if (ecx == H::Entities.GetLocal() ? G::Recharge : Vars::Visuals::Removals::Interpolation.Value)
		return true;

	return CALL_ORIGINAL(ecx, currentTime);
}