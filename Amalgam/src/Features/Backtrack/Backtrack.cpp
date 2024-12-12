#include "Backtrack.h"

#include "../PacketManip/FakeLag/FakeLag.h"
#include "../TickHandler/TickHandler.h"

void CBacktrack::Restart()
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

	if (iFlow != -1)
		return pNetChan->GetLatency(iFlow) - (bNoFake && iFlow == FLOW_INCOMING ? m_flFakeLatency : 0.f);
	return pNetChan->GetLatency(FLOW_INCOMING) + pNetChan->GetLatency(FLOW_OUTGOING) - (bNoFake ? m_flFakeLatency : 0.f);
}

void CBacktrack::SendLerp()
{
	auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan || !I::EngineClient->IsConnected())
		return;

	static Timer interpTimer{};
	if (interpTimer.Run(100))
	{
		float flTarget = GetLerp();
		if (flTarget == m_flWishInterp)
			return;

		NET_SetConVar cl_interp("cl_interp", std::to_string(m_flWishInterp = flTarget).c_str());
		pNetChan->SendNetMsg(cl_interp);
		NET_SetConVar cl_interp_ratio("cl_interp_ratio", "1.0");
		pNetChan->SendNetMsg(cl_interp_ratio);
		NET_SetConVar cl_interpolate("cl_interpolate", "1");
		pNetChan->SendNetMsg(cl_interpolate);
	}
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
		m_dSequences.push_front(CIncomingSequence(pNetChan->m_nInReliableState, pNetChan->m_nInSequenceNr, I::GlobalVars->realtime));
	}

	if (m_dSequences.size() > 67)
		m_dSequences.pop_back();
}



bool CBacktrack::WithinRewind(const TickRecord& record)
{
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return false;

	const float flCorrect = std::clamp(pNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(m_flFakeInterp) + ROUND_TO_TICKS(m_flFakeLatency), 0.f, m_flMaxUnlag) - pNetChan->GetLatency(FLOW_OUTGOING);
	const int iServerTick = m_iTickCount + G::AnticipatedChoke + Vars::Backtrack::Offset.Value;

	const float flDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(record.m_flSimTime));

	return fabsf(flDelta) < float(Vars::Backtrack::Window.Value) / 1000;
}

std::deque<TickRecord>* CBacktrack::GetRecords(CBaseEntity* pEntity)
{
	if (m_mRecords[pEntity].empty())
		return nullptr;

	return &m_mRecords[pEntity];
}

std::deque<TickRecord> CBacktrack::GetValidRecords(std::deque<TickRecord>* pRecords, CTFPlayer* pLocal, bool bDistance)
{
	std::deque<TickRecord> validRecords = {};
	if (!pRecords)
		return validRecords;

	for (auto& pTick : *pRecords)
	{
		if (!WithinRewind(pTick))
			continue;

		validRecords.push_back(pTick);
	}

	if (pLocal)
	{
		if (bDistance)
			std::sort(validRecords.begin(), validRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
				{
					if (Vars::Backtrack::PreferOnShot.Value && a.m_bOnShot != b.m_bOnShot)
						return a.m_bOnShot > b.m_bOnShot;

					return pLocal->m_vecOrigin().DistTo(a.m_vOrigin) < pLocal->m_vecOrigin().DistTo(b.m_vOrigin);
				});
		else
		{
			auto pNetChan = I::EngineClient->GetNetChannelInfo();
			if (!pNetChan)
				return validRecords;

			const float flCorrect = std::clamp(pNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(m_flFakeInterp) + ROUND_TO_TICKS(m_flFakeLatency), 0.f, m_flMaxUnlag) - pNetChan->GetLatency(FLOW_OUTGOING);
			const int iServerTick = m_iTickCount + G::AnticipatedChoke + Vars::Backtrack::Offset.Value;

			std::sort(validRecords.begin(), validRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
				{
					if (Vars::Backtrack::PreferOnShot.Value && a.m_bOnShot != b.m_bOnShot)
						return a.m_bOnShot > b.m_bOnShot;

					const float flADelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(a.m_flSimTime));
					const float flBDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(b.m_flSimTime));
					return fabsf(flADelta) < fabsf(flBDelta);
				});
		}
	}

	return validRecords;
}



