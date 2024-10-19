#include "Backtrack.h"

//#include "../Simulation/MovementSimulation/MovementSimulation.h"

void CBacktrack::Restart()
{
	mRecords.clear();
	dSequences.clear();
	iLastInSequence = 0;
}



// Returns the wish cl_interp
float CBacktrack::GetLerp()
{
	if (!Vars::Backtrack::Enabled.Value)
		return G::Lerp;

	return std::clamp(Vars::Backtrack::Interp.Value / 1000.f, G::Lerp, flMaxUnlag);
}

// Returns the wish backtrack latency
float CBacktrack::GetFake()
{
	if (!Vars::Backtrack::Enabled.Value)
		return 0.f;

	return std::clamp(Vars::Backtrack::Latency.Value / 1000.f, 0.f, flMaxUnlag);
}

// Returns the current real latency
float CBacktrack::GetReal(int iFlow)
{
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return 0.f;

	if (iFlow != -1)
		return pNetChan->GetLatency(iFlow) - (iFlow == FLOW_INCOMING ? flFakeLatency : 0.f);
	return pNetChan->GetLatency(FLOW_INCOMING) - flFakeLatency + pNetChan->GetLatency(FLOW_OUTGOING);
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
		if (flTarget == flWishInterp)
			return;

		NET_SetConVar cl_interp("cl_interp", std::to_string(flWishInterp = flTarget).c_str());
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
		flFakeInterp = flWishInterp;
}

// Store the last 2048 sequences
void CBacktrack::UpdateDatagram()
{
	auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan)
		return;

	if (pNetChan->m_nInSequenceNr > iLastInSequence)
	{
		iLastInSequence = pNetChan->m_nInSequenceNr;
		dSequences.push_front(CIncomingSequence(pNetChan->m_nInReliableState, pNetChan->m_nInSequenceNr, I::GlobalVars->realtime));
	}

	if (dSequences.size() > 2048)
		dSequences.pop_back();
}



bool CBacktrack::WithinRewind(const TickRecord& record)
{
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return false;

	const float flCorrect = std::clamp(pNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(flFakeInterp) + ROUND_TO_TICKS(flFakeLatency), 0.f, flMaxUnlag) - pNetChan->GetLatency(FLOW_OUTGOING);
	const int iServerTick = iTickCount + G::AnticipatedChoke + Vars::Backtrack::Offset.Value;

	const float flDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(record.flSimTime));

	return fabsf(flDelta) < float(Vars::Backtrack::Window.Value) / 1000;
}

std::deque<TickRecord>* CBacktrack::GetRecords(CBaseEntity* pEntity)
{
	if (mRecords[pEntity].empty())
		return nullptr;

	return &mRecords[pEntity];
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
					if (Vars::Backtrack::PreferOnShot.Value && a.bOnShot != b.bOnShot)
						return a.bOnShot > b.bOnShot;

					return pLocal->m_vecOrigin().DistTo(a.vOrigin) < pLocal->m_vecOrigin().DistTo(b.vOrigin);
				});
		else
		{
			auto pNetChan = I::EngineClient->GetNetChannelInfo();
			if (!pNetChan)
				return validRecords;

			const float flCorrect = std::clamp(pNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(flFakeInterp) + ROUND_TO_TICKS(flFakeLatency), 0.f, flMaxUnlag) - pNetChan->GetLatency(FLOW_OUTGOING);
			const int iServerTick = iTickCount + G::AnticipatedChoke + Vars::Backtrack::Offset.Value;

			std::sort(validRecords.begin(), validRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
				{
					if (Vars::Backtrack::PreferOnShot.Value && a.bOnShot != b.bOnShot)
						return a.bOnShot > b.bOnShot;

					const float flADelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(a.flSimTime));
					const float flBDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(b.flSimTime));
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
		if (pEntity->entindex() == I::EngineClient->GetLocalPlayer() || pEntity->IsDormant() || !H::Entities.GetBones(pEntity) || !H::Entities.GetDeltaTime(pEntity))
			continue;

		const TickRecord curRecord = {
			pEntity->m_flSimulationTime(),
			*reinterpret_cast<BoneMatrix*>(H::Entities.GetBones(pEntity)),
			pEntity->m_vecOrigin(),
			mDidShoot[pEntity->entindex()]
		};

		bool bLagComp = false;
		if (!mRecords[pEntity].empty()) // check for lagcomp breaking here
		{
			const Vec3 vDelta = curRecord.vOrigin - mRecords[pEntity].front().vOrigin;
			
			static auto sv_lagcompensation_teleport_dist = U::ConVars.FindVar("sv_lagcompensation_teleport_dist");
			const float flDist = powf(sv_lagcompensation_teleport_dist ? sv_lagcompensation_teleport_dist->GetFloat() : 64.f, 2.f);
			if (vDelta.Length2DSqr() > flDist)
			{
				bLagComp = true;
				for (auto& pRecord : mRecords[pEntity])
					pRecord.bInvalid = true;
			}

			for (auto& pRecord : mRecords[pEntity])
			{
				if (!pRecord.bInvalid)
					continue;

				pRecord.BoneMatrix = curRecord.BoneMatrix;
				pRecord.vOrigin = curRecord.vOrigin;
				pRecord.bOnShot = curRecord.bOnShot;
			}
		}

		mRecords[pEntity].push_front(curRecord);
		H::Entities.SetLagCompensation(pEntity, bLagComp);

		mDidShoot[pEntity->entindex()] = false;
	}
}

