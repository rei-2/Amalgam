#include "../SDK/SDK.h"

#include "../Features/TickHandler/TickHandler.h"

MAKE_SIGNATURE(CBaseAnimating_Interpolate, "client.dll", "48 8B C4 48 89 70 ? F3 0F 11 48", 0x0);

MAKE_HOOK(C_BaseAnimating_Interpolate, S::CBaseAnimating_Interpolate(), bool,
	void* rcx, float currentTime)
{
	if (rcx == H::Entities.GetLocal() ? F::Ticks.m_bRecharge : Vars::Visuals::Removals::Interpolation.Value)
		return true;

	return CALL_ORIGINAL(rcx, currentTime);
}