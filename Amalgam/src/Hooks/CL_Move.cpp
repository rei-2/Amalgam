#include "../SDK/SDK.h"

#include "../Features/TickHandler/TickHandler.h"
#include "../Features/Conditions/Conditions.h"
#include "../Features/CheaterDetection/CheaterDetection.h"
#include "../Features/Players/PlayerCore.h"
#include "../Features/AutoQueue/AutoQueue.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"

MAKE_SIGNATURE(CL_Move, "engine.dll", "40 55 53 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 83 3D", 0x0);

MAKE_HOOK(CL_Move, S::CL_Move(), void, __fastcall,
	float accumulated_extra_samples, bool bFinalTick)
{
	if (G::Unload)
		return CALL_ORIGINAL(accumulated_extra_samples, bFinalTick);

	F::Backtrack.iTickCount = I::GlobalVars->tickcount;

	auto pLocal = H::Entities.GetLocal();
	auto pWeapon = H::Entities.GetWeapon();

	F::Conditions.Run(pLocal, pWeapon);
	F::CheaterDetection.Run();
	F::Ticks.Run(accumulated_extra_samples, bFinalTick, pLocal);
	F::PlayerCore.Run();
	F::AutoQueue.Run();
	F::NoSpreadHitscan.AskForPlayerPerf();
	I::EngineClient->FireEvents();

	for (auto& Line : G::LinesStorage)
	{
		if (Line.m_flTime < 0.f)
			Line.m_flTime = std::min(Line.m_flTime + 1.f, 0.f);
	}
}