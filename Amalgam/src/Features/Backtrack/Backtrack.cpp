#include "Backtrack.h"

#include "../PacketManip/FakeLag/FakeLag.h"
#include "../TickHandler/TickHandler.h"

void CBacktrack::Reset()
{
	m_mRecords.clear();
	m_dSequences.clear();
	m_iLastInSequence = 0;
}



// Returns the wish cl_interp
float CBacktrack::GetLerp()
{
	if (!Vars::Backtrack::Enabled.Value)
		return G::Lerp;

	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
		return std::clamp(Vars::Backtrack::Interp.Value / 1000.f, G::Lerp, 0.1f);

	return std::clamp(Vars::Backtrack::Interp.Value / 1000.f, G::Lerp, m_flMaxUnlag);
}

// Returns the wish backtrack latency
float CBacktrack::GetFake()
{
	if (!Vars::Backtrack::Enabled.Value)
		return 0.f;

	return std::clamp(Vars::Backtrack::Latency.Value / 1000.f, 0.f, m_flMaxUnlag);
}

// Returns the current real latency
float CBacktrack::GetReal(int iFlow, bool bNoFake)
{
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return 0.f;

	if (iFlow != MAX_FLOWS)
		return pNetChan->GetLatency(iFlow) - (bNoFake && iFlow == FLOW_INCOMING ? m_flFakeLatency : 0.f);
	return pNetChan->GetLatency(FLOW_INCOMING) + pNetChan->GetLatency(FLOW_OUTGOING) - (bNoFake ? m_flFakeLatency : 0.f);
}

// Returns the current fake interp
float CBacktrack::GetFakeInterp()
{
	if (Vars::Misc::Game::AntiCheatCompatibility.Value)
		return std::min(m_flFakeInterp, 0.1f);

	return m_flFakeInterp;
}

// Returns the current anticipated choke
int CBacktrack::GetAnticipatedChoke(int iMethod)
{
	int iAnticipatedChoke = 0;
	if (F::Ticks.CanChoke() && G::PrimaryWeaponType != EWeaponType::HITSCAN && Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Silent)
		iAnticipatedChoke = 1;
	if (F::FakeLag.m_iGoal && !Vars::CL_Move::Fakelag::UnchokeOnAttack.Value && F::Ticks.m_iShiftedTicks == F::Ticks.m_iShiftedGoal && !F::Ticks.m_bDoubletap && !F::Ticks.m_bSpeedhack)
		iAnticipatedChoke = F::FakeLag.m_iGoal - I::ClientState->chokedcommands; // iffy, unsure if there is a good way to get it to work well without unchoking
	return iAnticipatedChoke;
}

void CBacktrack::SendLerp()
{
	static Timer tTimer = {};
	if (!tTimer.Run(0.1f))
		return;

	auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan || !I::EngineClient->IsConnected())
		return;

	float flTarget = GetLerp();
	if (flTarget == m_flWishInterp)
		return;

	NET_SetConVar cl_interp("cl_interp", std::to_string(m_flWishInterp = flTarget).c_str());
	pNetChan->SendNetMsg(cl_interp);
	NET_SetConVar cl_interp_ratio("cl_interp_ratio", "1");
	pNetChan->SendNetMsg(cl_interp_ratio);
	NET_SetConVar cl_interpolate("cl_interpolate", "1");
	pNetChan->SendNetMsg(cl_interpolate);
}

// Manages cl_interp client value
void CBacktrack::SetLerp(IGameEvent* pEvent)
{
	if (I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")) == I::EngineClient->GetLocalPlayer())
		m_flFakeInterp = m_flWishInterp;
}

void CBacktrack::UpdateDatagram()
{
	auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan)
		return;

	if (auto pLocal = H::Entities.GetLocal())
		m_nOldTickBase = pLocal->m_nTickBase();

	if (pNetChan->m_nInSequenceNr > m_iLastInSequence)
	{
		m_iLastInSequence = pNetChan->m_nInSequenceNr;
		m_dSequences.emplace_front(pNetChan->m_nInReliableState, pNetChan->m_nInSequenceNr, I::GlobalVars->realtime);
	}

	if (m_dSequences.size() > 67)
		m_dSequences.pop_back();
}



std::deque<TickRecord>* CBacktrack::GetRecords(CBaseEntity* pEntity)
{
	if (m_mRecords[pEntity].empty())
		return nullptr;

	return &m_mRecords[pEntity];
}

