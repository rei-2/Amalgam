#include "../SDK/SDK.h"

#include "../Features/Resolver/Resolver.h"
#include "../Features/EnginePrediction/EnginePrediction.h"

MAKE_HOOK(CBaseEntity_EstimateAbsVelocity, S::CBaseEntity_EstimateAbsVelocity(), void,
	void* rcx, Vector& vel)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseEntity_EstimateAbsVelocity[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, vel);
#endif

	auto pPlayer = reinterpret_cast<CTFPlayer*>(rcx);
	if (!pPlayer->IsPlayer())
		return CALL_ORIGINAL(rcx, vel);

	if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer())
	{
		vel = pPlayer->m_vecVelocity();
		return;
	}

	if (!Vars::Visuals::Removals::Interpolation.Value)
	{
		CALL_ORIGINAL(rcx, vel);
		vel.z = pPlayer->m_vecVelocity().z;
	}
	else
		vel = pPlayer->m_vecVelocity();

	if (pPlayer->IsOnGround() && vel.Length2DSqr() < 2.f)
	{
		bool bMinwalk;
		if (F::Resolver.GetAngles(pPlayer, nullptr, nullptr, &bMinwalk) && bMinwalk)
			vel = { 1, 1 };
	}
}