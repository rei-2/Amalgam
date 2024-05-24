#include "../SDK/SDK.h"

std::unordered_map<void*, std::pair<int, float>> pAnimatingInfo;

MAKE_HOOK(C_BaseAnimating_FrameAdvance, S::CBaseAnimating_FrameAdvance(), float, __fastcall,
	void* ecx, float flInterval)
{
	if (!Vars::Visuals::Removals::Interpolation.Value || ecx == H::Entities.GetLocal())
		return CALL_ORIGINAL(ecx, flInterval);

	const auto pEntity = static_cast<CBaseEntity*>(ecx);
	if (pEntity && pEntity->IsPlayer())
	{
		if (pEntity->m_flSimulationTime() == pEntity->m_flOldSimulationTime() || I::GlobalVars->tickcount == pAnimatingInfo[ecx].first)
		{
			pAnimatingInfo[ecx].second += flInterval;
			return 0.f;
		}
	}

	flInterval = pAnimatingInfo[ecx].second; pAnimatingInfo[ecx].second = 0.f; pAnimatingInfo[ecx].first = I::GlobalVars->tickcount;
	return CALL_ORIGINAL(ecx, flInterval);
}