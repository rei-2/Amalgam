#include "../SDK/SDK.h"

#include "../Features/EnginePrediction/EnginePrediction.h"
#include "../Features/Ticks/Ticks.h"

MAKE_SIGNATURE(CPrediction_RunSimulation, "client.dll", "48 83 EC 38 4C 8B 44", 0x0);

struct TickbaseFix_t
{
	CUserCmd* m_pCmd;
	int m_iLastOutgoingCommand;
	int m_iTickbaseShift;
};
static std::vector<TickbaseFix_t> s_vTickbaseFixes = {};

MAKE_HOOK(CPrediction_RunSimulation, S::CPrediction_RunSimulation(), void,
	void* rcx, int current_command, float curtime, CUserCmd* cmd, CTFPlayer* localPlayer)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CPrediction_RunSimulation[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx, current_command, curtime, cmd, localPlayer);
#endif

	if (F::Ticks.m_bShifting && F::Ticks.m_iShiftedTicks + 1 == F::Ticks.m_iShiftStart)
		s_vTickbaseFixes.emplace_back(cmd, I::ClientState->lastoutgoingcommand, F::Ticks.m_iShiftStart - F::Ticks.m_iShiftedGoal);

	for (auto it = s_vTickbaseFixes.begin(); it != s_vTickbaseFixes.end();)
	{
		if (it->m_iLastOutgoingCommand < I::ClientState->last_command_ack)
		{
			it = s_vTickbaseFixes.erase(it);
			continue;
		}
		if (cmd == it->m_pCmd)
		{
			localPlayer->m_nTickBase() -= it->m_iTickbaseShift;
			break;
		}
		++it;
	}

	F::EnginePrediction.ScalePlayers(localPlayer);
	CALL_ORIGINAL(rcx, current_command, curtime, cmd, localPlayer);
	F::EnginePrediction.RestorePlayers();
}