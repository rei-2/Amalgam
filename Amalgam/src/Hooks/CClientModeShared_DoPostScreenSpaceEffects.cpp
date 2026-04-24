#include "../SDK/SDK.h"

#include "../Features/Visuals/Chams/Chams.h"
#include "../Features/Visuals/Glow/Glow.h"
#include "../Features/Visuals/CameraWindow/CameraWindow.h"
#include "../Features/Visuals/Visuals.h"
#include "../Features/Visuals/Materials/Materials.h"
#include "../Features/Spectate/Spectate.h"

MAKE_SIGNATURE(CViewRender_DrawViewModels, "client.dll", "48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 8B FA", 0x0);

MAKE_HOOK(CClientModeShared_DoPostScreenSpaceEffects, U::Memory.GetVirtual(I::ClientModeShared, 39), bool,
	void* rcx, const CViewSetup* pSetup)
{
	DEBUG_RETURN(CClientModeShared_DoPostScreenSpaceEffects, rcx, pSetup);

	if (SDK::CleanScreenshot() || G::Unload)
		return CALL_ORIGINAL(rcx, pSetup);
	
	F::Visuals.ProjectileTrace(H::Entities.GetLocal(), H::Entities.GetWeapon());
	if (F::CameraWindow.m_bDrawing)
		return CALL_ORIGINAL(rcx, pSetup);

	F::Visuals.DrawEffects();
	F::Chams.m_mEntities.clear();
	if (I::EngineVGui->IsGameUIVisible() || !F::Materials.m_bLoaded)
		return CALL_ORIGINAL(rcx, pSetup);

	F::Chams.RenderMain();
	F::Glow.RenderFirst();

	return CALL_ORIGINAL(rcx, pSetup);
}

MAKE_HOOK(CViewRender_DrawViewModels, S::CViewRender_DrawViewModels(), void,
	void* rcx, const CViewSetup& viewRender, bool drawViewmodel)
{
	DEBUG_RETURN(CViewRender_DrawViewModels, rcx, viewRender, drawViewmodel);

	CALL_ORIGINAL(rcx, viewRender, F::Spectate.HasTarget() && !I::EngineClient->IsHLTV() ? false : drawViewmodel);

	if (SDK::CleanScreenshot() || F::CameraWindow.m_bDrawing || I::EngineVGui->IsGameUIVisible() || !F::Materials.m_bLoaded)
		return;

	F::Glow.RenderSecond();
}