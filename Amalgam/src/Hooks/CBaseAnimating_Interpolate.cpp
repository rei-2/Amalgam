#include "../SDK/SDK.h"

#include "../Features/TickHandler/TickHandler.h"
#include "../Features/Resolver/Resolver.h"

MAKE_SIGNATURE(CBaseAnimating_Interpolate, "client.dll", "48 8B C4 48 89 70 ? F3 0F 11 48", 0x0);

MAKE_HOOK(CBaseAnimating_Interpolate, S::CBaseAnimating_Interpolate(), bool,
	void* rcx, float currentTime)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseAnimating_Interpolate[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, currentTime);
#endif

	if (rcx == H::Entities.GetLocal() ? F::Ticks.m_bRecharge : Vars::Visuals::Removals::Interpolation.Value)
		return true;

	/*
	// fixes weird stupid flickering with the resolver, but completely disables interpolation
	if anyone knows of a better solution let me know. doesn't seem to cause nonvisual issues so i'm leaving this commented out for now
	if (F::Resolver.GetAngles(reinterpret_cast<CTFPlayer*>(rcx)))
		return true;
	*/

	return CALL_ORIGINAL(rcx, currentTime);
}