void CBacktrack::CleanRecords()
{
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pEntity->entindex() == I::EngineClient->GetLocalPlayer())
			continue;

		if (!pEntity->IsPlayer() || pEntity->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
		{
			mRecords[pEntity].clear();
			continue;
		}

		//const int iOldSize = pRecords.size();

		const int flDeadtime = I::GlobalVars->curtime + GetReal() - flMaxUnlag; // int ???
		while (!mRecords[pEntity].empty())
		{
			if (mRecords[pEntity].back().flSimTime >= flDeadtime)
				break;

			mRecords[pEntity].pop_back();
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
	flMaxUnlag = sv_maxunlag ? sv_maxunlag->GetFloat() : 1.f;
	
	MakeRecords();
	CleanRecords();
}

void CBacktrack::Run(CUserCmd* pCmd)
{
	SendLerp();

	// might not even be necessary
	G::AnticipatedChoke = 0;
	if (G::ShiftedTicks != G::MaxShift && G::PrimaryWeaponType != EWeaponType::HITSCAN && Vars::Aimbot::General::AimType.Value == Vars::Aimbot::General::AimTypeEnum::Silent)
		G::AnticipatedChoke = 1;
	if (G::ChokeAmount && !Vars::CL_Move::Fakelag::UnchokeOnAttack.Value && G::ShiftedTicks == G::ShiftedGoal && !G::DoubleTap)
		G::AnticipatedChoke = G::ChokeGoal - G::ChokeAmount; // iffy, unsure if there is a good way to get it to work well without unchoking
}

void CBacktrack::ResolverUpdate(CBaseEntity* pEntity)
{
	mRecords[pEntity].clear();	//	TODO: eventually remake records and rotate them or smthn idk, maybe just rotate them
}

void CBacktrack::ReportShot(int iIndex)
{
	if (!Vars::Backtrack::PreferOnShot.Value)
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
	if (!pEntity || SDK::GetWeaponType(pEntity->As<CTFPlayer>()->m_hActiveWeapon().Get()->As<CTFWeaponBase>()) != EWeaponType::HITSCAN)
		return;

	mDidShoot[pEntity->entindex()] = true;
}

// Adjusts the fake latency ping
void CBacktrack::AdjustPing(CNetChannel* pNetChan)
{
	for (auto& cSequence : dSequences)
	{
		if (I::GlobalVars->realtime - cSequence.CurTime >= GetFake())
		{
			pNetChan->m_nInReliableState = cSequence.InReliableState;
			pNetChan->m_nInSequenceNr = cSequence.SequenceNr;
			break;
		}
	}
}