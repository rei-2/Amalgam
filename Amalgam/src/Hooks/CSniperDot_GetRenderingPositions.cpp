#include "../SDK/SDK.h"

MAKE_SIGNATURE(CSniperDot_GetRenderingPositions, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 55 57 41 57 48 8D 6C 24", 0x0);
MAKE_SIGNATURE(CBasePlayer_EyePosition, "client.dll", "48 89 5C 24 ? 57 48 83 EC ? 44 8B 81 ? ? ? ? 48 8B FA", 0x0);
MAKE_SIGNATURE(CSniperDot_GetRenderingPositions_EyePosition_Call, "client.dll", "8B 08 89 0F 8B 48 ? 89 4F ? 49 8B CF", 0x0);
MAKE_SIGNATURE(CTFPlayer_EyeAngles, "client.dll", "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 84 C0 74 ? 83 3D", 0x0);
MAKE_SIGNATURE(CSniperDot_GetRenderingPositions_EyeAngles_Call, "client.dll", "48 8D 54 24 ? 48 8D 4C 24 ? F2 0F 10 00 F2 0F 11 44 24 ? 8B 40 ? 89 44 24 ? E8 ? ? ? ? 49 8B 07", 0x0);

static Vec3 vEyePosition = {};
static Vec3 vEyeAngles = {};

MAKE_HOOK(CSniperDot_GetRenderingPositions, S::CSniperDot_GetRenderingPositions(), bool,
	void* rcx, CTFPlayer* pPlayer, Vec3& vecAttachment, Vec3& vecEndPos, float& flSize)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CSniperDot_GetRenderingPositions[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, pPlayer, vecAttachment, vecEndPos, flSize);
#endif

	if (pPlayer && pPlayer->entindex() != I::EngineClient->GetLocalPlayer())
	{
		auto pDot = reinterpret_cast<CSniperDot*>(rcx);

		vEyePosition = pPlayer->m_vecOrigin() + pPlayer->GetViewOffset();
		Math::VectorAngles(pDot->GetAbsOrigin() - vEyePosition, vEyeAngles);
	}

	return CALL_ORIGINAL(rcx, pPlayer, vecAttachment, vecEndPos, flSize);
}

MAKE_HOOK(CBasePlayer_EyePosition, S::CBasePlayer_EyePosition(), Vec3*,
	void* rcx, void* rdx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CSniperDot_GetRenderingPositions[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, rdx);
#endif

	static const auto dwDesired = S::CSniperDot_GetRenderingPositions_EyePosition_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	if (dwRetAddr == dwDesired)
		return &vEyePosition;

	return CALL_ORIGINAL(rcx, rdx);
}

MAKE_HOOK(CTFPlayer_EyeAngles, S::CTFPlayer_EyeAngles(), Vec3*,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CSniperDot_GetRenderingPositions[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	static const auto dwDesired = S::CSniperDot_GetRenderingPositions_EyeAngles_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	if (dwRetAddr == dwDesired)
		return &vEyeAngles;

	return CALL_ORIGINAL(rcx);
}