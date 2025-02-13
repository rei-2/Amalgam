#include "../SDK/SDK.h"

#include "../Features/Resolver/Resolver.h"

MAKE_HOOK(CBaseEntity_EstimateAbsVelocity, S::CBaseEntity_EstimateAbsVelocity(), void,
	void* rcx, Vector& vel)
{
	CALL_ORIGINAL(rcx, vel);

	auto pPlayer = reinterpret_cast<CTFPlayer*>(rcx);
	if (pPlayer->IsPlayer() && pPlayer->entindex() != I::EngineClient->GetLocalPlayer())
	{
		// full override makes movement look a bit too choppy
		vel.z = pPlayer->m_vecVelocity().z;

		if (pPlayer->IsOnGround() && vel.Length2DSqr() < 2.f)
		{
			bool bMinwalk;
			if (F::Resolver.GetAngles(pPlayer, nullptr, nullptr, &bMinwalk) && bMinwalk)
				vel = { 1.f, 1.f, 0.f };
		}
	}
}