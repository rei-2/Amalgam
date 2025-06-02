#include "../SDK/SDK.h"

class AxisSet
{
public:
	float m_flOldAxisValue = 0.f;
	float m_flOldSimulationTime = 0.f;
	float m_flNewAxisValue = 0.f;
	float m_flNewSimulationTime = 0.f;

public:
	float Get(bool bZ = false) const
	{
		int iDeltaTicks = TIME_TO_TICKS(m_flNewSimulationTime - m_flOldSimulationTime);
		float flGravityCorrection = 0.f;
		if (bZ)
		{
			static auto sv_gravity = U::ConVars.FindVar("sv_gravity");
			float flDeltaTicks = float(iDeltaTicks) + 0.5f; // ?
			flGravityCorrection = (powf(flDeltaTicks, 2.f) - flDeltaTicks) / 2.f * sv_gravity->GetFloat() * powf(TICK_INTERVAL, 2);
		}
		float flDeltaValue = m_flNewAxisValue - m_flOldAxisValue;
		float flTickVelocity = flDeltaValue + (flDeltaValue ? 0.0625f * sign(m_flNewAxisValue) : 0.f) - flGravityCorrection;
		return flTickVelocity / TICKS_TO_TIME(iDeltaTicks);
	}
};

class AxisInfo
{
public:
	AxisSet x = {}, y = {}, z = {};

public:
	AxisSet& operator[](int i)
	{
		return ((AxisSet*)this)[i];
	}

	AxisSet operator[](int i) const
	{
		return ((AxisSet*)this)[i];
	}

	Vec3 Get(bool bGrounded = false) const
	{
		return { x.Get(), y.Get(), z.Get(!bGrounded) };
	}

	float Get(int i) const
	{
		return ((AxisSet*)this)[i].Get(i == 2);
	}
};

MAKE_SIGNATURE(CBasePlayer_PostDataUpdate_SetAbsVelocity_Call, "client.dll", "0F 28 74 24 ? 8B D6", 0x0);

MAKE_HOOK(CBaseEntity_SetAbsVelocity, S::CBaseEntity_SetAbsVelocity(), void,
	void* rcx, const Vec3& vecAbsVelocity)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CBaseEntity_SetAbsVelocity[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, vecAbsVelocity);
#endif

	static const auto dwDesired = S::CBasePlayer_PostDataUpdate_SetAbsVelocity_Call();
	const auto dwRetAddr = uintptr_t(_ReturnAddress());
	if (dwRetAddr != dwDesired)
		return CALL_ORIGINAL(rcx, vecAbsVelocity);
	
	const auto pPlayer = reinterpret_cast<CTFPlayer*>(rcx);
	if (pPlayer->IsDormant())
		return CALL_ORIGINAL(rcx, vecAbsVelocity);

	auto pRecords = H::Entities.GetOrigins(pPlayer->entindex());
	if (!pRecords || pRecords->empty())
		return CALL_ORIGINAL(rcx, vecAbsVelocity);

	auto& tOldRecord = pRecords->front();
	auto tNewRecord = VelFixRecord(pPlayer->m_vecOrigin() + Vec3(0, 0, pPlayer->GetSize().z), pPlayer->m_flSimulationTime());

	int iDeltaTicks = TIME_TO_TICKS(tNewRecord.m_flSimulationTime - tOldRecord.m_flSimulationTime);
	float flDeltaTime = TICKS_TO_TIME(iDeltaTicks);
	if (iDeltaTicks <= 0)
		return;

	static auto sv_lagcompensation_teleport_dist = U::ConVars.FindVar("sv_lagcompensation_teleport_dist");
	float flDist = powf(sv_lagcompensation_teleport_dist->GetFloat(), 2.f) * iDeltaTicks;
	if ((tNewRecord.m_vecOrigin - tOldRecord.m_vecOrigin).Length2DSqr() >= flDist)
		return pRecords->clear();

	bool bGrounded = pPlayer->IsOnGround();

	AxisInfo tAxisInfo = {};
	for (int i = 0; i < 3; i++)
	{
		tAxisInfo[i].m_flOldAxisValue = tOldRecord.m_vecOrigin[i];
		tAxisInfo[i].m_flNewAxisValue = tNewRecord.m_vecOrigin[i];
		tAxisInfo[i].m_flOldSimulationTime = tOldRecord.m_flSimulationTime;
		tAxisInfo[i].m_flNewSimulationTime = tNewRecord.m_flSimulationTime;

		if (i == 2 && bGrounded)
			break;

		float flOldPos1 = tOldRecord.m_vecOrigin[i], flOldPos2 = flOldPos1 + 0.125f * sign(flOldPos1);
		float flNewPos1 = tNewRecord.m_vecOrigin[i], flNewPos2 = flNewPos1 + 0.125f * sign(flNewPos1);
		if (!flOldPos1) flOldPos1 = -0.125f, flOldPos2 = 0.125f;
		if (!flNewPos1) flNewPos1 = -0.125f, flNewPos2 = 0.125f;

		FloatRange_t flVelocityRange;
		{
			std::deque<float> vDeltas = { flNewPos1 - flOldPos1, flNewPos2 - flOldPos1, flNewPos1 - flOldPos2, flNewPos2 - flOldPos2 };
			std::sort(vDeltas.begin(), vDeltas.end(), std::less<float>());
			flVelocityRange = { vDeltas.front() / flDeltaTime, vDeltas.back() / flDeltaTime };
		}

		for (auto& tRecord : *pRecords)
		{
			if (tAxisInfo[i].m_flOldSimulationTime <= tRecord.m_flSimulationTime)
				continue;

			float flRewind = -ROUND_TO_TICKS(tNewRecord.m_flSimulationTime - tRecord.m_flSimulationTime);
			FloatRange_t flPositionRange = { tAxisInfo[i].m_flNewAxisValue + flVelocityRange.Max * flRewind, tAxisInfo[i].m_flNewAxisValue + flVelocityRange.Min * flRewind };
			if (i == 2)
			{
				static auto sv_gravity = U::ConVars.FindVar("sv_gravity");
				float flGravityCorrection = sv_gravity->GetFloat() * powf(flRewind + TICK_INTERVAL / 2, 2.f) / 2;
				flPositionRange.Min -= flGravityCorrection, flPositionRange.Max -= flGravityCorrection;
			}
			if (flPositionRange.Min > tRecord.m_vecOrigin[i] || tRecord.m_vecOrigin[i] > flPositionRange.Max)
				break;

			tAxisInfo[i].m_flOldAxisValue = tRecord.m_vecOrigin[i];
			tAxisInfo[i].m_flOldSimulationTime = tRecord.m_flSimulationTime;
		}
	}

	H::Entities.SetAvgVelocity(pPlayer->entindex(), tAxisInfo.Get(bGrounded));
	CALL_ORIGINAL(rcx, (tNewRecord.m_vecOrigin - tOldRecord.m_vecOrigin) / flDeltaTime);
}