std::deque<TickRecord> CBacktrack::GetValidRecords(std::deque<TickRecord>* pRecords, CTFPlayer* pLocal, bool bDistance)
{
	std::deque<TickRecord> vRecords = {};
	if (!pRecords || pRecords->empty())
		return vRecords;

	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return vRecords;

	float flCorrect = std::clamp(F::Backtrack.GetReal(MAX_FLOWS, false) + ROUND_TO_TICKS(GetFakeInterp()), 0.f, m_flMaxUnlag);
	int iServerTick = m_iTickCount + GetAnticipatedChoke() + Vars::Backtrack::Offset.Value + TIME_TO_TICKS(F::Backtrack.GetReal(FLOW_OUTGOING));

	for (auto& tRecord : *pRecords)
	{
		float flDelta = fabsf(flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(tRecord.m_flSimTime)));
		float flWindow = Vars::Misc::Game::AntiCheatCompatibility.Value ? 0 : Vars::Backtrack::Window.Value;
		if (flDelta > flWindow / 1000)
			continue;

		vRecords.push_back(tRecord);
	}

	if (vRecords.empty())
	{	// make sure there is at least 1 record
		float flMinDelta = 0.2f;
		for (auto& tRecord : *pRecords)
		{
			float flDelta = fabsf(flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(tRecord.m_flSimTime)));
			if (flDelta > flMinDelta)
				continue;

			flMinDelta = flDelta;
			vRecords = { tRecord };
		}
	}
	else if (pLocal && vRecords.size() > 1)
	{
		if (bDistance)
			std::sort(vRecords.begin(), vRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
				{
					if (Vars::Backtrack::PreferOnShot.Value && a.m_bOnShot != b.m_bOnShot)
						return a.m_bOnShot > b.m_bOnShot;

					return pLocal->m_vecOrigin().DistTo(a.m_vOrigin) < pLocal->m_vecOrigin().DistTo(b.m_vOrigin);
				});
		else
		{
			std::sort(vRecords.begin(), vRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
				{
					if (Vars::Backtrack::PreferOnShot.Value && a.m_bOnShot != b.m_bOnShot)
						return a.m_bOnShot > b.m_bOnShot;

					const float flADelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(a.m_flSimTime));
					const float flBDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(b.m_flSimTime));
					return fabsf(flADelta) < fabsf(flBDelta);
				});
		}
	}

	return vRecords;
}



void CBacktrack::MakeRecords()
{
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost()
			|| !H::Entities.GetBones(pPlayer->entindex()) || !H::Entities.GetDeltaTime(pPlayer->entindex()))
			continue;

		auto& vRecords = m_mRecords[pPlayer];

		const TickRecord* pLastRecord = !vRecords.empty() ? &vRecords.front() : nullptr;
		vRecords.emplace_front(
			pPlayer->m_flSimulationTime(),
			*reinterpret_cast<BoneMatrix*>(H::Entities.GetBones(pPlayer->entindex())),
			pPlayer->m_vecOrigin(),
			pPlayer->m_vecMins(),
			pPlayer->m_vecMaxs(),
			m_mDidShoot[pPlayer->entindex()]
		);
		const TickRecord& tCurRecord = vRecords.front();

		bool bLagComp = false;
		if (pLastRecord)
		{
			const Vec3 vDelta = tCurRecord.m_vOrigin - pLastRecord->m_vOrigin;
			
			static auto sv_lagcompensation_teleport_dist = U::ConVars.FindVar("sv_lagcompensation_teleport_dist");
			const float flDist = powf(sv_lagcompensation_teleport_dist ? sv_lagcompensation_teleport_dist->GetFloat() : 64.f, 2.f);
			if (vDelta.Length2DSqr() > flDist)
			{
				bLagComp = true;
				for (size_t i = 1; i < vRecords.size(); i++)
					vRecords[i].m_bInvalid = true;
			}

			for (auto& pRecord : vRecords)
			{
				if (!pRecord.m_bInvalid)
					continue;

				pRecord.m_BoneMatrix = tCurRecord.m_BoneMatrix;
				pRecord.m_vOrigin = tCurRecord.m_vOrigin;
				pRecord.m_bOnShot = tCurRecord.m_bOnShot;
			}
		}

		H::Entities.SetLagCompensation(pPlayer->entindex(), bLagComp);
		m_mDidShoot[pPlayer->entindex()] = false;
	}
}

