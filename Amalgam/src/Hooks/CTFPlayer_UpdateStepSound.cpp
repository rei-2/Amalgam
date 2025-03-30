#include "../SDK/SDK.h"

MAKE_SIGNATURE(CTFPlayer_UpdateStepSound, "client.dll", "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 4C 8B F2 48 8B F9 BA", 0x0);
MAKE_SIGNATURE(CTFPlayer_FireEvent_UpdateStepSound_Call, "client.dll", "4D 85 F6 0F 84 ? ? ? ? 48 8D 8E", 0x0);

MAKE_HOOK(CTFPlayer_UpdateStepSound, S::CTFPlayer_UpdateStepSound(), void,
	void* rcx, void* psurface, const Vec3& vecOrigin, const Vec3& vecVelocity)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CTFPlayer_UpdateStepSound[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, psurface, vecOrigin, vecVelocity);
#endif

	static const auto dwDesired = S::CTFPlayer_FireEvent_UpdateStepSound_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());

	if (dwRetAddr == dwDesired && rcx == H::Entities.GetLocal())
		return;

	CALL_ORIGINAL(rcx, psurface, vecOrigin, vecVelocity);
}