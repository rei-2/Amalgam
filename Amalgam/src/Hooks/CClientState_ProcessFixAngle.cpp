#include "../SDK/SDK.h"
#include "../Features/Spectate/Spectate.h"

MAKE_SIGNATURE(CClientState_ProcessFixAngle, "engine.dll", "40 53 48 83 EC ? F3 0F 10 42", 0x0);

MAKE_HOOK(CClientState_ProcessFixAngle, S::CClientState_ProcessFixAngle(), bool,
	CClientState* rcx, SVC_FixAngle* msg)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CClientState_ProcessFixAngle[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, msg);
#endif

	if (Vars::Visuals::Removals::AngleForcing.Value)
		return false;

	if (F::Spectate.m_iTarget != -1)
		F::Spectate.m_vOldView = msg->m_Angle;
	return CALL_ORIGINAL(rcx, msg);
}