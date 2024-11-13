#include "EnginePrediction.h"

#include "../TickHandler/TickHandler.h"



// account for origin tolerance when simulating local player

void CEnginePrediction::ScalePlayers(CBaseEntity* pLocal)
{
	m_mRestore.clear();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		if (pEntity == pLocal || pEntity->IsDormant() || pEntity->As<CTFPlayer>()->IsAlive())
			continue;

		m_mRestore[pEntity] = { pEntity->m_vecMins(), pEntity->m_vecMaxs() };

		pEntity->m_vecMins() += 0.125f;
		pEntity->m_vecMaxs() -= 0.125f;
	}
}

void CEnginePrediction::RestorePlayers()
{
	for (auto& [pEntity, tRestore] : m_mRestore)
	{
		pEntity->m_vecMins() = tRestore.m_vecMins;
		pEntity->m_vecMaxs() = tRestore.m_vecMaxs;
	}
}



void CEnginePrediction::Simulate(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!I::MoveHelper)
		return;

	const int nOldTickBase = pLocal->m_nTickBase();
	const bool bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	const bool bOldInPrediction = I::Prediction->m_bInPrediction;

	pLocal->SetCurrentCmd(pCmd);
	*G::RandomSeed() = MD5_PseudoRandom(pCmd->command_number) & std::numeric_limits<int>::max();

	I::Prediction->m_bFirstTimePredicted = false;
	I::Prediction->m_bInPrediction = true;
	I::Prediction->SetLocalViewAngles(pCmd->viewangles);

	I::Prediction->SetupMove(pLocal, pCmd, I::MoveHelper, &m_MoveData);
	ScalePlayers(pLocal);
	I::GameMovement->ProcessMovement(pLocal, &m_MoveData);
	RestorePlayers();
	I::Prediction->FinishMove(pLocal, pCmd, &m_MoveData);

	pLocal->SetCurrentCmd(nullptr);
	*G::RandomSeed() = -1;

	pLocal->m_nTickBase() = nOldTickBase;
	I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;
	I::Prediction->m_bInPrediction = bOldInPrediction;

	vOrigin = pLocal->m_vecOrigin();
	vVelocity = pLocal->m_vecVelocity();
	vDirection = { m_MoveData.m_flForwardMove, -m_MoveData.m_flSideMove, m_MoveData.m_flUpMove };
	vAngles = m_MoveData.m_vecViewAngles;
}



void CEnginePrediction::Start(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || !pLocal->IsAlive())
		return;

	m_nOldTickCount = I::GlobalVars->tickcount;
	m_flOldCurrentTime = I::GlobalVars->curtime;
	m_flOldFrameTime = I::GlobalVars->frametime;

	I::GlobalVars->tickcount = pLocal->m_nTickBase();
	I::GlobalVars->curtime = TICKS_TO_TIME(I::GlobalVars->tickcount);
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;

	// fix
	if (pLocal->m_hGroundEntity())
		pLocal->m_fFlags() |= FL_ONGROUND;
	else
		pLocal->m_fFlags() &= ~FL_ONGROUND;

	// hopefully more accurate eyepos while dting
	m_bDoubletap = F::Ticks.GetTicks(H::Entities.GetWeapon()) && Vars::CL_Move::Doubletap::AntiWarp.Value && pLocal->m_hGroundEntity();

	if (m_bDoubletap)
	{
		Vec3 vOrigin = pLocal->m_vecOrigin();

		Simulate(pLocal, pCmd);
		
		m_vOrigin = pLocal->m_vecOrigin();
		pLocal->m_vecOrigin() = vOrigin;
	}
	else
		Simulate(pLocal, pCmd);
}

void CEnginePrediction::End(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || !pLocal->IsAlive())
		return;

	I::GlobalVars->tickcount = m_nOldTickCount;
	I::GlobalVars->curtime = m_flOldCurrentTime;
	I::GlobalVars->frametime = m_flOldFrameTime;

	if (m_bDoubletap)
		pLocal->m_vecOrigin() = m_vOrigin;
}