void CBacktrack::MakeRecords()
{
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer->entindex() == I::EngineClient->GetLocalPlayer() || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost()
			|| !H::Entities.GetBones(pPlayer->entindex()) || !H::Entities.GetDeltaTime(pPlayer->entindex()))
			continue;

		const TickRecord curRecord = {
			pPlayer->m_flSimulationTime(),
			*reinterpret_cast<BoneMatrix*>(H::Entities.GetBones(pPlayer->entindex())),
			pPlayer->m_vecOrigin(),
			m_mDidShoot[pPlayer->entindex()]
		};

		bool bLagComp = false;
		if (!m_mRecords[pPlayer].empty())
		{
			const Vec3 vDelta = curRecord.m_vOrigin - m_mRecords[pPlayer].front().m_vOrigin;
			
			static auto sv_lagcompensation_teleport_dist = U::ConVars.FindVar("sv_lagcompensation_teleport_dist");
			const float flDist = powf(sv_lagcompensation_teleport_dist ? sv_lagcompensation_teleport_dist->GetFloat() : 64.f, 2.f);
			if (vDelta.Length2DSqr() > flDist)
			{
				bLagComp = true;
				for (auto& pRecord : m_mRecords[pPlayer])
					pRecord.m_bInvalid = true;
			}

			for (auto& pRecord : m_mRecords[pPlayer])
			{
				if (!pRecord.m_bInvalid)
					continue;

				pRecord.m_BoneMatrix = curRecord.m_BoneMatrix;
				pRecord.m_vOrigin = curRecord.m_vOrigin;
				pRecord.m_bOnShot = curRecord.m_bOnShot;
			}
		}

		m_mRecords[pPlayer].push_front(curRecord);
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

		if (pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
		{
			m_mRecords[pPlayer].clear();
			continue;
		}

		//const int iOldSize = pRecords.size();

		const int flDeadtime = I::GlobalVars->curtime + GetReal() - m_flMaxUnlag; // int ???
		while (!m_mRecords[pPlayer].empty())
		{
			if (m_mRecords[pPlayer].back().m_flSimTime >= flDeadtime)
				break;

			m_mRecords[pPlayer].pop_back();
		}

		//const int iNewSize = pRecords.size();
		//if (iOldSize != iNewSize)
		//	SDK::Output("Clear", std::format("{} -> {}", iOldSize, iNewSize).c_str(), { 255, 0, 200, 255 }, Vars::Debug::Logging.Value);
	}
}



void CBacktrack::FrameStageNotify()
{
	UpdateDatagram();
	if (!I::EngineClient->IsInGame())
		return Restart();

	static auto sv_maxunlag = U::ConVars.FindVar("sv_maxunlag");
	m_flMaxUnlag = sv_maxunlag ? sv_maxunlag->GetFloat() : 1.f;
	
	MakeRecords();
	CleanRecords();
}

void CBacktrack::Run(CUserCmd* pCmd)
{
	SendLerp();

	// might not even be necessary
	G::AnticipatedChoke = 0;
	if (F::Ticks.m_iShiftedTicks != F::Ticks.m_iMaxShift && G::PrimaryWeaponType != EWeaponType::HITSCAN && Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Silent)
		G::AnticipatedChoke = 1;
	if (F::FakeLag.m_iGoal && !Vars::CL_Move::Fakelag::UnchokeOnAttack.Value && F::Ticks.m_iShiftedTicks == F::Ticks.m_iShiftedGoal && !F::Ticks.m_bDoubletap && !F::Ticks.m_bSpeedhack)
		G::AnticipatedChoke = F::FakeLag.m_iGoal - I::ClientState->chokedcommands; // iffy, unsure if there is a good way to get it to work well without unchoking
}

void CBacktrack::ResolverUpdate(CBaseEntity* pEntity)
{
	m_mRecords[pEntity].clear();	//	TODO: eventually remake records and rotate them or smthn idk, maybe just rotate them
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