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
	if (!Vars::Misc::Game::NetworkFix.Value && !SDK::IsLoopback())
		F::Backtrack.m_iTickCount--;

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();

	F::Binds.Run(pLocal, pWeapon);
	F::PlayerCore.Run();
	F::Backtrack.SendLerp();
	F::Misc.PingReducer();
	F::AutoQueue.Run();

	F::Ticks.Move(accumulated_extra_samples, bFinalTick, pLocal);

	for (auto& Line : G::PathStorage)
	{
		if (Line.m_flTime < 0.f)
			Line.m_flTime = std::min(Line.m_flTime + 1.f, 0.f);
	}
}