/*
MAKE_SIGNATURE(CPredictableId_ResetInstanceCounters, "client.dll", "33 D2 C7 05 ? ? ? ? ? ? ? ? 41 B8", 0x0);
MAKE_SIGNATURE(CBaseEntity_UpdateButtonState, "client.dll", "44 8B 81 ? ? ? ? 89 91", 0x0);
MAKE_SIGNATURE(CBaseEntity_PhysicsRunThink, "client.dll", "4C 8B DC 49 89 73 ? 57 48 81 EC ? ? ? ? 8B 81", 0x0);
MAKE_SIGNATURE(CBaseEntity_SetNextThink, "client.dll", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 0F 2E 0D", 0x0);

void CEnginePrediction::Start(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || !pLocal->IsAlive()) //|| G::ShouldShift?
		return;

	m_flOldCurrentTime = I::GlobalVars->curtime;
	m_flOldFrameTime = I::GlobalVars->frametime;
	m_nOldTickCount = I::GlobalVars->tickcount;



	const int nOldTickBase = pLocal->m_nTickBase();
	const bool bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	const bool bOldInPrediction = I::Prediction->m_bInPrediction;



	I::Prediction->m_bFirstTimePredicted = false;
	I::Prediction->m_bInPrediction = true;

	S::CPredictableId_ResetInstanceCounters.As<void(__cdecl*)()>()();
	pLocal->SetCurrentCmd(pCmd);
	*G::RandomSeed() = MD5_PseudoRandom(pCmd->command_number) & std::numeric_limits<int>::max();
	//I::MoveHelper->SetHost(pLocal);

	I::GlobalVars->tickcount = pLocal->m_nTickBase();
	I::GlobalVars->curtime = TICKS_TO_TIME(I::GlobalVars->tickcount);
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;

	I::GameMovement->StartTrackPredictionErrors(pLocal);

	if (pCmd->weaponselect)
	{
		if (auto pWeapon = I::ClientEntityList->GetClientEntity(pCmd->weaponselect)->As<CTFWeaponBase>())
			pLocal->SelectItem(pWeapon->GetName(), pCmd->weaponsubtype);
	}

	//pLocal->UpdateButtonState(pCmd->buttons);
	S::CBaseEntity_UpdateButtonState.As<void(__fastcall*)(void*, int)>()(pLocal, pCmd->buttons);

	I::Prediction->SetLocalViewAngles(pCmd->viewangles);


	//if (pLocal->PhysicsRunThink())
	if (S::CBaseEntity_PhysicsRunThink.As<bool(__fastcall*)(void*, int)>()(pLocal, 0))
		pLocal->PreThink();

	{
		int iThinkTick = pLocal->m_nNextThinkTick();
		if (iThinkTick > 0 && iThinkTick <= I::GlobalVars->tickcount)
		{
			//pLocal->SetNextThink(-1);
			S::CBaseEntity_SetNextThink.As<void(__fastcall*)(void*, float, const char*)>()(pLocal, -1, nullptr);
			pLocal->Think();
		}
	}

	I::Prediction->SetupMove(pLocal, pCmd, I::MoveHelper, &m_MoveData);

	I::GameMovement->ProcessMovement(pLocal, &m_MoveData);

	I::Prediction->FinishMove(pLocal, pCmd, &m_MoveData);

	pLocal->PostThink();

	I::GameMovement->FinishTrackPredictionErrors(pLocal);

	pLocal->SetCurrentCmd(nullptr);
	*G::RandomSeed() = -1;
	//I::MoveHelper->SetHost(nullptr);



	pLocal->m_nTickBase() = nOldTickBase;
	I::Prediction->m_bInPrediction = bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;
}

void CEnginePrediction::End(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	if (!pLocal || !pLocal->IsAlive()) //|| G::ShouldShift?
		return;

	I::GlobalVars->curtime = m_flOldCurrentTime;
	I::GlobalVars->frametime = m_flOldFrameTime;
	I::GlobalVars->tickcount = m_nOldTickCount;
}
*/