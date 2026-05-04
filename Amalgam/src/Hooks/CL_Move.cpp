#include "../SDK/SDK.h"

#include "../Features/NetworkFix/NetworkFix.h"
#include "../Features/Ticks/Ticks.h"
#include "../Features/Binds/Binds.h"
#include "../Features/Players/PlayerCore.h"
#include "../Features/Misc/AutoQueue/AutoQueue.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/Misc/Misc.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Output/Output.h"

MAKE_SIGNATURE(CL_Move, "engine.dll", "40 55 53 48 8D AC 24 ? ? ? ? B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 83 3D", 0x0);

MAKE_HOOK(CL_Move, S::CL_Move(), void,
	float accumulated_extra_samples, bool bFinalTick)
{
	DEBUG_RETURN(CL_Move, accumulated_extra_samples, bFinalTick);

	if (G::Unload)
		return CALL_ORIGINAL(accumulated_extra_samples, bFinalTick);

	F::NetworkFix.FixInputDelay(bFinalTick);
	F::Backtrack.m_iTickCount = I::GlobalVars->tickcount + 1;
	if (!Vars::Misc::Game::NetworkFix.Value && !SDK::IsLoopback())
		F::Backtrack.m_iTickCount--;

	F::Binds.Run();
	H::ConVars.Modify(Vars::Misc::Exploits::UnlockCVars.Value);
	F::Backtrack.SendLerp();
	F::Misc.PingReducer();

	F::Ticks.Move(accumulated_extra_samples, bFinalTick);

	F::PlayerCore.Run();
	F::AutoQueue.Run();
	F::Visuals.Tick();
	F::Output.Move();
}