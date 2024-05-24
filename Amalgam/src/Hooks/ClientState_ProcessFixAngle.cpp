#include "../SDK/SDK.h"

MAKE_SIGNATURE(ClientState_ProcessFixAngle, "engine.dll", "40 53 48 83 EC ? F3 0F 10 42", 0x0);

MAKE_HOOK(ClientState_ProcessFixAngle, S::ClientState_ProcessFixAngle(), bool, __fastcall,
	CClientState* ecx, SVC_FixAngle* msg)
{
	return Vars::Visuals::Removals::AngleForcing.Value ? false : CALL_ORIGINAL(ecx, msg);
}