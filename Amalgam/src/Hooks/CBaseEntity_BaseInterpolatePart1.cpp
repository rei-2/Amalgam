#include "../SDK/SDK.h"

#include "../Features/Ticks/Ticks.h"

MAKE_SIGNATURE(CBaseEntity_BaseInterpolatePart1, "client.dll", "48 89 5C 24 ? 56 57 41 55 41 56 41 57 48 83 EC ? 4C 8B BC 24", 0x0);

MAKE_HOOK(CBaseEntity_BaseInterpolatePart1, S::CBaseEntity_BaseInterpolatePart1(), int,
	void* rcx, float& currentTime, Vector& oldOrigin, QAngle& oldAngles, Vector& oldVel, int& bNoMoreChanges)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseEntity_BaseInterpolatePart1[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, currentTime, oldOrigin, oldAngles, oldVel, bNoMoreChanges);
#endif

	auto pEntity = reinterpret_cast<CBaseEntity*>(rcx);
	if (pEntity && pEntity->GetClassID() == ETFClassID::CTFViewModel && F::Ticks.m_bRecharge)
	{
		bNoMoreChanges = 1;
		return 0;
	}

	return CALL_ORIGINAL(rcx, currentTime, oldOrigin, oldAngles, oldVel, bNoMoreChanges);
}