#include "../SDK/SDK.h"

#include "../Features/Ticks/Ticks.h"

MAKE_SIGNATURE(CBaseViewModel_Interpolate, "client.dll", "48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 0F 29 70 ? 33 FF", 0x0);

MAKE_HOOK(CBaseViewModel_Interpolate, S::CBaseViewModel_Interpolate(), bool,
	void* rcx, float& currentTime)
{
	DEBUG_RETURN(CBaseEntity_BaseInterpolatePart1, rcx, currentTime);

	if (F::Ticks.m_bRecharge)
		return true;

	return CALL_ORIGINAL(rcx, currentTime);
}