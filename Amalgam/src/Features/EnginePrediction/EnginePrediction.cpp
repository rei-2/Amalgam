#include "EnginePrediction.h"

#include "../Ticks/Ticks.h"

// account for origin compression when simulating local player
void CEnginePrediction::ScalePlayers(CBaseEntity* pLocal)
{
	m_mRestore.clear();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pLocal || !pPlayer->IsAlive() || pPlayer->IsAGhost())
			continue;

		m_mRestore[pPlayer] = { pPlayer->m_vecMins(), pPlayer->m_vecMaxs() };

		pPlayer->m_vecMins() += 0.125f;
		pPlayer->m_vecMaxs() -= 0.125f;
	}
}
void CEnginePrediction::RestorePlayers()
{
	for (auto& [pPlayer, tRestore] : m_mRestore)
	{
		pPlayer->m_vecMins() = tRestore.m_vecMins;
		pPlayer->m_vecMaxs() = tRestore.m_vecMaxs;
	}
}

void CEnginePrediction::Simulate(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!I::MoveHelper)
		return;

	const int nOldTickBase = pLocal->m_nTickBase();
	const bool bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	const bool bOldInPrediction = I::Prediction->m_bInPrediction;

	I::MoveHelper->SetHost(pLocal);
	pLocal->m_pCurrentCommand() = pCmd;
	*G::RandomSeed() = MD5_PseudoRandom(pCmd->command_number) & std::numeric_limits<int>::max();

	I::Prediction->m_bFirstTimePredicted = false;
	I::Prediction->m_bInPrediction = true;
	I::Prediction->SetLocalViewAngles(pCmd->viewangles);

	Vec2 vOriginalMove; int iOriginalButtons;
	if (m_bDoubletap = m_bInPrediction && (F::Ticks.m_bAntiWarp || F::Ticks.GetTicks(H::Entities.GetWeapon()) && Vars::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity()))
	{
		m_vOriginalOrigin = pLocal->m_vecOrigin();
		m_vOriginalVelocity = pLocal->m_vecVelocity();
		vOriginalMove = { pCmd->forwardmove, pCmd->sidemove };
		iOriginalButtons = pCmd->buttons;

		F::Ticks.AntiWarp(pLocal, pCmd->viewangles.y, pCmd->forwardmove, pCmd->sidemove);
	}

	I::Prediction->SetupMove(pLocal, pCmd, I::MoveHelper, &m_MoveData);
	ScalePlayers(pLocal);
	I::GameMovement->ProcessMovement(pLocal, &m_MoveData);
	RestorePlayers();
	I::Prediction->FinishMove(pLocal, pCmd, &m_MoveData);

	if (m_bDoubletap)
	{
		pCmd->forwardmove = vOriginalMove.x, pCmd->sidemove = vOriginalMove.y;
		pCmd->buttons = iOriginalButtons;
	}

	I::MoveHelper->SetHost(nullptr);
	pLocal->m_pCurrentCommand() = nullptr;
	*G::RandomSeed() = -1;

	pLocal->m_nTickBase() = nOldTickBase;
	I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;
	I::Prediction->m_bInPrediction = bOldInPrediction;

	m_vOrigin = pLocal->m_vecOrigin();
	m_vVelocity = pLocal->m_vecVelocity();
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

	if (m_bDoubletap && !F::Ticks.m_bAntiWarp && !G::Attacking)
	{
		pLocal->m_vecOrigin() = m_vOriginalOrigin;
		pLocal->m_vecVelocity() = m_vOriginalVelocity;
		pLocal->SetAbsVelocity(m_vOriginalVelocity);

		Simulate(pLocal, pCmd);
	}
}