#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFInput_CAM_CapYaw, "client.dll", "40 53 48 83 EC ? 0F 29 74 24 ? 0F 28 F1", 0x0);
MAKE_SIGNATURE(CTFInput_ApplyMouse, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F 29 74 24 ? 49 8B E8", 0x0);
MAKE_SIGNATURE(CTFPlayerShared_CalculateChargeCap, "client.dll", "48 83 EC ? 4C 8B 81 ? ? ? ? 48 8D 15", 0x0);

MAKE_HOOK(CTFInput_ApplyMouse, S::CTFInput_ApplyMouse(), void,
	void* rcx, QAngle& viewangles, CUserCmd* cmd, float mouse_x, float mouse_y)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFInput_ApplyMouse[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, viewangles, cmd, mouse_x, mouse_y);
#endif

	// we should maybe predict the shield cond for better accuracy

	CALL_ORIGINAL(rcx, viewangles, cmd, mouse_x, mouse_y);

	if (!Vars::Misc::Movement::ShieldTurnRate.Value)
		return;

	auto pLocal = H::Entities.GetLocal();
	auto pCmd = G::CurrentUserCmd;
	if (!pLocal || !pCmd || !pLocal->InCond(TF_COND_SHIELD_CHARGE))
		return;

	float flOriginalFrame = I::GlobalVars->frametime;
	I::GlobalVars->frametime = TICK_INTERVAL;
	float flCap = S::CTFPlayerShared_CalculateChargeCap.Call<float>(pLocal->m_Shared()) * 2.5f;
	I::GlobalVars->frametime = flOriginalFrame;

	float flOldYaw = pCmd->viewangles.y;
	float& flNewYaw = viewangles.y;
	float flDiff = abs(flOldYaw) - abs(flNewYaw);
	if (flDiff > flCap)
	{
		if (flNewYaw > flOldYaw)
			flNewYaw = flOldYaw + flCap;
		else
			flNewYaw = flOldYaw - flCap;
	}
}

MAKE_HOOK(CTFInput_CAM_CapYaw, S::CTFInput_CAM_CapYaw(), float,
	void* rcx, float fVal)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFInput_ApplyMouse[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, fVal);
#endif

	if (!Vars::Misc::Movement::ShieldTurnRate.Value)
		return CALL_ORIGINAL(rcx, fVal);

	return fVal;
}