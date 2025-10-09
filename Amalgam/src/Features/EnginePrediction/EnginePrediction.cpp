#include "EnginePrediction.h"

#include "../Ticks/Ticks.h"

// account for interp and origin compression when simulating local player
void CEnginePrediction::AdjustPlayers(CBaseEntity* pLocal)
{
	m_mRestore.clear();

	for (auto pEntity : H::Entities.GetGroup(EGroupType::PLAYERS_ALL))
	{
		auto pPlayer = pEntity->As<CTFPlayer>();
		if (pPlayer == pLocal || pPlayer->IsDormant() || !pPlayer->IsAlive() || pPlayer->IsAGhost())
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
	FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file) {
		fprintf(log_file, "EnginePrediction::Simulate - Function called\n");
		fclose(log_file);
	}

	if (!I::MoveHelper)
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "EnginePrediction::Simulate - MoveHelper is null, returning\n");
			fclose(log_file);
		}
		return;
	}

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

		bool bOriginalWarp = F::Ticks.m_bAntiWarp;
		F::Ticks.m_bAntiWarp = true;
		F::Ticks.AntiWarp(pLocal, pCmd);
		F::Ticks.m_bAntiWarp = bOriginalWarp;
	}

	AdjustPlayers(pLocal);
	I::Prediction->SetupMove(pLocal, pCmd, I::MoveHelper, &m_MoveData);
	I::GameMovement->ProcessMovement(pLocal, &m_MoveData);
	I::Prediction->FinishMove(pLocal, pCmd, &m_MoveData);
	RestorePlayers();

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
	// File-based logging that works in Release builds
	FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file) {
		fprintf(log_file, "EnginePrediction::Start - Function called\n");
		fclose(log_file);
	}

	m_bInPrediction = true;
	if (!pLocal || !pLocal->IsAlive())
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "EnginePrediction::Start - pLocal invalid or not alive, returning\n");
			fclose(log_file);
		}
		return;
	}

	auto pMap = pLocal->GetPredDescMap();
	if (!pMap)
		return;

	m_nOldTickCount = I::GlobalVars->tickcount;
	m_flOldCurrentTime = I::GlobalVars->curtime;
	m_flOldFrameTime = I::GlobalVars->frametime;

	I::GlobalVars->tickcount = pLocal->m_nTickBase();
	I::GlobalVars->curtime = TICKS_TO_TIME(I::GlobalVars->tickcount);
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;

	size_t iSize = pLocal->GetIntermediateDataSize();
	if (!I::MemAlloc)
	{
		// MemAlloc interface not initialized - skip prediction
		return;
	}

	if (!m_tLocal.m_pData)
	{
		m_tLocal.m_pData = reinterpret_cast<byte*>(I::MemAlloc->Alloc(iSize));
		m_tLocal.m_iSize = iSize;
	}
	else if (m_tLocal.m_iSize != iSize)
	{
		m_tLocal.m_pData = reinterpret_cast<byte*>(I::MemAlloc->Realloc(m_tLocal.m_pData, iSize));
		m_tLocal.m_iSize = iSize;
	}

	CPredictionCopy copy = { PC_EVERYTHING, m_tLocal.m_pData, PC_DATA_PACKED, pLocal, PC_DATA_NORMAL };
	FILE* log_file1 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file1) {
		fprintf(log_file1, "EnginePrediction::Start - About to call TransferData START\n");
		fclose(log_file1);
	}

	// TEMPORARY: Disable CPredictionCopy completely to isolate crash
	// TODO: Remove this when crash is fixed
	const bool DISABLE_CPREDICTIONCOPY = true;
	if (DISABLE_CPREDICTIONCOPY)
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "EnginePrediction::Start - CPredictionCopy DISABLED, calling Simulate directly\n");
			fclose(log_file);
		}
		Simulate(pLocal, pCmd);
		return;
	}

	int result = copy.TransferData("EnginePredictionStart", pLocal->entindex(), pMap);
	if (result == -1)
	{
		// Signature failed - skip prediction to prevent crash
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "EnginePrediction::Start - TransferData START failed, skipping prediction\n");
			fclose(log_file);
		}
		return;
	}
	FILE* log_file2 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file2) {
		fprintf(log_file2, "EnginePrediction::Start - TransferData START success, calling Simulate\n");
		fclose(log_file2);
	}
	Simulate(pLocal, pCmd);
}

void CEnginePrediction::End(CTFPlayer* pLocal, CUserCmd* pCmd)
{
	m_bInPrediction = false;
	if (!pLocal || !pLocal->IsAlive())
		return;

	auto pMap = pLocal->GetPredDescMap();
	if (!pMap)
		return;

	I::GlobalVars->tickcount = m_nOldTickCount;
	I::GlobalVars->curtime = m_flOldCurrentTime;
	I::GlobalVars->frametime = m_flOldFrameTime;

	CPredictionCopy copy = { PC_EVERYTHING, pLocal, PC_DATA_NORMAL, m_tLocal.m_pData, PC_DATA_PACKED };
	FILE* log_file1 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file1) {
		fprintf(log_file1, "EnginePrediction::End - About to call TransferData END\n");
		fclose(log_file1);
	}

	// TEMPORARY: Disable CPredictionCopy completely to isolate crash
	// TODO: Remove this when crash is fixed
	const bool DISABLE_CPREDICTIONCOPY = true;
	if (DISABLE_CPREDICTIONCOPY)
	{
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "EnginePrediction::End - CPredictionCopy DISABLED, skipping restore\n");
			fclose(log_file);
		}
		return;
	}

	int result = copy.TransferData("EnginePredictionEnd", pLocal->entindex(), pMap);
	if (result == -1)
	{
		// Signature failed - skip restore to prevent crash
		FILE* log_file = fopen("C:\\temp\\amalgam_debug.log", "a");
		if (log_file) {
			fprintf(log_file, "EnginePrediction::End - TransferData END failed, skipping restore\n");
			fclose(log_file);
		}
		return;
	}
	FILE* log_file2 = fopen("C:\\temp\\amalgam_debug.log", "a");
	if (log_file2) {
		fprintf(log_file2, "EnginePrediction::End - TransferData END success\n");
		fclose(log_file2);
	}
}

void CEnginePrediction::Unload()
{
	if (m_tLocal.m_pData && I::MemAlloc)
	{
		I::MemAlloc->Free(m_tLocal.m_pData);
		m_tLocal = {};
	}
}