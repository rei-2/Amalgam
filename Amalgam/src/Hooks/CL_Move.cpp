#include "../SDK/SDK.h"

#include "../Features/TickHandler/TickHandler.h"
#include "../Features/Binds/Binds.h"
#include "../Features/Players/PlayerCore.h"
#include "../Features/AutoQueue/AutoQueue.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/Backtrack/Backtrack.h"

MAKE_SIGNATURE(CL_Move, "engine.dll", "40 55 53 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 83 3D", 0x0);

MAKE_HOOK(CL_Move, S::CL_Move(), void, __fastcall,
	float accumulated_extra_samples, bool bFinalTick)
{
	if (G::Unload)
		return CALL_ORIGINAL(accumulated_extra_samples, bFinalTick);

	F::Backtrack.iTickCount = I::GlobalVars->tickcount + (Vars::Misc::Game::NetworkFix.Value ? 1 : 0);

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();

	F::Binds.Run(pLocal, pWeapon);
	F::PlayerCore.Run();
	F::AutoQueue.Run();

	F::Ticks.Run(accumulated_extra_samples, bFinalTick, pLocal);
	I::EngineClient->FireEvents();

	F::NoSpreadHitscan.AskForPlayerPerf();

	for (auto& Line : G::LinesStorage)
	{
		if (Line.m_flTime < 0.f)
			Line.m_flTime = std::min(Line.m_flTime + 1.f, 0.f);
	}
}