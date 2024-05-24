#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBaseEntity_Interpolate, "client.dll", "4C 8B DC 49 89 5B ? F3 0F 11 4C 24", 0x0);

MAKE_HOOK(CBaseEntity_Interpolate, S::CBaseEntity_Interpolate(), bool, __fastcall,
	void* ecx, float currentTime)
{
	return !Vars::Visuals::Removals::Interpolation.Value ? CALL_ORIGINAL(ecx, currentTime) : true;
}