#include "../SDK/SDK.h"

MAKE_SIGNATURE(CClientState_ProcessFixAngle, "engine.dll", "40 53 48 83 EC ? F3 0F 10 42", 0x0);

MAKE_HOOK(CClientState_ProcessFixAngle, S::CClientState_ProcessFixAngle(), bool,
	CClientState* rcx, SVC_FixAngle* msg)
{
	return Vars::Visuals::Removals::AngleForcing.Value ? false : CALL_ORIGINAL(rcx, msg);
}