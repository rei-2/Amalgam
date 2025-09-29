#include "EnginePrediction.h"

#include "../Ticks/Ticks.h"

// account for interp and origin compression when simulating local player
void CEnginePrediction::AdjustPlayers(CBaseEntity* pLocal)
{
	m_mRestore.clear();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pLocal || !pPlayer->IsAlive() || pPlayer->IsAGhost())
			continue;

		m_mRestore[pPlayer] = { pPlayer->GetAbsOrigin(), pPlayer->m_vecMins(), pPlayer->m_vecMaxs() };

		pPlayer->SetAbsOrigin(pPlayer->m_vecOrigin());
		pPlayer->m_vecMins() += 0.125f;
		pPlayer->m_vecMaxs() -= 0.125f;
	}
}
void CEnginePrediction::RestorePlayers()
{
	for (auto& [pPlayer, tRestore] : m_mRestore)
	{
		pPlayer->SetAbsOrigin(tRestore.m_vOrigin);
		pPlayer->m_vecMins() = tRestore.m_vMins;
		pPlayer->m_vecMaxs() = tRestore.m_vMaxs;
	}
}

void CEnginePrediction::Simulate(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	const int nOldTickBase = pLocal->m_nTickBase();
	const bool bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	const bool bOldInPrediction = I::Prediction->m_bInPrediction;

	I::MoveHelper->SetHost(pLocal);
	pLocal->m_pCurrentCommand() = pCmd;
	*G::RandomSeed() = MD5_PseudoRandom(pCmd->command_number) & std::numeric_limits<int>::max();

	I::Prediction->m_bFirstTimePredicted = false;
	I::Prediction->m_bInPrediction = true;
	I::Prediction->SetLocalViewAngles(pCmd->viewangles);

	AdjustPlayers(pLocal);
	I::Prediction->SetupMove(pLocal, pCmd, I::MoveHelper, &m_MoveData);
	I::GameMovement->ProcessMovement(pLocal, &m_MoveData);
	I::Prediction->FinishMove(pLocal, pCmd, &m_MoveData);
	RestorePlayers();

	I::MoveHelper->SetHost(nullptr);
	pLocal->m_pCurrentCommand() = nullptr;
	*G::RandomSeed() = -1;

	pLocal->m_nTickBase() = nOldTickBase;
	I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;
	I::Prediction->m_bInPrediction = bOldInPrediction;

	m_vOrigin = m_MoveData.m_vecAbsOrigin;
	m_vVelocity = m_MoveData.m_vecVelocity;
	m_vDirection = { m_MoveData.m_flForwardMove, -m_MoveData.m_flSideMove, m_MoveData.m_flUpMove };
	m_vAngles = m_MoveData.m_vecViewAngles;
}



void CEnginePrediction::Start(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	m_bInPrediction = true;
	if (!pLocal->IsAlive())
		return;

	m_nOldTickCount = I::GlobalVars->tickcount;
	m_flOldCurrentTime = I::GlobalVars->curtime;
	m_flOldFrameTime = I::GlobalVars->frametime;

	I::GlobalVars->tickcount = pLocal->m_nTickBase();
	I::GlobalVars->curtime = TICKS_TO_TIME(I::GlobalVars->tickcount);
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;

	m_vOldOrigin = pLocal->m_vecOrigin();
	m_vOldVelocity = pLocal->m_vecVelocity();

	Simulate(pLocal, pCmd);
}

void CEnginePrediction::End(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	m_bInPrediction = false;
	if (!pLocal->IsAlive())
		return;

	I::GlobalVars->tickcount = m_nOldTickCount;
	I::GlobalVars->curtime = m_flOldCurrentTime;
	I::GlobalVars->frametime = m_flOldFrameTime;

	pLocal->SetAbsOrigin(pLocal->m_vecOrigin() = m_vOldOrigin);
	pLocal->SetAbsVelocity(pLocal->m_vecVelocity() = m_vOldVelocity);
}