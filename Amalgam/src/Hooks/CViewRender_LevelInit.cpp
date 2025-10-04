#include "../SDK/SDK.h"

#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Visuals/FlatTextures/FlatTextures.h"
#include "../Features/Backtrack/Backtrack.h"
#include "../Features/Ticks/Ticks.h"
#include "../Features/NoSpread/NoSpreadHitscan/NoSpreadHitscan.h"
#include "../Features/CheaterDetection/CheaterDetection.h"
#include "../Features/Resolver/Resolver.h"
#include "../Features/Spectate/Spectate.h"

MAKE_HOOK(CViewRender_LevelInit, U::Memory.GetVirtual(I::ViewRender, 1), void,
	void* rcx)
{
#ifdef DEBUG_HOOKS
	if (!Vars::Hooks::CViewRender_LevelInit[DEFAULT_BIND])
		return CALL_ORIGINAL(rcx);
#endif

	F::Materials.ReloadMaterials();
	F::Visuals.OverrideWorldTextures();

	// Clear avatar cache on map change to prevent memory leaks and invalid textures
	H::Draw.OnMapChange();

	F::Backtrack.Reset();
	F::Ticks.Reset();
	F::NoSpreadHitscan.Reset();
	F::CheaterDetection.Reset();
	F::Resolver.Reset();
	F::Spectate.m_iIntendedTarget = -1;

	CALL_ORIGINAL(rcx);
	
	F::FlatTextures.OnLevelInitPostEntity();
}