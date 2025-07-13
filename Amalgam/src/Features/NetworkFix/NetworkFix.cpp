#include "NetworkFix.h"

void CReadPacketState::Store()
{
	m_flFrameTimeClientState = I::ClientState->m_frameTime;
	m_flFrameTime = I::GlobalVars->frametime;
	m_flCurTime = I::GlobalVars->curtime;
	m_nTickCount = I::GlobalVars->tickcount;
}

void CReadPacketState::Restore()
{
	I::ClientState->m_frameTime = m_flFrameTimeClientState;
	I::GlobalVars->frametime = m_flFrameTime;
	I::GlobalVars->curtime = m_flCurTime;
	I::GlobalVars->tickcount = m_nTickCount;
}

void CNetworkFix::FixInputDelay(bool bFinalTick)
{
	static auto CL_ReadPackets = U::Hooks.m_mHooks["CL_ReadPackets"];
	if (!Vars::Misc::Game::NetworkFix.Value || !I::EngineClient->IsInGame() || SDK::IsLoopback() || !CL_ReadPackets)
		return;

	CReadPacketState Backup = {};
	Backup.Store();

	CL_ReadPackets->Call<void>(bFinalTick);
	m_State.Store();

	Backup.Restore();
}

bool CNetworkFix::ShouldReadPackets()
{
	if (!Vars::Misc::Game::NetworkFix.Value || !I::EngineClient->IsInGame() || SDK::IsLoopback())
		return true;

	m_State.Restore();
	return false;
}