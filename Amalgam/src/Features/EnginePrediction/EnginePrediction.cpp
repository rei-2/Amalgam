#include "EnginePrediction.h"

#include "../TickHandler/TickHandler.h"

int CEnginePrediction::GetTickbase(CUserCmd* pCmd, CTFPlayer* pLocal)
{
	static int nTick = 0;
	static CUserCmd* pLastCmd = nullptr;

	if (pCmd)
	{
		if (!pLastCmd || pLastCmd->hasbeenpredicted)
			nTick = pLocal->m_nTickBase();
		else
			nTick++;
		pLastCmd = pCmd;
	}

	return nTick;
}

void CEnginePrediction::Simulate(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || !pLocal->IsAlive() || !I::MoveHelper)
		return;

	pLocal->SetCurrentCmd(pCmd);

	*G::RandomSeed() = MD5_PseudoRandom(pCmd->command_number) & std::numeric_limits<int>::max();

	const int nOldTickBase = pLocal->m_nTickBase();
	const bool bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	const bool bOldInPrediction = I::Prediction->m_bInPrediction;

	I::Prediction->m_bFirstTimePredicted = false;
	I::Prediction->m_bInPrediction = true;

	I::Prediction->SetLocalViewAngles(pCmd->viewangles);

	I::Prediction->SetupMove(pLocal, pCmd, I::MoveHelper, &m_MoveData);
	I::GameMovement->ProcessMovement(pLocal, &m_MoveData);
	I::Prediction->FinishMove(pLocal, pCmd, &m_MoveData);

	pLocal->m_nTickBase() = nOldTickBase;
	I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;
	I::Prediction->m_bInPrediction = bOldInPrediction;
}

void CEnginePrediction::Start(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || !pLocal->IsAlive())
		return;

	m_nOldTickCount = I::GlobalVars->tickcount;
	m_fOldCurrentTime = I::GlobalVars->curtime;
	m_fOldFrameTime = I::GlobalVars->frametime;

	I::GlobalVars->tickcount = GetTickbase(pCmd, pLocal);
	I::GlobalVars->curtime = TICKS_TO_TIME(I::GlobalVars->tickcount);
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;

	bSimulated = false;
	if (F::Ticks.GetTicks(pLocal) && Vars::CL_Move::Doubletap::AntiWarp.Value && pLocal->OnSolid())
		return; // hopefully more accurate eyepos while dting

	bSimulated = true;
	Simulate(pLocal, pCmd);
}

void CEnginePrediction::End(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || !pLocal->IsAlive())
		return;

	I::GlobalVars->tickcount = m_nOldTickCount;
	I::GlobalVars->curtime = m_fOldCurrentTime;
	I::GlobalVars->frametime = m_fOldFrameTime;

	pLocal->SetCurrentCmd(nullptr);

	*G::RandomSeed() = -1;

	if (!bSimulated)
		Simulate(pLocal, pCmd);
}