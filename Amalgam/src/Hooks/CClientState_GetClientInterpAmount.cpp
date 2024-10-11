#include "../SDK/SDK.h"

MAKE_SIGNATURE(CClientState_GetClientInterpAmount, "engine.dll", "48 83 EC ? 48 8B 0D ? ? ? ? 48 85 C9 75", 0x0);

MAKE_HOOK(CClientState_GetClientInterpAmount, S::CClientState_GetClientInterpAmount(), float, __fastcall,
	CClientState* rcx)
{
	G::Lerp = CALL_ORIGINAL(rcx);
	return 0.f; //Vars::Visuals::Removals::Interpolation.Value ? 0.f : G::Lerp
}