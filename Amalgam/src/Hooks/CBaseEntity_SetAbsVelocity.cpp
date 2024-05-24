#include "../SDK/SDK.h"

MAKE_SIGNATURE(CBasePlayer_PostDataUpdate_Call, "client.dll", "E8 ? ? ? ? 0F 28 74 24 ? 8B D6", 0x5);

MAKE_HOOK(CBaseEntity_SetAbsVelocity, S::CBaseEntity_SetAbsVelocity(), void, __fastcall,
	void* ecx, const Vec3& vecAbsVelocity)
{
	static const auto dwDesired = S::CBasePlayer_PostDataUpdate_Call();
	const auto dwRetAddr = std::uintptr_t(_ReturnAddress());

	if (dwRetAddr == dwDesired)
	{
		const auto pPlayer = static_cast<CTFPlayer*>(ecx);
		if (pPlayer && G::VelocityMap.contains(pPlayer->entindex()))
		{
			const auto newRecord = VelFixRecord(pPlayer->m_vecOrigin(), pPlayer->m_vecMaxs().z - pPlayer->m_vecMins().z, pPlayer->m_flSimulationTime());
			const auto& oldRecord = G::VelocityMap[pPlayer->entindex()];

			const float flDelta = newRecord.m_flSimulationTime - oldRecord.m_flSimulationTime;
			const Vec3 vDelta = newRecord.m_vecOrigin - oldRecord.m_vecOrigin;

			static auto sv_lagcompensation_teleport_dist = U::ConVars.FindVar("sv_lagcompensation_teleport_dist");
			const float flDist = powf(sv_lagcompensation_teleport_dist ? sv_lagcompensation_teleport_dist->GetFloat() : 64.f, 2.f) * TIME_TO_TICKS(flDelta);
			if (flDelta > 0.f && vDelta.Length2DSqr() < flDist)
			{
				Vec3 vOldOrigin = oldRecord.m_vecOrigin;
				if (!pPlayer->IsOnGround())
					vOldOrigin.z += oldRecord.m_flHeight - newRecord.m_flHeight;
				CALL_ORIGINAL(ecx, (newRecord.m_vecOrigin - vOldOrigin) / flDelta);
			}
			return;
		}
	}

	CALL_ORIGINAL(ecx, vecAbsVelocity);
}