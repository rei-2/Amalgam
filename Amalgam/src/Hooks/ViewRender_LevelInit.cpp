#include "../SDK/SDK.h"

#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/CheaterDetection/CheaterDetection.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/TickHandler/TickHandler.h"

MAKE_HOOK(ViewRender_LevelInit, U::Memory.GetVFunc(I::ViewRender, 1), void, __fastcall,
	void* ecx)
{
	F::Materials.ReloadMaterials();
	F::Visuals.OverrideWorldTextures();

	F::Backtrack.Restart();
	F::Ticks.Reset();
	F::NoSpreadHitscan.Reset(true);
	F::CheaterDetection.Reset();

	CALL_ORIGINAL(ecx);
}