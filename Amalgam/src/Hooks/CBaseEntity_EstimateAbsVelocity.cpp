#include "../SDK/SDK.h"

#include "../Features/Resolver/Resolver.h"

MAKE_HOOK(CBaseEntity_EstimateAbsVelocity, S::CBaseEntity_EstimateAbsVelocity(), void,
	void* rcx, Vector& vel)
{
	auto pPlayer = reinterpret_cast<CTFPlayer*>(rcx);
	if (pPlayer->IsPlayer() && pPlayer->entindex() != I::EngineClient->GetLocalPlayer())
	{
		vel = pPlayer->m_vecVelocity();

		if (pPlayer->IsOnGround() && vel.Length2DSqr() < 2.f)
		{
			bool bMinwalk;
			if (F::Resolver.GetAngles(pPlayer, nullptr, nullptr, &bMinwalk) && bMinwalk)
				vel = { 1.f, 1.f, 0.f };
		}

		return;
	}

	CALL_ORIGINAL(rcx, vel);
}