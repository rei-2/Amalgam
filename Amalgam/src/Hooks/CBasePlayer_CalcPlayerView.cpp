#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBasePlayer_CalcPlayerView, "client.dll", "48 89 5C 24 ? 56 57 41 54 48 83 EC ? 48 8B D9", 0x0);

MAKE_HOOK(CBasePlayer_CalcPlayerView, S::CBasePlayer_CalcPlayerView(), void,
	void* rcx, Vector& eyeOrigin, QAngle& eyeAngles, float& fov)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBasePlayer_CalcPlayerView[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, eyeOrigin, eyeAngles, fov);
#endif

	if (Vars::Visuals::Removals::ViewPunch.Value)
	{
		auto pPlayer = reinterpret_cast<CBasePlayer*>(rcx);

		Vec3 vOldPunch = pPlayer->m_vecPunchAngle();
		pPlayer->m_vecPunchAngle() = {};
		CALL_ORIGINAL(rcx, eyeOrigin, eyeAngles, fov);
		pPlayer->m_vecPunchAngle() = vOldPunch;
		return;
	}

	CALL_ORIGINAL(rcx, eyeOrigin, eyeAngles, fov);
}
