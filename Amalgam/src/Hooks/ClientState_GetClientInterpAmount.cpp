#include "../SDK/SDK.h"

MAKE_SIGNATURE(ClientState_GetClientInterpAmount, "engine.dll", "48 83 EC ? 48 8B 0D ? ? ? ? 48 85 C9 75", 0x0);

MAKE_HOOK(ClientState_GetClientInterpAmount, S::ClientState_GetClientInterpAmount(), float, __fastcall,
	CClientState* ecx)
{
	G::Lerp = CALL_ORIGINAL(ecx);
	return Vars::Visuals::Removals::Interpolation.Value ? 0.f : G::Lerp;
}