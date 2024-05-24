#include "Backtrack.h"

//#include "../Simulation/MovementSimulation/MovementSimulation.h"

#define ROUND_TO_TICKS(t) (TICKS_TO_TIME(TIME_TO_TICKS(t)))

void CBacktrack::Restart()
{
	mRecords.clear();
	dSequences.clear();
	iLastInSequence = 0;
}



// Returns the wish cl_interp
float CBacktrack::GetLerp()
{
	return Vars::Backtrack::Enabled.Value ? std::clamp(static_cast<float>(Vars::Backtrack::Interp.Value), G::Lerp * 1000.f, flMaxUnlag * 1000.f) / 1000.f : G::Lerp;
}

// Returns the current (custom) backtrack latency
float CBacktrack::GetFake()
{
	return bFakeLatency ? std::clamp(static_cast<float>(Vars::Backtrack::Latency.Value), 0.f, flMaxUnlag * 1000.f) / 1000.f : 0.f;
}

// Returns the current real latency
float CBacktrack::GetReal(int iFlow)
{
	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pNetChan)
		return 0.f;

	if (iFlow != -1)
		return pNetChan->GetLatency(iFlow) - (iFlow == FLOW_INCOMING ? GetFake() : 0.f);
	return pNetChan->GetLatency(FLOW_INCOMING) - GetFake() + pNetChan->GetLatency(FLOW_OUTGOING);
}

void CBacktrack::SendLerp()
{
	auto pNetChan = reinterpret_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
	if (!pNetChan)
		return;

	static Timer interpTimer{};
	if (interpTimer.Run(100))
	{
		float flTarget = GetLerp();
		if (flTarget == flWishInterp) return;
		flWishInterp = flTarget;

		SDK::Output("SendNetMsg", std::format("cl_interp: {}", flTarget).c_str(), { 224, 255, 131, 255 }, Vars::Debug::Logging.Value);

		NET_SetConVar cl_interp("cl_interp", std::to_string(flTarget).c_str());
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
	const bool bLocal = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")) == I::EngineClient->GetLocalPlayer();
	if (bLocal)
		flFakeInterp = flWishInterp;
}

// Store the last 2048 sequences
void CBacktrack::UpdateDatagram()
{
	auto pNetChan = static_cast<CNetChannel*>(I::EngineClient->GetNetChannelInfo());
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

	const float flCorrect = std::clamp(pNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(flFakeInterp) + GetFake(), 0.f, flMaxUnlag) - pNetChan->GetLatency(FLOW_OUTGOING);
	const int iServerTick = iTickCount + (Vars::Misc::Game::NetworkFix.Value ? 1 : 0) + G::AnticipatedChoke + Vars::Backtrack::Offset.Value;

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

			const float flCorrect = std::clamp(pNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(flFakeInterp) + GetFake(), 0.f, flMaxUnlag) - pNetChan->GetLatency(FLOW_OUTGOING);
			const int iServerTick = iTickCount + (Vars::Misc::Game::NetworkFix.Value ? 1 : 0) + G::AnticipatedChoke + Vars::Backtrack::Offset.Value;

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



void CBacktrack::StoreNolerp()
{
	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		if (pEntity->entindex() == I::EngineClient->GetLocalPlayer())
			continue;
		
		// more of a placeholder, still interpolated iirc
		bSettingUpBones = true;
		mBones[pEntity].first = pEntity->SetupBones(mBones[pEntity].second, 128, BONE_USED_BY_ANYTHING, pEntity->m_flSimulationTime());
		bSettingUpBones = false;

		mEyeAngles[pEntity] = pEntity->As<CTFPlayer>()->GetEyeAngles();
	}
}

void CBacktrack::MakeRecords()
{
	if (iLastCreationTick == I::GlobalVars->tickcount)
		return;
	iLastCreationTick = I::GlobalVars->tickcount;

	for (auto& pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		if (pEntity->entindex() == I::EngineClient->GetLocalPlayer() || !mBones[pEntity].first)
			continue;

		const float flSimTime = pEntity->m_flSimulationTime(), flOldSimTime = pEntity->m_flOldSimulationTime();
		if (TIME_TO_TICKS(flSimTime - flOldSimTime) <= 0)
			continue;

		const TickRecord curRecord = {
			flSimTime,
			I::GlobalVars->curtime,
			I::GlobalVars->tickcount,
			mDidShoot[pEntity->entindex()],
			*reinterpret_cast<BoneMatrixes*>(&mBones[pEntity].second),
			pEntity->m_vecOrigin()
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

				pRecord.bOnShot = curRecord.bOnShot;
				pRecord.BoneMatrix = curRecord.BoneMatrix;
				pRecord.vOrigin = curRecord.vOrigin;
			}
		}

		mRecords[pEntity].push_front(curRecord);
		mLagCompensation[pEntity] = bLagComp;

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

	StoreNolerp();
	MakeRecords();
	CleanRecords();
}

void CBacktrack::Run(CUserCmd* pCmd)
{
	SendLerp();

	// might not even be necessary
	G::AnticipatedChoke = 0;
	if (G::ShiftedTicks != G::MaxShift && G::WeaponType != EWeaponType::HITSCAN && Vars::Aimbot::General::AimType.Value == 3)
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
void CBacktrack::AdjustPing(CNetChannel* netChannel)
{
	for (const auto& cSequence : dSequences)
	{
		if (I::GlobalVars->realtime - cSequence.CurTime >= GetFake())
		{
			netChannel->m_nInReliableState = cSequence.InReliableState;
			netChannel->m_nInSequenceNr = cSequence.SequenceNr;
			break;
		}
	}
}