#include "../SDK/SDK.h"

#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/Ticks/Ticks.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/CheatDetection/CheatDetection.h"
#include "../Features/Resolver/Resolver.h"
#include "../Features/Spectate/Spectate.h"

MAKE_HOOK(CViewRender_LevelInit, U::Memory.GetVirtual(I::ViewRender, 1), void,
	void* rcx)
{
	DEBUG_RETURN(CViewRender_LevelInit, rcx);

	F::Materials.ReloadMaterials();
	F::Visuals.OverrideWorldTextures();

	F::Backtrack.Reset();
	F::Ticks.Reset();
	F::NoSpreadHitscan.Reset();
	F::CheatDetection.Reset();
	F::Resolver.Reset();
	F::Spectate.Reset();

	CALL_ORIGINAL(rcx);
}