void CBacktrack::CleanRecords()
{
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer())
			continue;

		auto& vRecords = m_mRecords[pPlayer];

		if (pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
		{
			vRecords.clear();
			continue;
		}

		//const int iOldSize = pRecords.size();

		const int flDeadtime = I::GlobalVars->curtime + GetReal() - m_flMaxUnlag; // int ???
		while (!vRecords.empty())
		{
			if (vRecords.back().m_flSimTime >= flDeadtime)
				break;

			vRecords.pop_back();
		}

		//const int iNewSize = pRecords.size();
		//if (iOldSize != iNewSize)
		//	SDK::Output("Clear", std::format("{} -> {}", iOldSize, iNewSize).c_str(), { 255, 0, 200 }, Vars::Debug::Logging.Value);
	}
}



void CBacktrack::Store()
{
	UpdateDatagram();
	if (!I::EngineClient->IsInGame())
		return;

	static auto sv_maxunlag = U::ConVars.FindVar("sv_maxunlag");
	m_flMaxUnlag = sv_maxunlag ? sv_maxunlag->GetFloat() : 1.f;
	
	MakeRecords();
	CleanRecords();
}

void CBacktrack::Run(CUserCmd* pCmd)
{
	SendLerp();
}

void CBacktrack::ResolverUpdate(CBaseEntity* pEntity)
{
	/*
	if (!pEntity)
		return;

	m_mRecords[pEntity].clear();
	*/
}

void CBacktrack::ReportShot(int iIndex)
{
	if (!Vars::Backtrack::PreferOnShot.Value)
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
	if (!pEntity || SDK::GetWeaponType(pEntity->As<CTFPlayer>()->m_hActiveWeapon().Get()->As<CTFWeaponBase>()) != EWeaponType::HITSCAN)
		return;

	m_mDidShoot[pEntity->entindex()] = true;
}

void CBacktrack::AdjustPing(CNetChannel* pNetChan)
{
	m_nOldInSequenceNr = pNetChan->m_nInSequenceNr, m_nOldInReliableState = pNetChan->m_nInReliableState;

	auto Set = [&]()
		{
			if (!Vars::Backtrack::Enabled.Value || !Vars::Backtrack::Latency.Value)
				return 0.f;

			auto pLocal = H::Entities.GetLocal();
			if (!pLocal || !pLocal->m_iClass())
				return 0.f;

			static auto host_timescale = U::ConVars.FindVar("host_timescale");
			float flTimescale = host_timescale ? host_timescale->GetFloat() : 1.f;

			static float flStaticReal = 0.f;
			float flFake = GetFake(), flReal = TICKS_TO_TIME(pLocal->m_nTickBase() - m_nOldTickBase);
			flStaticReal += (flReal + 5 * TICK_INTERVAL - flStaticReal) * 0.1f;

			int nInReliableState = pNetChan->m_nInReliableState, nInSequenceNr = pNetChan->m_nInSequenceNr; float flLatency = 0.f;
			for (auto& cSequence : m_dSequences)
			{
				nInReliableState = cSequence.m_nInReliableState;
				nInSequenceNr = cSequence.m_nSequenceNr;
				flLatency = (I::GlobalVars->realtime - cSequence.m_flTime) * flTimescale - TICK_INTERVAL;

				if (flLatency > flFake || m_nLastInSequenceNr >= cSequence.m_nSequenceNr || flLatency > m_flMaxUnlag - flStaticReal)
					break;
			}

			pNetChan->m_nInReliableState = nInReliableState;
			pNetChan->m_nInSequenceNr = nInSequenceNr;
			return flLatency;
		};

	auto flLatency = Set();
	m_nLastInSequenceNr = pNetChan->m_nInSequenceNr;

	if (Vars::Backtrack::Enabled.Value && Vars::Backtrack::Latency.Value || m_flFakeLatency)
	{
		m_flFakeLatency = std::clamp(m_flFakeLatency + (flLatency - m_flFakeLatency) * 0.1f, m_flFakeLatency - TICK_INTERVAL, m_flFakeLatency + TICK_INTERVAL);
		if (!flLatency && m_flFakeLatency < TICK_INTERVAL)
			m_flFakeLatency = 0.f;
	}
}

void CBacktrack::RestorePing(CNetChannel* pNetChan)
{
	pNetChan->m_nInSequenceNr = m_nOldInSequenceNr, pNetChan->m_nInReliableState = m_nOldInReliableState;
}