#include "../SDK/SDK.h"

#include "../Features/Ticks/Ticks.h"
#include "../Features/Binds/Binds.h"
#include "../Features/Players/PlayerCore.h"
#include "../Features/Misc/AutoQueue/AutoQueue.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/Misc/Misc.h"

MAKE_SIGNATURE(CL_Move, "engine.dll", "40 55 53 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 83 3D", 0x0);

MAKE_HOOK(CL_Move, S::CL_Move(), void,
	float accumulated_extra_samples, bool bFinalTick)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CL_Move[DEFAULT_BIND])
		return CALL_ORIGINAL(accumulated_extra_samples, bFinalTick);
#endif

	if (G::Unload)
		return CALL_ORIGINAL(accumulated_extra_samples, bFinalTick);

	F::Backtrack.m_iTickCount = I::GlobalVars->tickcount + 1;
	if (!Vars::Misc::Game::NetworkFix.Value)
	{
		auto pNetChan = I::EngineClient->GetNetChannelInfo();
		if (pNetChan && !pNetChan->IsLoopback())
			F::Backtrack.m_iTickCount++;
	}

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();

	/*
	// proof of concept for handling lost ticks, leaving this commented out for now. probably a much better way
	if (pLocal && F::Ticks.m_iShiftedTicks && !F::Ticks.m_bShifted)
	{
		static int iStaticTick = 0;
		const int iLastTick = iStaticTick;
		const int iCurrTick = iStaticTick = pLocal->m_nTickBase();

		static int iStaticRecharge = 0;
		const int iCurrRecharge = std::max(F::Ticks.m_iShiftedGoal - F::Ticks.m_iShiftStart + 1, 0);
		if (iCurrRecharge)
			iStaticRecharge = iCurrRecharge;
		else
		{
			int iDeficit = iCurrTick - iLastTick;
			//if (1 < iDeficit && iDeficit < iStaticRecharge)
			if (1 < iDeficit)
			{
				if (!iStaticRecharge)
				{
					//SDK::Output("Deficit", std::format("{}, {}", iDeficit, iStaticRecharge).c_str());

					iDeficit = std::clamp(iDeficit - 1, 0, F::Ticks.m_iShiftedTicks);
					//F::Ticks.m_iDeficit += iDeficit;
					F::Ticks.m_iShiftedTicks -= iDeficit, F::Ticks.m_iShiftedGoal -= iDeficit;
				}
				else
					iStaticRecharge = false;
			}

			//SDK::Output("Tick", std::format("{} ({} - {})", iDeficit, iCurrTick, iLastTick).c_str());
		}
	}
	*/

	F::Binds.Run(pLocal, pWeapon);
	F::PlayerCore.Run();
	F::Backtrack.SendLerp();
	F::Misc.PingReducer();
	F::AutoQueue.Run();

	F::Ticks.Run(accumulated_extra_samples, bFinalTick, pLocal);

	for (auto& Line : G::PathStorage)
	{
		if (Line.m_flTime < 0.f)
			Line.m_flTime = std::min(Line.m_flTime + 1.f, 0.f);
	}
}